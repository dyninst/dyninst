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

#include "writeBackXCOFF.h"
#include <string.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>

void writeBackXCOFF::attachToText(Address addr, unsigned int size, char* data){
	textTramps=data;
	textTrampsAddr=addr;
	textTrampsSize=size;
}


int writeBackXCOFF::loadFile(XCOFF* file){


	file->fd = open( file->name, O_RDONLY);

	if( file->fd == -1 ){
		bperr(" error opening: %s \n", file->name);
		return 0;
	}

	struct stat statInfo;

	int statRet = fstat(file->fd, &statInfo);	
	
	if( statRet == -1) {
		bperr(" no stats found for %s \n", file->name);
		close(file->fd);
		return -1;
	}
	
	file->buffer = (char*) mmap(0x0, statInfo.st_size,
		PROT_READ, MAP_FILE, file->fd,0x0); 

	file->filesize = statInfo.st_size; 
	return 1;

}


void writeBackXCOFF::parseXCOFF(XCOFF *file){


	if(! file->opened){
		return;	
	}

	
	file->XCOFFhdr = (struct xcoffhdr*) file->buffer;

	if(file->XCOFFhdr->filehdr.f_magic != 0x01DF ){
		bperr(" %s is not a 32XCOFF file\n", file->name);
		file->opened = false;
	}

	file->sechdr = (struct scnhdr *) (file->buffer + sizeof(struct filehdr) +
		file->XCOFFhdr->filehdr.f_opthdr);

	file->loadhdr = 
	(struct ldhdr*) &(file->sechdr[file->XCOFFhdr->aouthdr.o_snloader-1]);	

}


writeBackXCOFF::writeBackXCOFF(char* oldFileName, char* newFileName,bool &error, 
	bool debugOutputFlag, int numbScns)
	: maxSections(numbScns), numberSections(0), debugFlag(debugOutputFlag) {


	oldFile.name = new char[strlen(oldFileName)+1];
	strcpy(oldFile.name, oldFileName);

	
	newFile.name = new char[strlen(newFileName)+1];
	strcpy(newFile.name, newFileName);

	int ret = loadFile(&oldFile);
	oldFile.opened = true;
	error = false; 
	if( !ret ){
		bperr(" Error: %s not opened properly\n",oldFile.name);
		oldFile.opened = false;
		error = true;
	}

	mutateeProcess = 0;

	parseXCOFF(&oldFile);

	newSections = new scnhdr[numbScns];		

}
writeBackXCOFF::~writeBackXCOFF(){

	if(oldFile.name){
		delete [] oldFile.name;
	}
	if(newFile.name){
		delete [] newFile.name;
	}

	for(int i=0;i<numberSections;i++){
		delete [] (char*) newSections[i].s_scnptr;
	}	
	munmap(oldFile.buffer, oldFile.filesize);
	delete [] newFile.buffer;	
	close(oldFile.fd);
	close(newFile.fd);
}

int writeBackXCOFF::addSection(char *name, unsigned int paddr, 
	unsigned int vaddr, int sectionSize, char*data, int flags){

	
	if(numberSections >= maxSections){
		return -1;
	}

	memset((char*) &(newSections[numberSections]),'\0', sizeof(struct scnhdr));

	strcpy(&(newSections[numberSections].s_name[0]), name);

	newSections[numberSections].s_paddr = paddr;
	newSections[numberSections].s_vaddr = vaddr;
	newSections[numberSections].s_size = sectionSize;

	newSections[numberSections].s_scnptr = (long int) new char[sectionSize];
	memcpy ( (char*)  newSections[numberSections].s_scnptr, data, sectionSize);

	newSections[numberSections].s_flags = flags;	

	return numberSections++;
}

bool writeBackXCOFF::setHeapAddr(unsigned int heapAddr){

}

