/* $Id: addLibraryLinux.C,v 1.2 2002/03/12 18:40:02 jaw Exp $ */

#if defined(BPATCH_LIBRARY) && defined(i386_unknown_linux2_0)
#include "addLibraryLinux.h"

void addLibrary::openOldElf(char *filename){


        if((oldFd = open(filename, O_RDONLY)) == -1){
                printf(" OLDELF_OPEN_FAIL ");
                return;
        }
        elf_version(EV_CURRENT);
        if ((oldElf = elf_begin(oldFd, ELF_C_READ, NULL)) == NULL){
                printf("OLDELF_BEGIN_FAIL");
                return;
        }

}


void addLibrary::createNewElf(){

	Elf32_Ehdr *oldEhdr;
	Elf32_Phdr *oldPhdr;
	Elf32_Shdr *oldShdr;
	Elf_Scn *oldScn;
	Elf_Data *oldData;

	// get Ehdr
	oldEhdr = elf32_getehdr(oldElf);

	newElfFileEhdr = (Elf32_Ehdr*) new char[oldEhdr->e_ehsize];
	memcpy(newElfFileEhdr, oldEhdr, oldEhdr->e_ehsize);

	// get Phdr
	oldPhdr = elf32_getphdr(oldElf);
	
	newElfFilePhdr = (Elf32_Phdr*) new char[oldEhdr->e_phentsize * oldEhdr->e_phnum];
	memcpy(newElfFilePhdr, oldPhdr, oldEhdr->e_phentsize * oldEhdr->e_phnum);

	newElfFileSec = (Elf_element *) new char[sizeof(Elf_element) * (oldEhdr->e_shnum + 1)];
	arraySize = oldEhdr->e_shnum + 1;
	oldScn = NULL;
	for(int cnt = 0; (oldScn = elf_nextscn(oldElf, oldScn));cnt++){
                oldData=elf_getdata(oldScn,NULL);
                oldShdr = elf32_getshdr(oldScn);

		newElfFileSec[cnt].sec_hdr = new Elf32_Shdr;
		newElfFileSec[cnt].sec_data = new Elf_Data;

                memcpy(newElfFileSec[cnt].sec_hdr, oldShdr, sizeof(Elf32_Shdr));
                memcpy(newElfFileSec[cnt].sec_data,oldData, sizeof(Elf_Data));

                if(oldData->d_buf && oldData->d_size){
                        newElfFileSec[cnt].sec_data->d_buf = malloc(oldData->d_size);
                        memcpy(newElfFileSec[cnt].sec_data->d_buf, oldData->d_buf, oldData->d_size);
                }
		if(cnt + 1 == newElfFileEhdr->e_shstrndx) {
			strTabData = newElfFileSec[cnt].sec_data;
		}
	}

}

void addLibrary::copySection(int oldInx, int newInx){

	newElfFileSec[newInx].sec_hdr = new Elf32_Shdr;
	newElfFileSec[newInx].sec_data = new Elf_Data;
	memcpy(newElfFileSec[newInx].sec_hdr, newElfFileSec[oldInx].sec_hdr, sizeof(Elf32_Shdr));
	memcpy(newElfFileSec[newInx].sec_data, newElfFileSec[oldInx].sec_data, sizeof(Elf_Data));
}

void addLibrary::shiftSections(int index, int distance){

	for(int cnt = arraySize - distance; cnt >= index; cnt--){
		newElfFileSec[cnt+distance].sec_hdr = newElfFileSec[cnt].sec_hdr;
		newElfFileSec[cnt+distance].sec_data = newElfFileSec[cnt].sec_data;
	}

}


void addLibrary::updateShLinks(int insertPt, int oldDynstr,  int shiftSize){

	for(int cnt = 0 ; cnt < newElfFileEhdr->e_shnum-1; cnt++){
		if(newElfFileSec[cnt].sec_hdr->sh_link > (unsigned) insertPt){
			newElfFileSec[cnt].sec_hdr->sh_link +=shiftSize;
		}
		if(newElfFileSec[cnt].sec_hdr->sh_link == (unsigned) oldDynstr){
			newElfFileSec[cnt].sec_hdr->sh_link = dynstr+1;
		}
	}
}

void addLibrary::updateDynamic(unsigned int dynstrAddr){
	Elf32_Dyn *dynData;
	int dynamic = findSection(".dynamic");

	for ( dynData = (Elf32_Dyn*) newElfFileSec[dynamic].sec_data->d_buf;dynData->d_tag!= DT_NULL;dynData++){
                if(dynData->d_tag == DT_STRTAB){
                        dynData->d_un.d_val=dynstrAddr;
                }
         }
}

void addLibrary::alignAddr(int index){
	while(newElfFileSec[index].sec_hdr->sh_addr %newElfFileSec[index].sec_hdr->sh_addralign !=0){
		newElfFileSec[index].sec_hdr->sh_addr ++;
	}
}

