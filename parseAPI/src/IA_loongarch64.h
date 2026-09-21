#if !defined(IA_LOONGARCH64__H)
#define IA_LOONGARCH64__H
#include "DynAST.h"
#include "dataflowAPI/h/Absloc.h"
#include "dataflowAPI/h/SymEval.h"
#include "dataflowAPI/h/slicing.h"
#include "IA_IAPI.h"

namespace Dyninst {
namespace InsnAdapter {

class IA_loongarch64 : public IA_IAPI {
    public:
        IA_loongarch64(Dyninst::InstructionAPI::InstructionDecoder dec_,
               Address start_,
               Dyninst::ParseAPI::CodeObject* o,
               Dyninst::ParseAPI::CodeRegion* r,
               Dyninst::InstructionSource *isrc,
               Dyninst::ParseAPI::Block * curBlk_);
    IA_loongarch64(const IA_loongarch64 &);
    virtual IA_loongarch64* clone() const;
    virtual bool isFrameSetupInsn(Dyninst::InstructionAPI::Instruction) const;
    virtual bool isNop() const;
    virtual bool isThunk() const;
    virtual bool isTailCall(const ParseAPI::Function* context, ParseAPI::EdgeTypeEnum type,
                            unsigned int, const set<Address>& knownTargets) const;
    virtual bool savesFP() const;
    virtual bool isStackFramePreamble() const;
    virtual bool cleansStack() const;
    virtual bool sliceReturn(ParseAPI::Block* bit, Address ret_addr, ParseAPI::Function * func) const;
    virtual bool isReturnAddrSave(Address& retAddr) const;
    virtual bool isReturn(Dyninst::ParseAPI::Function * context, Dyninst::ParseAPI::Block* currBlk) const;
    virtual bool isFakeCall() const;
    virtual bool isIATcall(std::string &) const;
    virtual bool isLinkerStub() const;
    virtual bool isNopJump() const;
    private:
    using IA_IAPI::isFrameSetupInsn;
};

} // namespace InsnAdapter
} // namespace Dyninst

#endif // IA_LOONGARCH64__H