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

/* -*- Mode: C; indent-tabs-mode: true -*- */
/* $Id: writeBackElf.C,v 1.28 2005/11/21 17:16:14 jaw Exp $ */

#if defined(sparc_sun_solaris2_4) \
 || defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */

#include "dyninstAPI/src/process.h"
#include "writeBackElf.h"

#define MALLOC 0 
#define DYNAMIC 1 

//unsigned int elf_version(unsigned int);

// This constructor opens both the old and new
// ELF files 
writeBackElf::writeBackElf(const char *oldElfName, const char* newElfName, 
									int debugOutputFlag) {

	if((oldfd = open(oldElfName, O_RDONLY)) == -1){
		bperr(" OLDELF_OPEN_FAIL %s",oldElfName);
		return;
	}
	if((newfd = (creat(newElfName, 0x1c0)))==-1){
		//bperr("NEWELF_OPEN_FAIL %s", newElfName);
		char *fileName = new char[strlen(newElfName)+1+3];
		for(int i=0;newfd == -1 && i<100;i++){
			sprintf(fileName, "%s%d",newElfName,i);
			newfd = (open(fileName, O_WRONLY|O_CREAT));
		}
		fflush(stdout);
		if(newfd == -1){
			bperr("NEWELF_OPEN_FAIL %s. clean up /tmp/dyninstMutatee*\n",
					 newElfName);
			return; 
		}
		delete [] fileName;
	}

	if(elf_version(EV_CURRENT) == EV_NONE){

		bpfatal(" elf_version failed!\n");
	} 
	if ((oldElf = elf_begin(oldfd, ELF_C_READ, NULL)) == NULL){
		bperr("OLDELF_BEGIN_FAIL");
                fflush(stdout);
		return;
	}
	if((newElf = elf_begin(newfd, ELF_C_WRITE, NULL)) == NULL){
		elf_end(oldElf);
		bperr("NEWELF_BEGIN_FAIL");
		fflush(stdout);
		return;
	}
	//elf_flagelf(newElf, ELF_C_SET, ELF_F_LAYOUT);
	newSections = NULL;
	newSectionsSize = 0;
	DEBUG_MSG = debugOutputFlag;
	pageSize = getpagesize();
	if(!DYNAMIC){
		if(MALLOC){
			newSections = (ELF_Section*) malloc(sizeof(ELF_Section) * 10);
		}else{
			newSections = new ELF_Section[10];
		}
	}	
	fflush(stdout);
	mutateeProcess = NULL;
	mutateeTextSize = 0;
	mutateeTextAddr = 0;
	elf_fill(0);
	newHeapAddr = 0;
	parseOldElf();
}


writeBackElf::~writeBackElf(){

	if(newSections){
		if(MALLOC){
			for(unsigned int i = 0;i<newSectionsSize;i++){
				free(newSections[i].data);
			}
			free(newSections);
		}else{
			for(unsigned int i = 0;i<newSectionsSize;i++){
				delete [] (char*) newSections[i].data;
				delete [] (char*) newSections[i].name; 
			}
			delete [] newSections;
		}
	}
	//elf_end(newElf); //closed in addLibrary::driver()
	P_close(newfd);

	P_close(oldfd);
	elf_end(oldElf);
}


int writeBackElf::addSection(unsigned int addr, void *data, unsigned int dataSize, const char* name, bool loadable) {
	ELF_Section *tmp;
	ELF_Section *newSection;
	if(DYNAMIC){

		if(MALLOC){
			tmp= (ELF_Section*) malloc(sizeof(ELF_Section) * (newSectionsSize+1));
		}else{
			tmp = new ELF_Section[newSectionsSize+1];
		}
		if(newSections){
			memcpy(tmp, newSections, sizeof(ELF_Section) * (newSectionsSize));
			if(MALLOC){
				free(newSections);
			}else{
				delete [] newSections;
			}
		}
		newSections = tmp;
	}	
	newSection = &newSections[newSectionsSize];
	newSection->vaddr = addr;
	newSection->loadable = loadable;
	if(MALLOC){
		newSection->data =  malloc(dataSize);
	}else{
		newSection->data = new char[dataSize];
	} 
	memcpy(newSection->data, data,dataSize); //INSURE memory block passed to memcpy overlap
	newSection->dataSize = dataSize;
	newSection->shdr = NULL;
	newSection->name = new char[strlen(name)+1];
	memcpy(newSection->name, name, strlen(name)+1);
	newSection->nameIndx = 0;
	if(DEBUG_MSG){
		bpinfo(" ADDED SECTION: %x %x\n", newSection->vaddr,
				 *(unsigned int*)newSection->data);
	}
	return ++newSectionsSize;
}

