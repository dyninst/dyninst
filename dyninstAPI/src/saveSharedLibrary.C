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

bool saveSharedLibrary::openElf(){
	   elf_version(EV_CURRENT);
	   if((oldfd = open(soObject.getpathname(), O_RDONLY)) == -1){ 
			 fprintf(stderr,"cannot open Old SO: %s\n",soObject.getpathname());
			 perror(" FAIL ");
			 return false;;
	   }
	  if((oldElf = elf_begin(oldfd, ELF_C_READ, NULL)) ==NULL){
			 fprintf(stderr,"cannot open ELF %s \n", soObject.getpathname());
			 return false;;
	   }

	   if((newfd = (open(newpathname, O_RDWR)))==-1){
			 fprintf(stderr,"cannot open new SO : %s\n",newpathname);
			 perror(" FAIL ");
			 return false;;
	   }
	   if((newElf = elf_begin(newfd, ELF_C_RDWR, NULL)) ==NULL){
			 fprintf(stderr,"cannot open ELF %s \n", newpathname);
			 return false;;
	   }
	  return readNewLib();
}

bool saveSharedLibrary::readNewLib(){
	Elf32_Shdr *newsh, *shdr;
	Elf_Scn *scn, *newScn;
	Elf32_Ehdr *ehdr;

	Elf32_Ehdr  *newEhdr;
	Elf_Data  *strdata, *newdata, *olddata;

	if ((ehdr = elf32_getehdr(newElf)) == NULL){ 
		fprintf(stderr," FAILED obtaining ehdr readNewLib\n");
		return false;
	}


	if((scn = elf_getscn(newElf, ehdr->e_shstrndx)) != NULL){
	    	if((strdata = elf_getdata(scn, NULL)) == NULL){
		 	fprintf(stderr," Failed obtaining .shstrtab data buffer \n");
			return false;
		}
	}else{
		fprintf(stderr," FAILED obtaining .shstrtab scn\n");
	}

	Elf32_Phdr *tmp, *newphdr;


	unsigned int newScnName =0;
	scn = NULL;
	Elf_Data shstrtabData;
	for (int cnt = 1; (scn = elf_nextscn(newElf, scn)); cnt++) {
		 //copy sections from newElf to newElf.

		 shdr = elf32_getshdr(scn);
		 olddata = elf_getdata(scn,NULL);



		 if(!strcmp( (char *)strdata->d_buf + shdr->sh_name, ".text")){
			    textAddr = shdr->sh_addr;
				textData = olddata;
				textSize = shdr->sh_size;
		 }

		 if(!strcmp( (char*) strdata->d_buf + shdr->sh_name, ".shstrtab")){
			    const char *secname =".dyninst_mutated\0";
			    shstrtabData.d_size = olddata->d_size+strlen(secname)+1;
			    shstrtabData.d_buf = new  char[shstrtabData.d_size];
			    memcpy( shstrtabData.d_buf,  olddata->d_buf, olddata->d_size);
			    memcpy(&(((char*) shstrtabData.d_buf)[olddata->d_size]), secname,
					  strlen(secname)+1);

			    newScnName = olddata->d_size;
			    olddata->d_buf = shstrtabData.d_buf;
			    olddata->d_size = shstrtabData.d_size;

			    shdr->sh_size +=strlen(secname)+1;
				if(ehdr ->e_shoff > shdr->sh_offset){
					ehdr->e_shoff += strlen(secname)+1;
				}
				elf_flagscn(scn,ELF_C_SET,ELF_F_DIRTY);

		 }

	}



	ehdr-> e_shnum++;
	newScn = elf_newscn(newElf);

	newsh = elf32_getshdr(newScn);
	newsh->sh_name = newScnName;
	newsh->sh_addr = 0x0;
	newsh->sh_type = 7;
	newsh->sh_offset = shdr->sh_offset;
	newdata = elf_newdata(newScn);
	newdata->d_size =0;
	newdata->d_buf=0;

	Elf32_Phdr *phdr = elf32_getphdr(newElf);
	elf_update(newElf, ELF_C_NULL);

	/* 	elfutils on linux does not write data back to an ELF file you
		have opened correctly. Specifically, if you add a section the
		section's section header has space allocated for it in the file
		but no data is written to it. lovely, eh?

		to combat this, we reopen the file we just closed, and find the
		empty section header and fill it with data.
	*/

#if  defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) 

	elf_update(newElf, ELF_C_WRITE);
  	elf_end(newElf);
	P_close(newfd);

	if((newfd = (open(newpathname, O_RDWR)))==-1){
		fprintf(stderr,"cannot open new SO : %s\n",newpathname);
		perror(" FAIL ");
		return false;;
	}
	if((newElf = elf_begin(newfd, ELF_C_RDWR, NULL)) ==NULL){
		fprintf(stderr,"cannot open ELF %s \n", newpathname);
		return false;;
	}
	if ((ehdr = elf32_getehdr(newElf)) == NULL){ 
		fprintf(stderr," FAILED obtaining ehdr readNewLib\n");
		return false;
	}


	if((scn = elf_getscn(newElf, ehdr->e_shstrndx)) != NULL){
	    	if((strdata = elf_getdata(scn, NULL)) == NULL){
		 	fprintf(stderr," Failed obtaining .shstrtab data buffer \n");
			return false;
		}
	}else{
		fprintf(stderr," FAILED obtaining .shstrtab scn\n");
	}

	scn = NULL;
	bool foundText=false; 
	for (int cnt = 1; (scn = elf_nextscn(newElf, scn)); cnt++) {
		 //copy sections from newElf to newElf.

		 shdr = elf32_getshdr(scn);
		 olddata = elf_getdata(scn,NULL);


		if(!foundText && !strcmp( (char *)strdata->d_buf + shdr->sh_name, ".text")){
			textAddr = shdr->sh_addr;
			textData = olddata;
			textSize = shdr->sh_size;
			elf_flagscn(scn,ELF_C_SET,ELF_F_DIRTY);
			foundText = true;
		}	

	}

		/**UPDATE THE LAST SHDR **/
	memset(shdr,'\0', sizeof(Elf32_Shdr));	
	shdr->sh_name = newScnName;
	shdr->sh_addr = 0x0;
	shdr->sh_type = 7;

	phdr = elf32_getphdr(newElf);
	elf_flagscn(scn,ELF_C_SET,ELF_F_DIRTY);
	elf_update(newElf, ELF_C_NULL);
#endif
	return true;

}

