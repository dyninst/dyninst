#ifndef _InstrucIter_h_
#define _InstrucIter_h_

#include "BPatch_Set.h"
#include "BPatch_point.h"
#include "BPatch_memoryAccess_NP.h"
#include "BPatch_function.h"

class InstrucIter;

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

  /** process of the function and image */
  process* addressProc;

  /** member which hold the image of that will be used to retrive
   * instruction values, where the address space resides.
   */
  image* addressImage;

  /** the starting point of the address spce */
  Address baseAddress;

  /** the range/length of the address space in bytes */
  unsigned range;

  /** current address of the address spce which will be used 
   * to iterate through the address space
   */
  Address currentAddress;
public:

  /** returns the instruction in the address of handle */
  instruction getInstruction();

  /** returns the instruction in the next address of handle */
  instruction getNextInstruction();

  /** returns the instruction in the prev address of handle */
  instruction getPrevInstruction();

  /** returns the image */
  image *getImage() { return addressImage; };

  instruction insn;
  const unsigned char* instPtr;

public:
  /** static method that returns true if the delay instruction is supported */
  static bool delayInstructionSupported ();

  /** constructor
   * @param bpFunction the BPatch_function to iterate over
   * @param useRelativeAddr whether we shall store absolute or relative address
   */

  // added useRelativeAddr to baseAddr line
  InstrucIter(function_base* func, process* proc, pdmodule *mod, 
	      bool useRelativeAddr = true) :
      addressProc(proc),
      addressImage(mod->exec()),
      baseAddress((Address) ( useRelativeAddr ? (void *)func->addr()
                              : (void *)func->getEffectiveAddress(proc) )),
      range(func->size()),
      currentAddress((Address) (useRelativeAddr ?
                                (void *)func->addr() :
                                (void *)func->getEffectiveAddress(proc)))
#if defined(i386_unknown_linux2_0) ||\
    defined(i386_unknown_solaris2_5) ||\
    defined(i386_unknown_nt4_0)
    {
        instPtr = mod->exec()->getPtrToInstruction( func->addr() );
        insn.getNextInstruction( instPtr );
#else
    {
#endif
    }
        
   InstrucIter(const BPatch_basicBlock* bpBasicBlock) :
       addressProc(bpBasicBlock->flowGraph->getProcess()),
       addressImage(bpBasicBlock->flowGraph->getModule()->exec()),
       baseAddress(bpBasicBlock->startAddress),
       range(bpBasicBlock->endAddress - bpBasicBlock->startAddress + sizeof(instruction)),
       currentAddress(bpBasicBlock->startAddress)
#if defined(i386_unknown_linux2_0) ||\
    defined(i386_unknown_solaris2_5) ||\
    defined(i386_unknown_nt4_0)
    {
        instPtr =addressImage->getPtrToInstruction(bpBasicBlock->startAddress);
        insn.getNextInstruction( instPtr ); 
#else
    {
#endif
    }


  /** copy constructor
   * @param ii InstrucIter to copy
   */
    InstrucIter(const InstrucIter& ii) :
        addressProc(ii.addressProc),
        addressImage(ii.addressImage),
        baseAddress(ii.baseAddress),
        range(ii.range),
        currentAddress(ii.currentAddress)
#if defined(i386_unknown_linux2_0) ||\
     defined(i386_unknown_solaris2_5) ||\
     defined(i386_unknown_nt4_0)    
    {
        instPtr = ii.addressImage->getPtrToInstruction( ii.baseAddress );
        insn.getNextInstruction( instPtr );
#else
    {
#endif
    }

    //used by findInstPoints
    InstrucIter( Address addr, Address base,  image* img, 
                 bool useRelativeAddr = true ) :
        currentAddress( addr )
#if defined(i386_unknown_linux2_0) ||\
    defined(i386_unknown_solaris2_5) ||\
    defined(i386_unknown_nt4_0)
    {
        addressImage = img;
        baseAddress = base;
        instPtr = img->getPtrToInstruction( addr );
        insn.getNextInstruction( instPtr );        
#else
    {
#endif   
    }

  ~InstrucIter() {  }

  /** return true iff the address is in the space represented by
   * the InstrucIter
   */
  bool containsAddress(Address addr) { 
    return ((addr >= baseAddress) && 
            (addr < (baseAddress + range))); 
  }

  /** method that returns the targets of a multi-branch instruction
   * it assumes the currentAddress of the handle instance points to the
   * multi branch instruction
   */
  void getMultipleJumpTargets(BPatch_Set<Address>& result
#if defined(i386_unknown_linux2_0) ||\
    defined(i386_unknown_solaris2_5) ||\
    defined(i386_unknown_nt4_0)
                              ,InstrucIter& mayUpdate
#endif
      );

void getMultipleJumpTargets( pdvector<Address>& result
#if defined(i386_unknown_linux2_0) ||\
       defined(i386_unknown_solaris2_5) ||\
       defined(i386_unknown_nt4_0)					 
			     ,instruction& tableInsn, 
			     instruction& maxSwitchInsn, 
			     bool isAddressInJmp
#endif
    );

  /** method that returns true if there are more instructions to iterate */
  bool hasMore();

  /** method that returns true if there are instruction previous to the
   * current one */
  bool hasPrev();

  /** prev address of the content of the address handle */
  Address prevAddress();

  /** next address of the content of the address handle */
  Address nextAddress();

  /** set the content of the handle to the argument 
   * @param addr the value that handle will be set to
   */
  void setCurrentAddress(Address);

#if defined(i386_unknown_linux2_0) || defined(i386_unknown_nt4_0)
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
  bool isAnneal();
  bool isStackFramePreamble();
  bool isANopInstruction();
  Address getBranchTargetAddress( Address pos );
  Address getBranchTarget();

  BPatch_memoryAccess* isLoadOrStore();
  BPatch_instruction* getBPInstruction();

#if defined(rs6000_ibm_aix4_1)
  bool isAIndirectJumpInstruction(InstrucIter);
#else 
  bool isAIndirectJumpInstruction();
#endif

};

#endif /* _InstrucIter_h_ */
