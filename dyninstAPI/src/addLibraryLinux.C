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

/* $Id: addLibraryLinux.C,v 1.16 2005/03/18 04:34:56 chadd Exp $ */

#if defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */

#include "addLibraryLinux.h"
#include "util.h"
#define MOVEDYNAMIC 1 

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
 * UPDATE: 17 may 2002
 * Previously it was assumed there would be a DT_CHECKSUM entry
 * in the dynamic table.  this is not always the case. some
 * compilers done put this entry in.  If it is not present there
 * is no place to put the link to the dyninstAPI runtime shared
 * library and everything fails.  
 * NOW: The dynamic section is moved to the start of the data 
 * segment.  This lets the dynamic table be expanded and the 
 * dyninstAPI library gets its own entry without overwriting a 
 * slot in the dynamic table. 
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
	//fprintf(stderr," NEW ELF e_shoff: %d\n", newElfFileEhdr->e_shoff);

	// get Phdr
	oldPhdr = elf32_getphdr(oldElf);
	
	newElfFilePhdr = (Elf32_Phdr*) new char[oldEhdr->e_phentsize * oldEhdr->e_phnum];
	memcpy(newElfFilePhdr, oldPhdr, oldEhdr->e_phentsize * oldEhdr->e_phnum);

	newElfFileSec = (Elf_element *) new char[sizeof(Elf_element) * (oldEhdr->e_shnum + 1)];
	memset(newElfFileSec, '\0', sizeof(Elf_element) * (oldEhdr->e_shnum + 1));

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

void addLibrary::updateDynamic(Elf_Data *newData,unsigned int dynstrOff,unsigned int dynsymOff, unsigned int hashOff,unsigned int stringSize){
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

#if MOVEDYNAMIC
	char *d_buf = new char[newData->d_size +sizeof(Elf32_Dyn) ];

	//fprintf(stderr," ALLOC NEW DYNAMIC: %x\n",newData->d_size +sizeof(Elf32_Dyn));
 
 	memset(d_buf, '\0', newData->d_size +sizeof(Elf32_Dyn) ); // make them all DT_NULL
 	memcpy(d_buf, newData->d_buf, newData->d_size );
 
 //	delete [] newData;
 	delete  [] (char*) (newData->d_buf); //ccw 8 mar 2004
 	newData->d_buf = d_buf;
 	newData->d_size += sizeof(Elf32_Dyn);
#endif
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
			((Elf32_Dyn*) (newData->d_buf))[counter].d_un.d_val = stringSize; 
		}
		/* ccw 8 mar 2004: leave DT_DEBUG as is, this allows gdb to
			properly access the link_map struct
		*/
                /*if( ((Elf32_Dyn*) (newData->d_buf))[counter].d_tag == DT_DEBUG){
                    	((Elf32_Dyn*) (newData->d_buf))[counter].d_un.d_val = 1;

		}*/
#if MOVEDYNAMIC
		if( ((Elf32_Dyn*) (newData->d_buf))[counter].d_tag == DT_NULL) {
		       	((Elf32_Dyn*) (newData->d_buf))[counter].d_tag = DT_NEEDED;
        		((Elf32_Dyn*) (newData->d_buf))[counter].d_un.d_val = libnameIndx;
			counter = newData->d_size/sizeof(Elf32_Dyn)+ 1;
		}
#endif
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
	//phdr[0].p_offset = newPhdrOffset;
	//phdr[0].p_vaddr =  newPhdrAddr ;
	//phdr[0].p_paddr = phdr[0].p_vaddr;

	if(gapFlag == TEXTGAP){
		//update TEXT SEGMENT
		phdr[2].p_memsz = newTextSegmentSize;
		phdr[2].p_filesz = newTextSegmentSize;

	} 
	/* we have already updated the data segments filesz and memsz in  fixUpPhdrForDynamic*/
	/*else if(gapFlag == DATAGAP){
		//update DATA SEGMENT
		phdr[3].p_memsz += phdr[0].p_filesz;
		phdr[3].p_filesz += phdr[0].p_filesz;
		//phdr[3].p_offset = newPhdrOffset;
		//phdr[3].p_vaddr = newPhdrAddr;
		//phdr[3].p_paddr = newPhdrAddr;
	}*/
	
	for(int i=0;i<newElfFileEhdr->e_phnum;i++){
		if(phdr[i].p_type == PT_NOTE ){
			phdr[i].p_vaddr += (newNoteOffset - phdr[i].p_offset);
			phdr[i].p_paddr = phdr[i].p_vaddr; 
			phdr[i].p_offset = newNoteOffset;
			//phdr[i].p_type = PT_NULL;// 6 Mar 2005
		}
		
	/*	if(phdr[i].p_offset && phdr[i].p_offset < dynstrOffset){
			phdr[i].p_offset -= libnameLen;
			phdr[i].p_vaddr -= libnameLen;
			phdr[i].p_paddr -= libnameLen;
		}else if(phdr[i].p_type != PT_PHDR && phdr[i].p_vaddr > newPhdrAddr){
			phdr[i].p_offset +=_pageSize;
		}
	*/
	}	
#if !MOVEDYNAMIC

	phdr[3].p_offset += _pageSize;
	phdr[4].p_offset += _pageSize;

#endif

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
	unsigned int dynsymOffset;
	unsigned int hashOffset;
	unsigned int dynstrSize;
	bool seenDynamic = false;
	int lastTextSegmentIndex = findEndOfTextSegment();
	
	int foundDynstr = 0;
	
	//open newElf
	//if the following call is to open() instead of creat()
	//and dyninst is compiled with gcc 3.0.[2|4] with -O3 
	//this file is written as all zeros and the permissions are 
	//odd ( u=--- g=--S o=r--)
	
	//if((newFd = open(filename,  O_WRONLY|O_CREAT|O_TRUNC )) ==-1){
	//S_IRWXU is not defined in gcc < 3.0
        if((newFd = creat(filename,0x1c0 /*S_IRWXU*/)) == -1) { 
                bperr("cannot creat: %s %d\n",filename,errno);
                switch(errno){
                case EACCES:
                        bperr("EACCESS \n");
                        break;
                }
                return -1;
        }

        if((newElf = elf_begin(newFd, ELF_C_WRITE, NULL)) == NULL){
		bperr(" elf_begin failed for newElf");
		return -1;
        }

	elf_flagelf(newElf,ELF_C_SET, ELF_F_LAYOUT);

	// ehdr
	realEhdr = elf32_newehdr(newElf);
     memcpy(realEhdr,newElfFileEhdr , sizeof(Elf32_Ehdr));

	// phdr
	realPhdr = elf32_newphdr(newElf, realEhdr->e_phnum);
	//fprintf(stderr," MAKING NEW PHDR: size: %d\n",realEhdr->e_phnum * realEhdr->e_phentsize);
	memcpy((char*)realPhdr, (char*)newElfFilePhdr, realEhdr->e_phnum * realEhdr->e_phentsize);

	//realEhdr ->e_phoff = newPhdrOffset;

	strTabData = newElfFileSec[findSection(".shstrtab")].sec_data; 
	//section data
	
	int pastPhdr = 0;

 	updateSymbols(newElfFileSec[findSection(".dynsym")].sec_data,
		newElfFileSec[findSection(".dynstr")].sec_data, 
		newElfFileSec[findSection(".dynamic")].sec_hdr->sh_addr );
 
 	updateSymbols(newElfFileSec[findSection(".symtab")].sec_data,
		newElfFileSec[findSection(".strtab")].sec_data, 
		newElfFileSec[findSection(".dynamic")].sec_hdr->sh_addr );
	

	updateSymbolsSectionInfo(newElfFileSec[findSection(".symtab")].sec_data,
		newElfFileSec[findSection(".strtab")].sec_data);

	
	updateSymbolsMovedTextSectionUp(newElfFileSec[findSection(".dynsym")].sec_data,
              newElfFileSec[findSection(".dynstr")].sec_data,12);
	updateSymbolsMovedTextSectionUp(newElfFileSec[findSection(".symtab")].sec_data,
              newElfFileSec[findSection(".strtab")].sec_data,12);


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
	
		//now dont shift everything down since we have already added the string
		/*if(!foundDynstr){
			realShdr->sh_offset -= libnameLen;
		}*/

 		if( !strcmp(".dynamic", (char *)strTabData->d_buf+realShdr->sh_name) && !seenDynamic) {
 			seenDynamic = true; 
 			updateDynamic(realData,dynstrOffset,dynsymOffset,hashOffset,dynstrSize);
#if MOVEDYNAMIC
 			realShdr->sh_size += sizeof(Elf32_Dyn);  // i added a shared library
#endif
 			
 
 		}else if( !strcmp(".dynamic", (char *)strTabData->d_buf+realShdr->sh_name) && seenDynamic) {
 			realShdr->sh_name = 0;
 			realShdr->sh_type = 1;
 			realData->d_size = realShdr->sh_size;
			memset(realData->d_buf, '\0', realShdr->sh_size);
			realShdr->sh_link--;
		}
		if( !strcmp(".dynstr", (char *)strTabData->d_buf+realShdr->sh_name) ){
			dynstrOffset = realShdr->sh_addr;
			//addStr(realData,newElfFileSec[cnt].sec_data, libname);
			//realShdr->sh_size += libnameLen;
			foundDynstr = 1;
			dynstrSize = realShdr->sh_size;
		}
		if( !strcmp(".dynsym", (char *)strTabData->d_buf+realShdr->sh_name) ){
			dynsymOffset = realShdr->sh_addr;
		}
		if( !strcmp(".hash", (char *)strTabData->d_buf+realShdr->sh_name) ){
			hashOffset = realShdr->sh_addr;
		}
#if MOVEDYNAMIC

		//put the address of the .dynamic section at the beginning of the .got
		if( !strcmp(".got", (char *)strTabData->d_buf+realShdr->sh_name) ){
			memcpy( (void*)realData->d_buf, (void*)&newElfFileSec[dataSegStartIndx].sec_hdr->sh_addr, 4);
		}
#else
/*
		if(  cnt >= dataSegStartIndx ){
			realShdr->sh_offset += _pageSize;
		}
		
 		if( !strcmp(".dynamic", (char *)strTabData->d_buf+realShdr->sh_name) ){
			realShdr->sh_link --;
		}
	*/
#endif


		if( (realShdr->sh_type == SHT_REL ||realShdr->sh_type == SHT_RELA) && realShdr->sh_info > 0){
			realShdr->sh_info--;
		}


		/*if( pastPhdr || realShdr->sh_addr >= newPhdrAddr){
			realShdr->sh_offset+=_pageSize;
			pastPhdr = 1;
		}*/
#if !MOVEDYNAMIC
		if( cnt > lastTextSegmentIndex){
			realShdr->sh_offset+=_pageSize;
		}
#endif

		//fprintf(stderr," WRITE NEW ELF %s\t\t%08x\t\t%06x\t\t%06x::\t%06x\t%06x\n",(char *)strTabData->d_buf+realShdr->sh_name,realShdr->sh_addr, realShdr->sh_offset, (unsigned int) realShdr->sh_size,newElfFileSec[cnt].sec_data, (unsigned int) newElfFileSec[cnt].sec_data->d_buf);

	}

	realEhdr ->e_shoff += _pageSize;	
	//fprintf(stderr," NEW PHT LOCAL: %d\n",realEhdr ->e_phoff);
	elf_update(newElf, ELF_C_NULL);

	updateProgramHeaders(realPhdr, dynstrOffset);
	
	elf_update(newElf, ELF_C_WRITE);
	return gapFlag;
}


unsigned int addLibrary::findSizeOfSegmentFromPHT(int type){
	Elf32_Phdr *tmpPhdr = newElfFilePhdr;
	int elements = newElfFileEhdr->e_phnum;
	unsigned int retValue=0;
 
 	for(int i =0;i< elements && tmpPhdr->p_type != type;i++){
 		tmpPhdr++;
 	}
	if( tmpPhdr->p_type == PT_NOTE){
		retValue = tmpPhdr->p_filesz;	
	}
 	return retValue;


}

// look in the PHT for a PT_NOTE entry, return its file size
unsigned int addLibrary::findSizeOfNoteSection(){

	return findSizeOfSegmentFromPHT(PT_NOTE);
}


// look in the PHT for a PT_DYNAMIC entry, return its file size
unsigned int addLibrary::findSizeOfDynamicSection(){

	return findSizeOfSegmentFromPHT(PT_DYNAMIC);
}

int addLibrary::expandDynstrUp(char *libname){

	int oldNoteSectionIndex = findSection(".note.ABI-tag");
	int oldDynstrSectionIndex = findSection(".dynstr");
	int lastTextSegmentIndex = findEndOfTextSegment();
	Elf_element noteSection;
	libnameIndx=-1;
	int libnameLen = strlen(libname)+1;

	noteSection = newElfFileSec[oldNoteSectionIndex];
	noteSection.sec_hdr->sh_size = 8;
	noteSection.sec_data->d_size=8;

	//ok, now change the offset and addr for everything up through .dynstr
	//change size of .dynstr
	int currentOffset=noteSection.sec_hdr->sh_offset+8;
	int currentAddr = noteSection.sec_hdr->sh_addr+8;

	for(int i=oldNoteSectionIndex+1;i<=oldDynstrSectionIndex;i++){

		if(i== (oldDynstrSectionIndex)){
			//the new .dynstr
			int oldDynstrSize = newElfFileSec[i].sec_hdr->sh_size;

			//the new size of the new .dynstr section is from the current
			//offset to the end of the original .dynstr section
			newElfFileSec[i].sec_hdr->sh_size = (newElfFileSec[i+1].sec_hdr->sh_offset - currentOffset);

			if( newElfFileSec[i].sec_hdr->sh_size - oldDynstrSize	 < strlen(libname)+1){
				//not enough room
				return -1;
			}

			//add the new string
			char *tmpBuf = (char*)newElfFileSec[i].sec_data->d_buf;
			int libnameLen = strlen(libname)+1;
			newElfFileSec[i].sec_data->d_buf = new char[(newElfFileSec[i].sec_hdr->sh_size)];
			newElfFileSec[i].sec_data->d_size = newElfFileSec[i].sec_hdr->sh_size;
			memcpy(newElfFileSec[i].sec_data->d_buf,tmpBuf ,oldDynstrSize);
			delete [] tmpBuf;
			
			memcpy(&(((char*) newElfFileSec[i].sec_data->d_buf)[newElfFileSec[i].sec_data->d_size-libnameLen]), 
				libname, strlen(libname)+1);

			//save the index that points to the new string, the .dynamic table needs it
			libnameIndx = newElfFileSec[i].sec_data->d_size-libnameLen;
		}

		newElfFileSec[i].sec_hdr->sh_offset=currentOffset;
		newElfFileSec[i].sec_hdr->sh_addr = currentAddr; 

		currentOffset += newElfFileSec[i].sec_hdr->sh_size;
		while(currentOffset %4 !=0){
			currentOffset++;
		}
		currentAddr += newElfFileSec[i].sec_hdr->sh_size;
		while(currentAddr%4 !=0){
			currentAddr++;
		}

		//fprintf(stderr,"NEW OFFSETS AND MEM ADDR AND SIZE AND SIZE \t%x\t\t%x\t\t%x\t\t%x\n",newElfFileSec[i].sec_hdr->sh_offset,newElfFileSec[i].sec_hdr->sh_addr, newElfFileSec[i].sec_hdr->sh_size,newElfFileSec[i].sec_data->d_size);
	}
	newTextSegmentSize = newElfFileSec[lastTextSegmentIndex].sec_hdr->sh_offset + newElfFileSec[lastTextSegmentIndex].sec_hdr->sh_size;
	newNoteOffset = newElfFileSec[oldNoteSectionIndex].sec_hdr->sh_offset;

	return libnameIndx;
}

//this moves the Note to the gap, and shifts upward
//everything until .dynstr.  .dynstr is expanded
//up, its end remains the same
//
//since the sections move we need to update the section links
//for .hash .dynsym .gnu.version .gnu.version_r .rel.dyn .rel.plt 
//(all sections after the original .note and before the new .note)
//
//return the index of the libname in the new .dynstr section
int addLibrary::moveNoteShiftFollowingSectionsUp(char *libname){

	int oldNoteSectionIndex = findSection(".note.ABI-tag");
	int oldDynstrSectionIndex = findSection(".dynstr");
	int lastTextSegmentIndex = findEndOfTextSegment();
	Elf_element noteSection;
	libnameIndx=-1;
	int libnameLen = strlen(libname)+1;

	if(oldNoteSectionIndex == -1 ){
		// try to find .note
		oldNoteSectionIndex = findSection(".note");

		//really should just look up the section from the PHT
	}
	if(oldNoteSectionIndex == -1){
		//failure
		return -1;
	}

	memcpy(&noteSection,&(newElfFileSec[oldNoteSectionIndex]),sizeof(Elf_element));

	for(int i=oldNoteSectionIndex;i<lastTextSegmentIndex;i++){

		memcpy(&(newElfFileSec[i]),&(newElfFileSec[i+1]),sizeof(Elf_element));

		// if the sh_link points to something that was shifted, move it.
		if( newElfFileSec[i].sec_hdr->sh_link > oldNoteSectionIndex && newElfFileSec[i].sec_hdr->sh_link< lastTextSegmentIndex){
			newElfFileSec[i].sec_hdr->sh_link--;
		}
	}

	//put the old .note section here at the end.
	newElfFileSec[lastTextSegmentIndex]=noteSection;

	//ok, now change the offset and addr for everything up through .dynstr
	//change size of .dynstr
	int currentOffset=noteSection.sec_hdr->sh_offset;
	int currentAddr = noteSection.sec_hdr->sh_addr;
	for(int i=oldNoteSectionIndex;i<oldDynstrSectionIndex;i++){

		if(i== (oldDynstrSectionIndex-1)){
			//the new .dynstr
			int oldDynstrSize = newElfFileSec[i].sec_hdr->sh_size;

			//the new size of the new .dynstr section is from the current
			//offset to the end of the original .dynstr section
			newElfFileSec[i].sec_hdr->sh_size = ((newElfFileSec[i].sec_hdr->sh_offset +
					newElfFileSec[i].sec_hdr->sh_size) - currentOffset);

			if( newElfFileSec[i].sec_hdr->sh_size - oldDynstrSize	 < strlen(libname)+1){
				//not enough room
				return -1;
			}

			//add the new string
			char *tmpBuf = (char*)newElfFileSec[i].sec_data->d_buf;
			int libnameLen = strlen(libname)+1;
			newElfFileSec[i].sec_data->d_buf = new char[(newElfFileSec[i].sec_hdr->sh_size)];
			newElfFileSec[i].sec_data->d_size = newElfFileSec[i].sec_hdr->sh_size;
			memcpy(newElfFileSec[i].sec_data->d_buf,tmpBuf ,oldDynstrSize);
			delete [] tmpBuf;
			
			memcpy(&(((char*) newElfFileSec[i].sec_data->d_buf)[newElfFileSec[i].sec_data->d_size-libnameLen]), 
				libname, strlen(libname)+1);

			//save the index that points to the new string, the .dynamic table needs it
			libnameIndx = newElfFileSec[i].sec_data->d_size-libnameLen;
		}

		newElfFileSec[i].sec_hdr->sh_offset=currentOffset;
		newElfFileSec[i].sec_hdr->sh_addr = currentAddr; 

		currentOffset += newElfFileSec[i].sec_hdr->sh_size;
		while(currentOffset %4 !=0){
			currentOffset++;
		}
		currentAddr += newElfFileSec[i].sec_hdr->sh_size;
		while(currentAddr%4 !=0){
			currentAddr++;
		}

		//fprintf(stderr,"NEW OFFSETS AND MEM ADDR AND SIZE AND SIZE %x %x %x %x\n",newElfFileSec[i].sec_hdr->sh_offset,newElfFileSec[i].sec_hdr->sh_addr, newElfFileSec[i].sec_hdr->sh_size,newElfFileSec[i].sec_data->d_size);
	}

	int offsetIncr = 1;
	if( gapFlag == TEXTGAP){
		newElfFileSec[lastTextSegmentIndex].sec_hdr->sh_offset = newElfFileSec[lastTextSegmentIndex-1].sec_hdr->sh_offset + newElfFileSec[lastTextSegmentIndex-1].sec_hdr->sh_size;
		newElfFileSec[lastTextSegmentIndex].sec_hdr->sh_addr = newElfFileSec[lastTextSegmentIndex-1].sec_hdr->sh_addr + newElfFileSec[lastTextSegmentIndex-1].sec_hdr->sh_size;
		newTextSegmentSize = newElfFileSec[lastTextSegmentIndex].sec_hdr->sh_offset + newElfFileSec[lastTextSegmentIndex].sec_hdr->sh_size;
	}else{
		newElfFileSec[lastTextSegmentIndex].sec_hdr->sh_offset = newElfFileSec[lastTextSegmentIndex+1].sec_hdr->sh_offset - newElfFileSec[lastTextSegmentIndex].sec_hdr->sh_size;
		newElfFileSec[lastTextSegmentIndex].sec_hdr->sh_addr = newElfFileSec[lastTextSegmentIndex+1].sec_hdr->sh_addr - newElfFileSec[lastTextSegmentIndex].sec_hdr->sh_size;
		offsetIncr = -1;
	 	dataSegStartIndx--; 
	}

	while(newElfFileSec[lastTextSegmentIndex].sec_hdr->sh_offset  %4 != 0){
		newElfFileSec[lastTextSegmentIndex].sec_hdr->sh_offset+=offsetIncr;
	}

	newNoteOffset = newElfFileSec[lastTextSegmentIndex].sec_hdr->sh_offset;

	while(newElfFileSec[lastTextSegmentIndex].sec_hdr->sh_addr %4 != 0){
		newElfFileSec[lastTextSegmentIndex].sec_hdr->sh_addr+=offsetIncr;
	}

	return libnameIndx;

} 