void addLibrary::moveDynstr(){

	int oldDynstr;
	int shiftSize = 1;

	oldDynstr = findSection(".dynstr");
        bss = findSection(".bss");
  
        strTab = newElfFileEhdr->e_shstrndx-1;
	shiftSections(bss + 1, shiftSize);
        if(strTab > bss){
 	       strTab +=shiftSize;
               newElfFileEhdr->e_shstrndx += shiftSize;
		strTabData = newElfFileSec[strTab].sec_data;
        }
	newElfFileEhdr->e_shnum += shiftSize;

	copySection(oldDynstr, bss + shiftSize);
	
        newElfFileSec[bss + 1].sec_hdr = new Elf32_Shdr;
        memcpy( newElfFileSec[bss + 1].sec_hdr, newElfFileSec[oldDynstr].sec_hdr, sizeof(Elf32_Shdr));
        newElfFileSec[oldDynstr].sec_hdr->sh_name = 0;

	dynstr = findSection(".dynstr");
	newElfFileSec[dynstr].sec_hdr->sh_addr = newElfFileSec[bss].sec_hdr->sh_addr +
		newElfFileSec[bss].sec_hdr->sh_size;
	alignAddr(dynstr);	
	newElfFileSec[oldDynstr].sec_hdr->sh_type = SHT_PROGBITS;
	updateShLinks(bss+1,oldDynstr+1, shiftSize);
	updateDynamic(newElfFileSec[dynstr].sec_hdr->sh_addr);

}

int addLibrary::findSection(char* name){
	
	for(int cnt = 0 ; cnt < arraySize; cnt ++ ){
		if (!strcmp(name, (char *)strTabData->d_buf+newElfFileSec[cnt].sec_hdr->sh_name) ) {
			return cnt;
		}
	}
	return -1;
}


int addLibrary::updateProgramHeaders(Elf32_Phdr *newFilePhdr, int sizeInc, int dynamicOffset, int dataOffset){ 
	Elf32_Phdr *phdr = newFilePhdr;
	int result=0;
	for(int cnt = 0;cnt < newElfFileEhdr->e_phnum;cnt++){

		if(phdr->p_offset == 0){
			//phdr->p_filesz += sizeInc;
			//phdr->p_memsz += sizeInc;
			phdr++;
			//phdr->p_offset = dataOffset ;
			phdr->p_filesz = phdr->p_memsz+sizeInc;
			phdr->p_memsz += sizeInc;	
			//phdr++;
			//phdr->p_offset = dynamicOffset;
			cnt = newElfFileEhdr->e_phnum;
			phdr --;
			if( ( phdr->p_vaddr - phdr->p_offset) %  phdr->p_align){
				result = ( phdr->p_vaddr - phdr->p_offset) %  phdr->p_align;
				printf(" NEED TO PAD %x\n", result);
			}
		}
		phdr++;
	}
	return result;
} 

void addLibrary::fix_end(Elf_Data *symData, Elf_Data *strData, int shiftSize){

	Elf32_Sym *symPtr = (Elf32_Sym*) symData->d_buf;

		
	for(unsigned int indx=0; indx < symData->d_size / sizeof(Elf32_Sym); indx++,symPtr++){
		if(!strcmp("_end", (char*) strData->d_buf + symPtr->st_name)){
			symPtr->st_value += shiftSize;
		}
	}


}


