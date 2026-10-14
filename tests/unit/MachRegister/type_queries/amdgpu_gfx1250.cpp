#include "registers/MachRegister.h"
#include "registers/AMDGPU/amdgpu_gfx1250_regs.h"
#include "type_query_check.h"

constexpr auto arch = Dyninst::Arch_amdgpu_gfx1250;

static bool is_arithmetic_vector(Dyninst::MachRegister);

int main() {
  TYPE_QUERIES_CHECK(Dyninst::amdgpu_gfx1250::pc_all, isPC);

  TYPE_QUERIES_CHECK_INVALID(getFramePointer, Dyninst::Arch_amdgpu_gfx1250);
  TYPE_QUERIES_CHECK_INVALID(getStackPointer, Dyninst::Arch_amdgpu_gfx1250);
  TYPE_QUERIES_CHECK_INVALID(getSyscallNumberReg, Dyninst::Arch_amdgpu_gfx1250);
  TYPE_QUERIES_CHECK_INVALID(getSyscallReturnValueReg, Dyninst::Arch_amdgpu_gfx1250);
  TYPE_QUERIES_CHECK_INVALID(getSyscallNumberOReg, Dyninst::Arch_amdgpu_gfx1250);
  TYPE_QUERIES_CHECK_INVALID(getZeroFlag, Dyninst::Arch_amdgpu_gfx1250);

  /*********************************************************************
   *      General Purpose
   *********************************************************************/
  TYPE_QUERIES_CHECK(Dyninst::amdgpu_gfx1250::s0, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::amdgpu_gfx1250::s105, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::amdgpu_gfx1250::ttmp0, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::amdgpu_gfx1250::ttmp15, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::amdgpu_gfx1250::s0, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::amdgpu_gfx1250::s105, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::amdgpu_gfx1250::v0, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::amdgpu_gfx1250::v255, isGeneralPurpose);

  /*********************************************************************
   *      Control/Status
  *********************************************************************/
  TYPE_QUERIES_CHECK_FALSE(Dyninst::amdgpu_gfx1250::s0, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::amdgpu_gfx1250::s105, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::amdgpu_gfx1250::v0, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::amdgpu_gfx1250::v255, isControlStatus);
  TYPE_QUERIES_CHECK(Dyninst::amdgpu_gfx1250::vcc_lo, isControlStatus);
  TYPE_QUERIES_CHECK(Dyninst::amdgpu_gfx1250::vcc_hi, isControlStatus);
  TYPE_QUERIES_CHECK(Dyninst::amdgpu_gfx1250::exec_lo, isControlStatus);
  TYPE_QUERIES_CHECK(Dyninst::amdgpu_gfx1250::exec_hi, isControlStatus);
  TYPE_QUERIES_CHECK(Dyninst::amdgpu_gfx1250::src_scc, isControlStatus);
  TYPE_QUERIES_CHECK(Dyninst::amdgpu_gfx1250::src_vccz, isControlStatus);
  TYPE_QUERIES_CHECK(Dyninst::amdgpu_gfx1250::src_execz, isControlStatus);

  /*********************************************************************
   *      Floating-Point
  *********************************************************************/
  TYPE_QUERIES_CHECK(Dyninst::amdgpu_gfx1250::s0, isFloatingPoint);
  TYPE_QUERIES_CHECK(Dyninst::amdgpu_gfx1250::s105, isFloatingPoint);
  TYPE_QUERIES_CHECK(Dyninst::amdgpu_gfx1250::v0, isFloatingPoint);
  TYPE_QUERIES_CHECK(Dyninst::amdgpu_gfx1250::v255, isFloatingPoint);

  /*********************************************************************
   *      Vector
  *********************************************************************/
  TYPE_QUERIES_CHECK(Dyninst::amdgpu_gfx1250::v0, isVector);
  TYPE_QUERIES_CHECK(Dyninst::amdgpu_gfx1250::v255, isVector);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::amdgpu_gfx1250::ttmp0, isVector);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::amdgpu_gfx1250::ttmp15, isVector);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::amdgpu_gfx1250::null, isVector);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::amdgpu_gfx1250::s0, isVector);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::amdgpu_gfx1250::s105, isVector);

  /*********************************************************************
   *      Filtering
  *********************************************************************/
  {
    for(auto reg : Dyninst::MachRegister::getAllRegistersForArch(arch)) {
      if(reg.isVector() && !reg.isControlStatus()) {
        TYPE_QUERIES_ASSERT_TRUE(reg, is_arithmetic_vector(reg));
      }
      if(is_arithmetic_vector(reg)) {
        TYPE_QUERIES_ASSERT_TRUE(reg, reg.isVector() && !reg.isControlStatus());
      }
    }
  }
  return EXIT_SUCCESS;
}

