/* $Id: writeBackElf.C,v 1.9 2002/05/14 20:20:51 chadd Exp $ */

#if defined(BPATCH_LIBRARY) 
#if defined(sparc_sun_solaris2_4) || defined(i386_unknown_linux2_0)

#include "writeBackElf.h"
#define MALLOC 0 
#define DYNAMIC 1 

unsigned int elf_version(unsigned int);

void writeBackElf::setHeapAddr(unsigned int heapAddr){
	newHeapAddr = heapAddr;
        while(newHeapAddr % 0x8){//SPARC alignment
        	newHeapAddr++;
        }
}
// This constructor opens both the old and new
// ELF files 
writeBackElf::writeBackElf(char *oldElfName, char* newElfName, int debugOutputFlag){

	int oldfd, newfd;


	if((oldfd = open(oldElfName, O_RDONLY)) == -1){
		printf(" OLDELF_OPEN_FAIL %s",oldElfName);
		fflush(stdout);		
		return;
	}
        if((newfd = (creat(newElfName, 0x1c0)))==-1){
		//printf("NEWELF_OPEN_FAIL %s", newElfName);
		char *fileName = new char[strlen(newElfName)+1+3];
		for(int i=0;newfd == -1 && i<100;i++){
			sprintf(fileName, "%s%d",newElfName,i);
			newfd = (open(fileName, O_WRONLY|O_CREAT));
		}
                fflush(stdout);
		if(newfd == -1){
			printf("NEWELF_OPEN_FAIL %s. clean up /tmp/dyninstMutatee*\n", newElfName);
			return; 
		}
		delete [] fileName;
	}
        elf_version(EV_CURRENT);
        if ((oldElf = elf_begin(oldfd, ELF_C_READ, NULL)) == NULL){
		printf("OLDELF_BEGIN_FAIL");
                fflush(stdout);
		return;
	}
      	if((newElf = elf_begin(newfd, ELF_C_WRITE, NULL)) == NULL){
		elf_end(oldElf);
		printf("NEWELF_BEGIN_FAIL");
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
	elf_end(oldElf);
	elf_end(newElf);
}


int writeBackElf::addSection(unsigned int addr, void *data, unsigned int dataSize, char* name,bool loadable){
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
		(char*)newSection->data = new char[dataSize];
	} 
	memcpy(newSection->data, data,dataSize);
	newSection->dataSize = dataSize;
	newSection->shdr = NULL;
	newSection->name = new char[strlen(name)+1];
	memcpy(newSection->name, name, strlen(name)+1);
	newSection->nameIndx = 0;
	if(DEBUG_MSG){
		printf(" ADDED SECTION: %x %x\n", newSection->vaddr, *(unsigned int*)newSection->data);
	}
	return ++newSectionsSize;
}

