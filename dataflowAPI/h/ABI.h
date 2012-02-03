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
    DATAFLOW_EXPORT const bitArray &getCallReadRegisters() const;
    DATAFLOW_EXPORT const bitArray &getCallWrittenRegisters() const;
    DATAFLOW_EXPORT const bitArray &getReturnReadRegisters() const;
    // No such thing as return written...

    // Syscall!
    DATAFLOW_EXPORT const bitArray &getSyscallReadRegisters() const;
    DATAFLOW_EXPORT const bitArray &getSyscallWrittenRegisters() const;

    DATAFLOW_EXPORT const bitArray &getAllRegs() const;

    DATAFLOW_EXPORT int getIndex(MachRegister machReg);
    DATAFLOW_EXPORT std::map<MachRegister,int>* getIndexMap();

    DATAFLOW_EXPORT static void initialize32();
    DATAFLOW_EXPORT static void initialize64();

    DATAFLOW_EXPORT static ABI* getABI(int addr_width);
    DATAFLOW_EXPORT bitArray getBitArray();
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
