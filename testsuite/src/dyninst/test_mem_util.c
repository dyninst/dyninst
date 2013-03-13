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
/* FIXME Remove assert() calls from this file, then remove this #include */
#include <assert.h>
#include <string.h>

#include "../src/dyninst/test_mem_util.h"
#include "../src/mutatee_util.h"

/* Sun Forte/WorkShop cc releases older than 6.2 do not like these defines: */
/* (macro versions of these calls are in test_mem_util.h) */
#if defined(__SUNPRO_C) && (__SUNPRO_C < 0x530)
void passorfail(int i, int p, char* d, char* r)
{
  if(p) {
    logerror("Passed test #%d (%s)\n", (i), (d));
    passedTest[(i)] = TRUE;
  } else {
    logerror("\n**Failed** test #%d (%s): %s\n", (i), (d), (r));
  }
}

void skiptest(int i, char* d)
{
  logerror("Skipping test #%d (%s)\n", (i), (d));
  logerror("    not implemented on this platform\n");
  passedTest[(i)] = TRUE;
}
#endif

int result_of_loadsnstores;

unsigned int loadCnt = 0;
unsigned int storeCnt = 0;
unsigned int prefeCnt = 0;
unsigned int accessCnt = 0;

unsigned int accessCntEA = 0;
unsigned int accessCntBC = 0;
int doomEA = 0;
int doomBC = 0;
void* eaList[1000];
unsigned int bcList[1000];
void* eaExp[1000];

unsigned int accessCntEAcc = 0;
unsigned int accessCntBCcc = 0;
int doomEAcc = 0;
int doomBCcc = 0;
void* eaListCC[1000];
unsigned int bcListCC[1000];
void* eaExpCC[1000];
unsigned int bcExpCC[1000];


#ifdef rs6000_ibm_aix4_1_test
const unsigned int loadExp=41;
const unsigned int storeExp=32;
const unsigned int prefeExp=0;
const unsigned int accessExp=73;
const unsigned int accessExpCC=73;

unsigned int bcExp[] = { 4,  1,1,1,1,  2,2,2,2,  2,2,2,2,  4,4,4,4,
			 4,4,4,  8,8,8,8,  1,1,1,1,  2,2,2,2,
			 4,4,4,4,  8,8,8,8,  2,4,2,4,  76,76,24,20,
			 20,20,  4,4,8,8,  4,  4,4,4,4,  4,  8,8,8,8,
			 4,4,4,4,  8,8,8,8,  4 };

int eaExpOffset[] =    { 0, 17,3,1,2,  0,4,2,0,  2,2,2,2,  0,4,4,4,
			 4,12,2,  0,0,0,0,  3,1,1,1,  2,6,2,2,
			 0,4,4,4,  0,0,0,0,  0,0,0,0,  -76,-76,-24,-24,
			 -20,-20,    0,0,0,0,  0,  0,4,0,4,  0,  0,8,8,8,
			 4,12,0,0,  0,8,8,8,  0 };

/* _inline */ void init_test_data()
{
  int i;

  void *toc = gettoc();
  void *sp  = getsp(1,2,3);

  dprintf("divarw = %p\n", divarw);
  dprintf("dfvars = %p\n", dfvars);
  dprintf("dfvard = %p\n", dfvard);

  dprintf("toc = %p\n", toc);
  dprintf("sp = %p\n", sp);

  eaExp[0] = toc; /* assuming that TOC entries are not reordered */

  for(i=1; i<44; ++i)
    eaExp[i] = (void*)((unsigned long)divarw + eaExpOffset[i]);

  for(i=44; i<50; ++i)
    eaExp[i] = (void*)((unsigned long)sp + eaExpOffset[i]);; /* SP */
  
  for(i=50; i<54; ++i)
    eaExp[i] = (void*)((unsigned long)divarw + eaExpOffset[i]);

  eaExp[54] = (void*)((unsigned long)toc + sizeof(void*)); /* TOC */

  for(i=55; i<59; ++i)
    eaExp[i] = (void*)((unsigned long)dfvars + eaExpOffset[i]);

  eaExp[59] = (void*)((unsigned long)toc + 2*sizeof(void*)); /* TOC */

  for(i=60; i<64; ++i)
    eaExp[i] = (void*)((unsigned long)dfvard + eaExpOffset[i]);

  for(i=64; i<68; ++i)
    eaExp[i] = (void*)((unsigned long)dfvars + eaExpOffset[i]);

  for(i=68; i<72; ++i)
    eaExp[i] = (void*)((unsigned long)dfvard + eaExpOffset[i]);
  
  eaExp[72] = (void*)((unsigned long)dfvars + eaExpOffset[i]);


  /* Duplicate the stream for cc */
  for(i=0; i<accessExp; ++i) {
    eaExpCC[i] = eaExp[i];
    bcExpCC[i] = bcExp[i];
  }
}
#endif /* defined(rs6000_ibm_aix4_1_test) */

