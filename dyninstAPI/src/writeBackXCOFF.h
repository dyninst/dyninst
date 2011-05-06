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

/* 	this file contains the class writeBackXCOFF
	which is used to handle the editing of
	XCOFF files for save the world
*/

//typedef unsigned int Address;

#include "XCOFFlib.h"



class writeBackXCOFF {


	private:

	XCOFF newFile;
	XCOFF oldFile;

	scnhdr *newSections;
	int numberSections;
	int maxSections;
	char *textTramps;
	unsigned int textTrampsAddr;
	unsigned int textTrampsSize;

	process *mutateeProcess;

	bool debugFlag;	

	int loadFile(XCOFF* file);

	void parseXCOFF(XCOFF *file);

	struct scnhdr *addSectionHeader(char *newFileCurrent, char* name,
		unsigned int paddr, unsigned int vaddr, int sectionSize, int flags);

	void  copySectionData(int offSet);

	void copyRelocationData(int offSet);

	void copyLineNumberData(int offSet);

	void* copySymbolTableData(int offSet);

	void addNewSectionData(char *newFileCurrent, char* data,int sectionSize, struct scnhdr *newHdr);

//	void expandMoveTextSection(char* newFileStart, int start, int sectionsize, int newTextFilePtr);

	public:
	writeBackXCOFF(const char* oldFileName, const char* newFileName, bool &error, bool debugOutputFlag = false,int numbScns=10);
	~writeBackXCOFF();

	bool createXCOFF(); //does all the final accounting to prepare for output
	int addSection(char *name, unsigned int paddr, unsigned int vaddr, 
		int sectionSize, char*data, int flags = 0x0020); //adds a section

	bool outputXCOFF(); //actually does the write

	void registerProcess(process *p){ mutateeProcess = p;};		
	void attachToText(Address addr, unsigned int size, char* data);
	

};