void saveSharedLibrary::closeElf(){

	elf_update(newElf, ELF_C_NULL);
	elf_update(newElf, ELF_C_WRITE);
  	elf_end(newElf);
	P_close(newfd);
}

void saveSharedLibrary::closeOriginalLibrary(){
	elf_end(oldElf);
	P_close(oldfd);
}


void saveSharedLibrary::openBothLibraries(){

	if(!soObject.getpathname()){
		return;
	}
	
	openElf();
	//copyElf();
}

void saveSharedLibrary::closeNewLibrary(){
	closeElf();
}

void saveSharedLibrary::saveMutations(char *textInsn){

	memcpy(textData->d_buf, textInsn, textData->d_size);	
	elf_flagdata(textData,ELF_C_SET,ELF_F_DIRTY);

}

/* get the text section from the ORIGINAL library */
char* saveSharedLibrary::getTextSection(){
	Elf32_Shdr  *shdr;
	Elf_Scn *scn; 
	   Elf32_Ehdr *ehdr = elf32_getehdr(oldElf);
	Elf_Data *strdata,*olddata;

	char * textSection=NULL;

	if (((ehdr = elf32_getehdr(oldElf)) != NULL)){ 
		if((scn = elf_getscn(oldElf, ehdr->e_shstrndx)) != NULL){
			if((strdata = elf_getdata(scn, NULL)) == NULL){
				fprintf(stderr," Failed obtaining .shstrtab data buffer \n");
				return NULL;
			}
		}else{
			fprintf(stderr," FAILED obtaining .shstrtab scn\n");		
			return NULL;
		}
	}else{
		fprintf(stderr," FAILED obtaining .shstrtab ehdr\n");
		return NULL;
	}

	scn = NULL;
	for (int cnt = 1; (scn = elf_nextscn(oldElf, scn)); cnt++) {
		//copy sections from oldElf to newElf.
	
		shdr = elf32_getshdr(scn);
		olddata = elf_getdata(scn,NULL);

		if(!strcmp( (char *)strdata->d_buf + shdr->sh_name, ".text")){

			textSection = new char[olddata->d_size];
			memcpy(textSection, olddata->d_buf, olddata->d_size);
		}
	}

	return textSection;
}

#endif

// vim:ts=5:
