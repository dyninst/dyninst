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

/* $Id: addLibraryLinux.h,v 1.9 2005/03/21 16:59:21 chadd Exp $ */

#if defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <libelf.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


typedef struct _Elf_element{
	Elf32_Shdr *sec_hdr;
	Elf_Data *sec_data;
} Elf_element;

#define TEXTGAP 1
#define DATAGAP 2

class addLibrary {

	private:

	int arraySize;
	Elf_element *newElfFileSec;
	Elf32_Phdr *newElfFilePhdr;
	Elf32_Ehdr *newElfFileEhdr;
	Elf *oldElf, *newElf;
	int newFd;
	Elf_Data *strTabData;

	int gapFlag;
	unsigned int newPhdrAddr;
	unsigned int newPhdrOffset;
	int libnameLen;
	unsigned int phdrSize;
	int libnameIndx;
	unsigned int textSegEndIndx;
 	unsigned int dataSegStartIndx;

	unsigned int textSideGap;
	unsigned int dataSideGap;

	unsigned int newTextSegmentSize;
	unsigned int newNoteOffset;
	int findIsNoteBeforeDynstr();


	void createNewElf();
	int findSection(char *name);
	void updateDynamic(Elf_Data *newData,unsigned int dynstrOff,unsigned int dynsymOff, unsigned int hashOff,unsigned int stringSize);
	void updateProgramHeaders(Elf32_Phdr *phdr, unsigned int dynstrOffset);
	void addStr(Elf_Data* newData, Elf_Data* oldData, char *str);
	int writeNewElf(char* filename, char* libname);

	int findNewPhdrAddr();
	int findNewPhdrOffset();
	int checkFile();
	int moveNoteShiftFollowingSectionsUp(char *libname);

	unsigned int findSizeOfSegmentFromPHT(int type);
	unsigned int findSizeOfNoteSection();
	unsigned int findSizeOfDynamicSection();
	unsigned int findEndOfTextSegment();
 	unsigned int findStartOfDataSegment();
	void fixUpPhdrForDynamic();
 	void moveDynamic();
 	void updateSymbols(Elf_Data* symtabData,Elf_Data* strData, unsigned int dynAddr);
	void updateSymbolsMovedTextSectionUp(Elf_Data* symtabData,Elf_Data* strData,int oldTextIndex);
	unsigned int sizeOfNoteSection();
	void updateSymbolsSectionInfo(Elf_Data* symtabData,Elf_Data* strData);

	unsigned int _pageSize;	
	public:
	int driver(Elf *elf, char *newfilename, char *libname);
	addLibrary();
	~addLibrary();

};


#endif

