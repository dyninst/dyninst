/* $Id: addLibrary.C,v 1.2 2002/02/05 17:01:37 chadd Exp $ */

#if defined(BPATCH_LIBRARY) && defined(sparc_sun_solaris2_4)

#include "addLibrary.h"

unsigned int elf_version(unsigned int);

void addLibrary::updateDynamic(Elf_Data* dynamicData, unsigned int dynstrAddr, unsigned int dynsymAddr){

        __Elf32_Dyn *dynData;

        for ( dynData = (__Elf32_Dyn*) dynamicData->d_buf;  dynData->d_tag  != DT_NULL;dynData++){
                if(dynData->d_tag == DT_STRTAB){
                        dynData->d_un.d_val=dynstrAddr;
                }else if(dynData->d_tag == DT_SYMTAB){
                        dynData->d_un.d_val=dynsymAddr;
                }
         }


}

void addLibrary::remakeHash(Elf_Data* hashData, Elf_Data* dynsymData, Elf_Data* dynstrData){
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
        int oldhashValue, hashValue;
        char *currentName=NULL;
        while(counter<symSize){
                currentName = (char*) dynstrData->d_buf+ symPtr->st_name;
                hashValue = elf_hash(currentName) % nbucket;
                if(debugFlag){
                        printf("\n %s %d-->%lx", currentName,hashValue , bucket[hashValue]);
                }
                if(bucket[hashValue]==0){
                        /* success, we can put it here*/
                        bucket[hashValue] = counter;
                }else{
                        /* fail, cant put it here */
			oldhashValue = hashValue;
                        hashValue = bucket[oldhashValue];
                        while(((unsigned int) hashValue) <= ((unsigned int) symSize) && chain[hashValue]){
                                if(debugFlag){
                                        printf("inWHILE %d-->%lx ",hashValue,chain[hashValue]);
					fflush(stdout);
                                }
                      		oldhashValue = hashValue; 
		        	hashValue = chain[hashValue];
                        
			}
                        if((unsigned int) hashValue > symSize){
				hashValue = oldhashValue;
			}
			if(debugFlag){
                                printf("outWHILE %d-->%lx ",hashValue,chain[hashValue]);
                        }
                        chain[hashValue]=counter;
                }
                counter++;
                symPtr++;
        }
}