//This method updates the symbol table,
//it shifts each symbol address as necessary AND
//sets _end and _END_ to move the heap
void writeBackElf::updateSymbols(Elf_Data* symtabData,Elf_Data* strData){

	if( symtabData && strData){

		Elf32_Sym *symPtr=(Elf32_Sym*)symtabData->d_buf;
		
		for(unsigned int i=0;i< symtabData->d_size/(sizeof(Elf32_Sym));i++,symPtr++){
						
			if( newHeapAddr && !(strcmp("_end", (char*) strData->d_buf + symPtr->st_name))){
				symPtr->st_value = newHeapAddr;
			}

			if( newHeapAddr &&  !(strcmp("_END_", (char*) strData->d_buf + symPtr->st_name))){
				symPtr->st_value = newHeapAddr; 
			}
			
		}
	}
}


//This is the main processing loop, called from outputElf()
void writeBackElf::driver(){

	Elf32_Shdr *newsh, *shdr;
	Elf_Scn *scn, *newScn; 
        Elf32_Ehdr *ehdr ;//= elf32_getehdr(oldElf);
	Elf_Data *data = NULL, *newdata = NULL, *olddata = NULL;

	ehdr = elf32_getehdr(oldElf);
	if(!(newEhdr = elf32_newehdr(newElf))){
		bperr("newEhdr failed\n");
		exit(1);
	}


	if (((ehdr = elf32_getehdr(oldElf)) != NULL)){ 
		if((scn = elf_getscn(oldElf, ehdr->e_shstrndx)) != NULL){
			if((data = elf_getdata(scn, NULL)) == NULL){
				bperr(" Failed obtaining .shstrtab data buffer \n");
				exit(1);
			}
		}else{
			bperr(" FAILED obtaining .shstrtab scn\n");		
		}
	}else{
		bperr(" FAILED obtaining .shstrtab ehdr\n");
	}
	memcpy(newEhdr, ehdr, sizeof(Elf32_Ehdr));


	scn = NULL;
	int currentOffset=-1;
	for (int cnt = 1; (scn = elf_nextscn(oldElf, scn)); cnt++) {
		//copy sections from oldElf to newElf.
	
		shdr = elf32_getshdr(scn);
		newScn = elf_newscn(newElf);
		newsh = elf32_getshdr(newScn);
		newdata = elf_newdata(newScn);
		olddata = elf_getdata(scn,NULL);
		memcpy(newsh, shdr, sizeof(Elf32_Shdr));
		memcpy(newdata,olddata, sizeof(Elf_Data));
		if(currentOffset == -1){
			currentOffset = shdr->sh_offset;
		}

               	//copy data buffer from oldElf 
		if(olddata->d_buf){
			if(MALLOC){
				newdata->d_buf = (char*) malloc(olddata->d_size);
			}else{
				newdata->d_buf = new char[olddata->d_size];
			} 
			memcpy(newdata->d_buf, olddata->d_buf, olddata->d_size);
		}

		if(!strcmp( (char *)data->d_buf + shdr->sh_name, ".strtab")){
			symStrData = newdata;
			elf_update(newElf,ELF_C_NULL);
			updateSymbols(symTabData, symStrData);
		}
		

		if(!strcmp( (char *)data->d_buf + shdr->sh_name, ".dynstr")){
			dynStrData = newdata;
			updateSymbols(dynsymData, dynStrData);
		}
		
		if(!strcmp( (char *)data->d_buf + shdr->sh_name, ".symtab")){
			if(newsh->sh_link >= insertPoint){
				newsh->sh_link += newSectionsSize;
			}
			symTabData = newdata;
		}
		if(!strcmp( (char *)data->d_buf + shdr->sh_name, ".dynsym")){
			dynsymData = newdata;
		}
		if(!strcmp( (char *)data->d_buf + shdr->sh_name, ".text")){
			textData = newdata;
			if(mutateeProcess){
				mutateeProcess->readTextSpace((const void*) newsh->sh_addr, newdata->d_size, 
														(void*)newdata->d_buf);
				
			}
			startAddr = newsh->sh_addr;
			endAddr = newsh->sh_addr + newsh->sh_size;
			textSh = newsh; 
		}
		if(!strcmp( (char *)data->d_buf + shdr->sh_name, ".bss")){
			createSections();
		}
		
		if(!strcmp( (char *)data->d_buf + shdr->sh_name, ".shstrtab")){
			addSectionNames(newdata,olddata);
		}
		if(!strcmp( (char *)data->d_buf + shdr->sh_name, ".data")){
			dataData = newdata;
			dataStartAddress = newsh->sh_addr;
			elf_update(newElf,ELF_C_NULL);
		}
	}

	
	Elf32_Phdr *tmp;
	
	tmp = elf32_getphdr(oldElf);
	newEhdr->e_phnum= ehdr->e_phnum; 
	newPhdr=elf32_newphdr(newElf,newEhdr->e_phnum);
	
	memcpy(newPhdr, tmp, (ehdr->e_phnum) * ehdr->e_phentsize);
	newEhdr->e_shstrndx+=newSectionsSize;
	
	fixPhdrs(newPhdr);
}

