#ifndef RT_COMPAT_
#define RT_COMPAT_

#if defined(rs6000_ibm_aix4_1)
/* sync on powerPC is actually more general than just a memory barrier,
   more like a total execution barrier, but the general use we are concerned
   of here is as a memory barrier 
*/
#define MEMORY_BARRIER     asm volatile ("sync")
#else
#define MEMORY_BARRIER
#endif


#endif

