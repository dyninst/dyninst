/* $Id: addLibraryLinux.C,v 1.3 2002/03/18 19:17:47 chadd Exp $ */

#if defined(BPATCH_LIBRARY) && defined(i386_unknown_linux2_0)

#include "addLibraryLinux.h"

/* 
 * This class will add a library to an existing Linux ELF binary
 * You must pass it an open Elf * (from libelf) pointer and a library
 * name, and a name for the new file.
 *
 * How it works:
 * Basically the .dynstr section contains all the strings associated
 * with the dynamic symbol table and dynamic loading functionality (ie shared
 * libraries).  The .dynamic table contains a list of dynamic linking
 * information, including a list of pointers in to the .dynstr table
 * denoting which shared libraries should be loaded at startup.
 *
 * First we must add a string to .dynstr.  Since .dynstr is in the
 * middle of the text segment we cannot expand it as it will shift
 * all the following instructions to different addresses in memory
 * causing much chaos.  So we expand it and shift it up as many
 * bytes as we expand it, so it ends on the same offset within the
 * file.  There are no instructions above .dynstr so any movement up
 * there is ok.  The sections above .dynstr are all accessed through
 * the Program Header Table or .dynamic table and those pointers
 * can be patched up easily
 *
 * Of course the next question is how do you keep from shifting
 * up past the beginning of the file? Well and ELF file has as its
 * first 0x34 bytes the Elf Header.  This is a mandatory position.
 * Followed by that ELF files usually (ie if they are produced by
 * gcc) have their Program Header Table (usually 0xc0 bytes) then
 * the actual data follows that.  The Program Header Table can
 * be anywhere in the file (*).  So we shift it to either the
 * end of the text segment or the beginning of the data segment and
 * patch up the entries in the Program Header Table as needed.  
 * How do we know we can do this? There is always one memory page
 * between the end of the text segment and the beginnig of the data
 * segment.  So either there will be a gap of > 0xc0 bytes between
 * the end of the text segment and the next page boundry or the start
 * of the data segment and the previous page boundry.  Note that it
 * is possible but highly unlikely that the Program Header Table is
 * greater than 0xc0 bytes, though it would need to grow quite large
 * for this to be a problem.
 *
 * Note that all the assumptions except the placement of the
 * ELF Header are merely convention, not a standard.  But they are 
 * followed by gcc and the like.
 *
 * (*) The Program Header Table should be able to be located
 * anywhere in the file and loaded to any place in memory.
 * The Linux loader does not seem to like it if the PHT
 * is loaded anywhere except in the text or data segment.
 * I have not been able to find any code to support this,
 * its all experimental evidence
 */
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
	arraySize = oldEhdr->e_shnum;
	oldScn = NULL;
	phdrSize = oldEhdr->e_phentsize * oldEhdr->e_phnum;

	for(int cnt = 0; (oldScn = elf_nextscn(oldElf, oldScn));cnt++){
                oldData=elf_getdata(oldScn,NULL);
                oldShdr = elf32_getshdr(oldScn);

		newElfFileSec[cnt].sec_hdr = new Elf32_Shdr;
		newElfFileSec[cnt].sec_data = new Elf_Data;

                memcpy(newElfFileSec[cnt].sec_hdr, oldShdr, sizeof(Elf32_Shdr));
                memcpy(newElfFileSec[cnt].sec_data,oldData, sizeof(Elf_Data));

                if(oldData->d_buf && oldData->d_size){
                        newElfFileSec[cnt].sec_data->d_buf = new char[oldData->d_size];
                        memcpy(newElfFileSec[cnt].sec_data->d_buf, oldData->d_buf, oldData->d_size);
                }
		if(cnt + 1 == newElfFileEhdr->e_shstrndx) {
			strTabData = newElfFileSec[cnt].sec_data;
		}
	}

}

