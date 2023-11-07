#include "registers/AMDGPU/amdgpu_gfx90a_regs.h"
#include "InstructionDecoder-amdgpu-gfx90a.h"

namespace Dyninst {
namespace InstructionAPI {
    bool InstructionDecoder_amdgpu_gfx90a::IS_ENC_SOP1(uint64_t I)
    {
        switch ( I & 0xff80ff00 )  {
            case 0xbe800000:  case 0xbe800100:  case 0xbe800200:  case 0xbe800300:  
            case 0xbe800400:  case 0xbe800500:  case 0xbe800600:  case 0xbe800700:  
            case 0xbe800800:  case 0xbe800900:  case 0xbe800a00:  case 0xbe800b00:  
            case 0xbe800c00:  case 0xbe800d00:  case 0xbe800e00:  case 0xbe800f00:  
            case 0xbe801000:  case 0xbe801100:  case 0xbe801200:  case 0xbe801300:  
            case 0xbe801400:  case 0xbe801500:  case 0xbe801600:  case 0xbe801700:  
            case 0xbe801800:  case 0xbe801900:  case 0xbe801a00:  case 0xbe801b00:  
            case 0xbe801c00:  case 0xbe801d00:  case 0xbe801e00:  case 0xbe801f00:  
            case 0xbe802000:  case 0xbe802100:  case 0xbe802200:  case 0xbe802300:  
            case 0xbe802400:  case 0xbe802500:  case 0xbe802600:  case 0xbe802700:  
            case 0xbe802800:  case 0xbe802900:  case 0xbe802a00:  case 0xbe802b00:  
            case 0xbe802c00:  case 0xbe802d00:  case 0xbe802e00:  case 0xbe803000:  
            case 0xbe803200:  case 0xbe803300:  case 0xbe803400:  case 0xbe803500:  
            case 0xbe803600:  case 0xbe803700:  
                return true;
                
            default:
                return false;
        }
    }

    bool InstructionDecoder_amdgpu_gfx90a::IS_ENC_SOPC(uint64_t I)
    {
        switch ( I & 0xffff0000 )  {
            case 0xbf000000:  case 0xbf010000:  case 0xbf020000:  case 0xbf030000:  
            case 0xbf040000:  case 0xbf050000:  case 0xbf060000:  case 0xbf070000:  
            case 0xbf080000:  case 0xbf090000:  case 0xbf0a0000:  case 0xbf0b0000:  
            case 0xbf0c0000:  case 0xbf0d0000:  case 0xbf0e0000:  case 0xbf0f0000:  
            case 0xbf100000:  case 0xbf110000:  case 0xbf120000:  case 0xbf130000:  
                return true;
                
            default:
                return false;
        }
    }

    bool InstructionDecoder_amdgpu_gfx90a::IS_ENC_SOPP(uint64_t I)
    {
        switch ( I & 0xffff0000 )  {
            case 0xbf800000:  case 0xbf810000:  case 0xbf820000:  case 0xbf830000:  
            case 0xbf840000:  case 0xbf850000:  case 0xbf860000:  case 0xbf870000:  
            case 0xbf880000:  case 0xbf890000:  case 0xbf8a0000:  case 0xbf8b0000:  
            case 0xbf8c0000:  case 0xbf8d0000:  case 0xbf8e0000:  case 0xbf8f0000:  
            case 0xbf900000:  case 0xbf910000:  case 0xbf920000:  case 0xbf930000:  
            case 0xbf940000:  case 0xbf950000:  case 0xbf960000:  case 0xbf970000:  
            case 0xbf980000:  case 0xbf990000:  case 0xbf9a0000:  case 0xbf9b0000:  
            case 0xbf9c0000:  case 0xbf9d0000:  case 0xbf9e0000:  
                return true;
                
            default:
                return false;
        }
    }

    bool InstructionDecoder_amdgpu_gfx90a::IS_ENC_SOPK(uint64_t I)
    {
        switch ( I & 0xff800000 )  {
            case 0xb0000000:  case 0xb0800000:  case 0xb1000000:  case 0xb1800000:  
            case 0xb2000000:  case 0xb2800000:  case 0xb3000000:  case 0xb3800000:  
            case 0xb4000000:  case 0xb4800000:  case 0xb5000000:  case 0xb5800000:  
            case 0xb6000000:  case 0xb6800000:  case 0xb7000000:  case 0xb7800000:  
            case 0xb8000000:  case 0xb8800000:  case 0xb9000000:  case 0xba800000:  
                return true;
                
            default:
                return false;
        }
    }

    bool InstructionDecoder_amdgpu_gfx90a::IS_ENC_SOP2(uint64_t I)
    {
        switch ( I & 0xff800000 )  {
            case 0x80000000:  case 0x80800000:  case 0x81000000:  case 0x81800000:  
            case 0x82000000:  case 0x82800000:  case 0x83000000:  case 0x83800000:  
            case 0x84000000:  case 0x84800000:  case 0x85000000:  case 0x85800000:  
            case 0x86000000:  case 0x86800000:  case 0x87000000:  case 0x87800000:  
            case 0x88000000:  case 0x88800000:  case 0x89000000:  case 0x89800000:  
            case 0x8a000000:  case 0x8a800000:  case 0x8b000000:  case 0x8b800000:  
            case 0x8c000000:  case 0x8c800000:  case 0x8d000000:  case 0x8d800000:  
            case 0x8e000000:  case 0x8e800000:  case 0x8f000000:  case 0x8f800000:  
            case 0x90000000:  case 0x90800000:  case 0x91000000:  case 0x91800000:  
            case 0x92000000:  case 0x92800000:  case 0x93000000:  case 0x93800000:  
            case 0x94000000:  case 0x94800000:  case 0x95000000:  case 0x95800000:  
            case 0x96000000:  case 0x96800000:  case 0x97000000:  case 0x97800000:  
            case 0x98000000:  case 0x98800000:  case 0x99000000:  case 0x99800000:  
            case 0x9a000000:  
                return true;
                
            default:
                return false;
        }
    }

    bool InstructionDecoder_amdgpu_gfx90a::IS_ENC_SMEM(uint64_t I)
    {
        switch ( I & 0xfffc0000 )  {
            case 0xc0000000:  case 0xc0040000:  case 0xc0080000:  case 0xc00c0000:  
            case 0xc0100000:  case 0xc0140000:  case 0xc0180000:  case 0xc01c0000:  
            case 0xc0200000:  case 0xc0240000:  case 0xc0280000:  case 0xc02c0000:  
            case 0xc0300000:  case 0xc0400000:  case 0xc0440000:  case 0xc0480000:  
            case 0xc0540000:  case 0xc0580000:  case 0xc05c0000:  case 0xc0600000:  
            case 0xc0640000:  case 0xc0680000:  case 0xc0800000:  case 0xc0840000:  
            case 0xc0880000:  case 0xc08c0000:  case 0xc0900000:  case 0xc0940000:  
            case 0xc0980000:  case 0xc09c0000:  case 0xc0a00000:  case 0xc0a40000:  
            case 0xc1000000:  case 0xc1040000:  case 0xc1080000:  case 0xc10c0000:  
            case 0xc1100000:  case 0xc1140000:  case 0xc1180000:  case 0xc11c0000:  
            case 0xc1200000:  case 0xc1240000:  case 0xc1280000:  case 0xc12c0000:  
            case 0xc1300000:  case 0xc1800000:  case 0xc1840000:  case 0xc1880000:  
            case 0xc18c0000:  case 0xc1900000:  case 0xc1940000:  case 0xc1980000:  
            case 0xc19c0000:  case 0xc1a00000:  case 0xc1a40000:  case 0xc1a80000:  
            case 0xc1ac0000:  case 0xc1b00000:  case 0xc2000000:  case 0xc2040000:  
            case 0xc2080000:  case 0xc20c0000:  case 0xc2100000:  case 0xc2140000:  
            case 0xc2180000:  case 0xc21c0000:  case 0xc2200000:  case 0xc2240000:  
            case 0xc2280000:  case 0xc22c0000:  case 0xc2300000:  case 0xc2800000:  
            case 0xc2840000:  case 0xc2880000:  case 0xc28c0000:  case 0xc2900000:  
            case 0xc2940000:  case 0xc2980000:  case 0xc29c0000:  case 0xc2a00000:  
            case 0xc2a40000:  case 0xc2a80000:  case 0xc2ac0000:  case 0xc2b00000:  
                return true;
                
            default:
                return false;
        }
    }

    bool InstructionDecoder_amdgpu_gfx90a::IS_ENC_VOP1(uint64_t I)
    {
        switch ( I & 0xfe01fe00 )  {
            case 0x7e000000:  case 0x7e000200:  case 0x7e000400:  case 0x7e000600:  
            case 0x7e000800:  case 0x7e000a00:  case 0x7e000c00:  case 0x7e000e00:  
            case 0x7e001000:  case 0x7e001400:  case 0x7e001600:  case 0x7e001800:  
            case 0x7e001a00:  case 0x7e001c00:  case 0x7e001e00:  case 0x7e002000:  
            case 0x7e002200:  case 0x7e002400:  case 0x7e002600:  case 0x7e002800:  
            case 0x7e002a00:  case 0x7e002c00:  case 0x7e002e00:  case 0x7e003000:  
            case 0x7e003200:  case 0x7e003400:  case 0x7e003600:  case 0x7e003800:  
            case 0x7e003a00:  case 0x7e003c00:  case 0x7e003e00:  case 0x7e004000:  
            case 0x7e004200:  case 0x7e004400:  case 0x7e004600:  case 0x7e004800:  
            case 0x7e004a00:  case 0x7e004c00:  case 0x7e004e00:  case 0x7e005000:  
            case 0x7e005200:  case 0x7e005400:  case 0x7e005600:  case 0x7e005800:  
            case 0x7e005a00:  case 0x7e005c00:  case 0x7e005e00:  case 0x7e006000:  
            case 0x7e006200:  case 0x7e006400:  case 0x7e006600:  case 0x7e006800:  
            case 0x7e006a00:  case 0x7e006e00:  case 0x7e007200:  case 0x7e007400:  
            case 0x7e007600:  case 0x7e007800:  case 0x7e007a00:  case 0x7e007c00:  
            case 0x7e007e00:  case 0x7e008000:  case 0x7e008200:  case 0x7e008400:  
            case 0x7e008600:  case 0x7e008800:  case 0x7e008a00:  case 0x7e008c00:  
            case 0x7e008e00:  case 0x7e009000:  case 0x7e009200:  case 0x7e009400:  
            case 0x7e009600:  case 0x7e009800:  case 0x7e009a00:  case 0x7e009c00:  
            case 0x7e009e00:  case 0x7e00a200:  case 0x7e00a400:  
                return true;
                
            default:
                return false;
        }
    }

