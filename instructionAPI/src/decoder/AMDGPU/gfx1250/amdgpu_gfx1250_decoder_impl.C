#include "registers/AMDGPU/amdgpu_gfx1250_regs.h"
#include "InstructionDecoder-amdgpu-gfx1250.h"

namespace Dyninst {
namespace InstructionAPI {
    bool InstructionDecoder_amdgpu_gfx1250::IS_ENC_SOP1(uint64_t I)
    {
        switch ( I & 0xff80ff00 )  {
            case 0xbe800000:  case 0xbe800100:  case 0xbe800200:  case 0xbe800300:
            case 0xbe800400:  case 0xbe800500:  case 0xbe800600:  case 0xbe800800:
            case 0xbe800900:  case 0xbe800a00:  case 0xbe800b00:  case 0xbe800c00:
            case 0xbe800d00:  case 0xbe800e00:  case 0xbe800f00:  case 0xbe801000:
            case 0xbe801100:  case 0xbe801200:  case 0xbe801300:  case 0xbe801400:
            case 0xbe801500:  case 0xbe801600:  case 0xbe801700:  case 0xbe801800:
            case 0xbe801900:  case 0xbe801a00:  case 0xbe801b00:  case 0xbe801c00:
            case 0xbe801d00:  case 0xbe801e00:  case 0xbe801f00:  case 0xbe802000:
            case 0xbe802100:  case 0xbe802200:  case 0xbe802300:  case 0xbe802400:
            case 0xbe802500:  case 0xbe802600:  case 0xbe802700:  case 0xbe802800:
            case 0xbe802900:  case 0xbe802a00:  case 0xbe802b00:  case 0xbe802c00:
            case 0xbe802d00:  case 0xbe802e00:  case 0xbe802f00:  case 0xbe803000:
            case 0xbe803100:  case 0xbe803200:  case 0xbe803300:  case 0xbe803400:
            case 0xbe803500:  case 0xbe803600:  case 0xbe803700:  case 0xbe804000:
            case 0xbe804100:  case 0xbe804200:  case 0xbe804300:  case 0xbe804400:
            case 0xbe804700:  case 0xbe804800:  case 0xbe804900:  case 0xbe804a00:
            case 0xbe804b00:  case 0xbe804c00:  case 0xbe804d00:  case 0xbe804e00:
            case 0xbe804f00:  case 0xbe805000:  case 0xbe805100:  case 0xbe805200:
            case 0xbe805300:  case 0xbe805700:  case 0xbe805800:  case 0xbe806000:
            case 0xbe806100:  case 0xbe806200:  case 0xbe806300:  case 0xbe806400:
            case 0xbe806500:  case 0xbe806600:  case 0xbe806700:  case 0xbe806800:
            case 0xbe806900:  case 0xbe806a00:  case 0xbe806b00:  case 0xbe806c00:
            case 0xbe806d00:  case 0xbe806e00:
                return true;

            default:
                return false;
        }
    }

    bool InstructionDecoder_amdgpu_gfx1250::IS_ENC_SOPC(uint64_t I)
    {
        switch ( I & 0xffff0000 )  {
            case 0xbf000000:  case 0xbf010000:  case 0xbf020000:  case 0xbf030000:
            case 0xbf040000:  case 0xbf050000:  case 0xbf060000:  case 0xbf070000:
            case 0xbf080000:  case 0xbf090000:  case 0xbf0a0000:  case 0xbf0b0000:
            case 0xbf0c0000:  case 0xbf0d0000:  case 0xbf0e0000:  case 0xbf0f0000:
            case 0xbf100000:  case 0xbf110000:  case 0xbf410000:  case 0xbf510000:
            case 0xbf420000:  case 0xbf520000:  case 0xbf430000:  case 0xbf530000:
            case 0xbf440000:  case 0xbf540000:  case 0xbf450000:  case 0xbf550000:
            case 0xbf460000:  case 0xbf560000:  case 0xbf470000:  case 0xbf570000:
            case 0xbf480000:  case 0xbf580000:  case 0xbf490000:  case 0xbf590000:
            case 0xbf4a0000:  case 0xbf5a0000:  case 0xbf4b0000:  case 0xbf5b0000:
            case 0xbf4c0000:  case 0xbf5c0000:  case 0xbf4d0000:  case 0xbf5d0000:
            case 0xbf4e0000:  case 0xbf5e0000:
                return true;

            default:
                return false;
        }
    }

    bool InstructionDecoder_amdgpu_gfx1250::IS_ENC_SOPP(uint64_t I)
    {
        switch ( I & 0xffff0000 )  {
            case 0xbf800000:  case 0xbf820000:  case 0xbf830000:  case 0xbf840000:
            case 0xbf850000:  case 0xbf860000:  case 0xbf870000:  case 0xbf880000:
            case 0xbf8a0000:  case 0xbf900000:  case 0xbf910000:  case 0xbf920000:
            case 0xbf940000:  case 0xbf950000:  case 0xbf9f0000:  case 0xbfa00000:
            case 0xbfa10000:  case 0xbfa20000:  case 0xbfa30000:  case 0xbfa40000:
            case 0xbfa50000:  case 0xbfa60000:  case 0xbfb00000:  case 0xbfb10000:
            case 0xbfb40000:  case 0xbfb50000:  case 0xbfb60000:  case 0xbfb70000:
            case 0xbfb80000:  case 0xbfb90000:  case 0xbfba0000:  case 0xbfbb0000:
            case 0xbfbc0000:  case 0xbfbe0000:  case 0xbfc00000:  case 0xbfc10000:
            case 0xbfc50000:  case 0xbfc60000:  case 0xbfc70000:  case 0xbfc80000:
            case 0xbfc90000:  case 0xbfca0000:  case 0xbfcb0000:
                return true;

            default:
                return false;
        }
    }

    bool InstructionDecoder_amdgpu_gfx1250::IS_ENC_SOPK(uint64_t I)
    {
        switch ( I & 0xff800000 )  {
            case 0xb0000000:  case 0xb0800000:  case 0xb1000000:  case 0xb7800000:
            case 0xb8000000:  case 0xb8800000:  case 0xb9000000:  case 0xba000000:
                return true;

            default:
                return false;
        }
    }

    bool InstructionDecoder_amdgpu_gfx1250::IS_ENC_SOP2(uint64_t I)
    {
        switch ( I & 0xff800000 )  {
            case 0x80000000:  case 0x80800000:  case 0x81000000:  case 0x81800000:
            case 0x82000000:  case 0x82800000:  case 0x83000000:  case 0x84000000:
            case 0x84800000:  case 0x85000000:  case 0x85800000:  case 0x86000000:
            case 0x86800000:  case 0x87000000:  case 0x87800000:  case 0x88000000:
            case 0x88800000:  case 0x89000000:  case 0x89800000:  case 0x8a000000:
            case 0x8a800000:  case 0x8b000000:  case 0x8b800000:  case 0x8c000000:
            case 0x8c800000:  case 0x8d000000:  case 0x8d800000:  case 0x8e000000:
            case 0x8e800000:  case 0x8f000000:  case 0x8f800000:  case 0x90000000:
            case 0x90800000:  case 0x91000000:  case 0x91800000:  case 0x92000000:
            case 0x92800000:  case 0x93000000:  case 0x93800000:  case 0x94000000:
            case 0x94800000:  case 0x95000000:  case 0x95800000:  case 0x96000000:
            case 0x96800000:  case 0x97000000:  case 0x98000000:  case 0x98800000:
            case 0x99000000:  case 0x99800000:  case 0x9a000000:  case 0x9a800000:
            case 0xa0000000:  case 0xa0800000:  case 0xa1000000:  case 0xa1800000:
            case 0xa2000000:  case 0xa3800000:  case 0xa4000000:  case 0xa4800000:
            case 0xa5000000:  case 0xa5800000:  case 0xa6000000:  case 0xa6800000:
            case 0xa7000000:  case 0xa7800000:  case 0xa8000000:  case 0xa8800000:
            case 0xa9000000:  case 0xa9800000:  case 0xaa000000:  case 0xaa800000:
                return true;

            default:
                return false;
        }
    }

    bool InstructionDecoder_amdgpu_gfx1250::IS_ENC_SMEM(uint64_t I)
    {
        switch ( I & 0xfc07e000 )  {
            case 0xf4000000:  case 0xf4002000:  case 0xf4004000:  case 0xf4006000:
            case 0xf4008000:  case 0xf400a000:  case 0xf4010000:  case 0xf4012000:
            case 0xf4014000:  case 0xf4016000:  case 0xf4020000:  case 0xf4022000:
            case 0xf4024000:  case 0xf4026000:  case 0xf4028000:  case 0xf402a000:
            case 0xf4030000:  case 0xf4032000:  case 0xf4034000:  case 0xf4036000:
            case 0xf4042000:  case 0xf4044000:  case 0xf4046000:  case 0xf4048000:
            case 0xf404a000:  case 0xf404c000:  case 0xf404e000:  case 0xf4050000:
                return true;

            default:
                return false;
        }
    }

    bool InstructionDecoder_amdgpu_gfx1250::IS_ENC_VOP1(uint64_t I)
    {
        switch ( I & 0xfe00fe00 )  {
            case 0x7e000000:  case 0x7e000200:  case 0x7e000400:  case 0x7e000600:
            case 0x7e000800:  case 0x7e000a00:  case 0x7e000c00:  case 0x7e000e00:
            case 0x7e001000:  case 0x7e001400:  case 0x7e001600:  case 0x7e001800:
            case 0x7e001a00:  case 0x7e001c00:  case 0x7e001e00:  case 0x7e002000:
            case 0x7e002200:  case 0x7e002400:  case 0x7e002600:  case 0x7e002800:
            case 0x7e002a00:  case 0x7e002c00:  case 0x7e002e00:  case 0x7e003000:
            case 0x7e003200:  case 0x7e003400:  case 0x7e003600:  case 0x7e003800:
            case 0x7e003a00:  case 0x7e003c00:  case 0x7e003e00:  case 0x7e004000:
            case 0x7e004200:  case 0x7e004400:  case 0x7e004600:  case 0x7e004800:
            case 0x7e004a00:  case 0x7e004e00:  case 0x7e005400:  case 0x7e005600:
            case 0x7e005c00:  case 0x7e005e00:  case 0x7e006200:  case 0x7e006600:
            case 0x7e006800:  case 0x7e006a00:  case 0x7e006c00:  case 0x7e006e00:
            case 0x7e007000:  case 0x7e007200:  case 0x7e007400:  case 0x7e007600:
            case 0x7e007800:  case 0x7e007a00:  case 0x7e007c00:  case 0x7e007e00:
            case 0x7e008000:  case 0x7e008400:  case 0x7e008600:  case 0x7e008800:
            case 0x7e009000:  case 0x7e009200:  case 0x7e009400:  case 0x7e009600:
            case 0x7e00a000:  case 0x7e00a200:  case 0x7e00a400:  case 0x7e00a600:
            case 0x7e00a800:  case 0x7e00aa00:  case 0x7e00ac00:  case 0x7e00ae00:
            case 0x7e00b000:  case 0x7e00b200:  case 0x7e00b400:  case 0x7e00b600:
            case 0x7e00b800:  case 0x7e00ba00:  case 0x7e00bc00:  case 0x7e00be00:
            case 0x7e00c000:  case 0x7e00c200:  case 0x7e00c400:  case 0x7e00c600:
            case 0x7e00c800:  case 0x7e00ca00:  case 0x7e00cc00:  case 0x7e00d000:
            case 0x7e00d200:  case 0x7e00d400:  case 0x7e00d600:  case 0x7e00d800:
            case 0x7e00da00:  case 0x7e00dc00:  case 0x7e00de00:  case 0x7e00e400:
            case 0x7e00e600:  case 0x7e00e800:  case 0x7e00ea00:  case 0x7e00ec00:
            case 0x7e00ee00:  case 0x7e00f000:  case 0x7e00f200:  case 0x7e00f400:
            case 0x7e00f600:  case 0x7e00f800:  case 0x7e00fa00:  case 0x7e00fc00:
            case 0x7e00fe00:
                return true;

            default:
                return false;
        }
    }

    bool InstructionDecoder_amdgpu_gfx1250::IS_ENC_VOPC(uint64_t I)
    {
        switch ( I & 0xfffe0000 )  {
            case 0x7c020000:  case 0x7c040000:  case 0x7c060000:  case 0x7c080000:
            case 0x7c0a0000:  case 0x7c0c0000:  case 0x7c0e0000:  case 0x7c100000:
            case 0x7c120000:  case 0x7c140000:  case 0x7c160000:  case 0x7c180000:
            case 0x7c1a0000:  case 0x7c1c0000:  case 0x7c220000:  case 0x7c240000:
            case 0x7c260000:  case 0x7c280000:  case 0x7c2a0000:  case 0x7c2c0000:
            case 0x7c2e0000:  case 0x7c300000:  case 0x7c320000:  case 0x7c340000:
            case 0x7c360000:  case 0x7c380000:  case 0x7c3a0000:  case 0x7c3c0000:
            case 0x7c420000:  case 0x7c440000:  case 0x7c460000:  case 0x7c480000:
            case 0x7c4a0000:  case 0x7c4c0000:  case 0x7c4e0000:  case 0x7c500000:
            case 0x7c520000:  case 0x7c540000:  case 0x7c560000:  case 0x7c580000:
            case 0x7c5a0000:  case 0x7c5c0000:  case 0x7c620000:  case 0x7c640000:
            case 0x7c660000:  case 0x7c680000:  case 0x7c6a0000:  case 0x7c6c0000:
            case 0x7c720000:  case 0x7c740000:  case 0x7c760000:  case 0x7c780000:
            case 0x7c7a0000:  case 0x7c7c0000:  case 0x7c820000:  case 0x7c840000:
            case 0x7c860000:  case 0x7c880000:  case 0x7c8a0000:  case 0x7c8c0000:
            case 0x7c920000:  case 0x7c940000:  case 0x7c960000:  case 0x7c980000:
            case 0x7c9a0000:  case 0x7c9c0000:  case 0x7ca20000:  case 0x7ca40000:
            case 0x7ca60000:  case 0x7ca80000:  case 0x7caa0000:  case 0x7cac0000:
            case 0x7cb20000:  case 0x7cb40000:  case 0x7cb60000:  case 0x7cb80000:
            case 0x7cba0000:  case 0x7cbc0000:  case 0x7cfa0000:  case 0x7cfc0000:
            case 0x7cfe0000:  case 0x7d020000:  case 0x7d040000:  case 0x7d060000:
            case 0x7d080000:  case 0x7d0a0000:  case 0x7d0c0000:  case 0x7d0e0000:
            case 0x7d100000:  case 0x7d120000:  case 0x7d140000:  case 0x7d160000:
            case 0x7d180000:  case 0x7d1a0000:  case 0x7d1c0000:  case 0x7d220000:
            case 0x7d240000:  case 0x7d260000:  case 0x7d280000:  case 0x7d2a0000:
            case 0x7d2c0000:  case 0x7d2e0000:  case 0x7d300000:  case 0x7d320000:
            case 0x7d340000:  case 0x7d360000:  case 0x7d380000:  case 0x7d3a0000:
            case 0x7d3c0000:  case 0x7d420000:  case 0x7d440000:  case 0x7d460000:
            case 0x7d480000:  case 0x7d4a0000:  case 0x7d4c0000:  case 0x7d4e0000:
            case 0x7d500000:  case 0x7d520000:  case 0x7d540000:  case 0x7d560000:
            case 0x7d580000:  case 0x7d5a0000:  case 0x7d5c0000:  case 0x7d620000:
            case 0x7d640000:  case 0x7d660000:  case 0x7d680000:  case 0x7d6a0000:
            case 0x7d6c0000:  case 0x7d720000:  case 0x7d740000:  case 0x7d760000:
            case 0x7d780000:  case 0x7d7a0000:  case 0x7d7c0000:  case 0x7d820000:
            case 0x7d840000:  case 0x7d860000:  case 0x7d880000:  case 0x7d8a0000:
            case 0x7d8c0000:  case 0x7d920000:  case 0x7d940000:  case 0x7d960000:
            case 0x7d980000:  case 0x7d9a0000:  case 0x7d9c0000:  case 0x7da20000:
            case 0x7da40000:  case 0x7da60000:  case 0x7da80000:  case 0x7daa0000:
            case 0x7dac0000:  case 0x7db20000:  case 0x7db40000:  case 0x7db60000:
            case 0x7db80000:  case 0x7dba0000:  case 0x7dbc0000:  case 0x7dfa0000:
            case 0x7dfc0000:  case 0x7dfe0000:
                return true;

            default:
                return false;
        }
    }

    bool InstructionDecoder_amdgpu_gfx1250::IS_ENC_VOP2(uint64_t I)
    {
        switch ( I & 0xfe000000 )  {
            case 0x2000000:  case 0x4000000:  case 0x6000000:  case 0x8000000:
            case 0xa000000:  case 0xc000000:  case 0xe000000:  case 0x10000000:
            case 0x12000000:  case 0x14000000:  case 0x16000000:  case 0x18000000:
            case 0x1a000000:  case 0x1c000000:  case 0x22000000:  case 0x24000000:
            case 0x26000000:  case 0x28000000:  case 0x2a000000:  case 0x2c000000:
            case 0x2e000000:  case 0x30000000:  case 0x32000000:  case 0x34000000:
            case 0x36000000:  case 0x38000000:  case 0x3a000000:  case 0x3c000000:
            case 0x3e000000:  case 0x40000000:  case 0x42000000:  case 0x44000000:
            case 0x4a000000:  case 0x4c000000:  case 0x4e000000:  case 0x50000000:
            case 0x52000000:  case 0x54000000:  case 0x56000000:  case 0x5e000000:
            case 0x60000000:  case 0x62000000:  case 0x64000000:  case 0x66000000:
            case 0x68000000:  case 0x6a000000:  case 0x6c000000:  case 0x76000000:
            case 0x78000000:
                return true;

            default:
                return false;
        }
    }

    bool InstructionDecoder_amdgpu_gfx1250::IS_ENC_VOP3(uint64_t I)
    {
        switch ( I & 0xffff0000 )  {
            case 0xd5800000:  case 0xd5810000:  case 0xd5820000:  case 0xd5830000:
            case 0xd5840000:  case 0xd5850000:  case 0xd5860000:  case 0xd5870000:
            case 0xd5880000:  case 0xd58a0000:  case 0xd58b0000:  case 0xd58c0000:
            case 0xd58d0000:  case 0xd58e0000:  case 0xd58f0000:  case 0xd5900000:
            case 0xd5910000:  case 0xd5920000:  case 0xd5930000:  case 0xd5940000:
            case 0xd5950000:  case 0xd5960000:  case 0xd5970000:  case 0xd5980000:
            case 0xd5990000:  case 0xd59a0000:  case 0xd59b0000:  case 0xd59c0000:
            case 0xd59d0000:  case 0xd59e0000:  case 0xd59f0000:  case 0xd5a00000:
            case 0xd5a10000:  case 0xd5a20000:  case 0xd5a30000:  case 0xd5a40000:
            case 0xd5a50000:  case 0xd5a70000:  case 0xd5aa0000:  case 0xd5ab0000:
            case 0xd5ae0000:  case 0xd5af0000:  case 0xd5b10000:  case 0xd5b30000:
            case 0xd5b40000:  case 0xd5b50000:  case 0xd5b60000:  case 0xd5b70000:
            case 0xd5b80000:  case 0xd5b90000:  case 0xd5ba0000:  case 0xd5bb0000:
            case 0xd5bc0000:  case 0xd5bd0000:  case 0xd5be0000:  case 0xd5bf0000:
            case 0xd5c00000:  case 0xd5c20000:  case 0xd5c30000:  case 0xd5c40000:
            case 0xd5c80000:  case 0xd5c90000:  case 0xd5ca0000:  case 0xd5cb0000:
            case 0xd5d00000:  case 0xd5d10000:  case 0xd5d20000:  case 0xd5d30000:
            case 0xd5d40000:  case 0xd5d50000:  case 0xd5d60000:  case 0xd5d70000:
            case 0xd5d80000:  case 0xd5d90000:  case 0xd5da0000:  case 0xd5db0000:
            case 0xd5dc0000:  case 0xd5dd0000:  case 0xd5de0000:  case 0xd5df0000:
            case 0xd5e00000:  case 0xd5e10000:  case 0xd5e20000:  case 0xd5e30000:
            case 0xd5e40000:  case 0xd5e90000:  case 0xd5ea0000:  case 0xd5eb0000:
            case 0xd5ec0000:  case 0xd5ed0000:  case 0xd5ee0000:  case 0xd5ef0000:
            case 0xd5f20000:  case 0xd5f30000:  case 0xd5f40000:  case 0xd5f50000:
            case 0xd5f60000:  case 0xd5f70000:  case 0xd5f80000:  case 0xd5f90000:
            case 0xd5fa0000:  case 0xd5fb0000:  case 0xd5fc0000:  case 0xd5fd0000:
            case 0xd5fe0000:  case 0xd5ff0000:  case 0xd5010000:  case 0xd5020000:
            case 0xd5030000:  case 0xd5040000:  case 0xd5050000:  case 0xd5060000:
            case 0xd5070000:  case 0xd5080000:  case 0xd5090000:  case 0xd50a0000:
            case 0xd50b0000:  case 0xd50c0000:  case 0xd50d0000:  case 0xd50e0000:
            case 0xd5110000:  case 0xd5120000:  case 0xd5130000:  case 0xd5140000:
            case 0xd5150000:  case 0xd5160000:  case 0xd5170000:  case 0xd5180000:
            case 0xd5190000:  case 0xd51a0000:  case 0xd51b0000:  case 0xd51c0000:
            case 0xd51d0000:  case 0xd51e0000:  case 0xd51f0000:  case 0xd5250000:
            case 0xd5260000:  case 0xd5270000:  case 0xd5280000:  case 0xd5290000:
            case 0xd52a0000:  case 0xd52b0000:  case 0xd52f0000:  case 0xd5300000:
            case 0xd5310000:  case 0xd5320000:  case 0xd5330000:  case 0xd5340000:
            case 0xd5350000:  case 0xd5360000:  case 0xd53b0000:  case 0xd6090000:
            case 0xd60a0000:  case 0xd60b0000:  case 0xd60c0000:  case 0xd60d0000:
            case 0xd60e0000:  case 0xd60f0000:  case 0xd6100000:  case 0xd6110000:
            case 0xd6120000:  case 0xd6130000:  case 0xd6140000:  case 0xd6150000:
            case 0xd6160000:  case 0xd6170000:  case 0xd6180000:  case 0xd61a0000:
            case 0xd61b0000:  case 0xd61d0000:  case 0xd61e0000:  case 0xd6200000:
            case 0xd6210000:  case 0xd6220000:  case 0xd6230000:  case 0xd6240000:
            case 0xd6250000:  case 0xd6260000:  case 0xd6270000:  case 0xd6280000:
            case 0xd6290000:  case 0xd62a0000:  case 0xd62b0000:  case 0xd62c0000:
            case 0xd62d0000:  case 0xd62e0000:  case 0xd62f0000:  case 0xd6300000:
            case 0xd6310000:  case 0xd6320000:  case 0xd6330000:  case 0xd6340000:
            case 0xd6350000:  case 0xd6370000:  case 0xd6380000:  case 0xd6390000:
            case 0xd63a0000:  case 0xd63b0000:  case 0xd63d0000:  case 0xd63f0000:
            case 0xd6400000:  case 0xd6410000:  case 0xd6420000:  case 0xd6430000:
            case 0xd6440000:  case 0xd6450000:  case 0xd6460000:  case 0xd6470000:
            case 0xd6480000:  case 0xd64a0000:  case 0xd64b0000:  case 0xd64d0000:
            case 0xd64e0000:  case 0xd6500000:  case 0xd6510000:  case 0xd6520000:
            case 0xd6530000:  case 0xd6540000:  case 0xd6550000:  case 0xd6560000:
            case 0xd6570000:  case 0xd6580000:  case 0xd6590000:  case 0xd65a0000:
            case 0xd65b0000:  case 0xd65c0000:  case 0xd65d0000:  case 0xd65e0000:
            case 0xd65f0000:  case 0xd6600000:  case 0xd6610000:  case 0xd6620000:
            case 0xd6630000:  case 0xd6640000:  case 0xd6650000:  case 0xd6680000:
            case 0xd6690000:  case 0xd66a0000:  case 0xd66b0000:  case 0xd66c0000:
            case 0xd66d0000:  case 0xd66e0000:  case 0xd66f0000:  case 0xd6700000:
            case 0xd6710000:  case 0xd6720000:  case 0xd6730000:  case 0xd6800000:
            case 0xd6810000:  case 0xd6820000:  case 0xd6830000:  case 0xd6840000:
            case 0xd6850000:  case 0xd6860000:  case 0xd6870000:  case 0xd6880000:
            case 0xd6890000:  case 0xd6900000:  case 0xd6910000:  case 0xd6970000:
            case 0xd6980000:  case 0xd6990000:  case 0xd69f0000:  case 0xd6a00000:
            case 0xd6a10000:  case 0xd6a80000:  case 0xd6a90000:  case 0xd6aa0000:
            case 0xd6ab0000:  case 0xd6ac0000:  case 0xd6ad0000:  case 0xd6b00000:
            case 0xd6b30000:  case 0xd6b40000:  case 0xd6b50000:  case 0xd6b80000:
            case 0xd6b90000:  case 0xd6bc0000:  case 0xd6bf0000:  case 0xd6c00000:
            case 0xd6c10000:  case 0xd6c20000:  case 0xd6c30000:  case 0xd6c40000:
            case 0xd6c50000:  case 0xd6c60000:  case 0xd6c70000:  case 0xd6c80000:
            case 0xd6c90000:  case 0xd6ca0000:  case 0xd6cb0000:  case 0xd6cc0000:
            case 0xd6cd0000:  case 0xd6ce0000:  case 0xd6cf0000:  case 0xd6d00000:
            case 0xd6d10000:  case 0xd6d20000:  case 0xd6d30000:  case 0xd6d40000:
            case 0xd6d50000:  case 0xd6d60000:  case 0xd6d70000:  case 0xd6d80000:
            case 0xd6fa0000:  case 0xd6fb0000:  case 0xd7030000:  case 0xd7040000:
            case 0xd7050000:  case 0xd7060000:  case 0xd7070000:  case 0xd7090000:
            case 0xd70a0000:  case 0xd70b0000:  case 0xd70c0000:  case 0xd70d0000:
            case 0xd70e0000:  case 0xd70f0000:  case 0xd7100000:  case 0xd7110000:
            case 0xd7120000:  case 0xd7130000:  case 0xd7140000:  case 0xd7180000:
            case 0xd7190000:  case 0xd71a0000:  case 0xd71b0000:  case 0xd71c0000:
            case 0xd71d0000:  case 0xd71e0000:  case 0xd71f0000:  case 0xd7200000:
            case 0xd7210000:  case 0xd7220000:  case 0xd7230000:  case 0xd7240000:
            case 0xd7250000:  case 0xd7260000:  case 0xd72b0000:  case 0xd72c0000:
            case 0xd72d0000:  case 0xd72e0000:  case 0xd72f0000:  case 0xd7380000:
            case 0xd7390000:  case 0xd73a0000:  case 0xd73d0000:  case 0xd73e0000:
            case 0xd7410000:  case 0xd7420000:  case 0xd7600000:  case 0xd7610000:
            case 0xd7620000:  case 0xd7630000:  case 0xd7640000:  case 0xd7650000:
            case 0xd7660000:  case 0xd7670000:  case 0xd7680000:  case 0xd7690000:
            case 0xd76a0000:  case 0xd76b0000:  case 0xd76c0000:  case 0xd76d0000:
            case 0xd76e0000:  case 0xd76f0000:  case 0xd7700000:  case 0xd7720000:
            case 0xd7730000:  case 0xd7740000:  case 0xd7750000:  case 0xd4010000:
            case 0xd4020000:  case 0xd4030000:  case 0xd4040000:  case 0xd4050000:
            case 0xd4060000:  case 0xd4070000:  case 0xd4080000:  case 0xd4090000:
            case 0xd40a0000:  case 0xd40b0000:  case 0xd40c0000:  case 0xd40d0000:
            case 0xd40e0000:  case 0xd4110000:  case 0xd4120000:  case 0xd4130000:
            case 0xd4140000:  case 0xd4150000:  case 0xd4160000:  case 0xd4170000:
            case 0xd4180000:  case 0xd4190000:  case 0xd41a0000:  case 0xd41b0000:
            case 0xd41c0000:  case 0xd41d0000:  case 0xd41e0000:  case 0xd4210000:
            case 0xd4220000:  case 0xd4230000:  case 0xd4240000:  case 0xd4250000:
            case 0xd4260000:  case 0xd4270000:  case 0xd4280000:  case 0xd4290000:
            case 0xd42a0000:  case 0xd42b0000:  case 0xd42c0000:  case 0xd42d0000:
            case 0xd42e0000:  case 0xd4310000:  case 0xd4320000:  case 0xd4330000:
            case 0xd4340000:  case 0xd4350000:  case 0xd4360000:  case 0xd4390000:
            case 0xd43a0000:  case 0xd43b0000:  case 0xd43c0000:  case 0xd43d0000:
            case 0xd43e0000:  case 0xd4410000:  case 0xd4420000:  case 0xd4430000:
            case 0xd4440000:  case 0xd4450000:  case 0xd4460000:  case 0xd4490000:
            case 0xd44a0000:  case 0xd44b0000:  case 0xd44c0000:  case 0xd44d0000:
            case 0xd44e0000:  case 0xd4510000:  case 0xd4520000:  case 0xd4530000:
            case 0xd4540000:  case 0xd4550000:  case 0xd4560000:  case 0xd4590000:
            case 0xd45a0000:  case 0xd45b0000:  case 0xd45c0000:  case 0xd45d0000:
            case 0xd45e0000:  case 0xd47d0000:  case 0xd47e0000:  case 0xd47f0000:
            case 0xd4810000:  case 0xd4820000:  case 0xd4830000:  case 0xd4840000:
            case 0xd4850000:  case 0xd4860000:  case 0xd4870000:  case 0xd4880000:
            case 0xd4890000:  case 0xd48a0000:  case 0xd48b0000:  case 0xd48c0000:
            case 0xd48d0000:  case 0xd48e0000:  case 0xd4910000:  case 0xd4920000:
            case 0xd4930000:  case 0xd4940000:  case 0xd4950000:  case 0xd4960000:
            case 0xd4970000:  case 0xd4980000:  case 0xd4990000:  case 0xd49a0000:
            case 0xd49b0000:  case 0xd49c0000:  case 0xd49d0000:  case 0xd49e0000:
            case 0xd4a10000:  case 0xd4a20000:  case 0xd4a30000:  case 0xd4a40000:
            case 0xd4a50000:  case 0xd4a60000:  case 0xd4a70000:  case 0xd4a80000:
            case 0xd4a90000:  case 0xd4aa0000:  case 0xd4ab0000:  case 0xd4ac0000:
            case 0xd4ad0000:  case 0xd4ae0000:  case 0xd4b10000:  case 0xd4b20000:
            case 0xd4b30000:  case 0xd4b40000:  case 0xd4b50000:  case 0xd4b60000:
            case 0xd4b90000:  case 0xd4ba0000:  case 0xd4bb0000:  case 0xd4bc0000:
            case 0xd4bd0000:  case 0xd4be0000:  case 0xd4c10000:  case 0xd4c20000:
            case 0xd4c30000:  case 0xd4c40000:  case 0xd4c50000:  case 0xd4c60000:
            case 0xd4c90000:  case 0xd4ca0000:  case 0xd4cb0000:  case 0xd4cc0000:
            case 0xd4cd0000:  case 0xd4ce0000:  case 0xd4d10000:  case 0xd4d20000:
            case 0xd4d30000:  case 0xd4d40000:  case 0xd4d50000:  case 0xd4d60000:
            case 0xd4d90000:  case 0xd4da0000:  case 0xd4db0000:  case 0xd4dc0000:
            case 0xd4dd0000:  case 0xd4de0000:  case 0xd4fd0000:  case 0xd4fe0000:
            case 0xd4ff0000:
                return true;

            default:
                return false;
        }
    }

