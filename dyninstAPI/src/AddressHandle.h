#ifndef _AddressHandle_h_
#define _AddressHandle_h_

#if defined(sparc_sun_solaris2_4) ||\
    defined(mips_sgi_irix6_4) ||\
    defined(rs6000_ibm_aix4_1)

#include "BPatch_Set.h"

class AddressHandle;

/** helper function to identify the properties of the
  * instruction such as the type and the offset.
  */
bool isReturn(const instruction);
bool isLocalCondBranch(const instruction);
bool isLocalJump(const instruction);
bool isLocalCall(const instruction);
Address getBranchTargetAddress(const instruction,Address pos);

#if defined(rs6000_ibm_aix4_1)
bool isLocalIndirectJump(const instruction,AddressHandle);
#else 
bool isLocalIndirectJump(const instruction);
#endif

/** class for manipulating the address space in an image for a given
  * range. Such as the valid addresses and the instructions in the addresses
  * and iteration on the address apce range for instructions
  */
class AddressHandle {
protected:
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
	/** constructor
	  * @param image_ptr image wher address spce resides
	  * @param base_addr start address of the addresss space
	  * @param range range of the address space in bytes
	  */ 
	AddressHandle (image*,Address,unsigned);

	/** constructor
	  * @param current current address of the iteration
	  * @param image_ptr image wher address spce resides
	  * @param base_addr start address of the addresss space
	  * @param range range of the address space in bytes
	  */ 
	AddressHandle (Address,image*,Address,unsigned);

	/** copy constructor
	  * @param ah address handler that will be copied 
	  */
	AddressHandle (const AddressHandle&);

	/** returns the image associated with the AddressHandle
	 */
	image *getImage() { return addressImage; };

	/** return true iff the address is in the space represented by
	 * the AddressHandle
	 */
	bool containsAddress(Address addr)
		{ return addr >= baseAddress && addr < baseAddress + range; };

	/** method that returns true if the delay instruction is supported */
	static bool delayInstructionSupported ();

	/** method that returns the targets of a multi-branch instruction
	  * it assumes the currentAddress of the handle instance points to the
	  * multi branch instruction
	  */
	void getMultipleJumpTargets(BPatch_Set<Address>& result);

	/** method that returns true if ther is more instruction to iterate */
	bool hasMore();

	/** method that returns true if there are instruction previous to the
	 * current one */
	bool hasPrev();

	/** prev address of the content of the address handle */
	Address prevAddress();

	/** prev address of the argument 
	  * @param addr address whose prev is looked for
	  */
	Address prevAddressOf(Address);

	/** next address of the content of the address handle */
	Address nextAddress();

	/** next address of the  argument 
	  * @param addr address whose next is looked for
	  */
	Address nextAddressOf(Address);

	/** set the content of the handle to the argument 
	  * @param addr the value that handle will be set to
	  */
	void setCurrentAddress(Address);

	/** returns the instruction count in the address space */
	unsigned getInstructionCount();

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

#endif
#endif /* _AddressHandle_h_ */
