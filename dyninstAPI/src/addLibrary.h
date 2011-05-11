/*
 * Copyright (c) 1996-2011 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
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

/* -*- Mode: C; indent-tabs-mode: true -*- */
// Since the author of this file chose to use tabs instead of spaces
// for the indentation mode, the above line switches users into tabs
// mode with emacs when editing this file.

/* $Id: addLibrary.h,v 1.8 2006/03/14 22:57:22 legendre Exp $ */

#if defined(sparc_sun_solaris2_4)

#include <unistd.h>
#include  <fcntl.h>
#include  <stdio.h>
#include  <libelf.h>
#include  <stdlib.h>
#include  <string.h>
#include <errno.h>
#include <elf.h>
#include <sys/link.h>

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


	int numberExtraSegs;
	unsigned int newPhdrAddr;
	unsigned int newPhdrOffset;
	int libnameLen;
	unsigned int phdrSize;
	int libnameIndx;
	unsigned int textSegEndIndx; 
	unsigned int dataSegStartIndx;

	
	void createNewElf();
	int findSection(const char *name);
	void updateDynamic(Elf_Data*newData,unsigned int hashOff, unsigned int dynsymOff, unsigned int dynstrOff);
	void updateProgramHeaders(Elf32_Phdr *phdr, unsigned int dynstrOffset);
	void addStr(Elf_Data* newData, Elf_Data* oldData, const char *str);
	int writeNewElf(char* filename, const char* libname);
	unsigned int findEndOfTextSegment();
	unsigned int findStartOfDataSegment();

	int findNewPhdrAddr();
	int findNewPhdrOffset();
	int checkFile();

	void fixUpPhdrForDynamic();
	void moveDynamic();
	void updateSymbols(Elf_Data* symtabData,Elf_Data* strData, unsigned int dynAddr);


	unsigned int _pageSize, realPageSize;	
	public:
	int driver(Elf *elf, char *newfilename, const char *libname);
	addLibrary();
	~addLibrary();

};


#endif