    bool InstructionDecoder_amdgpu_gfx1250::IS_ENC_VOP3PX2(uint64_t I)
    {
        switch ( I & 0xffff0000 )  {
            case 0xd3ac0000:
                return true;

            default:
                return false;
        }
    }

    bool InstructionDecoder_amdgpu_gfx1250::IS_ENC_VOP3PX3(uint64_t I)
    {
        switch ( I & 0xffff0000 )  {
            case 0xcc3b0000:  case 0xccba0000:
                return true;

            default:
                return false;
        }
    }

    bool InstructionDecoder_amdgpu_gfx1250::IS_ENC_VOP3P(uint64_t I)
    {
        switch ( I & 0xffff0000 )  {
            case 0xcc000000:  case 0xcc010000:  case 0xcc020000:  case 0xcc030000:
            case 0xcc040000:  case 0xcc050000:  case 0xcc060000:  case 0xcc070000:
            case 0xcc080000:  case 0xcc090000:  case 0xcc0a0000:  case 0xcc0b0000:
            case 0xcc0c0000:  case 0xcc0d0000:  case 0xcc0e0000:  case 0xcc0f0000:
            case 0xcc100000:  case 0xcc110000:  case 0xcc140000:  case 0xcc150000:
            case 0xcc160000:  case 0xcc170000:  case 0xcc180000:  case 0xcc190000:
            case 0xcc1b0000:  case 0xcc1c0000:  case 0xcc1d0000:  case 0xcc1e0000:
            case 0xcc1f0000:  case 0xcc200000:  case 0xcc210000:  case 0xcc220000:
            case 0xcc230000:  case 0xcc280000:  case 0xcc290000:  case 0xcc2a0000:
            case 0xcc2b0000:  case 0xcc2c0000:  case 0xcc2d0000:  case 0xcc2e0000:
            case 0xcc2f0000:  case 0xcc300000:  case 0xcc310000:  case 0xcc320000:
            case 0xcc330000:  case 0xcc360000:  case 0xcc370000:  case 0xcc380000:
            case 0xcc390000:  case 0xcc3d0000:  case 0xcc3e0000:  case 0xcc3f0000:
            case 0xcc5d0000:  case 0xcc600000:  case 0xcc610000:  case 0xcc620000:
            case 0xcc630000:  case 0xcc640000:  case 0xcc650000:  case 0xcc660000:
            case 0xcc670000:  case 0xcc680000:  case 0xcc690000:  case 0xcc6a0000:
            case 0xcc6b0000:  case 0xcc6c0000:  case 0xcc6d0000:  case 0xcc6e0000:
            case 0xcc6f0000:  case 0xcc700000:  case 0xcc710000:  case 0xcc720000:
            case 0xcc730000:  case 0xcc740000:  case 0xcc750000:  case 0xcc760000:
            case 0xcc770000:  case 0xcc780000:  case 0xcc790000:  case 0xcc7a0000:
            case 0xcc7b0000:  case 0xcc800000:  case 0xcc810000:  case 0xcc820000:
            case 0xcc830000:  case 0xcc840000:  case 0xcc850000:  case 0xcc860000:
            case 0xcc870000:  case 0xcc880000:
                return true;

            default:
                return false;
        }
    }

    bool InstructionDecoder_amdgpu_gfx1250::IS_ENC_VDS(uint64_t I)
    {
        switch ( I & 0xfffc0000 )  {
            case 0xd8000000:  case 0xd8040000:  case 0xd8080000:  case 0xd80c0000:
            case 0xd8100000:  case 0xd8140000:  case 0xd8180000:  case 0xd81c0000:
            case 0xd8200000:  case 0xd8240000:  case 0xd8280000:  case 0xd82c0000:
            case 0xd8300000:  case 0xd8340000:  case 0xd8380000:  case 0xd83c0000:
            case 0xd8400000:  case 0xd8480000:  case 0xd84c0000:  case 0xd8500000:
            case 0xd8540000:  case 0xd8780000:  case 0xd87c0000:  case 0xd8800000:
            case 0xd8840000:  case 0xd8880000:  case 0xd88c0000:  case 0xd8900000:
            case 0xd8940000:  case 0xd8980000:  case 0xd89c0000:  case 0xd8a00000:
            case 0xd8a40000:  case 0xd8a80000:  case 0xd8ac0000:  case 0xd8b00000:
            case 0xd8b40000:  case 0xd8b80000:  case 0xd8bc0000:  case 0xd8c00000:
            case 0xd8c80000:  case 0xd8cc0000:  case 0xd8d40000:  case 0xd8d80000:
            case 0xd8dc0000:  case 0xd8e00000:  case 0xd8e40000:  case 0xd8e80000:
            case 0xd8ec0000:  case 0xd8f00000:  case 0xd8f40000:  case 0xd8f80000:
            case 0xd9000000:  case 0xd9040000:  case 0xd9080000:  case 0xd90c0000:
            case 0xd9100000:  case 0xd9140000:  case 0xd9180000:  case 0xd91c0000:
            case 0xd9200000:  case 0xd9240000:  case 0xd9280000:  case 0xd92c0000:
            case 0xd9300000:  case 0xd9340000:  case 0xd9380000:  case 0xd93c0000:
            case 0xd9400000:  case 0xd9480000:  case 0xd94c0000:  case 0xd9500000:
            case 0xd9580000:  case 0xd9800000:  case 0xd9840000:  case 0xd9880000:
            case 0xd98c0000:  case 0xd9900000:  case 0xd9940000:  case 0xd9980000:
            case 0xd99c0000:  case 0xd9a00000:  case 0xd9a40000:  case 0xd9a80000:
            case 0xd9ac0000:  case 0xd9b00000:  case 0xd9b40000:  case 0xd9b80000:
            case 0xd9bc0000:  case 0xd9c00000:  case 0xd9c80000:  case 0xd9cc0000:
            case 0xd9d00000:  case 0xd9d40000:  case 0xd9d80000:  case 0xd9dc0000:
            case 0xd9e00000:  case 0xd9e40000:  case 0xd9f80000:  case 0xda600000:
            case 0xda640000:  case 0xda680000:  case 0xda6c0000:  case 0xda800000:
            case 0xda840000:  case 0xda880000:  case 0xda8c0000:  case 0xda900000:
            case 0xda940000:  case 0xda980000:  case 0xda9c0000:  case 0xdaa00000:
            case 0xdaa40000:  case 0xdaa80000:  case 0xdaac0000:  case 0xdac00000:
            case 0xdac40000:  case 0xdac80000:  case 0xdacc0000:  case 0xdb340000:
            case 0xdb780000:  case 0xdb7c0000:  case 0xdbe80000:  case 0xdbec0000:
            case 0xdbf00000:  case 0xdbf40000:  case 0xdbf80000:  case 0xdbfc0000:
                return true;

            default:
                return false;
        }
    }

    bool InstructionDecoder_amdgpu_gfx1250::IS_ENC_VBUFFER(uint64_t I)
    {
        switch ( I & 0xfc3fc000 )  {
            case 0xc4040000:  case 0xc4044000:  case 0xc4048000:  case 0xc404c000:
            case 0xc4050000:  case 0xc4054000:  case 0xc4058000:  case 0xc405c000:
            case 0xc4060000:  case 0xc4064000:  case 0xc4068000:  case 0xc406c000:
            case 0xc4070000:  case 0xc4074000:  case 0xc4078000:  case 0xc407c000:
            case 0xc4080000:  case 0xc4084000:  case 0xc4088000:  case 0xc408c000:
            case 0xc4090000:  case 0xc4094000:  case 0xc40cc000:  case 0xc40d0000:
            case 0xc40d4000:  case 0xc40d8000:  case 0xc40dc000:  case 0xc40e0000:
            case 0xc40e4000:  case 0xc40e8000:  case 0xc40ec000:  case 0xc40f0000:
            case 0xc40f4000:  case 0xc40f8000:  case 0xc40fc000:  case 0xc4100000:
            case 0xc4104000:  case 0xc4108000:  case 0xc410c000:  case 0xc4110000:
            case 0xc4114000:  case 0xc4118000:  case 0xc411c000:  case 0xc4120000:
            case 0xc4124000:  case 0xc4128000:  case 0xc412c000:  case 0xc4130000:
            case 0xc4134000:  case 0xc4140000:  case 0xc4144000:  case 0xc4148000:
            case 0xc4154000:  case 0xc4158000:  case 0xc4164000:  case 0xc4168000:
            case 0xc416c000:  case 0xc4170000:
                return true;

            default:
                return false;
        }
    }

    bool InstructionDecoder_amdgpu_gfx1250::IS_ENC_VIMAGE(uint64_t I)
    {
        switch ( I & 0xfc3fc000 )  {
            case 0xd0310000:  case 0xd0314000:
                return true;

            default:
                return false;
        }
    }

    bool InstructionDecoder_amdgpu_gfx1250::IS_ENC_VFLAT(uint64_t I)
    {
        switch ( I & 0xff3fc000 )  {
            case 0xec040000:  case 0xec044000:  case 0xec048000:  case 0xec04c000:
            case 0xec050000:  case 0xec054000:  case 0xec058000:  case 0xec05c000:
            case 0xec060000:  case 0xec064000:  case 0xec068000:  case 0xec06c000:
            case 0xec070000:  case 0xec074000:  case 0xec078000:  case 0xec07c000:
            case 0xec080000:  case 0xec084000:  case 0xec088000:  case 0xec08c000:
            case 0xec090000:  case 0xec094000:  case 0xec0cc000:  case 0xec0d0000:
            case 0xec0d4000:  case 0xec0d8000:  case 0xec0dc000:  case 0xec0e0000:
            case 0xec0e4000:  case 0xec0e8000:  case 0xec0ec000:  case 0xec0f0000:
            case 0xec0f4000:  case 0xec0f8000:  case 0xec0fc000:  case 0xec100000:
            case 0xec104000:  case 0xec108000:  case 0xec10c000:  case 0xec110000:
            case 0xec114000:  case 0xec118000:  case 0xec11c000:  case 0xec120000:
            case 0xec124000:  case 0xec128000:  case 0xec12c000:  case 0xec130000:
            case 0xec134000:  case 0xec140000:  case 0xec144000:  case 0xec148000:
            case 0xec154000:  case 0xec158000:  case 0xec164000:  case 0xec168000:
            case 0xec16c000:  case 0xec170000:  case 0xec174000:  case 0xec1c0000:
            case 0xec1c4000:  case 0xec1c8000:
                return true;

            default:
                return false;
        }
    }

    bool InstructionDecoder_amdgpu_gfx1250::IS_ENC_VSCRATCH(uint64_t I)
    {
        switch ( I & 0xff3fc000 )  {
            case 0xed040000:  case 0xed044000:  case 0xed048000:  case 0xed04c000:
            case 0xed050000:  case 0xed054000:  case 0xed058000:  case 0xed05c000:
            case 0xed060000:  case 0xed064000:  case 0xed068000:  case 0xed06c000:
            case 0xed070000:  case 0xed074000:  case 0xed078000:  case 0xed07c000:
            case 0xed080000:  case 0xed084000:  case 0xed088000:  case 0xed08c000:
            case 0xed090000:  case 0xed094000:  case 0xed14c000:  case 0xed150000:
                return true;

            default:
                return false;
        }
    }

    bool InstructionDecoder_amdgpu_gfx1250::IS_ENC_VGLOBAL(uint64_t I)
    {
        switch ( I & 0xff3fc000 )  {
            case 0xee040000:  case 0xee044000:  case 0xee048000:  case 0xee04c000:
            case 0xee050000:  case 0xee054000:  case 0xee058000:  case 0xee05c000:
            case 0xee060000:  case 0xee064000:  case 0xee068000:  case 0xee06c000:
            case 0xee070000:  case 0xee074000:  case 0xee078000:  case 0xee07c000:
            case 0xee080000:  case 0xee084000:  case 0xee088000:  case 0xee08c000:
            case 0xee090000:  case 0xee094000:  case 0xee0a0000:  case 0xee0a4000:
            case 0xee0ac000:  case 0xee0b0000:  case 0xee0cc000:  case 0xee0d0000:
            case 0xee0d4000:  case 0xee0d8000:  case 0xee0dc000:  case 0xee0e0000:
            case 0xee0e4000:  case 0xee0e8000:  case 0xee0ec000:  case 0xee0f0000:
            case 0xee0f4000:  case 0xee0f8000:  case 0xee0fc000:  case 0xee100000:
            case 0xee104000:  case 0xee108000:  case 0xee10c000:  case 0xee110000:
            case 0xee114000:  case 0xee118000:  case 0xee11c000:  case 0xee120000:
            case 0xee124000:  case 0xee128000:  case 0xee12c000:  case 0xee130000:
            case 0xee134000:  case 0xee13c000:  case 0xee140000:  case 0xee144000:
            case 0xee148000:  case 0xee14c000:  case 0xee150000:  case 0xee154000:
            case 0xee158000:  case 0xee15c000:  case 0xee160000:  case 0xee164000:
            case 0xee168000:  case 0xee16c000:  case 0xee170000:  case 0xee174000:
            case 0xee17c000:  case 0xee180000:  case 0xee184000:  case 0xee188000:
            case 0xee18c000:  case 0xee190000:  case 0xee194000:  case 0xee198000:
            case 0xee19c000:  case 0xee1a0000:  case 0xee1a4000:  case 0xee1a8000:
            case 0xee1ac000:  case 0xee1b0000:  case 0xee1b4000:  case 0xee1c0000:
            case 0xee1c4000:  case 0xee1c8000:  case 0xee1cc000:  case 0xee1d0000:
                return true;

            default:
                return false;
        }
    }

    bool InstructionDecoder_amdgpu_gfx1250::IS_SOP1_INST_LITERAL(uint64_t I)
    {
        switch ( I & 0xff80ff00 )  {
            case 0xbe800000:  case 0xbe800100:  case 0xbe800200:  case 0xbe800300:
            case 0xbe800400:  case 0xbe800500:  case 0xbe800800:  case 0xbe800900:
            case 0xbe800a00:  case 0xbe800b00:  case 0xbe800c00:  case 0xbe800d00:
            case 0xbe800e00:  case 0xbe800f00:  case 0xbe801000:  case 0xbe801100:
            case 0xbe801200:  case 0xbe801300:  case 0xbe801400:  case 0xbe801500:
            case 0xbe801600:  case 0xbe801700:  case 0xbe801800:  case 0xbe801900:
            case 0xbe801a00:  case 0xbe801b00:  case 0xbe801c00:  case 0xbe801d00:
            case 0xbe801e00:  case 0xbe801f00:  case 0xbe802000:  case 0xbe802100:
            case 0xbe802200:  case 0xbe802300:  case 0xbe802400:  case 0xbe802500:
            case 0xbe802600:  case 0xbe802700:  case 0xbe802800:  case 0xbe802900:
            case 0xbe802a00:  case 0xbe802b00:  case 0xbe802c00:  case 0xbe802d00:
            case 0xbe802e00:  case 0xbe802f00:  case 0xbe803000:  case 0xbe803100:
            case 0xbe803200:  case 0xbe803300:  case 0xbe803400:  case 0xbe803500:
            case 0xbe803600:  case 0xbe803700:  case 0xbe804200:  case 0xbe804300:
            case 0xbe804b00:  case 0xbe805300:  case 0xbe805800:  case 0xbe806000:
            case 0xbe806100:  case 0xbe806200:  case 0xbe806300:  case 0xbe806400:
            case 0xbe806500:  case 0xbe806600:  case 0xbe806700:  case 0xbe806800:
            case 0xbe806900:  case 0xbe806a00:  case 0xbe806b00:  case 0xbe806c00:
            case 0xbe806d00:  case 0xbe806e00:
                return true;

            default:
                return false;
        }
    }

    bool InstructionDecoder_amdgpu_gfx1250::IS_SOP1_INST_LITERAL64(uint64_t I)
    {
        switch ( I & 0xff80ff00 )  {
            case 0xbe800000:  case 0xbe800100:  case 0xbe800200:  case 0xbe800300:
            case 0xbe800400:  case 0xbe800500:  case 0xbe800800:  case 0xbe800900:
            case 0xbe800a00:  case 0xbe800b00:  case 0xbe800c00:  case 0xbe800d00:
            case 0xbe800e00:  case 0xbe800f00:  case 0xbe801000:  case 0xbe801100:
            case 0xbe801200:  case 0xbe801300:  case 0xbe801400:  case 0xbe801500:
            case 0xbe801600:  case 0xbe801700:  case 0xbe801800:  case 0xbe801900:
            case 0xbe801a00:  case 0xbe801b00:  case 0xbe801c00:  case 0xbe801d00:
            case 0xbe801e00:  case 0xbe801f00:  case 0xbe802000:  case 0xbe802100:
            case 0xbe802200:  case 0xbe802300:  case 0xbe802400:  case 0xbe802500:
            case 0xbe802600:  case 0xbe802700:  case 0xbe802800:  case 0xbe802900:
            case 0xbe802a00:  case 0xbe802b00:  case 0xbe802c00:  case 0xbe802d00:
            case 0xbe802e00:  case 0xbe802f00:  case 0xbe803000:  case 0xbe803100:
            case 0xbe803200:  case 0xbe803300:  case 0xbe803400:  case 0xbe803500:
            case 0xbe803600:  case 0xbe803700:  case 0xbe804200:  case 0xbe804300:
            case 0xbe804b00:  case 0xbe805300:  case 0xbe805800:  case 0xbe806000:
            case 0xbe806100:  case 0xbe806200:  case 0xbe806300:  case 0xbe806400:
            case 0xbe806500:  case 0xbe806600:  case 0xbe806700:  case 0xbe806800:
            case 0xbe806900:  case 0xbe806a00:  case 0xbe806b00:  case 0xbe806c00:
            case 0xbe806d00:  case 0xbe806e00:
                return true;

            default:
                return false;
        }
    }

    bool InstructionDecoder_amdgpu_gfx1250::IS_SOP2_INST_LITERAL(uint64_t I)
    {
        switch ( I & 0xff800000 )  {
            case 0x80000000:  case 0x80800000:  case 0x81000000:  case 0x81800000:
            case 0x82000000:  case 0x82800000:  case 0x83000000:  case 0x84000000:
            case 0x84800000:  case 0x85000000:  case 0x85800000:  case 0x86000000:
            case 0x86800000:  case 0x87000000:  case 0x87800000:  case 0x88000000:
            case 0x88800000:  case 0x89000000:  case 0x89800000:  case 0x8a000000:
            case 0x8a800000:  case 0x8b000000:  case 0x8b800000:  case 0x8c000000:
            case 0x8c800000:  case 0x8d000000:  case 0x8d800000:  case 0x8e000000:
            case 0x8e800000:  case 0x8f000000:  case 0x8f800000:  case 0x90000000:
            case 0x90800000:  case 0x91000000:  case 0x91800000:  case 0x92000000:
            case 0x92800000:  case 0x93000000:  case 0x93800000:  case 0x94000000:
            case 0x94800000:  case 0x95000000:  case 0x95800000:  case 0x96000000:
            case 0x96800000:  case 0x97000000:  case 0x98000000:  case 0x98800000:
            case 0x99000000:  case 0x99800000:  case 0x9a000000:  case 0x9a800000:
            case 0xa0000000:  case 0xa0800000:  case 0xa1000000:  case 0xa1800000:
            case 0xa2000000:  case 0xa2800000:  case 0xa3000000:  case 0xa3800000:
            case 0xa4000000:  case 0xa4800000:  case 0xa5000000:  case 0xa5800000:
            case 0xa6000000:  case 0xa6800000:  case 0xa7000000:  case 0xa7800000:
            case 0xa8000000:  case 0xa8800000:  case 0xa9000000:  case 0xa9800000:
            case 0xaa000000:  case 0xaa800000:
                return true;

            default:
                return false;
        }
    }

    bool InstructionDecoder_amdgpu_gfx1250::IS_SOP2_INST_LITERAL64(uint64_t I)
    {
        switch ( I & 0xff800000 )  {
            case 0x80000000:  case 0x80800000:  case 0x81000000:  case 0x81800000:
            case 0x82000000:  case 0x82800000:  case 0x83000000:  case 0x84000000:
            case 0x84800000:  case 0x85000000:  case 0x85800000:  case 0x86000000:
            case 0x86800000:  case 0x87000000:  case 0x87800000:  case 0x88000000:
            case 0x88800000:  case 0x89000000:  case 0x89800000:  case 0x8a000000:
            case 0x8a800000:  case 0x8b000000:  case 0x8b800000:  case 0x8c000000:
            case 0x8c800000:  case 0x8d000000:  case 0x8d800000:  case 0x8e000000:
            case 0x8e800000:  case 0x8f000000:  case 0x8f800000:  case 0x90000000:
            case 0x90800000:  case 0x91000000:  case 0x91800000:  case 0x92000000:
            case 0x92800000:  case 0x93000000:  case 0x93800000:  case 0x94000000:
            case 0x94800000:  case 0x95000000:  case 0x95800000:  case 0x96000000:
            case 0x96800000:  case 0x97000000:  case 0x98000000:  case 0x98800000:
            case 0x99000000:  case 0x99800000:  case 0x9a000000:  case 0x9a800000:
            case 0xa0000000:  case 0xa0800000:  case 0xa1000000:  case 0xa1800000:
            case 0xa2000000:  case 0xa3800000:  case 0xa4000000:  case 0xa4800000:
            case 0xa5000000:  case 0xa5800000:  case 0xa6000000:  case 0xa6800000:
            case 0xa7000000:  case 0xa7800000:  case 0xa8000000:  case 0xa8800000:
            case 0xa9000000:  case 0xa9800000:  case 0xaa000000:  case 0xaa800000:
                return true;

            default:
                return false;
        }
    }

    bool InstructionDecoder_amdgpu_gfx1250::IS_SOPC_INST_LITERAL(uint64_t I)
    {
        switch ( I & 0xffff0000 )  {
            case 0xbf000000:  case 0xbf010000:  case 0xbf020000:  case 0xbf030000:
            case 0xbf040000:  case 0xbf050000:  case 0xbf060000:  case 0xbf070000:
            case 0xbf080000:  case 0xbf090000:  case 0xbf0a0000:  case 0xbf0b0000:
            case 0xbf0c0000:  case 0xbf0d0000:  case 0xbf0e0000:  case 0xbf0f0000:
            case 0xbf100000:  case 0xbf110000:  case 0xbf410000:  case 0xbf510000:
            case 0xbf420000:  case 0xbf520000:  case 0xbf430000:  case 0xbf530000:
            case 0xbf440000:  case 0xbf540000:  case 0xbf450000:  case 0xbf550000:
            case 0xbf460000:  case 0xbf560000:  case 0xbf470000:  case 0xbf570000:
            case 0xbf480000:  case 0xbf580000:  case 0xbf490000:  case 0xbf590000:
            case 0xbf4a0000:  case 0xbf5a0000:  case 0xbf4b0000:  case 0xbf5b0000:
            case 0xbf4c0000:  case 0xbf5c0000:  case 0xbf4d0000:  case 0xbf5d0000:
            case 0xbf4e0000:  case 0xbf5e0000:
                return true;

            default:
                return false;
        }
    }

    bool InstructionDecoder_amdgpu_gfx1250::IS_SOPC_INST_LITERAL64(uint64_t I)
    {
        switch ( I & 0xffff0000 )  {
            case 0xbf000000:  case 0xbf010000:  case 0xbf020000:  case 0xbf030000:
            case 0xbf040000:  case 0xbf050000:  case 0xbf060000:  case 0xbf070000:
            case 0xbf080000:  case 0xbf090000:  case 0xbf0a0000:  case 0xbf0b0000:
            case 0xbf0c0000:  case 0xbf0d0000:  case 0xbf0e0000:  case 0xbf0f0000:
            case 0xbf100000:  case 0xbf110000:  case 0xbf410000:  case 0xbf510000:
            case 0xbf420000:  case 0xbf520000:  case 0xbf430000:  case 0xbf530000:
            case 0xbf440000:  case 0xbf540000:  case 0xbf450000:  case 0xbf550000:
            case 0xbf460000:  case 0xbf560000:  case 0xbf470000:  case 0xbf570000:
            case 0xbf480000:  case 0xbf580000:  case 0xbf490000:  case 0xbf590000:
            case 0xbf4a0000:  case 0xbf5a0000:  case 0xbf4b0000:  case 0xbf5b0000:
            case 0xbf4c0000:  case 0xbf5c0000:  case 0xbf4d0000:  case 0xbf5d0000:
            case 0xbf4e0000:  case 0xbf5e0000:
                return true;

            default:
                return false;
        }
    }

    bool InstructionDecoder_amdgpu_gfx1250::IS_SOPK_INST_LITERAL(uint64_t I)
    {
        switch ( I & 0xff800000 )  {
            case 0xb9800000:
                return true;

            default:
                return false;
        }
    }

    bool InstructionDecoder_amdgpu_gfx1250::IS_VOP1_INST_LITERAL(uint64_t I)
    {
        switch ( I & 0xfe00fe00 )  {
            case 0x7e000200:  case 0x7e000600:  case 0x7e000800:  case 0x7e000a00:
            case 0x7e000c00:  case 0x7e000e00:  case 0x7e001000:  case 0x7e001400:
            case 0x7e001600:  case 0x7e001800:  case 0x7e001a00:  case 0x7e001c00:
            case 0x7e001e00:  case 0x7e002000:  case 0x7e002200:  case 0x7e002400:
            case 0x7e002600:  case 0x7e002800:  case 0x7e002a00:  case 0x7e002c00:
            case 0x7e002e00:  case 0x7e003000:  case 0x7e003200:  case 0x7e003400:
            case 0x7e003800:  case 0x7e003a00:  case 0x7e003c00:  case 0x7e003e00:
            case 0x7e004000:  case 0x7e004200:  case 0x7e004400:  case 0x7e004600:
            case 0x7e004800:  case 0x7e004a00:  case 0x7e004e00:  case 0x7e005400:
            case 0x7e005600:  case 0x7e005c00:  case 0x7e005e00:  case 0x7e006200:
            case 0x7e006600:  case 0x7e006800:  case 0x7e006a00:  case 0x7e006c00:
            case 0x7e006e00:  case 0x7e007000:  case 0x7e007200:  case 0x7e007400:
            case 0x7e007600:  case 0x7e007800:  case 0x7e007a00:  case 0x7e007c00:
            case 0x7e007e00:  case 0x7e008000:  case 0x7e008400:  case 0x7e009400:
            case 0x7e009600:  case 0x7e00a000:  case 0x7e00a200:  case 0x7e00a400:
            case 0x7e00a600:  case 0x7e00a800:  case 0x7e00aa00:  case 0x7e00ac00:
            case 0x7e00ae00:  case 0x7e00b000:  case 0x7e00b200:  case 0x7e00b400:
            case 0x7e00b600:  case 0x7e00b800:  case 0x7e00ba00:  case 0x7e00bc00:
            case 0x7e00be00:  case 0x7e00c000:  case 0x7e00c200:  case 0x7e00c400:
            case 0x7e00c600:  case 0x7e00c800:  case 0x7e00d200:  case 0x7e00d400:
            case 0x7e00d600:  case 0x7e00d800:  case 0x7e00da00:  case 0x7e00dc00:
            case 0x7e00de00:  case 0x7e00e400:  case 0x7e00e600:  case 0x7e00e800:
            case 0x7e00ea00:  case 0x7e00ec00:  case 0x7e00ee00:  case 0x7e00f000:
            case 0x7e00f200:  case 0x7e00f400:  case 0x7e00f600:  case 0x7e00f800:
            case 0x7e00fa00:  case 0x7e00fc00:  case 0x7e00fe00:
                return true;

            default:
                return false;
        }
    }

    bool InstructionDecoder_amdgpu_gfx1250::IS_VOP1_INST_LITERAL64(uint64_t I)
    {
        switch ( I & 0xfe00fe00 )  {
            case 0x7e000200:  case 0x7e000600:  case 0x7e000800:  case 0x7e000a00:
            case 0x7e000c00:  case 0x7e000e00:  case 0x7e001000:  case 0x7e001400:
            case 0x7e001600:  case 0x7e001800:  case 0x7e001a00:  case 0x7e001c00:
            case 0x7e001e00:  case 0x7e002000:  case 0x7e002200:  case 0x7e002400:
            case 0x7e002600:  case 0x7e002800:  case 0x7e002a00:  case 0x7e002c00:
            case 0x7e002e00:  case 0x7e003000:  case 0x7e003200:  case 0x7e003400:
            case 0x7e003800:  case 0x7e003a00:  case 0x7e003c00:  case 0x7e003e00:
            case 0x7e004000:  case 0x7e004200:  case 0x7e004400:  case 0x7e004600:
            case 0x7e004800:  case 0x7e004a00:  case 0x7e004e00:  case 0x7e005400:
            case 0x7e005600:  case 0x7e005c00:  case 0x7e005e00:  case 0x7e006200:
            case 0x7e006600:  case 0x7e006800:  case 0x7e006a00:  case 0x7e006c00:
            case 0x7e006e00:  case 0x7e007000:  case 0x7e007200:  case 0x7e007400:
            case 0x7e007600:  case 0x7e007800:  case 0x7e007a00:  case 0x7e007c00:
            case 0x7e007e00:  case 0x7e008000:  case 0x7e008400:  case 0x7e009400:
            case 0x7e009600:  case 0x7e00a000:  case 0x7e00a200:  case 0x7e00a400:
            case 0x7e00a600:  case 0x7e00a800:  case 0x7e00aa00:  case 0x7e00ac00:
            case 0x7e00ae00:  case 0x7e00b000:  case 0x7e00b200:  case 0x7e00b400:
            case 0x7e00b600:  case 0x7e00b800:  case 0x7e00ba00:  case 0x7e00bc00:
            case 0x7e00be00:  case 0x7e00c000:  case 0x7e00c200:  case 0x7e00c400:
            case 0x7e00c600:  case 0x7e00c800:  case 0x7e00d200:  case 0x7e00d400:
            case 0x7e00d600:  case 0x7e00d800:  case 0x7e00da00:  case 0x7e00dc00:
            case 0x7e00de00:  case 0x7e00e400:  case 0x7e00e600:  case 0x7e00e800:
            case 0x7e00ea00:  case 0x7e00ec00:  case 0x7e00ee00:  case 0x7e00f000:
            case 0x7e00f200:  case 0x7e00f400:  case 0x7e00f600:  case 0x7e00f800:
            case 0x7e00fa00:  case 0x7e00fc00:  case 0x7e00fe00:
                return true;

            default:
                return false;
        }
    }

