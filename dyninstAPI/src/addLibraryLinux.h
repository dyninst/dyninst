/* $Id: addLibraryLinux.h,v 1.1 2002/02/12 15:42:03 chadd Exp $ */

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

class addLibrary {

	private:

	int arraySize;
	Elf_element *newElfFileSec;
	Elf32_Phdr *newElfFilePhdr;
	Elf32_Ehdr *newElfFileEhdr;
	Elf *oldElf, *newElf;
	int oldFd, newFd;
	int strTab;
	int  dynstr, bss;
	Elf_Data *strTabData;

	void openOldElf(char *filename);
	void createNewElf();
	void copySection(int oldInx, int newInx);
	void shiftSections(int index, int distance);
	int findSection(char *name);
	void updateShLinks(int insertPt, int oldDynstr, int shiftSize);
	void updateDynamic(unsigned int dynstrAddr);
	void alignAddr(int index);
	void moveDynstr();
	int updateProgramHeaders(Elf32_Phdr *newFilePhdr, int sizeInc, int dynamicOffset, int dataOffset);
	void writeNewElf(char *filename, char *libname);
	int addStr(int indx, char *str);
	void addSO(int strInx);
	void addDynamicSharedLibrary(char *libname);
	void fix_end(Elf_Data *symData, Elf_Data *strData, int shiftSize);
	public:
	void driver(Elf *elf, char *newfilename, char *libname);
	addLibrary();
	~addLibrary();

};


#endif