#if defined(i386_unknown_linux2_0_test) || (defined(os_freebsd_test) && defined(arch_x86_test))
unsigned int loadExp=67;

#if defined(i386_unknown_linux2_0_test)
unsigned int storeExp=27;
unsigned int accessExp=94;
unsigned int accessExpCC=93;
#else
unsigned int storeExp=43;
unsigned int accessExp=110;
unsigned int accessExpCC=109;
#endif

unsigned int prefeExp=2;
#if defined(i386_unknown_linux2_0_test)
const struct reduction mmxRed = { 2, 1, 0, 3, 49 };
const struct reduction sseRed = { 2, 0, 1, 3, 53 };
const struct reduction sse2Red = { 2, 0, 0, 2, 57 };
const struct reduction amdRed = { 2, 0, 1, 3, 60 };

const struct reduction ccRed = { 0, 0, 0, 1, 87 };
#else
const struct reduction mmxRed = { 2, 5, 0, 7, 49 };
const struct reduction sseRed = { 2, 4, 1, 7, 57 };
const struct reduction sse2Red = { 2, 4, 0, 6, 65 };
const struct reduction amdRed = { 2, 4, 1, 7, 72 };

const struct reduction ccRed = { 0, 0, 0, 1, 103 };
#endif

#else
#if defined(i386_unknown_nt4_0_test)
unsigned int loadExp=67;
unsigned int storeExp=31;
unsigned int prefeExp=2;
unsigned int accessExp=98;
unsigned int accessExpCC=97;

const struct reduction mmxRed = { 2, 1, 0, 3, 50 };
const struct reduction sseRed = { 2, 1, 1, 4, 55 };
const struct reduction sse2Red = { 2, 1, 0, 3, 60 };
const struct reduction amdRed = { 2, 1, 1, 4, 64 };

const struct reduction ccRed = { 0, 0, 0, 1, 91 };
#endif
#endif

#if defined(i386_unknown_linux2_0_test) \
 || defined(i386_unknown_nt4_0_test) \
 || (defined(os_freebsd_test) && defined(arch_x86_test))


int eaExpOffset[] =    { /* 0-3 */ 0,0,0,0,
                         /* 4-10 */ 0,0,0,0,0,0,0,
                         /* 11-17 */ 4,8,4,8,4,8,4,
                         /* 18 */ 0,
                         /* 19-25 */ 0,4,8,12,0,4,8,
                         /* 26-34 */ 12,0,8,8,8,0,4,8,4,
                         /* 35 */ 0,
                         /* 36-47 */ 4,4,4,0,4,0,4,8,0,0,4,0,
                         /* 48 */ 0,
#if defined(i386_unknown_nt4_0_test) 
						0,
#elif defined(i386_unknown_freebsd7_0_test)
                                                0,0,0,0,
#endif
						 /* 49 */ 0,
                         /* 50-51 */ 8,0,
                         /* 52 */ 0,
#if defined(i386_unknown_nt4_0_test)
						0,
#elif defined(i386_unknown_freebsd7_0_test)
                                                0,0,0,0,
#endif
                         /* 53-55 */ 0,0,0,
                         /* 56 */ 0,
#if defined(i386_unknown_nt4_0_test)
						0,
#elif defined(i386_unknown_freebsd7_0_test)
                                                0,0,0,0,
#endif
                         /* 57-58 */ 0,0,
                         /* 59 */ 0,
#if defined(i386_unknown_nt4_0_test)
						0,
#endif
#if defined(i386_unknown_freebsd7_0_test)
                                                0,0,0,0,
#endif
                         /* 60-62 */ 0,8,0,
                         /* 63-69 */ 0,12,0,0,0,44,25,
                         /* 70-75 */ 0,0,0,0,4,8,
                         /* 76-81 */ 0,0,0,2,4,8,
                         /* 82-83 */ 0,0,
                         /* 84-85 */ 0,0,
                         /* 86-88 */ 0,4,8,
                         /* 89-90 */ 0,0 };

