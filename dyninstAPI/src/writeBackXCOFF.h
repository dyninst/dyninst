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

/* 	this file contains the class writeBackXCOFF
	which is used to handle the editing of
	XCOFF files for save the world
*/

//typedef unsigned int Address;

#include "XCOFFlib.h"
#include "process.h"



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
	writeBackXCOFF(char* oldFileName, char* newFileName, bool &error, bool debugOutputFlag = false,int numbScns=10);
	~writeBackXCOFF();

	bool createXCOFF(); //does all the final accounting to prepare for output
	int addSection(char *name, unsigned int paddr, unsigned int vaddr, 
		int sectionSize, char*data, int flags = 0x0020); //adds a section

	bool outputXCOFF(); //actually does the write
	bool setHeapAddr(unsigned int heapAddr); //sets the address of _end

	void registerProcess(process *p){ mutateeProcess = p;};		
	void attachToText(Address addr, unsigned int size, char* data);
	

};