    bool InstructionDecoder_amdgpu_gfx90a::IS_ENC_VOPC(uint64_t I)
    {
        switch ( I & 0xfffe0000 )  {
            case 0x7c200000:  case 0x7c220000:  case 0x7c240000:  case 0x7c260000:  
            case 0x7c280000:  case 0x7c2a0000:  case 0x7c400000:  case 0x7c420000:  
            case 0x7c440000:  case 0x7c460000:  case 0x7c480000:  case 0x7c4a0000:  
            case 0x7c4c0000:  case 0x7c4e0000:  case 0x7c500000:  case 0x7c520000:  
            case 0x7c540000:  case 0x7c560000:  case 0x7c580000:  case 0x7c5a0000:  
            case 0x7c5c0000:  case 0x7c5e0000:  case 0x7c600000:  case 0x7c620000:  
            case 0x7c640000:  case 0x7c660000:  case 0x7c680000:  case 0x7c6a0000:  
            case 0x7c6c0000:  case 0x7c6e0000:  case 0x7c700000:  case 0x7c720000:  
            case 0x7c740000:  case 0x7c760000:  case 0x7c780000:  case 0x7c7a0000:  
            case 0x7c7c0000:  case 0x7c7e0000:  case 0x7c800000:  case 0x7c820000:  
            case 0x7c840000:  case 0x7c860000:  case 0x7c880000:  case 0x7c8a0000:  
            case 0x7c8c0000:  case 0x7c8e0000:  case 0x7c900000:  case 0x7c920000:  
            case 0x7c940000:  case 0x7c960000:  case 0x7c980000:  case 0x7c9a0000:  
            case 0x7c9c0000:  case 0x7c9e0000:  case 0x7ca00000:  case 0x7ca20000:  
            case 0x7ca40000:  case 0x7ca60000:  case 0x7ca80000:  case 0x7caa0000:  
            case 0x7cac0000:  case 0x7cae0000:  case 0x7cb00000:  case 0x7cb20000:  
            case 0x7cb40000:  case 0x7cb60000:  case 0x7cb80000:  case 0x7cba0000:  
            case 0x7cbc0000:  case 0x7cbe0000:  case 0x7cc00000:  case 0x7cc20000:  
            case 0x7cc40000:  case 0x7cc60000:  case 0x7cc80000:  case 0x7cca0000:  
            case 0x7ccc0000:  case 0x7cce0000:  case 0x7cd00000:  case 0x7cd20000:  
            case 0x7cd40000:  case 0x7cd60000:  case 0x7cd80000:  case 0x7cda0000:  
            case 0x7cdc0000:  case 0x7cde0000:  case 0x7ce00000:  case 0x7ce20000:  
            case 0x7ce40000:  case 0x7ce60000:  case 0x7ce80000:  case 0x7cea0000:  
            case 0x7cec0000:  case 0x7cee0000:  case 0x7cf00000:  case 0x7cf20000:  
            case 0x7cf40000:  case 0x7cf60000:  case 0x7cf80000:  case 0x7cfa0000:  
            case 0x7cfc0000:  case 0x7cfe0000:  case 0x7d400000:  case 0x7d420000:  
            case 0x7d440000:  case 0x7d460000:  case 0x7d480000:  case 0x7d4a0000:  
            case 0x7d4c0000:  case 0x7d4e0000:  case 0x7d500000:  case 0x7d520000:  
            case 0x7d540000:  case 0x7d560000:  case 0x7d580000:  case 0x7d5a0000:  
            case 0x7d5c0000:  case 0x7d5e0000:  case 0x7d600000:  case 0x7d620000:  
            case 0x7d640000:  case 0x7d660000:  case 0x7d680000:  case 0x7d6a0000:  
            case 0x7d6c0000:  case 0x7d6e0000:  case 0x7d700000:  case 0x7d720000:  
            case 0x7d740000:  case 0x7d760000:  case 0x7d780000:  case 0x7d7a0000:  
            case 0x7d7c0000:  case 0x7d7e0000:  case 0x7d800000:  case 0x7d820000:  
            case 0x7d840000:  case 0x7d860000:  case 0x7d880000:  case 0x7d8a0000:  
            case 0x7d8c0000:  case 0x7d8e0000:  case 0x7d900000:  case 0x7d920000:  
            case 0x7d940000:  case 0x7d960000:  case 0x7d980000:  case 0x7d9a0000:  
            case 0x7d9c0000:  case 0x7d9e0000:  case 0x7da00000:  case 0x7da20000:  
            case 0x7da40000:  case 0x7da60000:  case 0x7da80000:  case 0x7daa0000:  
            case 0x7dac0000:  case 0x7dae0000:  case 0x7db00000:  case 0x7db20000:  
            case 0x7db40000:  case 0x7db60000:  case 0x7db80000:  case 0x7dba0000:  
            case 0x7dbc0000:  case 0x7dbe0000:  case 0x7dc00000:  case 0x7dc20000:  
            case 0x7dc40000:  case 0x7dc60000:  case 0x7dc80000:  case 0x7dca0000:  
            case 0x7dcc0000:  case 0x7dce0000:  case 0x7dd00000:  case 0x7dd20000:  
            case 0x7dd40000:  case 0x7dd60000:  case 0x7dd80000:  case 0x7dda0000:  
            case 0x7ddc0000:  case 0x7dde0000:  case 0x7de00000:  case 0x7de20000:  
            case 0x7de40000:  case 0x7de60000:  case 0x7de80000:  case 0x7dea0000:  
            case 0x7dec0000:  case 0x7dee0000:  case 0x7df00000:  case 0x7df20000:  
            case 0x7df40000:  case 0x7df60000:  case 0x7df80000:  case 0x7dfa0000:  
            case 0x7dfc0000:  case 0x7dfe0000:  
                return true;
                
            default:
                return false;
        }
    }

    bool InstructionDecoder_amdgpu_gfx90a::IS_ENC_VOP2(uint64_t I)
    {
        switch ( I & 0xfe000000 )  {
            case 0x0:  case 0x2000000:  case 0x4000000:  case 0x6000000:  
            case 0x8000000:  case 0xa000000:  case 0xc000000:  case 0xe000000:  
            case 0x10000000:  case 0x12000000:  case 0x14000000:  case 0x16000000:  
            case 0x18000000:  case 0x1a000000:  case 0x1c000000:  case 0x1e000000:  
            case 0x20000000:  case 0x22000000:  case 0x24000000:  case 0x26000000:  
            case 0x28000000:  case 0x2a000000:  case 0x2c000000:  case 0x32000000:  
            case 0x34000000:  case 0x36000000:  case 0x38000000:  case 0x3a000000:  
            case 0x3c000000:  case 0x3e000000:  case 0x40000000:  case 0x42000000:  
            case 0x44000000:  case 0x46000000:  case 0x4c000000:  case 0x4e000000:  
            case 0x50000000:  case 0x52000000:  case 0x54000000:  case 0x56000000:  
            case 0x58000000:  case 0x5a000000:  case 0x5c000000:  case 0x5e000000:  
            case 0x60000000:  case 0x62000000:  case 0x64000000:  case 0x66000000:  
            case 0x68000000:  case 0x6a000000:  case 0x6c000000:  case 0x6e000000:  
            case 0x70000000:  case 0x72000000:  case 0x74000000:  case 0x76000000:  
            case 0x78000000:  case 0x7a000000:  
                return true;
                
            default:
                return false;
        }
    }

    bool InstructionDecoder_amdgpu_gfx90a::IS_ENC_VINTRP(uint64_t I)
    {
        switch ( I & 0xfc000000 )  {
            default:
                return false;
        }
    }

    bool InstructionDecoder_amdgpu_gfx90a::IS_ENC_VOP3P(uint64_t I)
    {
        switch ( I & 0xffff0000 )  {
            case 0xd3800000:  case 0xd3810000:  case 0xd3820000:  case 0xd3830000:  
            case 0xd3840000:  case 0xd3850000:  case 0xd3860000:  case 0xd3870000:  
            case 0xd3880000:  case 0xd3890000:  case 0xd38a0000:  case 0xd38b0000:  
            case 0xd38c0000:  case 0xd38d0000:  case 0xd38e0000:  case 0xd38f0000:  
            case 0xd3900000:  case 0xd3910000:  case 0xd3920000:  case 0xd3a00000:  
            case 0xd3a10000:  case 0xd3a20000:  case 0xd3a30000:  case 0xd3a60000:  
            case 0xd3a70000:  case 0xd3a80000:  case 0xd3a90000:  case 0xd3aa0000:  
            case 0xd3ab0000:  case 0xd3b00000:  case 0xd3b10000:  case 0xd3b20000:  
            case 0xd3b30000:  case 0xd3d80000:  case 0xd3d90000:  
                return true;
                
            default:
                return false;
        }
    }