unsigned int bcExp[] = { 4,4,4,4,  4,4,4,4,4,4,4,  4,4,4,4,4,4,4,  4,
                         4,4,4,4,4,4,4,   4,4,4,4,4,4,4,4,4,   4,  4,4,1,1,4,4,4,4,4,1,4,4, 4,
#if defined(i386_unknown_nt4_0_test) 
					4,
#elif defined(i386_unknown_freebsd7_0_test)
                                  4,4,4,4,
#endif
                         4,8,8, 4, 
#if defined(i386_unknown_nt4_0_test)
					4,
#elif defined(i386_unknown_freebsd7_0_test)
                                  4,4,4,4,
#endif
						 
						 16,4,0, 4, 
#if defined(i386_unknown_nt4_0_test)
					4,
#elif defined(i386_unknown_freebsd7_0_test)
                                  4,4,4,4,
#endif
						 16,8, 4, 
#if defined(i386_unknown_nt4_0_test)
					4,
#elif defined(i386_unknown_freebsd7_0_test)
                                  4,4,4,4,
#endif
						 8,8,0,  12,4,16,16,49,4,4,  4,8,10,2,4,8,
                         4,8,10,2,4,8, 2,2,  28,28,  4,4,4,  4,4,4, 4,4, 4,4,4 };

void reduce(const struct reduction x)
{
  unsigned int i;

  loadExp  -= x.loadRed;
  storeExp -= x.storeRed;
  prefeExp -= x.prefeRed;

  for(i=x.axsShift; i<accessExp; ++i)
    eaExp[i] = eaExp[i+x.axsRed];

  for(i=x.axsShift; i<accessExp; ++i)
    bcExp[i] = bcExp[i+x.axsRed];

  for(i=x.axsShift; i<accessExpCC; ++i)
    eaExpCC[i] = eaExpCC[i+x.axsRed];

  for(i=x.axsShift; i<accessExpCC; ++i)
    bcExpCC[i] = bcExpCC[i+x.axsRed];

  accessExp -= x.axsRed;
  accessExpCC -= x.axsRed;
}

void reduceCC(const struct reduction x)
{
  unsigned int i;

  for(i=x.axsShift; i<accessExpCC; ++i)
    eaExpCC[i] = eaExpCC[i+x.axsRed];

  for(i=x.axsShift; i<accessExpCC; ++i)
    bcExpCC[i] = bcExpCC[i+x.axsRed];

  accessExpCC -= x.axsRed;
}


