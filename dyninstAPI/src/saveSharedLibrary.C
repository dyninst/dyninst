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

#include "saveSharedLibrary.h"
#include "util.h"

#if defined(sparc_sun_solaris2_4) \
 || defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */

void saveSharedLibrary::openElf(){
	elf_version(EV_CURRENT);
	if((oldfd = open(soObject.getpathname(), O_RDONLY)) == -1){
		bpfatal("cannot open Old SO: %s\n",soObject.getpathname());
		perror(" FAIL ");
		exit(1);
	}
	if((oldElf = elf_begin(oldfd, ELF_C_READ, NULL)) ==NULL){
		bpfatal("cannot open ELF %s \n", soObject.getpathname());
		exit(1);
	}

	if((newfd = (creat(newpathname, 0x1c0)))==-1){
		bpfatal("cannot open new SO : %s\n",newpathname);
		perror(" FAIL ");
		exit(1);
	}
	if((newElf = elf_begin(newfd, ELF_C_WRITE, NULL)) ==NULL){
		bpfatal("cannot open ELF %s \n", newpathname);
		exit(1);
	}
}

void saveSharedLibrary::copyElf(){ 

	Elf32_Shdr *newsh, *shdr;
	Elf_Scn *scn, *newScn; 
        Elf32_Ehdr *ehdr = elf32_getehdr(oldElf), *newEhdr;
	Elf_Data *strdata, *newdata, *olddata;

        if(!(newEhdr = elf32_newehdr(newElf))){
		bpfatal("newEhdr failed\n");
		exit(1);
        }


        if (((ehdr = elf32_getehdr(oldElf)) != NULL)){ 
             if((scn = elf_getscn(oldElf, ehdr->e_shstrndx)) != NULL){
       		      if((strdata = elf_getdata(scn, NULL)) == NULL){
				bpfatal(" Failed obtaining .shstrtab data buffer \n");
				exit(1);
			}
		}else{
			bpfatal(" FAILED obtaining .shstrtab scn\n");		
		}
	}else{
		bpfatal(" FAILED obtaining .shstrtab ehdr\n");
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
	close(newfd);
	
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