//This method updates the symbol table,
//it shifts each symbol address as necessary AND
//sets _end and _END_ to move the heap
void writeBackElf::updateSymbols(Elf_Data* symtabData,Elf_Data* strData){

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

unsigned int writeBackElf::findAddressOf(char *objName){

	Elf32_Sym * symPtr = (Elf32_Sym*)symTabData->d_buf;	
	unsigned int objAddr = 0;
	
        for(unsigned int i=0;objAddr ==0 && i< symTabData->d_size/(sizeof(Elf32_Sym));i++,symPtr++){
                if( !(strcmp(objName, (char*) symStrData->d_buf + symPtr->st_name))){
                        objAddr = symPtr->st_value;
                }

        }

	return objAddr;
}


//This is the main processing loop, called from outputElf()
void writeBackElf::driver(){

	Elf32_Shdr *newsh, *shdr, *dynamicShdr;
	Elf_Scn *scn, *newScn; 
        Elf32_Ehdr *ehdr ;//= elf32_getehdr(oldElf);
	Elf_Data *data, *newdata, *olddata;

	ehdr = elf32_getehdr(oldElf);
        if(!(newEhdr = elf32_newehdr(newElf))){
		printf("newEhdr failed\n");
		exit(1);
        }


        if (((ehdr = elf32_getehdr(oldElf)) != NULL)){ 
             if((scn = elf_getscn(oldElf, ehdr->e_shstrndx)) != NULL){
       		      if((data = elf_getdata(scn, NULL)) == NULL){
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




	scn = NULL;
	for (int cnt = 1; (scn = elf_nextscn(oldElf, scn)); cnt++) {
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
			if(MALLOC){
				(char*) newdata->d_buf = (char*) malloc(olddata->d_size);
           		}else{
				(char*) newdata->d_buf = new char[olddata->d_size];
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
			createSections(newsh, newdata);
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

	fixPhdrs((int) ehdr->e_phnum-1);

}

bool writeBackElf::writeOutNewElf(){

        elf_update(newElf, ELF_C_WRITE);
        return !elf_end(newElf);
	
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
                                printf(" Failed obtaining .shstrtab data buffer \n");
                                exit(1);
                        }
                }else{
                        printf(" FAILED obtaining .shstrtab scn\n");
                }
        }else{
                printf(" FAILED obtaining .shstrtab ehdr\n");
        }




        if (((ehdr = elf32_getehdr(oldElf)) == NULL) ||
             ((scn = elf_getscn(oldElf, ehdr->e_shstrndx)) == NULL) ||
             ((data = elf_getdata(scn, NULL)) == NULL)){
                printf(" Failed obtaining .shstrtab data buffer \n");
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
        for(i=0;i< newSectionsSize && newSections[i].loadable;i++); // find the last loadable section
	if(i ||  oldLastPage == newSections[0].vaddr/pageSize  ){
		//if we find a loadable sectin
		//OR the first section is immediately after
		//.bss but not loadable
                if(!i){ //pretend we found a loadalbe seciton at position 0
                        i++;
                }
		newHeapAddr = newSections[i-1].vaddr +newSections[i-1].dataSize;

	        while(newHeapAddr % 0x8){
       	        	newHeapAddr++;
        	}
	}
        driver();
        return true;
}

 
bool writeBackElf::outputElf(){
	return writeOutNewElf();
}




void writeBackElf::createSections(Elf32_Shdr *bssSh, Elf_Data* bssData){

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
			printf("ADDING SECTION");
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
			(char*) newdata->d_buf = (char*)malloc(newSections[i].dataSize);
		}else{
			(char*) newdata->d_buf = new char[newSections[i].dataSize];
		}
	        newdata->d_size = newSections[i].dataSize;
		memcpy((char*) newdata->d_buf, (char*) newSections[i].data, newdata->d_size);
		elf_update(newElf, ELF_C_NULL);
		if(DEBUG_MSG){
			printf("ADDED: size %lx Addr %lx size %x data; %x\n",newsh->sh_size, newsh->sh_addr,
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
		(char*)newdata->d_buf = (char*) malloc(totalSize);
	}else{
		(char*)newdata->d_buf =new char[totalSize];
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


void writeBackElf::fixPhdrs(int oldPhdrs){ 

	elf_update(newElf, ELF_C_NULL);
	unsigned int i=0;
	if(oldLastPage == newSections[0].vaddr/pageSize){
		i=1;
	}
}


void writeBackElf::compactLoadableSections(vector <imageUpdate*> imagePatches, vector<imageUpdate*> &newPatches){
	int startPage, stopPage;
	imageUpdate *patch;
	//this function now returns only ONE section that is loadable.

	imageUpdate *curr, *next;
	bool foundDup=true;
	unsigned int j;

	VECTOR_SORT(imagePatches, imageUpdate::imageUpdateSort);

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
		VECTOR_SORT(imagePatches, imageUpdate::imageUpdateSort);
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
		VECTOR_SORT(imagePatches, imageUpdate::imageUpdateSort);
	}

	unsigned int k=0;

	while(imagePatches[k]->address==0 && k < imagePatches.size()){
	        k++;
        }

	startPage = imagePatches[k]->startPage;
	stopPage = imagePatches[imagePatches.size()-1]->stopPage;
	int startIndex=k, stopIndex=imagePatches.size()-1;
	/*if(DEBUG_MSG){
		printf("COMPACTING....\n");	
		printf("COMPACTING %x %x %x\n", imagePatches[0]->startPage, stopPage, imagePatches[0]->address);
	}
	patch = new imageUpdate;
        patch->address = imagePatches[startIndex]->address;
        patch->size = imagePatches[stopIndex]->address - imagePatches[startIndex]->address +
                                   imagePatches[stopIndex]->size;
        newPatches.push_back(patch);
	if(DEBUG_MSG){
		printf(" COMPACTED: %x --> %x \n", patch->address, patch->size);
	}*/
	bool finished = false;
	if(DEBUG_MSG){
		printf("COMPACTING....\n");	
		printf("COMPACTING %x %x %x\n", imagePatches[0]->startPage, stopPage, imagePatches[0]->address);
	}
	for(;k<imagePatches.size();k++){
		if(imagePatches[k]->address!=0){
			if(DEBUG_MSG){
				printf("COMPACTING k[start] %x k[stop] %x stop %x addr %x size %x\n", imagePatches[k]->startPage, 
					imagePatches[k]->stopPage,stopPage, imagePatches[k]->address, imagePatches[k]->size);
			}
			if(imagePatches[k]->startPage <= stopPage){
				stopIndex = k;
				stopPage = imagePatches[k]->stopPage;
			}else{

				patch = new imageUpdate;
				patch->address = imagePatches[startIndex]->address;
				patch->size = imagePatches[stopIndex]->address - imagePatches[startIndex]->address + 
						imagePatches[stopIndex]->size;
				newPatches.push_back(patch);
				if(DEBUG_MSG){
					printf(" COMPACTED: address %x --> %x    start %x  stop %x\n", 
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
			printf(" COMPACTED: %x --> %x \n", patch->address, patch->size);
			fflush(stdout);
		}
	}	

	
}

void writeBackElf::compactSections(vector <imageUpdate*> imagePatches, vector<imageUpdate*> &newPatches){

	int startPage, stopPage;
	imageUpdate *patch;


	imageUpdate *curr, *next;
	bool foundDup=true;
	unsigned int j;

	VECTOR_SORT(imagePatches, imageUpdate::imageUpdateSort);

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
		VECTOR_SORT(imagePatches, imageUpdate::imageUpdateSort);
	}
	if(DEBUG_MSG){
		printf(" SORT 1\n");
	
		for(unsigned int kk=0;kk<imagePatches.size();kk++){
			printf(" address 0x%x  size 0x%x \n", imagePatches[kk]->address, imagePatches[kk]->size);
		}
		fflush(stdout);
	}

	unsigned int endAddr;
	for(unsigned int i=0;i<imagePatches.size();i++){
		if(imagePatches[i]->address!=0){
			imagePatches[i]->startPage = imagePatches[i]->address- (imagePatches[i]->address%pageSize);
				
			endAddr = imagePatches[i]->address + imagePatches[i]->size;
			imagePatches[i]->stopPage =  endAddr - (endAddr % pageSize);

			if(DEBUG_MSG){
				printf(" address %x end addr %x : start page %x stop page %x \n",
					imagePatches[i]->address ,imagePatches[i]->address + imagePatches[i]->size,
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
					endAddr = imagePatches[j]->address + imagePatches[j]->size;
					imagePatches[j]->stopPage =  endAddr - (endAddr % pageSize);
				}
			}  
		}
		VECTOR_SORT(imagePatches, imageUpdate::imageUpdateSort);
	}

	unsigned int k=0;

	if(DEBUG_MSG){
		printf(" SORT 3\n");

		for(unsigned int kk=0;kk<imagePatches.size();kk++){
			printf(" address 0x%x  size 0x%x \n", imagePatches[kk]->address, imagePatches[kk]->size);
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
		printf("COMPACTING....\n");	
		printf("COMPACTING %x %x %x\n", imagePatches[0]->startPage, stopPage, imagePatches[0]->address);
	}
	for(;k<imagePatches.size();k++){
		if(imagePatches[k]->address!=0){
			if(DEBUG_MSG){
				printf("COMPACTING k[start] %x k[stop] %x stop %x addr %x size %x\n", imagePatches[k]->startPage, 
					imagePatches[k]->stopPage,stopPage, imagePatches[k]->address, imagePatches[k]->size);
			}
			if(imagePatches[k]->startPage <= stopPage){
				stopIndex = k;
				stopPage = imagePatches[k]->stopPage;
			}else{

				patch = new imageUpdate;
				patch->address = imagePatches[startIndex]->address;
				patch->size = imagePatches[stopIndex]->address - imagePatches[startIndex]->address + 
						imagePatches[stopIndex]->size;
				newPatches.push_back(patch);
				if(DEBUG_MSG){
					printf(" COMPACTED: address %x --> %x    start %x  stop %x\n", 
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
			printf(" COMPACTED: %x --> %x \n", patch->address, patch->size);
			fflush(stdout);
		}
	}	
	
}

void writeBackElf::alignHighMem(vector<imageUpdate*> imagePatches){
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
#endif
