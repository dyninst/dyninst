/* -*- Mode: C; indent-tabs-mode: true -*- */
// Since the author of this file chose to use tabs instead of spaces
// for the indentation mode, the above line switches users into tabs
// mode with emacs when editing this file.

/* $Id: addLibrary.h,v 1.5 2003/07/01 19:57:20 chadd Exp $ */

#if defined(BPATCH_LIBRARY) && defined(sparc_sun_solaris2_4)

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

