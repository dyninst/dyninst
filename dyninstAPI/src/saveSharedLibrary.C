#include "saveSharedLibrary.h"

#if defined(BPATCH_LIBRARY)
#if defined(sparc_sun_solaris2_4) || defined(i386_unknown_linux2_0)

void saveSharedLibrary::openElf(){
	elf_version(EV_CURRENT);
	if((oldfd = open(soObject.getpathname(), O_RDONLY)) == -1){
		printf("cannot open Old SO: %s\n",soObject.getpathname());
		perror(" FAIL ");
		exit(1);
	}
	if((oldElf = elf_begin(oldfd, ELF_C_READ, NULL)) ==NULL){
		printf("cannot open ELF %s \n", soObject.getpathname());
		exit(1);
	}

	if((newfd = open(newpathname, O_WRONLY|O_CREAT|O_TRUNC)) == -1){
		printf("cannot open new SO : %s\n",newpathname);
		perror(" FAIL ");
		exit(1);
	}
	if((newElf = elf_begin(newfd, ELF_C_WRITE, NULL)) ==NULL){
		printf("cannot open ELF %s \n", newpathname);
		exit(1);
	}
}

void saveSharedLibrary::copyElf(){ 

	Elf32_Shdr *newsh, *shdr;
	Elf_Scn *scn, *newScn; 
        Elf32_Ehdr *ehdr = elf32_getehdr(oldElf), *newEhdr;
	Elf_Data *strdata, *newdata, *olddata;

        if(!(newEhdr = elf32_newehdr(newElf))){
		printf("newEhdr failed\n");
		exit(1);
        }


        if (((ehdr = elf32_getehdr(oldElf)) != NULL)){ 
             if((scn = elf_getscn(oldElf, ehdr->e_shstrndx)) != NULL){
       		      if((strdata = elf_getdata(scn, NULL)) == NULL){
				printf(" Failed obtaining .shstrtab data buffer \n");
				exit(1);
			}
		}else{
			printf(" FAILED obtaining .shstrtab scn\n");		
		}
	}else{
		printf(" FAILED obtaining .shstrtab ehdr\n");
	}
	memcpy(newEhdr, ehdr, sizeof(Elf32_Ehdr));

	unsigned int newScnName =0;
	scn = NULL;
	Elf_Data shstrtabData;
	for (int cnt = 1; scn = elf_nextscn(oldElf, scn); cnt++) {
		//copy sections from oldElf to newElf.
	
		shdr = elf32_getshdr(scn);
               	newScn = elf_newscn(newElf);
                newsh = elf32_getshdr(newScn);
                newdata = elf_newdata(newScn);
                olddata = elf_getdata(scn,NULL);
                memcpy(newsh, shdr, sizeof(Elf32_Shdr));
                memcpy(newdata,olddata, sizeof(Elf_Data));

               	//copy data buffer from oldElf 
		if(olddata->d_buf){
			newdata->d_buf = new char[olddata->d_size];
		        memcpy(newdata->d_buf, olddata->d_buf, olddata->d_size);
                }


                if(!strcmp( (char *)strdata->d_buf + shdr->sh_name, ".text")){
			textAddr = shdr->sh_addr;
			textSize = newdata->d_size;
			textData = newdata;
			memset(newdata->d_buf, '\0', newdata->d_size);
		}

		if(!strcmp( (char*) strdata->d_buf + shdr->sh_name, ".shstrtab")){
			const char *secname =".dyninst_mutated\0";
			shstrtabData.d_size = olddata->d_size+strlen(secname)+1;
			shstrtabData.d_buf = new  char[shstrtabData.d_size];
			memcpy(	shstrtabData.d_buf,  olddata->d_buf, olddata->d_size); 
			memcpy(&(((char*) shstrtabData.d_buf)[olddata->d_size]), secname,
				strlen(secname)+1); 	

			newScnName = olddata->d_size;
			delete [] (char*) newdata->d_buf;

			newdata->d_buf = shstrtabData.d_buf;
			newdata->d_size = shstrtabData.d_size;
	
			newsh->sh_size +=strlen(secname)+1; 
		} 
       	}
	newScn = elf_newscn(newElf);

	newsh = elf32_getshdr(newScn);
	newsh->sh_name = newScnName;
	newsh->sh_addr = 0x0;
	newsh->sh_type = 7;

	newEhdr -> e_shnum++; 
        Elf32_Phdr *tmp,*newPhdr;

        tmp = elf32_getphdr(oldElf);
        newPhdr=elf32_newphdr(newElf,newEhdr->e_phnum);

        memcpy(newPhdr, tmp, (ehdr->e_phnum) * ehdr->e_phentsize);

}

void saveSharedLibrary::closeElf(){

        elf_update(newElf, ELF_C_WRITE);
        elf_end(newElf);
	
}

void saveSharedLibrary::writeLibrary(char* newname){

	if(newname){
		setnewname(newname);
	}
	if(!soObject.getpathname()){
		return;
	}

	
	openElf();
	copyElf();
}

void saveSharedLibrary::closeLibrary(){
	closeElf();
}

void saveSharedLibrary::saveMutations(char *textInsn){


	memcpy(textData->d_buf, textInsn, textData->d_size);	
}

#endif
#endif

