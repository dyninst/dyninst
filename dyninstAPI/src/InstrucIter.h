/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

#ifndef _InstrucIter_h_
#define _InstrucIter_h_

#include "BPatch_Set.h"
#include "BPatch_point.h"
#include "BPatch_memoryAccess_NP.h"
#include "BPatch_function.h"
#include "function.h"

class InstrucIter;
class bblInstance;

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
class InstrucIter {
    friend class BPatch_instruction;
 protected:
    
    /* We can iterate either over a process address space or 
       within a parsed image */
    // Only one will be true, and that controls all sorts of stuff
    // Probably worth making two InstrucIter classes, but so much is similar.
    process *proc_;
    image *img_;

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

    typedef struct {
        Address prevAddr;
        void *prevPtr;
    } previous;

    pdvector<previous> prevInsns;

 public:
    
    /** returns the instruction in the address of handle */
    instruction getInstruction();
    
    // And a pointer if we need to worry about virtualization. This is
    // "user needs to get rid of it" defined.
    instruction *getInsnPtr();

    /** returns the instruction in the next address of handle */
    instruction getNextInstruction();
    
    /** returns the instruction in the prev address of handle */
    instruction getPrevInstruction();
    
    instruction insn;
    void* instPtr;
#if defined(arch_ia64)
    // We need the bundle to iterate correctly
    IA64_bundle bundle;
#endif
    
 public:
    /** static method that returns true if the delay instruction is supported */
    static bool delayInstructionSupported ();

    // Used in parsing -- relative addrs. Goes over the func linearly,
    // which may be a spectacularly bad idea.
    InstrucIter (Address start, image_func *func);

    InstrucIter (image_basicBlock *b);
    
    /* Iterate over a function... this goes linearly, which is probably a bad idea */
    InstrucIter(int_function* func);
    // Sometimes we don't care about basic blocks...
    InstrucIter (Address start, int_function *func);


    InstrucIter(bblInstance *b);
        
    InstrucIter( CONST_EXPORT BPatch_basicBlock* bpBasicBlock);

    /** copy constructor
     * @param ii InstrucIter to copy
     */
    InstrucIter(const InstrucIter& ii);
    
    // Generic "address, process" iterator.
    InstrucIter( Address addr, unsigned size, process *proc);
    InstrucIter( Address addr, process *proc);
    

  ~InstrucIter() {  }

  /** return true iff the address is in the space represented by
   * the InstrucIter
   */
  bool containsAddress(Address addr) { 
      return ((addr >= base) && 
              (addr < (base + range))); 
  }

  /** method that returns the targets of a multi-branch instruction
   * it assumes the currentAddress of the handle instance points to the
   * multi branch instruction
   */

#if defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */ \
 || defined(i386_unknown_solaris2_5) \
 || defined(i386_unknown_nt4_0)
  bool getMultipleJumpTargets( pdvector<Address>& result, 
                               instruction& tableInsn, 
                               instruction& maxSwitchInsn, 
                               bool isAddressInJmp );
#else
  
  bool getMultipleJumpTargets( BPatch_Set< Address >& result );

#endif


  /** method that returns true if there are more instructions to iterate */
  bool hasMore();

  /** method that returns true if there are instruction previous to the
   * current one */
  bool hasPrev();

  /** Peek at the previous address */
  Address peekPrev();

  /** And the next address */
  Address peekNext();

  /** set the content of the handle to the argument 
   * @param addr the value that handle will be set to
   */
  void setCurrentAddress(Address);

#if defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */ \
 || defined(i386_unknown_nt4_0)
  /** does the address point to an instruction */
  bool isInstruction();
#endif

  /** returns the content of the address  handle */
  Address operator* ();

  /** prefix increment operation */
  Address operator++ ();

  /** prefix decrement operation */
  Address operator-- ();

  /** postfix increment operation */
  Address operator++ (int);

  /** postfix decrement operation */
  Address operator-- (int);

  /* Predicates */
   
  bool isALeaveInstruction();
  bool isIndir();
  bool isAReturnInstruction();
  bool isACondBranchInstruction();
  bool isAJumpInstruction();
  bool isACallInstruction();
  bool isADynamicCallInstruction();
  bool isAnneal();
  bool isStackFramePreamble(int &frameSize);
  bool isReturnValueSave();
  bool isFramePush();
  bool isFrameSetup();
  bool isANopInstruction();
  
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

  Address getBranchTargetOffset();
  Address getBranchTargetAddress();

  BPatch_memoryAccess* isLoadOrStore();
  BPatch_instruction* getBPInstruction();

  unsigned getRTValue();
  unsigned getRAValue();
  unsigned getRBValue();
  unsigned getFRTValue();
  unsigned getFRAValue();
  unsigned getFRBValue();
  unsigned getFRCValue();
  void printOpCode();

  bool isAIndirectJumpInstruction();

#if defined(arch_sparc)
  // Delay slot happiness. Thing is, a jump ends a basic block (especially
  // since you can branch into the delay slot, and we don't like overlapping
  // basic blocks). And I want to keep that. But we still need a way to say
  // "grab me the delay slot insn", which might _not_ be in a basic block
  // (say, unconditional jump). So I've added two instructions that go by 
  // address and grab delay slot (and aggregate doohickey)

  void getAndSkipDSandAgg(instruction* &ds,
                          instruction* &agg);
#endif

};

#endif /* _InstrucIter_h_ */