    bool InstructionDecoder_amdgpu_gfx90a::IS_ENC_VOP3(uint64_t I)
    {
        switch ( I & 0xffff0000 )  {
            case 0xd1400000:  case 0xd1410000:  case 0xd1420000:  case 0xd1430000:  
            case 0xd1440000:  case 0xd1450000:  case 0xd1460000:  case 0xd1470000:  
            case 0xd1480000:  case 0xd14a0000:  case 0xd14b0000:  case 0xd14c0000:  
            case 0xd14d0000:  case 0xd14e0000:  case 0xd14f0000:  case 0xd1500000:  
            case 0xd1510000:  case 0xd1520000:  case 0xd1530000:  case 0xd1540000:  
            case 0xd1550000:  case 0xd1560000:  case 0xd1570000:  case 0xd1580000:  
            case 0xd1590000:  case 0xd15a0000:  case 0xd15b0000:  case 0xd15c0000:  
            case 0xd15d0000:  case 0xd15e0000:  case 0xd15f0000:  case 0xd1600000:  
            case 0xd1610000:  case 0xd1620000:  case 0xd1630000:  case 0xd1640000:  
            case 0xd1650000:  case 0xd1660000:  case 0xd1670000:  case 0xd1680000:  
            case 0xd1690000:  case 0xd16a0000:  case 0xd16b0000:  case 0xd16c0000:  
            case 0xd16d0000:  case 0xd16e0000:  case 0xd16f0000:  case 0xd1700000:  
            case 0xd1710000:  case 0xd1720000:  case 0xd1730000:  case 0xd1740000:  
            case 0xd1750000:  case 0xd1770000:  case 0xd1790000:  case 0xd17a0000:  
            case 0xd17b0000:  case 0xd17c0000:  case 0xd17d0000:  case 0xd17e0000:  
            case 0xd17f0000:  case 0xd1800000:  case 0xd1810000:  case 0xd1820000:  
            case 0xd1830000:  case 0xd1840000:  case 0xd1850000:  case 0xd1860000:  
            case 0xd1870000:  case 0xd1880000:  case 0xd1890000:  case 0xd18a0000:  
            case 0xd18b0000:  case 0xd18c0000:  case 0xd18d0000:  case 0xd18e0000:  
            case 0xd18f0000:  case 0xd1910000:  case 0xd1920000:  case 0xd1000000:  
            case 0xd1010000:  case 0xd1020000:  case 0xd1030000:  case 0xd1040000:  
            case 0xd1050000:  case 0xd1060000:  case 0xd1070000:  case 0xd1080000:  
            case 0xd1090000:  case 0xd10a0000:  case 0xd10b0000:  case 0xd10c0000:  
            case 0xd10d0000:  case 0xd10e0000:  case 0xd10f0000:  case 0xd1100000:  
            case 0xd1110000:  case 0xd1120000:  case 0xd1130000:  case 0xd1140000:  
            case 0xd1150000:  case 0xd1160000:  case 0xd11f0000:  case 0xd1200000:  
            case 0xd1210000:  case 0xd1220000:  case 0xd1230000:  case 0xd1260000:  
            case 0xd1270000:  case 0xd1280000:  case 0xd1290000:  case 0xd12a0000:  
            case 0xd12b0000:  case 0xd12c0000:  case 0xd12d0000:  case 0xd12e0000:  
            case 0xd12f0000:  case 0xd1300000:  case 0xd1310000:  case 0xd1320000:  
            case 0xd1330000:  case 0xd1340000:  case 0xd1350000:  case 0xd1360000:  
            case 0xd1370000:  case 0xd1380000:  case 0xd1390000:  case 0xd13a0000:  
            case 0xd13b0000:  case 0xd13c0000:  case 0xd13d0000:  case 0xd1c00000:  
            case 0xd1c10000:  case 0xd1c20000:  case 0xd1c30000:  case 0xd1c40000:  
            case 0xd1c50000:  case 0xd1c60000:  case 0xd1c70000:  case 0xd1c80000:  
            case 0xd1c90000:  case 0xd1ca0000:  case 0xd1cb0000:  case 0xd1cc0000:  
            case 0xd1cd0000:  case 0xd1ce0000:  case 0xd1cf0000:  case 0xd1d00000:  
            case 0xd1d10000:  case 0xd1d20000:  case 0xd1d30000:  case 0xd1d40000:  
            case 0xd1d50000:  case 0xd1d60000:  case 0xd1d70000:  case 0xd1d80000:  
            case 0xd1d90000:  case 0xd1da0000:  case 0xd1db0000:  case 0xd1dc0000:  
            case 0xd1dd0000:  case 0xd1de0000:  case 0xd1df0000:  case 0xd1e20000:  
            case 0xd1e30000:  case 0xd1e40000:  case 0xd1e50000:  case 0xd1e60000:  
            case 0xd1e70000:  case 0xd1ea0000:  case 0xd1eb0000:  case 0xd1ec0000:  
            case 0xd1ed0000:  case 0xd1ee0000:  case 0xd1ef0000:  case 0xd1f00000:  
            case 0xd1f10000:  case 0xd1f20000:  case 0xd1f30000:  case 0xd1f40000:  
            case 0xd1f50000:  case 0xd1f60000:  case 0xd1f70000:  case 0xd1f80000:  
            case 0xd1f90000:  case 0xd1fa0000:  case 0xd1fb0000:  case 0xd1fc0000:  
            case 0xd1fd0000:  case 0xd1fe0000:  case 0xd1ff0000:  case 0xd2000000:  
            case 0xd2010000:  case 0xd2020000:  case 0xd2030000:  case 0xd2040000:  
            case 0xd2050000:  case 0xd2060000:  case 0xd2070000:  case 0xd2800000:  
            case 0xd2810000:  case 0xd2820000:  case 0xd2830000:  case 0xd2840000:  
            case 0xd2850000:  case 0xd2860000:  case 0xd2870000:  case 0xd2880000:  
            case 0xd2890000:  case 0xd28a0000:  case 0xd28b0000:  case 0xd28c0000:  
            case 0xd28d0000:  case 0xd28f0000:  case 0xd2900000:  case 0xd2910000:  
            case 0xd2920000:  case 0xd2930000:  case 0xd2940000:  case 0xd2950000:  
            case 0xd2960000:  case 0xd2970000:  case 0xd2980000:  case 0xd2990000:  
            case 0xd29a0000:  case 0xd29c0000:  case 0xd29d0000:  case 0xd29e0000:  
            case 0xd29f0000:  case 0xd2a00000:  case 0xd2a10000:  case 0xd0100000:  
            case 0xd0110000:  case 0xd0120000:  case 0xd0130000:  case 0xd0140000:  
            case 0xd0150000:  case 0xd0200000:  case 0xd0210000:  case 0xd0220000:  
            case 0xd0230000:  case 0xd0240000:  case 0xd0250000:  case 0xd0260000:  
            case 0xd0270000:  case 0xd0280000:  case 0xd0290000:  case 0xd02a0000:  
            case 0xd02b0000:  case 0xd02c0000:  case 0xd02d0000:  case 0xd02e0000:  
            case 0xd02f0000:  case 0xd0300000:  case 0xd0310000:  case 0xd0320000:  
            case 0xd0330000:  case 0xd0340000:  case 0xd0350000:  case 0xd0360000:  
            case 0xd0370000:  case 0xd0380000:  case 0xd0390000:  case 0xd03a0000:  
            case 0xd03b0000:  case 0xd03c0000:  case 0xd03d0000:  case 0xd03e0000:  
            case 0xd03f0000:  case 0xd0400000:  case 0xd0410000:  case 0xd0420000:  
            case 0xd0430000:  case 0xd0440000:  case 0xd0450000:  case 0xd0460000:  
            case 0xd0470000:  case 0xd0480000:  case 0xd0490000:  case 0xd04a0000:  
            case 0xd04b0000:  case 0xd04c0000:  case 0xd04d0000:  case 0xd04e0000:  
            case 0xd04f0000:  case 0xd0500000:  case 0xd0510000:  case 0xd0520000:  
            case 0xd0530000:  case 0xd0540000:  case 0xd0550000:  case 0xd0560000:  
            case 0xd0570000:  case 0xd0580000:  case 0xd0590000:  case 0xd05a0000:  
            case 0xd05b0000:  case 0xd05c0000:  case 0xd05d0000:  case 0xd05e0000:  
            case 0xd05f0000:  case 0xd0600000:  case 0xd0610000:  case 0xd0620000:  
            case 0xd0630000:  case 0xd0640000:  case 0xd0650000:  case 0xd0660000:  
            case 0xd0670000:  case 0xd0680000:  case 0xd0690000:  case 0xd06a0000:  
            case 0xd06b0000:  case 0xd06c0000:  case 0xd06d0000:  case 0xd06e0000:  
            case 0xd06f0000:  case 0xd0700000:  case 0xd0710000:  case 0xd0720000:  
            case 0xd0730000:  case 0xd0740000:  case 0xd0750000:  case 0xd0760000:  
            case 0xd0770000:  case 0xd0780000:  case 0xd0790000:  case 0xd07a0000:  
            case 0xd07b0000:  case 0xd07c0000:  case 0xd07d0000:  case 0xd07e0000:  
            case 0xd07f0000:  case 0xd0a00000:  case 0xd0a10000:  case 0xd0a20000:  
            case 0xd0a30000:  case 0xd0a40000:  case 0xd0a50000:  case 0xd0a60000:  
            case 0xd0a70000:  case 0xd0a80000:  case 0xd0a90000:  case 0xd0aa0000:  
            case 0xd0ab0000:  case 0xd0ac0000:  case 0xd0ad0000:  case 0xd0ae0000:  
            case 0xd0af0000:  case 0xd0b00000:  case 0xd0b10000:  case 0xd0b20000:  
            case 0xd0b30000:  case 0xd0b40000:  case 0xd0b50000:  case 0xd0b60000:  
            case 0xd0b70000:  case 0xd0b80000:  case 0xd0b90000:  case 0xd0ba0000:  
            case 0xd0bb0000:  case 0xd0bc0000:  case 0xd0bd0000:  case 0xd0be0000:  
            case 0xd0bf0000:  case 0xd0c00000:  case 0xd0c10000:  case 0xd0c20000:  
            case 0xd0c30000:  case 0xd0c40000:  case 0xd0c50000:  case 0xd0c60000:  
            case 0xd0c70000:  case 0xd0c80000:  case 0xd0c90000:  case 0xd0ca0000:  
            case 0xd0cb0000:  case 0xd0cc0000:  case 0xd0cd0000:  case 0xd0ce0000:  
            case 0xd0cf0000:  case 0xd0d00000:  case 0xd0d10000:  case 0xd0d20000:  
            case 0xd0d30000:  case 0xd0d40000:  case 0xd0d50000:  case 0xd0d60000:  
            case 0xd0d70000:  case 0xd0d80000:  case 0xd0d90000:  case 0xd0da0000:  
            case 0xd0db0000:  case 0xd0dc0000:  case 0xd0dd0000:  case 0xd0de0000:  
            case 0xd0df0000:  case 0xd0e00000:  case 0xd0e10000:  case 0xd0e20000:  
            case 0xd0e30000:  case 0xd0e40000:  case 0xd0e50000:  case 0xd0e60000:  
            case 0xd0e70000:  case 0xd0e80000:  case 0xd0e90000:  case 0xd0ea0000:  
            case 0xd0eb0000:  case 0xd0ec0000:  case 0xd0ed0000:  case 0xd0ee0000:  
            case 0xd0ef0000:  case 0xd0f00000:  case 0xd0f10000:  case 0xd0f20000:  
            case 0xd0f30000:  case 0xd0f40000:  case 0xd0f50000:  case 0xd0f60000:  
            case 0xd0f70000:  case 0xd0f80000:  case 0xd0f90000:  case 0xd0fa0000:  
            case 0xd0fb0000:  case 0xd0fc0000:  case 0xd0fd0000:  case 0xd0fe0000:  
            case 0xd0ff0000:  
                return true;
                
            default:
                return false;
        }
    }