    bool InstructionDecoder_amdgpu_gfx1250::IS_VOP1_VOP_DPP8(uint64_t I)
    {
        switch ( I & 0xfe00fe00 )  {
            case 0x7e000200:  case 0x7e000a00:  case 0x7e000c00:  case 0x7e000e00:
            case 0x7e001000:  case 0x7e001400:  case 0x7e001600:  case 0x7e001800:
            case 0x7e001a00:  case 0x7e001c00:  case 0x7e002200:  case 0x7e002400:
            case 0x7e002600:  case 0x7e002800:  case 0x7e003800:  case 0x7e003c00:
            case 0x7e003e00:  case 0x7e004000:  case 0x7e004200:  case 0x7e004400:
            case 0x7e004600:  case 0x7e004800:  case 0x7e004a00:  case 0x7e004e00:
            case 0x7e005400:  case 0x7e005600:  case 0x7e005c00:  case 0x7e006600:
            case 0x7e006a00:  case 0x7e006c00:  case 0x7e006e00:  case 0x7e007000:
            case 0x7e007200:  case 0x7e007400:  case 0x7e007600:  case 0x7e007e00:
            case 0x7e008000:  case 0x7e008400:  case 0x7e008600:  case 0x7e008800:
            case 0x7e009000:  case 0x7e009400:  case 0x7e009600:  case 0x7e00a000:
            case 0x7e00a200:  case 0x7e00a400:  case 0x7e00a600:  case 0x7e00a800:
            case 0x7e00aa00:  case 0x7e00ac00:  case 0x7e00ae00:  case 0x7e00b000:
            case 0x7e00b200:  case 0x7e00b400:  case 0x7e00b600:  case 0x7e00b800:
            case 0x7e00ba00:  case 0x7e00bc00:  case 0x7e00be00:  case 0x7e00c000:
            case 0x7e00c200:  case 0x7e00c400:  case 0x7e00c600:  case 0x7e00c800:
            case 0x7e00d200:  case 0x7e00d400:  case 0x7e00d600:  case 0x7e00d800:
            case 0x7e00da00:  case 0x7e00e400:  case 0x7e00e600:  case 0x7e00e800:
            case 0x7e00ea00:  case 0x7e00ec00:  case 0x7e00ee00:  case 0x7e00f000:
            case 0x7e00f200:  case 0x7e00f400:  case 0x7e00f600:  case 0x7e00f800:
            case 0x7e00fa00:  case 0x7e00fc00:  case 0x7e00fe00:
                return true;

            default:
                return false;
        }
    }

    bool InstructionDecoder_amdgpu_gfx1250::IS_VOP1_VOP_DPP16(uint64_t I)
    {
        switch ( I & 0xfe00fe00 )  {
            case 0x7e000200:  case 0x7e000600:  case 0x7e000800:  case 0x7e000a00:
            case 0x7e000c00:  case 0x7e000e00:  case 0x7e001000:  case 0x7e001400:
            case 0x7e001600:  case 0x7e001800:  case 0x7e001a00:  case 0x7e001c00:
            case 0x7e001e00:  case 0x7e002000:  case 0x7e002200:  case 0x7e002400:
            case 0x7e002600:  case 0x7e002800:  case 0x7e002a00:  case 0x7e002c00:
            case 0x7e002e00:  case 0x7e003000:  case 0x7e003200:  case 0x7e003400:
            case 0x7e003800:  case 0x7e003a00:  case 0x7e003c00:  case 0x7e003e00:
            case 0x7e004000:  case 0x7e004200:  case 0x7e004400:  case 0x7e004600:
            case 0x7e004800:  case 0x7e004a00:  case 0x7e004e00:  case 0x7e005400:
            case 0x7e005600:  case 0x7e005c00:  case 0x7e006600:  case 0x7e006a00:
            case 0x7e006c00:  case 0x7e006e00:  case 0x7e007000:  case 0x7e007200:
            case 0x7e007400:  case 0x7e007600:  case 0x7e007800:  case 0x7e007a00:
            case 0x7e007c00:  case 0x7e007e00:  case 0x7e008000:  case 0x7e008400:
            case 0x7e008600:  case 0x7e008800:  case 0x7e009000:  case 0x7e009400:
            case 0x7e009600:  case 0x7e00a000:  case 0x7e00a200:  case 0x7e00a400:
            case 0x7e00a600:  case 0x7e00a800:  case 0x7e00aa00:  case 0x7e00ac00:
            case 0x7e00ae00:  case 0x7e00b000:  case 0x7e00b200:  case 0x7e00b400:
            case 0x7e00b600:  case 0x7e00b800:  case 0x7e00ba00:  case 0x7e00bc00:
            case 0x7e00be00:  case 0x7e00c000:  case 0x7e00c200:  case 0x7e00c400:
            case 0x7e00c600:  case 0x7e00c800:  case 0x7e00d200:  case 0x7e00d400:
            case 0x7e00d600:  case 0x7e00d800:  case 0x7e00da00:  case 0x7e00e400:
            case 0x7e00e600:  case 0x7e00e800:  case 0x7e00ea00:  case 0x7e00ec00:
            case 0x7e00ee00:  case 0x7e00f000:  case 0x7e00f200:  case 0x7e00f400:
            case 0x7e00f600:  case 0x7e00f800:  case 0x7e00fa00:  case 0x7e00fc00:
            case 0x7e00fe00:
                return true;

            default:
                return false;
        }
    }

    bool InstructionDecoder_amdgpu_gfx1250::IS_VOP3_INST_LITERAL(uint64_t I)
    {
        switch ( I & 0xffff0000 )  {
            case 0xd5810000:  case 0xd5830000:  case 0xd5840000:  case 0xd5850000:
            case 0xd5860000:  case 0xd5870000:  case 0xd5880000:  case 0xd58a0000:
            case 0xd58b0000:  case 0xd58c0000:  case 0xd58d0000:  case 0xd58e0000:
            case 0xd58f0000:  case 0xd5900000:  case 0xd5910000:  case 0xd5920000:
            case 0xd5930000:  case 0xd5940000:  case 0xd5950000:  case 0xd5960000:
            case 0xd5970000:  case 0xd5980000:  case 0xd5990000:  case 0xd59a0000:
            case 0xd59c0000:  case 0xd59d0000:  case 0xd59e0000:  case 0xd59f0000:
            case 0xd5a00000:  case 0xd5a10000:  case 0xd5a20000:  case 0xd5a30000:
            case 0xd5a40000:  case 0xd5a50000:  case 0xd5a70000:  case 0xd5aa0000:
            case 0xd5ab0000:  case 0xd5ae0000:  case 0xd5af0000:  case 0xd5b10000:
            case 0xd5b30000:  case 0xd5b40000:  case 0xd5b50000:  case 0xd5b60000:
            case 0xd5b70000:  case 0xd5b80000:  case 0xd5b90000:  case 0xd5ba0000:
            case 0xd5bb0000:  case 0xd5bc0000:  case 0xd5bd0000:  case 0xd5be0000:
            case 0xd5bf0000:  case 0xd5c00000:  case 0xd5c20000:  case 0xd5ca0000:
            case 0xd5cb0000:  case 0xd5d00000:  case 0xd5d10000:  case 0xd5d20000:
            case 0xd5d30000:  case 0xd5d40000:  case 0xd5d50000:  case 0xd5d60000:
            case 0xd5d70000:  case 0xd5d80000:  case 0xd5d90000:  case 0xd5da0000:
            case 0xd5db0000:  case 0xd5dc0000:  case 0xd5dd0000:  case 0xd5de0000:
            case 0xd5df0000:  case 0xd5e00000:  case 0xd5e10000:  case 0xd5e20000:
            case 0xd5e30000:  case 0xd5e40000:  case 0xd5e90000:  case 0xd5ea0000:
            case 0xd5eb0000:  case 0xd5ec0000:  case 0xd5ed0000:  case 0xd5ee0000:
            case 0xd5ef0000:  case 0xd5f20000:  case 0xd5f30000:  case 0xd5f40000:
            case 0xd5f50000:  case 0xd5f60000:  case 0xd5f70000:  case 0xd5f80000:
            case 0xd5f90000:  case 0xd5fa0000:  case 0xd5fb0000:  case 0xd5fc0000:
            case 0xd5fd0000:  case 0xd5fe0000:  case 0xd5ff0000:  case 0xd5010000:
            case 0xd5020000:  case 0xd5030000:  case 0xd5040000:  case 0xd5050000:
            case 0xd5060000:  case 0xd5070000:  case 0xd5080000:  case 0xd5090000:
            case 0xd50a0000:  case 0xd50b0000:  case 0xd50c0000:  case 0xd50d0000:
            case 0xd50e0000:  case 0xd5110000:  case 0xd5120000:  case 0xd5130000:
            case 0xd5140000:  case 0xd5150000:  case 0xd5160000:  case 0xd5170000:
            case 0xd5180000:  case 0xd5190000:  case 0xd51a0000:  case 0xd51b0000:
            case 0xd51c0000:  case 0xd51d0000:  case 0xd51e0000:  case 0xd51f0000:
            case 0xd5250000:  case 0xd5260000:  case 0xd5270000:  case 0xd5280000:
            case 0xd5290000:  case 0xd52a0000:  case 0xd52b0000:  case 0xd52f0000:
            case 0xd5300000:  case 0xd5310000:  case 0xd5320000:  case 0xd5330000:
            case 0xd5340000:  case 0xd5350000:  case 0xd5360000:  case 0xd53b0000:
            case 0xd6090000:  case 0xd60a0000:  case 0xd60b0000:  case 0xd60c0000:
            case 0xd60d0000:  case 0xd60e0000:  case 0xd60f0000:  case 0xd6100000:
            case 0xd6110000:  case 0xd6120000:  case 0xd6130000:  case 0xd6140000:
            case 0xd6150000:  case 0xd6160000:  case 0xd6170000:  case 0xd6180000:
            case 0xd61a0000:  case 0xd61b0000:  case 0xd61d0000:  case 0xd61e0000:
            case 0xd6200000:  case 0xd6210000:  case 0xd6220000:  case 0xd6230000:
            case 0xd6240000:  case 0xd6250000:  case 0xd6260000:  case 0xd6270000:
            case 0xd6280000:  case 0xd6290000:  case 0xd62a0000:  case 0xd62b0000:
            case 0xd62c0000:  case 0xd62d0000:  case 0xd62e0000:  case 0xd62f0000:
            case 0xd6300000:  case 0xd6310000:  case 0xd6320000:  case 0xd6330000:
            case 0xd6340000:  case 0xd6350000:  case 0xd6370000:  case 0xd6380000:
            case 0xd6390000:  case 0xd63a0000:  case 0xd63b0000:  case 0xd63d0000:
            case 0xd63f0000:  case 0xd6400000:  case 0xd6410000:  case 0xd6420000:
            case 0xd6430000:  case 0xd6440000:  case 0xd6450000:  case 0xd6460000:
            case 0xd6470000:  case 0xd6480000:  case 0xd64a0000:  case 0xd64b0000:
            case 0xd64d0000:  case 0xd64e0000:  case 0xd6500000:  case 0xd6510000:
            case 0xd6520000:  case 0xd6530000:  case 0xd6540000:  case 0xd6550000:
            case 0xd6560000:  case 0xd6570000:  case 0xd6580000:  case 0xd6590000:
            case 0xd65a0000:  case 0xd65b0000:  case 0xd65c0000:  case 0xd65d0000:
            case 0xd65e0000:  case 0xd65f0000:  case 0xd6600000:  case 0xd6610000:
            case 0xd6620000:  case 0xd6630000:  case 0xd6640000:  case 0xd6650000:
            case 0xd6680000:  case 0xd6690000:  case 0xd66a0000:  case 0xd66b0000:
            case 0xd66c0000:  case 0xd66d0000:  case 0xd66e0000:  case 0xd66f0000:
            case 0xd6700000:  case 0xd6710000:  case 0xd6720000:  case 0xd6730000:
            case 0xd6800000:  case 0xd6810000:  case 0xd6820000:  case 0xd6830000:
            case 0xd6840000:  case 0xd6850000:  case 0xd6860000:  case 0xd6870000:
            case 0xd6880000:  case 0xd6890000:  case 0xd6900000:  case 0xd6910000:
            case 0xd6970000:  case 0xd6980000:  case 0xd6990000:  case 0xd69f0000:
            case 0xd6a00000:  case 0xd6a10000:  case 0xd6a80000:  case 0xd6a90000:
            case 0xd6aa0000:  case 0xd6ab0000:  case 0xd6ac0000:  case 0xd6ad0000:
            case 0xd6b00000:  case 0xd6b30000:  case 0xd6b40000:  case 0xd6b50000:
            case 0xd6b80000:  case 0xd6b90000:  case 0xd6bc0000:  case 0xd6bf0000:
            case 0xd6c00000:  case 0xd6c10000:  case 0xd6c20000:  case 0xd6c30000:
            case 0xd6c40000:  case 0xd6c50000:  case 0xd6c60000:  case 0xd6c70000:
            case 0xd6c80000:  case 0xd6c90000:  case 0xd6ca0000:  case 0xd6cb0000:
            case 0xd6cc0000:  case 0xd6cd0000:  case 0xd6ce0000:  case 0xd6cf0000:
            case 0xd6d00000:  case 0xd6d10000:  case 0xd6d20000:  case 0xd6d30000:
            case 0xd6d40000:  case 0xd6d50000:  case 0xd6d60000:  case 0xd6d70000:
            case 0xd6d80000:  case 0xd6fa0000:  case 0xd6fb0000:  case 0xd7030000:
            case 0xd7040000:  case 0xd7050000:  case 0xd7060000:  case 0xd7070000:
            case 0xd7090000:  case 0xd70a0000:  case 0xd70b0000:  case 0xd70c0000:
            case 0xd70d0000:  case 0xd70e0000:  case 0xd7110000:  case 0xd7120000:
            case 0xd7130000:  case 0xd7140000:  case 0xd7180000:  case 0xd7190000:
            case 0xd71a0000:  case 0xd71b0000:  case 0xd71c0000:  case 0xd71d0000:
            case 0xd71e0000:  case 0xd71f0000:  case 0xd7200000:  case 0xd7210000:
            case 0xd7220000:  case 0xd7230000:  case 0xd7240000:  case 0xd7250000:
            case 0xd7260000:  case 0xd72b0000:  case 0xd72c0000:  case 0xd72d0000:
            case 0xd72e0000:  case 0xd72f0000:  case 0xd7380000:  case 0xd7390000:
            case 0xd73a0000:  case 0xd73d0000:  case 0xd73e0000:  case 0xd7410000:
            case 0xd7420000:  case 0xd7610000:  case 0xd7620000:  case 0xd7630000:
            case 0xd7640000:  case 0xd7650000:  case 0xd7660000:  case 0xd7670000:
            case 0xd7680000:  case 0xd7690000:  case 0xd76a0000:  case 0xd76b0000:
            case 0xd76c0000:  case 0xd76d0000:  case 0xd76e0000:  case 0xd76f0000:
            case 0xd7700000:  case 0xd7720000:  case 0xd7730000:  case 0xd7740000:
            case 0xd7750000:  case 0xd4010000:  case 0xd4020000:  case 0xd4030000:
            case 0xd4040000:  case 0xd4050000:  case 0xd4060000:  case 0xd4070000:
            case 0xd4080000:  case 0xd4090000:  case 0xd40a0000:  case 0xd40b0000:
            case 0xd40c0000:  case 0xd40d0000:  case 0xd40e0000:  case 0xd4110000:
            case 0xd4120000:  case 0xd4130000:  case 0xd4140000:  case 0xd4150000:
            case 0xd4160000:  case 0xd4170000:  case 0xd4180000:  case 0xd4190000:
            case 0xd41a0000:  case 0xd41b0000:  case 0xd41c0000:  case 0xd41d0000:
            case 0xd41e0000:  case 0xd4210000:  case 0xd4220000:  case 0xd4230000:
            case 0xd4240000:  case 0xd4250000:  case 0xd4260000:  case 0xd4270000:
            case 0xd4280000:  case 0xd4290000:  case 0xd42a0000:  case 0xd42b0000:
            case 0xd42c0000:  case 0xd42d0000:  case 0xd42e0000:  case 0xd4310000:
            case 0xd4320000:  case 0xd4330000:  case 0xd4340000:  case 0xd4350000:
            case 0xd4360000:  case 0xd4390000:  case 0xd43a0000:  case 0xd43b0000:
            case 0xd43c0000:  case 0xd43d0000:  case 0xd43e0000:  case 0xd4410000:
            case 0xd4420000:  case 0xd4430000:  case 0xd4440000:  case 0xd4450000:
            case 0xd4460000:  case 0xd4490000:  case 0xd44a0000:  case 0xd44b0000:
            case 0xd44c0000:  case 0xd44d0000:  case 0xd44e0000:  case 0xd4510000:
            case 0xd4520000:  case 0xd4530000:  case 0xd4540000:  case 0xd4550000:
            case 0xd4560000:  case 0xd4590000:  case 0xd45a0000:  case 0xd45b0000:
            case 0xd45c0000:  case 0xd45d0000:  case 0xd45e0000:  case 0xd47d0000:
            case 0xd47e0000:  case 0xd47f0000:  case 0xd4810000:  case 0xd4820000:
            case 0xd4830000:  case 0xd4840000:  case 0xd4850000:  case 0xd4860000:
            case 0xd4870000:  case 0xd4880000:  case 0xd4890000:  case 0xd48a0000:
            case 0xd48b0000:  case 0xd48c0000:  case 0xd48d0000:  case 0xd48e0000:
            case 0xd4910000:  case 0xd4920000:  case 0xd4930000:  case 0xd4940000:
            case 0xd4950000:  case 0xd4960000:  case 0xd4970000:  case 0xd4980000:
            case 0xd4990000:  case 0xd49a0000:  case 0xd49b0000:  case 0xd49c0000:
            case 0xd49d0000:  case 0xd49e0000:  case 0xd4a10000:  case 0xd4a20000:
            case 0xd4a30000:  case 0xd4a40000:  case 0xd4a50000:  case 0xd4a60000:
            case 0xd4a70000:  case 0xd4a80000:  case 0xd4a90000:  case 0xd4aa0000:
            case 0xd4ab0000:  case 0xd4ac0000:  case 0xd4ad0000:  case 0xd4ae0000:
            case 0xd4b10000:  case 0xd4b20000:  case 0xd4b30000:  case 0xd4b40000:
            case 0xd4b50000:  case 0xd4b60000:  case 0xd4b90000:  case 0xd4ba0000:
            case 0xd4bb0000:  case 0xd4bc0000:  case 0xd4bd0000:  case 0xd4be0000:
            case 0xd4c10000:  case 0xd4c20000:  case 0xd4c30000:  case 0xd4c40000:
            case 0xd4c50000:  case 0xd4c60000:  case 0xd4c90000:  case 0xd4ca0000:
            case 0xd4cb0000:  case 0xd4cc0000:  case 0xd4cd0000:  case 0xd4ce0000:
            case 0xd4d10000:  case 0xd4d20000:  case 0xd4d30000:  case 0xd4d40000:
            case 0xd4d50000:  case 0xd4d60000:  case 0xd4d90000:  case 0xd4da0000:
            case 0xd4db0000:  case 0xd4dc0000:  case 0xd4dd0000:  case 0xd4de0000:
            case 0xd4fd0000:  case 0xd4fe0000:  case 0xd4ff0000:
                return true;

            default:
                return false;
        }
    }

    bool InstructionDecoder_amdgpu_gfx1250::IS_VOP3_VOP_DPP8(uint64_t I)
    {
        switch ( I & 0xffff0000 )  {
            case 0xd5810000:  case 0xd5850000:  case 0xd5860000:  case 0xd5870000:
            case 0xd5880000:  case 0xd58a0000:  case 0xd58b0000:  case 0xd58c0000:
            case 0xd58d0000:  case 0xd58e0000:  case 0xd5910000:  case 0xd5920000:
            case 0xd5930000:  case 0xd5940000:  case 0xd59c0000:  case 0xd59e0000:
            case 0xd59f0000:  case 0xd5a00000:  case 0xd5a10000:  case 0xd5a20000:
            case 0xd5a30000:  case 0xd5a40000:  case 0xd5a50000:  case 0xd5a70000:
            case 0xd5aa0000:  case 0xd5ab0000:  case 0xd5ae0000:  case 0xd5b30000:
            case 0xd5b50000:  case 0xd5b60000:  case 0xd5b70000:  case 0xd5b80000:
            case 0xd5b90000:  case 0xd5ba0000:  case 0xd5bb0000:  case 0xd5bf0000:
            case 0xd5c00000:  case 0xd5c20000:  case 0xd5c30000:  case 0xd5c40000:
            case 0xd5c80000:  case 0xd5ca0000:  case 0xd5cb0000:  case 0xd5d00000:
            case 0xd5d10000:  case 0xd5d20000:  case 0xd5d30000:  case 0xd5d40000:
            case 0xd5d50000:  case 0xd5d60000:  case 0xd5d70000:  case 0xd5d80000:
            case 0xd5d90000:  case 0xd5da0000:  case 0xd5db0000:  case 0xd5dc0000:
            case 0xd5dd0000:  case 0xd5de0000:  case 0xd5df0000:  case 0xd5e00000:
            case 0xd5e10000:  case 0xd5e20000:  case 0xd5e30000:  case 0xd5e40000:
            case 0xd5e90000:  case 0xd5ea0000:  case 0xd5eb0000:  case 0xd5ec0000:
            case 0xd5ed0000:  case 0xd5f20000:  case 0xd5f30000:  case 0xd5f40000:
            case 0xd5f50000:  case 0xd5f60000:  case 0xd5f70000:  case 0xd5f80000:
            case 0xd5f90000:  case 0xd5fa0000:  case 0xd5fb0000:  case 0xd5fc0000:
            case 0xd5fd0000:  case 0xd5fe0000:  case 0xd5ff0000:  case 0xd5010000:
            case 0xd5030000:  case 0xd5040000:  case 0xd5050000:  case 0xd5070000:
            case 0xd5080000:  case 0xd5090000:  case 0xd50a0000:  case 0xd50b0000:
            case 0xd50c0000:  case 0xd5110000:  case 0xd5120000:  case 0xd5130000:
            case 0xd5140000:  case 0xd5150000:  case 0xd5160000:  case 0xd5180000:
            case 0xd5190000:  case 0xd51a0000:  case 0xd51b0000:  case 0xd51c0000:
            case 0xd51d0000:  case 0xd51e0000:  case 0xd5250000:  case 0xd5260000:
            case 0xd5270000:  case 0xd52b0000:  case 0xd52f0000:  case 0xd5300000:
            case 0xd5310000:  case 0xd5320000:  case 0xd5330000:  case 0xd5340000:
            case 0xd5350000:  case 0xd5360000:  case 0xd53b0000:  case 0xd6090000:
            case 0xd60a0000:  case 0xd60b0000:  case 0xd60c0000:  case 0xd60d0000:
            case 0xd60e0000:  case 0xd60f0000:  case 0xd6100000:  case 0xd6110000:
            case 0xd6120000:  case 0xd6130000:  case 0xd6150000:  case 0xd6160000:
            case 0xd6170000:  case 0xd6180000:  case 0xd61a0000:  case 0xd61b0000:
            case 0xd61d0000:  case 0xd61e0000:  case 0xd6200000:  case 0xd6210000:
            case 0xd6220000:  case 0xd6230000:  case 0xd6240000:  case 0xd6250000:
            case 0xd6260000:  case 0xd6290000:  case 0xd62a0000:  case 0xd62b0000:
            case 0xd62c0000:  case 0xd62d0000:  case 0xd62e0000:  case 0xd62f0000:
            case 0xd6300000:  case 0xd6310000:  case 0xd6320000:  case 0xd6330000:
            case 0xd6340000:  case 0xd6390000:  case 0xd6400000:  case 0xd6410000:
            case 0xd6440000:  case 0xd6450000:  case 0xd6460000:  case 0xd6470000:
            case 0xd6480000:  case 0xd64a0000:  case 0xd64b0000:  case 0xd64d0000:
            case 0xd64e0000:  case 0xd6500000:  case 0xd6510000:  case 0xd6530000:
            case 0xd6540000:  case 0xd6550000:  case 0xd6560000:  case 0xd6570000:
            case 0xd6580000:  case 0xd6590000:  case 0xd65a0000:  case 0xd65d0000:
            case 0xd65e0000:  case 0xd65f0000:  case 0xd6600000:  case 0xd6610000:
            case 0xd6620000:  case 0xd6630000:  case 0xd6640000:  case 0xd6650000:
            case 0xd6680000:  case 0xd6690000:  case 0xd66a0000:  case 0xd66b0000:
            case 0xd66c0000:  case 0xd66d0000:  case 0xd66e0000:  case 0xd66f0000:
            case 0xd6900000:  case 0xd6910000:  case 0xd7030000:  case 0xd7040000:
            case 0xd7050000:  case 0xd7060000:  case 0xd7070000:  case 0xd7090000:
            case 0xd70a0000:  case 0xd70b0000:  case 0xd70c0000:  case 0xd70d0000:
            case 0xd70e0000:  case 0xd7110000:  case 0xd7120000:  case 0xd7130000:
            case 0xd71c0000:  case 0xd71d0000:  case 0xd71e0000:  case 0xd71f0000:
            case 0xd7200000:  case 0xd7210000:  case 0xd7220000:  case 0xd7230000:
            case 0xd7240000:  case 0xd7250000:  case 0xd7260000:  case 0xd7380000:
            case 0xd7390000:  case 0xd73a0000:  case 0xd7620000:  case 0xd7630000:
            case 0xd7640000:  case 0xd7650000:  case 0xd7660000:  case 0xd7670000:
            case 0xd7680000:  case 0xd7690000:  case 0xd76a0000:  case 0xd76b0000:
            case 0xd76c0000:  case 0xd76d0000:  case 0xd76e0000:  case 0xd76f0000:
            case 0xd7700000:  case 0xd7720000:  case 0xd7730000:  case 0xd7740000:
            case 0xd7750000:  case 0xd4010000:  case 0xd4020000:  case 0xd4030000:
            case 0xd4040000:  case 0xd4050000:  case 0xd4060000:  case 0xd4070000:
            case 0xd4080000:  case 0xd4090000:  case 0xd40a0000:  case 0xd40b0000:
            case 0xd40c0000:  case 0xd40d0000:  case 0xd40e0000:  case 0xd4110000:
            case 0xd4120000:  case 0xd4130000:  case 0xd4140000:  case 0xd4150000:
            case 0xd4160000:  case 0xd4170000:  case 0xd4180000:  case 0xd4190000:
            case 0xd41a0000:  case 0xd41b0000:  case 0xd41c0000:  case 0xd41d0000:
            case 0xd41e0000:  case 0xd4310000:  case 0xd4320000:  case 0xd4330000:
            case 0xd4340000:  case 0xd4350000:  case 0xd4360000:  case 0xd4390000:
            case 0xd43a0000:  case 0xd43b0000:  case 0xd43c0000:  case 0xd43d0000:
            case 0xd43e0000:  case 0xd4410000:  case 0xd4420000:  case 0xd4430000:
            case 0xd4440000:  case 0xd4450000:  case 0xd4460000:  case 0xd4490000:
            case 0xd44a0000:  case 0xd44b0000:  case 0xd44c0000:  case 0xd44d0000:
            case 0xd44e0000:  case 0xd47d0000:  case 0xd47e0000:  case 0xd4810000:
            case 0xd4820000:  case 0xd4830000:  case 0xd4840000:  case 0xd4850000:
            case 0xd4860000:  case 0xd4870000:  case 0xd4880000:  case 0xd4890000:
            case 0xd48a0000:  case 0xd48b0000:  case 0xd48c0000:  case 0xd48d0000:
            case 0xd48e0000:  case 0xd4910000:  case 0xd4920000:  case 0xd4930000:
            case 0xd4940000:  case 0xd4950000:  case 0xd4960000:  case 0xd4970000:
            case 0xd4980000:  case 0xd4990000:  case 0xd49a0000:  case 0xd49b0000:
            case 0xd49c0000:  case 0xd49d0000:  case 0xd49e0000:  case 0xd4b10000:
            case 0xd4b20000:  case 0xd4b30000:  case 0xd4b40000:  case 0xd4b50000:
            case 0xd4b60000:  case 0xd4b90000:  case 0xd4ba0000:  case 0xd4bb0000:
            case 0xd4bc0000:  case 0xd4bd0000:  case 0xd4be0000:  case 0xd4c10000:
            case 0xd4c20000:  case 0xd4c30000:  case 0xd4c40000:  case 0xd4c50000:
            case 0xd4c60000:  case 0xd4c90000:  case 0xd4ca0000:  case 0xd4cb0000:
            case 0xd4cc0000:  case 0xd4cd0000:  case 0xd4ce0000:  case 0xd4fd0000:
            case 0xd4fe0000:
                return true;

            default:
                return false;
        }
    }

