/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

/* $Id: ELF_Section.h,v 1.5 2004/03/23 19:10:34 eli Exp $ */

/* ccw 21 nov 2001 */

#if defined(sparc_sun_solaris2_4) || defined(i386_unknown_linux2_0) || defined(ia64_unknown_linux2_4)

#ifndef ELF_Section__
#define ELF_Section__
typedef struct {
	unsigned int vaddr;
	void *data;
	unsigned int dataSize;
	Elf32_Shdr *shdr;
	char *name;
	int nameIndx;
	unsigned int align;
	unsigned int flags;
	unsigned int type;
	bool loadable;
} ELF_Section;
//make this a class w/desctuctor


typedef struct {
      Elf32_Sword d_tag;
      union {
          Elf32_Sword d_val;
          Elf32_Addr d_ptr;
      } d_un;
  } __Elf32_Dyn;


#define DT_NULL   0
#define DT_NEEDED 1
#define DT_STRTAB 5
#define DT_PLTREL 20
#define DT_PLTGOT 3
#define DT_HASH 4
#define DT_SYMTAB 6
#define DT_RELA 7
#define DT_INIT 12
#define DT_FINI 13
#define DT_REL 17
#define DT_VERDEF 0x6ffffffc
#define DT_VERDEFNUM 0x6ffffffd
#define DT_VERNEED 0x6ffffffe
#define DT_VERNEEDNUM 0x6fffffff
#define DT_JMPREL 23
#define DT_STRSZ 10
#define DT_CHECKSUM 0x6ffffdf8
#define DT_DEBUG 21

#endif
#endif
