#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "mrnet/src/byte_order.h"
#include "mrnet/src/config.h"

void byte_swap(char * out, char * in, uint32_t nelems, uint32_t elemsize)
{
  uint32_t i, j;

  for(i=0; i<nelems*elemsize; i+=elemsize){
    for(j=0; j<elemsize; j++){
      out[i+j] = in[i + elemsize - j - 1];
    }
  }
}

#define SWAP2(a, b) {char t; t = *(a); *(a)=*(b); *(b)=t;}
void byte_swap_inplace(char * inout, uint32_t nelems, uint32_t elemsize)
{
  uint32_t i, j;

  for(i=0; i<nelems*elemsize; i+=elemsize){
    for(j=0; j<elemsize/2; j++){
      SWAP2(inout+i+j, inout+i+elemsize-j-1);
    }
  }
}

void hton_bytes(char * out, char * in, uint32_t elsize){
#if defined(WORDS_BIGENDIAN)
  memcpy(out, in, elsize);
#else
  byte_swap(out, in, 1, elsize);
#endif
}