void addLibrary::writeNewElf(char* filename, char* libname){

	Elf32_Ehdr *realEhdr;
	Elf32_Phdr *realPhdr;
	Elf_Scn *realScn, *dynstrScn;
	Elf32_Shdr *realShdr, *realDynamicShdr, *realDataShdr, *realBssShdr, *realSbssShdr;
	Elf_Data *realData, *strTabData;

	//open newElf
        if((newFd = open(filename, O_WRONLY|O_CREAT|O_TRUNC )) ==-1){
                printf("cannot open: %s %d\n",filename,errno);
                switch(errno){
                case EACCES:
                        printf("EACCESS \n");
                        break;
                }
                exit(1);
        }

        if((newElf = elf_begin(newFd, ELF_C_WRITE, NULL)) == NULL){
		printf(" elf_begin failed for newElf");
		exit(1);
        }


	// ehdr
	realEhdr = elf32_newehdr(newElf);
        memcpy(realEhdr,newElfFileEhdr , sizeof(Elf32_Ehdr));

	// phdr
	realPhdr = elf32_newphdr(newElf, realEhdr->e_phnum);
	memcpy(realPhdr, newElfFilePhdr, realEhdr->e_phnum * realEhdr->e_phentsize);

	strTabData = newElfFileSec[findSection(".shstrtab")].sec_data; 
	//section data
	for(int cnt = 0; cnt < newElfFileEhdr->e_shnum-1 ; cnt++){
		realScn = elf_newscn(newElf);
                realShdr = elf32_getshdr(realScn);
                realData = elf_newdata(realScn);

		// data
                memcpy(realShdr,newElfFileSec[cnt].sec_hdr, sizeof(Elf32_Shdr));
                memcpy(realData,newElfFileSec[cnt].sec_data, sizeof(Elf_Data));
                if(newElfFileSec[cnt].sec_data->d_buf && newElfFileSec[cnt].sec_data->d_size){
                        realData->d_buf = malloc(newElfFileSec[cnt].sec_data->d_size);
                        memcpy(realData->d_buf, newElfFileSec[cnt].sec_data->d_buf, newElfFileSec[cnt].sec_data->d_size);
                }

		if( !strcmp(".dynamic", (char *)strTabData->d_buf+realShdr->sh_name) ) {
			realDynamicShdr = realShdr;
		}
		if( !strcmp(".data", (char *)strTabData->d_buf+realShdr->sh_name) ) {
			realDataShdr = realShdr;
		}
		if( !strcmp(".dynstr", (char *)strTabData->d_buf+realShdr->sh_name) ){
			dynstrScn = realScn;
		}
		if( 0&& !strcmp(".sbss", (char*)strTabData->d_buf+realShdr->sh_name) ){
			// .sbss is set as type SHT_PROGBITS so there should be
			//  something in the file if need be, no need to pad it
			if(realShdr->sh_size !=0 ){
				realData->d_buf = new char[realShdr->sh_size];
				realData->d_size = realShdr->sh_size;
				memset(realData->d_buf, '\0', realData->d_size);
				realSbssShdr = realShdr;
			}
		}
		if( !strcmp(".bss", (char*)strTabData->d_buf+realShdr->sh_name) ){
			realData->d_buf = new char[realShdr->sh_size];
			realData->d_size = realShdr->sh_size;
			memset(realData->d_buf, '\0', realData->d_size);
			realBssShdr = realShdr;
			realShdr->sh_type = SHT_PROGBITS;
		}
		if( !strcmp(".dynsym", (char*)strTabData->d_buf+realShdr->sh_name) ){
			fix_end(realData, newElfFileSec[realShdr->sh_link-1].sec_data,
					newElfFileSec[dynstr].sec_hdr->sh_size);
		}
		if( !strcmp(".symtab", (char*)strTabData->d_buf+realShdr->sh_name) ){
			fix_end(realData, newElfFileSec[realShdr->sh_link-1].sec_data,
					newElfFileSec[dynstr].sec_hdr->sh_size);
		}
	}
	elf_update(newElf, ELF_C_NULL);

	//int pad = 
	updateProgramHeaders(realPhdr, newElfFileSec[dynstr].sec_hdr->sh_size,
			     realDynamicShdr->sh_offset, realDataShdr->sh_offset);
/*	if(pad){
		realData = elf_newdata(dynstrScn);
		realData->d_buf = new char[pad]; 
		realData->d_size = pad;
		elf_update(newElf, ELF_C_NULL);
		updateProgramHeaders(realPhdr,0,realDynamicShdr->sh_offset,realDataShdr->sh_offset);
		elf_update(newElf, ELF_C_NULL);	
	}*/

	elf_update(newElf, ELF_C_WRITE);
}

int addLibrary::addStr(int indx, char* str){

	int result = 0;
	char * tmp = new char[newElfFileSec[indx].sec_data->d_size];
	memcpy(tmp, newElfFileSec[indx].sec_data->d_buf, newElfFileSec[indx].sec_data->d_size);

	delete [] (char*) newElfFileSec[indx].sec_data->d_buf;
	newElfFileSec[indx].sec_data->d_buf = new char[newElfFileSec[indx].sec_data->d_size + strlen(str) +1];
	result = newElfFileSec[indx].sec_data->d_size;

	memcpy(newElfFileSec[indx].sec_data->d_buf, tmp,result);
	memcpy(&(((char*)newElfFileSec[indx].sec_data->d_buf)[result]), str, strlen(str) + 1);
	delete [] tmp;
	newElfFileSec[indx].sec_data->d_size += (strlen(str) +1);
	newElfFileSec[indx].sec_hdr->sh_size = newElfFileSec[indx].sec_data->d_size;
	return result;
}

void addLibrary::addSO(int strInx){

	Elf_Data *newData = newElfFileSec[findSection(".dynamic")].sec_data;
	for(unsigned int counter=0;counter<newData->d_size/sizeof(Elf32_Dyn);counter++){
        	if( ((Elf32_Dyn*) (newData->d_buf))[counter].d_tag == DT_DEBUG){
                	((Elf32_Dyn*) (newData->d_buf))[counter].d_un.d_val = 1;
                }else if( ((Elf32_Dyn*) (newData->d_buf))[counter].d_tag == 0){//DT_CHECKSUM){
			((Elf32_Dyn*) (newData->d_buf))[counter].d_tag = DT_NEEDED;
                	((Elf32_Dyn*) (newData->d_buf))[counter].d_un.d_val = strInx;
			counter = newData->d_size/sizeof(Elf32_Dyn) + 1;
                }
        }

}

void addLibrary::addDynamicSharedLibrary(char *libname){

	int index = addStr(dynstr, libname);
	addSO(index);

}

void addLibrary::driver(Elf *elf /*char *filename*/, char* newfilename, char *libname){

	//openOldElf(filename);
	oldElf = elf;	
	createNewElf();
	moveDynstr();
	addDynamicSharedLibrary(libname);
	writeNewElf(newfilename, libname);
}

addLibrary::addLibrary(){

}

addLibrary::~addLibrary(){

}
#endif