    bool InstructionDecoder_amdgpu_gfx90a::IS_ENC_DS(uint64_t I)
    {
        switch ( I & 0xfdfe0000 )  {
            case 0xd8000000:  case 0xd8020000:  case 0xd8040000:  case 0xd8060000:  
            case 0xd8080000:  case 0xd80a0000:  case 0xd80c0000:  case 0xd80e0000:  
            case 0xd8100000:  case 0xd8120000:  case 0xd8140000:  case 0xd8160000:  
            case 0xd8180000:  case 0xd81a0000:  case 0xd81c0000:  case 0xd81e0000:  
            case 0xd8200000:  case 0xd8220000:  case 0xd8240000:  case 0xd8260000:  
            case 0xd8280000:  case 0xd82a0000:  case 0xd83a0000:  case 0xd83c0000:  
            case 0xd83e0000:  case 0xd8400000:  case 0xd8420000:  case 0xd8440000:  
            case 0xd8460000:  case 0xd8480000:  case 0xd84a0000:  case 0xd84c0000:  
            case 0xd84e0000:  case 0xd8500000:  case 0xd8520000:  case 0xd8540000:  
            case 0xd8560000:  case 0xd8580000:  case 0xd85a0000:  case 0xd85c0000:  
            case 0xd85e0000:  case 0xd8600000:  case 0xd8620000:  case 0xd8640000:  
            case 0xd8660000:  case 0xd8680000:  case 0xd86a0000:  case 0xd86c0000:  
            case 0xd86e0000:  case 0xd8700000:  case 0xd8720000:  case 0xd8740000:  
            case 0xd8760000:  case 0xd8780000:  case 0xd87a0000:  case 0xd87c0000:  
            case 0xd87e0000:  case 0xd8800000:  case 0xd8820000:  case 0xd8840000:  
            case 0xd8860000:  case 0xd8880000:  case 0xd88a0000:  case 0xd88c0000:  
            case 0xd88e0000:  case 0xd8900000:  case 0xd8920000:  case 0xd8940000:  
            case 0xd8960000:  case 0xd8980000:  case 0xd89a0000:  case 0xd89c0000:  
            case 0xd89e0000:  case 0xd8a00000:  case 0xd8a20000:  case 0xd8a40000:  
            case 0xd8a60000:  case 0xd8a80000:  case 0xd8aa0000:  case 0xd8ac0000:  
            case 0xd8ae0000:  case 0xd8b00000:  case 0xd8b20000:  case 0xd8b40000:  
            case 0xd8b60000:  case 0xd8b80000:  case 0xd8c00000:  case 0xd8c20000:  
            case 0xd8c40000:  case 0xd8c60000:  case 0xd8c80000:  case 0xd8ca0000:  
            case 0xd8cc0000:  case 0xd8ce0000:  case 0xd8d00000:  case 0xd8d20000:  
            case 0xd8d40000:  case 0xd8d60000:  case 0xd8d80000:  case 0xd8da0000:  
            case 0xd8dc0000:  case 0xd8de0000:  case 0xd8e00000:  case 0xd8e20000:  
            case 0xd8e40000:  case 0xd8e60000:  case 0xd8ec0000:  case 0xd8ee0000:  
            case 0xd8f00000:  case 0xd8f80000:  case 0xd8fc0000:  case 0xd9300000:  
            case 0xd9320000:  case 0xd9340000:  case 0xd9360000:  case 0xd9380000:  
            case 0xd93a0000:  case 0xd96c0000:  case 0xd97a0000:  case 0xd97c0000:  
            case 0xd9bc0000:  case 0xd9be0000:  case 0xd9fc0000:  case 0xd9fe0000:  
                return true;
                
            default:
                return false;
        }
    }

    bool InstructionDecoder_amdgpu_gfx90a::IS_ENC_MUBUF(uint64_t I)
    {
        switch ( I & 0xfdfc0000 )  {
            case 0xe0000000:  case 0xe0040000:  case 0xe0080000:  case 0xe00c0000:  
            case 0xe0100000:  case 0xe0140000:  case 0xe0180000:  case 0xe01c0000:  
            case 0xe0200000:  case 0xe0240000:  case 0xe0280000:  case 0xe02c0000:  
            case 0xe0300000:  case 0xe0340000:  case 0xe0380000:  case 0xe03c0000:  
            case 0xe0400000:  case 0xe0440000:  case 0xe0480000:  case 0xe04c0000:  
            case 0xe0500000:  case 0xe0540000:  case 0xe0580000:  case 0xe05c0000:  
            case 0xe0600000:  case 0xe0640000:  case 0xe0680000:  case 0xe06c0000:  
            case 0xe0700000:  case 0xe0740000:  case 0xe0780000:  case 0xe07c0000:  
            case 0xe0800000:  case 0xe0840000:  case 0xe0880000:  case 0xe08c0000:  
            case 0xe0900000:  case 0xe0940000:  case 0xe0980000:  case 0xe09c0000:  
            case 0xe0a00000:  case 0xe0a40000:  case 0xe0f40000:  case 0xe0f80000:  
            case 0xe0fc0000:  case 0xe1000000:  case 0xe1040000:  case 0xe1080000:  
            case 0xe10c0000:  case 0xe1100000:  case 0xe1140000:  case 0xe1180000:  
            case 0xe11c0000:  case 0xe1200000:  case 0xe1240000:  case 0xe1280000:  
            case 0xe12c0000:  case 0xe1300000:  case 0xe1340000:  case 0xe1380000:  
            case 0xe13c0000:  case 0xe1400000:  case 0xe1440000:  case 0xe1800000:  
            case 0xe1840000:  case 0xe1880000:  case 0xe18c0000:  case 0xe1900000:  
            case 0xe1940000:  case 0xe1980000:  case 0xe19c0000:  case 0xe1a00000:  
            case 0xe1a40000:  case 0xe1a80000:  case 0xe1ac0000:  case 0xe1b00000:  
                return true;
                
            default:
                return false;
        }
    }

    bool InstructionDecoder_amdgpu_gfx90a::IS_ENC_MTBUF(uint64_t I)
    {
        switch ( I & 0xfc078000 )  {
            case 0xe8000000:  case 0xe8008000:  case 0xe8010000:  case 0xe8018000:  
            case 0xe8020000:  case 0xe8028000:  case 0xe8030000:  case 0xe8038000:  
            case 0xe8040000:  case 0xe8048000:  case 0xe8050000:  case 0xe8058000:  
            case 0xe8060000:  case 0xe8068000:  case 0xe8070000:  case 0xe8078000:  
                return true;
                
            default:
                return false;
        }
    }

    bool InstructionDecoder_amdgpu_gfx90a::IS_ENC_MIMG(uint64_t I)
    {
        switch ( I & 0xfdfc0000 )  {
            case 0xf0000000:  case 0xf0040000:  case 0xf0080000:  case 0xf00c0000:  
            case 0xf0100000:  case 0xf0140000:  case 0xf0200000:  case 0xf0240000:  
            case 0xf0280000:  case 0xf02c0000:  case 0xf0380000:  case 0xf0400000:  
            case 0xf0440000:  case 0xf0480000:  case 0xf04c0000:  case 0xf0500000:  
            case 0xf0540000:  case 0xf0580000:  case 0xf05c0000:  case 0xf0600000:  
            case 0xf0640000:  case 0xf0680000:  case 0xf06c0000:  case 0xf0700000:  
            case 0xf0800000:  
                return true;
                
            default:
                return false;
        }
    }

    bool InstructionDecoder_amdgpu_gfx90a::IS_ENC_FLAT(uint64_t I)
    {
        switch ( I & 0xfdfcc000 )  {
            case 0xdc400000:  case 0xdc440000:  case 0xdc480000:  case 0xdc4c0000:  
            case 0xdc500000:  case 0xdc540000:  case 0xdc580000:  case 0xdc5c0000:  
            case 0xdc600000:  case 0xdc640000:  case 0xdc680000:  case 0xdc6c0000:  
            case 0xdc700000:  case 0xdc740000:  case 0xdc780000:  case 0xdc7c0000:  
            case 0xdc800000:  case 0xdc840000:  case 0xdc880000:  case 0xdc8c0000:  
            case 0xdc900000:  case 0xdc940000:  case 0xdd000000:  case 0xdd040000:  
            case 0xdd080000:  case 0xdd0c0000:  case 0xdd100000:  case 0xdd140000:  
            case 0xdd180000:  case 0xdd1c0000:  case 0xdd200000:  case 0xdd240000:  
            case 0xdd280000:  case 0xdd2c0000:  case 0xdd300000:  case 0xdd3c0000:  
            case 0xdd400000:  case 0xdd440000:  case 0xdd800000:  case 0xdd840000:  
            case 0xdd880000:  case 0xdd8c0000:  case 0xdd900000:  case 0xdd940000:  
            case 0xdd980000:  case 0xdd9c0000:  case 0xdda00000:  case 0xdda40000:  
            case 0xdda80000:  case 0xddac0000:  case 0xddb00000:  
                return true;
                
            default:
                return false;
        }
    }

    bool InstructionDecoder_amdgpu_gfx90a::IS_ENC_FLAT_GLBL(uint64_t I)
    {
        switch ( I & 0xfdfcc000 )  {
            case 0xdc408000:  case 0xdc448000:  case 0xdc488000:  case 0xdc4c8000:  
            case 0xdc508000:  case 0xdc548000:  case 0xdc588000:  case 0xdc5c8000:  
            case 0xdc608000:  case 0xdc648000:  case 0xdc688000:  case 0xdc6c8000:  
            case 0xdc708000:  case 0xdc748000:  case 0xdc788000:  case 0xdc7c8000:  
            case 0xdc808000:  case 0xdc848000:  case 0xdc888000:  case 0xdc8c8000:  
            case 0xdc908000:  case 0xdc948000:  case 0xdd008000:  case 0xdd048000:  
            case 0xdd088000:  case 0xdd0c8000:  case 0xdd108000:  case 0xdd148000:  
            case 0xdd188000:  case 0xdd1c8000:  case 0xdd208000:  case 0xdd248000:  
            case 0xdd288000:  case 0xdd2c8000:  case 0xdd308000:  case 0xdd348000:  
            case 0xdd388000:  case 0xdd3c8000:  case 0xdd408000:  case 0xdd448000:  
            case 0xdd808000:  case 0xdd848000:  case 0xdd888000:  case 0xdd8c8000:  
            case 0xdd908000:  case 0xdd948000:  case 0xdd988000:  case 0xdd9c8000:  
            case 0xdda08000:  case 0xdda48000:  case 0xdda88000:  case 0xddac8000:  
            case 0xddb08000:  
                return true;
                
            default:
                return false;
        }
    }