//calculate offsets; return total size of the file (MINUS the new sections!)
unsigned int calcOffsets(XCOFF* newFile, XCOFF* oldFile, unsigned int newTextSize){
	//here i need to calculate all the offsets for the NEW 
	//file.  I will return the TOTALSIZE of the new file

	//this will happen in two passes. 
	//First pass will lay in all the scnptrs
	//Second pass will patch up the reloc/lnno/etc
	//pointers


	//first pass
	int numbSecs = oldFile->XCOFFhdr->filehdr.f_nscns;
	struct scnhdr *shdr = newFile->sechdr;
	unsigned int currentIndex=oldFile->sechdr[0].s_scnptr;//index to step through new file

	for(int i=0;i<numbSecs;i++, shdr++){

		if(shdr->s_scnptr  && (i+1) != oldFile->XCOFFhdr->aouthdr.o_sntext ){
		//	shdr->s_scnptr += offSet; //ccw 1 aug 2002

			shdr->s_scnptr = currentIndex;

			/*while( shdr->s_scnptr % 4 ){ //align!
				shdr->s_scnptr++;
			}*/
			currentIndex = shdr->s_scnptr;
		}

		if( (i+1) == oldFile->XCOFFhdr->aouthdr.o_sntext ) {
			//update s_size for the text section

			shdr->s_size = newTextSize;

		}
		currentIndex += shdr->s_size;
	}

	//second pass
	unsigned int lastDataByte = currentIndex;
	unsigned int offSet = lastDataByte - (oldFile->sechdr[numbSecs-1].s_scnptr +oldFile->sechdr[numbSecs-1].s_size );
	unsigned int lastByte;

	//bpinfo( " DIFFERENCE:  0x%x = 0x%x - 0x%x\n", offSet, lastDataByte, (oldFile->sechdr[numbSecs-1].s_scnptr +oldFile->sechdr[numbSecs-1].s_size ));

	shdr = newFile->sechdr;
	for(int i=0;i<numbSecs;i++, shdr++){
		
		if(shdr->s_relptr){
			shdr->s_relptr += offSet; //ccw 1 aug 2002
		}
		if(shdr->s_lnnoptr){
			shdr->s_lnnoptr += offSet; //ccw 1 aug 2002
		}

		//set currentIndex to the largest offset of the two if they
		//are beyond the currentIndex	
		lastByte = shdr->s_lnnoptr + (shdr->s_nlnno *(sizeof(struct lineno)));
		currentIndex = ((currentIndex < lastByte) ? lastByte : currentIndex);

		lastByte = shdr->s_relptr + (shdr->s_nreloc *(sizeof(struct reloc)));
		currentIndex = (currentIndex < lastByte) ? lastByte : currentIndex;
	}
	

	//now update the symbol table info.
	newFile->XCOFFhdr->filehdr.f_symptr = currentIndex;
	
	int symentSize = 18;
	int *len;
	int symTableSize=0;
	
	if(newFile->XCOFFhdr->filehdr.f_symptr ){

		symTableSize = oldFile->XCOFFhdr->filehdr.f_nsyms * symentSize;

		len = (int*) &(oldFile->buffer[ oldFile->XCOFFhdr->filehdr.f_symptr +(oldFile->XCOFFhdr->filehdr.f_nsyms*symentSize)]);
		symTableSize += *len;

	}

	currentIndex += symTableSize;

	return currentIndex; 
}