static bool is_arithmetic_vector(Dyninst::MachRegister reg) {
  switch(reg) {

    case Dyninst::amdgpu_gfx1250::iv0:
    case Dyninst::amdgpu_gfx1250::iv1:
    case Dyninst::amdgpu_gfx1250::iv2:
    case Dyninst::amdgpu_gfx1250::iv3:
    case Dyninst::amdgpu_gfx1250::iv4:
    case Dyninst::amdgpu_gfx1250::iv5:
    case Dyninst::amdgpu_gfx1250::iv6:
    case Dyninst::amdgpu_gfx1250::iv7:
    case Dyninst::amdgpu_gfx1250::iv8:
    case Dyninst::amdgpu_gfx1250::iv9:
    case Dyninst::amdgpu_gfx1250::iv10:
    case Dyninst::amdgpu_gfx1250::iv11:
    case Dyninst::amdgpu_gfx1250::iv12:
    case Dyninst::amdgpu_gfx1250::iv13:
    case Dyninst::amdgpu_gfx1250::iv14:
    case Dyninst::amdgpu_gfx1250::iv15:
    case Dyninst::amdgpu_gfx1250::iv16:
    case Dyninst::amdgpu_gfx1250::iv17:
    case Dyninst::amdgpu_gfx1250::iv18:
    case Dyninst::amdgpu_gfx1250::iv19:
    case Dyninst::amdgpu_gfx1250::iv20:
    case Dyninst::amdgpu_gfx1250::iv21:
    case Dyninst::amdgpu_gfx1250::iv22:
    case Dyninst::amdgpu_gfx1250::iv23:
    case Dyninst::amdgpu_gfx1250::iv24:
    case Dyninst::amdgpu_gfx1250::iv25:
    case Dyninst::amdgpu_gfx1250::iv26:
    case Dyninst::amdgpu_gfx1250::iv27:
    case Dyninst::amdgpu_gfx1250::iv28:
    case Dyninst::amdgpu_gfx1250::iv29:
    case Dyninst::amdgpu_gfx1250::iv30:
    case Dyninst::amdgpu_gfx1250::iv31:
    case Dyninst::amdgpu_gfx1250::iv32:
    case Dyninst::amdgpu_gfx1250::iv33:
    case Dyninst::amdgpu_gfx1250::iv34:
    case Dyninst::amdgpu_gfx1250::iv35:
    case Dyninst::amdgpu_gfx1250::iv36:
    case Dyninst::amdgpu_gfx1250::iv37:
    case Dyninst::amdgpu_gfx1250::iv38:
    case Dyninst::amdgpu_gfx1250::iv39:
    case Dyninst::amdgpu_gfx1250::iv40:
    case Dyninst::amdgpu_gfx1250::iv41:
    case Dyninst::amdgpu_gfx1250::iv42:
    case Dyninst::amdgpu_gfx1250::iv43:
    case Dyninst::amdgpu_gfx1250::iv44:
    case Dyninst::amdgpu_gfx1250::iv45:
    case Dyninst::amdgpu_gfx1250::iv46:
    case Dyninst::amdgpu_gfx1250::iv47:
    case Dyninst::amdgpu_gfx1250::iv48:
    case Dyninst::amdgpu_gfx1250::iv49:
    case Dyninst::amdgpu_gfx1250::iv50:
    case Dyninst::amdgpu_gfx1250::iv51:
    case Dyninst::amdgpu_gfx1250::iv52:
    case Dyninst::amdgpu_gfx1250::iv53:
    case Dyninst::amdgpu_gfx1250::iv54:
    case Dyninst::amdgpu_gfx1250::iv55:
    case Dyninst::amdgpu_gfx1250::iv56:
    case Dyninst::amdgpu_gfx1250::iv57:
    case Dyninst::amdgpu_gfx1250::iv58:
    case Dyninst::amdgpu_gfx1250::iv59:
    case Dyninst::amdgpu_gfx1250::iv60:
    case Dyninst::amdgpu_gfx1250::iv61:
    case Dyninst::amdgpu_gfx1250::iv62:
    case Dyninst::amdgpu_gfx1250::iv63:
    case Dyninst::amdgpu_gfx1250::iv64:
    case Dyninst::amdgpu_gfx1250::iv65:
    case Dyninst::amdgpu_gfx1250::iv66:
    case Dyninst::amdgpu_gfx1250::iv67:
    case Dyninst::amdgpu_gfx1250::iv68:
    case Dyninst::amdgpu_gfx1250::iv69:
    case Dyninst::amdgpu_gfx1250::iv70:
    case Dyninst::amdgpu_gfx1250::iv71:
    case Dyninst::amdgpu_gfx1250::iv72:
    case Dyninst::amdgpu_gfx1250::iv73:
    case Dyninst::amdgpu_gfx1250::iv74:
    case Dyninst::amdgpu_gfx1250::iv75:
    case Dyninst::amdgpu_gfx1250::iv76:
    case Dyninst::amdgpu_gfx1250::iv77:
    case Dyninst::amdgpu_gfx1250::iv78:
    case Dyninst::amdgpu_gfx1250::iv79:
    case Dyninst::amdgpu_gfx1250::iv80:
    case Dyninst::amdgpu_gfx1250::iv81:
    case Dyninst::amdgpu_gfx1250::iv82:
    case Dyninst::amdgpu_gfx1250::iv83:
    case Dyninst::amdgpu_gfx1250::iv84:
    case Dyninst::amdgpu_gfx1250::iv85:
    case Dyninst::amdgpu_gfx1250::iv86:
    case Dyninst::amdgpu_gfx1250::iv87:
    case Dyninst::amdgpu_gfx1250::iv88:
    case Dyninst::amdgpu_gfx1250::iv89:
    case Dyninst::amdgpu_gfx1250::iv90:
    case Dyninst::amdgpu_gfx1250::iv91:
    case Dyninst::amdgpu_gfx1250::iv92:
    case Dyninst::amdgpu_gfx1250::iv93:
    case Dyninst::amdgpu_gfx1250::iv94:
    case Dyninst::amdgpu_gfx1250::iv95:
    case Dyninst::amdgpu_gfx1250::iv96:
    case Dyninst::amdgpu_gfx1250::iv97:
    case Dyninst::amdgpu_gfx1250::iv98:
    case Dyninst::amdgpu_gfx1250::iv99:
    case Dyninst::amdgpu_gfx1250::iv100:
    case Dyninst::amdgpu_gfx1250::iv101:
    case Dyninst::amdgpu_gfx1250::iv102:
    case Dyninst::amdgpu_gfx1250::iv103:
    case Dyninst::amdgpu_gfx1250::iv104:
    case Dyninst::amdgpu_gfx1250::iv105:
    case Dyninst::amdgpu_gfx1250::iv106:
    case Dyninst::amdgpu_gfx1250::iv107:
    case Dyninst::amdgpu_gfx1250::iv108:
    case Dyninst::amdgpu_gfx1250::iv109:
    case Dyninst::amdgpu_gfx1250::iv110:
    case Dyninst::amdgpu_gfx1250::iv111:
    case Dyninst::amdgpu_gfx1250::iv112:
    case Dyninst::amdgpu_gfx1250::iv113:
    case Dyninst::amdgpu_gfx1250::iv114:
    case Dyninst::amdgpu_gfx1250::iv115:
    case Dyninst::amdgpu_gfx1250::iv116:
    case Dyninst::amdgpu_gfx1250::iv117:
    case Dyninst::amdgpu_gfx1250::iv118:
    case Dyninst::amdgpu_gfx1250::iv119:
    case Dyninst::amdgpu_gfx1250::iv120:
    case Dyninst::amdgpu_gfx1250::iv121:
    case Dyninst::amdgpu_gfx1250::iv122:
    case Dyninst::amdgpu_gfx1250::iv123:
    case Dyninst::amdgpu_gfx1250::iv124:
    case Dyninst::amdgpu_gfx1250::iv125:
    case Dyninst::amdgpu_gfx1250::iv126:
    case Dyninst::amdgpu_gfx1250::iv127:
    case Dyninst::amdgpu_gfx1250::iv128:
    case Dyninst::amdgpu_gfx1250::iv129:
    case Dyninst::amdgpu_gfx1250::iv130:
    case Dyninst::amdgpu_gfx1250::iv131:
    case Dyninst::amdgpu_gfx1250::iv132:
    case Dyninst::amdgpu_gfx1250::iv133:
    case Dyninst::amdgpu_gfx1250::iv134:
    case Dyninst::amdgpu_gfx1250::iv135:
    case Dyninst::amdgpu_gfx1250::iv136:
    case Dyninst::amdgpu_gfx1250::iv137:
    case Dyninst::amdgpu_gfx1250::iv138:
    case Dyninst::amdgpu_gfx1250::iv139:
    case Dyninst::amdgpu_gfx1250::iv140:
    case Dyninst::amdgpu_gfx1250::iv141:
    case Dyninst::amdgpu_gfx1250::iv142:
    case Dyninst::amdgpu_gfx1250::iv143:
    case Dyninst::amdgpu_gfx1250::iv144:
    case Dyninst::amdgpu_gfx1250::iv145:
    case Dyninst::amdgpu_gfx1250::iv146:
    case Dyninst::amdgpu_gfx1250::iv147:
    case Dyninst::amdgpu_gfx1250::iv148:
    case Dyninst::amdgpu_gfx1250::iv149:
    case Dyninst::amdgpu_gfx1250::iv150:
    case Dyninst::amdgpu_gfx1250::iv151:
    case Dyninst::amdgpu_gfx1250::iv152:
    case Dyninst::amdgpu_gfx1250::iv153:
    case Dyninst::amdgpu_gfx1250::iv154:
    case Dyninst::amdgpu_gfx1250::iv155:
    case Dyninst::amdgpu_gfx1250::iv156:
    case Dyninst::amdgpu_gfx1250::iv157:
    case Dyninst::amdgpu_gfx1250::iv158:
    case Dyninst::amdgpu_gfx1250::iv159:
    case Dyninst::amdgpu_gfx1250::iv160:
    case Dyninst::amdgpu_gfx1250::iv161:
    case Dyninst::amdgpu_gfx1250::iv162:
    case Dyninst::amdgpu_gfx1250::iv163:
    case Dyninst::amdgpu_gfx1250::iv164:
    case Dyninst::amdgpu_gfx1250::iv165:
    case Dyninst::amdgpu_gfx1250::iv166:
    case Dyninst::amdgpu_gfx1250::iv167:
    case Dyninst::amdgpu_gfx1250::iv168:
    case Dyninst::amdgpu_gfx1250::iv169:
    case Dyninst::amdgpu_gfx1250::iv170:
    case Dyninst::amdgpu_gfx1250::iv171:
    case Dyninst::amdgpu_gfx1250::iv172:
    case Dyninst::amdgpu_gfx1250::iv173:
    case Dyninst::amdgpu_gfx1250::iv174:
    case Dyninst::amdgpu_gfx1250::iv175:
    case Dyninst::amdgpu_gfx1250::iv176:
    case Dyninst::amdgpu_gfx1250::iv177:
    case Dyninst::amdgpu_gfx1250::iv178:
    case Dyninst::amdgpu_gfx1250::iv179:
    case Dyninst::amdgpu_gfx1250::iv180:
    case Dyninst::amdgpu_gfx1250::iv181:
    case Dyninst::amdgpu_gfx1250::iv182:
    case Dyninst::amdgpu_gfx1250::iv183:
    case Dyninst::amdgpu_gfx1250::iv184:
    case Dyninst::amdgpu_gfx1250::iv185:
    case Dyninst::amdgpu_gfx1250::iv186:
    case Dyninst::amdgpu_gfx1250::iv187:
    case Dyninst::amdgpu_gfx1250::iv188:
    case Dyninst::amdgpu_gfx1250::iv189:
    case Dyninst::amdgpu_gfx1250::iv190:
    case Dyninst::amdgpu_gfx1250::iv191:
    case Dyninst::amdgpu_gfx1250::iv192:
    case Dyninst::amdgpu_gfx1250::iv193:
    case Dyninst::amdgpu_gfx1250::iv194:
    case Dyninst::amdgpu_gfx1250::iv195:
    case Dyninst::amdgpu_gfx1250::iv196:
    case Dyninst::amdgpu_gfx1250::iv197:
    case Dyninst::amdgpu_gfx1250::iv198:
    case Dyninst::amdgpu_gfx1250::iv199:
    case Dyninst::amdgpu_gfx1250::iv200:
    case Dyninst::amdgpu_gfx1250::iv201:
    case Dyninst::amdgpu_gfx1250::iv202:
    case Dyninst::amdgpu_gfx1250::iv203:
    case Dyninst::amdgpu_gfx1250::iv204:
    case Dyninst::amdgpu_gfx1250::iv205:
    case Dyninst::amdgpu_gfx1250::iv206:
    case Dyninst::amdgpu_gfx1250::iv207:
    case Dyninst::amdgpu_gfx1250::iv208:
    case Dyninst::amdgpu_gfx1250::iv209:
    case Dyninst::amdgpu_gfx1250::iv210:
    case Dyninst::amdgpu_gfx1250::iv211:
    case Dyninst::amdgpu_gfx1250::iv212:
    case Dyninst::amdgpu_gfx1250::iv213:
    case Dyninst::amdgpu_gfx1250::iv214:
    case Dyninst::amdgpu_gfx1250::iv215:
    case Dyninst::amdgpu_gfx1250::iv216:
    case Dyninst::amdgpu_gfx1250::iv217:
    case Dyninst::amdgpu_gfx1250::iv218:
    case Dyninst::amdgpu_gfx1250::iv219:
    case Dyninst::amdgpu_gfx1250::iv220:
    case Dyninst::amdgpu_gfx1250::iv221:
    case Dyninst::amdgpu_gfx1250::iv222:
    case Dyninst::amdgpu_gfx1250::iv223:
    case Dyninst::amdgpu_gfx1250::iv224:
    case Dyninst::amdgpu_gfx1250::iv225:
    case Dyninst::amdgpu_gfx1250::iv226:
    case Dyninst::amdgpu_gfx1250::iv227:
    case Dyninst::amdgpu_gfx1250::iv228:
    case Dyninst::amdgpu_gfx1250::iv229:
    case Dyninst::amdgpu_gfx1250::iv230:
    case Dyninst::amdgpu_gfx1250::iv231:
    case Dyninst::amdgpu_gfx1250::iv232:
    case Dyninst::amdgpu_gfx1250::iv233:
    case Dyninst::amdgpu_gfx1250::iv234:
    case Dyninst::amdgpu_gfx1250::iv235:
    case Dyninst::amdgpu_gfx1250::iv236:
    case Dyninst::amdgpu_gfx1250::iv237:
    case Dyninst::amdgpu_gfx1250::iv238:
    case Dyninst::amdgpu_gfx1250::iv239:
    case Dyninst::amdgpu_gfx1250::iv240:
    case Dyninst::amdgpu_gfx1250::iv241:
    case Dyninst::amdgpu_gfx1250::iv242:
    case Dyninst::amdgpu_gfx1250::iv243:
    case Dyninst::amdgpu_gfx1250::iv244:
    case Dyninst::amdgpu_gfx1250::iv245:
    case Dyninst::amdgpu_gfx1250::iv246:
    case Dyninst::amdgpu_gfx1250::iv247:
    case Dyninst::amdgpu_gfx1250::iv248:
    case Dyninst::amdgpu_gfx1250::iv249:
    case Dyninst::amdgpu_gfx1250::iv250:
    case Dyninst::amdgpu_gfx1250::iv251:
    case Dyninst::amdgpu_gfx1250::iv252:
    case Dyninst::amdgpu_gfx1250::iv253:
    case Dyninst::amdgpu_gfx1250::iv254:
    case Dyninst::amdgpu_gfx1250::iv255:
      return true;
  }
  return false;
}