    bool InstructionDecoder_amdgpu_gfx90a::IS_ENC_FLAT_SCRATCH(uint64_t I)
    {
        switch ( I & 0xfdfcc000 )  {
            case 0xdc404000:  case 0xdc444000:  case 0xdc484000:  case 0xdc4c4000:  
            case 0xdc504000:  case 0xdc544000:  case 0xdc584000:  case 0xdc5c4000:  
            case 0xdc604000:  case 0xdc644000:  case 0xdc684000:  case 0xdc6c4000:  
            case 0xdc704000:  case 0xdc744000:  case 0xdc784000:  case 0xdc7c4000:  
            case 0xdc804000:  case 0xdc844000:  case 0xdc884000:  case 0xdc8c4000:  
            case 0xdc904000:  case 0xdc944000:  
                return true;
                
            default:
                return false;
        }
    }

    bool InstructionDecoder_amdgpu_gfx90a::IS_SOPK_INST_LITERAL_(uint64_t I)
    {
        switch ( I & 0xff800000 )  {
            case 0xba000000:  
                return true;
                
            default:
                return false;
        }
    }

    bool InstructionDecoder_amdgpu_gfx90a::IS_ENC_VOP2_LITERAL(uint64_t I)
    {
        switch ( I & 0xfe000000 )  {
            case 0x2e000000:  case 0x30000000:  case 0x48000000:  case 0x4a000000:  
                return true;
                
            default:
                return false;
        }
    }

    bool InstructionDecoder_amdgpu_gfx90a::IS_ENC_VOP3B(uint64_t I)
    {
        switch ( I & 0xffff0000 )  {
            case 0xd1190000:  case 0xd11a0000:  case 0xd11b0000:  case 0xd11c0000:  
            case 0xd11d0000:  case 0xd11e0000:  case 0xd1e00000:  case 0xd1e10000:  
            case 0xd1e80000:  case 0xd1e90000:  
                return true;
                
            default:
                return false;
        }
    }

    bool InstructionDecoder_amdgpu_gfx90a::IS_ENC_VOP3P_MFMA(uint64_t I)
    {
        switch ( I & 0xffff0000 )  {
            case 0xd3c00000:  case 0xd3c10000:  case 0xd3c20000:  case 0xd3c40000:  
            case 0xd3c50000:  case 0xd3c80000:  case 0xd3c90000:  case 0xd3ca0000:  
            case 0xd3cc0000:  case 0xd3cd0000:  case 0xd3d00000:  case 0xd3d10000:  
            case 0xd3d20000:  case 0xd3d40000:  case 0xd3d50000:  case 0xd3e30000:  
            case 0xd3e40000:  case 0xd3e50000:  case 0xd3e60000:  case 0xd3e70000:  
            case 0xd3e80000:  case 0xd3e90000:  case 0xd3eb0000:  case 0xd3ec0000:  
            case 0xd3ed0000:  case 0xd3ee0000:  case 0xd3ef0000:  
                return true;
                
            default:
                return false;
        }
    }

    void InstructionDecoder_amdgpu_gfx90a::decodeENC_SOP1()
    {
        insn_size = 4;
        layout_ENC_SOP1 & layout = insn_layout.ENC_SOP1;
        layout.ENCODING = longfield<23,31>(insn_long);
        layout.OP = longfield<8,15>(insn_long);
        layout.SDST = longfield<16,22>(insn_long);
        layout.SSRC0 = longfield<0,7>(insn_long);
        assert(isArrayIndexValid(ENC_SOP1_insn_table, layout.OP) && "Opcode over or underflow");
        const amdgpu_gfx90a_insn_entry &insn_entry = ENC_SOP1_insn_table[layout.OP];
        this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
        finalizeENC_SOP1Operands();
        this->insn_in_progress->updateSize(insn_size + immLen);
        this->insn_in_progress->updateMnemonic(std::string(insn_entry.mnemonic) + extension);
    }

    void InstructionDecoder_amdgpu_gfx90a::decodeENC_SOPC()
    {
        insn_size = 4;
        layout_ENC_SOPC & layout = insn_layout.ENC_SOPC;
        layout.ENCODING = longfield<23,31>(insn_long);
        layout.OP = longfield<16,22>(insn_long);
        layout.SSRC0 = longfield<0,7>(insn_long);
        layout.SSRC1 = longfield<8,15>(insn_long);
        assert(isArrayIndexValid(ENC_SOPC_insn_table, layout.OP) && "Opcode over or underflow");
        const amdgpu_gfx90a_insn_entry &insn_entry = ENC_SOPC_insn_table[layout.OP];
        this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
        finalizeENC_SOPCOperands();
        this->insn_in_progress->updateSize(insn_size + immLen);
        this->insn_in_progress->updateMnemonic(std::string(insn_entry.mnemonic) + extension);
    }

    void InstructionDecoder_amdgpu_gfx90a::decodeENC_SOPP()
    {
        insn_size = 4;
        layout_ENC_SOPP & layout = insn_layout.ENC_SOPP;
        layout.ENCODING = longfield<23,31>(insn_long);
        layout.OP = longfield<16,22>(insn_long);
        layout.SIMM16 = longfield<0,15>(insn_long);
        assert(isArrayIndexValid(ENC_SOPP_insn_table, layout.OP) && "Opcode over or underflow");
        const amdgpu_gfx90a_insn_entry &insn_entry = ENC_SOPP_insn_table[layout.OP];
        this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
        finalizeENC_SOPPOperands();
        this->insn_in_progress->updateSize(insn_size + immLen);
        this->insn_in_progress->updateMnemonic(std::string(insn_entry.mnemonic) + extension);
    }

    void InstructionDecoder_amdgpu_gfx90a::decodeENC_SOPK()
    {
        insn_size = 4;
        layout_ENC_SOPK & layout = insn_layout.ENC_SOPK;
        layout.ENCODING = longfield<28,31>(insn_long);
        layout.OP = longfield<23,27>(insn_long);
        layout.SDST = longfield<16,22>(insn_long);
        layout.SIMM16 = longfield<0,15>(insn_long);
        assert(isArrayIndexValid(ENC_SOPK_insn_table, layout.OP) && "Opcode over or underflow");
        const amdgpu_gfx90a_insn_entry &insn_entry = ENC_SOPK_insn_table[layout.OP];
        this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
        finalizeENC_SOPKOperands();
        this->insn_in_progress->updateSize(insn_size + immLen);
        this->insn_in_progress->updateMnemonic(std::string(insn_entry.mnemonic) + extension);
    }

    void InstructionDecoder_amdgpu_gfx90a::decodeENC_SOP2()
    {
        insn_size = 4;
        layout_ENC_SOP2 & layout = insn_layout.ENC_SOP2;
        layout.ENCODING = longfield<30,31>(insn_long);
        layout.OP = longfield<23,29>(insn_long);
        layout.SDST = longfield<16,22>(insn_long);
        layout.SSRC0 = longfield<0,7>(insn_long);
        layout.SSRC1 = longfield<8,15>(insn_long);
        assert(isArrayIndexValid(ENC_SOP2_insn_table, layout.OP) && "Opcode over or underflow");
        const amdgpu_gfx90a_insn_entry &insn_entry = ENC_SOP2_insn_table[layout.OP];
        this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
        finalizeENC_SOP2Operands();
        this->insn_in_progress->updateSize(insn_size + immLen);
        this->insn_in_progress->updateMnemonic(std::string(insn_entry.mnemonic) + extension);
    }

    void InstructionDecoder_amdgpu_gfx90a::decodeENC_SMEM()
    {
        insn_size = 8;
        layout_ENC_SMEM & layout = insn_layout.ENC_SMEM;
        layout.ENCODING = longfield<26,31>(insn_long);
        layout.GLC = longfield<16,16>(insn_long);
        layout.IMM = longfield<17,17>(insn_long);
        layout.NV = longfield<15,15>(insn_long);
        layout.OFFSET = longfield<32,52>(insn_long);
        layout.OP = longfield<18,25>(insn_long);
        layout.SBASE = (longfield<0,5>(insn_long) << 1 ) | 0 ;
        layout.SDATA = longfield<6,12>(insn_long);
        layout.SOFFSET = longfield<57,63>(insn_long);
        layout.SOFFSET_EN = longfield<14,14>(insn_long);
        assert(isArrayIndexValid(ENC_SMEM_insn_table, layout.OP) && "Opcode over or underflow");
        const amdgpu_gfx90a_insn_entry &insn_entry = ENC_SMEM_insn_table[layout.OP];
        this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
        finalizeENC_SMEMOperands();
        this->insn_in_progress->updateSize(insn_size + immLen);
        this->insn_in_progress->updateMnemonic(std::string(insn_entry.mnemonic) + extension);
    }

    void InstructionDecoder_amdgpu_gfx90a::decodeENC_VOP1()
    {
        insn_size = 4;
        layout_ENC_VOP1 & layout = insn_layout.ENC_VOP1;
        layout.ENCODING = longfield<25,31>(insn_long);
        layout.OP = longfield<9,16>(insn_long);
        layout.SRC0 = longfield<0,8>(insn_long);
        layout.VDST = longfield<17,24>(insn_long);
        assert(isArrayIndexValid(ENC_VOP1_insn_table, layout.OP) && "Opcode over or underflow");
        const amdgpu_gfx90a_insn_entry &insn_entry = ENC_VOP1_insn_table[layout.OP];
        this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
        finalizeENC_VOP1Operands();
        this->insn_in_progress->updateSize(insn_size + immLen);
        this->insn_in_progress->updateMnemonic(std::string(insn_entry.mnemonic) + extension);
    }

    void InstructionDecoder_amdgpu_gfx90a::decodeENC_VOPC()
    {
        insn_size = 4;
        layout_ENC_VOPC & layout = insn_layout.ENC_VOPC;
        layout.ENCODING = longfield<25,31>(insn_long);
        layout.OP = longfield<17,24>(insn_long);
        layout.SRC0 = longfield<0,8>(insn_long);
        layout.VSRC1 = longfield<9,16>(insn_long);
        assert(isArrayIndexValid(ENC_VOPC_insn_table, layout.OP) && "Opcode over or underflow");
        const amdgpu_gfx90a_insn_entry &insn_entry = ENC_VOPC_insn_table[layout.OP];
        this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
        finalizeENC_VOPCOperands();
        this->insn_in_progress->updateSize(insn_size + immLen);
        this->insn_in_progress->updateMnemonic(std::string(insn_entry.mnemonic) + extension);
    }

