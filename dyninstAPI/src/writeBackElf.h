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

/* -*- Mode: C; indent-tabs-mode: true -*- */
/* $Id: writeBackElf.h,v 1.18 2005/03/18 04:34:57 chadd Exp $ */

#ifndef writeBackElf__
#define writeBackElf__

#if defined(sparc_sun_solaris2_4) \
 || defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */ \
 || defined(ia64_unknown_linux2_4)

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libelf.h>
#if defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */ \
 || defined(ia64_unknown_linux2_4)

#include "linux.h"
#endif
#include "common/h/Vector.h"
#include "ELF_Section.h"
#include <unistd.h>
#include "imageUpdate.h"

class process;

class writeBackElf{

private:
	
	Elf* newElf;
	Elf* oldElf;

	int oldfd, newfd;
	//important data sections in the
	//new Elf that need updated
	Elf_Data *textData;
	Elf_Data *symStrData;
	Elf_Data *dynStrData;
        Elf_Data *symTabData;
	Elf_Data *hashData;
	Elf_Data *dynsymData;
	Elf_Data *rodata;
	Elf_Data *dataData;

	Elf32_Shdr *textSh;
	Elf32_Shdr *rodataSh;
	Elf32_Ehdr *newEhdr;
	Elf32_Phdr *newPhdr;

	//when this is added to dyninst make this a
	//vector or something nice like that
	ELF_Section *newSections;
	unsigned int newSectionsSize;

	unsigned int lastAddr; //last used address in the old Elf
	unsigned int shiftSize; //shift caused by adding program headers, a multiple of 0x20
	unsigned int dataStartAddress; 
	unsigned int insertPoint; //index in newElf of first new section 

	unsigned int startAddr; //start address of .text in newElf
	unsigned int endAddr; //end address of .text in newElf 
	unsigned int rodataAddr; //start address of .rodata in newElf 
	unsigned int rodataSize; //size of .rodata in newElf
	unsigned int newHeapAddr; //location of the heap in the newElf

	unsigned int oldLastPage;//location of the last page in memory allocated in the oldElf
	int DEBUG_MSG;
	int pageSize ; // page size on this system
	process* mutateeProcess;
	int mutateeTextSize;
	unsigned int mutateeTextAddr;


	void updateSymbols(Elf_Data* symtabData,Elf_Data* strData);
	void updateDynamic(Elf_Data* dynamicData);
	void driver(); // main processing loop of outputElf()
	void createSections();
	void addSectionNames(Elf_Data* newdata, Elf_Data *olddata);
	void fixPhdrs(Elf32_Phdr*);
	void parseOldElf();
public:

	writeBackElf(const char* oldElfName, const char* newElfName,
					 int debugOutputFlag=0);
	~writeBackElf();

	void registerProcess(process *proc);
	
	int addSection(unsigned int addr, void *data, unsigned int dataSize,
						const char* name, bool loadable=true);

	bool createElf();
	void compactSections(pdvector<imageUpdate*> imagePatches,
								pdvector<imageUpdate*> &newPatches); 
	void compactLoadableSections(pdvector<imageUpdate*> imagePatches,
										  pdvector<imageUpdate*> &newPatches);
	void alignHighMem(pdvector<imageUpdate*> imagesPatches);
	Elf* getElf(){ return newElf; };
};

#endif
#endif
