/* $Id $ */
#include <editSharedLibrary.h>
#include "dyninstAPI/src/util.h"


/*	This function will open the shared library and remove the
	padded out bss section.  The PHT will be updated, the data segment
	filesz will be corrected.  The Ehdr section header index will also be
	corrected
*/
bool editSharedLibrary::removeBSSfromSharedLibrary(char *libName){

	elf_version(EV_CURRENT);
	if((fd = open(libName, O_RDWR)) == -1){
		bpfatal("cannot open Old SO: %s\n",libName);
		perror(" FAIL ");
		return false;
	}
	if((oldElf = elf_begin(fd, ELF_C_RDWR, NULL)) ==NULL){
		bpfatal("cannot open ELF %s \n", libName);
		return false;
	}

	bool res = removeBSS();

	elf_update(oldElf, ELF_C_WRITE);
	elf_end(oldElf);
	close(fd);

	return res;
}

bool editSharedLibrary::removeBSS(){

	int bssSize;
	bool bssFound=false;
	int offsetMovement;

	Elf32_Ehdr *ehdr = elf32_getehdr(oldElf);
	Elf32_Phdr *phdr = elf32_getphdr(oldElf);

	Elf32_Shdr *shdr;
	Elf_Scn *scn; 
	Elf_Data *strdata, *olddata;

     if (((ehdr = elf32_getehdr(oldElf)) != NULL)){ 

     	if((scn = elf_getscn(oldElf, ehdr->e_shstrndx)) != NULL){

       		if((strdata = elf_getdata(scn, NULL)) == NULL){
				bperr(" Failed obtaining .shstrtab data buffer \n");
				return false;
			}
		}else{
			bperr(" FAILED obtaining .shstrtab scn\n");		
			return false;
		}
	}else{
		bperr(" FAILED obtaining .shstrtab ehdr\n");
		return false;
	}

	scn = NULL;

	for (int cnt = 1; (scn = elf_nextscn(oldElf, scn)); cnt++) {
	
		shdr = elf32_getshdr(scn);
		olddata = elf_getdata(scn,NULL);
	
		if( bssFound ){
			shdr->sh_offset -= offsetMovement;	

			while( (shdr->sh_offset % shdr->sh_addralign) != 0){
				shdr->sh_offset ++;
				/* offsetMovement needs decreased everytime we move a section down b/c of alignment */
				offsetMovement --;
			}
		}

		if(!strcmp( (char*) strdata->d_buf + shdr->sh_name, ".bss")){
			bssFound =true;
			bssSize = shdr->sh_size;
			/* we will shift every succeeding section up by offsetMovement */
			offsetMovement = bssSize;

			//delete [] (char*)olddata->d_buf;

			/* this should get deallocated by elf when the file is closed */
			/* insure gives errors if i try to delete it */
			olddata->d_buf = NULL;


			shdr->sh_type = SHT_NOBITS;
       	}
	}

	/* 	the section header table has moved up by offsetMovement
		this takes into account the size of the bss and the shifts
		we needed to perform for alignment purposes
	*/
	ehdr->e_shoff -= offsetMovement;	

	/* the data segment will be the second loadable section*/
	bool foundFirstLoadable=false;
	for(int i=0;i<ehdr->e_phnum;i++){
		if(!foundFirstLoadable && phdr[i].p_type == PT_LOAD){
			foundFirstLoadable=true;
		}else if(foundFirstLoadable == true && phdr[i].p_type == PT_LOAD ){
			
			phdr[i].p_filesz -= bssSize;
			i=ehdr->e_phnum;
		}
	}
	if( !foundFirstLoadable ){
		bperr(" FAILED could not find the data segment in the shared library\n");
		return false;
	}
	return true;
}

// vim:ts=5:
