/*
 * Copyright (c) 1996-2009 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#if !defined(_InstrucIter_h_) && !defined(cap_instruction_api)
#define _InstrucIter_h_

#include <vector>
#include <set>

#include "common/h/Types.h"
#include "common/h/arch.h"

#include "dyninstAPI/h/BPatch_memoryAccess_NP.h"
#include "dyninstAPI/h/BPatch_instruction.h"

#include "parseAPI/h/InstructionSource.h"
#include "parseAPI/h/CFG.h"

/*
   VG (02/19/02): New abstraction layer for instruction parsing.

   Functionality/design:
   - Allows iteration over instructions, forward and backward.
   - To guarantee backward iteration we only allow iteration
     over well known address spaces (right now only function bodies).
   - The only state stored is the offset position relative to the
     starting address.
   - Any other state, like caching a instruction pointer list for CISC
     cpus is saved via a callback. Typically the container of the
     address space (e.g. function) should store this.
   - Any number of iterators may be created over a given address space.
   - Creating another iterator that starts from the current position 
     is done via the copy constructor.
   - Supports basic predicates abut the current instruction.
   - Gives additional information about certain instructions (jump/calls,
     load/stores etc.)
   - Finding a certain instruction (type) from this point forward. Backward
     search should also be easy to do.

   Most code is derived from Tikir's AddressHandle, but it has:
   - Better (if not complete) encapsulation of the underlying instruction
     class making it transparent for the user. This allows for instance
     the new IA-32 decoder to replace the x86 one transparently.
   - All predicate functions are now methods that test the current instruction.
   - Pimple (pointer-to-implementation) separation for machine specific
     stuff. This does away with ifdefs.
   - A new name :)
*/

/** class for manipulating the address space in an image for a given
  * range. Such as the valid addresses and the instructions in the addresses
  * and iteration on the address apce range for instructions
  */
using namespace Dyninst;

class InstrucIter {
 private:
   
    /* Dyninst::AddressSpace, Dyninst::Process, ParseAPI::CodeSource, etc */
    InstructionSource *instructions_;

    /* Starting address/offset */
    Address base;
    
    /** the range/length of the address space in bytes */
    unsigned range;
    
    /** current address of the address spce which will be used 
     * to iterate through the address space
     */
    Address current;

    void initializeInsn();

    // For iterating backwards over architectures with variable-length
    // instructions.  When you go forwards, you push the address on;
    // when you go back, you pop.
    std::vector<std::pair<Address, void*> >prevInsns;

    instruction insn;
    void* instPtr;

 public:
    
    /** returns the instruction in the address of handle */
    virtual instruction getInstruction();
    
    // And a pointer if we need to worry about virtualization. This is
    // "user needs to get rid of it" defined.
    virtual instruction *getInsnPtr();

 public:
    /** static method that returns true if the delay instruction is supported */
    static bool delayInstructionSupported ();

 protected:
    // For InstrucIterFunction
    InstrucIter() : base(0), range(1), current(0)    
    {    
    }
    
 public:    

    /** copy constructor
     * @param ii InstrucIter to copy
     */
    InstrucIter(const InstrucIter& ii);

    // Shorthand for basic block iteration 
    InstrucIter (ParseAPI::Block *b);
    
    // Generic "address, process" iterator.
    InstrucIter( Address addr, Address size, InstructionSource *source);
    InstrucIter( Address addr, InstructionSource *source); 

  virtual ~InstrucIter() {  }

  /** return true iff the address is in the space represented by
   * the InstrucIter
   */
  bool containsAddress(Address addr);

  /** method that returns the targets of a multi-branch instruction
   * it assumes the currentAddress of the handle instance points to the
   * multi branch instruction
   */
  bool getMultipleJumpTargets( std::set< Address >& result );

  /** method that returns true if there are more instructions to iterate */
  virtual bool hasMore();

  /** method that returns true if there are instruction previous to the
   * current one */
  virtual bool hasPrev();

  /** Peek at the previous address */
  virtual Address peekPrev();

  /** And the next address */
  virtual Address peekNext();

  /** set the content of the handle to the argument 
   * @param addr the value that handle will be set to
   */
  virtual void setCurrentAddress(Address);
  virtual Address getCurrentAddress(){return current;}
  
#if defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */ \
 || defined(i386_unknown_nt4_0)
  /** does the address point to an instruction */
  bool isInstruction();
#endif

  /** returns the content of the address  handle */
  virtual Address operator*() const;
  virtual Address begin() const
  {
    return base;
  }
  virtual Address end() const
  {
    return base + range;
  }
  

  /** prefix increment operation */
  virtual Address operator++ ();

  /** prefix decrement operation */
  virtual Address operator-- ();

  /** postfix increment operation */
  virtual Address operator++ (int);

  /** postfix decrement operation */
  virtual Address operator-- (int);

  /* Predicates */
   
  bool isALeaveInstruction();
  bool isAReturnInstruction();
  bool isACondReturnInstruction();
  bool isACondBranchInstruction();
  bool isAJumpInstruction();
  bool isAIndirectJumpInstruction();
  bool isACallInstruction();
  bool isADynamicCallInstruction();
  bool isAnneal();
  bool isStackFramePreamble();
  bool isReturnValueSave();
  bool isFramePush();
  bool isFrameSetup();
  bool isANopInstruction();
  bool isAnAbortInstruction();
  bool isAnAllocInstruction();
  bool isAnInterruptInstruction(); 
  bool isDelaySlot();
  bool isSyscall();

  bool isTailCall(ParseAPI::Function*);
  bool isIndirectTailCall(ParseAPI::Function*);
  bool isRealCall(InstructionSource * isrc,
    bool &validTarget,
    bool &simulateJump);

  

  /* Power only */

  bool isA_RT_WriteInstruction();
  bool isA_RA_WriteInstruction(); 
 
  bool isA_RT_ReadInstruction();
  bool isA_RA_ReadInstruction(); 
  bool isA_RB_ReadInstruction();

  bool isA_FRT_WriteInstruction(); 
  bool isA_FRA_WriteInstruction(); 

  bool isA_FRT_ReadInstruction(); 
  bool isA_FRA_ReadInstruction(); 
  bool isA_FRB_ReadInstruction();
  bool isA_FRC_ReadInstruction();

  bool isA_MX_Instruction();

  bool isA_MRT_ReadInstruction();
  bool isA_MRT_WriteInstruction();

  bool usesSPR(std::set<Register> &);
  bool definesSPR(std::set<Register> &);

  unsigned getRTValue();
  unsigned getRAValue();
  unsigned getRBValue();
  unsigned getFRTValue();
  unsigned getFRAValue();
  unsigned getFRBValue();
  unsigned getFRCValue();
  signed getDFormDValue();

  bool isClauseInstruction();
  bool isRegConstantAssignment(int * regArray, Address *);

  bool isACondBDZInstruction();
  bool isACondBDNInstruction();

  /* END Power only */

  /* Sparc Only */
  bool isAOMPDoFor();
  bool isTstInsn();
  bool isACondBLEInstruction();

  /* END Sparc Only */

  /* x86 Only */
  void readWriteRegisters(int * readRegs, int * writeRegs);
  bool isFPWrite();
  bool isFPRead();
  void getAllRegistersUsedAndDefined(std::set<Register>& used, std::set<Register> &defined);
  /* END x86 Only */

  // Thought: since we check, we can probably get the data for
  // free.
  bool isInterModuleCallSnippet(Address &info);

  Address getBranchTargetOffset();
  Address getBranchTargetAddress(bool *isAbsolute = NULL);
  Address getCallTarget();

  BPatch_memoryAccess* isLoadOrStore();
  BPatch_instruction* getBPInstruction();

  void printOpCode();

#if defined(arch_sparc)
  // Delay slot happiness. Thing is, a jump ends a basic block (especially
  // since you can branch into the delay slot, and we don't like overlapping
  // basic blocks). And I want to keep that. But we still need a way to say
  // "grab me the delay slot insn", which might _not_ be in a basic block
  // (say, unconditional jump). So I've added two instructions that go by 
  // address and grab delay slot (and aggregate doohickey)

  void getAndSkipDSandAgg(instruction* &ds,
                          instruction* &agg);
  bool isASaveInstruction();
  bool isARestoreInstruction();
  void adjustRegNumbers(int* readRegs, int* writeRegs,int window);
  int adjustRegNumbers(int regNum, int window);
#endif

};

#endif /* _InstrucIter_h_ */
