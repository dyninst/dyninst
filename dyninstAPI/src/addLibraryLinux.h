/* $Id: addLibraryLinux.h,v 1.2 2002/03/18 19:17:47 chadd Exp $ */

#if defined(BPATCH_LIBRARY) && defined(i386_unknown_linux2_0)

#include <unistd.h>
#include  <fcntl.h>
#include  <stdio.h>
#include  <libelf/libelf.h>
#include  <stdlib.h>
#include  <string.h>
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
	
	void createNewElf();
	int findSection(char *name);
	void updateDynamic(Elf_Data*newData);
	void updateProgramHeaders(Elf32_Phdr *phdr, unsigned int dynstrOffset);
	void addStr(Elf_Data* newData, Elf_Data* oldData, char *str);
	int writeNewElf(char* filename, char* libname);

	int findNewPhdrAddr();
	int findNewPhdrOffset();
	int checkFile();
	unsigned int _pageSize;	
	public:
	int driver(Elf *elf, char *newfilename, char *libname);
	addLibrary();
	~addLibrary();

};


#endif