bool writeBackXCOFF::createXCOFF(){
	//This is the function that actually creates the
	//new XCOFF file by allocating the buffer and
	//patching up all the necessary pointers within
	//the file
	
	//new plan:
	//the text section CANNOT move OR it will be
	//loaded into a different place. :(

	//this means: I CANNOT put in new scnhdrs in the
	//usual place because they will shift the .text section
	//down a few bits
	//

	//The new file will look like this:
	//
	// filehdr
	// aouthdr
	// scnhdr
	// ...
	// scnhdr
	// section data (.text - expanded) 
	// PAD
	// section data
	// relocation data
	// relocation data
	// line number data
	// symbol table
	// NUMBER OF NEW SECTIONS ccw 10 sep 2003
	// NEW SECTION HDR
	// NEW SECTION HDR
	// NEW SECTION DATA

	//i have no idea if this will work.
	int pageSize = getpagesize();

	int totalSize=0; // totalSize of the NEW file

	int newTextSize =0; // size of NEW TEXT SECTION	

	int textPadding = 0; //padding bytes between end of text and start of data

	int newDataSize = 0;	 	//newDataSize is the size of the 
					//NEW SCNHDRs and the new SECTIONS

	int offset = 0;			//offset is the difference of the
					//size of the NEW EXPANDED text section
					//and the old text section. AND  any
					//padding needed between the end of the
					//text section and the start of the
					//data section
					//it is used to move EVERYTHING after
					//the .text section
				
	int trueTextStart; 	// in fortan execs made by xlcf the start address of the
				// text section is 0x0, but the loader (appears to) default to 0x1000000+ 92 + (fnscns * 40)
	char *newFileCurrent, *oldFileCurrent;
	struct scnhdr *tmpScnhdr, *oldTextScnhdr;

	struct xcoffhdr *OldXCOFFhdr = (struct xcoffhdr*) oldFile.buffer;
	
	newDataSize = sizeof(struct scnhdr) * numberSections;
	tmpScnhdr = newSections;
	for(int i=0 ; i<numberSections; i++, tmpScnhdr++){
		newDataSize += tmpScnhdr->s_size;		
	}
//really should check that .data is immedialtey after .text
	oldTextScnhdr = (struct scnhdr*) &(oldFile.sechdr[OldXCOFFhdr->aouthdr.o_sntext-1]); // ccw 1 aug 2002 added -1!
	struct scnhdr *oldDataScnhdr =  (struct scnhdr*) &(oldFile.sechdr[OldXCOFFhdr->aouthdr.o_sndata-1]); 

	int fakeTextStart = 0x10000000 + oldTextScnhdr->s_scnptr;//+ 92 + (OldXCOFFhdr->filehdr.f_nscns * 40);
	int fakeDataStart = 0x20000000 + oldDataScnhdr->s_scnptr;

	int trueDataStart =   ( OldXCOFFhdr->aouthdr.o_data_start != 0) ? OldXCOFFhdr->aouthdr.o_data_start : fakeDataStart;

	trueTextStart = ( OldXCOFFhdr->aouthdr.o_text_start != 0) ? OldXCOFFhdr->aouthdr.o_text_start : fakeTextStart;

	newTextSize = ( textTrampsAddr - trueTextStart+ textTrampsSize);


	// distance from end of text to start of next page PLUS 
	// distance from start of page to start of data 
	textPadding = (pageSize - ((trueTextStart+ newTextSize) % pageSize)) +	((trueDataStart % pageSize )); 


	// if we span an entire page compress ourselves 
	if(textPadding > pageSize){
		textPadding -= pageSize; 
	}

	if(textPadding > 0){
		textPadding += pageSize;
	}

	newTextSize += textPadding;
	//offset = newTextSize - OldXCOFFhdr->aouthdr.o_tsize; 


	//totalSize = oldFile.filesize +  newTextSize - OldXCOFFhdr->aouthdr.o_tsize + newDataSize;
	//bpinfo("\n\n trueDataStart %x  trueTextStart %x newTextSIZE: %x  textEND: %x\n\n", trueDataStart, trueTextStart,newTextSize, trueTextStart+newTextSize);
/*

	//allocate memory
	newFile.buffer = new char[ totalSize ];	
*/

	//allocate memory for the HEADERS
	newFile.buffer = new char[ sizeof(struct filehdr) + OldXCOFFhdr->filehdr.f_opthdr + 
					(sizeof(struct scnhdr) * OldXCOFFhdr->filehdr.f_nscns)]; 	

	//setup new file
	newFile.XCOFFhdr = (struct xcoffhdr*) newFile.buffer;
	
	newFile.sechdr = (struct scnhdr *) (newFile.buffer + sizeof(struct filehdr) +
		oldFile.XCOFFhdr->filehdr.f_opthdr);


	//copy over filehdr and aouthdr
	memcpy((char*) newFile.XCOFFhdr, oldFile.buffer, sizeof(struct filehdr) 
		+ oldFile.XCOFFhdr->filehdr.f_opthdr );

	//copy section headers to the new file.
	memcpy(newFile.sechdr, oldFile.sechdr, sizeof(struct scnhdr) * oldFile.XCOFFhdr->filehdr.f_nscns);

	//calculate offsets; return total size of the file (less the size of the new data)
	totalSize = calcOffsets( &newFile, &oldFile,newTextSize);

	//create tmp variable to hold the new offset info while we allocate the entire file 
	char* tmp = new char[sizeof(struct filehdr) + OldXCOFFhdr->filehdr.f_opthdr + 
					(sizeof(struct scnhdr) * OldXCOFFhdr->filehdr.f_nscns)];

	memcpy (tmp, newFile.buffer, sizeof(struct filehdr) + OldXCOFFhdr->filehdr.f_opthdr + 
					(sizeof(struct scnhdr) * OldXCOFFhdr->filehdr.f_nscns));

	delete [] newFile.buffer;

	// add in size of new sections here!
	// i'm adding in the sizeof(unsigned int) so i can add a SIZE (number of sections) before the
	// section headers ccw 10 sep 2003
	totalSize += (sizeof(unsigned int) + newDataSize + (sizeof(struct scnhdr) * numberSections));
	newFile.buffer = new char[ totalSize ];

	memcpy (newFile.buffer,tmp, sizeof(struct filehdr) + OldXCOFFhdr->filehdr.f_opthdr + 
					(sizeof(struct scnhdr) * OldXCOFFhdr->filehdr.f_nscns));

	//setup new file
	newFile.XCOFFhdr = (struct xcoffhdr*) newFile.buffer;
	
	newFile.sechdr = (struct scnhdr *) (newFile.buffer + sizeof(struct filehdr) +
		oldFile.XCOFFhdr->filehdr.f_opthdr);

	delete [] tmp;
	
	newFile.XCOFFhdr->aouthdr.o_tsize = newTextSize;

	//copy over section raw data.
	//for each section
	//UPDATE AND EXPAND TEXT SECTION
	copySectionData(offset);

	//copy over the relocation data as specified in
	//the section headers
	copyRelocationData(offset);

	
	//copy over the line number data as specified in the 
	//section headers
	copyLineNumberData(offset);

	//copy over the symbol table data
	newFileCurrent = (char*) copySymbolTableData(offset /*+ additionalPadding*/);


	if(!newFileCurrent){
		bperr(" ERROR! NO SYMBOL TABLE FOUND!\n");
		//assert(0);
	}

	//ADD NEW DATA

	//ADD number of new section headers
	*((unsigned int *) newFileCurrent) = (unsigned int) numberSections;
	newFileCurrent += sizeof(unsigned int);
	
	//new scnhdrs
	struct scnhdr* addedSectionHeader[numberSections];
	for(int i=0;i<numberSections;i++){
		//create new section header!
		addedSectionHeader[i] = addSectionHeader(newFileCurrent, newSections[i].s_name, newSections[i].s_paddr, 
			newSections[i].s_vaddr, newSections[i].s_size, newSections[i].s_flags); 
	
		newFileCurrent += sizeof(struct scnhdr);
	}
		

	//new raw data
	for(int i=0;i<numberSections;i++){

		//open file argv[6] and add sectionSize bytes as the new section
		addNewSectionData(newFileCurrent, (char*) newSections[i].s_scnptr, newSections[i].s_size, addedSectionHeader[i]); 

		newFileCurrent += newSections[i].s_size;
	}

	newFile.filesize = totalSize;

}