void addLibrary::updateRela(Elf_Data* newData,int shift, unsigned int shiftIndex){

        Elf32_Rela *rData;
        unsigned int newValue, rOldvalue, oldValue;

        rData= (Elf32_Rela*)newData->d_buf;
        for(unsigned int i = 0;i< newData->d_size/(sizeof(Elf32_Rela));i++,rData++){
                if(ELF32_R_SYM(rData->r_info) >= shiftIndex){
                        newValue = 0;
                        rOldvalue = 0;
                        oldValue = 0;
                        newValue = 0x000000ff & rData->r_info;
                        rOldvalue= rData->r_info;
                        oldValue = ELF32_R_SYM(rData->r_info);
                        oldValue +=shift;
                        newValue = newValue | (oldValue<<8);

                        rData->r_info = newValue;
                        if(debugFlag){
                                printf("RELA %x --> %lx  %d --> %lx\n",rOldvalue, rData->r_info,
                                        ELF32_R_SYM(rOldvalue),ELF32_R_SYM(rData->r_info));
                        }
                }
        }

}
void addLibrary::updateGOT(Elf_Data* GOTData,unsigned int dynamicAddr){
        ((Elf32_Rela*) GOTData->d_buf)->r_offset = dynamicAddr;
}
void addLibrary::updateSymbols(Elf_Data* symtabData, Elf_Data *oldsymData,
                       RelocationData relocData, int maxShndx){

        Elf32_Sym *symPtr=(Elf32_Sym*)symtabData->d_buf;
        Elf32_Sym *oldsymPtr= (Elf32_Sym*)oldsymData->d_buf;


        for(unsigned int i=0;i< symtabData->d_size/(sizeof(Elf32_Sym));i++,oldsymPtr++){

                if(i==relocData.newdynsymIndx){
                        oldsymPtr--;
                        memcpy(symPtr,oldsymPtr,sizeof(Elf32_Sym));
                        symPtr->st_shndx = relocData.newdynsymIndx;
                        symPtr->st_value = relocData.newdynsymAddr;
                        if(debugFlag){
                                printf("1: %d: %lx %d --> %lx %d \n",i,oldsymPtr->st_value, oldsymPtr->st_shndx,
                                        symPtr->st_value, symPtr->st_shndx);
                        }

                }else if(i==relocData.newdynstrIndx){
                        oldsymPtr--;
                        memcpy(symPtr,oldsymPtr,sizeof(Elf32_Sym));
                        symPtr->st_shndx = relocData.newdynstrIndx;
                        symPtr->st_value = relocData.newdynstrAddr;
                        if(debugFlag){
                                printf("2: %d: %lx %d --> %lx %d \n",i,oldsymPtr->st_value, oldsymPtr->st_shndx,
                                        symPtr->st_value, symPtr->st_shndx);
                        }

                }else if(i==relocData.newdynamicIndx){
                        oldsymPtr--;
                        memcpy(symPtr,oldsymPtr,sizeof(Elf32_Sym));
                        symPtr->st_shndx = relocData.newdynamicIndx;
                        symPtr->st_value = relocData.newdynamicAddr;
                        if(debugFlag){
                                printf("3: %d: %lx %d --> %lx %d \n",i,oldsymPtr->st_value, oldsymPtr->st_shndx,
                                        symPtr->st_value, symPtr->st_shndx);
                        }

                }else{
                        memcpy(symPtr,oldsymPtr,sizeof(Elf32_Sym));
                        if(symPtr->st_shndx < maxShndx){
                                if(symPtr->st_shndx>=relocData.newdynstrIndx &&
                                        symPtr->st_shndx<relocData.newdynamicIndx-2){
                                        symPtr->st_shndx+=2;
                                }else if(symPtr->st_shndx>=relocData.newdynamicIndx-2){
                                        symPtr->st_shndx+=3;
                                }
                        }

                        if(debugFlag){
                                printf("4: %d: %lx %d --> %lx %d \n",i,oldsymPtr->st_value, oldsymPtr->st_shndx,
                                        symPtr->st_value, symPtr->st_shndx);
                        }
                }
                if(oldsymPtr->st_shndx==relocData.olddynamicIndx){
                        symPtr->st_shndx=relocData.newdynamicIndx;
                }
                if(oldsymPtr->st_value == relocData.olddynamicAddr){
                        symPtr->st_value = relocData.newdynamicAddr;
                }
                if(oldsymPtr->st_value == relocData.olddynstrAddr){
                        symPtr->st_value = relocData.newdynstrAddr;
                }else if(oldsymPtr->st_value == relocData.olddynsymAddr){
                        symPtr->st_value = relocData.newdynsymAddr;
                }
                symPtr++;
        }


}
void addLibrary::updateSh_link(Elf32_Word *sh_link,RelocationData relocData){

        if(debugFlag){
                printf("UPDATES SHLINK %lx \n", (*sh_link));
        }
        if(relocData.newdynstrIndx && *sh_link>relocData.newdynstrIndx){
                (*sh_link)++;
        }

        if(relocData.newdynsymIndx && *sh_link>relocData.newdynsymIndx){
                (*sh_link)++;
        }

        if(relocData.newdynamicIndx&& *sh_link>relocData.newdynamicIndx){
                (*sh_link)++;
        }
        if(debugFlag){
                printf("UPDATED SHLINK: %lx %d %d\n", *sh_link,relocData.newdynstrIndx,relocData.newdynsymIndx);
        }

}

