#ifndef TEST_MEM_UTIL_H
#define TEST_MEM_UTIL_H

#ifdef __cplusplus
extern "C" {
#endif

/* Sun Forte/WorkShop cc releases older than 6.2 do not like these defines: */
#if !defined(__SUNPRO_C) || (__SUNPRO_C >= 0x530)
#define passorfail(i,p,d,r) if((p)) { \
                              logerror("Passed test #%d (%s)\n", (i), (d)); \
                              passedTest[(i)] = TRUE; \
                            } else { \
                              logerror("\n**Failed** test #%d (%s): %s\n", (i), (d), (r)); \
                            }

#define skiptest(i,d) { logerror("Skipping test #%d (%s)\n", (i), (d)); \
                        logerror("    not implemented on this platform\n"); \
                        passedTest[(i)] = TRUE; }
#else
void passorfail(int i, int p, char* d, char* r);
void skiptest(int i, char* d);
#endif

extern long loadsnstores(long, long, long); /* ILP32 & LP64 */

extern int result_of_loadsnstores;

extern unsigned int loadCnt;
extern unsigned int storeCnt;
extern unsigned int prefeCnt;
extern unsigned int accessCnt;

extern unsigned int accessCntEA;
extern unsigned int accessCntBC;
extern int doomEA;
extern int doomBC;
extern void* eaList[1000];
extern unsigned int bcList[1000];
extern void* eaExp[1000];

extern unsigned int accessCntEAcc;
extern unsigned int accessCntBCcc;
extern int doomEAcc;
extern int doomBCcc;
extern void* eaListCC[1000];
extern unsigned int bcListCC[1000];
extern void* eaExpCC[1000];
extern unsigned int bcExpCC[1000];

#ifdef sparc_sun_solaris2_4
extern /* const */ unsigned int loadExp;
extern /* const */ unsigned int storeExp;
extern /* const */ unsigned int prefeExp;
extern /* const */ unsigned int accessExp;
extern /* const */ unsigned int accessExpCC;

extern unsigned int bcExp[];

extern int eaExpOffset[];

extern void* eaExp[]; /* forward */
extern int divarw;
extern float dfvars;
extern double dfvard;
extern long double dfvarq;

extern /* _inline */ void init_test_data();
#endif /* defined(sparc_sun_solaris2_4) */

#ifdef rs6000_ibm_aix4_1
extern const unsigned int loadExp;
extern const unsigned int storeExp;
extern const unsigned int prefeExp;
extern const unsigned int accessExp;
extern const unsigned int accessExpCC;

extern unsigned int bcExp[];

extern int eaExpOffset[];

extern void* eaExp[]; /* forward */
extern int divarw;
extern float dfvars;
extern double dfvard;

extern void* gettoc();
#ifdef rs6000_ibm_aix4_1
/* Had trouble with this one */
extern void *getsp(int, int, int);
#else
extern void* getsp();
#endif

#ifdef __GNUC__
#define _inline inline
#endif

extern /* _inline */ void init_test_data();
#endif /* defined(rs6000_ibm_aix4_1) */

#if defined(i386_unknown_linux2_0) \
 || defined(i386_unknown_nt4_0)
extern unsigned int loadExp;
extern unsigned int storeExp;
extern unsigned int prefeExp;
extern unsigned int accessExp;
extern unsigned int accessExpCC;

struct reduction {
  unsigned int loadRed;
  unsigned int storeRed;
  unsigned int prefeRed;
  unsigned int axsRed;
  unsigned int axsShift;
};

extern const struct reduction mmxRed;
extern const struct reduction sseRed;
extern const struct reduction sse2Red;
extern const struct reduction amdRed;

extern const struct reduction ccRed;

extern int eaExpOffset[];

extern void* eaExp[]; /* forward */

extern unsigned int bcExp[];

extern int ia32features();
extern int amd_features();

extern int divarw;
extern float dfvars;
extern double dfvard;
extern long double dfvart; /* 10 byte hopefully, but it shouldn't matter... */
extern unsigned char dlarge[512];

#define CAP_MMX   (1<<23)
#define CAP_SSE   (1<<25)
#define CAP_SSE2  (1<<26)
#define CAP_3DNOW (1<<31)

void reduce(const struct reduction x);

void reduceCC(const struct reduction x);

void init_test_data();
#endif /* defined(i386_unknown_linux2_0) || defined(i386_unknown_nt4_0) */

#ifdef x86_64_unknown_linux2_4

extern unsigned int loadExp;
extern unsigned int storeExp;
extern unsigned int prefeExp;
extern unsigned int accessExp;
extern unsigned int accessExpCC;

extern int eaExpOffset[];

extern unsigned int bcExp[];

extern int divarw;
extern float dfvars;
extern double dfvard;
extern long double dfvart;
extern char dlarge[512];

extern void* rip_relative_load_address;

void init_test_data();
#endif /* defined(x86_64_unknown_linux2_4) */

#ifdef ia64_unknown_linux2_4

#define loadExp 6
#define storeExp 3
#define prefeExp 3

/* Other platforms don't seem to count prefetches as accesses.  I'm not sure why. */
#define accessExp 12
#define accessExpCC 12

extern unsigned int bcExp[];
extern unsigned int bcExpCC[];

/* FIXME: this should be made more complicated and/or assembly
   to actually test all the loads and stores that I know about
   and claim that Dyninst will recognize and handle.  This
   means redefining the stuff above to match up to the new
   code.
   
   FIXME: I don't understand what the "CC" stuff is or does. 
   
   FIXME: I don't understand what the "EA" stuff is or does. 
*/
extern long loadsnstores( long x, long y, long z );

void init_test_data();
#endif /* defined(ia64_unknown_linux2_4) */

#ifdef mips_sgi_irix6_4
#define loadExp 0
#define storeExp 0
#define prefeExp 0
#define accessExp 1
#define accessExpCC 1

long loadsnstores(long x, long y, long z);

extern unsigned int bcExp[];

void init_test_data();
#endif /* defined(mips_sgi_irix6_4) */

#ifdef alpha_dec_osf4_0
#define loadExp 0
#define storeExp 0
#define prefeExp 0
#define accessExp 1
#define accessExpCC 1

long loadsnstores(long x, long y, long z);

extern unsigned int bcExp[];

void init_test_data();
#endif /* defined(alpha_dec_osf4_0) */

#if defined(arch_power) && defined(os_linux)
#define loadExp 0
#define storeExp 0
#define prefeExp 0
#define accessExp 1
#define accessExpCC 1

long loadsnstores(long x, long y, long z);

extern unsigned int bcExp[];

void init_test_data();
#endif /* power linux */

/* functions called by the simple instrumentation points */
void countLoad();
void countStore();
void countPrefetch();
void countAccess();

int validateEA(void* ea1[], void* ea2[], unsigned int n);
int validateBC(unsigned int bc1[], unsigned int bc2[], unsigned int n);

/* functions called by the effective address/byte count instrumentation points */
void listEffAddr(void* addr);
void listByteCnt(unsigned int count);
void listEffAddrCC(void* addr);
void listByteCntCC(unsigned int count);


#ifdef __cplusplus
}
#endif

#endif /* !defined(TEST_MEM_UTIL_H) */