/*
void writeBackXCOFF::expandMoveTextSection(char* newFileStart, int start, int sectionsize, int newTextFilePtr){

	int newTextAddr= start;
	struct xcoffhdr * XCOFFhdr = (struct xcoffhdr*) newFileStart;
	int align;
	struct scnhdr *newtextsechdr =  
		(struct scnhdr*) ( &newFileStart[sizeof(struct xcoffhdr)+ (sizeof(struct scnhdr) * (XCOFFhdr->aouthdr.o_sntext-1))]);
	align = (int) pow(2.0, (double) XCOFFhdr->aouthdr.o_algntext);

	while(  (newtextsechdr->s_vaddr - newTextAddr) % align){
		newTextAddr ++;
	}

	//copy the old text section data here! 
	//NOTE: this is really the mutated text section
	memcpy((newFileStart +newTextFilePtr 
//newTextAddr ccw 18 jul 2002
), (newFileStart+ newtextsechdr->s_scnptr), 
		newtextsechdr->s_size);	


	//memcpy the text tramps in to place
	memcpy( (newFileStart + 
//newTextAddr ccw 18 jul 2002 
newTextFilePtr  + (textTrampsAddr - newtextsechdr->s_vaddr)), textTramps, textTrampsSize);

	newtextsechdr->s_scnptr = (long int) (newTextFilePtr 
//newTextAddr ccw 18 jul 2002 
);
	newtextsechdr->s_size = sectionsize;
	XCOFFhdr->aouthdr.o_tsize = sectionsize;
}
*/


	//add sectionSize bytes as the new section
void writeBackXCOFF::addNewSectionData(char *newFileCurrent, char* data,int sectionSize, struct scnhdr *newHdr){


	memcpy( newFileCurrent, data, sectionSize);
	newHdr->s_scnptr = (newFileCurrent - newFile.buffer);

}

	//create new section header!
	//NOTE: name MUST be NO LONGER than 8 characaters (including the \0)
struct scnhdr *writeBackXCOFF::addSectionHeader(char *newFileCurrent, char* name,unsigned int paddr, 
	unsigned int vaddr, int sectionSize, int flags){

	struct scnhdr *shdr = (struct scnhdr*) newFileCurrent;

	*shdr->s_name = '\0';
	if(strlen(name) < 8){
		strcpy(shdr->s_name, name);
	}else{
		bperr(" name %s is too long!\n", name);
	}
	shdr->s_paddr = paddr;
	shdr->s_vaddr = vaddr;
	shdr->s_size = sectionSize;
	shdr->s_scnptr = 0;
	shdr->s_relptr = 0;
	shdr->s_lnnoptr = 0;
	shdr->s_nreloc = 0;
	shdr->s_nlnno = 0;
	shdr->s_flags = flags ;
	return shdr;
}
	
	//copy over section raw data.
	//for each section

void  writeBackXCOFF::copySectionData(int offSet){

	struct scnhdr *shdr = oldFile.sechdr;
	struct scnhdr *shdrNew = newFile.sechdr; //ccw 1 aug 2002

	char *newTmp = newFile.buffer;
	int foundText = 0;
	int f_nscns = oldFile.XCOFFhdr->filehdr.f_nscns;
	char *tmpBuf;


	for(int i=0;i<f_nscns;i++,shdr++, shdrNew++){

		if((i+1) == newFile.XCOFFhdr->aouthdr.o_sntext ){ //TEXT SECTION
			//copy normal text section over...
			//GET THIS FROM MEMORY!!

			//again, the fortran binary does not fill in the text scnhdr so i must fake it
			int fakeTextStart =  ((shdr->s_vaddr != 0x0 ) ? shdr->s_vaddr : (0x10000000 + shdr->s_scnptr ));//+92+(f_nscns * 40)));
			int textTrampOffset = textTrampsAddr -  fakeTextStart;	

			tmpBuf =  (char*) oldFile.buffer+shdr->s_scnptr; //if not mutatee

			if( mutateeProcess ){
				tmpBuf = new char[shdr->s_size];
				mutateeProcess->readTextSpace((const void*) (fakeTextStart), shdr->s_size, (void*) tmpBuf);
			}

			memcpy( &(newTmp[shdrNew->s_scnptr/*shdr->s_scnptr*/]), tmpBuf, shdr->s_size);
	
			//copy over tramps	
			memcpy( &(newTmp[shdrNew->s_scnptr/*shdr->s_scnptr*/ + textTrampOffset]), textTramps, textTrampsSize);	

		}else if(shdr->s_scnptr){
			memcpy( &(newTmp[shdrNew->s_scnptr/*shdr->s_scnptr + offSet*/]), oldFile.buffer+shdr->s_scnptr, shdr->s_size);
		}
	}

} 

