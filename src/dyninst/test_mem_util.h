/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */
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


#ifdef rs6000_ibm_aix4_1_test
extern const unsigned int loadExp;
extern const unsigned int storeExp;
extern const unsigned int prefeExp;
extern const unsigned int accessExp;
extern const unsigned int accessExpCC;

extern unsigned int bcExp[];

extern int eaExpOffset[];

extern void* eaExp[]; /* forward */
/* Make sure this is 4 words long... */
extern int divarw[4];
extern float dfvars[4];
extern double dfvard[4];

extern void* gettoc();
#ifdef rs6000_ibm_aix4_1_test
/* Had trouble with this one */
extern void *getsp(int, int, int);
#else
extern void* getsp();
#endif

#ifdef __GNUC__
#define _inline inline
#endif

extern /* _inline */ void init_test_data();
#endif /* defined(rs6000_ibm_aix4_1_test) */

#if defined(i386_unknown_linux2_0_test) \
 || defined(i386_unknown_nt4_0_test) \
 || (defined(os_freebsd_test) && defined(arch_x86_test))
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

extern int divarw[4];
extern float dfvars[4];
extern double dfvard[4];
extern long double dfvart; /* 10 byte hopefully, but it shouldn't matter... */
extern unsigned char dlarge[512];

#define CAP_MMX   (1<<23)
#define CAP_SSE   (1<<25)
#define CAP_SSE2  (1<<26)
#define CAP_3DNOW (1<<31)

void reduce(const struct reduction x);

void reduceCC(const struct reduction x);

void init_test_data();
#endif /* defined(i386_unknown_linux2_0_test) || defined(i386_unknown_nt4_0_test) */

#if defined(x86_64_unknown_linux2_4_test) || defined(amd64_unknown_freebsd7_0_test)

extern unsigned int loadExp;
extern unsigned int storeExp;
extern unsigned int prefeExp;
extern unsigned int accessExp;
extern unsigned int accessExpCC;

extern int eaExpOffset[];

extern unsigned int bcExp[];

extern int divarw[4];
extern float dfvars[4];
extern double dfvard[4];
extern long double dfvart;
extern char dlarge[512];

extern void* rip_relative_load_address;

void init_test_data();

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

extern int ia32features();
extern int amd_features();
#define CAP_MMX   (1<<23)
#define CAP_SSE   (1<<25)
#define CAP_SSE2  (1<<26)
#define CAP_3DNOW (1<<31)
void reduce(const struct reduction x);
void reduceCC(const struct reduction x);

#endif /* defined(x86_64_unknown_linux2_4_test) */

#if defined(arch_power_test) && defined(os_linux_test)
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
void listEffAddr(const char* insn, void* addr);
void listByteCnt(const char* insn, unsigned int count);
void listEffAddrCC(const char* insn, void* addr);
void listByteCntCC(const char* insn, unsigned int count);


#ifdef __cplusplus
}
#endif

#endif /* !defined(TEST_MEM_UTIL_H) */