void init_test_data()
{
  int caps, i;

  dprintf("divarw = %p\n", divarw);
  dprintf("dfvars = %p\n", dfvars);
  dprintf("dfvard = %p\n", dfvard);
  dprintf("&dfvart = %p\n", &dfvart);
  dprintf("&dlarge = %p\n", &dlarge);

#if defined(i386_unknown_nt4_0_test)
  for(i=4; i<15; ++i)
    eaExp[i] = (void*)((unsigned long)divarw + eaExpOffset[i]); /* skip ebp for now */
  for(i=16; i<18; ++i)
    eaExp[i] = (void*)((unsigned long)divarw + eaExpOffset[i]);
  for(i=19; i<26; ++i)
    eaExp[i] = (void*)((unsigned long)divarw + eaExpOffset[i]);
  i=26;
  eaExp[i] = (void*)((unsigned long)divarw + eaExpOffset[i]);
  for(i=28; i<35; ++i)
    eaExp[i] = (void*)((unsigned long)divarw + eaExpOffset[i]);
  for(i=36; i<48; ++i)
    eaExp[i] = (void*)((unsigned long)divarw + eaExpOffset[i]);
  /* skip call @ i=48 (access 49)*/
  /* skip call @ i=49 (access 50)*/
  for(i=50; i<53; ++i)
    eaExp[i] = (void*)((unsigned long)divarw + eaExpOffset[i]);
  /* skip call @ i=53 (access 54) */
  /* skip call @ i=54 (access 55) */
  for(i=55; i<57; ++i)
    eaExp[i] = (void*)((unsigned long)dfvars + eaExpOffset[i]);
  i=57;
  eaExp[i] = (void*)((unsigned long)divarw + eaExpOffset[i]);
  /* skip call @ i=58 (access 59)*/
  /* skip call @ i=59 (access 60)*/
  for(i=60; i<62; ++i)
    eaExp[i] = (void*)((unsigned long)dfvard + eaExpOffset[i]);
  /* skip call @ i = 62 (access 63)*/
  /* skip call @ i = 63 (access 64)*/
  for(i=64; i<66; ++i)
    eaExp[i] = (void*)((unsigned long)dfvars + eaExpOffset[i]);
  i=66;
  eaExp[i] = (void*)((unsigned long)divarw + eaExpOffset[i]);
  for(i=67; i<70; ++i)
    eaExp[i] = (void*)((unsigned long)divarw + eaExpOffset[i]);
  i=70; /* 2nd of mov */
  eaExp[i] = (void*)((unsigned long)dfvars + eaExpOffset[i]);
  for(i=71; i<74; ++i) /* scas, cmps */
    eaExp[i] = (void*)((unsigned long)&dlarge + eaExpOffset[i]);
  i=74;
  eaExp[i] = (void*)((unsigned long)dfvars + eaExpOffset[i]);
  i=75;
  eaExp[i] = (void*)((unsigned long)dfvard + eaExpOffset[i]);
  i=76;
  eaExp[i] = (void*)((unsigned long)&dfvart + eaExpOffset[i]);
  for(i=77; i<80; ++i)
    eaExp[i] = (void*)((unsigned long)divarw + eaExpOffset[i]);
  i=80;
  eaExp[i] = (void*)((unsigned long)dfvars + eaExpOffset[i]);
  i=81;
  eaExp[i] = (void*)((unsigned long)dfvard + eaExpOffset[i]);
  i=82;
  eaExp[i] = (void*)((unsigned long)&dfvart + eaExpOffset[i]);
  for(i=83; i<88; ++i)
    eaExp[i] = (void*)((unsigned long)divarw + eaExpOffset[i]);
  for(i=88; i<90; ++i)
    eaExp[i] = (void*)((unsigned long)&dlarge + eaExpOffset[i]);
  for(i=90; i<93; ++i)
    eaExp[i] = (void*)((unsigned long)divarw + eaExpOffset[i]);
#elif defined(i386_unknown_freebsd7_0_test)
  for(i=4; i<15; ++i)
    eaExp[i] = (void*)((unsigned long)divarw + eaExpOffset[i]); /* skip ebp for now */
  for(i=16; i<18; ++i)
    eaExp[i] = (void*)((unsigned long)divarw + eaExpOffset[i]);
  for(i=19; i<26; ++i)
    eaExp[i] = (void*)((unsigned long)divarw + eaExpOffset[i]);
  i=26;
  eaExp[i] = (void*)((unsigned long)divarw + eaExpOffset[i]);
  for(i=28; i<35; ++i)
    eaExp[i] = (void*)((unsigned long)divarw + eaExpOffset[i]);
  for(i=36; i<48; ++i)
    eaExp[i] = (void*)((unsigned long)divarw + eaExpOffset[i]);
  /* skip call @ i=48 (access 49)*/
  /* skip saymsg @ i=49-52 (access 50-53) */
  for(i=53; i<56; ++i)
    eaExp[i] = (void*)((unsigned long)divarw + eaExpOffset[i]);
  /* skip call @ i=56 (access 57) */
  /* skip saymsg @ i=57-60 (access 58-61) */
  for(i=61; i<63; ++i)
    eaExp[i] = (void*)((unsigned long)dfvars + eaExpOffset[i]);
  i=63;
  eaExp[i] = (void*)((unsigned long)divarw + eaExpOffset[i]);
  /* skip call @ i=64 (access 65)*/
  /* skip saymsg @ i=65-68 (access 66-69)*/
  for(i=69; i<71; ++i)
    eaExp[i] = (void*)((unsigned long)dfvard + eaExpOffset[i]);
  /* skip call @ i = 71 (access 72) */
  /* skip saymsg @ i=72-75 (access 73-76) */
  for(i=76; i<78; ++i)
    eaExp[i] = (void*)((unsigned long)dfvars + eaExpOffset[i]);
  i=78;
  eaExp[i] = (void*)((unsigned long)divarw + eaExpOffset[i]);
  for(i=79; i<82; ++i)
    eaExp[i] = (void*)((unsigned long)divarw + eaExpOffset[i]);
  i=82; /* 2nd of mov */
  eaExp[i] = (void*)((unsigned long)dfvars + eaExpOffset[i]);
  for(i=83; i<86; ++i) /* scas, cmps */
    eaExp[i] = (void*)((unsigned long)&dlarge + eaExpOffset[i]);
  i=86;
  eaExp[i] = (void*)((unsigned long)dfvars + eaExpOffset[i]);
  i=87;
  eaExp[i] = (void*)((unsigned long)dfvard + eaExpOffset[i]);
  i=88;
  eaExp[i] = (void*)((unsigned long)&dfvart + eaExpOffset[i]);
  for(i=89; i<92; ++i)
    eaExp[i] = (void*)((unsigned long)divarw + eaExpOffset[i]);
  i=92;
  eaExp[i] = (void*)((unsigned long)dfvars + eaExpOffset[i]);
  i=93;
  eaExp[i] = (void*)((unsigned long)dfvard + eaExpOffset[i]);
  i=94;
  eaExp[i] = (void*)((unsigned long)&dfvart + eaExpOffset[i]);
  for(i=95; i<100; ++i)
    eaExp[i] = (void*)((unsigned long)divarw + eaExpOffset[i]);
  for(i=100; i<102; ++i)
    eaExp[i] = (void*)((unsigned long)&dlarge + eaExpOffset[i]);
  for(i=102; i<104; ++i)
    eaExp[i] = (void*)((unsigned long)divarw + eaExpOffset[i]);
#else
  for(i=4; i<15; ++i)
    eaExp[i] = (void*)((unsigned long)divarw + eaExpOffset[i]); /* skip ebp for now */
  for(i=16; i<18; ++i)
    eaExp[i] = (void*)((unsigned long)divarw + eaExpOffset[i]);
  for(i=19; i<26; ++i)
    eaExp[i] = (void*)((unsigned long)divarw + eaExpOffset[i]);
  i=26;
  eaExp[i] = (void*)((unsigned long)divarw + eaExpOffset[i]);
  for(i=28; i<35; ++i)
    eaExp[i] = (void*)((unsigned long)divarw + eaExpOffset[i]);
  for(i=36; i<48; ++i)
    eaExp[i] = (void*)((unsigned long)divarw + eaExpOffset[i]);
  /* skip call @ i=48 (access 49)*/
  for(i=49; i<52; ++i)
    eaExp[i] = (void*)((unsigned long)divarw + eaExpOffset[i]);
  /* skip call @ i=52 (access 53)*/
  for(i=53; i<55; ++i)
    eaExp[i] = (void*)((unsigned long)dfvars + eaExpOffset[i]);
  i=55;
  eaExp[i] = (void*)((unsigned long)divarw + eaExpOffset[i]);
  /* skip call @ i=56 (access 57)*/
  for(i=57; i<59; ++i)
    eaExp[i] = (void*)((unsigned long)dfvard + eaExpOffset[i]);
  /* skip call @ i = 59 (access 60)*/
  for(i=60; i<62; ++i)
    eaExp[i] = (void*)((unsigned long)dfvars + eaExpOffset[i]);
  i=62;
  eaExp[i] = (void*)((unsigned long)divarw + eaExpOffset[i]);
  for(i=63; i<66; ++i)
    eaExp[i] = (void*)((unsigned long)divarw + eaExpOffset[i]);
  i=66; /* 2nd of mov */
  eaExp[i] = (void*)((unsigned long)dfvars + eaExpOffset[i]);
  for(i=67; i<70; ++i) /* scas, cmps */
    eaExp[i] = (void*)((unsigned long)&dlarge + eaExpOffset[i]);
  i=70;
  eaExp[i] = (void*)((unsigned long)dfvars + eaExpOffset[i]);
  i=71;
  eaExp[i] = (void*)((unsigned long)dfvard + eaExpOffset[i]);
  i=72;
  eaExp[i] = (void*)((unsigned long)&dfvart + eaExpOffset[i]);
  for(i=73; i<76; ++i)
    eaExp[i] = (void*)((unsigned long)divarw + eaExpOffset[i]);
  i=76;
  eaExp[i] = (void*)((unsigned long)dfvars + eaExpOffset[i]);
  i=77;
  eaExp[i] = (void*)((unsigned long)dfvard + eaExpOffset[i]);
  i=78;
  eaExp[i] = (void*)((unsigned long)&dfvart + eaExpOffset[i]);
  for(i=79; i<84; ++i)
    eaExp[i] = (void*)((unsigned long)divarw + eaExpOffset[i]);
  for(i=84; i<86; ++i)
    eaExp[i] = (void*)((unsigned long)&dlarge + eaExpOffset[i]);
  for(i=86; i<89; ++i)
    eaExp[i] = (void*)((unsigned long)divarw + eaExpOffset[i]);
  /* Duplicate & reduce the stream for cc */
#endif
  for(i=0; i<accessExp; ++i) {
    eaExpCC[i] = eaExp[i];
    bcExpCC[i] = bcExp[i];
  }

  reduceCC(ccRed);

  /* Order of reductions matters! It must be right to left. */

  caps = amd_features();
  if(!(caps & CAP_3DNOW))
    reduce(amdRed);
  caps = ia32features();
  if(!(caps & CAP_SSE2))
    reduce(sse2Red);
  if(!(caps & CAP_SSE))
    reduce(sseRed);
  if(!(caps & CAP_MMX))
    reduce(mmxRed);

  /*fprintf(stderr, "eaExp at call 1 (access 49) = 0x%lx\n", eaExp[48]);
  fprintf(stderr, "eaExp at call 2 (access 53) = 0x%lx\n", eaExp[52]);
  fprintf(stderr, "eaExp at call 3 (access 56) = 0x%lx\n", eaExp[56]);
  fprintf(stderr, "eaExp at call 4 (access 60) = 0x%lx\n", eaExp[59]);*/

}
#endif /* defined(i386_unknown_linux2_0_test) || defined(i386_unknown_nt4_0_test) */