void writeBackXCOFF::copyRelocationData(int offSet){

	char *newFileStart = newFile.buffer;
	char *oldFileStart = oldFile.buffer;
	
	struct scnhdr* shdr = oldFile.sechdr;
	struct scnhdr* shdrNew = newFile.sechdr; //ccw 1 aug 2002
	int f_nscns = oldFile.XCOFFhdr->filehdr.f_nscns;

	for(int i=0;i<f_nscns;i++, shdr++){
		if(shdr->s_relptr){
			memcpy( &(newFileStart[shdrNew->s_relptr /*offSet + shdr->s_relptr*/]),&(oldFileStart[shdr->s_relptr]),
				shdr->s_nreloc * sizeof(struct reloc)); 	

		}
	}
	
}

	//copy over the line number data as specified in the 
	//section headers
void writeBackXCOFF::copyLineNumberData(int offSet){

	char *newFileStart = newFile.buffer;
	char *oldFileStart = oldFile.buffer;
	struct scnhdr* shdr = oldFile.sechdr;
	struct scnhdr* shdrNew = newFile.sechdr; //ccw 1 aug 2002
 
	int f_nscns = oldFile.XCOFFhdr->filehdr.f_nscns;

	for(int i=0;i<f_nscns;i++, shdr++){
		if(shdr->s_relptr){
			memcpy( &(newFileStart[shdrNew->s_lnnoptr /*offSet + shdr->s_lnnoptr*/]),&(oldFileStart[shdr->s_lnnoptr]),
				shdr->s_nlnno * sizeof(struct lineno)); 	
		}
	}
}


	//copy over the symbol table data
void* writeBackXCOFF::copySymbolTableData(int offSet){

	char *newFileStart = newFile.buffer;
	char *oldFileStart = oldFile.buffer;
	struct filehdr hdr = oldFile.XCOFFhdr->filehdr;
	struct filehdr hdrNew = newFile.XCOFFhdr->filehdr; //ccw 1 aug 2002
	char *tmpNew, *tmpOld;
	int symentSize = 18;
	int *len;

	
	if(hdr.f_symptr){
		//copy over symbol table.
		memcpy( & (newFileStart[/*offSet + */hdrNew.f_symptr]), & (oldFileStart[hdr.f_symptr]),
			hdr.f_nsyms * symentSize);

		len = (int*) &(oldFileStart[ hdr.f_symptr +(hdr.f_nsyms*symentSize)]);
		//copy over string table
		memcpy( & (newFileStart[/*offSet + */hdrNew.f_symptr +(hdr.f_nsyms*symentSize)]),
			& (oldFileStart[hdr.f_symptr +(hdr.f_nsyms*symentSize)]),*len);

		//hdrNew.f_symptr = offSet + hdr.f_symptr; //ccw 1 aug 2002

		return &newFileStart[hdrNew.f_symptr + (hdr.f_nsyms*symentSize) + *len];
	}else{
		return NULL;
	}
}

bool writeBackXCOFF::outputXCOFF(){

	
	newFile.fd = open(newFile.name, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IXUSR);

	if(newFile.fd == -1){
		bperr(" Opening %s for writing failed\n", newFile.name);
		return false;
	}


	//memcpy(buf, hdr, fileSize + loaderSecSize);	
	if(debugFlag){
		bpinfo(" WRITING %x bytes to %s \n", newFile.filesize, newFile.name);
	}
	write(newFile.fd, (void*) newFile.buffer, newFile.filesize);

	close(newFile.fd);



}