void addLibrary::updateDynamic(Elf_Data *newData){
/*
	for(unsigned int k =(newData->d_size/sizeof(Elf32_Dyn))-1 ; k>0;k--){
		memcpy( &(((Elf32_Dyn*) (newData->d_buf))[k]),&(((Elf32_Dyn*) (newData->d_buf))[k-1]),
				sizeof(Elf32_Dyn));	
	}
	((Elf32_Dyn*) (newData->d_buf))[0].d_tag = DT_NEEDED;
        ((Elf32_Dyn*) (newData->d_buf))[0].d_un.d_val = libnameIndx;

	//ok, if we do the above, and remove DT_NULL if below, we are reordering
	//the _DYNAMIC table, putting libdyninstAPI_RT.so in the first position
	//this reorders the load of the SO and moves it! once i get the
	//dlopen problem fixed then this will be handled ok.
*/	
        for(unsigned int counter=0;counter<newData->d_size/sizeof(Elf32_Dyn);counter++){

		if( 	((Elf32_Dyn*) (newData->d_buf))[counter].d_tag == DT_HASH ||
		 	((Elf32_Dyn*) (newData->d_buf))[counter].d_tag == DT_SYMTAB ||
		 	((Elf32_Dyn*) (newData->d_buf))[counter].d_tag == DT_STRTAB ) {
			((Elf32_Dyn*) (newData->d_buf))[counter].d_un.d_val -= libnameLen;
		}

		if( 	((Elf32_Dyn*) (newData->d_buf))[counter].d_tag == DT_STRSZ ){
			((Elf32_Dyn*) (newData->d_buf))[counter].d_un.d_val += libnameLen;
		}

                if( ((Elf32_Dyn*) (newData->d_buf))[counter].d_tag == DT_DEBUG){
                    	((Elf32_Dyn*) (newData->d_buf))[counter].d_un.d_val = 1;

		}
		if( ((Elf32_Dyn*) (newData->d_buf))[counter].d_tag == DT_NULL) {
		       	((Elf32_Dyn*) (newData->d_buf))[counter].d_tag = DT_NEEDED;
        		((Elf32_Dyn*) (newData->d_buf))[counter].d_un.d_val = libnameIndx;
			counter = newData->d_size/sizeof(Elf32_Dyn)+ 1;
		}
	}


}

int addLibrary::findSection(char* name){
	
	for(int cnt = 0 ; cnt < arraySize; cnt ++ ){
		if (!strcmp(name, (char *)strTabData->d_buf+newElfFileSec[cnt].sec_hdr->sh_name) ) {
			return cnt;
		}
	}
	return -1;
}


void addLibrary::updateProgramHeaders(Elf32_Phdr *phdr, unsigned int dynstrOffset){


	//update PT_PHDR
	phdr[0].p_offset = newPhdrOffset;
	phdr[0].p_vaddr =  newPhdrAddr ;
	phdr[0].p_paddr = phdr[0].p_vaddr;

	if(gapFlag == TEXTGAP){
		//update TEXT SEGMENT
		phdr[2].p_memsz += phdr[0].p_filesz;
		phdr[2].p_filesz += phdr[0].p_filesz;

	}else if(gapFlag == DATAGAP){
		//update DATA SEGMENT
		phdr[3].p_memsz += phdr[0].p_filesz;
		phdr[3].p_filesz += phdr[0].p_filesz;
		phdr[3].p_offset = newPhdrOffset;
		phdr[3].p_vaddr = newPhdrAddr;
		phdr[3].p_paddr = newPhdrAddr;

	}

	for(int i=0;i<newElfFileEhdr->e_phnum;i++){
		if(phdr[i].p_offset && phdr[i].p_offset < dynstrOffset){
			phdr[i].p_offset -= libnameLen;
			phdr[i].p_vaddr -= libnameLen;
			phdr[i].p_paddr -= libnameLen;
		}else if(phdr[i].p_type != PT_PHDR && phdr[i].p_vaddr > newPhdrAddr){
			phdr[i].p_offset +=_pageSize;
		}
	}

} 


void addLibrary::addStr(Elf_Data *newData,Elf_Data *oldData ,char* libname){

	newData->d_size += libnameLen;
	
	delete [] (char*) newData->d_buf;
	
        newData->d_buf = new char[newData->d_size];
        memcpy(newData->d_buf, oldData->d_buf,oldData->d_size);	

	memcpy(&(((char*) newData->d_buf)[newData->d_size-libnameLen]), libname, libnameLen);
	libnameIndx = newData->d_size-libnameLen;			

}