void addLibrary::updateSymTab(Elf_Data* symData, RelocationData relocData, int shift,
                        int max, Elf_Data*symstrPtr){


        Elf32_Sym* symPtr;
        int  count = symData->d_size / sizeof(Elf32_Sym);
        symPtr = (Elf32_Sym*) symData->d_buf;

        for(int i = 1; i<count; i++){

                if(symPtr->st_shndx == relocData.olddynsymIndx){
                        symPtr->st_shndx = relocData.newdynsymIndx;
                }else if(symPtr->st_shndx == relocData.olddynstrIndx){
                        symPtr->st_shndx = relocData.newdynstrIndx;
                }else if(symPtr->st_shndx > relocData.newdynstrIndx-1 && symPtr->st_shndx < max){
                        symPtr->st_shndx+=shift;
                }

                if(debugFlag){
                        printf(" SYMBOL %s\t\t%x\t%d\n", (char*) symstrPtr->d_buf+ symPtr->st_name,
                                symPtr->st_other, symPtr->st_shndx);
                }
                symPtr++;
        }

}
void addLibrary::updateSectionHdrs(Elf * newElf){
Elf_Scn *scn;
Elf32_Shdr *shdr;
int index;
int fix,newAddr;
        for (scn = NULL; (scn = elf_nextscn(newElf, scn)); ) {
                shdr = elf32_getshdr(scn);
                index = 0;
                newAddr = shdr->sh_addr;
                while(shdr->sh_addr >= relocations[index].d_addr && index<relocIndex){
                        newAddr += relocations[index].d_size;
                        fix=0;
                        if(newAddr % shdr->sh_addralign){
                                while((newAddr+ ++fix) % shdr->sh_addralign);
                                if(index == relocIndex){
                                        relocations[relocIndex].d_size = fix;
                                        relocations[relocIndex++].d_addr = newAddr;
                                        index++;
                                }else{

                                        int shift=relocIndex;
                                        for(;shift>0;shift--){
                                                memcpy(&(relocations[shift]), &(relocations[shift-1]),8);
                                        }
                                        relocations[index+1].d_size = fix;
                                        relocations[index+1].d_addr = newAddr;
                                        index++;
                                        relocIndex++;
                                        for(int xx=0;xx<relocIndex;xx++){
                                                if(debugFlag){
                                                        printf(" [ %lx %lx ] \n",relocations[xx].d_size, 
								relocations[xx].d_addr);
                                                }
                                        }
                                }
                                //relocations[index].d_size+=fix;
                                newAddr+=fix;
                        }
                        if(debugFlag){
                                printf(" ADDING %x->%lx %lx\n", newAddr,shdr->sh_addr,relocations[index].d_size);
                        }
                        index++;
                }
                shdr->sh_addr = newAddr;
        }
}
void addLibrary::updateProgramHdrs(Elf *newElf,int size, unsigned int gotOffset, Elf32_Word dynamicAddr,
            Elf32_Word dynamicOffset, int bssSize,int dynamicSize, int dynSize){

	int index;
        Elf32_Phdr *phdr,*phdrPtr;

        phdr= elf32_getphdr(newElf);

        phdr[2].p_filesz+=size;
        phdr[2].p_memsz+=size;

        if(gotOffset){
                phdr[3].p_offset = (gotOffset>dynamicOffset? dynamicOffset: gotOffset);
                //gotAddr;//dynamicOffset;//gotAddr;
        }
        if(dynamicAddr){
                phdr[4].p_vaddr = dynamicAddr;
                phdr[4].p_paddr = dynamicAddr;
                phdr[4].p_offset = dynamicOffset;
                phdr[4].p_filesz = dynamicSize;
        }
        if(debugFlag){
                printf(" updateProgramHdrs %x %x %lx %lx %x %x\n", size, gotOffset, dynamicAddr,
                        dynamicOffset, bssSize,  dynamicSize);
        }
        phdrPtr = &phdr[5];
        for( ; phdrPtr->p_type != PT_NULL; phdrPtr++){
                phdrPtr->p_offset += (dynSize);
        }
        int cnt=0;
        for ( ;  phdr->p_type != PT_NULL;phdr++){
                index = 0;
                int newAddr = phdr->p_vaddr;
                while(phdr->p_vaddr >= relocations[index].d_addr && index<relocIndex){
                        newAddr += relocations[index++].d_size;
                        if(debugFlag){
                                printf("%d ADDING PHDR %lx %lx %lx\n",cnt, phdr->p_offset,phdr->p_vaddr,
					relocations[index-1].d_size);
                        }
                }
                phdr->p_vaddr = newAddr;
                cnt++;
        }

}