void writeBackElf::parseOldElf(){


	Elf_Scn *scn;
	Elf32_Shdr *shdr;
	Elf32_Ehdr *ehdr;
	Elf_Data *data;
	
	insertPoint = 0;


	if (((ehdr = elf32_getehdr(oldElf)) != NULL)){
		if((scn = elf_getscn(oldElf, ehdr->e_shstrndx)) != NULL){
			if((data = elf_getdata(scn, NULL)) == NULL){
				bperr(" Failed obtaining .shstrtab data buffer \n");
				exit(1);
			}
		}else{
			bperr(" FAILED obtaining .shstrtab scn\n");
		}
	}else{
		bperr(" FAILED obtaining .shstrtab ehdr\n");
	}
	
	
	

	if (((ehdr = elf32_getehdr(oldElf)) == NULL) ||
		 ((scn = elf_getscn(oldElf, ehdr->e_shstrndx)) == NULL) ||
		 ((data = elf_getdata(scn, NULL)) == NULL)){
		bpfatal(" Failed obtaining .shstrtab data buffer \n");
		exit(1);
	}
	
	scn = NULL;
	for (int cnt = 1; !insertPoint && (scn = elf_nextscn(oldElf, scn)); cnt++) {
		shdr = elf32_getshdr(scn);
		if(!(strcmp(".bss",(char*) data->d_buf + shdr->sh_name))){
			insertPoint = cnt;
			lastAddr = shdr->sh_addr + shdr->sh_size;
			oldLastPage = lastAddr / pageSize;
		}			
	}
	//oldLastPage = 0;	// ccw 20 mar
}

