/*
 * Copyright (c) 1996-2011 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "saveSharedLibrary.h"
#include "util.h"

#if defined(sparc_sun_solaris2_4) \
 || defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */

saveSharedLibrary::saveSharedLibrary(sharedLibrary sharedLib, char* newname){
	soObject.setpathname(sharedLib.getpathname());
	soObject.setbase_addr(sharedLib.getbase_addr());
	if(newname){
		newpathname = new char[strlen(newname) +1];
		strcpy(newpathname, newname);	
	}else{
		newpathname = NULL;
	}	
	newShstrtabData_d_buf=NULL;
}

saveSharedLibrary::saveSharedLibrary(Address baseAddr, const char* oldname,
			char* newname){
	soObject.setbase_addr(baseAddr);
	soObject.setpathname(oldname);
	newpathname = new char[strlen(newname) +1];
	strcpy(newpathname, newname);	
	newShstrtabData_d_buf=NULL;
}

saveSharedLibrary::saveSharedLibrary(): newpathname(NULL),newShstrtabData_d_buf(NULL) {}

void saveSharedLibrary::getTextInfo(Address &textScnAddr, Address &textScnSize){
	textScnAddr = textAddr;
	textScnSize = textSize;
}

void saveSharedLibrary::setnewname(char *newname){
	if(newpathname){
		delete [] newpathname;
	}
	newpathname = new char[strlen(newname) +1];
	strcpy(newpathname, newname);	
}

bool saveSharedLibrary::openElf(){
	   elf_version(EV_CURRENT);
	   /*if((oldfd = open(soObject.getpathname(), O_RDONLY)) == -1){ 
			 fprintf(stderr,"cannot open Old SO: %s\n",soObject.getpathname());
			 perror(" FAIL ");
			 return false;;
	   }
	  if((oldElf = elf_begin(oldfd, ELF_C_READ, NULL)) ==NULL){
			 fprintf(stderr,"cannot open ELF %s \n", soObject.getpathname());
			 return false;;
	   }*/

	   if((newfd = (open(newpathname, O_RDWR)))==-1){
			 fprintf(stderr,"%s[%d]: cannot open new SO : %s\n",FILE__, __LINE__,newpathname);
			 perror(" FAIL ");
			 return false;;
	   }
	   if((newElf = elf_begin(newfd, ELF_C_RDWR, NULL)) ==NULL){
			 fprintf(stderr,"cannot open ELF %s \n", newpathname);
			 return false;;
	   }
	  return readNewLib();
}

saveSharedLibrary::~saveSharedLibrary(){
	if(newpathname){
		delete []newpathname;
	}
	if( newShstrtabData_d_buf ){
		delete [] newShstrtabData_d_buf;
	}
}

bool saveSharedLibrary::readNewLib(){
	Elf32_Shdr *newsh, *shdr;
	Elf_Scn *scn, *newScn;
	Elf32_Ehdr *ehdr;
	Elf32_Phdr *oldPhdr;
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

	unsigned int newScnName =0;
	scn = NULL;
	Elf_Data shstrtabData;
#if  defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) 
	/* save the PHDR from the library */
     oldPhdr = elf32_getphdr(newElf);
	Elf32_Phdr phdrBuffer[ehdr->e_phnum];
	/* 	copy it over to a buffer because, on linux, we will close newElf and reopen it.
		This closing of newElf should dealloc the data pointed to by oldPhdr
	*/
	memcpy(phdrBuffer, oldPhdr, ehdr->e_phnum * ehdr->e_phentsize);
#endif

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
				newShstrtabData_d_buf = new  char[shstrtabData.d_size];
			    shstrtabData.d_buf = newShstrtabData_d_buf;
			    memcpy( shstrtabData.d_buf,  olddata->d_buf, olddata->d_size);
			    memcpy(&(((char*) shstrtabData.d_buf)[olddata->d_size]), secname,
					  strlen(secname)+1);

			    newScnName = olddata->d_size;
			    olddata->d_buf = shstrtabData.d_buf;
			    olddata->d_size = shstrtabData.d_size;

			    shdr->sh_size +=strlen(secname)+1;

				/* 	if the section header table is past this section in the
					ELF file, calculate the new offset*/
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
	newsh->sh_type = SHT_NOBITS; // SHT_NOTE;
	newsh->sh_flags=0;
	newsh->sh_addr = 0x0;
	newsh->sh_offset = shdr->sh_offset;
	newsh->sh_size=0;
	newsh->sh_link=0;
	newsh->sh_info=0;
	newsh->sh_addralign = 0x1; //Values 0 and 1 mean the section has no alignment constraints.
	newsh->sh_entsize = 0;


	newdata = elf_newdata(newScn);
	newdata->d_size =0;
	newdata->d_buf=0;

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
		fprintf(stderr,"%s[%d]: cannot open new SO : %s\n",FILE__, __LINE__, newpathname);
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

	/* 	update the PHDR, well just make sure it is reset to 
		what was in the original library */
     Elf32_Phdr *newPhdr = elf32_getphdr(newElf);
	memcpy(newPhdr,phdrBuffer, ehdr->e_phnum * ehdr->e_phentsize);

	/* be extra sure, set the DIRTY flag */
	elf_flagphdr(newElf, ELF_C_SET,ELF_F_DIRTY);
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
	/*elf_end(oldElf);
	P_close(oldfd);*/
}


void saveSharedLibrary::openBothLibraries(){

	if(!soObject.getpathname()){
		return;
	}
	
	openElf();
}

void saveSharedLibrary::closeNewLibrary(){
	closeElf();
}

void saveSharedLibrary::saveMutations(char *textInsn){

	memcpy(textData->d_buf, textInsn, textData->d_size);	
	elf_flagdata(textData,ELF_C_SET,ELF_F_DIRTY);

}

/* get the text section from the ORIGINAL library */
/*char* saveSharedLibrary::getTextSection(){
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
}*/

#endif

// vim:ts=5:
