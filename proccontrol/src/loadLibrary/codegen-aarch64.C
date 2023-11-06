// Linux-specific routines to generate a code sequence to load a library
// into a target process.

#include "codegen.h"
#include "Symbol.h"
#include "PCProcess.h"
#include "codegen.h"

#include "common/src/arch-aarch64.h"

#include "unaligned_memory_access.h"

using namespace Dyninst;
using namespace NS_aarch64;
using namespace ProcControlAPI;
using namespace std;

#define BYTE_ASSGN(BUF, POS, VAL)\
            (*(((char *) BUF) + POS + 1)) |= ((VAL>>11)&0x1f);\
            (*(((char *) BUF) + POS + 2)) |= ((VAL>> 3)&0xff);\
            (*(((char *) BUF) + POS + 3)) |= ((VAL<< 5)&0xf0);

#define SWAP4BYTE(BUF, POS) \
            BUF[POS+3]^= BUF[POS]; \
            BUF[POS]  ^= BUF[POS+3]; \
            BUF[POS+3]^= BUF[POS]; \
            BUF[POS+2]^= BUF[POS+1]; \
            BUF[POS+1]^= BUF[POS+2]; \
            BUF[POS+2]^= BUF[POS+1];

#define X8 (8)


bool Codegen::generatePreambleAARCH64(){
    //temporily I make 256byte space
//#warning "This needs to be verified!"
    //sub sp, sp, #48   ;#48 in decimal
    unsigned long addr = copyInt(0xd100c3ff); //#48
    pthrd_printf("generate Preamble:\n");
    pthrd_printf("0x%8lx: 0x%8x\n",addr, 0xd10403ff);
    return true;
}

bool Codegen::generateCallAARCH64(Address addr, const std::vector<Address> &args){
    if( args.size() > 6)
        return false;

    char movLong_buf[] = {
        (char)0xd2, (char)0x80, (char)0x00, (char)0x00,     // mov  x0, #0
        (char)0xf2, (char)0xa0, (char)0x00, (char)0x00,     // movk x0, #0, lsl #16
        (char)0xf2, (char)0xc0, (char)0x00, (char)0x00,     // movk x0, #0, lsl #32
        (char)0xf2, (char)0xe0, (char)0x00, (char)0x00,     // movk x0, #0, lsl #48
    };

    /*
    char blr_buf[] = {
        (char)0xd6, (char)0x3f, (char)0x00, (char)0x00,     // blr x0
    };
    */

    char* _buf;
    _buf = (char *)malloc(sizeof(movLong_buf));

    //pass the args
    pthrd_printf("generate Call aarch64:\n");
    unsigned int regIndex=0;
    unsigned long retAddr;
    for(std::vector<Address>::const_iterator itA = args.begin();
            itA != args.end(); itA++){
        assert( regIndex < 6 ); //max args num
        unsigned long val = *itA;
        memcpy(_buf, movLong_buf, sizeof(movLong_buf));

        //one mov long combo
        for(unsigned int i = 0; i <= 12; i+=4){
            BYTE_ASSGN(_buf, i,  (uint16_t)(val>>(i*4)) )
            SWAP4BYTE(_buf, i)
            retAddr = copyInt( Dyninst::read_memory_as<uint32_t>(_buf+i) + regIndex);
            pthrd_printf("0x%8lx: 0x%8x\n", retAddr, Dyninst::read_memory_as<uint32_t>(_buf+i) + regIndex);
        }
        regIndex ++;
    }

    // mov x8, <addr>
    memcpy(_buf, movLong_buf, sizeof(movLong_buf));
    for(unsigned int i = 0; i <= 12; i+=4){
        BYTE_ASSGN(_buf, i,  (uint16_t)(addr>>(i*4)) )
        SWAP4BYTE(_buf, i)
        retAddr = copyInt( Dyninst::read_memory_as<uint32_t>(_buf+i) + X8); //x8 = 8
        pthrd_printf("0x%8lx: 0x%8x\n", retAddr, Dyninst::read_memory_as<uint32_t>(_buf+i) + X8);
    }

    //blr x8
    unsigned int blrInst = 0xd63f0100 ;
    retAddr = copyInt( blrInst ); //x8 = 8
    pthrd_printf("0x%8lx: 0x%8x\n", retAddr, blrInst);

    return true;
}