    void InstructionDecoder_amdgpu_gfx90a::decodeENC_VOP2()
    {
        insn_size = 4;
        layout_ENC_VOP2 & layout = insn_layout.ENC_VOP2;
        layout.ENCODING = longfield<31,31>(insn_long);
        layout.OP = longfield<25,30>(insn_long);
        layout.SRC0 = longfield<0,8>(insn_long);
        layout.VDST = longfield<17,24>(insn_long);
        layout.VSRC1 = longfield<9,16>(insn_long);
        assert(isArrayIndexValid(ENC_VOP2_insn_table, layout.OP) && "Opcode over or underflow");
        const amdgpu_gfx90a_insn_entry &insn_entry = ENC_VOP2_insn_table[layout.OP];
        this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
        finalizeENC_VOP2Operands();
        this->insn_in_progress->updateSize(insn_size + immLen);
        this->insn_in_progress->updateMnemonic(std::string(insn_entry.mnemonic) + extension);
    }

    void InstructionDecoder_amdgpu_gfx90a::decodeENC_VINTRP()
    {
        insn_size = 4;
        layout_ENC_VINTRP & layout = insn_layout.ENC_VINTRP;
        layout.ATTR = longfield<10,15>(insn_long);
        layout.ATTRCHAN = longfield<8,9>(insn_long);
        layout.ENCODING = longfield<26,31>(insn_long);
        layout.OP = longfield<16,17>(insn_long);
        layout.VDST = longfield<18,25>(insn_long);
        layout.VSRC = longfield<0,7>(insn_long);
        assert(isArrayIndexValid(ENC_VINTRP_insn_table, layout.OP) && "Opcode over or underflow");
        const amdgpu_gfx90a_insn_entry &insn_entry = ENC_VINTRP_insn_table[layout.OP];
        this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
        finalizeENC_VINTRPOperands();
        this->insn_in_progress->updateSize(insn_size + immLen);
        this->insn_in_progress->updateMnemonic(std::string(insn_entry.mnemonic) + extension);
    }

    void InstructionDecoder_amdgpu_gfx90a::decodeENC_VOP3P()
    {
        insn_size = 8;
        layout_ENC_VOP3P & layout = insn_layout.ENC_VOP3P;
        layout.CLAMP = longfield<15,15>(insn_long);
        layout.ENCODING = longfield<23,31>(insn_long);
        layout.NEG = longfield<61,63>(insn_long);
        layout.NEG_HI = longfield<8,10>(insn_long);
        layout.OP = longfield<16,22>(insn_long);
        layout.OP_SEL = longfield<11,13>(insn_long);
        layout.OP_SEL_HI = longfield<59,60>(insn_long);
        layout.OP_SEL_HI_2 = longfield<14,14>(insn_long);
        layout.SRC0 = longfield<32,40>(insn_long);
        layout.SRC1 = longfield<41,49>(insn_long);
        layout.SRC2 = longfield<50,58>(insn_long);
        layout.VDST = longfield<0,7>(insn_long);
        assert(isArrayIndexValid(ENC_VOP3P_insn_table, layout.OP) && "Opcode over or underflow");
        const amdgpu_gfx90a_insn_entry &insn_entry = ENC_VOP3P_insn_table[layout.OP];
        this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
        finalizeENC_VOP3POperands();
        this->insn_in_progress->updateSize(insn_size + immLen);
        this->insn_in_progress->updateMnemonic(std::string(insn_entry.mnemonic) + extension);
    }

    void InstructionDecoder_amdgpu_gfx90a::decodeENC_VOP3()
    {
        insn_size = 8;
        layout_ENC_VOP3 & layout = insn_layout.ENC_VOP3;
        layout.ABS = longfield<8,10>(insn_long);
        layout.CLAMP = longfield<15,15>(insn_long);
        layout.ENCODING = longfield<26,31>(insn_long);
        layout.NEG = longfield<61,63>(insn_long);
        layout.OMOD = longfield<59,60>(insn_long);
        layout.OP = longfield<16,25>(insn_long);
        layout.OP_SEL = longfield<11,14>(insn_long);
        layout.SRC0 = longfield<32,40>(insn_long);
        layout.SRC1 = longfield<41,49>(insn_long);
        layout.SRC2 = longfield<50,58>(insn_long);
        layout.VDST = longfield<0,7>(insn_long);
        assert(isArrayIndexValid(ENC_VOP3_insn_table, layout.OP) && "Opcode over or underflow");
        const amdgpu_gfx90a_insn_entry &insn_entry = ENC_VOP3_insn_table[layout.OP];
        this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
        finalizeENC_VOP3Operands();
        this->insn_in_progress->updateSize(insn_size + immLen);
        this->insn_in_progress->updateMnemonic(std::string(insn_entry.mnemonic) + extension);
    }

    void InstructionDecoder_amdgpu_gfx90a::decodeENC_DS()
    {
        insn_size = 8;
        layout_ENC_DS & layout = insn_layout.ENC_DS;
        layout.ACC = longfield<25,25>(insn_long);
        layout.ADDR = longfield<32,39>(insn_long);
        layout.DATA0 = longfield<40,47>(insn_long);
        layout.DATA1 = longfield<48,55>(insn_long);
        layout.ENCODING = longfield<26,31>(insn_long);
        layout.GDS = longfield<16,16>(insn_long);
        layout.OFFSET0 = longfield<0,7>(insn_long);
        layout.OFFSET1 = longfield<8,15>(insn_long);
        layout.OP = longfield<17,24>(insn_long);
        layout.VDST = longfield<56,63>(insn_long);
        assert(isArrayIndexValid(ENC_DS_insn_table, layout.OP) && "Opcode over or underflow");
        const amdgpu_gfx90a_insn_entry &insn_entry = ENC_DS_insn_table[layout.OP];
        this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
        finalizeENC_DSOperands();
        this->insn_in_progress->updateSize(insn_size + immLen);
        this->insn_in_progress->updateMnemonic(std::string(insn_entry.mnemonic) + extension);
    }

    void InstructionDecoder_amdgpu_gfx90a::decodeENC_MUBUF()
    {
        insn_size = 8;
        layout_ENC_MUBUF & layout = insn_layout.ENC_MUBUF;
        layout.ACC = longfield<55,55>(insn_long);
        layout.ENCODING = longfield<26,31>(insn_long);
        layout.GLC = longfield<14,14>(insn_long);
        layout.IDXEN = longfield<13,13>(insn_long);
        layout.LDS = longfield<16,16>(insn_long);
        layout.OFFEN = longfield<12,12>(insn_long);
        layout.OFFSET = longfield<0,11>(insn_long);
        layout.OP = longfield<18,24>(insn_long);
        layout.SCC = longfield<15,15>(insn_long);
        layout.SLC = longfield<17,17>(insn_long);
        layout.SOFFSET = longfield<56,63>(insn_long);
        layout.SRSRC = (longfield<48,52>(insn_long) << 2 ) | 0 ;
        layout.VADDR = longfield<32,39>(insn_long);
        layout.VDATA = longfield<40,47>(insn_long);
        assert(isArrayIndexValid(ENC_MUBUF_insn_table, layout.OP) && "Opcode over or underflow");
        const amdgpu_gfx90a_insn_entry &insn_entry = ENC_MUBUF_insn_table[layout.OP];
        this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
        finalizeENC_MUBUFOperands();
        this->insn_in_progress->updateSize(insn_size + immLen);
        this->insn_in_progress->updateMnemonic(std::string(insn_entry.mnemonic) + extension);
    }

    void InstructionDecoder_amdgpu_gfx90a::decodeENC_MTBUF()
    {
        insn_size = 8;
        layout_ENC_MTBUF & layout = insn_layout.ENC_MTBUF;
        layout.ACC = longfield<55,55>(insn_long);
        layout.DFMT = longfield<19,22>(insn_long);
        layout.ENCODING = longfield<26,31>(insn_long);
        layout.GLC = longfield<14,14>(insn_long);
        layout.IDXEN = longfield<13,13>(insn_long);
        layout.NFMT = longfield<23,25>(insn_long);
        layout.OFFEN = longfield<12,12>(insn_long);
        layout.OFFSET = longfield<0,11>(insn_long);
        layout.OP = longfield<15,18>(insn_long);
        layout.SCC = longfield<53,53>(insn_long);
        layout.SLC = longfield<54,54>(insn_long);
        layout.SOFFSET = longfield<56,63>(insn_long);
        layout.SRSRC = (longfield<48,52>(insn_long) << 2 ) | 0 ;
        layout.VADDR = longfield<32,39>(insn_long);
        layout.VDATA = longfield<40,47>(insn_long);
        assert(isArrayIndexValid(ENC_MTBUF_insn_table, layout.OP) && "Opcode over or underflow");
        const amdgpu_gfx90a_insn_entry &insn_entry = ENC_MTBUF_insn_table[layout.OP];
        this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
        finalizeENC_MTBUFOperands();
        this->insn_in_progress->updateSize(insn_size + immLen);
        this->insn_in_progress->updateMnemonic(std::string(insn_entry.mnemonic) + extension);
    }

    void InstructionDecoder_amdgpu_gfx90a::decodeENC_MIMG()
    {
        insn_size = 8;
        layout_ENC_MIMG & layout = insn_layout.ENC_MIMG;
        layout.A16 = longfield<15,15>(insn_long);
        layout.ACC = longfield<16,16>(insn_long);
        layout.D16 = longfield<63,63>(insn_long);
        layout.DA = longfield<14,14>(insn_long);
        layout.DMASK = longfield<8,11>(insn_long);
        layout.ENCODING = longfield<26,31>(insn_long);
        layout.GLC = longfield<13,13>(insn_long);
        layout.LWE = longfield<17,17>(insn_long);
        layout.OP = longfield<18,24>(insn_long);
        layout.OPM = longfield<0,0>(insn_long);
        layout.SCC = longfield<7,7>(insn_long);
        layout.SLC = longfield<25,25>(insn_long);
        layout.SRSRC = (longfield<48,52>(insn_long) << 2 ) | 0 ;
        layout.SSAMP = (longfield<53,57>(insn_long) << 2 ) | 0 ;
        layout.UNORM = longfield<12,12>(insn_long);
        layout.VADDR = longfield<32,39>(insn_long);
        layout.VDATA = longfield<40,47>(insn_long);
        assert(isArrayIndexValid(ENC_MIMG_insn_table, layout.OP) && "Opcode over or underflow");
        const amdgpu_gfx90a_insn_entry &insn_entry = ENC_MIMG_insn_table[layout.OP];
        this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
        finalizeENC_MIMGOperands();
        this->insn_in_progress->updateSize(insn_size + immLen);
        this->insn_in_progress->updateMnemonic(std::string(insn_entry.mnemonic) + extension);
    }