void addLibrary::padData(Elf32_Shdr* newDataShdr, Elf32_Shdr* newDynamicShdr, Elf_Scn *dynSec, 
		Elf_Data* newDataData){

        if(newDynamicShdr->sh_addr + newDynamicShdr->sh_size != newDataShdr->sh_addr){

		newDataShdr ->sh_addralign /=2;
		newDataData->d_align /= 2;	

		Elf_Data* padData = elf_newdata(dynSec);
		padData->d_buf = new char[4];
		padData->d_size = 4;
		((char*) padData->d_buf)[0]=0x4a;
                ((char*) padData->d_buf)[1]=0x65;
                ((char*) padData->d_buf)[2]=0x66;
                ((char*) padData->d_buf)[3]=0x66;
		elf_update(newElf, ELF_C_NULL);
	}
}

void addLibrary::driver(){

        Elf_Data* newDataData;
        Elf_Scn *newScnDynamic;
        Elf32_Shdr *newShdrDynamic=NULL,*newDataShdr,*GOTShdr;
        Elf_Scn * scn,*newScn;
        Elf_Data *     data,*newData;
        Elf32_Shdr *   shdr,*newShdr;
	int cnt;
        unsigned int pageSize = getpagesize();


	newEhdr = elf32_newehdr(newElf);
        memcpy(newEhdr, ehdr, sizeof(Elf32_Ehdr));

        for(cnt = 0, scn = NULL; (scn = elf_nextscn(oldElf, scn));cnt++){
                data=elf_getdata(scn,NULL);
                shdr = elf32_getshdr(scn);

                newScn = elf_newscn(newElf);
                newShdr = elf32_getshdr(newScn);
                newData = elf_newdata(newScn);

                memcpy(newShdr, shdr, sizeof(Elf32_Shdr));
                memcpy(newData,data, sizeof(Elf_Data));
                if(data->d_buf && data->d_size){
                        if(debugFlag){
                                printf(" COPYING %s \n", (char *)strData->d_buf + shdr->sh_name);
                        }
                        newData->d_buf = malloc(data->d_size);
                        memcpy(newData->d_buf, data->d_buf, data->d_size);
                }
                if( !strcmp((char *)strData->d_buf + shdr->sh_name, ".symtab")){
                        symtabData = newData;
                        delete [] (char*)newData->d_buf;
                        newData->d_buf = malloc(newData->d_size+ 3*(sizeof(Elf32_Sym)));
                        newData->d_size +=3*(sizeof(Elf32_Sym));
                        memset(newData->d_buf,'\0',newData->d_size);
                        sizeChange += 3*(sizeof(Elf32_Sym));
                        oldsymtabData = data;
                        updateSh_link(&(newShdr->sh_link),relocData);//was 2*
                        newShdr->sh_link--;//keep this???
                }

                if( !strncmp((char *)strData->d_buf+shdr->sh_name,"dyninstAPIhighmem_",18)){
                        //this needs to be aligned on the pageSize
                        elf_update(newElf, ELF_C_NULL);
                        int pageFix = newShdr->sh_offset % pageSize;
                        pageFix = pageSize - pageFix; // how much do we need to move
                                                        // the data so it is on a page boundry?
                        newShdr->sh_size +=pageFix;
                        newData->d_size += pageFix;
                        char* tmpHighMem = (char*) malloc(newData->d_size);
                        memset(tmpHighMem, '\0', newData->d_size);
                        memcpy( &(tmpHighMem[pageFix]), newData->d_buf, newData->d_size - pageFix);
                        delete [] (char*) newData->d_buf;
                        newData->d_buf = tmpHighMem;
                }
                if( !strcmp((char *)strData->d_buf + shdr->sh_name, ".strtab")){
                        symstrData = newData;
                }
                if( !strcmp((char *)strData->d_buf + shdr->sh_name, ".got")){
                        GOTData = newData;
                        GOTShdr = newShdr;
                }
		if( !strcmp((char *)strData->d_buf + shdr->sh_name, ".shstrtab")){
			newStrData = newData;
		}
                if( !strcmp((char *)strData->d_buf + shdr->sh_name, ".SUNW_version")){
                        newShdr->sh_link = relocData.newdynstrIndx;
                }
                if( !strcmp((char *)strData->d_buf + shdr->sh_name, ".dynstr")){
                        dynstrData = data;
                        dynstrShdr = shdr;
                        newShdr->sh_name=0;
                        newShdr->sh_type= SHT_PROGBITS;//  SHT_NOTE;
                        memset(newData->d_buf, '\0',newData->d_size);
                        dynstrOffset= newData->d_size;
                }
                if( !strcmp((char *)strData->d_buf + shdr->sh_name, ".dynsym")){
                        dynsymData=data;
                        olddynsymData = data;
                        dynsymShdr = newShdr;
                        memset(newData->d_buf, '\0',newData->d_size);
                }
                if(!strcmp((char *)strData->d_buf + shdr->sh_name, ".dynamic")){
                        dynamicDataNew = newData;
			newShdrDynamic = newShdr;
                        newShdr->sh_link = relocData.newdynstrIndx;
                        relocData.newdynamicIndx = cnt+1;
                        relocData.newdynamicAddr = newShdr->sh_addr;
                        for(unsigned int counter=0;counter<newData->d_size/sizeof(__Elf32_Dyn);counter++){
                                if( ((__Elf32_Dyn*) (newData->d_buf))[counter].d_tag == DT_DEBUG){
                                        ((__Elf32_Dyn*) (newData->d_buf))[counter].d_un.d_val = 1;
                                }else if( ((__Elf32_Dyn*) (newData->d_buf))[counter].d_tag == DT_CHECKSUM){
                                        ((__Elf32_Dyn*) (newData->d_buf))[counter].d_tag = DT_NEEDED;
                                        ((__Elf32_Dyn*) (newData->d_buf))[counter].d_un.d_val = dynstrOffset;
                                }
                        }
               		newScnDynamic = newScn; 
		}
                if( !strcmp((char *)strData->d_buf + shdr->sh_name, ".hash")){
                        newShdr->sh_link=relocData.newdynsymIndx;
                        hashData= newData;
                }
                if( !strcmp((char*)strData->d_buf+shdr->sh_name,".rela.got")){
                        newShdr->sh_link=relocData.newdynsymIndx;
                        updateSh_link(&newShdr->sh_info,relocData);
                        updateRela(newData,3,relocData.newdynstrIndx);
                }
                if( !strcmp((char*)strData->d_buf+shdr->sh_name,".rela.bss")){
                        newShdr->sh_link=relocData.newdynsymIndx;
                        updateSh_link(&newShdr->sh_info,relocData);
                        updateRela(newData,3,relocData.newdynstrIndx);
                }

                if( !strcmp((char*)strData->d_buf+shdr->sh_name,".rela.plt")){
                        newShdr->sh_link=relocData.newdynsymIndx;
                        updateSh_link(&newShdr->sh_info,relocData);
                        updateRela(newData,3,relocData.newdynstrIndx);
                }
                if( !strcmp((char*)strData->d_buf+shdr->sh_name,".text")){
                        textData = newData;
                }

               	if( !strcmp((char*)strData->d_buf+shdr->sh_name,".data")){
                        elf_update(newElf,ELF_C_NULL);
    			newDataShdr = newShdr;
			newDataData = newData; 
 
		}

                if( !strcmp((char *)strData->d_buf + shdr->sh_name, ".rodata")){
                        if(dynstrData){

                                newScn = elf_newscn(newElf);
                                newShdr = elf32_getshdr(newScn);
                                newData = elf_newdata(newScn);
                                memcpy(newShdr, dynstrShdr, sizeof(Elf32_Shdr));
                                memcpy(newData,dynstrData, sizeof(Elf_Data));
                                if(debugFlag){
                                        printf("MOVING SECTION  %s %lx %lx %lx\n",(char *)strData->d_buf + shdr->sh_name,
                                        shdr->sh_offset, shdr->sh_size, shdr->sh_addr );
                                }
	                        /* insert lib */
                        	char *tmpBuf = (char*) malloc(newData->d_size+strlen(libraryName)+1);
                        	memcpy(tmpBuf, dynstrData->d_buf, dynstrData->d_size);
                        	memcpy(&(tmpBuf[dynstrData->d_size]), libraryName, strlen(libraryName)+1);
                        	delete [] (char*) newData->d_buf;
                        	newData->d_buf = tmpBuf;
                        	newData->d_size +=strlen(libraryName)+1;
                        	newShdr->sh_size+=strlen(libraryName)+1;
                        	if(debugFlag){
                                	printf("NEW RELOCATION %lx %lx \n", relocations[relocIndex-1].d_addr,
                                        	relocations[relocIndex-1].d_size);
                        	}


                                sizeChange += newShdr->sh_size;
                                newEhdr->e_shstrndx++;
                                newEhdr->e_shnum++;
                                relocData.newdynstrAddr = shdr->sh_addr + shdr->sh_size;
                                newShdr->sh_addr = relocData.newdynstrAddr;
                                dynstrData = newData;
                                dynstrShdr->sh_type  = SHT_PROGBITS ;// SHT_NOTE;



                        }

                        if(dynsymData){
                                relocData.newdynsymAddr = relocData.newdynstrAddr+newShdr->sh_size;
                                newScn = elf_newscn(newElf);
                                newShdr = elf32_getshdr(newScn);
                                newData = elf_newdata(newScn);
                                while(relocData.newdynsymAddr % dynsymShdr->sh_addralign !=0){
                                        relocData.newdynsymAddr++;
                                }
                                memcpy(newShdr, dynsymShdr, sizeof(Elf32_Shdr));
                                memcpy(newData,dynsymData, sizeof(Elf_Data));
                                dynsymShdr->sh_name = 0;
                                if(debugFlag){
                                        printf("MOVING SECTION %s %lx %lx %lx\n",(char *)strData->d_buf + shdr->sh_name,
                                        shdr->sh_offset, shdr->sh_size, shdr->sh_addr );
                                }
                                newEhdr->e_shstrndx++;
                                newEhdr->e_shnum++;
                                newShdr->sh_addr = relocData.newdynsymAddr;
                                newShdr->sh_link = relocData.newdynstrIndx;
                                dynsymShdr->sh_type = SHT_PROGBITS;// SHT_NOTE ;
                                dynsymShdr->sh_flags = 1;

                                newData->d_buf = malloc(dynsymData->d_size + 3*(sizeof(Elf32_Sym)));
                                newData->d_size +=3*(sizeof(Elf32_Sym));
                                memset(newData->d_buf,'\0',newData->d_size);
                                dynsymData = newData;
                                newShdr->sh_size += 3*(sizeof(Elf32_Sym));
                                sizeChange += newShdr->sh_size;


                        }
                }



        }


	elf_update(newElf, ELF_C_NULL);
	
        Elf32_Phdr *phdr;
        Elf32_Phdr *tmp;
        int dynamicOffset;
        tmp = elf32_getphdr(oldElf);
        phdr=elf32_newphdr(newElf,ehdr->e_phnum);

        memcpy(phdr, tmp, ehdr->e_phnum * ehdr->e_phentsize);
        updateSectionHdrs(newElf);
        if(relocData.newdynstrAddr && relocData.newdynsymAddr){
	        if(debugFlag){
                        printf("UPDATING DYNAMIC");
                }
                updateDynamic(dynamicDataNew, relocData.newdynstrAddr, relocData.newdynsymAddr);
        }
        updateSymTab(symtabData,relocData, 2, newEhdr->e_shnum-2,symstrData);
	
        elf_update(newElf, ELF_C_NULL);
        for (scn = NULL,cnt=0; (scn = elf_nextscn(newElf, scn)); cnt++) {
                shdr=elf32_getshdr(scn);
                if(!strcmp(".dynamic", (char *)strData->d_buf + shdr->sh_name)){
                        dynamicOffset = shdr->sh_offset;
                }else if(!strcmp(".got",(char *)strData->d_buf + shdr->sh_name )){
                        gotAddr = shdr->sh_offset;
                }
        }
        if(debugFlag){
                printf("::size change: %x gotAddr: %x dynamicAddr: %lx\n", sizeChange, gotAddr, relocData.newdynamicAddr);
                printf("oldynanmicIndx %x newdynamicIndx %x oldynamicAddr %lx newdynamicAddr %lx\n",
                relocData.olddynamicIndx, relocData.newdynamicIndx, relocData.olddynamicAddr, relocData.newdynamicAddr);
        }
        updateProgramHdrs(newElf,sizeChange,gotAddr, relocData.newdynamicAddr,dynamicOffset,bssSize,
                dynamicDataNew->d_size, dynsymData->d_size + dynstrData->d_size);
        updateSymbols(dynsymData, olddynsymData,relocData,newEhdr->e_shnum-1);
        updateSymbols(symtabData, oldsymtabData,relocData,newEhdr->e_shnum-1);
        remakeHash(hashData, dynsymData, dynstrData);
        updateGOT(GOTData,relocData.newdynamicAddr);

        elf_update(newElf, ELF_C_NULL);

}

