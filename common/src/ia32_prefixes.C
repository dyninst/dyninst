#include "ia32_prefixes.h"

namespace NS_x86 {

  int count_prefixes(unsigned insnType) {
    int nPrefixes = 0;
    if(insnType & PREFIX_OPR) {
      nPrefixes++;
    }
    if(insnType & PREFIX_SEG) {
      nPrefixes++;
    }
    if(insnType & PREFIX_OPCODE) {
      nPrefixes++;
    }
    if(insnType & PREFIX_REX) {
      nPrefixes++;
    }
    if(insnType & PREFIX_INST) {
      nPrefixes++;
    }
    if(insnType & PREFIX_ADDR) {
      nPrefixes++;
    }

    if(insnType & PREFIX_AVX) { /* VEX2 */
      nPrefixes += 2;
    } else if(insnType & PREFIX_AVX2) { /* VEX3 */
      nPrefixes += 3;
    } else if(insnType & PREFIX_AVX512) { /* EVEX */
      nPrefixes += 4;
    }

    /* Make sure dyninst didn't decode more than one VEX prefix. */
    if(((insnType & PREFIX_AVX) && (insnType & PREFIX_AVX2)) ||
       ((insnType & PREFIX_AVX2) && (insnType & PREFIX_AVX512)) ||
       ((insnType & PREFIX_AVX512) && (insnType & PREFIX_AVX)) ||
       ((insnType & PREFIX_AVX) && (insnType & PREFIX_AVX2) && (insnType & PREFIX_AVX512))) {
      assert(!"An instruction can only have one type of VEX prefix!\n");
    }

    return nPrefixes;
  }

  unsigned copy_prefixes(const unsigned char *&origInsn, unsigned char *&newInsn,
                         unsigned insnType) {
    unsigned nPrefixes = count_prefixes(insnType);

    for(unsigned u = 0; u < nPrefixes; u++) {
      *newInsn++ = *origInsn++;
    }

    return nPrefixes;
  }

  // Copy all prefixes but the Operand-Size and Address-Size prefixes (0x66 and 0x67)
  unsigned copy_prefixes_nosize(const unsigned char *&origInsn, unsigned char *&newInsn,
                                unsigned insnType) {
    unsigned nPrefixes = count_prefixes(insnType);

    for(unsigned u = 0; u < nPrefixes; u++) {
      if(*origInsn == 0x66 || *origInsn == 0x67) {
        origInsn++;
        continue;
      }
      *newInsn++ = *origInsn++;
    }
    return nPrefixes;
  }
}