int addLibrary::writeNewElf(char* filename, char* libname){

	Elf32_Ehdr *realEhdr;
	Elf32_Phdr *realPhdr;
	Elf_Scn *realScn;
	Elf32_Shdr *realShdr;
	Elf_Data *realData, *strTabData;
	unsigned int dynstrOffset;

	int foundDynstr = 0;
	
	//open newElf
	//if the following call is to open() instead of creat()
	//and dyninst is compiled with gcc 3.0.[2|4] with -O3 
	//this file is written as all zeros and the permissions are 
	//odd ( u=--- g=--S o=r--)
        if((newFd = creat(filename,S_IRWXU)) == -1) { // open ( O_WRONLY|O_CREAT|O_TRUNC )) ==-1){
                printf("cannot creat: %s %d\n",filename,errno);
                switch(errno){
                case EACCES:
                        printf("EACCESS \n");
                        break;
                }
                return -1;
        }

        if((newElf = elf_begin(newFd, ELF_C_WRITE, NULL)) == NULL){
		printf(" elf_begin failed for newElf");
		return -1;
        }

	elf_flagelf(newElf,ELF_C_SET, ELF_F_LAYOUT);

	// ehdr
	realEhdr = elf32_newehdr(newElf);
        memcpy(realEhdr,newElfFileEhdr , sizeof(Elf32_Ehdr));

	// phdr
	realPhdr = elf32_newphdr(newElf, realEhdr->e_phnum);
	memcpy(realPhdr, newElfFilePhdr, realEhdr->e_phnum * realEhdr->e_phentsize);

	realEhdr ->e_phoff = newPhdrOffset;

	strTabData = newElfFileSec[findSection(".shstrtab")].sec_data; 
	//section data
	
	int pastPhdr = 0;
	
	for(int cnt = 0; cnt < newElfFileEhdr->e_shnum-1 ; cnt++){
		realScn = elf_newscn(newElf);
                realShdr = elf32_getshdr(realScn);
                realData = elf_newdata(realScn);

		// data
                memcpy(realShdr,newElfFileSec[cnt].sec_hdr, sizeof(Elf32_Shdr));
                memcpy(realData,newElfFileSec[cnt].sec_data, sizeof(Elf_Data));
                if(newElfFileSec[cnt].sec_data->d_buf && newElfFileSec[cnt].sec_data->d_size){
                        realData->d_buf = new char[newElfFileSec[cnt].sec_data->d_size];
                        memcpy(realData->d_buf, newElfFileSec[cnt].sec_data->d_buf, newElfFileSec[cnt].sec_data->d_size);
                }

		if(!foundDynstr){
			realShdr->sh_offset -= libnameLen;
		}

		if( !strcmp(".dynamic", (char *)strTabData->d_buf+realShdr->sh_name) ) {
			updateDynamic(realData);
		}
		
		if( !strcmp(".dynstr", (char *)strTabData->d_buf+realShdr->sh_name) ){
			dynstrOffset = realShdr->sh_offset;
			addStr(realData,newElfFileSec[cnt].sec_data, libname);
			realShdr->sh_size += libnameLen;
			foundDynstr = 1;
		}
		if( pastPhdr || realShdr->sh_addr >= newPhdrAddr){
			realShdr->sh_offset+=_pageSize;
			pastPhdr = 1;
		}

	}
	realEhdr ->e_shoff += _pageSize;	
	elf_update(newElf, ELF_C_NULL);

	updateProgramHeaders(realPhdr, dynstrOffset);
	
	elf_update(newElf, ELF_C_WRITE);
	return gapFlag;
}