int addLibrary::findIsNoteBeforeDynstr(){

	int NoteSectionIndex = findSection(".note.ABI-tag");
	int DynstrSectionIndex = findSection(".dynstr");

	if (NoteSectionIndex == -1){
		NoteSectionIndex = findSection(".note");
	}
	if( NoteSectionIndex < DynstrSectionIndex){
		return 1;
	}
	return 0;

	
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
 	textSegEndIndx = findEndOfTextSegment();
 	dataSegStartIndx = findStartOfDataSegment();
	checkFile(); // this sets textSideGap and dataSideGap


	int sizeOfNoteSection = findSizeOfNoteSection();
	int isNoteBeforeDynstr;

	if( sizeOfNoteSection == -1 ){
		//no .note section found!
		isNoteBeforeDynstr = 0;
	}else{	
		isNoteBeforeDynstr = findIsNoteBeforeDynstr();
	}
	
	if( sizeOfNoteSection == -1 || isNoteBeforeDynstr == 0){
		//failure: no .note or .note not before .dynstr
		return -1;
	}

	if( sizeOfNoteSection < strlen(libname)+1 ){
		//failure: not enough space after .note is shifted up
		return -1;
	}


	/*if( textSideGap > sizeOfNoteSection ){
		gapFlag = TEXTGAP;
	}else*/ if (dataSideGap > (sizeOfNoteSection + newElfFileSec[findSection(".dynamic")].sec_hdr->sh_size+sizeof(Elf32_Dyn)) ){
		//check to see if it fits in the data segment, plus the the dynamic table
		//with its increased size
		gapFlag = DATAGAP;
	}else{
		//failure: neither gap is big enough
		return -1;
	}


		
//	if(gapFlag){
//		findNewPhdrAddr();
//		findNewPhdrOffset();

		//this moves the Note to the gap, and shifts upward
		//everything until .dynstr.  .dynstr is expanded
		//up, its end remains the same
		int moved = /*expandDynstrUp(libname);*/moveNoteShiftFollowingSectionsUp(libname); 

		if(moved == -1){
			//failure;
			return -1;
		}

#if MOVEDYNAMIC
		moveDynamic();
#endif
	
		gapFlag = writeNewElf(newfilename, libname);
		elf_end(newElf);
//	}else{
		//error
//	}
	close(newFd); //ccw 6 jul 2003
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
			delete /*[] */ newElfFileSec[cnt].sec_hdr; //INSURE
			if( cnt != dataSegStartIndx ){ //this is .dynamic, dont delete twice.
				if( newElfFileSec[cnt].sec_data->d_buf ){
					//fprintf(stderr,"DELETING d_buf %x\n",(unsigned int)newElfFileSec[cnt].sec_data->d_buf);
					delete [] (char*) newElfFileSec[cnt].sec_data->d_buf ;
				}
				//fprintf(stderr,"DELETING sec_data %x\n",(unsigned int)newElfFileSec[cnt].sec_data);
				delete /*[]*/  newElfFileSec[cnt].sec_data; //INSURE
			}
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
		tmpShdr = newElfFileSec[/*findSection(".rodata")*/ textSegEndIndx].sec_hdr;
		newPhdrAddr = tmpShdr->sh_addr + tmpShdr->sh_size;
	}else if(gapFlag == DATAGAP){
		tmpShdr = newElfFileSec[/*findSection(".data")*/ dataSegStartIndx].sec_hdr;
		newPhdrAddr = tmpShdr->sh_addr - phdrSize;
	}
	while(newPhdrAddr %4){
		newPhdrAddr ++;
	}	


        return 0;		
}