    void InstructionDecoder_amdgpu_gfx90a::decodeENC_FLAT()
    {
        insn_size = 8;
        layout_ENC_FLAT & layout = insn_layout.ENC_FLAT;
        layout.ACC = longfield<55,55>(insn_long);
        layout.ADDR = longfield<32,39>(insn_long);
        layout.DATA = longfield<40,47>(insn_long);
        layout.ENCODING = longfield<26,31>(insn_long);
        layout.GLC = longfield<16,16>(insn_long);
        layout.LDS = longfield<13,13>(insn_long);
        layout.OFFSET = longfield<0,11>(insn_long);
        layout.OP = longfield<18,24>(insn_long);
        layout.SADDR = longfield<48,54>(insn_long);
        layout.SCC = longfield<25,25>(insn_long);
        layout.SEG = longfield<14,15>(insn_long);
        layout.SLC = longfield<17,17>(insn_long);
        layout.VDST = longfield<56,63>(insn_long);
        assert(isArrayIndexValid(ENC_FLAT_insn_table, layout.OP) && "Opcode over or underflow");
        const amdgpu_gfx90a_insn_entry &insn_entry = ENC_FLAT_insn_table[layout.OP];
        this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
        finalizeENC_FLATOperands();
        this->insn_in_progress->updateSize(insn_size + immLen);
        this->insn_in_progress->updateMnemonic(std::string(insn_entry.mnemonic) + extension);
    }

    void InstructionDecoder_amdgpu_gfx90a::decodeENC_FLAT_GLBL()
    {
        insn_size = 8;
        layout_ENC_FLAT_GLBL & layout = insn_layout.ENC_FLAT_GLBL;
        layout.ACC = longfield<55,55>(insn_long);
        layout.ADDR = longfield<32,39>(insn_long);
        layout.DATA = longfield<40,47>(insn_long);
        layout.ENCODING = longfield<26,31>(insn_long);
        layout.GLC = longfield<16,16>(insn_long);
        layout.LDS = longfield<13,13>(insn_long);
        layout.OFFSET = longfield<0,12>(insn_long);
        layout.OP = longfield<18,24>(insn_long);
        layout.SADDR = longfield<48,54>(insn_long);
        layout.SCC = longfield<25,25>(insn_long);
        layout.SEG = longfield<14,15>(insn_long);
        layout.SLC = longfield<17,17>(insn_long);
        layout.VDST = longfield<56,63>(insn_long);
        assert(isArrayIndexValid(ENC_FLAT_GLBL_insn_table, layout.OP) && "Opcode over or underflow");
        const amdgpu_gfx90a_insn_entry &insn_entry = ENC_FLAT_GLBL_insn_table[layout.OP];
        this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
        finalizeENC_FLAT_GLBLOperands();
        this->insn_in_progress->updateSize(insn_size + immLen);
        this->insn_in_progress->updateMnemonic(std::string(insn_entry.mnemonic) + extension);
    }

    void InstructionDecoder_amdgpu_gfx90a::decodeENC_FLAT_SCRATCH()
    {
        insn_size = 8;
        layout_ENC_FLAT_SCRATCH & layout = insn_layout.ENC_FLAT_SCRATCH;
        layout.ACC = longfield<55,55>(insn_long);
        layout.ADDR = longfield<32,39>(insn_long);
        layout.DATA = longfield<40,47>(insn_long);
        layout.ENCODING = longfield<26,31>(insn_long);
        layout.GLC = longfield<16,16>(insn_long);
        layout.LDS = longfield<13,13>(insn_long);
        layout.OFFSET = longfield<0,12>(insn_long);
        layout.OP = longfield<18,24>(insn_long);
        layout.SADDR = longfield<48,54>(insn_long);
        layout.SCC = longfield<25,25>(insn_long);
        layout.SEG = longfield<14,15>(insn_long);
        layout.SLC = longfield<17,17>(insn_long);
        layout.VDST = longfield<56,63>(insn_long);
        assert(isArrayIndexValid(ENC_FLAT_SCRATCH_insn_table, layout.OP) && "Opcode over or underflow");
        const amdgpu_gfx90a_insn_entry &insn_entry = ENC_FLAT_SCRATCH_insn_table[layout.OP];
        this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
        finalizeENC_FLAT_SCRATCHOperands();
        this->insn_in_progress->updateSize(insn_size + immLen);
        this->insn_in_progress->updateMnemonic(std::string(insn_entry.mnemonic) + extension);
    }

    void InstructionDecoder_amdgpu_gfx90a::decodeSOPK_INST_LITERAL_()
    {
        insn_size = 8;
        layout_SOPK_INST_LITERAL_ & layout = insn_layout.SOPK_INST_LITERAL_;
        layout.ENCODING = longfield<28,31>(insn_long);
        layout.OP = longfield<23,27>(insn_long);
        layout.SDST = longfield<16,22>(insn_long);
        layout.SIMM16 = longfield<0,15>(insn_long);
        layout.SIMM32 = longfield<32,63>(insn_long);
        assert(isArrayIndexValid(SOPK_INST_LITERAL__insn_table, layout.OP) && "Opcode over or underflow");
        const amdgpu_gfx90a_insn_entry &insn_entry = SOPK_INST_LITERAL__insn_table[layout.OP];
        this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
        finalizeSOPK_INST_LITERAL_Operands();
        this->insn_in_progress->updateSize(insn_size + immLen);
        this->insn_in_progress->updateMnemonic(std::string(insn_entry.mnemonic) + extension);
    }

    void InstructionDecoder_amdgpu_gfx90a::decodeENC_VOP2_LITERAL()
    {
        insn_size = 8;
        layout_ENC_VOP2_LITERAL & layout = insn_layout.ENC_VOP2_LITERAL;
        layout.ENCODING = longfield<31,31>(insn_long);
        layout.OP = longfield<25,30>(insn_long);
        layout.SIMM32 = longfield<32,63>(insn_long);
        layout.SRC0 = longfield<0,8>(insn_long);
        layout.VDST = longfield<17,24>(insn_long);
        layout.VSRC1 = longfield<9,16>(insn_long);
        assert(isArrayIndexValid(ENC_VOP2_LITERAL_insn_table, layout.OP) && "Opcode over or underflow");
        const amdgpu_gfx90a_insn_entry &insn_entry = ENC_VOP2_LITERAL_insn_table[layout.OP];
        this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
        finalizeENC_VOP2_LITERALOperands();
        this->insn_in_progress->updateSize(insn_size + immLen);
        this->insn_in_progress->updateMnemonic(std::string(insn_entry.mnemonic) + extension);
    }

    void InstructionDecoder_amdgpu_gfx90a::decodeENC_VOP3B()
    {
        insn_size = 8;
        layout_ENC_VOP3B & layout = insn_layout.ENC_VOP3B;
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
        assert(isArrayIndexValid(ENC_VOP3B_insn_table, layout.OP) && "Opcode over or underflow");
        const amdgpu_gfx90a_insn_entry &insn_entry = ENC_VOP3B_insn_table[layout.OP];
        this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
        finalizeENC_VOP3BOperands();
        this->insn_in_progress->updateSize(insn_size + immLen);
        this->insn_in_progress->updateMnemonic(std::string(insn_entry.mnemonic) + extension);
    }

    void InstructionDecoder_amdgpu_gfx90a::decodeENC_VOP3P_MFMA()
    {
        insn_size = 8;
        layout_ENC_VOP3P_MFMA & layout = insn_layout.ENC_VOP3P_MFMA;
        layout.ABID = longfield<11,14>(insn_long);
        layout.ACC = longfield<59,60>(insn_long);
        layout.ACC_CD = longfield<15,15>(insn_long);
        layout.BLGP = longfield<61,63>(insn_long);
        layout.CBSZ = longfield<8,10>(insn_long);
        layout.ENCODING = longfield<23,31>(insn_long);
        layout.OP = longfield<16,22>(insn_long);
        layout.SRC0 = longfield<32,40>(insn_long);
        layout.SRC1 = longfield<41,49>(insn_long);
        layout.SRC2 = longfield<50,58>(insn_long);
        layout.VDST = longfield<0,7>(insn_long);
        assert(isArrayIndexValid(ENC_VOP3P_MFMA_insn_table, layout.OP) && "Opcode over or underflow");
        const amdgpu_gfx90a_insn_entry &insn_entry = ENC_VOP3P_MFMA_insn_table[layout.OP];
        this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
        finalizeENC_VOP3P_MFMAOperands();
        this->insn_in_progress->updateSize(insn_size + immLen);
        this->insn_in_progress->updateMnemonic(std::string(insn_entry.mnemonic) + extension);
    }