#if defined(x86_64_unknown_linux2_4_test) || defined(amd64_unknown_freebsd7_0_test)
unsigned int loadExp = 75;
unsigned int storeExp = 28;
unsigned int prefeExp = 2;
unsigned int accessExp = 103;
unsigned int accessExpCC = 102;

int eaExpOffset[] =    { 0,0,0,0,0,0,0,                             /* 7 initial stack pushes (EA not checked) */
			 0,0,0,0,0,0,0,0,0,0,0,0,0,                 /* 13 mod=0 loads */
			 4,8,-4,-8,4,8,-4,-8,4,8,-4,-8,127,-128,    /* 14 mod=1 loads */
			 12,0,8,8,8,0,4,8,4,                        /* 9 SIB tests (same as x86) */
			 4,4,4,0,4,0,4,8,0,4,0,0,                   /* 11 semantic tests (one has two accesses) */
                         0,                                         /* call to ia32_features */
			 0,8,0,                                     /* 3 MMX tests */
			 0,0,0,                                     /* 3 SSE tests */
                         0,                                         /* call to ia32_features */
                         0,0,                                       /* 2 SSE2 tests */
                         0,                                         /* call to amd_features */
                         0,8,0,                                     /* 3 3DNow! tests */
			 0,12,0,0,0,44,25,                          /* 5 REP tests (two have two accesses each) */
			 0,0,0,0,4,8,                               /* x87 */
			 0,0,0,2,4,8,
			 0,0,
			 0,0,
			 0,4,8,                                     /* conditional moves */
			 0,0,0,0,0,0,                               /* 6 final stack pops */
                         0,0                                        /* leave and return */
};

