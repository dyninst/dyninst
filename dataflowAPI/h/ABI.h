#ifndef ABI_H
#define ABI_H

#include "dyn_regs.h"
#include "bitArray.h"
#include <map>

using namespace Dyninst;
class ABI{

#if defined(cap_liveness)

    static ABI* globalABI_; 
    static ABI* globalABI64_;
    std::map<MachRegister,int> *index;
    int addr_width;

 public:
    const bitArray &getCallReadRegisters() const;
    const bitArray &getCallWrittenRegisters() const;
    const bitArray &getReturnReadRegisters() const;
    // No such thing as return written...

    // Syscall!
    const bitArray &getSyscallReadRegisters() const;
    const bitArray &getSyscallWrittenRegisters() const;

    const bitArray &getAllRegs() const;

    int getIndex(MachRegister machReg);
    std::map<MachRegister,int>* getIndexMap();

    static void initialize32();
    static void initialize64();

    static ABI* getABI(int addr_width);
    bitArray getBitArray();
 private:
    static bitArray callRead_;
    static bitArray callRead64_;

    static bitArray callWritten_;
    static bitArray callWritten64_;

    static bitArray returnRead_;
    static bitArray returnRead64_;

    static bitArray syscallRead_;
    static bitArray syscallRead64_;

    static bitArray syscallWritten_;
    static bitArray syscallWritten64_;

    static bitArray allRegs_;
    static bitArray allRegs64_;

    static bitArray getBitArray(int size){
    	return bitArray(size);
    }

    ABI() {}
#endif



};


#endif //ABI_H
