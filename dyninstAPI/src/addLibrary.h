/* $Id: addLibrary.h,v 1.2 2002/03/22 21:55:17 chadd Exp $ */

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
	int gapFlag;
	unsigned int newPhdrAddr;
	unsigned int newPhdrOffset;
	int libnameLen;
	unsigned int phdrSize;
	int libnameIndx;
	
	void createNewElf();
	int findSection(char *name);
	void updateDynamic(Elf_Data*newData,unsigned int hashOff, unsigned int dynsymOff, unsigned int dynstrOff);
	void updateProgramHeaders(Elf32_Phdr *phdr, unsigned int dynstrOffset, Elf32_Shdr**newSegs);
	void addStr(Elf_Data* newData, Elf_Data* oldData, char *str);
	int writeNewElf(char* filename, char* libname);

	int findNewPhdrAddr();
	int findNewPhdrOffset();
	int checkFile();
	unsigned int _pageSize, realPageSize;	
	public:
	int driver(Elf *elf, char *newfilename, char *libname);
	addLibrary();
	~addLibrary();

};


#endif