unsigned int bcExp[] = { 8,8,8,8,8,8,8,                  /* 7 initial stack pushes */
			 4,8,4,8,4,8,4,8,4,8,4,8,4,      /* 13 mod=0 loads */
			 4,8,4,8,4,8,4,8,4,8,4,8,4,8,    /* 14 mod=1 loads */
			 4,8,4,8,4,8,4,8,4,              /* 9 SIB tests */
			 4,4,1,1,4,4,4,4,4,4,4,4,        /* 11 semantic tests (one has two accesses) */
                         8,                              /* call to ia32_features */
                         8,8,8,                          /* 3 MMX tests */
			 16,4,0,                         /* 3 SSE tests */
                         8,                              /* call to ia32_features */
                         16,8,                           /* 2 SSE2 tests */
                         8,                              /* call to amd_features */
                         8,8,0,                          /* 3 3DNow! tests */
			 12,16,16,16,49,4,4,             /* 5 REP tests (two have two accesses each) */
			 4,8,10,2,4,8,                   /* x87 */
			 4,8,10,2,4,8,
			 2,2,
                         28,28,
			 4,4,4,                          /* conditional moves */
                         8,8,8,8,8,8,                    /* 6 final stack pops */
                         8,8                             /* leave and return */
};

int divarw[4];
float dfvars[4];
double dfvard[4];
long double dfvart;
char dlarge[512] = "keep the interface small and easy to understand.";

