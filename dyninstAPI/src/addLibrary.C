/* -*- Mode: C; indent-tabs-mode: true -*- */
// Since the author of this file chose to use tabs instead of spaces
// for the indentation mode, the above line switches users into tabs
// mode with emacs when editing this file.
/* $Id: addLibrary.C,v 1.10 2003/09/05 16:28:32 schendel Exp $ */


#if defined(sparc_sun_solaris2_4)
#include "addLibrary.h"
#define BASEADDR 0x10000


/* 
 * This class will add a library to an existing Sparc ELF binary
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
 * UPDATE: 17 may 2002
 * Previously it was assumed there would be a DT_CHECKSUM entry
 * in the dynamic table.  this is not always the case. some
 * compilers dont put this entry in.  If it is not present there
 * is no place to put the link to the dyninstAPI runtime shared
 * library and everything fails.  
 *
 * UPDATE: 25 june 2003
 * corrected moveDynamic() to move the .dynamic section to the end
 * of the text segment
 * corrected fixUpPhdrForDynamic() to reflect the above change and
 * make the text segment read/write/execute, instead of just read/execute,
 * because the .dynamic section gets updated by the loader
 * 
 * UPDATE: 26 june 2003
 * Now the TEXTGAP/DATAGAP test determines if there is enough room for
 * both the PHT and the .dynamic section in the gap.  
 *
 * UPDATE: 27 june 2003
 * There is a gap of 0x10000 (+ some padding for alignment) bytes between
 * the end of the text segment and the start of the data segment in MEMORY.
 * In the original ELF file, the OFFSET for the last byte of the last section
 * in the text segment and the first byte of the first section in the data section 
 * are different by 0 to 0x10 bytes (alignment padding).  In the NEW ELF file,
 * the data segment is shifted down IN THE FILE 0x10000 bytes, keeping the 
 * 0x0FFFF part of the offset the same so it can be loaded correctly to memory
 * in the specified location.
 * This means that there is ALWAYS room to put the PHT and .dynamic section
 * immediately after the text segment.  The notion of the TEXT and DATA gap
 * is not important!  
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
	memset(newElfFileSec, '\0',sizeof(Elf_element) * (oldEhdr->e_shnum + 1)); 
	arraySize = oldEhdr->e_shnum;

	oldScn = NULL;
	_pageSize = newElfFilePhdr[3].p_align;
	phdrSize = oldEhdr->e_phentsize * oldEhdr->e_phnum;
	numberExtraSegs=0;
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

void addLibrary::updateDynamic(Elf_Data *newData, unsigned int hashOff, unsigned int dynsymOff,
	unsigned int dynstrOff){

	char *d_buf = new char[newData->d_size +sizeof(Elf32_Dyn) ];

	memset(d_buf, '\0', newData->d_size +sizeof(Elf32_Dyn) ); // make them all DT_NULL
	memcpy(d_buf, newData->d_buf, newData->d_size );

	delete [] newData->d_buf;
	newData->d_buf = d_buf;
	newData->d_size += sizeof(Elf32_Dyn);
	
        for(unsigned int counter=0;counter<newData->d_size/sizeof(Elf32_Dyn);counter++){

		if( 	((Elf32_Dyn*) (newData->d_buf))[counter].d_tag == DT_HASH ){
			 ((Elf32_Dyn*) (newData->d_buf))[counter].d_un.d_val = hashOff;
		}
		if( ((Elf32_Dyn*) (newData->d_buf))[counter].d_tag == DT_SYMTAB ){
			 ((Elf32_Dyn*) (newData->d_buf))[counter].d_un.d_val = dynsymOff;
		}
		if( ((Elf32_Dyn*) (newData->d_buf))[counter].d_tag == DT_STRTAB ) {
			((Elf32_Dyn*) (newData->d_buf))[counter].d_un.d_val = dynstrOff;
		}

		if( 	((Elf32_Dyn*) (newData->d_buf))[counter].d_tag == DT_STRSZ ){
			((Elf32_Dyn*) (newData->d_buf))[counter].d_un.d_val += libnameLen;
		}

                if( ((Elf32_Dyn*) (newData->d_buf))[counter].d_tag == DT_DEBUG){
                    	((Elf32_Dyn*) (newData->d_buf))[counter].d_un.d_val = 1;

		}
		if( ((Elf32_Dyn*) (newData->d_buf))[counter].d_tag == DT_NULL) { /* was DT_CHECKSUM */
		       	((Elf32_Dyn*) (newData->d_buf))[counter].d_tag = DT_NEEDED;
        		((Elf32_Dyn*) (newData->d_buf))[counter].d_un.d_val = libnameIndx;
			counter = newData->d_size/sizeof(Elf32_Dyn)+ 1;
		}
	}

}

