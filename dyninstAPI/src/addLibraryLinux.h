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

/* $Id: addLibraryLinux.h,v 1.11 2006/03/14 22:57:23 legendre Exp $ */

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

	unsigned int findSizeOfSegmentFromPHT(Elf32_Word type);
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