/* FIXME Remove calls to assert() from this function */
void init_test_data()
{
    int caps;
    int i;

  dprintf("divarw = %p\n", divarw);
  dprintf("dfvars = %p\n", dfvars);
  dprintf("dfvard = %p\n", dfvard);
  dprintf("&dfvart = %p\n", &dfvart);
  dprintf("&dlarge = %p\n", &dlarge);

  // we do not check the effective address for stack accesses,
  // since it depends on the stack pointer,
  // so we skip the initial 6 pushes
  i = 7;

  // ModRM and SIB loads and semantic tests (there are 54, but one has two accesses)
  for (; i < 55; i++)
      eaExp[i] = (void *)((unsigned long)divarw + eaExpOffset[i]);
  
  // the 12th is a load from [RIP + 1]
  eaExp[11] = rip_relative_load_address;

  // the 36th access uses RSP
  eaExp[35] = 0;

  // MMX
  assert(i == 55);
  i++; // skip the call
  for (; i < 59; i++)
      eaExp[i] = (void *)((unsigned long)divarw + eaExpOffset[i]);

  // SSE
  assert(i == 59);
  for (; i < 61; i++)
      eaExp[i] = (void *)((unsigned long)&dfvart + eaExpOffset[i]);
  assert(i == 61);
  eaExp[i] = (void *)((unsigned long)divarw + eaExpOffset[i]); i++; // the prefetch

  assert(i == 62);
  // SSE2
  i++; // skip the call
  for (; i < 65; i++)
      eaExp[i] = (void *)((unsigned long)&dfvart + eaExpOffset[i]);
  assert(i == 65);

  // 3DNow!
  i++; // skip the call        
  assert(i == 66);
  eaExp[i] = (void *)((unsigned long)dfvard + eaExpOffset[i]); i++;
  eaExp[i] = (void *)((unsigned long)dfvard + eaExpOffset[i]); i++;
  eaExp[i] = (void *)((unsigned long)divarw + eaExpOffset[i]); i++;

  // REP prefixes
  assert(i == 69);
  for (; i < 72; i++)
      eaExp[i] = (void *)((unsigned long)divarw + eaExpOffset[i]);
  assert(i == 72);
  eaExp[i] = (void *)((unsigned long)dfvars + eaExpOffset[i]); i++;
  for (; i < 76; i++)
      eaExp[i] = (void *)((unsigned long)&dlarge + eaExpOffset[i]);

  // x87
  assert(i == 76);
  eaExp[i] = (void *)((unsigned long)dfvars + eaExpOffset[i]); i++;
  eaExp[i] = (void *)((unsigned long)dfvard + eaExpOffset[i]); i++;
  eaExp[i] = (void *)((unsigned long)&dfvart + eaExpOffset[i]); i++;
  eaExp[i] = (void *)((unsigned long)divarw + eaExpOffset[i]); i++;
  eaExp[i] = (void *)((unsigned long)divarw + eaExpOffset[i]); i++;
  eaExp[i] = (void *)((unsigned long)divarw + eaExpOffset[i]); i++;

  eaExp[i] = (void *)((unsigned long)dfvars + eaExpOffset[i]); i++;
  eaExp[i] = (void *)((unsigned long)dfvard + eaExpOffset[i]); i++;
  eaExp[i] = (void *)((unsigned long)&dfvart + eaExpOffset[i]); i++;
  eaExp[i] = (void *)((unsigned long)divarw + eaExpOffset[i]); i++;
  eaExp[i] = (void *)((unsigned long)divarw + eaExpOffset[i]); i++;
  eaExp[i] = (void *)((unsigned long)divarw + eaExpOffset[i]); i++;

  eaExp[i] = (void *)((unsigned long)divarw + eaExpOffset[i]); i++;
  eaExp[i] = (void *)((unsigned long)divarw + eaExpOffset[i]); i++;

  eaExp[i] = (void *)((unsigned long)&dlarge + eaExpOffset[i]); i++;
   eaExp[i] = (void *)((unsigned long)&dlarge + eaExpOffset[i]); i++;

  // conditional moves
  assert(i == 92);
  for (; i < 95; i++)
      eaExp[i] = (void *)((unsigned long)divarw + eaExpOffset[i]);

  // duplicate stream for CC (except the second-to-last item)
  for(i=0; i<(int)accessExp; ++i) {
      eaExpCC[i] = eaExp[i];
      bcExpCC[i] = bcExp[i];
  }

  reduceCC(ccRed);
  caps = amd_features();
  if(!(caps & CAP_3DNOW)) {
      reduce(amdRed);
  }
  caps = ia32features();
  if(!(caps & CAP_SSE2))
      reduce(sse2Red);
  if(!(caps & CAP_SSE))
      reduce(sseRed);
  if(!(caps & CAP_MMX))
      reduce(mmxRed);

}