int addLibrary::findNewPhdrOffset(){
	Elf32_Shdr *tmpShdr;

	if(gapFlag == TEXTGAP){
		tmpShdr = newElfFileSec[/*findSection(".rodata")*/textSegEndIndx].sec_hdr;
		newPhdrOffset = tmpShdr->sh_offset + tmpShdr->sh_size;
	}else if(gapFlag == DATAGAP){
		tmpShdr = newElfFileSec[/*findSection(".data")*/dataSegStartIndx].sec_hdr; 
		newPhdrOffset = tmpShdr->sh_offset - phdrSize + _pageSize;
	}
	while(newPhdrOffset %4){
		newPhdrOffset ++;
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
        roDataShdr = newElfFileSec[/*findSection(".rodata")*/ textSegEndIndx].sec_hdr;
        dataShdr = newElfFileSec[/*findSection(".data")*/ dataSegStartIndx].sec_hdr;
/*
        roDataShdr = newElfFileSec[findSection(".rodata")].sec_hdr;
        dataShdr = newElfFileSec[findSection(".data")].sec_hdr;
*/

        endrodata = roDataShdr->sh_addr + roDataShdr->sh_size;

        startdata = dataShdr -> sh_addr;

        if( startdata - endrodata >= phdrSize ){

		//where to put the phdr?
		//find the gap between the text segment
		//end and the next page boundry
		unsigned int nextPage = endrodata >> 12;
		nextPage ++;
		nextPage = nextPage << 12;

		textSideGap = nextPage - endrodata;

		//find the gap between the data segment start
		//and the previous page boundry
                unsigned int prevPage = startdata >> 12;
                prevPage = prevPage << 12;
		dataSideGap = startdata - prevPage;

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
 
 	firstOffset = tmpPhdr->p_vaddr /*p_offset*/;
 
 	for(int i=0;i<arraySize && !retVal; i++){
 
 		if( firstOffset == newElfFileSec[i].sec_hdr->sh_addr/*offset*/){
 			//found it!
 			retVal = i;
 		}
 	}
 	return retVal;
 }

void addLibrary::updateSymbolsSectionInfo(Elf_Data* symtabData,Elf_Data* strData){

	if(symtabData){
	      Elf32_Sym *symPtr=(Elf32_Sym*)symtabData->d_buf;
 
        	 for(unsigned int i=0;i< symtabData->d_size/(sizeof(Elf32_Sym));i++,symPtr++){
 
			if( ELF32_ST_TYPE(symPtr->st_info) == STT_SECTION){
				int index = symPtr->st_shndx;
				symPtr->st_value= newElfFileSec[index].sec_hdr->sh_addr;
			} 
        	 }

	}
}
 

void addLibrary::updateSymbolsMovedTextSectionUp(Elf_Data* symtabData,Elf_Data* strData,int oldTextIndex){

	if(symtabData && strData ) { 
	      Elf32_Sym *symPtr=(Elf32_Sym*)symtabData->d_buf;
 
        	 for(unsigned int i=0;i< symtabData->d_size/(sizeof(Elf32_Sym));i++,symPtr++){
 
			if( symPtr->st_shndx < dataSegStartIndx && symPtr->st_shndx > 1){
				symPtr->st_shndx--;
			} 
        	 }
	}

}
 //This method updates the symbol table,
 //it updates the address of _DYNAMIC
 void addLibrary::updateSymbols(Elf_Data* symtabData,Elf_Data* strData, unsigned int dynAddr){

	if(symtabData && strData ) { 
	         Elf32_Sym *symPtr=(Elf32_Sym*)symtabData->d_buf;
 
        	 for(unsigned int i=0;i< symtabData->d_size/(sizeof(Elf32_Sym));i++,symPtr++){
 
 
                	 if( !(strcmp("_DYNAMIC", (char*) strData->d_buf + symPtr->st_name))){
                        	 symPtr->st_value = dynAddr;
	                 }
        	 }
	}
 }


 void addLibrary::fixUpPhdrForDynamic(){
 
 //change data segment
 //change dynamic ptr
 	unsigned int dataSegSizeChange;
 	int dataSegIndex=0, dynSegIndex=0;
	int oldDataSegStartIndx = dataSegStartIndx+1;

	if(gapFlag == DATAGAP){

		oldDataSegStartIndx++;
	}
 
 
 	while( newElfFilePhdr[dataSegIndex].p_vaddr != newElfFileSec[oldDataSegStartIndx].sec_hdr->sh_addr){
 		dataSegIndex++;	
 	}
 	
	/*fprintf(stderr,"fixUpPhdrForDynamic: %i %x %i %x\n",oldDataSegStartIndx,newElfFileSec[oldDataSegStartIndx].sec_hdr->sh_offset,
			dataSegStartIndx, 
 			newElfFileSec[dataSegStartIndx].sec_hdr->sh_offset );*/
 	dataSegSizeChange = newElfFileSec[oldDataSegStartIndx].sec_hdr->sh_offset -  /* +1 here because the .note and .dynamic sections have moved */
 			newElfFileSec[dataSegStartIndx].sec_hdr->sh_offset;
 	/* change data segment*/
 	newElfFilePhdr[dataSegIndex].p_offset = newElfFileSec[dataSegStartIndx].sec_hdr->sh_offset;
	/*fprintf(stderr," DATA SEG FILE SIZE: OLD %x INCR %x NEW %x\n",newElfFilePhdr[dataSegIndex].p_filesz,dataSegSizeChange,newElfFilePhdr[dataSegIndex].p_filesz + dataSegSizeChange);*/
 	newElfFilePhdr[dataSegIndex].p_filesz += dataSegSizeChange;
 	newElfFilePhdr[dataSegIndex].p_memsz += dataSegSizeChange;
 	newElfFilePhdr[dataSegIndex].p_vaddr =  newElfFileSec[dataSegStartIndx].sec_hdr->sh_addr;
 	newElfFilePhdr[dataSegIndex].p_paddr =  newElfFileSec[dataSegStartIndx].sec_hdr->sh_addr;


 	while( newElfFilePhdr[dynSegIndex].p_type != PT_DYNAMIC){
 		dynSegIndex ++;
 	}
 	newElfFilePhdr[dynSegIndex].p_offset = newElfFileSec[dataSegStartIndx].sec_hdr->sh_offset;
 	newElfFilePhdr[dynSegIndex].p_vaddr =  newElfFileSec[dataSegStartIndx].sec_hdr->sh_addr;
 	newElfFilePhdr[dynSegIndex].p_filesz += sizeof(Elf32_Dyn);
	newElfFilePhdr[dynSegIndex].p_memsz += sizeof(Elf32_Dyn);//ccw 23 jun 2003
	newElfFilePhdr[dynSegIndex].p_paddr  = newElfFileSec[dataSegStartIndx].sec_hdr->sh_addr; // ccw 8 mar 2004
 	
 
 }
 
 
 void addLibrary::moveDynamic(){
 
 	int oldDynamicIndex;
 	unsigned int newDynamicIndex;
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

		/*	fprintf(stderr,"UPDATE DYNAMIC: %x- %x- %x= %x\n", newElfFileSec[cnt].sec_hdr->sh_offset , 
 				updatedElfFile[newIndex].sec_hdr->sh_size , sizeof(Elf32_Dyn), (newElfFileSec[cnt].sec_hdr->sh_offset - 
 				updatedElfFile[newIndex].sec_hdr->sh_size - sizeof(Elf32_Dyn)));*/
 			updatedElfFile[newIndex].sec_hdr->sh_offset = newElfFileSec[cnt].sec_hdr->sh_offset - 
 				updatedElfFile[newIndex].sec_hdr->sh_size - sizeof(Elf32_Dyn); /* increase in size */

			updatedElfFile[newIndex].sec_hdr->sh_offset += _pageSize;
 
 			while(updatedElfFile[newIndex].sec_hdr->sh_offset  % 0x10){
 				 updatedElfFile[newIndex].sec_hdr->sh_offset --;
 			}
 			updatedElfFile[newIndex].sec_hdr->sh_addr = newElfFileSec[cnt].sec_hdr->sh_addr - 
 				 newElfFileSec[cnt].sec_hdr->sh_offset +updatedElfFile[newIndex].sec_hdr->sh_offset - _pageSize;
			//fprintf(stderr, " UPDATE DYNAMIC NEW ADDR: %x\n", updatedElfFile[newIndex].sec_hdr->sh_addr);

			// new data!
			//updatedElfFile[newIndex].sec_data->d_buf = new char[newElfFileSec[oldDynamicIndex].sec_data->d_size];
			//memcpy(updatedElfFile[newIndex].sec_data->d_buf,newElfFileSec[oldDynamicIndex].sec_data->d_buf,newElfFileSec[oldDynamicIndex].sec_data->d_size);
			 
			//fprintf(stderr,"ALLOC: %x\n",newElfFileSec[oldDynamicIndex].sec_data->d_size);
 
 			newIndex++;
 			//copy old entry to to next slot
 		} 
 		memcpy( &(updatedElfFile[newIndex]), &(newElfFileSec[cnt]), sizeof(Elf_element));
 		if(cnt == oldDynamicIndex){
 			//reset name to zero
 			//allocat new secHdr
 			updatedElfFile[newIndex].sec_hdr = new Elf32_Shdr;//(Elf32_Shdr*) new char[sizeof(Elf32_Shdr)];
/*
			updatedElfFile[newIndex].sec_data = new Elf_Data;
			updatedElfFile[newIndex].sec_data->d_buf = new char[tmpShdr.sh_size]; 
			fprintf(stderr," ALLOC %x\n", tmpShdr.sh_size);
*/			
 			memcpy( updatedElfFile[newIndex].sec_hdr, &tmpShdr, sizeof(Elf32_Shdr));

 		}	
		if( cnt >= newDynamicIndex){
			updatedElfFile[newIndex].sec_hdr->sh_offset += _pageSize;
		}

 
 		if(updatedElfFile[newIndex].sec_hdr->sh_link >= newDynamicIndex){
 			updatedElfFile[newIndex].sec_hdr->sh_link++;
 		}
 		if(updatedElfFile[newIndex].sec_hdr->sh_info >= newDynamicIndex){
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

#endif

// vim:ts=5:
