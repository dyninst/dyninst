#ifndef _AddressHandle_h_
#define _AddressHandle_h_

#include "BPatch_Set.h"
#include "BPatch_point.h"
#include "BPatch_memoryAccess_NP.h"

class AddressHandle;

/** helper function to identify the properties of the
  * instruction such as the type and the offset.
  *
  * VG (10/09/01): Also does matching on opcode sets.
  * After discussion with JKH and Tikir it seems the right
  * place(TM) to add predicates like this. Unfortunately right
  * now there are still functions that do this in inst-XXX.C...
  */
bool isAReturnInstruction(const instruction);
bool isACondBranchInstruction(const instruction);
bool isAJumpInstruction(const instruction);
bool isACallInstruction(const instruction);
bool isAnneal(const instruction);
Address getBranchTargetAddress(const instruction,Address pos);
BPatch_memoryAccess isLoadOrStore(const instruction);

#if defined(rs6000_ibm_aix4_1)
bool isAIndirectJumpInstruction(const instruction,AddressHandle);
#else 
bool isAIndirectJumpInstruction(const instruction);
#endif

/** class for manipulating the address space in an image for a given
  * range. Such as the valid addresses and the instructions in the addresses
  * and iteration on the address apce range for instructions
  */
class AddressHandle {
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

#if defined(i386_unknown_linux2_0) ||\
    defined(i386_unknown_solaris2_5) ||\
    defined(i386_unknown_nt4_0) ||\
    defined(ia64_unknown_linux2_4) /* Temporary duplication - TLM */
	const unsigned char** instructionPointers;
#endif

public:
	/** constructor
	  * @param process_ptr process wher address spce resides
	  * @param base_addr start address of the addresss space
	  * @param range range of the address space in bytes
	  */ 
	AddressHandle (process*,image*,Address,unsigned);

	/** copy constructor
	  * @param ah address handler that will be copied 
	  */
	AddressHandle (const AddressHandle&);

	/** destructor */
	~AddressHandle();

	/** returns the image associated with the AddressHandle
	 */
	image *getImage() { return addressImage; };

	/** return true iff the address is in the space represented by
	 * the AddressHandle
	 */
	bool containsAddress(Address addr) { 
		return ((addr >= baseAddress) && 
	                (addr < (baseAddress + range))); 
	}

	/** method that returns true if the delay instruction is supported */
	static bool delayInstructionSupported ();

	/** method that returns the targets of a multi-branch instruction
	  * it assumes the currentAddress of the handle instance points to the
	  * multi branch instruction
	  */
	void getMultipleJumpTargets(BPatch_Set<Address>& result
#if defined(i386_unknown_linux2_0) ||\
    defined(i386_unknown_solaris2_5) ||\
    defined(i386_unknown_nt4_0) ||\
    defined(ia64_unknown_linux2_4) /* Temporary duplication - TLM */
				    ,AddressHandle& mayUpdate
#endif
				    );

	/** method that returns true if ther is more instruction to iterate */
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

	/** returns the instruction in the address of handle */
	instruction getInstruction();

	/** returns the instruction in the next address of handle */
	instruction getNextInstruction();

	/** returns the instruction in the prev address of handle */
	instruction getPrevInstruction();

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
};

#endif /* _AddressHandle_h_ */
