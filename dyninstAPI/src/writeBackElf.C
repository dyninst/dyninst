/* $Id: writeBackElf.C,v 1.1 2001/12/11 20:22:26 chadd Exp $ */

#if defined(BPATCH_LIBRARY) && defined(sparc_sun_solaris2_4)

#include "writeBackElf.h"
#define MALLOC 0 
#define DYNAMIC 1 
#define SHIFTSIZE  0x20

unsigned int elf_version(unsigned int);

// This constructor opens both the old and new
// ELF files 
writeBackElf::writeBackElf(char *oldElfName, char* newElfName, int debugOutputFlag){

	int oldfd, newfd;


	if((oldfd = open(oldElfName, O_RDONLY)) == -1){
		printf(" OLDELF_OPEN_FAIL %s",oldElfName);
		fflush(stdout);		
		return;
	}
        if((newfd = (open(newElfName, O_WRONLY|O_CREAT)))==-1){
		printf("NEWELF_OPEN_FAIL %s", newElfName);
		char *fileName = new char[strlen(newElfName+1+3)];
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
	newSections = NULL;
	newSectionsSize = 0;
	DEBUG_MSG = debugOutputFlag;
	shiftSize = 0;
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
	firstValidInstruction = 0;
	elf_fill(0);
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
	if(loadable){
		shiftSize +=SHIFTSIZE;
	}
	if(DEBUG_MSG){
		printf(" ADDED SECTION: %x %x\n", newSection->vaddr, *(unsigned int*)newSection->data);
	}
	return ++newSectionsSize;
}

void writeBackElf::fixRela(Elf_Data *relaData){
        Elf32_Rela *rData;
        unsigned int rSym, rType;

	if(DEBUG_MSG){	
		printf(" RELA\n");
	}
       	rData = (Elf32_Rela*) (relaData->d_buf);
       	for(unsigned int i=0;i< relaData->d_size/(sizeof(Elf32_Rela)) ; i++, rData++){
                rSym = ELF32_R_SYM(rData->r_info);
                rType = ELF32_R_TYPE(rData->r_info);
		rData->r_offset +=shiftSize;
                if(rType > insertPoint){
                        rType += (shiftSize/SHIFTSIZE);
			if(DEBUG_MSG){
	                        printf(" CHANGED: %x\n",shiftSize);
     			} 
	        }
                rData->r_info = ELF32_R_INFO(rSym,rType);
       }
}

//This method looks for a
// SETHI
// OR
// CALL/LD 
// block and patches the address by the shift value
void writeBackElf::fixMainJmp(Elf_Data *textData){
//this function currently contains SPARC specific op codes!
        unsigned int *currInsn;
        unsigned int mask = 0xc1c00000;
        unsigned int currAddr=startAddr;
        unsigned int simm13, disp30;
        currInsn = (unsigned int*) textData->d_buf;
	if(DEBUG_MSG){
	        printf(" fixMainJmp: %x \n", mask);
	}
        while(currAddr<=endAddr){
                if( ((*currInsn & mask) == (0x01000000 /*+ (rodataAddr/0x100)/0x4*/)) ){
                        if( (*(currInsn+1) & 0x80102000)==0x80102000){
				if(DEBUG_MSG){
                                	printf(" FOUND OR %x %x\n",currAddr+4, *(currInsn+1));
					printf(" RODATA ADDR %x %x\n", rodataAddr, rodataSize);
				}
                                simm13 = *(currInsn+1) & 0x00001fff;
                                unsigned int jumpTO = simm13 +( (0x003fffff & *(currInsn)) * 0x100 * 0x4);
                                if(1 && (jumpTO>0x10000 /*rodataAddr*/ /*&& jumpTO<rodataAddr+rodataSize*/)) {
                                        simm13+=shiftSize;
                                        if(simm13 >= 0x400){
                                  		unsigned int imm22 = *currInsn & 0x003fffff; 
					        *currInsn &= 0xffc00000;
						//fix this such that rodataAddr is
						//replaced with the value in the instruction
						
                                                *currInsn |= imm22 + 0x1;//((rodataAddr)/0x100)/0x4 + 0x1;
                                                simm13 -= 0x400;
                                        }


                                        *(currInsn+1) = (*(currInsn+1) & (0xffffe000));
                                        *(currInsn+1) |= simm13;
                                        if(DEBUG_MSG){
						printf("%x NEW OR %x -- %x\n",currAddr,  *(currInsn+1), simm13);
					}
                                }
                        }

                }
		//This checks each CALL instruction and shifts the displacement the
		//correct amount. Since everything shifts the same amount this should
		//never be necessary.
                if(0 &&  (*currInsn & 0x40000000) == 0x40000000 &&
                    (*currInsn & 0x80000000) == 0x00000000) {

                        disp30 = currAddr + ( (0x3fffffff & *currInsn)*0x4);
                        if(disp30>rodataAddr+rodataSize){
				if(DEBUG_MSG){	
                                	printf("%x: FOUND CALL %x --> %x\n", currAddr,disp30, disp30-shiftSize);
				}
                                disp30-=currAddr;
                                disp30-=shiftSize;
                                disp30/=0x4;
                                *currInsn = (0x40000000 | disp30);

                        }

                }
                currInsn++;
                currAddr+=4;
        }

}


//This method fixes up the Global Offst Table entries by
//shifting them all the correct amount.
void writeBackElf::updateGOTEntries(Elf_Data *gotData){
        unsigned int *gotEntry;
        unsigned int cnt;
        for(cnt=0, gotEntry = (unsigned int*) gotData->d_buf;cnt< gotData->d_size/sizeof(unsigned int);
                                cnt++, gotEntry++){

                unsigned int newAddr = *gotEntry;
                if(newAddr > (unsigned int) (firstValidInstruction-0x10000) && newAddr < lastAddr ){
                        newAddr += shiftSize;
                        if(DEBUG_MSG){
                                printf(" GOT ADDING %x %x\n", shiftSize, newAddr-shiftSize );
                        }
                 }
                *gotEntry=newAddr;
        }



}

//This method patches the text segment to fix any jumps that
//leave the text segment
void writeBackElf::patchMain(Elf_Data*textData){
//this function contains SPARC specific op codes
        unsigned int *currInsn;
        unsigned int currAddr;
        unsigned int mask = 0x40000000;
        unsigned int antiMask = ~mask;
        unsigned int jumpTo;

        currAddr = startAddr;
        currInsn = (unsigned int*) textData->d_buf;
        while(currAddr<=endAddr){
		if(DEBUG_MSG){
	                printf(" %x  %x  %x \n", *currInsn, ((*currInsn & mask) &mask), ((*currInsn) >> 31) );
		}
                if( ((*currInsn & mask) &mask) &&
                        (((*currInsn) >> 31) ==0) ){
                        jumpTo = antiMask & *currInsn;
                        jumpTo *=4;
                        if((jumpTo + currAddr) > endAddr){
                                jumpTo -= shiftSize;
                                jumpTo /= 0x4;
                                *currInsn = mask | jumpTo;
                        }
                }
                currInsn++;
                currAddr+=4;
        }
}

//This method remakes the Hash, elf_update(ELF_C_NULL)
//must be called before calling this
void writeBackElf::remakeHash(){

        Elf32_Sym* symPtr=(Elf32_Sym*) dynsymData->d_buf;
        int nbucket= *((Elf32_Word*) hashData->d_buf), nchain, symSize= dynsymData->d_size/(sizeof(Elf32_Sym)) ;
        nchain = ((Elf32_Word*) hashData->d_buf)[1];
        int counter =0;
        Elf32_Word* bucket, *chain;
        hashData->d_size = (nbucket+nchain+2) * (sizeof(Elf32_Word));
        bucket = &(((Elf32_Word*) hashData->d_buf)[2]);
        chain  = &(((Elf32_Word*) hashData->d_buf)[2+nbucket]);
        memset(hashData->d_buf, '\0', hashData->d_size);
        *(Elf32_Word*) hashData->d_buf = nbucket;
        ((Elf32_Word*) hashData->d_buf)[1]= nchain;
        while(!symPtr->st_name){
                symPtr++;
                counter++;
        }
        int hashValue;
        char *currentName=NULL;

        if(DEBUG_MSG){
                printf(" HASH TABLE\n");
        }
        while(counter<symSize){
                currentName = (char*) dynStrData->d_buf+ symPtr->st_name;
                hashValue = elf_hash(currentName) % nbucket;
                if(DEBUG_MSG){
                        printf("\n %s %d-->%lx", currentName,hashValue , bucket[hashValue]);
                }
                if(bucket[hashValue]==0){
                        /* success, we can put it here*/
                        bucket[hashValue] = counter;
                }else{
                        /* fail, cant put it here */
                        hashValue = bucket[hashValue];
                        while(chain[hashValue]){
                                if(DEBUG_MSG){
                                        printf(" %d-->%lx ",hashValue,chain[hashValue]);
                                }
                                hashValue = chain[hashValue];
                        }
                        if(DEBUG_MSG){
                                printf(" %d-->%lx ",hashValue,chain[hashValue]);
                        }
                        chain[hashValue]=counter;
                }
                counter++;
                symPtr++;
        }

}

//This method updates the symbol table,
//it shifts each symbol address as necessary AND
//sets _end and _END_ to move the heap
void writeBackElf::updateSymbols(Elf_Data* symtabData,Elf_Data* strData){

        Elf32_Sym *symPtr=(Elf32_Sym*)symtabData->d_buf;

        for(unsigned int i=0;i< symtabData->d_size/(sizeof(Elf32_Sym));i++,symPtr++){



                if(  (unsigned int) symPtr->st_value >= (unsigned int) firstValidInstruction &&
			 ( symPtr->st_value < lastAddr)){
                        symPtr->st_value += shiftSize;
                }
                if( !(strcmp("_end", (char*) strData->d_buf + symPtr->st_name))){
                        symPtr->st_value = newHeapAddr;
                }

                if( !(strcmp("_END_", (char*) strData->d_buf + symPtr->st_name))){
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


void writeBackElf::fixData(Elf_Data* newdata, unsigned int startAddress){
//this needs fixed. is p.3 always right there?
//why do i need to fix it?

	unsigned int p3Address;
	unsigned int *tmp;
	tmp = (unsigned int * ) newdata->d_buf;
	p3Address = findAddressOf("p.3");

	while(startAddress != p3Address){
		startAddress+=sizeof(*tmp);
		tmp ++;
	}	
	(*tmp) +=shiftSize;//this fixes p.3!
}



//This method updates the addresses of various symbols in the
//_DYNAMIC array
void  writeBackElf::updateDynamic(Elf_Data* dynamicData){

        __Elf32_Dyn *dynData;

        for ( dynData = (__Elf32_Dyn*) dynamicData->d_buf;  dynData->d_tag!= DT_NULL;dynData++){
                switch(dynData->d_tag){
                        case DT_STRTAB:
                        case DT_SYMTAB:
                        case DT_HASH:
                        case DT_RELA:
                        case DT_INIT:
                        case DT_FINI:
                        case DT_REL:
                        case DT_VERNEED:
                        case DT_JMPREL:
                        case DT_PLTGOT: 
                        dynData->d_un.d_val+=shiftSize;
                }
         }


}

//This is the main processing loop, called from outputElf()
void writeBackElf::driver(){

	Elf32_Shdr *newsh, *shdr, *dynamicShdr;
	Elf_Scn *scn, *newScn; 
        Elf32_Ehdr *ehdr = elf32_getehdr(oldElf);
	Elf_Data *data, *newdata, *olddata;

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




	if(oldLastPage == newSections[0].vaddr/pageSize ){
		//ok, the .bss section an the first newSection must
		//be merged. aint that nice? 
		shiftSize -=SHIFTSIZE;
	}
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

		if(!strcmp( (char *)data->d_buf + shdr->sh_name, ".interp")){
			firstValidInstruction = shdr->sh_addr;
		}
                if(newsh->sh_addr<lastAddr){
                        newsh->sh_addr +=shiftSize;
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
                        	newsh->sh_link += newSectionsSize;//(shiftSize/SHIFTSIZE);
			}
                        symTabData = newdata;
                }
                if(!strcmp( (char *)data->d_buf + shdr->sh_name, ".dynsym")){
                        dynsymData = newdata;
                }
                if(!strcmp( (char *)data->d_buf + shdr->sh_name, ".dynamic")){
			dynamicShdr = shdr;
                        updateDynamic(newdata);
       			Elf_Data * padData = elf_getdata(scn, olddata);
			if(padData){
				newdata = elf_newdata(newScn);
				memcpy(newdata,padData, sizeof(Elf_Data));
		                if(padData->d_buf){
                        		if(MALLOC){
                                		(char*) newdata->d_buf = (char*) malloc(olddata->d_size);
                        		}else{
                                		(char*) newdata->d_buf = new char[olddata->d_size];
                        		}
                        		memcpy(newdata->d_buf, olddata->d_buf, olddata->d_size);
                		}
				printf("ADDED PADDATA\n"); 
			}
	        }
                if(!strcmp( (char *)data->d_buf + shdr->sh_name, ".hash")){
                        hashData =newdata;
                }
                if(!strcmp( (char *)data->d_buf + shdr->sh_name, ".rodata")){
			rodataAddr = shdr->sh_addr;
			rodataSize = shdr->sh_size;
			rodataSh = newsh;
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
                if(!strncmp((char*) data->d_buf + shdr->sh_name,".rela",5)){
                        fixRela(newdata);
                }

                if(!strcmp( (char *)data->d_buf + shdr->sh_name, ".got")){
                        updateGOTEntries(newdata);
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
			//fixData(newdata, newsh->sh_addr);
		}
        }
	fixData(dataData, dataStartAddress);
        Elf32_Phdr *tmp;

        tmp = elf32_getphdr(oldElf);
        newEhdr->e_phnum+=shiftSize/SHIFTSIZE;
        newPhdr=elf32_newphdr(newElf,newEhdr->e_phnum);

        memcpy(newPhdr, tmp, (ehdr->e_phnum) * ehdr->e_phentsize);
        fixMainJmp(textData);
        newEhdr->e_shstrndx+=newSectionsSize;//(shiftSize/SHIFTSIZE);
        newEhdr->e_entry+=shiftSize;

	fixPhdrs((int) ehdr->e_phnum-1);
        remakeHash();

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

}

bool writeBackElf::createElf(){
//      parseOldElf();
        unsigned int i;
        for(i=0;i< newSectionsSize && newSections[i].loadable;i++); // find the last loadable section
        newHeapAddr = newSections[i-1].vaddr +newSections[i-1].dataSize;
        while(newHeapAddr % 0x8){
                newHeapAddr++;
        }

        driver();
        return true;
}

 
bool writeBackElf::outputElf(){
//	parseOldElf();
	return writeOutNewElf();
}




void writeBackElf::createSections(Elf32_Shdr *bssSh, Elf_Data* bssData){

//newSections
//lets assume that newSections is sorted on vaddr

	Elf_Scn *newScn;
	Elf32_Shdr *newsh;
	Elf_Data *newdata;

	unsigned int i=0;

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
			newsh->sh_type = SHT_NOTE;
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
        for(;i<newSectionsSize;i++){
		if(newSections[i].loadable){
        		newPhdr[oldPhdrs+i].p_vaddr = newSections[i].vaddr;
	        	newPhdr[oldPhdrs+i].p_paddr=0;
	        	newPhdr[oldPhdrs+i].p_filesz =newSections[i].dataSize;
	        	newPhdr[oldPhdrs+i].p_flags =  PF_R|PF_X|PF_W;
	        	newPhdr[oldPhdrs+i].p_type = PT_LOAD ;
	        	newPhdr[oldPhdrs+i].p_memsz =newSections[i].dataSize;
	        	newPhdr[oldPhdrs+i].p_align = 0x10000;//newSections[i].align;
	        	newPhdr[oldPhdrs+i].p_offset = newSections[i].shdr->sh_offset;
		}
	}	

        newPhdr[0].p_filesz += shiftSize;
        newPhdr[0].p_memsz += shiftSize;
        newPhdr[1].p_offset += shiftSize;
        newPhdr[2].p_filesz += shiftSize;
        newPhdr[2].p_memsz += shiftSize;
	if(oldLastPage == newSections[0].vaddr/pageSize){
		newPhdr[3].p_filesz =  newSections[0].vaddr - newPhdr[3].p_vaddr + newSections[0].dataSize;
		newPhdr[3].p_memsz =  newSections[0].vaddr- newPhdr[3].p_vaddr + newSections[0].dataSize;
	}
        newPhdr[3].p_offset += shiftSize;
        newPhdr[3].p_vaddr += shiftSize;
        newPhdr[4].p_offset += shiftSize;
        newPhdr[4].p_vaddr += shiftSize;
}


void writeBackElf::compactSections(vector <imageUpdate*> imagePatches, vector<imageUpdate*> &newPatches){

	int startPage, stopPage;
	imageUpdate *patch;




	imageUpdate *curr, *next;
	bool foundDup=true;
	unsigned int j;
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
		imagePatches.sort(imageUpdate::imageUpdateSort);
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
		imagePatches.sort(imageUpdate::imageUpdateSort);
	}

	unsigned int k=0;

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
			if(imagePatches[k]->startPage == stopPage){
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
