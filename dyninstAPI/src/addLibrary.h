/* $Id: addLibrary.h,v 1.1 2001/12/11 20:22:22 chadd Exp $ */


#if defined(BPATCH_LIBRARY) && defined(sparc_sun_solaris2_4)

#ifndef addlibrary_h_
#define addlibrary_h_


#include <unistd.h>
#include  <fcntl.h>
#include  <stdio.h>
#include  <libelf.h>
#include  <stdlib.h>
#include  <string.h>
#include <errno.h>
#include "ELF_Section.h"


typedef struct _relocationData{

        unsigned int olddynstrIndx;
        unsigned int olddynsymIndx;
        unsigned int olddynamicIndx;
        unsigned int newdynstrIndx;
        unsigned int newdynsymIndx;
        unsigned int newdynamicIndx;

        Elf32_Word olddynstrAddr;
        Elf32_Word olddynsymAddr;
        Elf32_Word olddynamicAddr;
        Elf32_Word newdynstrAddr;
        Elf32_Word newdynsymAddr;
        Elf32_Word newdynamicAddr;

} RelocationData;

typedef struct _node{
        char* name;
        Elf32_Addr newAddr;
        Elf32_Addr delta;
        _node *next;
} Node;

typedef struct {
        Elf32_Addr d_addr;
        Elf32_Addr d_size;
} addrReloc;

#define DT_NULL    0
#define DT_NEEDED 1
#define DT_STRTAB 5
#define DT_PLTREL 20
#define DT_PLTGOT 3
#define DT_HASH   4
#define DT_SYMTAB 6
#define DT_RELA   7
#define DT_INIT   12
#define DT_FINI   13
#define DT_REL    17
#define DT_VERDEF 0x6ffffffc/* Points to SUNW_verdef */
#define DT_VERDEFNUM 0x6ffffffd/* #def */
#define DT_VERNEED 0x6ffffffe/* Points to SUNW_verneed */
#define DT_VERNEEDNUM 0x6fffffff/* #need */
#define DT_JMPREL 23
#define DT_STRSZ  10
#define DT_CHECKSUM 0x6ffffdf8
#define DT_DEBUG  21


class addLibrary {



	private:
		Elf* newElf;
		Elf* oldElf;
		char* libraryName;

		addrReloc *relocations;
		int relocIndex;
		Elf_Data *dynamicData, *dynstrData, *dynsymData, *strData, *newStrData;		
	        RelocationData relocData;
       		int bssSize;
        	Elf32_Shdr* dynstrShdr, *dynsymShdr, *dynamicShdr;
        	unsigned int gotAddr, sizeChange ;
        	Elf_Data *textData,*dynamicDataNew,*symtabData,*symstrData, *oldsymtabData;
		Elf_Data *olddyn, symData,*GOTData,*hashData, *olddynsymData;
        	int dynstrOffset ;
		int debugFlag;
		Elf32_Ehdr *   ehdr,*newEhdr;
		void updateDynamic(Elf_Data* dynamicData, unsigned int dynstrAddr, unsigned int dynsymAddr);
		void remakeHash(Elf_Data* hashData, Elf_Data* dynsymData, Elf_Data* dynstrData);
		void updateRela(Elf_Data* newData,int shift, unsigned int shiftIndex);
		void updateGOT(Elf_Data* GOTData,unsigned int dynamicAddr);
		void updateSymbols(Elf_Data* symtabData, Elf_Data *oldsymData,
        		RelocationData relocData, int maxShndx);	
		void updateSh_link(Elf32_Word *sh_link,RelocationData relocData);

		void  updateSymTab(Elf_Data* symData, RelocationData relocData, int shift,
			int max, Elf_Data*symstrPtr);	
		void updateSectionHdrs(Elf * newElf);
		void updateProgramHdrs(Elf *newElf,int size, unsigned int gotOffset, Elf32_Word dynamicAddr,
			Elf32_Word dynamicOffset, int bssSize,int dynamicSize, int dynSize);
		bool writeOutNewElf();	
		void driver();
		void parseOldElf();
		void padData(Elf32_Shdr* newDataShdr, Elf32_Shdr* newDynamicShdr, Elf_Scn* newDynSec, 
			Elf_Data* newDataData);
		void verifyNewFile();
	public:

		addLibrary(char*oldElfName, char* libname, int debugOutputFlag=false);
		addLibrary(Elf *oldElfPtr, char* libname, int debugOutputFlag=false);
		~addLibrary();
		bool outputElf(char* filename=NULL);

		Elf* getNewElf();
};

#endif
#endif