    bool InstructionDecoder_amdgpu_gfx1250::IS_VOP3_VOP_DPP16(uint64_t I)
    {
        switch ( I & 0xffff0000 )  {
            case 0xd5810000:  case 0xd5830000:  case 0xd5840000:  case 0xd5850000:
            case 0xd5860000:  case 0xd5870000:  case 0xd5880000:  case 0xd58a0000:
            case 0xd58b0000:  case 0xd58c0000:  case 0xd58d0000:  case 0xd58e0000:
            case 0xd58f0000:  case 0xd5900000:  case 0xd5910000:  case 0xd5920000:
            case 0xd5930000:  case 0xd5940000:  case 0xd5950000:  case 0xd5960000:
            case 0xd5970000:  case 0xd5980000:  case 0xd5990000:  case 0xd59a0000:
            case 0xd59c0000:  case 0xd59d0000:  case 0xd59e0000:  case 0xd59f0000:
            case 0xd5a00000:  case 0xd5a10000:  case 0xd5a20000:  case 0xd5a30000:
            case 0xd5a40000:  case 0xd5a50000:  case 0xd5a70000:  case 0xd5aa0000:
            case 0xd5ab0000:  case 0xd5ae0000:  case 0xd5b30000:  case 0xd5b50000:
            case 0xd5b60000:  case 0xd5b70000:  case 0xd5b80000:  case 0xd5b90000:
            case 0xd5ba0000:  case 0xd5bb0000:  case 0xd5bc0000:  case 0xd5bd0000:
            case 0xd5be0000:  case 0xd5bf0000:  case 0xd5c00000:  case 0xd5c20000:
            case 0xd5c30000:  case 0xd5c40000:  case 0xd5c80000:  case 0xd5ca0000:
            case 0xd5cb0000:  case 0xd5d00000:  case 0xd5d10000:  case 0xd5d20000:
            case 0xd5d30000:  case 0xd5d40000:  case 0xd5d50000:  case 0xd5d60000:
            case 0xd5d70000:  case 0xd5d80000:  case 0xd5d90000:  case 0xd5da0000:
            case 0xd5db0000:  case 0xd5dc0000:  case 0xd5dd0000:  case 0xd5de0000:
            case 0xd5df0000:  case 0xd5e00000:  case 0xd5e10000:  case 0xd5e20000:
            case 0xd5e30000:  case 0xd5e40000:  case 0xd5e90000:  case 0xd5ea0000:
            case 0xd5eb0000:  case 0xd5ec0000:  case 0xd5ed0000:  case 0xd5f20000:
            case 0xd5f30000:  case 0xd5f40000:  case 0xd5f50000:  case 0xd5f60000:
            case 0xd5f70000:  case 0xd5f80000:  case 0xd5f90000:  case 0xd5fa0000:
            case 0xd5fb0000:  case 0xd5fc0000:  case 0xd5fd0000:  case 0xd5fe0000:
            case 0xd5ff0000:  case 0xd5010000:  case 0xd5020000:  case 0xd5030000:
            case 0xd5040000:  case 0xd5050000:  case 0xd5060000:  case 0xd5070000:
            case 0xd5080000:  case 0xd5090000:  case 0xd50a0000:  case 0xd50b0000:
            case 0xd50c0000:  case 0xd50d0000:  case 0xd50e0000:  case 0xd5110000:
            case 0xd5120000:  case 0xd5130000:  case 0xd5140000:  case 0xd5150000:
            case 0xd5160000:  case 0xd5170000:  case 0xd5180000:  case 0xd5190000:
            case 0xd51a0000:  case 0xd51b0000:  case 0xd51c0000:  case 0xd51d0000:
            case 0xd51e0000:  case 0xd51f0000:  case 0xd5250000:  case 0xd5260000:
            case 0xd5270000:  case 0xd5280000:  case 0xd5290000:  case 0xd52b0000:
            case 0xd52f0000:  case 0xd5300000:  case 0xd5310000:  case 0xd5320000:
            case 0xd5330000:  case 0xd5340000:  case 0xd5350000:  case 0xd5360000:
            case 0xd53b0000:  case 0xd6090000:  case 0xd60a0000:  case 0xd60b0000:
            case 0xd60c0000:  case 0xd60d0000:  case 0xd60e0000:  case 0xd60f0000:
            case 0xd6100000:  case 0xd6110000:  case 0xd6120000:  case 0xd6130000:
            case 0xd6140000:  case 0xd6150000:  case 0xd6160000:  case 0xd6170000:
            case 0xd6180000:  case 0xd61a0000:  case 0xd61b0000:  case 0xd61d0000:
            case 0xd61e0000:  case 0xd6200000:  case 0xd6210000:  case 0xd6220000:
            case 0xd6230000:  case 0xd6240000:  case 0xd6250000:  case 0xd6260000:
            case 0xd6280000:  case 0xd6290000:  case 0xd62a0000:  case 0xd62b0000:
            case 0xd62c0000:  case 0xd62d0000:  case 0xd62e0000:  case 0xd62f0000:
            case 0xd6300000:  case 0xd6310000:  case 0xd6320000:  case 0xd6330000:
            case 0xd6340000:  case 0xd6350000:  case 0xd6380000:  case 0xd6390000:
            case 0xd6400000:  case 0xd6410000:  case 0xd6440000:  case 0xd6450000:
            case 0xd6460000:  case 0xd6470000:  case 0xd6480000:  case 0xd64a0000:
            case 0xd64b0000:  case 0xd64d0000:  case 0xd64e0000:  case 0xd6500000:
            case 0xd6510000:  case 0xd6520000:  case 0xd6530000:  case 0xd6540000:
            case 0xd6550000:  case 0xd6560000:  case 0xd6570000:  case 0xd6580000:
            case 0xd6590000:  case 0xd65a0000:  case 0xd65d0000:  case 0xd65e0000:
            case 0xd65f0000:  case 0xd6600000:  case 0xd6610000:  case 0xd6620000:
            case 0xd6630000:  case 0xd6640000:  case 0xd6650000:  case 0xd6680000:
            case 0xd6690000:  case 0xd66a0000:  case 0xd66b0000:  case 0xd66c0000:
            case 0xd66d0000:  case 0xd66e0000:  case 0xd66f0000:  case 0xd6900000:
            case 0xd6910000:  case 0xd6fa0000:  case 0xd6fb0000:  case 0xd7030000:
            case 0xd7040000:  case 0xd7050000:  case 0xd7060000:  case 0xd7070000:
            case 0xd7090000:  case 0xd70a0000:  case 0xd70b0000:  case 0xd70c0000:
            case 0xd70d0000:  case 0xd70e0000:  case 0xd7110000:  case 0xd7120000:
            case 0xd7130000:  case 0xd7180000:  case 0xd7190000:  case 0xd71a0000:
            case 0xd71b0000:  case 0xd71c0000:  case 0xd71d0000:  case 0xd71e0000:
            case 0xd71f0000:  case 0xd7200000:  case 0xd7210000:  case 0xd7220000:
            case 0xd7230000:  case 0xd7240000:  case 0xd7250000:  case 0xd7260000:
            case 0xd72b0000:  case 0xd72c0000:  case 0xd72d0000:  case 0xd72e0000:
            case 0xd7380000:  case 0xd7390000:  case 0xd73a0000:  case 0xd73d0000:
            case 0xd73e0000:  case 0xd7410000:  case 0xd7420000:  case 0xd7620000:
            case 0xd7630000:  case 0xd7640000:  case 0xd7650000:  case 0xd7660000:
            case 0xd7670000:  case 0xd7680000:  case 0xd7690000:  case 0xd76a0000:
            case 0xd76b0000:  case 0xd76c0000:  case 0xd76d0000:  case 0xd76e0000:
            case 0xd76f0000:  case 0xd7700000:  case 0xd7720000:  case 0xd7730000:
            case 0xd7740000:  case 0xd7750000:  case 0xd4010000:  case 0xd4020000:
            case 0xd4030000:  case 0xd4040000:  case 0xd4050000:  case 0xd4060000:
            case 0xd4070000:  case 0xd4080000:  case 0xd4090000:  case 0xd40a0000:
            case 0xd40b0000:  case 0xd40c0000:  case 0xd40d0000:  case 0xd40e0000:
            case 0xd4110000:  case 0xd4120000:  case 0xd4130000:  case 0xd4140000:
            case 0xd4150000:  case 0xd4160000:  case 0xd4170000:  case 0xd4180000:
            case 0xd4190000:  case 0xd41a0000:  case 0xd41b0000:  case 0xd41c0000:
            case 0xd41d0000:  case 0xd41e0000:  case 0xd4310000:  case 0xd4320000:
            case 0xd4330000:  case 0xd4340000:  case 0xd4350000:  case 0xd4360000:
            case 0xd4390000:  case 0xd43a0000:  case 0xd43b0000:  case 0xd43c0000:
            case 0xd43d0000:  case 0xd43e0000:  case 0xd4410000:  case 0xd4420000:
            case 0xd4430000:  case 0xd4440000:  case 0xd4450000:  case 0xd4460000:
            case 0xd4490000:  case 0xd44a0000:  case 0xd44b0000:  case 0xd44c0000:
            case 0xd44d0000:  case 0xd44e0000:  case 0xd47d0000:  case 0xd47e0000:
            case 0xd4810000:  case 0xd4820000:  case 0xd4830000:  case 0xd4840000:
            case 0xd4850000:  case 0xd4860000:  case 0xd4870000:  case 0xd4880000:
            case 0xd4890000:  case 0xd48a0000:  case 0xd48b0000:  case 0xd48c0000:
            case 0xd48d0000:  case 0xd48e0000:  case 0xd4910000:  case 0xd4920000:
            case 0xd4930000:  case 0xd4940000:  case 0xd4950000:  case 0xd4960000:
            case 0xd4970000:  case 0xd4980000:  case 0xd4990000:  case 0xd49a0000:
            case 0xd49b0000:  case 0xd49c0000:  case 0xd49d0000:  case 0xd49e0000:
            case 0xd4b10000:  case 0xd4b20000:  case 0xd4b30000:  case 0xd4b40000:
            case 0xd4b50000:  case 0xd4b60000:  case 0xd4b90000:  case 0xd4ba0000:
            case 0xd4bb0000:  case 0xd4bc0000:  case 0xd4bd0000:  case 0xd4be0000:
            case 0xd4c10000:  case 0xd4c20000:  case 0xd4c30000:  case 0xd4c40000:
            case 0xd4c50000:  case 0xd4c60000:  case 0xd4c90000:  case 0xd4ca0000:
            case 0xd4cb0000:  case 0xd4cc0000:  case 0xd4cd0000:  case 0xd4ce0000:
            case 0xd4fd0000:  case 0xd4fe0000:
                return true;

            default:
                return false;
        }
    }

    bool InstructionDecoder_amdgpu_gfx1250::IS_VOP2_INST_LITERAL(uint64_t I)
    {
        switch ( I & 0xfe000000 )  {
            case 0x2000000:  case 0x4000000:  case 0x6000000:  case 0x8000000:
            case 0xa000000:  case 0xc000000:  case 0xe000000:  case 0x10000000:
            case 0x12000000:  case 0x14000000:  case 0x16000000:  case 0x18000000:
            case 0x1a000000:  case 0x1c000000:  case 0x22000000:  case 0x24000000:
            case 0x26000000:  case 0x28000000:  case 0x2a000000:  case 0x2c000000:
            case 0x2e000000:  case 0x30000000:  case 0x32000000:  case 0x34000000:
            case 0x36000000:  case 0x38000000:  case 0x3a000000:  case 0x3c000000:
            case 0x3e000000:  case 0x40000000:  case 0x42000000:  case 0x44000000:
            case 0x4a000000:  case 0x4c000000:  case 0x4e000000:  case 0x50000000:
            case 0x52000000:  case 0x54000000:  case 0x56000000:  case 0x58000000:
            case 0x5a000000:  case 0x5e000000:  case 0x60000000:  case 0x62000000:
            case 0x64000000:  case 0x66000000:  case 0x68000000:  case 0x6a000000:
            case 0x6c000000:  case 0x6e000000:  case 0x70000000:  case 0x76000000:
            case 0x78000000:
                return true;

            default:
                return false;
        }
    }

    bool InstructionDecoder_amdgpu_gfx1250::IS_VOP2_INST_LITERAL64(uint64_t I)
    {
        switch ( I & 0xfe000000 )  {
            case 0x2000000:  case 0x4000000:  case 0x6000000:  case 0x8000000:
            case 0xa000000:  case 0xc000000:  case 0xe000000:  case 0x10000000:
            case 0x12000000:  case 0x14000000:  case 0x16000000:  case 0x18000000:
            case 0x1a000000:  case 0x1c000000:  case 0x22000000:  case 0x24000000:
            case 0x26000000:  case 0x28000000:  case 0x2a000000:  case 0x2c000000:
            case 0x2e000000:  case 0x30000000:  case 0x32000000:  case 0x34000000:
            case 0x36000000:  case 0x38000000:  case 0x3a000000:  case 0x3c000000:
            case 0x3e000000:  case 0x40000000:  case 0x42000000:  case 0x44000000:
            case 0x46000000:  case 0x48000000:  case 0x4a000000:  case 0x4c000000:
            case 0x4e000000:  case 0x50000000:  case 0x52000000:  case 0x54000000:
            case 0x56000000:  case 0x5e000000:  case 0x60000000:  case 0x62000000:
            case 0x64000000:  case 0x66000000:  case 0x68000000:  case 0x6a000000:
            case 0x6c000000:  case 0x76000000:  case 0x78000000:
                return true;

            default:
                return false;
        }
    }

    bool InstructionDecoder_amdgpu_gfx1250::IS_VOP2_VOP_DPP8(uint64_t I)
    {
        switch ( I & 0xfe000000 )  {
            case 0x2000000:  case 0x6000000:  case 0x8000000:  case 0xa000000:
            case 0xe000000:  case 0x10000000:  case 0x12000000:  case 0x14000000:
            case 0x16000000:  case 0x18000000:  case 0x22000000:  case 0x24000000:
            case 0x26000000:  case 0x28000000:  case 0x2a000000:  case 0x2c000000:
            case 0x30000000:  case 0x32000000:  case 0x34000000:  case 0x36000000:
            case 0x38000000:  case 0x3a000000:  case 0x3c000000:  case 0x40000000:
            case 0x42000000:  case 0x44000000:  case 0x4a000000:  case 0x4c000000:
            case 0x4e000000:  case 0x56000000:  case 0x5e000000:  case 0x60000000:
            case 0x62000000:  case 0x64000000:  case 0x66000000:  case 0x68000000:
            case 0x6a000000:  case 0x6c000000:  case 0x76000000:
                return true;

            default:
                return false;
        }
    }

    bool InstructionDecoder_amdgpu_gfx1250::IS_VOP2_VOP_DPP16(uint64_t I)
    {
        switch ( I & 0xfe000000 )  {
            case 0x2000000:  case 0x4000000:  case 0x6000000:  case 0x8000000:
            case 0xa000000:  case 0xc000000:  case 0xe000000:  case 0x10000000:
            case 0x12000000:  case 0x14000000:  case 0x16000000:  case 0x18000000:
            case 0x1a000000:  case 0x1c000000:  case 0x22000000:  case 0x24000000:
            case 0x26000000:  case 0x28000000:  case 0x2a000000:  case 0x2c000000:
            case 0x2e000000:  case 0x30000000:  case 0x32000000:  case 0x34000000:
            case 0x36000000:  case 0x38000000:  case 0x3a000000:  case 0x3c000000:
            case 0x3e000000:  case 0x40000000:  case 0x42000000:  case 0x44000000:
            case 0x4a000000:  case 0x4c000000:  case 0x4e000000:  case 0x50000000:
            case 0x52000000:  case 0x56000000:  case 0x5e000000:  case 0x60000000:
            case 0x62000000:  case 0x64000000:  case 0x66000000:  case 0x68000000:
            case 0x6a000000:  case 0x6c000000:  case 0x76000000:
                return true;

            default:
                return false;
        }
    }

    bool InstructionDecoder_amdgpu_gfx1250::IS_VOP3_SDST_ENC(uint64_t I)
    {
        switch ( I & 0xffff0000 )  {
            case 0xd5200000:  case 0xd5210000:  case 0xd5220000:  case 0xd6fc0000:
            case 0xd6fd0000:  case 0xd6fe0000:  case 0xd6ff0000:  case 0xd7000000:
            case 0xd7010000:  case 0xd7020000:
                return true;

            default:
                return false;
        }
    }

    bool InstructionDecoder_amdgpu_gfx1250::IS_VOP3_SDST_ENC_INST_LITERAL(uint64_t I)
    {
        switch ( I & 0xffff0000 )  {
            case 0xd5200000:  case 0xd5210000:  case 0xd5220000:  case 0xd6fc0000:
            case 0xd6fd0000:  case 0xd6fe0000:  case 0xd6ff0000:  case 0xd7000000:
            case 0xd7010000:  case 0xd7020000:
                return true;

            default:
                return false;
        }
    }

    bool InstructionDecoder_amdgpu_gfx1250::IS_VOP3_SDST_ENC_VOP_DPP8(uint64_t I)
    {
        switch ( I & 0xffff0000 )  {
            case 0xd5200000:  case 0xd5210000:  case 0xd5220000:  case 0xd7000000:
            case 0xd7010000:  case 0xd7020000:
                return true;

            default:
                return false;
        }
    }

    bool InstructionDecoder_amdgpu_gfx1250::IS_VOP3_SDST_ENC_VOP_DPP16(uint64_t I)
    {
        switch ( I & 0xffff0000 )  {
            case 0xd5200000:  case 0xd5210000:  case 0xd5220000:  case 0xd6fd0000:
            case 0xd6fe0000:  case 0xd6ff0000:  case 0xd7000000:  case 0xd7010000:
            case 0xd7020000:
                return true;

            default:
                return false;
        }
    }

    bool InstructionDecoder_amdgpu_gfx1250::IS_VOP3P_INST_LITERAL(uint64_t I)
    {
        switch ( I & 0xffff0000 )  {
            case 0xcc000000:  case 0xcc010000:  case 0xcc020000:  case 0xcc030000:
            case 0xcc040000:  case 0xcc050000:  case 0xcc060000:  case 0xcc070000:
            case 0xcc080000:  case 0xcc090000:  case 0xcc0a0000:  case 0xcc0b0000:
            case 0xcc0c0000:  case 0xcc0d0000:  case 0xcc0e0000:  case 0xcc0f0000:
            case 0xcc100000:  case 0xcc110000:  case 0xcc140000:  case 0xcc150000:
            case 0xcc160000:  case 0xcc170000:  case 0xcc180000:  case 0xcc190000:
            case 0xcc1b0000:  case 0xcc1c0000:  case 0xcc1d0000:  case 0xcc1e0000:
            case 0xcc1f0000:  case 0xcc200000:  case 0xcc210000:  case 0xcc220000:
            case 0xcc230000:  case 0xcc280000:  case 0xcc290000:  case 0xcc2a0000:
            case 0xcc2b0000:  case 0xcc2c0000:  case 0xcc2d0000:  case 0xcc2e0000:
            case 0xcc2f0000:  case 0xcc300000:  case 0xcc310000:  case 0xcc320000:
            case 0xcc360000:  case 0xcc370000:  case 0xcc380000:  case 0xcc390000:
            case 0xcc3d0000:  case 0xcc3e0000:  case 0xcc3f0000:
                return true;

            default:
                return false;
        }
    }

    bool InstructionDecoder_amdgpu_gfx1250::IS_VOP3P_VOP_DPP8(uint64_t I)
    {
        switch ( I & 0xffff0000 )  {
            case 0xcc200000:  case 0xcc210000:  case 0xcc220000:  case 0xcc3d0000:
            case 0xcc3e0000:  case 0xcc3f0000:
                return true;

            default:
                return false;
        }
    }

    bool InstructionDecoder_amdgpu_gfx1250::IS_VOP3P_VOP_DPP16(uint64_t I)
    {
        switch ( I & 0xffff0000 )  {
            case 0xcc200000:  case 0xcc210000:  case 0xcc220000:  case 0xcc3d0000:
            case 0xcc3e0000:  case 0xcc3f0000:
                return true;

            default:
                return false;
        }
    }

    bool InstructionDecoder_amdgpu_gfx1250::IS_VOPC_INST_LITERAL(uint64_t I)
    {
        switch ( I & 0xfffe0000 )  {
            case 0x7c020000:  case 0x7c040000:  case 0x7c060000:  case 0x7c080000:
            case 0x7c0a0000:  case 0x7c0c0000:  case 0x7c0e0000:  case 0x7c100000:
            case 0x7c120000:  case 0x7c140000:  case 0x7c160000:  case 0x7c180000:
            case 0x7c1a0000:  case 0x7c1c0000:  case 0x7c220000:  case 0x7c240000:
            case 0x7c260000:  case 0x7c280000:  case 0x7c2a0000:  case 0x7c2c0000:
            case 0x7c2e0000:  case 0x7c300000:  case 0x7c320000:  case 0x7c340000:
            case 0x7c360000:  case 0x7c380000:  case 0x7c3a0000:  case 0x7c3c0000:
            case 0x7c420000:  case 0x7c440000:  case 0x7c460000:  case 0x7c480000:
            case 0x7c4a0000:  case 0x7c4c0000:  case 0x7c4e0000:  case 0x7c500000:
            case 0x7c520000:  case 0x7c540000:  case 0x7c560000:  case 0x7c580000:
            case 0x7c5a0000:  case 0x7c5c0000:  case 0x7c620000:  case 0x7c640000:
            case 0x7c660000:  case 0x7c680000:  case 0x7c6a0000:  case 0x7c6c0000:
            case 0x7c720000:  case 0x7c740000:  case 0x7c760000:  case 0x7c780000:
            case 0x7c7a0000:  case 0x7c7c0000:  case 0x7c820000:  case 0x7c840000:
            case 0x7c860000:  case 0x7c880000:  case 0x7c8a0000:  case 0x7c8c0000:
            case 0x7c920000:  case 0x7c940000:  case 0x7c960000:  case 0x7c980000:
            case 0x7c9a0000:  case 0x7c9c0000:  case 0x7ca20000:  case 0x7ca40000:
            case 0x7ca60000:  case 0x7ca80000:  case 0x7caa0000:  case 0x7cac0000:
            case 0x7cb20000:  case 0x7cb40000:  case 0x7cb60000:  case 0x7cb80000:
            case 0x7cba0000:  case 0x7cbc0000:  case 0x7cfa0000:  case 0x7cfc0000:
            case 0x7cfe0000:  case 0x7d020000:  case 0x7d040000:  case 0x7d060000:
            case 0x7d080000:  case 0x7d0a0000:  case 0x7d0c0000:  case 0x7d0e0000:
            case 0x7d100000:  case 0x7d120000:  case 0x7d140000:  case 0x7d160000:
            case 0x7d180000:  case 0x7d1a0000:  case 0x7d1c0000:  case 0x7d220000:
            case 0x7d240000:  case 0x7d260000:  case 0x7d280000:  case 0x7d2a0000:
            case 0x7d2c0000:  case 0x7d2e0000:  case 0x7d300000:  case 0x7d320000:
            case 0x7d340000:  case 0x7d360000:  case 0x7d380000:  case 0x7d3a0000:
            case 0x7d3c0000:  case 0x7d420000:  case 0x7d440000:  case 0x7d460000:
            case 0x7d480000:  case 0x7d4a0000:  case 0x7d4c0000:  case 0x7d4e0000:
            case 0x7d500000:  case 0x7d520000:  case 0x7d540000:  case 0x7d560000:
            case 0x7d580000:  case 0x7d5a0000:  case 0x7d5c0000:  case 0x7d620000:
            case 0x7d640000:  case 0x7d660000:  case 0x7d680000:  case 0x7d6a0000:
            case 0x7d6c0000:  case 0x7d720000:  case 0x7d740000:  case 0x7d760000:
            case 0x7d780000:  case 0x7d7a0000:  case 0x7d7c0000:  case 0x7d820000:
            case 0x7d840000:  case 0x7d860000:  case 0x7d880000:  case 0x7d8a0000:
            case 0x7d8c0000:  case 0x7d920000:  case 0x7d940000:  case 0x7d960000:
            case 0x7d980000:  case 0x7d9a0000:  case 0x7d9c0000:  case 0x7da20000:
            case 0x7da40000:  case 0x7da60000:  case 0x7da80000:  case 0x7daa0000:
            case 0x7dac0000:  case 0x7db20000:  case 0x7db40000:  case 0x7db60000:
            case 0x7db80000:  case 0x7dba0000:  case 0x7dbc0000:  case 0x7dfa0000:
            case 0x7dfc0000:  case 0x7dfe0000:
                return true;

            default:
                return false;
        }
    }

    bool InstructionDecoder_amdgpu_gfx1250::IS_VOPC_INST_LITERAL64(uint64_t I)
    {
        switch ( I & 0xfffe0000 )  {
            case 0x7c020000:  case 0x7c040000:  case 0x7c060000:  case 0x7c080000:
            case 0x7c0a0000:  case 0x7c0c0000:  case 0x7c0e0000:  case 0x7c100000:
            case 0x7c120000:  case 0x7c140000:  case 0x7c160000:  case 0x7c180000:
            case 0x7c1a0000:  case 0x7c1c0000:  case 0x7c220000:  case 0x7c240000:
            case 0x7c260000:  case 0x7c280000:  case 0x7c2a0000:  case 0x7c2c0000:
            case 0x7c2e0000:  case 0x7c300000:  case 0x7c320000:  case 0x7c340000:
            case 0x7c360000:  case 0x7c380000:  case 0x7c3a0000:  case 0x7c3c0000:
            case 0x7c420000:  case 0x7c440000:  case 0x7c460000:  case 0x7c480000:
            case 0x7c4a0000:  case 0x7c4c0000:  case 0x7c4e0000:  case 0x7c500000:
            case 0x7c520000:  case 0x7c540000:  case 0x7c560000:  case 0x7c580000:
            case 0x7c5a0000:  case 0x7c5c0000:  case 0x7c620000:  case 0x7c640000:
            case 0x7c660000:  case 0x7c680000:  case 0x7c6a0000:  case 0x7c6c0000:
            case 0x7c720000:  case 0x7c740000:  case 0x7c760000:  case 0x7c780000:
            case 0x7c7a0000:  case 0x7c7c0000:  case 0x7c820000:  case 0x7c840000:
            case 0x7c860000:  case 0x7c880000:  case 0x7c8a0000:  case 0x7c8c0000:
            case 0x7c920000:  case 0x7c940000:  case 0x7c960000:  case 0x7c980000:
            case 0x7c9a0000:  case 0x7c9c0000:  case 0x7ca20000:  case 0x7ca40000:
            case 0x7ca60000:  case 0x7ca80000:  case 0x7caa0000:  case 0x7cac0000:
            case 0x7cb20000:  case 0x7cb40000:  case 0x7cb60000:  case 0x7cb80000:
            case 0x7cba0000:  case 0x7cbc0000:  case 0x7cfa0000:  case 0x7cfc0000:
            case 0x7cfe0000:  case 0x7d020000:  case 0x7d040000:  case 0x7d060000:
            case 0x7d080000:  case 0x7d0a0000:  case 0x7d0c0000:  case 0x7d0e0000:
            case 0x7d100000:  case 0x7d120000:  case 0x7d140000:  case 0x7d160000:
            case 0x7d180000:  case 0x7d1a0000:  case 0x7d1c0000:  case 0x7d220000:
            case 0x7d240000:  case 0x7d260000:  case 0x7d280000:  case 0x7d2a0000:
            case 0x7d2c0000:  case 0x7d2e0000:  case 0x7d300000:  case 0x7d320000:
            case 0x7d340000:  case 0x7d360000:  case 0x7d380000:  case 0x7d3a0000:
            case 0x7d3c0000:  case 0x7d420000:  case 0x7d440000:  case 0x7d460000:
            case 0x7d480000:  case 0x7d4a0000:  case 0x7d4c0000:  case 0x7d4e0000:
            case 0x7d500000:  case 0x7d520000:  case 0x7d540000:  case 0x7d560000:
            case 0x7d580000:  case 0x7d5a0000:  case 0x7d5c0000:  case 0x7d620000:
            case 0x7d640000:  case 0x7d660000:  case 0x7d680000:  case 0x7d6a0000:
            case 0x7d6c0000:  case 0x7d720000:  case 0x7d740000:  case 0x7d760000:
            case 0x7d780000:  case 0x7d7a0000:  case 0x7d7c0000:  case 0x7d820000:
            case 0x7d840000:  case 0x7d860000:  case 0x7d880000:  case 0x7d8a0000:
            case 0x7d8c0000:  case 0x7d920000:  case 0x7d940000:  case 0x7d960000:
            case 0x7d980000:  case 0x7d9a0000:  case 0x7d9c0000:  case 0x7da20000:
            case 0x7da40000:  case 0x7da60000:  case 0x7da80000:  case 0x7daa0000:
            case 0x7dac0000:  case 0x7db20000:  case 0x7db40000:  case 0x7db60000:
            case 0x7db80000:  case 0x7dba0000:  case 0x7dbc0000:  case 0x7dfa0000:
            case 0x7dfc0000:  case 0x7dfe0000:
                return true;

            default:
                return false;
        }
    }

    bool InstructionDecoder_amdgpu_gfx1250::IS_VOPC_VOP_DPP8(uint64_t I)
    {
        switch ( I & 0xfffe0000 )  {
            case 0x7c020000:  case 0x7c040000:  case 0x7c060000:  case 0x7c080000:
            case 0x7c0a0000:  case 0x7c0c0000:  case 0x7c0e0000:  case 0x7c100000:
            case 0x7c120000:  case 0x7c140000:  case 0x7c160000:  case 0x7c180000:
            case 0x7c1a0000:  case 0x7c1c0000:  case 0x7c220000:  case 0x7c240000:
            case 0x7c260000:  case 0x7c280000:  case 0x7c2a0000:  case 0x7c2c0000:
            case 0x7c2e0000:  case 0x7c300000:  case 0x7c320000:  case 0x7c340000:
            case 0x7c360000:  case 0x7c380000:  case 0x7c3a0000:  case 0x7c3c0000:
            case 0x7c620000:  case 0x7c640000:  case 0x7c660000:  case 0x7c680000:
            case 0x7c6a0000:  case 0x7c6c0000:  case 0x7c720000:  case 0x7c740000:
            case 0x7c760000:  case 0x7c780000:  case 0x7c7a0000:  case 0x7c7c0000:
            case 0x7c820000:  case 0x7c840000:  case 0x7c860000:  case 0x7c880000:
            case 0x7c8a0000:  case 0x7c8c0000:  case 0x7c920000:  case 0x7c940000:
            case 0x7c960000:  case 0x7c980000:  case 0x7c9a0000:  case 0x7c9c0000:
            case 0x7cfa0000:  case 0x7cfc0000:  case 0x7d020000:  case 0x7d040000:
            case 0x7d060000:  case 0x7d080000:  case 0x7d0a0000:  case 0x7d0c0000:
            case 0x7d0e0000:  case 0x7d100000:  case 0x7d120000:  case 0x7d140000:
            case 0x7d160000:  case 0x7d180000:  case 0x7d1a0000:  case 0x7d1c0000:
            case 0x7d220000:  case 0x7d240000:  case 0x7d260000:  case 0x7d280000:
            case 0x7d2a0000:  case 0x7d2c0000:  case 0x7d2e0000:  case 0x7d300000:
            case 0x7d320000:  case 0x7d340000:  case 0x7d360000:  case 0x7d380000:
            case 0x7d3a0000:  case 0x7d3c0000:  case 0x7d620000:  case 0x7d640000:
            case 0x7d660000:  case 0x7d680000:  case 0x7d6a0000:  case 0x7d6c0000:
            case 0x7d720000:  case 0x7d740000:  case 0x7d760000:  case 0x7d780000:
            case 0x7d7a0000:  case 0x7d7c0000:  case 0x7d820000:  case 0x7d840000:
            case 0x7d860000:  case 0x7d880000:  case 0x7d8a0000:  case 0x7d8c0000:
            case 0x7d920000:  case 0x7d940000:  case 0x7d960000:  case 0x7d980000:
            case 0x7d9a0000:  case 0x7d9c0000:  case 0x7dfa0000:  case 0x7dfc0000:
                return true;

            default:
                return false;
        }
    }