bool writeBackElf::createElf(){
	unsigned int i;
	for(i=0;i< newSectionsSize ;i++){

	/* move the heap address beyond the last dyninstAPI_######## section */
		if( newSections[i].name ){
			char *strStart = strstr(newSections[i].name,"dyninstAPI_");

			if(strStart){
				strStart =  strstr(newSections[i].name,"_");
				strStart ++;
				/*make sure this is a dyninstAPI_######## section*/
				if( *strStart>=0x30 && *strStart <= 0x39 ) {

					newHeapAddr = newSections[i].vaddr +newSections[i].dataSize;

       			        	while(newHeapAddr % pageSize){
                       				newHeapAddr++;
                			}
				}

			}

		}
	}

	driver();
	return true;
}

 
void writeBackElf::createSections(){

//newSections
//lets assume that newSections is sorted on vaddr

	Elf_Scn *newScn;
	Elf32_Shdr *newsh;
	Elf_Data *newdata;

	unsigned int i=0;
/*  this is the expansion of the bss section ccw 20 mar 
    it is not longer needed

	if(oldLastPage == newSections[0].vaddr/pageSize){
		bssSh->sh_size = newSections[0].vaddr - bssSh->sh_addr;// + newSections[0].dataSize;
		void *tmpBuf;
		if(MALLOC){
			tmpBuf = malloc(bssSh->sh_size);
		}else{
			tmpBuf = (void*) new char[bssSh->sh_size];
		}
		memset(tmpBuf, '\0', bssSh->sh_size);
		if(bssData->d_buf){
			memcpy(tmpBuf, bssData->d_buf, bssData->d_size);
		}
		bssData->d_size = bssSh->sh_size;


		delete [] (char*)bssData->d_buf; 
		bssData->d_buf = tmpBuf;
		bssSh->sh_type = SHT_PROGBITS;
		elf_update(newElf,ELF_C_NULL);
	}
*/

	for(;i<newSectionsSize;i++){
		if(DEBUG_MSG){
			bpinfo("ADDING SECTION");
		}
		newScn = elf_newscn(newElf);
		newsh = elf32_getshdr(newScn);
		newdata = elf_newdata(newScn);
		newSections[i].shdr=newsh;
		elf_update(newElf,ELF_C_NULL);
		newsh->sh_addr = newSections[i].vaddr;
		newsh->sh_size = newSections[i].dataSize;
		newsh->sh_addralign = 0x4;//newSections[i].align;
		if(newSections[i].loadable){
			newsh->sh_flags=  SHF_EXECINSTR | SHF_WRITE | SHF_ALLOC ;//newSections[i].flags;
			newsh->sh_type = SHT_PROGBITS;//newSections[i].type; 
		}else{
			newsh->sh_flags = 0;
			newsh->sh_type = SHT_PROGBITS; //SHT_NOTE
		}
		if(MALLOC){
			newdata->d_buf = (char*)malloc(newSections[i].dataSize);
		}else{
			newdata->d_buf = new char[newSections[i].dataSize];
		}
		newdata->d_size = newSections[i].dataSize;
		memcpy((char*) newdata->d_buf, (char*) newSections[i].data, newdata->d_size);
		elf_update(newElf, ELF_C_NULL);
		if(DEBUG_MSG){
			bpinfo("ADDED: size %lx Addr %lx size %x data; %x\n",newsh->sh_size, newsh->sh_addr,
					 newdata->d_size,*(unsigned int*) newdata->d_buf);
		}
	}
}


void writeBackElf::addSectionNames(Elf_Data *newdata, Elf_Data*olddata){

	int totalSize = olddata->d_size;
	for(unsigned int i=0;i<newSectionsSize;i++){
		totalSize += (strlen(newSections[i].name)+1);
	}
	if(MALLOC){
	        free(newdata->d_buf);
	}else{
		delete [](char*) newdata->d_buf;
	}
	if(MALLOC){
		newdata->d_buf = (char*) malloc(totalSize);
	}else{
		newdata->d_buf =new char[totalSize];
	}
	memcpy(newdata->d_buf, olddata->d_buf, olddata->d_size);

	int currLoc = olddata->d_size;
	for(unsigned int k=0;k<newSectionsSize;k++){
		memcpy(&( ((char*)newdata->d_buf)[currLoc]), 
				 newSections[k].name,strlen(newSections[k].name)+1);
		if(newSections[k].shdr){
			newSections[k].shdr->sh_name = currLoc;
		}
		currLoc += (strlen(newSections[k].name)+1);
		newdata->d_size += (strlen(newSections[k].name)+1);
	}
}


void writeBackElf::fixPhdrs(Elf32_Phdr * /*phdr*/){ 

	elf_update(newElf, ELF_C_NULL);
	unsigned int i=0;
	if(oldLastPage == newSections[0].vaddr/pageSize){
		i=1;
	}

#if  0  
 defined(i386_unknown_linux2_0) || defined(x86_64_unknown_linux2_4)

	/* 	on LINUX we may have a STACK section in the PHT (PT_GNU_STACK) 
		if we do, remove it
	*/

	while(phdr->p_type != PT_NULL && phdr->p_type != PT_GNU_STACK ){
		phdr ++;
	}
	phdr->p_type = PT_NULL;
		
#endif
		
}


void writeBackElf::compactLoadableSections(pdvector <imageUpdate*> imagePatches,
														 pdvector<imageUpdate*> &newPatches){
	int startPage, stopPage;
	imageUpdate *patch;
	//this function now returns only ONE section that is loadable.

	imageUpdate *curr, *next;
	bool foundDup=true;
	unsigned int j;

	VECTOR_SORT(imagePatches, imageUpdateSort);

	if(imagePatches.size() == 0){
		return;
	}

	while(foundDup){
		foundDup = false;
		j =0;
		while(imagePatches[j]->address==0 && j < imagePatches.size()){
			j++;
		}
		curr = imagePatches[j];
		
		for(j++;j<imagePatches.size();j++){
			next = imagePatches[j];		
			if(curr->address == next->address){
				//duplicate
				//find which is bigger and save that one.
				if(curr->size > next->size){
					next->address=0;
				}else{
					curr->address=0;
					curr=next;
				}
				foundDup =true;
			}else{
				curr=next;
			}

		}
		VECTOR_SORT(imagePatches, imageUpdateSort);
	}


	for(unsigned int i=0;i<imagePatches.size();i++){
		if(imagePatches[i]->address!=0){
			imagePatches[i]->startPage = imagePatches[i]->address- imagePatches[i]->address%pageSize;
			imagePatches[i]->stopPage = imagePatches[i]->address + imagePatches[i]->size- 
				(imagePatches[i]->address + imagePatches[i]->size )%pageSize;

		}
	}

	foundDup = true;

	while(foundDup){
		foundDup = false;
		j =0;
		while(imagePatches[j]->address==0 && j < imagePatches.size()){
			j++;
		}
		imagePatches.erase(0,j-1);
		j=0;
		for(;j<imagePatches.size()-1;j++){
			if(imagePatches[j]->stopPage > imagePatches[j+1]->startPage){
				foundDup = true;
				if(imagePatches[j]->stopPage > imagePatches[j+1]->stopPage){
					imagePatches[j+1]->address = 0;	
				}else{
					
					imagePatches[j]->size = (imagePatches[j+1]->address + imagePatches[j+1]->size) -
						imagePatches[j]->address;
					imagePatches[j+1]->address = 0; 
					imagePatches[j]->stopPage = imagePatches[j]->address + imagePatches[j]->size-
						(imagePatches[j]->address + imagePatches[j]->size )%pageSize;		
				}
			}  
		}
		VECTOR_SORT(imagePatches, imageUpdateSort);
	}

	unsigned int k=0;

	while(imagePatches[k]->address==0 && k < imagePatches.size()){
		k++;
	}

	startPage = imagePatches[k]->startPage;
	stopPage = imagePatches[imagePatches.size()-1]->stopPage;
	int startIndex=k, stopIndex=imagePatches.size()-1;
	/*if(DEBUG_MSG){
		bpinfo("COMPACTING....\n");	
		bpinfo("COMPACTING %x %x %x\n", imagePatches[0]->startPage, stopPage, imagePatches[0]->address);
	}
	patch = new imageUpdate;
        patch->address = imagePatches[startIndex]->address;
        patch->size = imagePatches[stopIndex]->address - imagePatches[startIndex]->address +
                                   imagePatches[stopIndex]->size;
        newPatches.push_back(patch);
	if(DEBUG_MSG){
		bperr(" COMPACTED: %x --> %x \n", patch->address, patch->size);
	}*/
	bool finished = false;
	if(DEBUG_MSG){
		bperr("COMPACTING....\n");	
		bperr("COMPACTING %x %x %x\n", imagePatches[0]->startPage, stopPage, imagePatches[0]->address);
	}
	for(;k<imagePatches.size();k++){
		if(imagePatches[k]->address!=0){
			if(DEBUG_MSG){
				bperr("COMPACTING k[start] %x k[stop] %x stop %x addr %x size %x\n", imagePatches[k]->startPage, 
						 imagePatches[k]->stopPage,stopPage, imagePatches[k]->address, imagePatches[k]->size);
			}
			if(imagePatches[k]->startPage <= (unsigned int) stopPage){
				stopIndex = k;
				stopPage = imagePatches[k]->stopPage;
			}else{
				
				patch = new imageUpdate;
				patch->address = imagePatches[startIndex]->address;
				patch->size = imagePatches[stopIndex]->address - imagePatches[startIndex]->address + 
						imagePatches[stopIndex]->size;
				newPatches.push_back(patch);
				if(DEBUG_MSG){
					bperr(" COMPACTED: address %x --> %x    start %x  stop %x\n", 
							 patch->address, patch->size, startPage,  stopPage);
				}
				finished = true;
				//was k+1	
				if(k < imagePatches.size()){
					while(imagePatches[k]->address==0 && k < imagePatches.size()){
						k++;
					}
					startIndex = k;
					stopIndex = k;
					startPage = imagePatches[k]->startPage;
					stopPage  = imagePatches[k]->stopPage;
					finished = false;
					if(k == imagePatches.size()){
						finished = true;
					}
				} 
			}
		}
		
	}

	if(!finished){
		patch = new imageUpdate;
		patch->address = imagePatches[startIndex]->address;
		patch->size = imagePatches[stopIndex]->address - imagePatches[startIndex]->address +
			imagePatches[stopIndex]->size;
		newPatches.push_back(patch);
		if(DEBUG_MSG){
			bperr(" COMPACTED: %x --> %x \n", patch->address, patch->size);
		}
	}	

	
}