void addLibrary::parseOldElf(){

	Elf_Scn * scn;
	Elf_Data *data;
	Elf32_Shdr *shdr;
        ehdr = elf32_getehdr(oldElf);
        scn = elf_getscn(oldElf, ehdr->e_shstrndx);
        strData = elf_getdata(scn,NULL);

        relocations=(addrReloc*) malloc(sizeof(addrReloc) * 100);
	int cnt;
        for(cnt = 0, scn = NULL; (scn = elf_nextscn(oldElf, scn));cnt++){
                data=elf_getdata(scn,NULL);
                shdr = elf32_getshdr(scn);
                if(!strcmp((char *)strData->d_buf + shdr->sh_name, ".dynamic")){
                        relocData.olddynamicIndx = cnt+1;
                        relocData.olddynamicAddr = shdr->sh_addr;
                }
                if(!strcmp((char *)strData->d_buf + shdr->sh_name, ".dynstr")){
                        relocData.olddynstrIndx = cnt+1;
                        relocData.olddynstrAddr = shdr->sh_addr;
                }
                if(!strcmp((char *)strData->d_buf + shdr->sh_name, ".dynsym")){
                        relocData.olddynsymIndx = cnt+1;
                        relocData.olddynsymAddr = shdr->sh_addr;
                }

                if(!strcmp((char *)strData->d_buf + shdr->sh_name, ".rodata")){
                        relocData.newdynstrIndx = cnt+2;
                        relocData.newdynsymIndx = cnt+3;

                }
                if(!strcmp((char *)strData->d_buf + shdr->sh_name, ".got")){
                        relocData.newdynamicIndx = cnt+2;
                        if(relocData.newdynsymIndx && relocData.newdynamicIndx>relocData.newdynsymIndx){
                                relocData.newdynamicIndx++;
                        }
                        if(relocData.newdynstrIndx && relocData.newdynamicIndx>relocData.newdynstrIndx){
                                relocData.newdynamicIndx++;
                        }
                }
        }

}