int addLibrary::findSection(const char* name){
	
	for(int cnt = 0 ; cnt < arraySize; cnt ++ ){
		if (newElfFileSec[cnt].sec_hdr && 
			!strcmp(name, (char *)strTabData->d_buf+newElfFileSec[cnt].sec_hdr->sh_name) ) {
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

	//ccw 27 jun 2003 ALWAYS PUT THE PHT AND .dynamic section IN THE TEXT SEGMENT
	//update TEXT SEGMENT

	//the PHT is put AFTER the .dynamic section. The phdr slot [0] for the PHT is
	//already correct.  the new size of the text segment is the distance from
	//the start of the text segment to the start of the PHT plus the size of the
	//PHT.  This accounts for the .dynamic section and all padding.
	phdr[2].p_filesz = ((phdr[0].p_offset - phdr[2].p_offset) + phdr[0].p_filesz);
	phdr[2].p_memsz = phdr[2].p_filesz;

	/* 	i need to make the text section writeable b/c the 
		dynamic section is now there and needs
		to be updated during execution
	*/	
	
	phdr[2].p_flags = PF_R+PF_W+PF_X ; //ccw 26 jun 2003
	phdr[0].p_flags = PF_R+PF_W+PF_X ; //ccw 30 jun 2003


	

	for(int i=0;i<newElfFileEhdr->e_phnum;i++){
		if(phdr[i].p_offset && phdr[i].p_offset < dynstrOffset){
			phdr[i].p_offset -= libnameLen;
			if(phdr[i].p_vaddr){
				phdr[i].p_vaddr -= libnameLen;
				phdr[i].p_paddr -= libnameLen;
			}
		}else if(phdr[i].p_type != PT_PHDR && phdr[i].p_vaddr > newPhdrAddr){
			phdr[i].p_offset +=_pageSize;
		}
	}
} 


void addLibrary::addStr(Elf_Data *newData,Elf_Data *oldData, 
			const char* libname){
	newData->d_size += libnameLen;
	
	delete [] (char*) newData->d_buf;
	
        newData->d_buf = new char[newData->d_size];
        memcpy(newData->d_buf, oldData->d_buf,oldData->d_size);	

	memcpy(&(((char*) newData->d_buf)[newData->d_size-libnameLen]), libname, libnameLen);
	libnameIndx = newData->d_size-libnameLen;			

}

int addLibrary::writeNewElf(char* filename, const char* libname){

	Elf32_Ehdr *realEhdr;
	Elf32_Phdr *realPhdr;
	Elf_Scn *realScn;
	Elf32_Shdr *realShdr;
	Elf_Data *realData, *l_strTabData;
	unsigned int dynstrOffset;
	bool seenDynamic = false;

	Elf32_Shdr **newSegments;

	int foundDynstr = 0;
	
	//open newElf
	//if the following call is to open() instead of creat()
	//and dyninst is compiled with gcc 3.0.[2|4] with -O3 
	//this file is written as all zeros and the permissions are 
	//odd ( u=--- g=--S o=r--)
	
	//if((newFd = open(filename,  O_WRONLY|O_CREAT|O_TRUNC )) ==-1){
	//S_IRWXU is not defined in gcc < 3.0
        if((newFd = creat(filename,0x1c0 /*S_IRWXU*/)) == -1) { 
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
//	realEhdr->e_phnum += numberExtraSegs;
	realPhdr = elf32_newphdr(newElf, realEhdr->e_phnum);
	memset(realPhdr, '\0', realEhdr->e_phnum * realEhdr->e_phentsize);
	memcpy(realPhdr, newElfFilePhdr, (realEhdr->e_phnum/*-numberExtraSegs*/) * realEhdr->e_phentsize);

	realEhdr ->e_phoff = newPhdrOffset;

	l_strTabData = newElfFileSec[findSection(".shstrtab")].sec_data; 
	//section data
	
	int pastPhdr = 0;

	unsigned int hashOffset, dynsymOffset, dynstrOffsetNEW;
	unsigned int hashAlign, dynsymAlign, dynstrAlign;

	int symTabIndex, strTabIndex, dynamicIndex, dynsymIndex, dynstrIndex, hashIndex;
	dynsymIndex = findSection(".dynsym");
	dynstrIndex = findSection(".dynstr");
	hashIndex = findSection(".hash");

	if( dynsymIndex == -1 || dynstrIndex == -1 ){
		printf(" Dynamic symbol table missing...cannot save world\n");
		return 0;
	}

	if( hashIndex == -1 ){
		printf(" Hash table not found...cannot save world\n");
		return 0;
	}

	hashOffset = newElfFileSec[hashIndex].sec_hdr->sh_offset;
	dynsymOffset = newElfFileSec[dynsymIndex].sec_hdr->sh_offset;
	dynstrOffsetNEW = newElfFileSec[dynstrIndex].sec_hdr->sh_offset;
	hashAlign =  newElfFileSec[hashIndex].sec_hdr->sh_addralign;

	dynsymAlign =  newElfFileSec[dynsymIndex].sec_hdr->sh_addralign;
	dynstrAlign =  newElfFileSec[dynstrIndex].sec_hdr->sh_addralign;

	dynstrOffsetNEW -= libnameLen;
	while(dynstrOffsetNEW % dynstrAlign){
		dynstrOffsetNEW --;
		dynsymOffset --;
		hashOffset --;
	}
	dynsymOffset -= libnameLen;

	while(dynsymOffset % dynsymAlign){
		dynsymOffset --;
		hashOffset --;
	}

	hashOffset -=libnameLen;	
	while(hashOffset % hashAlign){
		hashOffset--;
	}
	//.interp has align of 1

	unsigned int foundExtraSegment=0, extraSegmentPad=0;
	
	dynamicIndex = findSection(".dynamic");

	updateSymbols(newElfFileSec[dynsymIndex].sec_data,newElfFileSec[dynstrIndex].sec_data,
		newElfFileSec[dynamicIndex].sec_hdr->sh_addr );

	symTabIndex = findSection(".symtab");
	strTabIndex = findSection(".strtab");

	if(symTabIndex != -1 && strTabIndex!= -1&&dynamicIndex!= -1){
		updateSymbols(newElfFileSec[symTabIndex].sec_data,newElfFileSec[strTabIndex].sec_data,
		newElfFileSec[dynamicIndex].sec_hdr->sh_addr );
	}


	
	newSegments = (Elf32_Shdr**) new char[sizeof(Elf32_Shdr*) * numberExtraSegs];
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
			if(!strcmp(".hash", (char *)l_strTabData->d_buf+realShdr->sh_name)){
				realShdr->sh_offset = hashOffset;
			}else if(!strcmp(".dynsym", (char *)l_strTabData->d_buf+realShdr->sh_name)){
				realShdr->sh_offset = dynsymOffset;
			}else if(!strcmp(".hash", (char *)l_strTabData->d_buf+realShdr->sh_name)){
				realShdr->sh_offset = dynstrOffsetNEW;
			}else{ // .interp 
				realShdr->sh_offset -= libnameLen;
			}
		}

		if( !strcmp(".dynamic", (char *)l_strTabData->d_buf+realShdr->sh_name) && !seenDynamic) {
			seenDynamic = true; 
			updateDynamic(realData, hashOffset+BASEADDR, dynsymOffset+BASEADDR, 
				dynstrOffsetNEW+BASEADDR);
			realShdr->sh_size += sizeof(Elf32_Dyn);  // i added a shared library
			

		}else if( !strcmp(".dynamic", (char *)l_strTabData->d_buf+realShdr->sh_name) && seenDynamic) {
			realShdr->sh_name = 0;
			realShdr->sh_type = 1;
			realData->d_size = realShdr->sh_size;
		}

		
		if( !strcmp(".dynstr", (char *)l_strTabData->d_buf+realShdr->sh_name) ){
			dynstrOffset = realShdr->sh_offset;
			addStr(realData,newElfFileSec[cnt].sec_data, libname);
			realShdr->sh_size += libnameLen;
			foundDynstr = 1;
		}
		if( pastPhdr || realShdr->sh_addr >= newPhdrAddr){
			realShdr->sh_offset+=_pageSize;
			pastPhdr = 1;
		}
		if( !strncmp("dyninstAPI",(char *)l_strTabData->d_buf+realShdr->sh_name, 10)){
	
			if(!strncmp("dyninstAPIhighmem_",(char *)l_strTabData->d_buf+realShdr->sh_name, 18)){
				//since this is not loaded by the loader, ie it is mmaped (maybe)
				//by libdyninstAPI_RT.so it only needs aligned on the real pag size
				realShdr->sh_offset +=extraSegmentPad; //ccw 23 jul 2003
				while( (realShdr->sh_offset)%realPageSize ){
					realShdr->sh_offset ++;
					extraSegmentPad++;
				}
				foundExtraSegment = 1;
			}else if(strcmp("dyninstAPI_mutatedSO", (char *)l_strTabData->d_buf+realShdr->sh_name)  &&
				strcmp("dyninstAPI_data", (char *)l_strTabData->d_buf+realShdr->sh_name) 	){
	
				realShdr->sh_offset += extraSegmentPad;
				while( (realShdr->sh_addr - realShdr->sh_offset)%0x10000 ){
					realShdr->sh_offset ++;
					extraSegmentPad++;
	
				}
				foundExtraSegment = 1;
				newSegments[atoi(& ( ((char *)l_strTabData->d_buf+realShdr->sh_name )[11]))] = realShdr;
			} else{
				realShdr->sh_offset += extraSegmentPad;
			}	

		}else if(foundExtraSegment){
			realShdr->sh_offset += extraSegmentPad;
		}
	}
	realEhdr ->e_shoff += _pageSize + extraSegmentPad;;	
	elf_update(newElf, ELF_C_NULL);

	updateProgramHeaders(realPhdr, dynstrOffset);
	
	elf_update(newElf, ELF_C_WRITE);
	return 1;
}

//if we cannot edit the file (no space at the top, no space
//in the text/data gap etc) we return 0.
//otherwise we return 1 or 2, depending on if we use the
//text gap (1) or the data gap (2)
//
//If we cannot open the new file we return -1
int addLibrary::driver(Elf *elf,  char* newfilename, const char *libname) {

	libnameLen = strlen(libname) +1;
	oldElf = elf;	
	//_pageSize = getpagesize();
	_pageSize = 0x10000;
	realPageSize = getpagesize();
	
	createNewElf();
	elf_end(elf);
	textSegEndIndx = findEndOfTextSegment();
	dataSegStartIndx = findStartOfDataSegment();
	
	int gapFlag = checkFile();
	if(gapFlag){  
		
	
		moveDynamic();
		findNewPhdrAddr();
		findNewPhdrOffset();

		gapFlag = writeNewElf(newfilename, libname);
		elf_end(newElf);
	}else{
		//error
	}
	return gapFlag;
}


void addLibrary::fixUpPhdrForDynamic(){

	//change data segment
	//change dynamic ptr
	unsigned int dataSegSizeChange;
	int dataSegIndex=0, dynSegIndex=0;

	while( newElfFilePhdr[dataSegIndex].p_offset != newElfFileSec[dataSegStartIndx+1].sec_hdr->sh_offset){

		dataSegIndex++;	
	}
		
	dataSegSizeChange = newElfFileSec[dataSegStartIndx+1].sec_hdr->sh_offset - 
			newElfFileSec[dataSegStartIndx].sec_hdr->sh_offset;

	while( newElfFilePhdr[dynSegIndex].p_type != PT_DYNAMIC){
		dynSegIndex ++;
	}
	newElfFilePhdr[dynSegIndex].p_offset = newElfFileSec[dataSegStartIndx].sec_hdr->sh_offset;
	newElfFilePhdr[dynSegIndex].p_vaddr =  newElfFileSec[dataSegStartIndx].sec_hdr->sh_addr;
	newElfFilePhdr[dynSegIndex].p_filesz += sizeof(Elf32_Dyn);
	

}


void addLibrary::moveDynamic(){

	int oldDynamicIndex;
	int newDynamicIndex;
	Elf_element *updatedElfFile;
	Elf32_Shdr tmpShdr;

	oldDynamicIndex = findSection(".dynamic");

	newDynamicIndex = dataSegStartIndx;

	updatedElfFile = (Elf_element*) new char[sizeof(Elf_element) * (newElfFileEhdr->e_shnum+1)]; 

	arraySize ++;
		
	for(int cnt = 0, newIndex = 0; cnt < newElfFileEhdr->e_shnum-1 ; cnt++, newIndex++){
	
		if( cnt == newDynamicIndex ){
			//copy in dynamic here

			//save original info for later.
			memcpy( &tmpShdr, newElfFileSec[oldDynamicIndex].sec_hdr, sizeof(Elf32_Shdr));

			memcpy( &(updatedElfFile[newIndex]), &(newElfFileSec[oldDynamicIndex]), sizeof(Elf_element));

			//ok, now the .dynamic section is at the offset for .got (or whatever is the first in the
			//segment
			updatedElfFile[newIndex].sec_hdr->sh_offset = newElfFileSec[cnt].sec_hdr->sh_offset; //ccw 27 jun 2003 - 
				//updatedElfFile[newIndex].sec_hdr->sh_size - sizeof(Elf32_Dyn); /* increase in size */

			while(updatedElfFile[newIndex].sec_hdr->sh_offset  % 0x10){
				 updatedElfFile[newIndex].sec_hdr->sh_offset ++; //ccw 27 jun 2003 was: --
			}

			// ccw 25 jun 2003
			//ok, we need to find the address of the dynamic section.
			//we know the offset in the file.  Given that offset, the address should
			//be that offset plus the 'memory offset'.  the 'memory offset' is the difference
			//between the real mem location and the file offset.  we calcuate this by using
			//the previous (if the dynamic section is at the end of the text segment; if the dynamic section
			//is at the start of the data section we use the next) section's information (on the assumption that 
			//the previous (next) section and the .dynamic section are in the same segment).

			updatedElfFile[newIndex].sec_hdr->sh_addr = updatedElfFile[newIndex].sec_hdr->sh_offset +
			( updatedElfFile[newIndex-1].sec_hdr->sh_addr -updatedElfFile[newIndex-1].sec_hdr->sh_offset);

			newIndex++;
			//copy old entry to to next slot
		} 
		memcpy( &(updatedElfFile[newIndex]), &(newElfFileSec[cnt]), sizeof(Elf_element));
		if(cnt == oldDynamicIndex){
			//reset name to zero
			//allocat new secHdr
			updatedElfFile[newIndex].sec_hdr = new Elf32_Shdr;//(Elf32_Shdr*) new char[sizeof(Elf32_Shdr)];



			memcpy( updatedElfFile[newIndex].sec_hdr, &tmpShdr, sizeof(Elf32_Shdr));
			//updatedElfFile[newIndex].sec_hdr->sh_addr = updatedElfFile[newDynamicIndex].sec_hdr->sh_addr; //ccw 1 jul 2003
			
		}	

		if(updatedElfFile[newIndex].sec_hdr->sh_link >= (unsigned int) newDynamicIndex){
			updatedElfFile[newIndex].sec_hdr->sh_link++;
		}
		if(updatedElfFile[newIndex].sec_hdr->sh_info >=  (unsigned int) newDynamicIndex){
			updatedElfFile[newIndex].sec_hdr->sh_info++;
		}

	}

	newElfFileEhdr->e_shnum++;
	if(newElfFileEhdr->e_shstrndx >= newDynamicIndex){
		newElfFileEhdr->e_shstrndx++;
	}
	delete [] newElfFileSec;
	newElfFileSec = updatedElfFile;

	fixUpPhdrForDynamic();
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

unsigned int addLibrary::findEndOfTextSegment(){
//newElfFilePhdr
	Elf32_Phdr *tmpPhdr = newElfFilePhdr;
	unsigned int lastOffset;
	unsigned int retVal=0;

	while(tmpPhdr->p_type != PT_LOAD){	
		//find first loadable segment
		//it should be the text segment
		tmpPhdr++;
	}
	lastOffset = tmpPhdr->p_offset + tmpPhdr->p_filesz;

	for(int i=0;i<arraySize && !retVal; i++){

		if( lastOffset == newElfFileSec[i].sec_hdr->sh_offset + newElfFileSec[i].sec_hdr->sh_size){
			//found it!
			retVal = i;
		}
	}
	

	return retVal;
	
	
}

unsigned int addLibrary::findStartOfDataSegment(){
	Elf32_Phdr *tmpPhdr = newElfFilePhdr;
	unsigned int firstOffset;
	unsigned int retVal=0;

	while(tmpPhdr->p_type != PT_LOAD){	
		//find first loadable segment
		//it should be the text segment
		tmpPhdr++;
	}
	tmpPhdr++;
	while(tmpPhdr->p_type != PT_LOAD){	
		//find second loadable segment
		//it should be the data segment
		tmpPhdr++;
	}

	firstOffset = tmpPhdr->p_offset;

	for(int i=0;i<arraySize && !retVal; i++){

		if( firstOffset == newElfFileSec[i].sec_hdr->sh_offset){
			//found it!
			retVal = i;
		}
	}
	return retVal;
}


int addLibrary::findNewPhdrAddr(){
	//ccw 27 jun 2003 THIS MUST BE CALLED AFTER moveDynamic() !!
	//and before writeElf
	Elf32_Shdr *tmpShdr;


	tmpShdr = newElfFileSec[findSection(".dynamic")].sec_hdr;
	//i have not yet added the extra record the .dynamic table so account for that 
	//in the addr calc!
	newPhdrAddr = tmpShdr->sh_addr + tmpShdr->sh_size + sizeof(Elf32_Dyn);
	
	while(newPhdrAddr %4){
		newPhdrAddr ++;
	}	

        return 0;		
}

int addLibrary::findNewPhdrOffset(){
	Elf32_Shdr *tmpShdr;
	//ccw 27 jun 2003 THIS MUST BE CALLED AFTER moveDynamic() !!
	//and before writeElf
	
	tmpShdr = newElfFileSec[findSection(".dynamic")].sec_hdr;
	//i have not yet added the extra record the .dynamic table so account for that 
	//in the offset calc!
	newPhdrOffset = tmpShdr->sh_offset + tmpShdr->sh_size + sizeof(Elf32_Dyn);

	while(newPhdrOffset %4){
		newPhdrOffset ++;
	}	


	return 0;	
}


int addLibrary::checkFile(){
        Elf32_Shdr *roDataShdr, *dataShdr;
        unsigned int endrodata, startdata;
	char extraSegName[20];
	
	//is there space at the beginning?
	
	//must be space between 0x34 (end of the ehdr) and
	//the text segment start.
	
	if(newElfFileSec[0].sec_hdr->sh_offset < (unsigned int) (sizeof(Elf32_Ehdr)+libnameLen) ){
		//there is not enough room
		return 0;
	}

        roDataShdr = newElfFileSec[/*findSection(".rodata")*/ textSegEndIndx].sec_hdr;
        dataShdr = newElfFileSec[/*findSection(".data")*/ dataSegStartIndx].sec_hdr;

	numberExtraSegs = 0;
	sprintf(extraSegName,"dyninstAPI_%08x", numberExtraSegs);

	while ( findSection(extraSegName) != -1 ){
		numberExtraSegs ++;
		sprintf(extraSegName,"dyninstAPI_%08x", numberExtraSegs);
	}	

        endrodata = roDataShdr->sh_addr + roDataShdr->sh_size;

        startdata = dataShdr -> sh_addr;

	return 1;//result; //ccw 27 jun 2003

}

//This method updates the symbol table,
//it updates the address of _DYNAMIC
void addLibrary::updateSymbols(Elf_Data* symtabData,Elf_Data* strData, unsigned int dynAddr){

	if( symtabData && strData){
	
       		Elf32_Sym *symPtr=(Elf32_Sym*)symtabData->d_buf;

	        for(unsigned int i=0;i< symtabData->d_size/(sizeof(Elf32_Sym));i++,symPtr++){

	
        	        if( !(strcmp("_DYNAMIC", (char*) strData->d_buf + symPtr->st_name))){
                	        symPtr->st_value = dynAddr;
	                }
        	}
	}
}


#endif
