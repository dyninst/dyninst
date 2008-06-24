#if !defined(REGISTER_IDS_X86_H)
#define REGISTER_IDS_X86_H

#include <vector>
#include <map>
#include <boost/assign/list_of.hpp>
#include "../../common/h/Types.h"
#include "Result.h"

namespace Dyninst
{
  namespace Instruction
  {
    /// \enum Dyninst::Instruction::IA32Regs
    /// \brief Registers for IA32 and AMD64 processors.
    ///
    enum IA32Regs { r_AH=100, r_BH, r_CH, r_DH, r_AL, r_BL, r_CL, r_DL, //107
		      r_AX, r_DX, //109
		      r_eAX, r_eBX, r_eCX, r_eDX, //113
		      r_EAX, r_EBX, r_ECX, r_EDX, //117
		      r_CS, r_DS, r_ES, r_FS, r_GS, r_SS, //123
		      r_eSP, r_eBP, r_eSI, r_eDI, //127
		      r_ESP, r_EBP, r_ESI, r_EDI, //131
		      r_EDXEAX, r_ECXEBX, //133
		      r_OF, r_SF, r_ZF, r_AF, r_PF, r_CF, r_TF, r_IF, r_DF, r_NT, r_RF,
		      // flags need to be separate registers for proper liveness analysis
		      r_DummyFPR, r_Reserved,
		      // and we have a dummy register to make liveness consistent since floating point saves are all/none at present
		    r_R8, r_R9, r_R10, r_R11, r_R12, r_R13, r_R14, r_R15,
		      // AMD64 GPRs
		    r_EIP, r_RIP,
		    r_RSP, r_RBP
		    // Instruction pointers
    }; 
    struct RegInfo
    {
      RegInfo(Result_Type t, std::string n) :
	regSize(t), regName(n) 
      {
      }
      RegInfo() :
	regSize(u8), regName("*** UNDEFINED REGISTER ***")
      {
      }
      
      Result_Type regSize;
      std::string regName;
    };
    
    

    using std::vector;
    using namespace boost::assign;
    using std::map;
    
    typedef std::map<IA32Regs, RegInfo> RegTable;

    /// \brief Register names for disassembly and debugging
    static RegTable IA32_register_names =
    map_list_of(r_AH, RegInfo(u8,"AH"))(r_BH, RegInfo(u8, "BH"))(r_CH, RegInfo(u8,"CH"))(r_DH, RegInfo(u8,"DH"))
    (r_AL, RegInfo(u8,"AL"))(r_BL, RegInfo(u8,"BL"))
    (r_CL, RegInfo(u8,"CL"))(r_DL, RegInfo(u8,"DL"))(r_AX, RegInfo(u16,"AX"))(r_DX, RegInfo(u16,"DX"))(r_eAX, RegInfo(u32,"eAX"))
    (r_eBX, RegInfo(u32,"eBX"))(r_eCX, RegInfo(u32,"eCX"))
    (r_eDX, RegInfo(u32,"eDX"))(r_EAX, RegInfo(u32,"EAX"))(r_EBX, RegInfo(u32,"EBX"))(r_ECX, RegInfo(u32,"ECX"))
    (r_EDX, RegInfo(u32,"EDX"))(r_CS, RegInfo(u32,"CS"))(r_DS, RegInfo(u32,"DS"))
    (r_ES, RegInfo(u32,"ES"))(r_FS, RegInfo(u32,"FS"))(r_GS, RegInfo(u32,"GS"))(r_SS, RegInfo(u32,"SS"))
    (r_eSP, RegInfo(u32,"eSP"))(r_eBP, RegInfo(u32,"eBP"))(r_eSI, RegInfo(u32,"eSI"))(r_eDI, RegInfo(u32,"eDI"))
    (r_ESP, RegInfo(u32,"ESP"))(r_EBP, RegInfo(u32,"EBP"))(r_ESI, RegInfo(u32,"ESI"))(r_EDI, RegInfo(u32,"EDI"))
    (r_EIP, RegInfo(u32,"EIP"))(r_CF, RegInfo(bit_flag,"Carry"))(r_ZF,  RegInfo(bit_flag,"Zero"))
    (r_PF,  RegInfo(bit_flag,"Parity"))(r_OF,  RegInfo(bit_flag,"Overflow"))(r_SF,  RegInfo(bit_flag,"Sign"))
    (r_RSP, RegInfo(u64, "RSP"))(r_RBP, RegInfo(u64, "RBP"));
    

  };
};



#endif //!defined(REGISTER_IDS_X86_H)