addLibrary::addLibrary(char*oldElfName, char* libname, int debugOutputFlag){
	
	int oldfd;


        if((oldfd = open(oldElfName, O_RDONLY)) == -1){
                printf(" OLDELF_OPEN_FAIL ");
                return;
        }
        elf_version(EV_CURRENT);
        if ((oldElf = elf_begin(oldfd, ELF_C_READ, NULL)) == NULL){
                printf("OLDELF_BEGIN_FAIL");
                return;
        }
        debugFlag = debugOutputFlag;
        libraryName = new char[strlen(libname)+1];
        memcpy(libraryName, libname, strlen(libname)+1);
        relocations = NULL;
        relocIndex = 0;
        dynamicData = NULL;
        dynstrData = NULL;
        sizeChange = 0;
        dynstrOffset = 0;
	elf_fill(0);

        parseOldElf();
	 
}

addLibrary::addLibrary(Elf *oldElfPtr, char *libname, int debugOutputFlag){
	debugFlag = debugOutputFlag; 
	libraryName = new char[strlen(libname)+1];
	memcpy(libraryName, libname, strlen(libname)+1);

	oldElf = oldElfPtr;

	relocations = NULL;
	relocIndex = 0;
	dynamicData = NULL;
	dynstrData = NULL;
	sizeChange = 0;
	dynstrOffset = 0;
	elf_fill(0);
	
	parseOldElf();
}	