const struct reduction mmxRed = { 2, 1, 0, 3, 55 };
const struct reduction sseRed = { 2, 0, 1, 3, 59 };
const struct reduction sse2Red = { 2, 0, 0, 2, 63 };
const struct reduction amdRed = { 2, 0, 1, 3, 66 };
const struct reduction ccRed = { 0, 0, 0, 1, 93 };

void reduceCC(const struct reduction x)
{
    unsigned int i;

    for(i=x.axsShift; i<accessExpCC; ++i)
        eaExpCC[i] = eaExpCC[i+x.axsRed];

    for(i=x.axsShift; i<accessExpCC; ++i)
        bcExpCC[i] = bcExpCC[i+x.axsRed];

    accessExpCC -= x.axsRed;
}

void reduce(const struct reduction x)
{
    unsigned int i;

    loadExp  -= x.loadRed;
    storeExp -= x.storeRed;
    prefeExp -= x.prefeRed;

    for(i=x.axsShift; i<accessExp; ++i)
        eaExp[i] = eaExp[i+x.axsRed];

    for(i=x.axsShift; i<accessExp; ++i)
        bcExp[i] = bcExp[i+x.axsRed];

    for(i=x.axsShift; i<accessExpCC; ++i)
        eaExpCC[i] = eaExpCC[i+x.axsRed];

    for(i=x.axsShift; i<accessExpCC; ++i)
        bcExpCC[i] = bcExpCC[i+x.axsRed];

    accessExp -= x.axsRed;
    accessExpCC -= x.axsRed;
}

#endif /* defined(x86_64_unknown_linux2_4_test) */


#if defined(arch_power_test) && defined(os_linux_test)

long loadsnstores(long x, long y, long z)
{
  return x + y + z;
}

unsigned int bcExp[] = { 0 };

void init_test_data()
{
}
#endif /* power linux */

/* functions called by the simple instrumentation points */
void countLoad() {
  ++loadCnt;
}

void countStore()
{
  ++storeCnt;
}

void countPrefetch()
{
  ++prefeCnt;
}

void countAccess()
{
  ++accessCnt;
}

int validateEA(void* ea1[], void* ea2[], unsigned int n)
{
  int ok = 1;
  unsigned int i=0;
  int ret = 1;

  for(; i<n; ++i) {
    ok = (ok && ((ea1[i] == ea2[i]) || ea1[i] == NULL));
    if(!((ea1[i] == ea2[i]) || ea1[i] == NULL)) {
      logerror("EA Validation failed at access #%u. Expecting: %p. Got: %p.\n",
	      i+1, ea1[i], ea2[i]);
      ret = 0;
    }
  }
  return ret;
}

int validateBC(unsigned int bc1[], unsigned int bc2[], unsigned int n)
{
  int ok = 1;
  unsigned int i=0;
  int ret = 1;

  for(; i<n; ++i) {
    ok = (ok && (bc1[i] == bc2[i]));
    if(!bc1[i] == bc2[i]) {
      logerror("BC Validation failed at access #%d. Expecting: %d. Got: %d.\n",
	      i+1, bc1[i], bc2[i]);
      ret = 0;
    }
  }
  return ret;
}

/* functions called by the effective address/byte count instrumentation points */
void listEffAddr(const char* insn, void* addr)
{
  if(accessCntEA < accessExp)
    eaList[accessCntEA] = addr;
  else
    doomEA = 1;
  accessCntEA++;
  dprintf("EA[%d] (%s):%p\n", accessCntEA, insn, addr);
}

void listByteCnt(const char* insn, unsigned int count)
{
  if(accessCntBC < accessExp)
    bcList[accessCntBC] = count;
  else
    doomBC = 1;
  accessCntBC++;
  dprintf("BC[%d] (%s):%d\n", accessCntBC, insn, count);
}


void listEffAddrCC(const char* insn, void* addr)
{
  if(accessCntEAcc < accessExpCC)
    eaListCC[accessCntEAcc] = addr;
  else
    doomEAcc = 1;
  accessCntEAcc++;
  dprintf("?A[%d] (%s):%p\n", accessCntEAcc, insn, addr);
}

void listByteCntCC(const char* insn, unsigned int count)
{
  if(accessCntBCcc < accessExpCC)
    bcListCC[accessCntBCcc] = count;
  else
    doomBCcc = 1;
  accessCntBCcc++;
  dprintf("?C[%d] (%s):%d\n", accessCntBCcc, insn, count);
}