//if we cannot edit the file (no space at the top, no space
//in the text/data gap etc) we return 0.
//otherwise we return 1 or 2, depending on if we use the
//text gap (1) or the data gap (2)
//
//If we cannot open the new file we return -1
int addLibrary::driver(Elf *elf,  char* newfilename, char *libname){

	libnameLen = strlen(libname) +1;
	oldElf = elf;	
	_pageSize = getpagesize();

	createNewElf();
	elf_end(elf);
	checkFile();
	if(gapFlag){
		findNewPhdrAddr();
		findNewPhdrOffset();
		
		gapFlag = writeNewElf(newfilename, libname);
		elf_end(newElf);
	}else{
		//error
	}
	return gapFlag;
}

addLibrary::addLibrary(){

	newElfFileSec = NULL;
	newElfFileEhdr = NULL;
	newElfFilePhdr = NULL;
}

addLibrary::~addLibrary(){
	if(newElfFileSec != NULL){
		
		for(int cnt = 0; cnt < newElfFileEhdr->e_shnum-1 ; cnt++){
			delete [] newElfFileSec[cnt].sec_hdr;
			if( newElfFileSec[cnt].sec_data->d_buf ){
				delete [] (char*) newElfFileSec[cnt].sec_data->d_buf ;
			}
			delete []  newElfFileSec[cnt].sec_data;
		}

		if(newElfFileEhdr){
			delete [] newElfFileEhdr;
		}
		if(newElfFilePhdr){
			delete [] (char*) newElfFilePhdr;
		}

		delete [] newElfFileSec;
	}

}

int addLibrary::findNewPhdrAddr(){
        Elf32_Shdr *tmpShdr;

        if(gapFlag == TEXTGAP){
                tmpShdr = newElfFileSec[findSection(".rodata")].sec_hdr;
                newPhdrAddr = tmpShdr->sh_addr + tmpShdr->sh_size;
        }else if(gapFlag == DATAGAP){
                tmpShdr = newElfFileSec[findSection(".data")].sec_hdr;
                newPhdrAddr = tmpShdr->sh_addr - phdrSize;
	}
        return 0;		
}

int addLibrary::findNewPhdrOffset(){
	Elf32_Shdr *tmpShdr;

	if(gapFlag == TEXTGAP){
		tmpShdr = newElfFileSec[findSection(".rodata")].sec_hdr;
		newPhdrOffset = tmpShdr->sh_offset + tmpShdr->sh_size;
	}else if(gapFlag == DATAGAP){
		tmpShdr = newElfFileSec[findSection(".data")].sec_hdr;
		newPhdrOffset = tmpShdr->sh_offset - phdrSize + _pageSize;
	}

	return 0;	
}

int addLibrary::checkFile(){
	int result=0;
        Elf32_Shdr *roDataShdr, *dataShdr;
        unsigned int endrodata, startdata;

	//is there space at the beginning?
	
	//must be space between 0x34 (end of the ehdr) and
	//the text segment start.
	
	if(newElfFileSec[0].sec_hdr->sh_offset < (unsigned int) (sizeof(Elf32_Ehdr)+libnameLen) ){
		//there is not enough room
		gapFlag = 0;
		return 0;
	}

        roDataShdr = newElfFileSec[findSection(".rodata")].sec_hdr;
        dataShdr = newElfFileSec[findSection(".data")].sec_hdr;

        endrodata = roDataShdr->sh_addr + roDataShdr->sh_size;

        startdata = dataShdr -> sh_addr;

        if( startdata - endrodata >= phdrSize ){

		//where to put the phdr?
		//find the gap between the text segment
		//end and the next page boundry
		unsigned int nextPage = endrodata >> 12;
		nextPage ++;
		nextPage = nextPage << 12;

		unsigned int textSideGap = nextPage - endrodata;

		//find the gap between the data segment start
		//and the previous page boundry
                unsigned int prevPage = startdata >> 12;
                prevPage = prevPage << 12;
		unsigned int dataSideGap = startdata - prevPage;

		if(textSideGap >= phdrSize ){ // i prefer the text side gap, no real reason
			result = TEXTGAP;
		}else if(dataSideGap >= phdrSize){
			result = DATAGAP;
		} 
        }else{
		result = 0;
        }
	gapFlag = result;

	return result;

}


#endif