addLibrary::~addLibrary(){
	delete [] libraryName;	
	delete [] relocations;
}


bool addLibrary::outputElf(char* filename){

	//open newElf
	int fd2; 
	if((fd2 = open(filename, O_WRONLY|O_CREAT|O_TRUNC )) ==-1){
               	printf("cannot open: %s %d\n",filename,errno);
                switch(errno){
 	       	case EACCES:
                        printf("EACCESS \n");
                        break;
                }
                exit(1);
        }

        if((newElf = elf_begin(fd2, ELF_C_WRITE, NULL)) == NULL){
		printf(" elf_begin failed for newElf");
		exit(1);
        }
	driver();
	
	//verifyNewFile();
	//write file	
	return writeOutNewElf();
}


void addLibrary::verifyNewFile(){

	unsigned int oldAddr, oldOff, oldSize;
	bool afterDynsym = false;
	unsigned int lastPage, thisPage;
	Elf_Scn *scn, *prevScn;
	Elf32_Shdr *shdr, *prevShdr;
	unsigned int pageSize = getpagesize();
	Elf_Data *data;


	scn=elf_nextscn(newElf,NULL);
	shdr = elf32_getshdr(scn);
	oldAddr = shdr->sh_addr;
	oldOff = shdr->sh_offset;
	oldSize = shdr->sh_size;
	prevScn = scn;
	prevShdr = shdr;

        for(; (scn = elf_nextscn(newElf, scn));){
                shdr = elf32_getshdr(scn);	
		lastPage = (oldAddr + oldSize ) - ((oldAddr + oldSize )%pageSize);
		thisPage = shdr->sh_addr - (shdr->sh_addr % pageSize);
		if( afterDynsym && (shdr->sh_flags & 0x2 ) && (lastPage == thisPage)) {
			if(oldAddr + oldSize != shdr->sh_addr){
				data = elf_getdata(scn,NULL);
				padData(shdr, prevShdr, prevScn, data);

			}
		}
	        oldAddr = shdr->sh_addr;
       		oldOff = shdr->sh_offset;
        	oldSize = shdr->sh_size;
		prevShdr = shdr;
		prevScn = scn;
		if( !strcmp((char *)newStrData->d_buf + shdr->sh_name, ".dynsym")){
			afterDynsym = true;
		}

	}
}

 
bool addLibrary::writeOutNewElf(){

	verifyNewFile();
        elf_update(newElf, ELF_C_WRITE);
        return !elf_end(newElf);

}

	
Elf* addLibrary::getNewElf(){

	return newElf;
}
#endif