void writeBackElf::compactSections(pdvector <imageUpdate*> imagePatches, pdvector<imageUpdate*> &newPatches){

	int startPage, stopPage;
	imageUpdate *patch;


	imageUpdate *curr, *next;
	bool foundDup=true;
	unsigned int j;

	VECTOR_SORT(imagePatches, imageUpdateSort);

	if(imagePatches.size() == 0){ //ccw 2 jul 2003
		return;
	}

	while(foundDup){
		foundDup = false;
		j =0;
		while(imagePatches[j]->address==0 && j < imagePatches.size()){
			j++;
		}
		curr = imagePatches[j];
		
		for(j++;j<imagePatches.size();j++){
			next = imagePatches[j];		
			if(curr->address == next->address){
				//duplicate
				//find which is bigger and save that one.
				if(curr->size > next->size){
					next->address=0;
				}else{
					curr->address=0;
					curr=next;
				}
				foundDup =true;
			}else{
				curr=next;
			}

		}
		VECTOR_SORT(imagePatches, imageUpdateSort);
	}
	if(DEBUG_MSG){
		bperr(" SORT 1\n");
	
		for(unsigned int kk=0;kk<imagePatches.size();kk++){
			bperr(" address 0x%x  size 0x%x \n", imagePatches[kk]->address, imagePatches[kk]->size);
		}
		fflush(stdout);
	}

	unsigned int end_addr;
	for(unsigned int i=0;i<imagePatches.size();i++){
		if(imagePatches[i]->address!=0){
			imagePatches[i]->startPage = imagePatches[i]->address- (imagePatches[i]->address%pageSize);
				
			end_addr = imagePatches[i]->address + imagePatches[i]->size;
			imagePatches[i]->stopPage =  end_addr - (end_addr % pageSize);

			if(DEBUG_MSG){
				bperr(" address %x end addr %x : start page %x stop page %x \n",
						 imagePatches[i]->address,
						 imagePatches[i]->address + imagePatches[i]->size,
						 imagePatches[i]->startPage, imagePatches[i]->stopPage);
			}
			
		}
	}
	foundDup = true;

	while(foundDup){
		foundDup = false;
		j =0;
		while(imagePatches[j]->address==0 && j < imagePatches.size()){
			j++;
		}
		//imagePatches.erase(0,j-1); //is it correct to erase here? 
		//j = 0;
		for(;j<imagePatches.size()-1;j++){
			if(imagePatches[j]->address!=0 && imagePatches[j]->stopPage >= imagePatches[j+1]->startPage){
				foundDup = true;
				if(imagePatches[j]->stopPage > imagePatches[j+1]->stopPage){
					imagePatches[j+1]->address = 0;	
				}else{
					imagePatches[j]->size = (imagePatches[j+1]->address + imagePatches[j+1]->size) -
						imagePatches[j]->address;
					imagePatches[j+1]->address = 0; 
					end_addr = imagePatches[j]->address + imagePatches[j]->size;
					imagePatches[j]->stopPage =  end_addr - (end_addr % pageSize);
				}
			}  
		}
		VECTOR_SORT(imagePatches, imageUpdateSort);
	}

	unsigned int k=0;

	if(DEBUG_MSG){
		bperr(" SORT 3\n");

		for(unsigned int kk=0;kk<imagePatches.size();kk++){
			bperr(" address 0x%x  size 0x%x \n", imagePatches[kk]->address, imagePatches[kk]->size);
		}
		fflush(stdout);
	}
	while(imagePatches[k]->address==0 && k < imagePatches.size()){
		k++;
	}

	startPage = imagePatches[k]->startPage;
	stopPage = imagePatches[k]->stopPage;
	int startIndex=k, stopIndex=k;
	bool finished = false;
	if(DEBUG_MSG){
		bperr("COMPACTING....\n");	
		bperr("COMPACTING %x %x %x\n", imagePatches[0]->startPage, stopPage, imagePatches[0]->address);
	}
	for(;k<imagePatches.size();k++){
		if(imagePatches[k]->address!=0){
			if(DEBUG_MSG){
				bperr("COMPACTING k[start] %x k[stop] %x stop %x addr %x size %x\n", imagePatches[k]->startPage, 
						 imagePatches[k]->stopPage,stopPage, imagePatches[k]->address, imagePatches[k]->size);
			}
			if(imagePatches[k]->startPage <= (unsigned int) stopPage){
				stopIndex = k;
				stopPage = imagePatches[k]->stopPage;
			}else{

				patch = new imageUpdate;
				patch->address = imagePatches[startIndex]->address;
				patch->size = imagePatches[stopIndex]->address - imagePatches[startIndex]->address + 
						imagePatches[stopIndex]->size;
				newPatches.push_back(patch);
				if(DEBUG_MSG){
					bperr(" COMPACTED: address %x --> %x    start %x  stop %x\n", 
							 patch->address, patch->size, startPage,  stopPage);
				}
				finished = true;
				//was k+1	
				if(k < imagePatches.size()){
					while(imagePatches[k]->address==0 && k < imagePatches.size()){
						k++;
					}
					startIndex = k;
					stopIndex = k;
					startPage = imagePatches[k]->startPage;
					stopPage  = imagePatches[k]->stopPage;
					finished = false;
					if(k == imagePatches.size()){
						finished = true;
					}
				} 
			}
		}

	}

	if(!finished){
		patch = new imageUpdate;
		patch->address = imagePatches[startIndex]->address;
		patch->size = imagePatches[stopIndex]->address - imagePatches[startIndex]->address +
			imagePatches[stopIndex]->size;
		newPatches.push_back(patch);
		if(DEBUG_MSG){
			bperr(" COMPACTED: %x --> %x \n", patch->address, patch->size);
		}
	}	
	
}

void writeBackElf::alignHighMem(pdvector<imageUpdate*> imagePatches){
	unsigned int currPage;
	unsigned int sizeDiff;

	for(unsigned int i=0;i<imagePatches.size();i++){
		if(imagePatches[i]->address % pageSize != 0){
			currPage = imagePatches[i]->address - ( imagePatches[i]->address % pageSize);
			sizeDiff = imagePatches[i]->address - currPage;
			imagePatches[i]->address = currPage;
			imagePatches[i]->size = imagePatches[i]->size + sizeDiff;
		}
	}
 
}

void writeBackElf::registerProcess(process *proc){

	mutateeProcess = proc;
}
#endif