    bool InstructionDecoder_amdgpu_gfx1250::IS_VOPC_VOP_DPP16(uint64_t I)
    {
        switch ( I & 0xfffe0000 )  {
            case 0x7c020000:  case 0x7c040000:  case 0x7c060000:  case 0x7c080000:
            case 0x7c0a0000:  case 0x7c0c0000:  case 0x7c0e0000:  case 0x7c100000:
            case 0x7c120000:  case 0x7c140000:  case 0x7c160000:  case 0x7c180000:
            case 0x7c1a0000:  case 0x7c1c0000:  case 0x7c220000:  case 0x7c240000:
            case 0x7c260000:  case 0x7c280000:  case 0x7c2a0000:  case 0x7c2c0000:
            case 0x7c2e0000:  case 0x7c300000:  case 0x7c320000:  case 0x7c340000:
            case 0x7c360000:  case 0x7c380000:  case 0x7c3a0000:  case 0x7c3c0000:
            case 0x7c620000:  case 0x7c640000:  case 0x7c660000:  case 0x7c680000:
            case 0x7c6a0000:  case 0x7c6c0000:  case 0x7c720000:  case 0x7c740000:
            case 0x7c760000:  case 0x7c780000:  case 0x7c7a0000:  case 0x7c7c0000:
            case 0x7c820000:  case 0x7c840000:  case 0x7c860000:  case 0x7c880000:
            case 0x7c8a0000:  case 0x7c8c0000:  case 0x7c920000:  case 0x7c940000:
            case 0x7c960000:  case 0x7c980000:  case 0x7c9a0000:  case 0x7c9c0000:
            case 0x7cfa0000:  case 0x7cfc0000:  case 0x7d020000:  case 0x7d040000:
            case 0x7d060000:  case 0x7d080000:  case 0x7d0a0000:  case 0x7d0c0000:
            case 0x7d0e0000:  case 0x7d100000:  case 0x7d120000:  case 0x7d140000:
            case 0x7d160000:  case 0x7d180000:  case 0x7d1a0000:  case 0x7d1c0000:
            case 0x7d220000:  case 0x7d240000:  case 0x7d260000:  case 0x7d280000:
            case 0x7d2a0000:  case 0x7d2c0000:  case 0x7d2e0000:  case 0x7d300000:
            case 0x7d320000:  case 0x7d340000:  case 0x7d360000:  case 0x7d380000:
            case 0x7d3a0000:  case 0x7d3c0000:  case 0x7d620000:  case 0x7d640000:
            case 0x7d660000:  case 0x7d680000:  case 0x7d6a0000:  case 0x7d6c0000:
            case 0x7d720000:  case 0x7d740000:  case 0x7d760000:  case 0x7d780000:
            case 0x7d7a0000:  case 0x7d7c0000:  case 0x7d820000:  case 0x7d840000:
            case 0x7d860000:  case 0x7d880000:  case 0x7d8a0000:  case 0x7d8c0000:
            case 0x7d920000:  case 0x7d940000:  case 0x7d960000:  case 0x7d980000:
            case 0x7d9a0000:  case 0x7d9c0000:  case 0x7dfa0000:  case 0x7dfc0000:
                return true;

            default:
                return false;
        }
    }

    bool InstructionDecoder_amdgpu_gfx1250::IS_VOPD3XY_X(uint64_t I)
    {
        switch ( I & 0xfffc0000 )  {
            case 0xcf800000:  case 0xcf020000:  case 0xcf840000:  case 0xcf021000:
            case 0xcf880000:  case 0xcf022000:  case 0xcf8c0000:  case 0xcf023000:
            case 0xcf900000:  case 0xcf024000:  case 0xcf000000:  case 0xcf0c0000:
            case 0xcf003000:  case 0xcf100000:  case 0xcf004000:  case 0xcf140000:
            case 0xcf005000:  case 0xcf180000:  case 0xcf006000:  case 0xcf1c0000:
            case 0xcf007000:  case 0xcf200000:  case 0xcf008000:  case 0xcf240000:
            case 0xcf009000:  case 0xcf280000:  case 0xcf00a000:  case 0xcf2c0000:
            case 0xcf00b000:  case 0xcf400000:  case 0xcf010000:  case 0xcf440000:
            case 0xcf011000:  case 0xcf480000:  case 0xcf012000:  case 0xcf4c0000:
            case 0xcf013000:  case 0xcf500000:  case 0xcf014000:  case 0xcf540000:
            case 0xcf015000:  case 0xcf580000:  case 0xcf016000:  case 0xcf5c0000:
            case 0xcf017000:  case 0xcf600000:  case 0xcf018000:
                return true;

            default:
                return false;
        }
    }

    bool InstructionDecoder_amdgpu_gfx1250::IS_VOPD3XY_Y(uint64_t I)
    {
        switch ( I & 0xfffc0000 )  {
            case 0xcf800000:  case 0xcf020000:  case 0xcf840000:  case 0xcf021000:
            case 0xcf880000:  case 0xcf022000:  case 0xcf8c0000:  case 0xcf023000:
            case 0xcf900000:  case 0xcf024000:  case 0xcf000000:  case 0xcf0c0000:
            case 0xcf003000:  case 0xcf100000:  case 0xcf004000:  case 0xcf140000:
            case 0xcf005000:  case 0xcf180000:  case 0xcf006000:  case 0xcf1c0000:
            case 0xcf007000:  case 0xcf200000:  case 0xcf008000:  case 0xcf240000:
            case 0xcf009000:  case 0xcf280000:  case 0xcf00a000:  case 0xcf2c0000:
            case 0xcf00b000:  case 0xcf400000:  case 0xcf010000:  case 0xcf440000:
            case 0xcf011000:  case 0xcf480000:  case 0xcf012000:  case 0xcf4c0000:
            case 0xcf013000:  case 0xcf500000:  case 0xcf014000:  case 0xcf540000:
            case 0xcf015000:  case 0xcf580000:  case 0xcf016000:  case 0xcf5c0000:
            case 0xcf017000:  case 0xcf600000:  case 0xcf018000:
                return true;

            default:
                return false;
        }
    }

    bool InstructionDecoder_amdgpu_gfx1250::IS_VOPDXY_X(uint64_t I)
    {
        switch ( I & 0xffc00000 )  {
            case 0xc8000000:  case 0xc8c00000:  case 0xc8060000:  case 0xc9000000:
            case 0xc8080000:  case 0xc9400000:  case 0xc80a0000:  case 0xc9800000:
            case 0xc80c0000:  case 0xc9c00000:  case 0xc80e0000:  case 0xca000000:
            case 0xc8100000:  case 0xca400000:  case 0xc8120000:  case 0xca800000:
            case 0xc8140000:  case 0xcac00000:  case 0xc8160000:
                return true;

            default:
                return false;
        }
    }

    bool InstructionDecoder_amdgpu_gfx1250::IS_VOPDXY_INST_LITERAL_X(uint64_t I)
    {
        switch ( I & 0xffc00000 )  {
            case 0xc8000000:  case 0xc8400000:  case 0xc8020000:  case 0xc8800000:
            case 0xc8040000:  case 0xc8c00000:  case 0xc8060000:  case 0xc9000000:
            case 0xc8080000:  case 0xc9400000:  case 0xc80a0000:  case 0xc9800000:
            case 0xc80c0000:  case 0xc9c00000:  case 0xc80e0000:  case 0xca000000:
            case 0xc8100000:  case 0xca400000:  case 0xc8120000:  case 0xca800000:
            case 0xc8140000:  case 0xcac00000:  case 0xc8160000:
                return true;

            default:
                return false;
        }
    }

    bool InstructionDecoder_amdgpu_gfx1250::IS_VOPDXY_Y(uint64_t I)
    {
        switch ( I & 0xffc00000 )  {
            case 0xc8000000:  case 0xc8c00000:  case 0xc8060000:  case 0xc9000000:
            case 0xc8080000:  case 0xc9400000:  case 0xc80a0000:  case 0xc9800000:
            case 0xc80c0000:  case 0xc9c00000:  case 0xc80e0000:  case 0xca000000:
            case 0xc8100000:  case 0xca400000:  case 0xc8120000:  case 0xca800000:
            case 0xc8140000:  case 0xcac00000:  case 0xc8160000:  case 0xcc000000:
            case 0xc8200000:  case 0xcc400000:  case 0xc8220000:  case 0xcd000000:
            case 0xc8280000:  case 0xcd400000:  case 0xc82a0000:  case 0xcd800000:
            case 0xc82c0000:  case 0xcdc00000:  case 0xc82e0000:  case 0xce000000:
            case 0xc8300000:
                return true;

            default:
                return false;
        }
    }

    bool InstructionDecoder_amdgpu_gfx1250::IS_VOPDXY_INST_LITERAL_Y(uint64_t I)
    {
        switch ( I & 0xffc00000 )  {
            case 0xc8000000:  case 0xc8400000:  case 0xc8020000:  case 0xc8800000:
            case 0xc8040000:  case 0xc8c00000:  case 0xc8060000:  case 0xc9000000:
            case 0xc8080000:  case 0xc9400000:  case 0xc80a0000:  case 0xc9800000:
            case 0xc80c0000:  case 0xc9c00000:  case 0xc80e0000:  case 0xca000000:
            case 0xc8100000:  case 0xca400000:  case 0xc8120000:  case 0xca800000:
            case 0xc8140000:  case 0xcac00000:  case 0xc8160000:  case 0xcc000000:
            case 0xc8200000:  case 0xcc400000:  case 0xc8220000:  case 0xcd000000:
            case 0xc8280000:  case 0xcd400000:  case 0xc82a0000:  case 0xcd800000:
            case 0xc82c0000:  case 0xcdc00000:  case 0xc82e0000:  case 0xce000000:
            case 0xc8300000:
                return true;

            default:
                return false;
        }
    }

    bool InstructionDecoder_amdgpu_gfx1250::COND_ENC_SOP1_not_has_lit64_0_and_not_has_lit64_1_and_not_has_lit_0_and_not_has_lit_1(uint64_t I)
    {
        return ((longfield<0,7>(I)) != 254 && (longfield<0,7>(I)) != 255);
    }

    bool InstructionDecoder_amdgpu_gfx1250::COND_ENC_SOPC_default(uint64_t I)
    {
        return (((((longfield<0,7>(I)) != 254) && ((longfield<8,15>(I)) != 254)) && ((longfield<0,7>(I)) != 255)) && ((longfield<8,15>(I)) != 255));
    }

    bool InstructionDecoder_amdgpu_gfx1250::COND_ENC_SOP2_default(uint64_t I)
    {
        return (((((longfield<0,7>(I)) != 254) && ((longfield<8,15>(I)) != 254)) && ((longfield<0,7>(I)) != 255)) && ((longfield<8,15>(I)) != 255));
    }

    bool InstructionDecoder_amdgpu_gfx1250::COND_ENC_VOP1_default(uint64_t I)
    {
        return ((longfield<0,8>(I)) != 250 && (longfield<0,8>(I)) != 233 && (longfield<0,8>(I)) != 234 && (longfield<0,8>(I)) != 255 && (longfield<0,8>(I)) != 254);
    }

    bool InstructionDecoder_amdgpu_gfx1250::COND_ENC_VOPC_default(uint64_t I)
    {
        return ((longfield<0,8>(I)) != 250 && (longfield<0,8>(I)) != 233 && (longfield<0,8>(I)) != 234 && (longfield<0,8>(I)) != 255 && (longfield<0,8>(I)) != 254);
    }

    bool InstructionDecoder_amdgpu_gfx1250::COND_ENC_VOP2_default(uint64_t I)
    {
        return ((longfield<0,8>(I)) != 250 && (longfield<0,8>(I)) != 233 && (longfield<0,8>(I)) != 234 && (longfield<0,8>(I)) != 255 && (longfield<0,8>(I)) != 254);
    }

    bool InstructionDecoder_amdgpu_gfx1250::COND_ENC_VOP3_default(uint64_t I)
    {
        return ((((longfield<32,40>(I)) != 250 && (longfield<32,40>(I)) != 233 && (longfield<32,40>(I)) != 234 && (longfield<32,40>(I)) != 255) && ((longfield<41,49>(I)) != 255)) && ((longfield<50,58>(I)) != 255));
    }

    bool InstructionDecoder_amdgpu_gfx1250::COND_ENC_VOP3P_default(uint64_t I)
    {
        return ((((longfield<32,40>(I)) != 250 && (longfield<32,40>(I)) != 233 && (longfield<32,40>(I)) != 234 && (longfield<32,40>(I)) != 255) && ((longfield<41,49>(I)) != 255)) && ((longfield<50,58>(I)) != 255));
    }

    bool InstructionDecoder_amdgpu_gfx1250::COND_SOP1_INST_LITERAL_has_lit_0(uint64_t I)
    {
        return ((longfield<0,7>(I)) == 255);
    }

    bool InstructionDecoder_amdgpu_gfx1250::COND_SOP1_INST_LITERAL64_has_lit64_0(uint64_t I)
    {
        return ((longfield<0,7>(I)) == 254);
    }

    bool InstructionDecoder_amdgpu_gfx1250::COND_SOP2_INST_LITERAL_has_lit_0(uint64_t I)
    {
        return ((((longfield<8,15>(I)) != 254) && ((longfield<0,7>(I)) == 255)) && ((longfield<8,15>(I)) != 255));
    }

    bool InstructionDecoder_amdgpu_gfx1250::COND_SOP2_INST_LITERAL_has_lit_1(uint64_t I)
    {
        return (((longfield<0,7>(I)) != 254 && (longfield<0,7>(I)) != 255) && ((longfield<8,15>(I)) == 255));
    }

    bool InstructionDecoder_amdgpu_gfx1250::COND_SOP2_INST_LITERAL_has_lit_0_and_has_lit_1(uint64_t I)
    {
        return (((longfield<0,7>(I)) == 255) && ((longfield<8,15>(I)) == 255));
    }

    bool InstructionDecoder_amdgpu_gfx1250::COND_SOP2_INST_LITERAL_default(uint64_t I)
    {
        return (((((longfield<0,7>(I)) != 254) && ((longfield<8,15>(I)) != 254)) && ((longfield<0,7>(I)) != 255)) && ((longfield<8,15>(I)) != 255));
    }

    bool InstructionDecoder_amdgpu_gfx1250::COND_SOP2_INST_LITERAL64_has_lit64_0(uint64_t I)
    {
        return (((((longfield<0,7>(I)) == 254) && ((longfield<8,15>(I)) != 254)) && ((longfield<0,7>(I)) != 255)) && ((longfield<8,15>(I)) != 255));
    }

    bool InstructionDecoder_amdgpu_gfx1250::COND_SOP2_INST_LITERAL64_has_lit64_1(uint64_t I)
    {
        return (((((longfield<0,7>(I)) != 254) && ((longfield<8,15>(I)) == 254)) && ((longfield<0,7>(I)) != 255)) && ((longfield<8,15>(I)) != 255));
    }

    bool InstructionDecoder_amdgpu_gfx1250::COND_SOP2_INST_LITERAL64_has_lit64_0_and_has_lit64_1(uint64_t I)
    {
        return (((((longfield<0,7>(I)) == 254) && ((longfield<8,15>(I)) == 254)) && ((longfield<0,7>(I)) != 255)) && ((longfield<8,15>(I)) != 255));
    }

    bool InstructionDecoder_amdgpu_gfx1250::COND_SOPC_INST_LITERAL_has_lit_0(uint64_t I)
    {
        return ((((longfield<8,15>(I)) != 254) && ((longfield<0,7>(I)) == 255)) && ((longfield<8,15>(I)) != 255));
    }

    bool InstructionDecoder_amdgpu_gfx1250::COND_SOPC_INST_LITERAL_has_lit_1(uint64_t I)
    {
        return (((longfield<0,7>(I)) != 254 && (longfield<0,7>(I)) != 255) && ((longfield<8,15>(I)) == 255));
    }

    bool InstructionDecoder_amdgpu_gfx1250::COND_SOPC_INST_LITERAL_has_lit_0_and_has_lit_1(uint64_t I)
    {
        return (((longfield<0,7>(I)) == 255) && ((longfield<8,15>(I)) == 255));
    }

    bool InstructionDecoder_amdgpu_gfx1250::COND_SOPC_INST_LITERAL64_has_lit64_0(uint64_t I)
    {
        return (((((longfield<0,7>(I)) == 254) && ((longfield<8,15>(I)) != 254)) && ((longfield<0,7>(I)) != 255)) && ((longfield<8,15>(I)) != 255));
    }

    bool InstructionDecoder_amdgpu_gfx1250::COND_SOPC_INST_LITERAL64_has_lit64_1(uint64_t I)
    {
        return (((((longfield<0,7>(I)) != 254) && ((longfield<8,15>(I)) == 254)) && ((longfield<0,7>(I)) != 255)) && ((longfield<8,15>(I)) != 255));
    }

    bool InstructionDecoder_amdgpu_gfx1250::COND_SOPC_INST_LITERAL64_has_lit64_0_and_has_lit64_1(uint64_t I)
    {
        return (((((longfield<0,7>(I)) == 254) && ((longfield<8,15>(I)) == 254)) && ((longfield<0,7>(I)) != 255)) && ((longfield<8,15>(I)) != 255));
    }

    bool InstructionDecoder_amdgpu_gfx1250::COND_VOP1_INST_LITERAL_has_lit(uint64_t I)
    {
        return ((longfield<0,8>(I)) == 255);
    }

    bool InstructionDecoder_amdgpu_gfx1250::COND_VOP1_INST_LITERAL64_has_lit64(uint64_t I)
    {
        return ((longfield<0,8>(I)) == 254);
    }

    bool InstructionDecoder_amdgpu_gfx1250::COND_VOP1_VOP_DPP8_has_dpp8(uint64_t I)
    {
        return ((longfield<0,8>(I)) == 233 || (longfield<0,8>(I)) == 234);
    }

    bool InstructionDecoder_amdgpu_gfx1250::COND_VOP1_VOP_DPP16_has_dpp16(uint64_t I)
    {
        return ((longfield<0,8>(I)) == 250);
    }

    bool InstructionDecoder_amdgpu_gfx1250::COND_VOP3_INST_LITERAL_has_lit_0(uint64_t I)
    {
        return ((((longfield<32,40>(I)) == 255) && ((longfield<41,49>(I)) != 255)) && ((longfield<50,58>(I)) != 255));
    }

    bool InstructionDecoder_amdgpu_gfx1250::COND_VOP3_INST_LITERAL_has_lit_1(uint64_t I)
    {
        return ((((longfield<32,40>(I)) != 250 && (longfield<32,40>(I)) != 233 && (longfield<32,40>(I)) != 234 && (longfield<32,40>(I)) != 255) && ((longfield<41,49>(I)) == 255)) && ((longfield<50,58>(I)) != 255));
    }

    bool InstructionDecoder_amdgpu_gfx1250::COND_VOP3_INST_LITERAL_has_lit_0_and_has_lit_1(uint64_t I)
    {
        return ((((longfield<32,40>(I)) == 255) && ((longfield<41,49>(I)) == 255)) && ((longfield<50,58>(I)) != 255));
    }

    bool InstructionDecoder_amdgpu_gfx1250::COND_VOP3_INST_LITERAL_has_lit_2(uint64_t I)
    {
        return ((((longfield<32,40>(I)) != 250 && (longfield<32,40>(I)) != 233 && (longfield<32,40>(I)) != 234 && (longfield<32,40>(I)) != 255) && ((longfield<41,49>(I)) != 255)) && ((longfield<50,58>(I)) == 255));
    }

    bool InstructionDecoder_amdgpu_gfx1250::COND_VOP3_INST_LITERAL_has_lit_0_and_has_lit_2(uint64_t I)
    {
        return ((((longfield<32,40>(I)) == 255) && ((longfield<41,49>(I)) != 255)) && ((longfield<50,58>(I)) == 255));
    }

    bool InstructionDecoder_amdgpu_gfx1250::COND_VOP3_INST_LITERAL_has_lit_1_and_has_lit_2(uint64_t I)
    {
        return ((((longfield<32,40>(I)) != 250 && (longfield<32,40>(I)) != 233 && (longfield<32,40>(I)) != 234 && (longfield<32,40>(I)) != 255) && ((longfield<41,49>(I)) == 255)) && ((longfield<50,58>(I)) == 255));
    }

    bool InstructionDecoder_amdgpu_gfx1250::COND_VOP3_INST_LITERAL_has_lit_0_and_has_lit_1_and_has_lit_2(uint64_t I)
    {
        return ((((longfield<32,40>(I)) == 255) && ((longfield<41,49>(I)) == 255)) && ((longfield<50,58>(I)) == 255));
    }

    bool InstructionDecoder_amdgpu_gfx1250::COND_VOP3_VOP_DPP8_has_dpp8(uint64_t I)
    {
        return ((((longfield<32,40>(I)) == 233 || (longfield<32,40>(I)) == 234) && ((longfield<41,49>(I)) != 255)) && ((longfield<50,58>(I)) != 255));
    }

    bool InstructionDecoder_amdgpu_gfx1250::COND_VOP3_VOP_DPP16_has_dpp16(uint64_t I)
    {
        return ((((longfield<32,40>(I)) == 250) && ((longfield<41,49>(I)) != 255)) && ((longfield<50,58>(I)) != 255));
    }

    bool InstructionDecoder_amdgpu_gfx1250::COND_VOP2_INST_LITERAL_has_lit(uint64_t I)
    {
        return ((longfield<0,8>(I)) == 255);
    }

    bool InstructionDecoder_amdgpu_gfx1250::COND_VOP2_INST_LITERAL_default(uint64_t I)
    {
        return ((longfield<0,8>(I)) != 250 && (longfield<0,8>(I)) != 233 && (longfield<0,8>(I)) != 234 && (longfield<0,8>(I)) != 255 && (longfield<0,8>(I)) != 254);
    }

    bool InstructionDecoder_amdgpu_gfx1250::COND_VOP2_INST_LITERAL64_has_lit64(uint64_t I)
    {
        return ((longfield<0,8>(I)) == 254);
    }

    bool InstructionDecoder_amdgpu_gfx1250::COND_VOP2_INST_LITERAL64_default(uint64_t I)
    {
        return ((longfield<0,8>(I)) != 250 && (longfield<0,8>(I)) != 233 && (longfield<0,8>(I)) != 234 && (longfield<0,8>(I)) != 255 && (longfield<0,8>(I)) != 254);
    }

    bool InstructionDecoder_amdgpu_gfx1250::COND_VOP2_VOP_DPP8_has_dpp8(uint64_t I)
    {
        return ((longfield<0,8>(I)) == 233 || (longfield<0,8>(I)) == 234);
    }

    bool InstructionDecoder_amdgpu_gfx1250::COND_VOP2_VOP_DPP16_has_dpp16(uint64_t I)
    {
        return ((longfield<0,8>(I)) == 250);
    }

    bool InstructionDecoder_amdgpu_gfx1250::COND_VOP3_SDST_ENC_default(uint64_t I)
    {
        return ((((longfield<32,40>(I)) != 250 && (longfield<32,40>(I)) != 233 && (longfield<32,40>(I)) != 234 && (longfield<32,40>(I)) != 255) && ((longfield<41,49>(I)) != 255)) && ((longfield<50,58>(I)) != 255));
    }

    bool InstructionDecoder_amdgpu_gfx1250::COND_VOP3_SDST_ENC_INST_LITERAL_has_lit_0(uint64_t I)
    {
        return ((((longfield<32,40>(I)) == 255) && ((longfield<41,49>(I)) != 255)) && ((longfield<50,58>(I)) != 255));
    }

    bool InstructionDecoder_amdgpu_gfx1250::COND_VOP3_SDST_ENC_INST_LITERAL_has_lit_1(uint64_t I)
    {
        return ((((longfield<32,40>(I)) != 250 && (longfield<32,40>(I)) != 233 && (longfield<32,40>(I)) != 234 && (longfield<32,40>(I)) != 255) && ((longfield<41,49>(I)) == 255)) && ((longfield<50,58>(I)) != 255));
    }

    bool InstructionDecoder_amdgpu_gfx1250::COND_VOP3_SDST_ENC_INST_LITERAL_has_lit_0_and_has_lit_1(uint64_t I)
    {
        return ((((longfield<32,40>(I)) == 255) && ((longfield<41,49>(I)) == 255)) && ((longfield<50,58>(I)) != 255));
    }

    bool InstructionDecoder_amdgpu_gfx1250::COND_VOP3_SDST_ENC_INST_LITERAL_has_lit_2(uint64_t I)
    {
        return ((((longfield<32,40>(I)) != 250 && (longfield<32,40>(I)) != 233 && (longfield<32,40>(I)) != 234 && (longfield<32,40>(I)) != 255) && ((longfield<41,49>(I)) != 255)) && ((longfield<50,58>(I)) == 255));
    }

    bool InstructionDecoder_amdgpu_gfx1250::COND_VOP3_SDST_ENC_INST_LITERAL_has_lit_0_and_has_lit_2(uint64_t I)
    {
        return ((((longfield<32,40>(I)) == 255) && ((longfield<41,49>(I)) != 255)) && ((longfield<50,58>(I)) == 255));
    }

    bool InstructionDecoder_amdgpu_gfx1250::COND_VOP3_SDST_ENC_INST_LITERAL_has_lit_1_and_has_lit_2(uint64_t I)
    {
        return ((((longfield<32,40>(I)) != 250 && (longfield<32,40>(I)) != 233 && (longfield<32,40>(I)) != 234 && (longfield<32,40>(I)) != 255) && ((longfield<41,49>(I)) == 255)) && ((longfield<50,58>(I)) == 255));
    }

    bool InstructionDecoder_amdgpu_gfx1250::COND_VOP3_SDST_ENC_INST_LITERAL_has_lit_0_and_has_lit_1_and_has_lit_2(uint64_t I)
    {
        return ((((longfield<32,40>(I)) == 255) && ((longfield<41,49>(I)) == 255)) && ((longfield<50,58>(I)) == 255));
    }

    bool InstructionDecoder_amdgpu_gfx1250::COND_VOP3_SDST_ENC_VOP_DPP8_has_dpp8(uint64_t I)
    {
        return ((((longfield<32,40>(I)) == 233 || (longfield<32,40>(I)) == 234) && ((longfield<41,49>(I)) != 255)) && ((longfield<50,58>(I)) != 255));
    }

    bool InstructionDecoder_amdgpu_gfx1250::COND_VOP3_SDST_ENC_VOP_DPP16_has_dpp16(uint64_t I)
    {
        return ((((longfield<32,40>(I)) == 250) && ((longfield<41,49>(I)) != 255)) && ((longfield<50,58>(I)) != 255));
    }

    bool InstructionDecoder_amdgpu_gfx1250::COND_VOP3P_INST_LITERAL_has_lit_0(uint64_t I)
    {
        return ((((longfield<32,40>(I)) == 255) && ((longfield<41,49>(I)) != 255)) && ((longfield<50,58>(I)) != 255));
    }

    bool InstructionDecoder_amdgpu_gfx1250::COND_VOP3P_INST_LITERAL_has_lit_1(uint64_t I)
    {
        return ((((longfield<32,40>(I)) != 250 && (longfield<32,40>(I)) != 233 && (longfield<32,40>(I)) != 234 && (longfield<32,40>(I)) != 255) && ((longfield<41,49>(I)) == 255)) && ((longfield<50,58>(I)) != 255));
    }

    bool InstructionDecoder_amdgpu_gfx1250::COND_VOP3P_INST_LITERAL_has_lit_0_and_has_lit_1(uint64_t I)
    {
        return ((((longfield<32,40>(I)) == 255) && ((longfield<41,49>(I)) == 255)) && ((longfield<50,58>(I)) != 255));
    }

    bool InstructionDecoder_amdgpu_gfx1250::COND_VOP3P_INST_LITERAL_has_lit_2(uint64_t I)
    {
        return ((((longfield<32,40>(I)) != 250 && (longfield<32,40>(I)) != 233 && (longfield<32,40>(I)) != 234 && (longfield<32,40>(I)) != 255) && ((longfield<41,49>(I)) != 255)) && ((longfield<50,58>(I)) == 255));
    }

    bool InstructionDecoder_amdgpu_gfx1250::COND_VOP3P_INST_LITERAL_has_lit_0_and_has_lit_2(uint64_t I)
    {
        return ((((longfield<32,40>(I)) == 255) && ((longfield<41,49>(I)) != 255)) && ((longfield<50,58>(I)) == 255));
    }

    bool InstructionDecoder_amdgpu_gfx1250::COND_VOP3P_INST_LITERAL_has_lit_1_and_has_lit_2(uint64_t I)
    {
        return ((((longfield<32,40>(I)) != 250 && (longfield<32,40>(I)) != 233 && (longfield<32,40>(I)) != 234 && (longfield<32,40>(I)) != 255) && ((longfield<41,49>(I)) == 255)) && ((longfield<50,58>(I)) == 255));
    }

    bool InstructionDecoder_amdgpu_gfx1250::COND_VOP3P_INST_LITERAL_has_lit_0_and_has_lit_1_and_has_lit_2(uint64_t I)
    {
        return ((((longfield<32,40>(I)) == 255) && ((longfield<41,49>(I)) == 255)) && ((longfield<50,58>(I)) == 255));
    }

    bool InstructionDecoder_amdgpu_gfx1250::COND_VOP3P_VOP_DPP8_has_dpp8(uint64_t I)
    {
        return ((((longfield<32,40>(I)) == 233 || (longfield<32,40>(I)) == 234) && ((longfield<41,49>(I)) != 255)) && ((longfield<50,58>(I)) != 255));
    }

    bool InstructionDecoder_amdgpu_gfx1250::COND_VOP3P_VOP_DPP16_has_dpp16(uint64_t I)
    {
        return ((((longfield<32,40>(I)) == 250) && ((longfield<41,49>(I)) != 255)) && ((longfield<50,58>(I)) != 255));
    }

    bool InstructionDecoder_amdgpu_gfx1250::COND_VOPC_INST_LITERAL_has_lit(uint64_t I)
    {
        return ((longfield<0,8>(I)) == 255);
    }

    bool InstructionDecoder_amdgpu_gfx1250::COND_VOPC_INST_LITERAL64_has_lit64(uint64_t I)
    {
        return ((longfield<0,8>(I)) == 254);
    }

    bool InstructionDecoder_amdgpu_gfx1250::COND_VOPC_VOP_DPP8_has_dpp8(uint64_t I)
    {
        return ((longfield<0,8>(I)) == 233 || (longfield<0,8>(I)) == 234);
    }

    bool InstructionDecoder_amdgpu_gfx1250::COND_VOPC_VOP_DPP16_has_dpp16(uint64_t I)
    {
        return ((longfield<0,8>(I)) == 250);
    }

    bool InstructionDecoder_amdgpu_gfx1250::COND_VOPDXY_X_default(uint64_t I)
    {
        return ((longfield<0,8>(I)) != 255);
    }

    bool InstructionDecoder_amdgpu_gfx1250::COND_VOPDXY_INST_LITERAL_X_has_lit(uint64_t I)
    {
        return ((longfield<0,8>(I)) == 255);
    }

    bool InstructionDecoder_amdgpu_gfx1250::COND_VOPDXY_INST_LITERAL_X_default(uint64_t I)
    {
        return ((longfield<0,8>(I)) != 255);
    }

    bool InstructionDecoder_amdgpu_gfx1250::COND_VOPDXY_Y_default(uint64_t I)
    {
        return ((longfield<32,40>(I)) != 255);
    }

    bool InstructionDecoder_amdgpu_gfx1250::COND_VOPDXY_INST_LITERAL_Y_has_lit(uint64_t I)
    {
        return ((longfield<32,40>(I)) == 255);
    }

    bool InstructionDecoder_amdgpu_gfx1250::COND_VOPDXY_INST_LITERAL_Y_default(uint64_t I)
    {
        return ((longfield<32,40>(I)) != 255);
    }

    bool InstructionDecoder_amdgpu_gfx1250::TRIVIAL_ENC_SOP1(uint64_t I)
    {
        uint64_t op = longfield<8,15>(I);
        return op == 76 || op == 77;
    }

    bool InstructionDecoder_amdgpu_gfx1250::FORCED_SOP2_INST_LITERAL(uint64_t I)
    {
        uint64_t op = longfield<23,29>(I);
        return op == 69 || op == 70;
    }

    bool InstructionDecoder_amdgpu_gfx1250::FORCED_SOPK_INST_LITERAL(uint64_t I)
    {
        uint64_t op = longfield<23,27>(I);
        return op == 19;
    }

    bool InstructionDecoder_amdgpu_gfx1250::FORCED_VOP2_INST_LITERAL(uint64_t I)
    {
        uint64_t op = longfield<25,30>(I);
        return op == 44 || op == 45 || op == 55 || op == 56;
    }

    bool InstructionDecoder_amdgpu_gfx1250::FORCED_VOP2_INST_LITERAL64(uint64_t I)
    {
        uint64_t op = longfield<25,30>(I);
        return op == 35 || op == 36;
    }

    void InstructionDecoder_amdgpu_gfx1250::decodeENC_SOP1()
    {
        insn_size = 4;
        layout_ENC_SOP1 & layout = insn_layout.ENC_SOP1;
        layout.ENCODING = longfield<23,31>(insn_long);
        layout.OP = longfield<8,15>(insn_long);
        layout.SDST = longfield<16,22>(insn_long);
        layout.SSRC0 = longfield<0,7>(insn_long);
        assert(isArrayIndexValid(ENC_SOP1_insn_table, layout.OP) && "Opcode over or underflow");
        const amdgpu_gfx1250_insn_entry &insn_entry = ENC_SOP1_insn_table[layout.OP];
        this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
        finalizeENC_SOP1Operands();
        this->insn_in_progress->updateSize(insn_size + immLen,reinterpret_cast<unsigned char*>(&insn));
        this->insn_in_progress->updateMnemonic(std::string(insn_entry.mnemonic) + extension);
    }

    void InstructionDecoder_amdgpu_gfx1250::decodeENC_SOPC()
    {
        insn_size = 4;
        layout_ENC_SOPC & layout = insn_layout.ENC_SOPC;
        layout.ENCODING = longfield<23,31>(insn_long);
        layout.OP = longfield<16,22>(insn_long);
        layout.SSRC0 = longfield<0,7>(insn_long);
        layout.SSRC1 = longfield<8,15>(insn_long);
        assert(isArrayIndexValid(ENC_SOPC_insn_table, layout.OP) && "Opcode over or underflow");
        const amdgpu_gfx1250_insn_entry &insn_entry = ENC_SOPC_insn_table[layout.OP];
        this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
        finalizeENC_SOPCOperands();
        this->insn_in_progress->updateSize(insn_size + immLen,reinterpret_cast<unsigned char*>(&insn));
        this->insn_in_progress->updateMnemonic(std::string(insn_entry.mnemonic) + extension);
    }

    void InstructionDecoder_amdgpu_gfx1250::decodeENC_SOPP()
    {
        insn_size = 4;
        layout_ENC_SOPP & layout = insn_layout.ENC_SOPP;
        layout.ENCODING = longfield<23,31>(insn_long);
        layout.OP = longfield<16,22>(insn_long);
        layout.SIMM16 = longfield<0,15>(insn_long);
        assert(isArrayIndexValid(ENC_SOPP_insn_table, layout.OP) && "Opcode over or underflow");
        const amdgpu_gfx1250_insn_entry &insn_entry = ENC_SOPP_insn_table[layout.OP];
        this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
        finalizeENC_SOPPOperands();
        this->insn_in_progress->updateSize(insn_size + immLen,reinterpret_cast<unsigned char*>(&insn));
        this->insn_in_progress->updateMnemonic(std::string(insn_entry.mnemonic) + extension);
    }

    void InstructionDecoder_amdgpu_gfx1250::decodeENC_SOPK()
    {
        insn_size = 4;
        layout_ENC_SOPK & layout = insn_layout.ENC_SOPK;
        layout.ENCODING = longfield<28,31>(insn_long);
        layout.OP = longfield<23,27>(insn_long);
        layout.SDST = longfield<16,22>(insn_long);
        layout.SIMM16 = longfield<0,15>(insn_long);
        assert(isArrayIndexValid(ENC_SOPK_insn_table, layout.OP) && "Opcode over or underflow");
        const amdgpu_gfx1250_insn_entry &insn_entry = ENC_SOPK_insn_table[layout.OP];
        this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
        finalizeENC_SOPKOperands();
        this->insn_in_progress->updateSize(insn_size + immLen,reinterpret_cast<unsigned char*>(&insn));
        this->insn_in_progress->updateMnemonic(std::string(insn_entry.mnemonic) + extension);
    }

    void InstructionDecoder_amdgpu_gfx1250::decodeENC_SOP2()
    {
        insn_size = 4;
        layout_ENC_SOP2 & layout = insn_layout.ENC_SOP2;
        layout.ENCODING = longfield<30,31>(insn_long);
        layout.OP = longfield<23,29>(insn_long);
        layout.SDST = longfield<16,22>(insn_long);
        layout.SSRC0 = longfield<0,7>(insn_long);
        layout.SSRC1 = longfield<8,15>(insn_long);
        assert(isArrayIndexValid(ENC_SOP2_insn_table, layout.OP) && "Opcode over or underflow");
        const amdgpu_gfx1250_insn_entry &insn_entry = ENC_SOP2_insn_table[layout.OP];
        this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
        finalizeENC_SOP2Operands();
        this->insn_in_progress->updateSize(insn_size + immLen,reinterpret_cast<unsigned char*>(&insn));
        this->insn_in_progress->updateMnemonic(std::string(insn_entry.mnemonic) + extension);
    }

    void InstructionDecoder_amdgpu_gfx1250::decodeENC_SMEM()
    {
        insn_size = 8;
        layout_ENC_SMEM & layout = insn_layout.ENC_SMEM;
        layout.ENCODING = longfield<26,31>(insn_long);
        layout.IOFFSET = longfield<32,55>(insn_long);
        layout.NV = longfield<20,20>(insn_long);
        layout.OP = longfield<13,18>(insn_long);
        layout.SBASE = ((longfield<0,5>(insn_long) << 1) | 0);
        layout.SCALE_OFFSET = longfield<56,56>(insn_long);
        layout.SCOPE = longfield<21,22>(insn_long);
        layout.SDATA = longfield<6,12>(insn_long);
        layout.SOFFSET = longfield<57,63>(insn_long);
        layout.TH = longfield<23,24>(insn_long);
        assert(isArrayIndexValid(ENC_SMEM_insn_table, layout.OP) && "Opcode over or underflow");
        const amdgpu_gfx1250_insn_entry &insn_entry = ENC_SMEM_insn_table[layout.OP];
        this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
        finalizeENC_SMEMOperands();
        this->insn_in_progress->updateSize(insn_size + immLen,reinterpret_cast<unsigned char*>(&insn));
        this->insn_in_progress->updateMnemonic(std::string(insn_entry.mnemonic) + extension);
    }

    void InstructionDecoder_amdgpu_gfx1250::decodeENC_VOP1()
    {
        insn_size = 4;
        layout_ENC_VOP1 & layout = insn_layout.ENC_VOP1;
        layout.ENCODING = longfield<25,31>(insn_long);
        layout.OP = longfield<9,15>(insn_long);
        layout.SRC0 = longfield<0,8>(insn_long);
        layout.VDST = longfield<17,24>(insn_long);
        assert(isArrayIndexValid(ENC_VOP1_insn_table, layout.OP) && "Opcode over or underflow");
        const amdgpu_gfx1250_insn_entry &insn_entry = ENC_VOP1_insn_table[layout.OP];
        this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
        finalizeENC_VOP1Operands();
        this->insn_in_progress->updateSize(insn_size + immLen,reinterpret_cast<unsigned char*>(&insn));
        this->insn_in_progress->updateMnemonic(std::string(insn_entry.mnemonic) + extension);
    }

    void InstructionDecoder_amdgpu_gfx1250::decodeENC_VOPC()
    {
        insn_size = 4;
        layout_ENC_VOPC & layout = insn_layout.ENC_VOPC;
        layout.ENCODING = longfield<25,31>(insn_long);
        layout.OP = longfield<17,24>(insn_long);
        layout.SRC0 = longfield<0,8>(insn_long);
        layout.VSRC1 = longfield<9,16>(insn_long);
        assert(isArrayIndexValid(ENC_VOPC_insn_table, layout.OP) && "Opcode over or underflow");
        const amdgpu_gfx1250_insn_entry &insn_entry = ENC_VOPC_insn_table[layout.OP];
        this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
        finalizeENC_VOPCOperands();
        this->insn_in_progress->updateSize(insn_size + immLen,reinterpret_cast<unsigned char*>(&insn));
        this->insn_in_progress->updateMnemonic(std::string(insn_entry.mnemonic) + extension);
    }

    void InstructionDecoder_amdgpu_gfx1250::decodeENC_VOP2()
    {
        insn_size = 4;
        layout_ENC_VOP2 & layout = insn_layout.ENC_VOP2;
        layout.ENCODING = longfield<31,31>(insn_long);
        layout.OP = longfield<25,30>(insn_long);
        layout.SRC0 = longfield<0,8>(insn_long);
        layout.VDST = longfield<17,24>(insn_long);
        layout.VSRC1 = longfield<9,16>(insn_long);
        assert(isArrayIndexValid(ENC_VOP2_insn_table, layout.OP) && "Opcode over or underflow");
        const amdgpu_gfx1250_insn_entry &insn_entry = ENC_VOP2_insn_table[layout.OP];
        this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
        finalizeENC_VOP2Operands();
        this->insn_in_progress->updateSize(insn_size + immLen,reinterpret_cast<unsigned char*>(&insn));
        this->insn_in_progress->updateMnemonic(std::string(insn_entry.mnemonic) + extension);
    }

    void InstructionDecoder_amdgpu_gfx1250::decodeENC_VOP3()
    {
        insn_size = 8;
        layout_ENC_VOP3 & layout = insn_layout.ENC_VOP3;
        layout.ABS = longfield<8,10>(insn_long);
        layout.CLAMP = longfield<15,15>(insn_long);
        layout.ENCODING = longfield<26,31>(insn_long);
        layout.NEG = longfield<61,63>(insn_long);
        layout.OMOD = longfield<59,60>(insn_long);
        layout.OP = longfield<16,25>(insn_long);
        layout.OPSEL = longfield<11,14>(insn_long);
        layout.SRC0 = longfield<32,40>(insn_long);
        layout.SRC1 = longfield<41,49>(insn_long);
        layout.SRC2 = longfield<50,58>(insn_long);
        layout.VDST = longfield<0,7>(insn_long);
        assert(isArrayIndexValid(ENC_VOP3_insn_table, layout.OP) && "Opcode over or underflow");
        const amdgpu_gfx1250_insn_entry &insn_entry = ENC_VOP3_insn_table[layout.OP];
        this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
        finalizeENC_VOP3Operands();
        this->insn_in_progress->updateSize(insn_size + immLen,reinterpret_cast<unsigned char*>(&insn));
        this->insn_in_progress->updateMnemonic(std::string(insn_entry.mnemonic) + extension);
    }

    void InstructionDecoder_amdgpu_gfx1250::decodeENC_VOP3PX2()
    {
        insn_size = 16;
        layout_ENC_VOP3PX2 & layout = insn_layout.ENC_VOP3PX2;
        layout.CLAMP = longfield<15,15>(insn_long2);
        layout.ENCODING = longfield<24,31>(insn_long2);
        layout.NEG = longfield<61,63>(insn_long2);
        layout.NEG_HI = longfield<8,10>(insn_long2);
        layout.OP = longfield<16,23>(insn_long2);
        layout.OPSEL = longfield<11,13>(insn_long2);
        layout.OPSEL_HI = longfield<59,60>(insn_long2) | (longfield<14,14>(insn_long2) << 2) | (longfield<14,14>(insn_long) << 3);
        layout.SCALE_CLAMP = longfield<15,15>(insn_long);
        layout.SCALE_NEG = longfield<61,63>(insn_long);
        layout.SCALE_NEG_HI = longfield<8,10>(insn_long);
        layout.SCALE_OPSEL = longfield<11,13>(insn_long);
        layout.SCALE_OPSEL_HI = longfield<59,60>(insn_long);
        layout.SCALE_SRC0 = longfield<32,40>(insn_long);
        layout.SCALE_SRC1 = longfield<41,49>(insn_long);
        layout.SCALE_SRC2 = longfield<50,58>(insn_long);
        layout.SRC0 = longfield<32,40>(insn_long2);
        layout.SRC1 = longfield<41,49>(insn_long2);
        layout.SRC2 = longfield<50,58>(insn_long2);
        layout.VDST = longfield<0,7>(insn_long2);
        layout.X2ENCODING = longfield<16,31>(insn_long);
        assert(isArrayIndexValid(ENC_VOP3PX2_insn_table, layout.OP) && "Opcode over or underflow");
        const amdgpu_gfx1250_insn_entry &insn_entry = ENC_VOP3PX2_insn_table[layout.OP];
        this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
        finalizeENC_VOP3PX2Operands();
        this->insn_in_progress->updateSize(insn_size + immLen,reinterpret_cast<unsigned char*>(&insn));
        this->insn_in_progress->updateMnemonic(std::string(insn_entry.mnemonic) + extension);
    }

    void InstructionDecoder_amdgpu_gfx1250::decodeENC_VOP3PX3()
    {
        insn_size = 16;
        layout_ENC_VOP3PX3 & layout = insn_layout.ENC_VOP3PX3;
        layout.CLAMP = longfield<15,15>(insn_long2);
        layout.ENCODING = longfield<24,31>(insn_long2);
        layout.NEG = longfield<61,63>(insn_long2);
        layout.NEG_HI = longfield<8,10>(insn_long2);
        layout.OP = longfield<16,23>(insn_long2);
        layout.OPSEL = longfield<11,13>(insn_long2);
        layout.OPSEL_HI = longfield<59,60>(insn_long2) | (longfield<14,14>(insn_long2) << 2) | (longfield<14,14>(insn_long) << 3);
        layout.SCALE_CLAMP = longfield<15,15>(insn_long);
        layout.SCALE_NEG = longfield<61,63>(insn_long);
        layout.SCALE_NEG_HI = longfield<8,10>(insn_long);
        layout.SCALE_OPSEL = longfield<11,13>(insn_long);
        layout.SCALE_OPSEL_HI = longfield<59,60>(insn_long);
        layout.SCALE_SRC0 = longfield<32,40>(insn_long);
        layout.SCALE_SRC1 = longfield<41,49>(insn_long);
        layout.SCALE_SRC2 = longfield<50,58>(insn_long);
        layout.SRC0 = longfield<32,40>(insn_long2);
        layout.SRC1 = longfield<41,49>(insn_long2);
        layout.SRC2 = longfield<50,58>(insn_long2);
        layout.VDST = longfield<0,7>(insn_long2);
        layout.X3ENCODING = longfield<16,31>(insn_long);
        assert(isArrayIndexValid(ENC_VOP3PX3_insn_table, layout.OP) && "Opcode over or underflow");
        const amdgpu_gfx1250_insn_entry &insn_entry = ENC_VOP3PX3_insn_table[layout.OP];
        this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
        finalizeENC_VOP3PX3Operands();
        this->insn_in_progress->updateSize(insn_size + immLen,reinterpret_cast<unsigned char*>(&insn));
        this->insn_in_progress->updateMnemonic(std::string(insn_entry.mnemonic) + extension);
    }

    void InstructionDecoder_amdgpu_gfx1250::decodeENC_VOP3P()
    {
        insn_size = 8;
        layout_ENC_VOP3P & layout = insn_layout.ENC_VOP3P;
        layout.CLAMP = longfield<15,15>(insn_long);
        layout.ENCODING = longfield<24,31>(insn_long);
        layout.NEG = longfield<61,63>(insn_long);
        layout.NEG_HI = longfield<8,10>(insn_long);
        layout.OP = longfield<16,23>(insn_long);
        layout.OPSEL = longfield<11,13>(insn_long);
        layout.OPSEL_HI = longfield<59,60>(insn_long) | (longfield<14,14>(insn_long) << 2);
        layout.SRC0 = longfield<32,40>(insn_long);
        layout.SRC1 = longfield<41,49>(insn_long);
        layout.SRC2 = longfield<50,58>(insn_long);
        layout.VDST = longfield<0,7>(insn_long);
        assert(isArrayIndexValid(ENC_VOP3P_insn_table, layout.OP) && "Opcode over or underflow");
        const amdgpu_gfx1250_insn_entry &insn_entry = ENC_VOP3P_insn_table[layout.OP];
        this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
        finalizeENC_VOP3POperands();
        this->insn_in_progress->updateSize(insn_size + immLen,reinterpret_cast<unsigned char*>(&insn));
        this->insn_in_progress->updateMnemonic(std::string(insn_entry.mnemonic) + extension);
    }

    void InstructionDecoder_amdgpu_gfx1250::decodeENC_VDS()
    {
        insn_size = 8;
        layout_ENC_VDS & layout = insn_layout.ENC_VDS;
        layout.ADDR = longfield<32,39>(insn_long);
        layout.DATA0 = longfield<40,47>(insn_long);
        layout.DATA1 = longfield<48,55>(insn_long);
        layout.ENCODING = longfield<26,31>(insn_long);
        layout.OFFSET0 = longfield<0,7>(insn_long);
        layout.OFFSET1 = longfield<8,15>(insn_long);
        layout.OP = longfield<18,25>(insn_long);
        layout.VDST = longfield<56,63>(insn_long);
        assert(isArrayIndexValid(ENC_VDS_insn_table, layout.OP) && "Opcode over or underflow");
        const amdgpu_gfx1250_insn_entry &insn_entry = ENC_VDS_insn_table[layout.OP];
        this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
        finalizeENC_VDSOperands();
        this->insn_in_progress->updateSize(insn_size + immLen,reinterpret_cast<unsigned char*>(&insn));
        this->insn_in_progress->updateMnemonic(std::string(insn_entry.mnemonic) + extension);
    }

    void InstructionDecoder_amdgpu_gfx1250::decodeENC_VBUFFER()
    {
        insn_size = 12;
        layout_ENC_VBUFFER & layout = insn_layout.ENC_VBUFFER;
        layout.ENCODING = longfield<26,31>(insn_long);
        layout.FORMAT = longfield<55,61>(insn_long);
        layout.IDXEN = longfield<63,63>(insn_long);
        layout.IOFFSET = longfield<8,31>(insn_long2);
        layout.NV = longfield<7,7>(insn_long);
        layout.OFFEN = longfield<62,62>(insn_long);
        layout.OP = longfield<14,21>(insn_long);
        layout.RSRC = longfield<41,49>(insn_long);
        layout.SCOPE = longfield<50,51>(insn_long);
        layout.SOFFSET = longfield<0,6>(insn_long);
        layout.TFE = longfield<22,22>(insn_long);
        layout.TH = longfield<52,54>(insn_long);
        layout.VADDR = longfield<0,7>(insn_long2);
        layout.VDATA = longfield<32,39>(insn_long);
        assert(isArrayIndexValid(ENC_VBUFFER_insn_table, layout.OP) && "Opcode over or underflow");
        const amdgpu_gfx1250_insn_entry &insn_entry = ENC_VBUFFER_insn_table[layout.OP];
        this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
        finalizeENC_VBUFFEROperands();
        this->insn_in_progress->updateSize(insn_size + immLen,reinterpret_cast<unsigned char*>(&insn));
        this->insn_in_progress->updateMnemonic(std::string(insn_entry.mnemonic) + extension);
    }

    void InstructionDecoder_amdgpu_gfx1250::decodeENC_VIMAGE()
    {
        insn_size = 12;
        layout_ENC_VIMAGE & layout = insn_layout.ENC_VIMAGE;
        layout.A16 = longfield<6,6>(insn_long);
        layout.D16 = longfield<5,5>(insn_long);
        layout.DIM = longfield<0,2>(insn_long);
        layout.DMASK = longfield<22,25>(insn_long);
        layout.ENCODING = longfield<26,31>(insn_long);
        layout.NV = longfield<7,7>(insn_long);
        layout.OP = longfield<14,21>(insn_long);
        layout.R128 = longfield<4,4>(insn_long);
        layout.RSRC = longfield<41,49>(insn_long);
        layout.SCOPE = longfield<50,51>(insn_long);
        layout.TFE = longfield<55,55>(insn_long);
        layout.TH = longfield<52,54>(insn_long);
        layout.VADDR0 = longfield<0,7>(insn_long2);
        layout.VADDR1 = longfield<8,15>(insn_long2);
        layout.VADDR2 = longfield<16,23>(insn_long2);
        layout.VADDR3 = longfield<24,31>(insn_long2);
        layout.VADDR4 = longfield<56,63>(insn_long);
        layout.VDATA = longfield<32,39>(insn_long);
        assert(isArrayIndexValid(ENC_VIMAGE_insn_table, layout.OP) && "Opcode over or underflow");
        const amdgpu_gfx1250_insn_entry &insn_entry = ENC_VIMAGE_insn_table[layout.OP];
        this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
        finalizeENC_VIMAGEOperands();
        this->insn_in_progress->updateSize(insn_size + immLen,reinterpret_cast<unsigned char*>(&insn));
        this->insn_in_progress->updateMnemonic(std::string(insn_entry.mnemonic) + extension);
    }

    void InstructionDecoder_amdgpu_gfx1250::decodeENC_VFLAT()
    {
        insn_size = 12;
        layout_ENC_VFLAT & layout = insn_layout.ENC_VFLAT;
        layout.ENCODING = longfield<24,31>(insn_long);
        layout.IOFFSET = longfield<8,31>(insn_long2);
        layout.NV = longfield<7,7>(insn_long);
        layout.OP = longfield<14,21>(insn_long);
        layout.SADDR = longfield<0,6>(insn_long);
        layout.SCALE_OFFSET = longfield<48,48>(insn_long);
        layout.SCOPE = longfield<50,51>(insn_long);
        layout.SVE = longfield<49,49>(insn_long);
        layout.TH = longfield<52,54>(insn_long);
        layout.VADDR = longfield<0,7>(insn_long2);
        layout.VDST = longfield<32,39>(insn_long);
        layout.VSRC = longfield<55,62>(insn_long);
        assert(isArrayIndexValid(ENC_VFLAT_insn_table, layout.OP) && "Opcode over or underflow");
        const amdgpu_gfx1250_insn_entry &insn_entry = ENC_VFLAT_insn_table[layout.OP];
        this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
        finalizeENC_VFLATOperands();
        this->insn_in_progress->updateSize(insn_size + immLen,reinterpret_cast<unsigned char*>(&insn));
        this->insn_in_progress->updateMnemonic(std::string(insn_entry.mnemonic) + extension);
    }

    void InstructionDecoder_amdgpu_gfx1250::decodeENC_VSCRATCH()
    {
        insn_size = 12;
        layout_ENC_VSCRATCH & layout = insn_layout.ENC_VSCRATCH;
        layout.ENCODING = longfield<24,31>(insn_long);
        layout.IOFFSET = longfield<8,31>(insn_long2);
        layout.NV = longfield<7,7>(insn_long);
        layout.OP = longfield<14,21>(insn_long);
        layout.SADDR = longfield<0,6>(insn_long);
        layout.SCALE_OFFSET = longfield<48,48>(insn_long);
        layout.SCOPE = longfield<50,51>(insn_long);
        layout.SVE = longfield<49,49>(insn_long);
        layout.TH = longfield<52,54>(insn_long);
        layout.VADDR = longfield<0,7>(insn_long2);
        layout.VDST = longfield<32,39>(insn_long);
        layout.VSRC = longfield<55,62>(insn_long);
        assert(isArrayIndexValid(ENC_VSCRATCH_insn_table, layout.OP) && "Opcode over or underflow");
        const amdgpu_gfx1250_insn_entry &insn_entry = ENC_VSCRATCH_insn_table[layout.OP];
        this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
        finalizeENC_VSCRATCHOperands();
        this->insn_in_progress->updateSize(insn_size + immLen,reinterpret_cast<unsigned char*>(&insn));
        this->insn_in_progress->updateMnemonic(std::string(insn_entry.mnemonic) + extension);
    }

    void InstructionDecoder_amdgpu_gfx1250::decodeENC_VGLOBAL()
    {
        insn_size = 12;
        layout_ENC_VGLOBAL & layout = insn_layout.ENC_VGLOBAL;
        layout.ENCODING = longfield<24,31>(insn_long);
        layout.IOFFSET = longfield<8,31>(insn_long2);
        layout.NV = longfield<7,7>(insn_long);
        layout.OP = longfield<14,21>(insn_long);
        layout.SADDR = longfield<0,6>(insn_long);
        layout.SCALE_OFFSET = longfield<48,48>(insn_long);
        layout.SCOPE = longfield<50,51>(insn_long);
        layout.SVE = longfield<49,49>(insn_long);
        layout.TH = longfield<52,54>(insn_long);
        layout.VADDR = longfield<0,7>(insn_long2);
        layout.VDST = longfield<32,39>(insn_long);
        layout.VSRC = longfield<55,62>(insn_long);
        assert(isArrayIndexValid(ENC_VGLOBAL_insn_table, layout.OP) && "Opcode over or underflow");
        const amdgpu_gfx1250_insn_entry &insn_entry = ENC_VGLOBAL_insn_table[layout.OP];
        this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
        finalizeENC_VGLOBALOperands();
        this->insn_in_progress->updateSize(insn_size + immLen,reinterpret_cast<unsigned char*>(&insn));
        this->insn_in_progress->updateMnemonic(std::string(insn_entry.mnemonic) + extension);
    }

    void InstructionDecoder_amdgpu_gfx1250::decodeSOP1_INST_LITERAL()
    {
        insn_size = 8;
        layout_SOP1_INST_LITERAL & layout = insn_layout.SOP1_INST_LITERAL;
        layout.ENCODING = longfield<23,31>(insn_long);
        layout.LITERAL = longfield<32,63>(insn_long);
        layout.OP = longfield<8,15>(insn_long);
        layout.SDST = longfield<16,22>(insn_long);
        layout.SSRC0 = longfield<0,7>(insn_long);
        assert(isArrayIndexValid(SOP1_INST_LITERAL_insn_table, layout.OP) && "Opcode over or underflow");
        const amdgpu_gfx1250_insn_entry &insn_entry = SOP1_INST_LITERAL_insn_table[layout.OP];
        this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
        finalizeSOP1_INST_LITERALOperands();
        this->insn_in_progress->updateSize(insn_size + immLen,reinterpret_cast<unsigned char*>(&insn));
        this->insn_in_progress->updateMnemonic(std::string(insn_entry.mnemonic) + extension);
    }

    void InstructionDecoder_amdgpu_gfx1250::decodeSOP1_INST_LITERAL64()
    {
        insn_size = 12;
        layout_SOP1_INST_LITERAL64 & layout = insn_layout.SOP1_INST_LITERAL64;
        layout.ENCODING = longfield<23,31>(insn_long);
        layout.LITERAL64 = (longfield<32,63>(insn_long) | (longfield<0,31>(insn_long2) << 32));
        layout.OP = longfield<8,15>(insn_long);
        layout.SDST = longfield<16,22>(insn_long);
        layout.SSRC0 = longfield<0,7>(insn_long);
        assert(isArrayIndexValid(SOP1_INST_LITERAL64_insn_table, layout.OP) && "Opcode over or underflow");
        const amdgpu_gfx1250_insn_entry &insn_entry = SOP1_INST_LITERAL64_insn_table[layout.OP];
        this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
        finalizeSOP1_INST_LITERAL64Operands();
        this->insn_in_progress->updateSize(insn_size + immLen,reinterpret_cast<unsigned char*>(&insn));
        this->insn_in_progress->updateMnemonic(std::string(insn_entry.mnemonic) + extension);
    }

    void InstructionDecoder_amdgpu_gfx1250::decodeSOP2_INST_LITERAL()
    {
        insn_size = 8;
        layout_SOP2_INST_LITERAL & layout = insn_layout.SOP2_INST_LITERAL;
        layout.ENCODING = longfield<30,31>(insn_long);
        layout.LITERAL = longfield<32,63>(insn_long);
        layout.OP = longfield<23,29>(insn_long);
        layout.SDST = longfield<16,22>(insn_long);
        layout.SSRC0 = longfield<0,7>(insn_long);
        layout.SSRC1 = longfield<8,15>(insn_long);
        assert(isArrayIndexValid(SOP2_INST_LITERAL_insn_table, layout.OP) && "Opcode over or underflow");
        const amdgpu_gfx1250_insn_entry &insn_entry = SOP2_INST_LITERAL_insn_table[layout.OP];
        this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
        finalizeSOP2_INST_LITERALOperands();
        this->insn_in_progress->updateSize(insn_size + immLen,reinterpret_cast<unsigned char*>(&insn));
        this->insn_in_progress->updateMnemonic(std::string(insn_entry.mnemonic) + extension);
    }

    void InstructionDecoder_amdgpu_gfx1250::decodeSOP2_INST_LITERAL64()
    {
        insn_size = 12;
        layout_SOP2_INST_LITERAL64 & layout = insn_layout.SOP2_INST_LITERAL64;
        layout.ENCODING = longfield<30,31>(insn_long);
        layout.LITERAL64 = (longfield<32,63>(insn_long) | (longfield<0,31>(insn_long2) << 32));
        layout.OP = longfield<23,29>(insn_long);
        layout.SDST = longfield<16,22>(insn_long);
        layout.SSRC0 = longfield<0,7>(insn_long);
        layout.SSRC1 = longfield<8,15>(insn_long);
        assert(isArrayIndexValid(SOP2_INST_LITERAL64_insn_table, layout.OP) && "Opcode over or underflow");
        const amdgpu_gfx1250_insn_entry &insn_entry = SOP2_INST_LITERAL64_insn_table[layout.OP];
        this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
        finalizeSOP2_INST_LITERAL64Operands();
        this->insn_in_progress->updateSize(insn_size + immLen,reinterpret_cast<unsigned char*>(&insn));
        this->insn_in_progress->updateMnemonic(std::string(insn_entry.mnemonic) + extension);
    }

    void InstructionDecoder_amdgpu_gfx1250::decodeSOPC_INST_LITERAL()
    {
        insn_size = 8;
        layout_SOPC_INST_LITERAL & layout = insn_layout.SOPC_INST_LITERAL;
        layout.ENCODING = longfield<23,31>(insn_long);
        layout.LITERAL = longfield<32,63>(insn_long);
        layout.OP = longfield<16,22>(insn_long);
        layout.SSRC0 = longfield<0,7>(insn_long);
        layout.SSRC1 = longfield<8,15>(insn_long);
        assert(isArrayIndexValid(SOPC_INST_LITERAL_insn_table, layout.OP) && "Opcode over or underflow");
        const amdgpu_gfx1250_insn_entry &insn_entry = SOPC_INST_LITERAL_insn_table[layout.OP];
        this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
        finalizeSOPC_INST_LITERALOperands();
        this->insn_in_progress->updateSize(insn_size + immLen,reinterpret_cast<unsigned char*>(&insn));
        this->insn_in_progress->updateMnemonic(std::string(insn_entry.mnemonic) + extension);
    }

    void InstructionDecoder_amdgpu_gfx1250::decodeSOPC_INST_LITERAL64()
    {
        insn_size = 12;
        layout_SOPC_INST_LITERAL64 & layout = insn_layout.SOPC_INST_LITERAL64;
        layout.ENCODING = longfield<23,31>(insn_long);
        layout.LITERAL64 = (longfield<32,63>(insn_long) | (longfield<0,31>(insn_long2) << 32));
        layout.OP = longfield<16,22>(insn_long);
        layout.SSRC0 = longfield<0,7>(insn_long);
        layout.SSRC1 = longfield<8,15>(insn_long);
        assert(isArrayIndexValid(SOPC_INST_LITERAL64_insn_table, layout.OP) && "Opcode over or underflow");
        const amdgpu_gfx1250_insn_entry &insn_entry = SOPC_INST_LITERAL64_insn_table[layout.OP];
        this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
        finalizeSOPC_INST_LITERAL64Operands();
        this->insn_in_progress->updateSize(insn_size + immLen,reinterpret_cast<unsigned char*>(&insn));
        this->insn_in_progress->updateMnemonic(std::string(insn_entry.mnemonic) + extension);
    }

    void InstructionDecoder_amdgpu_gfx1250::decodeSOPK_INST_LITERAL()
    {
        insn_size = 8;
        layout_SOPK_INST_LITERAL & layout = insn_layout.SOPK_INST_LITERAL;
        layout.ENCODING = longfield<28,31>(insn_long);
        layout.LITERAL = longfield<32,63>(insn_long);
        layout.OP = longfield<23,27>(insn_long);
        layout.SDST = longfield<16,22>(insn_long);
        layout.SIMM16 = longfield<0,15>(insn_long);
        assert(isArrayIndexValid(SOPK_INST_LITERAL_insn_table, layout.OP) && "Opcode over or underflow");
        const amdgpu_gfx1250_insn_entry &insn_entry = SOPK_INST_LITERAL_insn_table[layout.OP];
        this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
        finalizeSOPK_INST_LITERALOperands();
        this->insn_in_progress->updateSize(insn_size + immLen,reinterpret_cast<unsigned char*>(&insn));
        this->insn_in_progress->updateMnemonic(std::string(insn_entry.mnemonic) + extension);
    }

    void InstructionDecoder_amdgpu_gfx1250::decodeVOP1_INST_LITERAL()
    {
        insn_size = 8;
        layout_VOP1_INST_LITERAL & layout = insn_layout.VOP1_INST_LITERAL;
        layout.ENCODING = longfield<25,31>(insn_long);
        layout.LITERAL = longfield<32,63>(insn_long);
        layout.OP = longfield<9,15>(insn_long);
        layout.SRC0 = longfield<0,8>(insn_long);
        layout.VDST = longfield<17,24>(insn_long);
        assert(isArrayIndexValid(VOP1_INST_LITERAL_insn_table, layout.OP) && "Opcode over or underflow");
        const amdgpu_gfx1250_insn_entry &insn_entry = VOP1_INST_LITERAL_insn_table[layout.OP];
        this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
        finalizeVOP1_INST_LITERALOperands();
        this->insn_in_progress->updateSize(insn_size + immLen,reinterpret_cast<unsigned char*>(&insn));
        this->insn_in_progress->updateMnemonic(std::string(insn_entry.mnemonic) + extension);
    }

    void InstructionDecoder_amdgpu_gfx1250::decodeVOP1_INST_LITERAL64()
    {
        insn_size = 12;
        layout_VOP1_INST_LITERAL64 & layout = insn_layout.VOP1_INST_LITERAL64;
        layout.ENCODING = longfield<25,31>(insn_long);
        layout.LITERAL64 = (longfield<32,63>(insn_long) | (longfield<0,31>(insn_long2) << 32));
        layout.OP = longfield<9,15>(insn_long);
        layout.SRC0 = longfield<0,8>(insn_long);
        layout.VDST = longfield<17,24>(insn_long);
        assert(isArrayIndexValid(VOP1_INST_LITERAL64_insn_table, layout.OP) && "Opcode over or underflow");
        const amdgpu_gfx1250_insn_entry &insn_entry = VOP1_INST_LITERAL64_insn_table[layout.OP];
        this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
        finalizeVOP1_INST_LITERAL64Operands();
        this->insn_in_progress->updateSize(insn_size + immLen,reinterpret_cast<unsigned char*>(&insn));
        this->insn_in_progress->updateMnemonic(std::string(insn_entry.mnemonic) + extension);
    }

    void InstructionDecoder_amdgpu_gfx1250::decodeVOP1_VOP_DPP8()
    {
        insn_size = 8;
        extension = std::string("_DPP");
        layout_VOP1_VOP_DPP8 & layout = insn_layout.VOP1_VOP_DPP8;
        layout.ENCODING = longfield<25,31>(insn_long);
        layout.LANE_SEL_0 = longfield<40,42>(insn_long);
        layout.LANE_SEL_1 = longfield<43,45>(insn_long);
        layout.LANE_SEL_2 = longfield<46,48>(insn_long);
        layout.LANE_SEL_3 = longfield<49,51>(insn_long);
        layout.LANE_SEL_4 = longfield<52,54>(insn_long);
        layout.LANE_SEL_5 = longfield<55,57>(insn_long);
        layout.LANE_SEL_6 = longfield<58,60>(insn_long);
        layout.LANE_SEL_7 = longfield<61,63>(insn_long);
        layout.OP = longfield<9,15>(insn_long);
        layout.SRC0 = longfield<0,8>(insn_long);
        layout.VDST = longfield<17,24>(insn_long);
        layout.VSRC0 = longfield<32,39>(insn_long);
        assert(isArrayIndexValid(VOP1_VOP_DPP8_insn_table, layout.OP) && "Opcode over or underflow");
        const amdgpu_gfx1250_insn_entry &insn_entry = VOP1_VOP_DPP8_insn_table[layout.OP];
        this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
        finalizeVOP1_VOP_DPP8Operands();
        this->insn_in_progress->updateSize(insn_size + immLen,reinterpret_cast<unsigned char*>(&insn));
        this->insn_in_progress->updateMnemonic(std::string(insn_entry.mnemonic) + extension);
    }

    void InstructionDecoder_amdgpu_gfx1250::decodeVOP1_VOP_DPP16()
    {
        insn_size = 8;
        extension = std::string("_DPP");
        layout_VOP1_VOP_DPP16 & layout = insn_layout.VOP1_VOP_DPP16;
        layout.BANK_MASK = longfield<56,59>(insn_long);
        layout.BOUND_CTRL = longfield<51,51>(insn_long);
        layout.DPP_CTRL = longfield<40,48>(insn_long);
        layout.ENCODING = longfield<25,31>(insn_long);
        layout.FI = longfield<50,50>(insn_long);
        layout.OP = longfield<9,15>(insn_long);
        layout.ROW_MASK = longfield<60,63>(insn_long);
        layout.SRC0 = longfield<0,8>(insn_long);
        layout.SRC0_ABS = longfield<53,53>(insn_long);
        layout.SRC0_NEG = longfield<52,52>(insn_long);
        layout.SRC1_ABS = longfield<55,55>(insn_long);
        layout.SRC1_NEG = longfield<54,54>(insn_long);
        layout.VDST = longfield<17,24>(insn_long);
        layout.VSRC0 = longfield<32,39>(insn_long);
        assert(isArrayIndexValid(VOP1_VOP_DPP16_insn_table, layout.OP) && "Opcode over or underflow");
        const amdgpu_gfx1250_insn_entry &insn_entry = VOP1_VOP_DPP16_insn_table[layout.OP];
        this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
        finalizeVOP1_VOP_DPP16Operands();
        this->insn_in_progress->updateSize(insn_size + immLen,reinterpret_cast<unsigned char*>(&insn));
        this->insn_in_progress->updateMnemonic(std::string(insn_entry.mnemonic) + extension);
    }

    void InstructionDecoder_amdgpu_gfx1250::decodeVOP3_INST_LITERAL()
    {
        insn_size = 12;
        layout_VOP3_INST_LITERAL & layout = insn_layout.VOP3_INST_LITERAL;
        layout.ABS = longfield<8,10>(insn_long);
        layout.CLAMP = longfield<15,15>(insn_long);
        layout.ENCODING = longfield<26,31>(insn_long);
        layout.LITERAL = longfield<0,31>(insn_long2);
        layout.NEG = longfield<61,63>(insn_long);
        layout.OMOD = longfield<59,60>(insn_long);
        layout.OP = longfield<16,25>(insn_long);
        layout.OPSEL = longfield<11,14>(insn_long);
        layout.SRC0 = longfield<32,40>(insn_long);
        layout.SRC1 = longfield<41,49>(insn_long);
        layout.SRC2 = longfield<50,58>(insn_long);
        layout.VDST = longfield<0,7>(insn_long);
        assert(isArrayIndexValid(VOP3_INST_LITERAL_insn_table, layout.OP) && "Opcode over or underflow");
        const amdgpu_gfx1250_insn_entry &insn_entry = VOP3_INST_LITERAL_insn_table[layout.OP];
        this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
        finalizeVOP3_INST_LITERALOperands();
        this->insn_in_progress->updateSize(insn_size + immLen,reinterpret_cast<unsigned char*>(&insn));
        this->insn_in_progress->updateMnemonic(std::string(insn_entry.mnemonic) + extension);
    }

    void InstructionDecoder_amdgpu_gfx1250::decodeVOP3_VOP_DPP8()
    {
        insn_size = 12;
        extension = std::string("_DPP");
        layout_VOP3_VOP_DPP8 & layout = insn_layout.VOP3_VOP_DPP8;
        layout.ABS = longfield<8,10>(insn_long);
        layout.CLAMP = longfield<15,15>(insn_long);
        layout.ENCODING = longfield<26,31>(insn_long);
        layout.LANE_SEL_0 = longfield<8,10>(insn_long2);
        layout.LANE_SEL_1 = longfield<11,13>(insn_long2);
        layout.LANE_SEL_2 = longfield<14,16>(insn_long2);
        layout.LANE_SEL_3 = longfield<17,19>(insn_long2);
        layout.LANE_SEL_4 = longfield<20,22>(insn_long2);
        layout.LANE_SEL_5 = longfield<23,25>(insn_long2);
        layout.LANE_SEL_6 = longfield<26,28>(insn_long2);
        layout.LANE_SEL_7 = longfield<29,31>(insn_long2);
        layout.NEG = longfield<61,63>(insn_long);
        layout.OMOD = longfield<59,60>(insn_long);
        layout.OP = longfield<16,25>(insn_long);
        layout.OPSEL = longfield<11,14>(insn_long);
        layout.SRC0 = longfield<32,40>(insn_long);
        layout.SRC1 = longfield<41,49>(insn_long);
        layout.SRC2 = longfield<50,58>(insn_long);
        layout.VDST = longfield<0,7>(insn_long);
        layout.VSRC0 = longfield<0,7>(insn_long2);
        assert(isArrayIndexValid(VOP3_VOP_DPP8_insn_table, layout.OP) && "Opcode over or underflow");
        const amdgpu_gfx1250_insn_entry &insn_entry = VOP3_VOP_DPP8_insn_table[layout.OP];
        this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
        finalizeVOP3_VOP_DPP8Operands();
        this->insn_in_progress->updateSize(insn_size + immLen,reinterpret_cast<unsigned char*>(&insn));
        this->insn_in_progress->updateMnemonic(std::string(insn_entry.mnemonic) + extension);
    }

    void InstructionDecoder_amdgpu_gfx1250::decodeVOP3_VOP_DPP16()
    {
        insn_size = 12;
        extension = std::string("_DPP");
        layout_VOP3_VOP_DPP16 & layout = insn_layout.VOP3_VOP_DPP16;
        layout.ABS = longfield<8,10>(insn_long);
        layout.BANK_MASK = longfield<24,27>(insn_long2);
        layout.BOUND_CTRL = longfield<19,19>(insn_long2);
        layout.CLAMP = longfield<15,15>(insn_long);
        layout.DPP_CTRL = longfield<8,16>(insn_long2);
        layout.ENCODING = longfield<26,31>(insn_long);
        layout.FI = longfield<18,18>(insn_long2);
        layout.NEG = longfield<61,63>(insn_long);
        layout.OMOD = longfield<59,60>(insn_long);
        layout.OP = longfield<16,25>(insn_long);
        layout.OPSEL = longfield<11,14>(insn_long);
        layout.ROW_MASK = longfield<28,31>(insn_long2);
        layout.SRC0 = longfield<32,40>(insn_long);
        layout.SRC0_ABS = longfield<21,21>(insn_long2);
        layout.SRC0_NEG = longfield<20,20>(insn_long2);
        layout.SRC1 = longfield<41,49>(insn_long);
        layout.SRC1_ABS = longfield<23,23>(insn_long2);
        layout.SRC1_NEG = longfield<22,22>(insn_long2);
        layout.SRC2 = longfield<50,58>(insn_long);
        layout.VDST = longfield<0,7>(insn_long);
        layout.VSRC0 = longfield<0,7>(insn_long2);
        assert(isArrayIndexValid(VOP3_VOP_DPP16_insn_table, layout.OP) && "Opcode over or underflow");
        const amdgpu_gfx1250_insn_entry &insn_entry = VOP3_VOP_DPP16_insn_table[layout.OP];
        this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
        finalizeVOP3_VOP_DPP16Operands();
        this->insn_in_progress->updateSize(insn_size + immLen,reinterpret_cast<unsigned char*>(&insn));
        this->insn_in_progress->updateMnemonic(std::string(insn_entry.mnemonic) + extension);
    }

    void InstructionDecoder_amdgpu_gfx1250::decodeVOP2_INST_LITERAL()
    {
        insn_size = 8;
        layout_VOP2_INST_LITERAL & layout = insn_layout.VOP2_INST_LITERAL;
        layout.ENCODING = longfield<31,31>(insn_long);
        layout.LITERAL = longfield<32,63>(insn_long);
        layout.OP = longfield<25,30>(insn_long);
        layout.SRC0 = longfield<0,8>(insn_long);
        layout.VDST = longfield<17,24>(insn_long);
        layout.VSRC1 = longfield<9,16>(insn_long);
        assert(isArrayIndexValid(VOP2_INST_LITERAL_insn_table, layout.OP) && "Opcode over or underflow");
        const amdgpu_gfx1250_insn_entry &insn_entry = VOP2_INST_LITERAL_insn_table[layout.OP];
        this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
        finalizeVOP2_INST_LITERALOperands();
        this->insn_in_progress->updateSize(insn_size + immLen,reinterpret_cast<unsigned char*>(&insn));
        this->insn_in_progress->updateMnemonic(std::string(insn_entry.mnemonic) + extension);
    }

    void InstructionDecoder_amdgpu_gfx1250::decodeVOP2_INST_LITERAL64()
    {
        insn_size = 12;
        layout_VOP2_INST_LITERAL64 & layout = insn_layout.VOP2_INST_LITERAL64;
        layout.ENCODING = longfield<31,31>(insn_long);
        layout.LITERAL64 = (longfield<32,63>(insn_long) | (longfield<0,31>(insn_long2) << 32));
        layout.OP = longfield<25,30>(insn_long);
        layout.SRC0 = longfield<0,8>(insn_long);
        layout.VDST = longfield<17,24>(insn_long);
        layout.VSRC1 = longfield<9,16>(insn_long);
        assert(isArrayIndexValid(VOP2_INST_LITERAL64_insn_table, layout.OP) && "Opcode over or underflow");
        const amdgpu_gfx1250_insn_entry &insn_entry = VOP2_INST_LITERAL64_insn_table[layout.OP];
        this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
        finalizeVOP2_INST_LITERAL64Operands();
        this->insn_in_progress->updateSize(insn_size + immLen,reinterpret_cast<unsigned char*>(&insn));
        this->insn_in_progress->updateMnemonic(std::string(insn_entry.mnemonic) + extension);
    }

    void InstructionDecoder_amdgpu_gfx1250::decodeVOP2_VOP_DPP8()
    {
        insn_size = 8;
        extension = std::string("_DPP");
        layout_VOP2_VOP_DPP8 & layout = insn_layout.VOP2_VOP_DPP8;
        layout.ENCODING = longfield<31,31>(insn_long);
        layout.LANE_SEL_0 = longfield<40,42>(insn_long);
        layout.LANE_SEL_1 = longfield<43,45>(insn_long);
        layout.LANE_SEL_2 = longfield<46,48>(insn_long);
        layout.LANE_SEL_3 = longfield<49,51>(insn_long);
        layout.LANE_SEL_4 = longfield<52,54>(insn_long);
        layout.LANE_SEL_5 = longfield<55,57>(insn_long);
        layout.LANE_SEL_6 = longfield<58,60>(insn_long);
        layout.LANE_SEL_7 = longfield<61,63>(insn_long);
        layout.OP = longfield<25,30>(insn_long);
        layout.SRC0 = longfield<0,8>(insn_long);
        layout.VDST = longfield<17,24>(insn_long);
        layout.VSRC0 = longfield<32,39>(insn_long);
        layout.VSRC1 = longfield<9,16>(insn_long);
        assert(isArrayIndexValid(VOP2_VOP_DPP8_insn_table, layout.OP) && "Opcode over or underflow");
        const amdgpu_gfx1250_insn_entry &insn_entry = VOP2_VOP_DPP8_insn_table[layout.OP];
        this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
        finalizeVOP2_VOP_DPP8Operands();
        this->insn_in_progress->updateSize(insn_size + immLen,reinterpret_cast<unsigned char*>(&insn));
        this->insn_in_progress->updateMnemonic(std::string(insn_entry.mnemonic) + extension);
    }

    void InstructionDecoder_amdgpu_gfx1250::decodeVOP2_VOP_DPP16()
    {
        insn_size = 8;
        extension = std::string("_DPP");
        layout_VOP2_VOP_DPP16 & layout = insn_layout.VOP2_VOP_DPP16;
        layout.BANK_MASK = longfield<56,59>(insn_long);
        layout.BOUND_CTRL = longfield<51,51>(insn_long);
        layout.DPP_CTRL = longfield<40,48>(insn_long);
        layout.ENCODING = longfield<31,31>(insn_long);
        layout.FI = longfield<50,50>(insn_long);
        layout.OP = longfield<25,30>(insn_long);
        layout.ROW_MASK = longfield<60,63>(insn_long);
        layout.SRC0 = longfield<0,8>(insn_long);
        layout.SRC0_ABS = longfield<53,53>(insn_long);
        layout.SRC0_NEG = longfield<52,52>(insn_long);
        layout.SRC1_ABS = longfield<55,55>(insn_long);
        layout.SRC1_NEG = longfield<54,54>(insn_long);
        layout.VDST = longfield<17,24>(insn_long);
        layout.VSRC0 = longfield<32,39>(insn_long);
        layout.VSRC1 = longfield<9,16>(insn_long);
        assert(isArrayIndexValid(VOP2_VOP_DPP16_insn_table, layout.OP) && "Opcode over or underflow");
        const amdgpu_gfx1250_insn_entry &insn_entry = VOP2_VOP_DPP16_insn_table[layout.OP];
        this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
        finalizeVOP2_VOP_DPP16Operands();
        this->insn_in_progress->updateSize(insn_size + immLen,reinterpret_cast<unsigned char*>(&insn));
        this->insn_in_progress->updateMnemonic(std::string(insn_entry.mnemonic) + extension);
    }

    void InstructionDecoder_amdgpu_gfx1250::decodeVOP3_SDST_ENC()
    {
        insn_size = 8;
        layout_VOP3_SDST_ENC & layout = insn_layout.VOP3_SDST_ENC;
        layout.CLAMP = longfield<15,15>(insn_long);
        layout.ENCODING = longfield<26,31>(insn_long);
        layout.NEG = longfield<61,63>(insn_long);
        layout.OMOD = longfield<59,60>(insn_long);
        layout.OP = longfield<16,25>(insn_long);
        layout.SDST = longfield<8,14>(insn_long);
        layout.SRC0 = longfield<32,40>(insn_long);
        layout.SRC1 = longfield<41,49>(insn_long);
        layout.SRC2 = longfield<50,58>(insn_long);
        layout.VDST = longfield<0,7>(insn_long);
        assert(isArrayIndexValid(VOP3_SDST_ENC_insn_table, layout.OP) && "Opcode over or underflow");
        const amdgpu_gfx1250_insn_entry &insn_entry = VOP3_SDST_ENC_insn_table[layout.OP];
        this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
        finalizeVOP3_SDST_ENCOperands();
        this->insn_in_progress->updateSize(insn_size + immLen,reinterpret_cast<unsigned char*>(&insn));
        this->insn_in_progress->updateMnemonic(std::string(insn_entry.mnemonic) + extension);
    }

    void InstructionDecoder_amdgpu_gfx1250::decodeVOP3_SDST_ENC_INST_LITERAL()
    {
        insn_size = 12;
        layout_VOP3_SDST_ENC_INST_LITERAL & layout = insn_layout.VOP3_SDST_ENC_INST_LITERAL;
        layout.CLAMP = longfield<15,15>(insn_long);
        layout.ENCODING = longfield<26,31>(insn_long);
        layout.LITERAL = longfield<0,31>(insn_long2);
        layout.NEG = longfield<61,63>(insn_long);
        layout.OMOD = longfield<59,60>(insn_long);
        layout.OP = longfield<16,25>(insn_long);
        layout.SDST = longfield<8,14>(insn_long);
        layout.SRC0 = longfield<32,40>(insn_long);
        layout.SRC1 = longfield<41,49>(insn_long);
        layout.SRC2 = longfield<50,58>(insn_long);
        layout.VDST = longfield<0,7>(insn_long);
        assert(isArrayIndexValid(VOP3_SDST_ENC_INST_LITERAL_insn_table, layout.OP) && "Opcode over or underflow");
        const amdgpu_gfx1250_insn_entry &insn_entry = VOP3_SDST_ENC_INST_LITERAL_insn_table[layout.OP];
        this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
        finalizeVOP3_SDST_ENC_INST_LITERALOperands();
        this->insn_in_progress->updateSize(insn_size + immLen,reinterpret_cast<unsigned char*>(&insn));
        this->insn_in_progress->updateMnemonic(std::string(insn_entry.mnemonic) + extension);
    }

    void InstructionDecoder_amdgpu_gfx1250::decodeVOP3_SDST_ENC_VOP_DPP8()
    {
        insn_size = 12;
        extension = std::string("_DPP");
        layout_VOP3_SDST_ENC_VOP_DPP8 & layout = insn_layout.VOP3_SDST_ENC_VOP_DPP8;
        layout.CLAMP = longfield<15,15>(insn_long);
        layout.ENCODING = longfield<26,31>(insn_long);
        layout.LANE_SEL_0 = longfield<8,10>(insn_long2);
        layout.LANE_SEL_1 = longfield<11,13>(insn_long2);
        layout.LANE_SEL_2 = longfield<14,16>(insn_long2);
        layout.LANE_SEL_3 = longfield<17,19>(insn_long2);
        layout.LANE_SEL_4 = longfield<20,22>(insn_long2);
        layout.LANE_SEL_5 = longfield<23,25>(insn_long2);
        layout.LANE_SEL_6 = longfield<26,28>(insn_long2);
        layout.LANE_SEL_7 = longfield<29,31>(insn_long2);
        layout.NEG = longfield<61,63>(insn_long);
        layout.OMOD = longfield<59,60>(insn_long);
        layout.OP = longfield<16,25>(insn_long);
        layout.SDST = longfield<8,14>(insn_long);
        layout.SRC0 = longfield<32,40>(insn_long);
        layout.SRC1 = longfield<41,49>(insn_long);
        layout.SRC2 = longfield<50,58>(insn_long);
        layout.VDST = longfield<0,7>(insn_long);
        layout.VSRC0 = longfield<0,7>(insn_long2);
        assert(isArrayIndexValid(VOP3_SDST_ENC_VOP_DPP8_insn_table, layout.OP) && "Opcode over or underflow");
        const amdgpu_gfx1250_insn_entry &insn_entry = VOP3_SDST_ENC_VOP_DPP8_insn_table[layout.OP];
        this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
        finalizeVOP3_SDST_ENC_VOP_DPP8Operands();
        this->insn_in_progress->updateSize(insn_size + immLen,reinterpret_cast<unsigned char*>(&insn));
        this->insn_in_progress->updateMnemonic(std::string(insn_entry.mnemonic) + extension);
    }

    void InstructionDecoder_amdgpu_gfx1250::decodeVOP3_SDST_ENC_VOP_DPP16()
    {
        insn_size = 12;
        extension = std::string("_DPP");
        layout_VOP3_SDST_ENC_VOP_DPP16 & layout = insn_layout.VOP3_SDST_ENC_VOP_DPP16;
        layout.BANK_MASK = longfield<24,27>(insn_long2);
        layout.BOUND_CTRL = longfield<19,19>(insn_long2);
        layout.CLAMP = longfield<15,15>(insn_long);
        layout.DPP_CTRL = longfield<8,16>(insn_long2);
        layout.ENCODING = longfield<26,31>(insn_long);
        layout.FI = longfield<18,18>(insn_long2);
        layout.NEG = longfield<61,63>(insn_long);
        layout.OMOD = longfield<59,60>(insn_long);
        layout.OP = longfield<16,25>(insn_long);
        layout.ROW_MASK = longfield<28,31>(insn_long2);
        layout.SDST = longfield<8,14>(insn_long);
        layout.SRC0 = longfield<32,40>(insn_long);
        layout.SRC0_ABS = longfield<21,21>(insn_long2);
        layout.SRC0_NEG = longfield<20,20>(insn_long2);
        layout.SRC1 = longfield<41,49>(insn_long);
        layout.SRC1_ABS = longfield<23,23>(insn_long2);
        layout.SRC1_NEG = longfield<22,22>(insn_long2);
        layout.SRC2 = longfield<50,58>(insn_long);
        layout.VDST = longfield<0,7>(insn_long);
        layout.VSRC0 = longfield<0,7>(insn_long2);
        assert(isArrayIndexValid(VOP3_SDST_ENC_VOP_DPP16_insn_table, layout.OP) && "Opcode over or underflow");
        const amdgpu_gfx1250_insn_entry &insn_entry = VOP3_SDST_ENC_VOP_DPP16_insn_table[layout.OP];
        this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
        finalizeVOP3_SDST_ENC_VOP_DPP16Operands();
        this->insn_in_progress->updateSize(insn_size + immLen,reinterpret_cast<unsigned char*>(&insn));
        this->insn_in_progress->updateMnemonic(std::string(insn_entry.mnemonic) + extension);
    }

    void InstructionDecoder_amdgpu_gfx1250::decodeVOP3P_INST_LITERAL()
    {
        insn_size = 12;
        layout_VOP3P_INST_LITERAL & layout = insn_layout.VOP3P_INST_LITERAL;
        layout.CLAMP = longfield<15,15>(insn_long);
        layout.ENCODING = longfield<24,31>(insn_long);
        layout.LITERAL = longfield<0,31>(insn_long2);
        layout.NEG = longfield<61,63>(insn_long);
        layout.NEG_HI = longfield<8,10>(insn_long);
        layout.OP = longfield<16,23>(insn_long);
        layout.OPSEL = longfield<11,13>(insn_long);
        layout.OPSEL_HI = longfield<59,60>(insn_long) | (longfield<14,14>(insn_long) << 2);
        layout.SRC0 = longfield<32,40>(insn_long);
        layout.SRC1 = longfield<41,49>(insn_long);
        layout.SRC2 = longfield<50,58>(insn_long);
        layout.VDST = longfield<0,7>(insn_long);
        assert(isArrayIndexValid(VOP3P_INST_LITERAL_insn_table, layout.OP) && "Opcode over or underflow");
        const amdgpu_gfx1250_insn_entry &insn_entry = VOP3P_INST_LITERAL_insn_table[layout.OP];
        this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
        finalizeVOP3P_INST_LITERALOperands();
        this->insn_in_progress->updateSize(insn_size + immLen,reinterpret_cast<unsigned char*>(&insn));
        this->insn_in_progress->updateMnemonic(std::string(insn_entry.mnemonic) + extension);
    }

    void InstructionDecoder_amdgpu_gfx1250::decodeVOP3P_VOP_DPP8()
    {
        insn_size = 12;
        extension = std::string("_DPP");
        layout_VOP3P_VOP_DPP8 & layout = insn_layout.VOP3P_VOP_DPP8;
        layout.CLAMP = longfield<15,15>(insn_long);
        layout.ENCODING = longfield<24,31>(insn_long);
        layout.LANE_SEL_0 = longfield<8,10>(insn_long2);
        layout.LANE_SEL_1 = longfield<11,13>(insn_long2);
        layout.LANE_SEL_2 = longfield<14,16>(insn_long2);
        layout.LANE_SEL_3 = longfield<17,19>(insn_long2);
        layout.LANE_SEL_4 = longfield<20,22>(insn_long2);
        layout.LANE_SEL_5 = longfield<23,25>(insn_long2);
        layout.LANE_SEL_6 = longfield<26,28>(insn_long2);
        layout.LANE_SEL_7 = longfield<29,31>(insn_long2);
        layout.NEG = longfield<61,63>(insn_long);
        layout.NEG_HI = longfield<8,10>(insn_long);
        layout.OP = longfield<16,23>(insn_long);
        layout.OPSEL = longfield<11,13>(insn_long);
        layout.OPSEL_HI = longfield<59,60>(insn_long) | (longfield<14,14>(insn_long) << 2);
        layout.SRC0 = longfield<32,40>(insn_long);
        layout.SRC1 = longfield<41,49>(insn_long);
        layout.SRC2 = longfield<50,58>(insn_long);
        layout.VDST = longfield<0,7>(insn_long);
        layout.VSRC0 = longfield<0,7>(insn_long2);
        assert(isArrayIndexValid(VOP3P_VOP_DPP8_insn_table, layout.OP) && "Opcode over or underflow");
        const amdgpu_gfx1250_insn_entry &insn_entry = VOP3P_VOP_DPP8_insn_table[layout.OP];
        this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
        finalizeVOP3P_VOP_DPP8Operands();
        this->insn_in_progress->updateSize(insn_size + immLen,reinterpret_cast<unsigned char*>(&insn));
        this->insn_in_progress->updateMnemonic(std::string(insn_entry.mnemonic) + extension);
    }

    void InstructionDecoder_amdgpu_gfx1250::decodeVOP3P_VOP_DPP16()
    {
        insn_size = 12;
        extension = std::string("_DPP");
        layout_VOP3P_VOP_DPP16 & layout = insn_layout.VOP3P_VOP_DPP16;
        layout.BANK_MASK = longfield<24,27>(insn_long2);
        layout.BOUND_CTRL = longfield<19,19>(insn_long2);
        layout.CLAMP = longfield<15,15>(insn_long);
        layout.DPP_CTRL = longfield<8,16>(insn_long2);
        layout.ENCODING = longfield<24,31>(insn_long);
        layout.FI = longfield<18,18>(insn_long2);
        layout.NEG = longfield<61,63>(insn_long);
        layout.NEG_HI = longfield<8,10>(insn_long);
        layout.OP = longfield<16,23>(insn_long);
        layout.OPSEL = longfield<11,13>(insn_long);
        layout.OPSEL_HI = longfield<59,60>(insn_long) | (longfield<14,14>(insn_long) << 2);
        layout.ROW_MASK = longfield<28,31>(insn_long2);
        layout.SRC0 = longfield<32,40>(insn_long);
        layout.SRC0_ABS = longfield<21,21>(insn_long2);
        layout.SRC0_NEG = longfield<20,20>(insn_long2);
        layout.SRC1 = longfield<41,49>(insn_long);
        layout.SRC1_ABS = longfield<23,23>(insn_long2);
        layout.SRC1_NEG = longfield<22,22>(insn_long2);
        layout.SRC2 = longfield<50,58>(insn_long);
        layout.VDST = longfield<0,7>(insn_long);
        layout.VSRC0 = longfield<0,7>(insn_long2);
        assert(isArrayIndexValid(VOP3P_VOP_DPP16_insn_table, layout.OP) && "Opcode over or underflow");
        const amdgpu_gfx1250_insn_entry &insn_entry = VOP3P_VOP_DPP16_insn_table[layout.OP];
        this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
        finalizeVOP3P_VOP_DPP16Operands();
        this->insn_in_progress->updateSize(insn_size + immLen,reinterpret_cast<unsigned char*>(&insn));
        this->insn_in_progress->updateMnemonic(std::string(insn_entry.mnemonic) + extension);
    }

    void InstructionDecoder_amdgpu_gfx1250::decodeVOPC_INST_LITERAL()
    {
        insn_size = 8;
        layout_VOPC_INST_LITERAL & layout = insn_layout.VOPC_INST_LITERAL;
        layout.ENCODING = longfield<25,31>(insn_long);
        layout.LITERAL = longfield<32,63>(insn_long);
        layout.OP = longfield<17,24>(insn_long);
        layout.SRC0 = longfield<0,8>(insn_long);
        layout.VSRC1 = longfield<9,16>(insn_long);
        assert(isArrayIndexValid(VOPC_INST_LITERAL_insn_table, layout.OP) && "Opcode over or underflow");
        const amdgpu_gfx1250_insn_entry &insn_entry = VOPC_INST_LITERAL_insn_table[layout.OP];
        this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
        finalizeVOPC_INST_LITERALOperands();
        this->insn_in_progress->updateSize(insn_size + immLen,reinterpret_cast<unsigned char*>(&insn));
        this->insn_in_progress->updateMnemonic(std::string(insn_entry.mnemonic) + extension);
    }

    void InstructionDecoder_amdgpu_gfx1250::decodeVOPC_INST_LITERAL64()
    {
        insn_size = 12;
        layout_VOPC_INST_LITERAL64 & layout = insn_layout.VOPC_INST_LITERAL64;
        layout.ENCODING = longfield<25,31>(insn_long);
        layout.LITERAL64 = (longfield<32,63>(insn_long) | (longfield<0,31>(insn_long2) << 32));
        layout.OP = longfield<17,24>(insn_long);
        layout.SRC0 = longfield<0,8>(insn_long);
        layout.VSRC1 = longfield<9,16>(insn_long);
        assert(isArrayIndexValid(VOPC_INST_LITERAL64_insn_table, layout.OP) && "Opcode over or underflow");
        const amdgpu_gfx1250_insn_entry &insn_entry = VOPC_INST_LITERAL64_insn_table[layout.OP];
        this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
        finalizeVOPC_INST_LITERAL64Operands();
        this->insn_in_progress->updateSize(insn_size + immLen,reinterpret_cast<unsigned char*>(&insn));
        this->insn_in_progress->updateMnemonic(std::string(insn_entry.mnemonic) + extension);
    }

    void InstructionDecoder_amdgpu_gfx1250::decodeVOPC_VOP_DPP8()
    {
        insn_size = 8;
        extension = std::string("_DPP");
        layout_VOPC_VOP_DPP8 & layout = insn_layout.VOPC_VOP_DPP8;
        layout.ENCODING = longfield<25,31>(insn_long);
        layout.LANE_SEL_0 = longfield<40,42>(insn_long);
        layout.LANE_SEL_1 = longfield<43,45>(insn_long);
        layout.LANE_SEL_2 = longfield<46,48>(insn_long);
        layout.LANE_SEL_3 = longfield<49,51>(insn_long);
        layout.LANE_SEL_4 = longfield<52,54>(insn_long);
        layout.LANE_SEL_5 = longfield<55,57>(insn_long);
        layout.LANE_SEL_6 = longfield<58,60>(insn_long);
        layout.LANE_SEL_7 = longfield<61,63>(insn_long);
        layout.OP = longfield<17,24>(insn_long);
        layout.SRC0 = longfield<0,8>(insn_long);
        layout.VSRC0 = longfield<32,39>(insn_long);
        layout.VSRC1 = longfield<9,16>(insn_long);
        assert(isArrayIndexValid(VOPC_VOP_DPP8_insn_table, layout.OP) && "Opcode over or underflow");
        const amdgpu_gfx1250_insn_entry &insn_entry = VOPC_VOP_DPP8_insn_table[layout.OP];
        this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
        finalizeVOPC_VOP_DPP8Operands();
        this->insn_in_progress->updateSize(insn_size + immLen,reinterpret_cast<unsigned char*>(&insn));
        this->insn_in_progress->updateMnemonic(std::string(insn_entry.mnemonic) + extension);
    }

    void InstructionDecoder_amdgpu_gfx1250::decodeVOPC_VOP_DPP16()
    {
        insn_size = 8;
        extension = std::string("_DPP");
        layout_VOPC_VOP_DPP16 & layout = insn_layout.VOPC_VOP_DPP16;
        layout.BANK_MASK = longfield<56,59>(insn_long);
        layout.BOUND_CTRL = longfield<51,51>(insn_long);
        layout.DPP_CTRL = longfield<40,48>(insn_long);
        layout.ENCODING = longfield<25,31>(insn_long);
        layout.FI = longfield<50,50>(insn_long);
        layout.OP = longfield<17,24>(insn_long);
        layout.ROW_MASK = longfield<60,63>(insn_long);
        layout.SRC0 = longfield<0,8>(insn_long);
        layout.SRC0_ABS = longfield<53,53>(insn_long);
        layout.SRC0_NEG = longfield<52,52>(insn_long);
        layout.SRC1_ABS = longfield<55,55>(insn_long);
        layout.SRC1_NEG = longfield<54,54>(insn_long);
        layout.VSRC0 = longfield<32,39>(insn_long);
        layout.VSRC1 = longfield<9,16>(insn_long);
        assert(isArrayIndexValid(VOPC_VOP_DPP16_insn_table, layout.OP) && "Opcode over or underflow");
        const amdgpu_gfx1250_insn_entry &insn_entry = VOPC_VOP_DPP16_insn_table[layout.OP];
        this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
        finalizeVOPC_VOP_DPP16Operands();
        this->insn_in_progress->updateSize(insn_size + immLen,reinterpret_cast<unsigned char*>(&insn));
        this->insn_in_progress->updateMnemonic(std::string(insn_entry.mnemonic) + extension);
    }

    void InstructionDecoder_amdgpu_gfx1250::decodeHalfVOPD3XY_X()
    {
        layout_VOPD3XY_X & layout = insn_layout.VOPD3XY_X;
        layout.ENCODING = longfield<24,31>(insn_long);
        layout.NEGX = longfield<41,43>(insn_long);
        layout.NEGY = longfield<44,46>(insn_long);
        layout.OPX = longfield<18,23>(insn_long);
        layout.OPY = longfield<12,17>(insn_long);
        layout.SRCX0 = longfield<0,8>(insn_long);
        layout.SRCY0 = longfield<32,40>(insn_long);
        layout.VDSTX = longfield<0,7>(insn_long2);
        layout.VDSTY = longfield<24,31>(insn_long2);
        layout.VSRCX1 = longfield<48,55>(insn_long);
        layout.VSRCX2 = longfield<56,63>(insn_long);
        layout.VSRCY1 = longfield<8,15>(insn_long2);
        layout.VSRCY2 = longfield<16,23>(insn_long2);
        finalizeVOPD3XY_XOperands();
    }

    void InstructionDecoder_amdgpu_gfx1250::decodeVOPD3XY_X()
    {
        uint64_t I = insn_long;
        uint64_t opX = longfield<18,23>(I);
        uint64_t opY = longfield<12,17>(I);
        insn_size = 12;
        const amdgpu_gfx1250_insn_entry &entryX = VOPD3XY_X_insn_table[opX];
        const amdgpu_gfx1250_insn_entry &entryY = VOPD3XY_Y_insn_table[opY];
        this->insn_in_progress = makeInstruction(entryX.op,entryX.mnemonic,insn_size,reinterpret_cast<unsigned char *>(&insn));
        decodeHalfVOPD3XY_X();
        decodeHalfVOPD3XY_Y();
        this->insn_in_progress->updateSize(insn_size,reinterpret_cast<unsigned char*>(&insn));
        this->insn_in_progress->updateMnemonic(std::string(entryX.mnemonic) + "::" + entryY.mnemonic);
    }

    void InstructionDecoder_amdgpu_gfx1250::decodeHalfVOPD3XY_Y()
    {
        layout_VOPD3XY_Y & layout = insn_layout.VOPD3XY_Y;
        layout.ENCODING = longfield<24,31>(insn_long);
        layout.NEGX = longfield<41,43>(insn_long);
        layout.NEGY = longfield<44,46>(insn_long);
        layout.OPX = longfield<18,23>(insn_long);
        layout.OPY = longfield<12,17>(insn_long);
        layout.SRCX0 = longfield<0,8>(insn_long);
        layout.SRCY0 = longfield<32,40>(insn_long);
        layout.VDSTX = longfield<0,7>(insn_long2);
        layout.VDSTY = longfield<24,31>(insn_long2);
        layout.VSRCX1 = longfield<48,55>(insn_long);
        layout.VSRCX2 = longfield<56,63>(insn_long);
        layout.VSRCY1 = longfield<8,15>(insn_long2);
        layout.VSRCY2 = longfield<16,23>(insn_long2);
        finalizeVOPD3XY_YOperands();
    }

    void InstructionDecoder_amdgpu_gfx1250::decodeVOPD3XY_Y()
    {
        // decoded through its primary X encoding
    }

    void InstructionDecoder_amdgpu_gfx1250::decodeHalfVOPDXY_X()
    {
        layout_VOPDXY_X & layout = insn_layout.VOPDXY_X;
        layout.ENCODING = longfield<26,31>(insn_long);
        layout.OPX = longfield<22,25>(insn_long);
        layout.OPY = longfield<17,21>(insn_long);
        layout.SRCX0 = longfield<0,8>(insn_long);
        layout.SRCY0 = longfield<32,40>(insn_long);
        layout.VDSTX = longfield<56,63>(insn_long);
        layout.VDSTY = ((longfield<49,55>(insn_long) << 1) | 0);
        layout.VSRCX1 = longfield<9,16>(insn_long);
        layout.VSRCY1 = longfield<41,48>(insn_long);
        finalizeVOPDXY_XOperands();
    }

    void InstructionDecoder_amdgpu_gfx1250::decodeVOPDXY_X()
    {
        uint64_t I = insn_long;
        uint64_t opX = longfield<22,25>(I);
        bool litX = (longfield<0,8>(I)) == 255 || opX == 1 || opX == 2;
        uint64_t opY = longfield<17,21>(I);
        bool litY = (longfield<32,40>(I)) == 255 || opY == 1 || opY == 2;
        insn_size = (litX || litY) ? 12 : 8;
        const amdgpu_gfx1250_insn_entry &entryX = (litX ? VOPDXY_INST_LITERAL_X_insn_table : VOPDXY_X_insn_table)[opX];
        const amdgpu_gfx1250_insn_entry &entryY = (litY ? VOPDXY_INST_LITERAL_Y_insn_table : VOPDXY_Y_insn_table)[opY];
        this->insn_in_progress = makeInstruction(entryX.op,entryX.mnemonic,insn_size,reinterpret_cast<unsigned char *>(&insn));
        if (litX)  {
            decodeHalfVOPDXY_INST_LITERAL_X();
        } else  {
            decodeHalfVOPDXY_X();
        }
        if (litY)  {
            decodeHalfVOPDXY_INST_LITERAL_Y();
        } else  {
            decodeHalfVOPDXY_Y();
        }
        this->insn_in_progress->updateSize(insn_size,reinterpret_cast<unsigned char*>(&insn));
        this->insn_in_progress->updateMnemonic(std::string(entryX.mnemonic) + "::" + entryY.mnemonic);
    }

    void InstructionDecoder_amdgpu_gfx1250::decodeHalfVOPDXY_INST_LITERAL_X()
    {
        layout_VOPDXY_INST_LITERAL_X & layout = insn_layout.VOPDXY_INST_LITERAL_X;
        layout.ENCODING = longfield<26,31>(insn_long);
        layout.LITERAL = longfield<0,31>(insn_long2);
        layout.OPX = longfield<22,25>(insn_long);
        layout.OPY = longfield<17,21>(insn_long);
        layout.SRCX0 = longfield<0,8>(insn_long);
        layout.SRCY0 = longfield<32,40>(insn_long);
        layout.VDSTX = longfield<56,63>(insn_long);
        layout.VDSTY = ((longfield<49,55>(insn_long) << 1) | 0);
        layout.VSRCX1 = longfield<9,16>(insn_long);
        layout.VSRCY1 = longfield<41,48>(insn_long);
        finalizeVOPDXY_INST_LITERAL_XOperands();
    }

    void InstructionDecoder_amdgpu_gfx1250::decodeVOPDXY_INST_LITERAL_X()
    {
        // decoded through its primary X encoding
    }

    void InstructionDecoder_amdgpu_gfx1250::decodeHalfVOPDXY_Y()
    {
        layout_VOPDXY_Y & layout = insn_layout.VOPDXY_Y;
        layout.ENCODING = longfield<26,31>(insn_long);
        layout.OPX = longfield<22,25>(insn_long);
        layout.OPY = longfield<17,21>(insn_long);
        layout.SRCX0 = longfield<0,8>(insn_long);
        layout.SRCY0 = longfield<32,40>(insn_long);
        layout.VDSTX = longfield<56,63>(insn_long);
        layout.VDSTY = ((longfield<49,55>(insn_long) << 1) | 0);
        layout.VSRCX1 = longfield<9,16>(insn_long);
        layout.VSRCY1 = longfield<41,48>(insn_long);
        finalizeVOPDXY_YOperands();
    }

    void InstructionDecoder_amdgpu_gfx1250::decodeVOPDXY_Y()
    {
        // decoded through its primary X encoding
    }

    void InstructionDecoder_amdgpu_gfx1250::decodeHalfVOPDXY_INST_LITERAL_Y()
    {
        layout_VOPDXY_INST_LITERAL_Y & layout = insn_layout.VOPDXY_INST_LITERAL_Y;
        layout.ENCODING = longfield<26,31>(insn_long);
        layout.LITERAL = longfield<0,31>(insn_long2);
        layout.OPX = longfield<22,25>(insn_long);
        layout.OPY = longfield<17,21>(insn_long);
        layout.SRCX0 = longfield<0,8>(insn_long);
        layout.SRCY0 = longfield<32,40>(insn_long);
        layout.VDSTX = longfield<56,63>(insn_long);
        layout.VDSTY = ((longfield<49,55>(insn_long) << 1) | 0);
        layout.VSRCX1 = longfield<9,16>(insn_long);
        layout.VSRCY1 = longfield<41,48>(insn_long);
        finalizeVOPDXY_INST_LITERAL_YOperands();
    }

    void InstructionDecoder_amdgpu_gfx1250::decodeVOPDXY_INST_LITERAL_Y()
    {
        // decoded through its primary X encoding
    }

    void InstructionDecoder_amdgpu_gfx1250::mainDecode()
    {
        if (IS_SOP2_INST_LITERAL(insn_long) && FORCED_SOP2_INST_LITERAL(insn_long))  {
            decodeSOP2_INST_LITERAL();
        }
        else if (IS_SOPK_INST_LITERAL(insn_long) && FORCED_SOPK_INST_LITERAL(insn_long))  {
            decodeSOPK_INST_LITERAL();
        }
        else if (IS_VOP2_INST_LITERAL(insn_long) && FORCED_VOP2_INST_LITERAL(insn_long))  {
            decodeVOP2_INST_LITERAL();
        }
        else if (IS_VOP2_INST_LITERAL64(insn_long) && FORCED_VOP2_INST_LITERAL64(insn_long))  {
            decodeVOP2_INST_LITERAL64();
        }
        else if (IS_ENC_SOP1(insn_long) && (COND_ENC_SOP1_not_has_lit64_0_and_not_has_lit64_1_and_not_has_lit_0_and_not_has_lit_1(insn_long) || TRIVIAL_ENC_SOP1(insn_long)))  {
            decodeENC_SOP1();
        }
        else if (IS_ENC_SOPC(insn_long) && (COND_ENC_SOPC_default(insn_long)))  {
            decodeENC_SOPC();
        }
        else if (IS_ENC_SOPP(insn_long))  {
            decodeENC_SOPP();
        }
        else if (IS_ENC_SOPK(insn_long))  {
            decodeENC_SOPK();
        }
        else if (IS_ENC_SOP2(insn_long) && (COND_ENC_SOP2_default(insn_long)))  {
            decodeENC_SOP2();
        }
        else if (IS_ENC_SMEM(insn_long))  {
            decodeENC_SMEM();
        }
        else if (IS_ENC_VOP1(insn_long) && (COND_ENC_VOP1_default(insn_long)))  {
            decodeENC_VOP1();
        }
        else if (IS_ENC_VOPC(insn_long) && (COND_ENC_VOPC_default(insn_long)))  {
            decodeENC_VOPC();
        }
        else if (IS_ENC_VOP2(insn_long) && (COND_ENC_VOP2_default(insn_long)))  {
            decodeENC_VOP2();
        }
        else if (IS_ENC_VOP3(insn_long) && (COND_ENC_VOP3_default(insn_long)))  {
            decodeENC_VOP3();
        }
        else if (IS_ENC_VOP3PX2(insn_long))  {
            decodeENC_VOP3PX2();
        }
        else if (IS_ENC_VOP3PX3(insn_long))  {
            decodeENC_VOP3PX3();
        }
        else if (IS_ENC_VOP3P(insn_long) && (COND_ENC_VOP3P_default(insn_long)))  {
            decodeENC_VOP3P();
        }
        else if (IS_ENC_VDS(insn_long))  {
            decodeENC_VDS();
        }
        else if (IS_ENC_VBUFFER(insn_long))  {
            decodeENC_VBUFFER();
        }
        else if (IS_ENC_VIMAGE(insn_long))  {
            decodeENC_VIMAGE();
        }
        else if (IS_ENC_VFLAT(insn_long))  {
            decodeENC_VFLAT();
        }
        else if (IS_ENC_VSCRATCH(insn_long))  {
            decodeENC_VSCRATCH();
        }
        else if (IS_ENC_VGLOBAL(insn_long))  {
            decodeENC_VGLOBAL();
        }
        else if (IS_SOP1_INST_LITERAL(insn_long) && (COND_SOP1_INST_LITERAL_has_lit_0(insn_long)))  {
            decodeSOP1_INST_LITERAL();
        }
        else if (IS_SOP1_INST_LITERAL64(insn_long) && (COND_SOP1_INST_LITERAL64_has_lit64_0(insn_long)))  {
            decodeSOP1_INST_LITERAL64();
        }
        else if (IS_SOP2_INST_LITERAL(insn_long) && (COND_SOP2_INST_LITERAL_has_lit_0(insn_long) || COND_SOP2_INST_LITERAL_has_lit_1(insn_long) || COND_SOP2_INST_LITERAL_has_lit_0_and_has_lit_1(insn_long) || COND_SOP2_INST_LITERAL_default(insn_long)))  {
            decodeSOP2_INST_LITERAL();
        }
        else if (IS_SOP2_INST_LITERAL64(insn_long) && (COND_SOP2_INST_LITERAL64_has_lit64_0(insn_long) || COND_SOP2_INST_LITERAL64_has_lit64_1(insn_long) || COND_SOP2_INST_LITERAL64_has_lit64_0_and_has_lit64_1(insn_long)))  {
            decodeSOP2_INST_LITERAL64();
        }
        else if (IS_SOPC_INST_LITERAL(insn_long) && (COND_SOPC_INST_LITERAL_has_lit_0(insn_long) || COND_SOPC_INST_LITERAL_has_lit_1(insn_long) || COND_SOPC_INST_LITERAL_has_lit_0_and_has_lit_1(insn_long)))  {
            decodeSOPC_INST_LITERAL();
        }
        else if (IS_SOPC_INST_LITERAL64(insn_long) && (COND_SOPC_INST_LITERAL64_has_lit64_0(insn_long) || COND_SOPC_INST_LITERAL64_has_lit64_1(insn_long) || COND_SOPC_INST_LITERAL64_has_lit64_0_and_has_lit64_1(insn_long)))  {
            decodeSOPC_INST_LITERAL64();
        }
        else if (IS_SOPK_INST_LITERAL(insn_long))  {
            decodeSOPK_INST_LITERAL();
        }
        else if (IS_VOP1_INST_LITERAL(insn_long) && (COND_VOP1_INST_LITERAL_has_lit(insn_long)))  {
            decodeVOP1_INST_LITERAL();
        }
        else if (IS_VOP1_INST_LITERAL64(insn_long) && (COND_VOP1_INST_LITERAL64_has_lit64(insn_long)))  {
            decodeVOP1_INST_LITERAL64();
        }
        else if (IS_VOP1_VOP_DPP8(insn_long) && (COND_VOP1_VOP_DPP8_has_dpp8(insn_long)))  {
            decodeVOP1_VOP_DPP8();
        }
        else if (IS_VOP1_VOP_DPP16(insn_long) && (COND_VOP1_VOP_DPP16_has_dpp16(insn_long)))  {
            decodeVOP1_VOP_DPP16();
        }
        else if (IS_VOP3_INST_LITERAL(insn_long) && (COND_VOP3_INST_LITERAL_has_lit_0(insn_long) || COND_VOP3_INST_LITERAL_has_lit_1(insn_long) || COND_VOP3_INST_LITERAL_has_lit_0_and_has_lit_1(insn_long) || COND_VOP3_INST_LITERAL_has_lit_2(insn_long) || COND_VOP3_INST_LITERAL_has_lit_0_and_has_lit_2(insn_long) || COND_VOP3_INST_LITERAL_has_lit_1_and_has_lit_2(insn_long) || COND_VOP3_INST_LITERAL_has_lit_0_and_has_lit_1_and_has_lit_2(insn_long)))  {
            decodeVOP3_INST_LITERAL();
        }
        else if (IS_VOP3_VOP_DPP8(insn_long) && (COND_VOP3_VOP_DPP8_has_dpp8(insn_long)))  {
            decodeVOP3_VOP_DPP8();
        }
        else if (IS_VOP3_VOP_DPP16(insn_long) && (COND_VOP3_VOP_DPP16_has_dpp16(insn_long)))  {
            decodeVOP3_VOP_DPP16();
        }
        else if (IS_VOP2_INST_LITERAL(insn_long) && (COND_VOP2_INST_LITERAL_has_lit(insn_long) || COND_VOP2_INST_LITERAL_default(insn_long)))  {
            decodeVOP2_INST_LITERAL();
        }
        else if (IS_VOP2_INST_LITERAL64(insn_long) && (COND_VOP2_INST_LITERAL64_has_lit64(insn_long) || COND_VOP2_INST_LITERAL64_default(insn_long)))  {
            decodeVOP2_INST_LITERAL64();
        }
        else if (IS_VOP2_VOP_DPP8(insn_long) && (COND_VOP2_VOP_DPP8_has_dpp8(insn_long)))  {
            decodeVOP2_VOP_DPP8();
        }
        else if (IS_VOP2_VOP_DPP16(insn_long) && (COND_VOP2_VOP_DPP16_has_dpp16(insn_long)))  {
            decodeVOP2_VOP_DPP16();
        }
        else if (IS_VOP3_SDST_ENC(insn_long) && (COND_VOP3_SDST_ENC_default(insn_long)))  {
            decodeVOP3_SDST_ENC();
        }
        else if (IS_VOP3_SDST_ENC_INST_LITERAL(insn_long) && (COND_VOP3_SDST_ENC_INST_LITERAL_has_lit_0(insn_long) || COND_VOP3_SDST_ENC_INST_LITERAL_has_lit_1(insn_long) || COND_VOP3_SDST_ENC_INST_LITERAL_has_lit_0_and_has_lit_1(insn_long) || COND_VOP3_SDST_ENC_INST_LITERAL_has_lit_2(insn_long) || COND_VOP3_SDST_ENC_INST_LITERAL_has_lit_0_and_has_lit_2(insn_long) || COND_VOP3_SDST_ENC_INST_LITERAL_has_lit_1_and_has_lit_2(insn_long) || COND_VOP3_SDST_ENC_INST_LITERAL_has_lit_0_and_has_lit_1_and_has_lit_2(insn_long)))  {
            decodeVOP3_SDST_ENC_INST_LITERAL();
        }
        else if (IS_VOP3_SDST_ENC_VOP_DPP8(insn_long) && (COND_VOP3_SDST_ENC_VOP_DPP8_has_dpp8(insn_long)))  {
            decodeVOP3_SDST_ENC_VOP_DPP8();
        }
        else if (IS_VOP3_SDST_ENC_VOP_DPP16(insn_long) && (COND_VOP3_SDST_ENC_VOP_DPP16_has_dpp16(insn_long)))  {
            decodeVOP3_SDST_ENC_VOP_DPP16();
        }
        else if (IS_VOP3P_INST_LITERAL(insn_long) && (COND_VOP3P_INST_LITERAL_has_lit_0(insn_long) || COND_VOP3P_INST_LITERAL_has_lit_1(insn_long) || COND_VOP3P_INST_LITERAL_has_lit_0_and_has_lit_1(insn_long) || COND_VOP3P_INST_LITERAL_has_lit_2(insn_long) || COND_VOP3P_INST_LITERAL_has_lit_0_and_has_lit_2(insn_long) || COND_VOP3P_INST_LITERAL_has_lit_1_and_has_lit_2(insn_long) || COND_VOP3P_INST_LITERAL_has_lit_0_and_has_lit_1_and_has_lit_2(insn_long)))  {
            decodeVOP3P_INST_LITERAL();
        }
        else if (IS_VOP3P_VOP_DPP8(insn_long) && (COND_VOP3P_VOP_DPP8_has_dpp8(insn_long)))  {
            decodeVOP3P_VOP_DPP8();
        }
        else if (IS_VOP3P_VOP_DPP16(insn_long) && (COND_VOP3P_VOP_DPP16_has_dpp16(insn_long)))  {
            decodeVOP3P_VOP_DPP16();
        }
        else if (IS_VOPC_INST_LITERAL(insn_long) && (COND_VOPC_INST_LITERAL_has_lit(insn_long)))  {
            decodeVOPC_INST_LITERAL();
        }
        else if (IS_VOPC_INST_LITERAL64(insn_long) && (COND_VOPC_INST_LITERAL64_has_lit64(insn_long)))  {
            decodeVOPC_INST_LITERAL64();
        }
        else if (IS_VOPC_VOP_DPP8(insn_long) && (COND_VOPC_VOP_DPP8_has_dpp8(insn_long)))  {
            decodeVOPC_VOP_DPP8();
        }
        else if (IS_VOPC_VOP_DPP16(insn_long) && (COND_VOPC_VOP_DPP16_has_dpp16(insn_long)))  {
            decodeVOPC_VOP_DPP16();
        }
        else if (IS_VOPD3XY_X(insn_long))  {
            decodeVOPD3XY_X();
        }
        else if ((IS_VOPDXY_X(insn_long) || IS_VOPDXY_INST_LITERAL_X(insn_long)))  {
            decodeVOPDXY_X();
        }
        else  {
            // no encoding claims this word: emit an INVALID instruction of
            // one encoding-word size so decoding can move past it
            insn_size = 4;
            this->insn_in_progress = makeInstruction(amdgpu_gfx1250_op_INVALID,"INVALID",insn_size,reinterpret_cast<unsigned char *>(&insn));
        }
    }

}
}