    void InstructionDecoder_amdgpu_gfx90a::mainDecodeOpcode()
    {
        if (IS_ENC_SOP1(insn_long))  {
            insn_size = 4;
            uint32_t op_value = longfield<8,15>(insn_long);
            assert(isArrayIndexValid(ENC_SOP1_insn_table, op_value) && "Opcode over or underflow");
            const amdgpu_gfx90a_insn_entry &insn_entry = ENC_SOP1_insn_table[op];
            this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
            instr_family = ENC_SOP1;
        }
        else if (IS_ENC_SOPC(insn_long))  {
            insn_size = 4;
            uint32_t op_value = longfield<16,22>(insn_long);
            assert(isArrayIndexValid(ENC_SOPC_insn_table, op_value) && "Opcode over or underflow");
            const amdgpu_gfx90a_insn_entry &insn_entry = ENC_SOPC_insn_table[op];
            this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
            instr_family = ENC_SOPC;
        }
        else if (IS_ENC_SOPP(insn_long))  {
            insn_size = 4;
            uint32_t op_value = longfield<16,22>(insn_long);
            assert(isArrayIndexValid(ENC_SOPP_insn_table, op_value) && "Opcode over or underflow");
            const amdgpu_gfx90a_insn_entry &insn_entry = ENC_SOPP_insn_table[op];
            this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
            instr_family = ENC_SOPP;
        }
        else if (IS_ENC_SOPK(insn_long))  {
            insn_size = 4;
            uint32_t op_value = longfield<23,27>(insn_long);
            assert(isArrayIndexValid(ENC_SOPK_insn_table, op_value) && "Opcode over or underflow");
            const amdgpu_gfx90a_insn_entry &insn_entry = ENC_SOPK_insn_table[op];
            this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
            instr_family = ENC_SOPK;
        }
        else if (IS_ENC_SOP2(insn_long))  {
            insn_size = 4;
            uint32_t op_value = longfield<23,29>(insn_long);
            assert(isArrayIndexValid(ENC_SOP2_insn_table, op_value) && "Opcode over or underflow");
            const amdgpu_gfx90a_insn_entry &insn_entry = ENC_SOP2_insn_table[op];
            this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
            instr_family = ENC_SOP2;
        }
        else if (IS_ENC_SMEM(insn_long))  {
            insn_size = 8;
            uint32_t op_value = longfield<18,25>(insn_long);
            assert(isArrayIndexValid(ENC_SMEM_insn_table, op_value) && "Opcode over or underflow");
            const amdgpu_gfx90a_insn_entry &insn_entry = ENC_SMEM_insn_table[op];
            this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
            instr_family = ENC_SMEM;
        }
        else if (IS_ENC_VOP1(insn_long))  {
            insn_size = 4;
            uint32_t op_value = longfield<9,16>(insn_long);
            assert(isArrayIndexValid(ENC_VOP1_insn_table, op_value) && "Opcode over or underflow");
            const amdgpu_gfx90a_insn_entry &insn_entry = ENC_VOP1_insn_table[op];
            this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
            instr_family = ENC_VOP1;
        }
        else if (IS_ENC_VOPC(insn_long))  {
            insn_size = 4;
            uint32_t op_value = longfield<17,24>(insn_long);
            assert(isArrayIndexValid(ENC_VOPC_insn_table, op_value) && "Opcode over or underflow");
            const amdgpu_gfx90a_insn_entry &insn_entry = ENC_VOPC_insn_table[op];
            this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
            instr_family = ENC_VOPC;
        }
        else if (IS_ENC_VOP2(insn_long))  {
            insn_size = 4;
            uint32_t op_value = longfield<25,30>(insn_long);
            assert(isArrayIndexValid(ENC_VOP2_insn_table, op_value) && "Opcode over or underflow");
            const amdgpu_gfx90a_insn_entry &insn_entry = ENC_VOP2_insn_table[op];
            this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
            instr_family = ENC_VOP2;
        }
        else if (IS_ENC_VINTRP(insn_long))  {
            insn_size = 4;
            uint32_t op_value = longfield<16,17>(insn_long);
            assert(isArrayIndexValid(ENC_VINTRP_insn_table, op_value) && "Opcode over or underflow");
            const amdgpu_gfx90a_insn_entry &insn_entry = ENC_VINTRP_insn_table[op];
            this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
            instr_family = ENC_VINTRP;
        }
        else if (IS_ENC_VOP3P(insn_long))  {
            insn_size = 8;
            uint32_t op_value = longfield<16,22>(insn_long);
            assert(isArrayIndexValid(ENC_VOP3P_insn_table, op_value) && "Opcode over or underflow");
            const amdgpu_gfx90a_insn_entry &insn_entry = ENC_VOP3P_insn_table[op];
            this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
            instr_family = ENC_VOP3P;
        }
        else if (IS_ENC_VOP3(insn_long))  {
            insn_size = 8;
            uint32_t op_value = longfield<16,25>(insn_long);
            assert(isArrayIndexValid(ENC_VOP3_insn_table, op_value) && "Opcode over or underflow");
            const amdgpu_gfx90a_insn_entry &insn_entry = ENC_VOP3_insn_table[op];
            this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
            instr_family = ENC_VOP3;
        }
        else if (IS_ENC_DS(insn_long))  {
            insn_size = 8;
            uint32_t op_value = longfield<17,24>(insn_long);
            assert(isArrayIndexValid(ENC_DS_insn_table, op_value) && "Opcode over or underflow");
            const amdgpu_gfx90a_insn_entry &insn_entry = ENC_DS_insn_table[op];
            this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
            instr_family = ENC_DS;
        }
        else if (IS_ENC_MUBUF(insn_long))  {
            insn_size = 8;
            uint32_t op_value = longfield<18,24>(insn_long);
            assert(isArrayIndexValid(ENC_MUBUF_insn_table, op_value) && "Opcode over or underflow");
            const amdgpu_gfx90a_insn_entry &insn_entry = ENC_MUBUF_insn_table[op];
            this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
            instr_family = ENC_MUBUF;
        }
        else if (IS_ENC_MTBUF(insn_long))  {
            insn_size = 8;
            uint32_t op_value = longfield<15,18>(insn_long);
            assert(isArrayIndexValid(ENC_MTBUF_insn_table, op_value) && "Opcode over or underflow");
            const amdgpu_gfx90a_insn_entry &insn_entry = ENC_MTBUF_insn_table[op];
            this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
            instr_family = ENC_MTBUF;
        }
        else if (IS_ENC_MIMG(insn_long))  {
            insn_size = 8;
            uint32_t op_value = longfield<18,24>(insn_long);
            assert(isArrayIndexValid(ENC_MIMG_insn_table, op_value) && "Opcode over or underflow");
            const amdgpu_gfx90a_insn_entry &insn_entry = ENC_MIMG_insn_table[op];
            this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
            instr_family = ENC_MIMG;
        }
        else if (IS_ENC_FLAT(insn_long))  {
            insn_size = 8;
            uint32_t op_value = longfield<18,24>(insn_long);
            assert(isArrayIndexValid(ENC_FLAT_insn_table, op_value) && "Opcode over or underflow");
            const amdgpu_gfx90a_insn_entry &insn_entry = ENC_FLAT_insn_table[op];
            this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
            instr_family = ENC_FLAT;
        }
        else if (IS_ENC_FLAT_GLBL(insn_long))  {
            insn_size = 8;
            uint32_t op_value = longfield<18,24>(insn_long);
            assert(isArrayIndexValid(ENC_FLAT_GLBL_insn_table, op_value) && "Opcode over or underflow");
            const amdgpu_gfx90a_insn_entry &insn_entry = ENC_FLAT_GLBL_insn_table[op];
            this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
            instr_family = ENC_FLAT_GLBL;
        }
        else if (IS_ENC_FLAT_SCRATCH(insn_long))  {
            insn_size = 8;
            uint32_t op_value = longfield<18,24>(insn_long);
            assert(isArrayIndexValid(ENC_FLAT_SCRATCH_insn_table, op_value) && "Opcode over or underflow");
            const amdgpu_gfx90a_insn_entry &insn_entry = ENC_FLAT_SCRATCH_insn_table[op];
            this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
            instr_family = ENC_FLAT_SCRATCH;
        }
        else if (IS_SOPK_INST_LITERAL_(insn_long))  {
            insn_size = 8;
            uint32_t op_value = longfield<23,27>(insn_long);
            assert(isArrayIndexValid(SOPK_INST_LITERAL__insn_table, op_value) && "Opcode over or underflow");
            const amdgpu_gfx90a_insn_entry &insn_entry = SOPK_INST_LITERAL__insn_table[op];
            this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
            instr_family = SOPK_INST_LITERAL_;
        }
        else if (IS_ENC_VOP2_LITERAL(insn_long))  {
            insn_size = 8;
            uint32_t op_value = longfield<25,30>(insn_long);
            assert(isArrayIndexValid(ENC_VOP2_LITERAL_insn_table, op_value) && "Opcode over or underflow");
            const amdgpu_gfx90a_insn_entry &insn_entry = ENC_VOP2_LITERAL_insn_table[op];
            this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
            instr_family = ENC_VOP2_LITERAL;
        }
        else if (IS_ENC_VOP3B(insn_long))  {
            insn_size = 8;
            uint32_t op_value = longfield<16,25>(insn_long);
            assert(isArrayIndexValid(ENC_VOP3B_insn_table, op_value) && "Opcode over or underflow");
            const amdgpu_gfx90a_insn_entry &insn_entry = ENC_VOP3B_insn_table[op];
            this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
            instr_family = ENC_VOP3B;
        }
        else if (IS_ENC_VOP3P_MFMA(insn_long))  {
            insn_size = 8;
            uint32_t op_value = longfield<16,22>(insn_long);
            assert(isArrayIndexValid(ENC_VOP3P_MFMA_insn_table, op_value) && "Opcode over or underflow");
            const amdgpu_gfx90a_insn_entry &insn_entry = ENC_VOP3P_MFMA_insn_table[op];
            this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
            instr_family = ENC_VOP3P_MFMA;
        }
    }

    void InstructionDecoder_amdgpu_gfx90a::mainDecode()
    {
        if (IS_ENC_SOP1(insn_long))  {
            decodeENC_SOP1();
        }
        else if (IS_ENC_SOPC(insn_long))  {
            decodeENC_SOPC();
        }
        else if (IS_ENC_SOPP(insn_long))  {
            decodeENC_SOPP();
        }
        else if (IS_ENC_SOPK(insn_long))  {
            decodeENC_SOPK();
        }
        else if (IS_ENC_SOP2(insn_long))  {
            decodeENC_SOP2();
        }
        else if (IS_ENC_SMEM(insn_long))  {
            decodeENC_SMEM();
        }
        else if (IS_ENC_VOP1(insn_long))  {
            decodeENC_VOP1();
        }
        else if (IS_ENC_VOPC(insn_long))  {
            decodeENC_VOPC();
        }
        else if (IS_ENC_VOP2(insn_long))  {
            decodeENC_VOP2();
        }
        else if (IS_ENC_VINTRP(insn_long))  {
            decodeENC_VINTRP();
        }
        else if (IS_ENC_VOP3P(insn_long))  {
            decodeENC_VOP3P();
        }
        else if (IS_ENC_VOP3(insn_long))  {
            decodeENC_VOP3();
        }
        else if (IS_ENC_DS(insn_long))  {
            decodeENC_DS();
        }
        else if (IS_ENC_MUBUF(insn_long))  {
            decodeENC_MUBUF();
        }
        else if (IS_ENC_MTBUF(insn_long))  {
            decodeENC_MTBUF();
        }
        else if (IS_ENC_MIMG(insn_long))  {
            decodeENC_MIMG();
        }
        else if (IS_ENC_FLAT(insn_long))  {
            decodeENC_FLAT();
        }
        else if (IS_ENC_FLAT_GLBL(insn_long))  {
            decodeENC_FLAT_GLBL();
        }
        else if (IS_ENC_FLAT_SCRATCH(insn_long))  {
            decodeENC_FLAT_SCRATCH();
        }
        else if (IS_SOPK_INST_LITERAL_(insn_long))  {
            decodeSOPK_INST_LITERAL_();
        }
        else if (IS_ENC_VOP2_LITERAL(insn_long))  {
            decodeENC_VOP2_LITERAL();
        }
        else if (IS_ENC_VOP3B(insn_long))  {
            decodeENC_VOP3B();
        }
        else if (IS_ENC_VOP3P_MFMA(insn_long))  {
            decodeENC_VOP3P_MFMA();
        }
    }

}
}
