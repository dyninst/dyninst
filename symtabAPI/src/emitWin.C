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

#include <algorithm>
#include "../common/h/parseauxv.h"
#include "emitWin.h"
#include "Symtab.h"
#include <iostream>
#include <dbghelp.h>

//extern void symtab_log_perror(const char *msg);

using namespace Dyninst;
using namespace Dyninst::SymtabAPI;
using namespace std;


emitWin::emitWin(PCHAR baseaddress, Object *o_nt, void (*err_func)(const char *)){
    base_addr =  baseaddress;
    obj_nt = o_nt;
	err_func_ = err_func;
}

PIMAGE_SECTION_HEADER emitWin::CreateSecHeader(unsigned int size, PIMAGE_SECTION_HEADER preSecHdr){
    DWORD mem_offset, mem_size, disk_offset, disk_size;
    PIMAGE_NT_HEADERS pNTHeader = obj_nt -> GetImageHeader();
   
    disk_size=PEAlign(size,pNTHeader -> OptionalHeader.FileAlignment);
    mem_size=PEAlign(disk_size, pNTHeader->OptionalHeader.SectionAlignment);

    disk_offset=PEAlign(preSecHdr->PointerToRawData+preSecHdr->SizeOfRawData,
                pNTHeader->OptionalHeader.FileAlignment);

    mem_offset=PEAlign(preSecHdr->VirtualAddress+preSecHdr->Misc.VirtualSize,
                pNTHeader->OptionalHeader.SectionAlignment);
    PIMAGE_SECTION_HEADER newSecHdr=new IMAGE_SECTION_HEADER;  
    memset(newSecHdr,0,(size_t)sizeof(IMAGE_SECTION_HEADER));
  
    newSecHdr->PointerToRawData=disk_offset;
    newSecHdr->VirtualAddress=mem_offset;

    newSecHdr->SizeOfRawData=disk_size;
    newSecHdr->Misc.VirtualSize=mem_size;
    newSecHdr->Characteristics=0xC0000040;
    return newSecHdr;
}

bool emitWin::AlignSection(PIMAGE_SECTION_HEADER p){
    return true;
}

bool emitWin::driver(Symtab *obj, std::string fName){
	Offset MoveAheadOffset=0;
    //get the number of new added sections
    std::vector<Region *> newregs;
    if(!obj -> getAllNewRegions(newregs)){
		log_winerror(err_func_, "No new section added, no need to drive");
        printf("No new section added, no need to drive\n");
        return false;
    }
    //printf("number of new added sections is %d\n",newregs.size());

    unsigned int n1 = NumOfAllowedSecInSectionTable();
	
    if(newregs.size() > n1){
		//no enough space in section table for new sections
		//need to use space in Dos Stub Area
		unsigned int n2= NumOfAllowedSecInDosHeader();
		if(newregs.size() > n1+n2){
			//can not accommandate all new sections even if steal all space in Dos Stub.
			log_winerror(err_func_, "no way to insert all new added sections, abort.\n");
            printf("no way to insert all new added sections, abort.\n");
            return false;
		}
		//need to move into Dos Stub Area
		isMoveAhead = true;
		//put n1 new section headers in section table, rest in dos stub
		MoveAheadOffset = SizeOfSecHeader*(newregs.size() - n1);
		//printf("move ahead in %lu bytes\n", MoveAheadOffset);

    }
    //there is enough space in section header table for new added sections
    //no need to steal space in Dos Stub Area
    else 
		isMoveAhead = false;

    //get the set of regions which include new added sections.
    std::vector<Region *> regs;
    if(!(obj -> getAllRegions(regs))) {
        printf("Failed to get regions.\n");
        log_winerror(err_func_, "Failed to get regions.\n");
        return false;
    }

    IMAGE_DOS_HEADER DosHeader;
    memcpy(&DosHeader, base_addr, sizeof(IMAGE_DOS_HEADER));
    DWORD peHdrOffset = DosHeader.e_lfanew;
    DWORD dwDosStufOffset = sizeof(IMAGE_DOS_HEADER);
    PIMAGE_NT_HEADERS NTHeader=new IMAGE_NT_HEADERS;

    memcpy(NTHeader, base_addr + DosHeader.e_lfanew, sizeof(IMAGE_NT_HEADERS));
    assert(NTHeader->Signature == IMAGE_NT_SIGNATURE);
   
    //vector of section headers
    //including the new added sections
    std::vector<PIMAGE_SECTION_HEADER> secHdrs;
    PIMAGE_NT_HEADERS pehdr = obj_nt->GetImageHeader();
   
    //pointer of 1st section header
    PIMAGE_SECTION_HEADER pScnHdr = (PIMAGE_SECTION_HEADER)(((char*)pehdr) +
                                 sizeof(DWORD) + sizeof(IMAGE_FILE_HEADER) +
                                 pehdr->FileHeader.SizeOfOptionalHeader);

    //first, the original sections
    for(unsigned int i=0; i<NTHeader->FileHeader.NumberOfSections; i++){
        PIMAGE_SECTION_HEADER p=new IMAGE_SECTION_HEADER;
        memcpy(p,pScnHdr, sizeof(IMAGE_SECTION_HEADER));
        //printf("%d, name: %s, disk_off:%x\n",i, (const char*)p->Name, p->PointerToRawData);
        secHdrs.push_back(p);
        //AlignSection(p);
        pScnHdr++;
    }
    //then, create section header for new added sections
    PIMAGE_SECTION_HEADER pre = secHdrs[secHdrs.size()-1];
    for(unsigned int i=0; i<newregs.size(); i++){
        PIMAGE_SECTION_HEADER p = CreateSecHeader(newregs[i]->getDiskSize(), pre);
        if((size_t)strlen(newregs[i]->getRegionName().c_str())> 8)
            memcpy(p->Name, newregs[i]->getRegionName().c_str(),8);
        else
            memcpy(p->Name, newregs[i]->getRegionName().c_str(),
            (size_t)strlen(newregs[i]->getRegionName().c_str()));
        secHdrs.push_back(p);
        pre = p;
    }
    //printf("size of sec headers:%d\n", secHdrs.size());
    for(unsigned int i=0; i<secHdrs.size(); i++){
        printf("%s, diskOff=%x, diskSize=%x, memoff=%x, memsize=%x\n", (const char*)secHdrs[i]->Name,secHdrs[i]->PointerToRawData, secHdrs[i]->SizeOfRawData,
            secHdrs[i]->VirtualAddress, secHdrs[i]->Misc.VirtualSize);
    }

    //calculate size of file
    DWORD dwFileSize = secHdrs[secHdrs.size()-1]->PointerToRawData + secHdrs[secHdrs.size()-1]->SizeOfRawData;
    //printf("size of file=%x", dwFileSize);


    //open a file to write the image to disk
    HANDLE hFile = CreateFileA(fName.c_str(),GENERIC_WRITE,
                               FILE_SHARE_WRITE | FILE_SHARE_READ, NULL,
                               CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if(hFile == INVALID_HANDLE_VALUE){
		log_winerror(err_func_, "Failed to open a file to write.\n");
        printf("Failed to open a file to write.\n");
        return false;
    }

    PCHAR pMem = (char*)GlobalAlloc(GMEM_FIXED|GMEM_ZEROINIT,dwFileSize);
    if(pMem == NULL){
		log_winerror(err_func_, "Failed to allocate memory space for written file\n");
        printf("Failed to allocate memory space for written file\n");
        return false;
    }

    // start to wrtie the file
	Offset writeOffset = 0;
    //retrieve the dos header from exe image
    memcpy(pMem,&DosHeader,sizeof(IMAGE_DOS_HEADER));
	writeOffset += sizeof(IMAGE_DOS_HEADER);

	//if move ahead, do adjustment for the pe pointer
	if(isMoveAhead)
		((PIMAGE_DOS_HEADER)pMem)->e_lfanew -= MoveAheadOffset;

    //write the dos stub
    PCHAR dosStub = base_addr + sizeof(IMAGE_DOS_HEADER);
	if(isMoveAhead){
		memcpy(pMem+writeOffset, dosStub, peHdrOffset-sizeof(IMAGE_DOS_HEADER)-MoveAheadOffset);
		writeOffset+=peHdrOffset-sizeof(IMAGE_DOS_HEADER)-MoveAheadOffset;
	}
	else{
    	memcpy(pMem+writeOffset, dosStub, peHdrOffset-sizeof(IMAGE_DOS_HEADER));
		writeOffset += peHdrOffset-sizeof(IMAGE_DOS_HEADER);
	}
   
    //write the PE file header
   
    //write NT header
    NTHeader->FileHeader.NumberOfSections = regs.size();
    //update SizeOfImage
    NTHeader->OptionalHeader.SizeOfImage = secHdrs[secHdrs.size()-1]->VirtualAddress + 
        secHdrs[secHdrs.size()-1]->Misc.VirtualSize;

    memcpy(pMem+writeOffset, NTHeader,sizeof(IMAGE_NT_HEADERS));
    //if bound import table is not empty, update its virtual address
    if(bit_addr != 0){
        PIMAGE_NT_HEADERS newpeHdr = (PIMAGE_NT_HEADERS)((void *)(pMem+writeOffset));
        PIMAGE_OPTIONAL_HEADER newoptHdr = (PIMAGE_OPTIONAL_HEADER)&newpeHdr -> OptionalHeader;
        printf("old address: 0x%x\n", newoptHdr->DataDirectory[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT].VirtualAddress);
        newoptHdr->DataDirectory[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT].VirtualAddress+=sizeof(IMAGE_SECTION_HEADER);
        printf("new address: 0x%x\n", newoptHdr->DataDirectory[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT].VirtualAddress);
    }
	writeOffset += sizeof(IMAGE_NT_HEADERS);

    //write section header table and section
    //DWORD dwFirstSectionHeaderOffset = peHdrOffset + sizeof(IMAGE_NT_HEADERS);
	DWORD dwFirstSectionHeaderOffset = writeOffset;

    for(unsigned int i=0; i<secHdrs.size(); i++){
        memcpy(pMem+dwFirstSectionHeaderOffset + i*sizeof(IMAGE_SECTION_HEADER),secHdrs[i],
            sizeof(IMAGE_SECTION_HEADER));
        memcpy(pMem+secHdrs[i]->PointerToRawData, regs[i]->getPtrToRawData(),regs[i]->getDiskSize());

    }

    //write bound import table info
    if(bit_addr != 0){
        memcpy(pMem+dwFirstSectionHeaderOffset + secHdrs.size()*sizeof(IMAGE_SECTION_HEADER),
            (char*)(base_addr+bit_addr), bit_size);
    }


        //WriteFile(hFile, secHdrs[i],sizeof(IMAGE_SECTION_HEADER), &dwByteWritten, NULL);
        //printf("section header%d:%lu, %lu\n", i,sizeof(IMAGE_SECTION_HEADER), dwByteWritten);
    //}   

    //write sections
    //for(unsigned int i=0; i<secHdrs.size(); i++){
    //    SetFilePointer(hFile,secHdrs[0]->PointerToRawData,NULL,FILE_BEGIN);
    //    WriteFile(hFile, regs[i]->getPtrToRawData(),regs[i]->getDiskSize(),&dwByteWritten, NULL);
    //    printf("section %d:%lu, %lu\n", i,regs[i]->getDiskSize(), dwByteWritten);
    //}
   

    //move the file pointer to beginning to write
    DWORD dwByteWritten =0;
    SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
    WriteFile(hFile, pMem, dwFileSize,&dwByteWritten, NULL);
    // ------ FORCE CALCULATED FILE SIZE ------
    SetFilePointer(hFile,dwFileSize,NULL,FILE_BEGIN);
    SetEndOfFile(hFile);
    CloseHandle(hFile);

    GlobalFree(pMem);

    return true;
}

/*
void emitWin::AlignSections(){
    PIMAGE_NT_HEADERS pNTHeader = obj_nt -> GetImageHeader();
    std::vector<PIMAGE_SECTION_HEADER> sectionHeader = obj_nt ->getSectionHeaders();
    std::vector<PIMAGE_SECTION_HEADER>::iterator iter;
   
    for(iter = sectionHeader.begin(); iter != sectionHeader.end(); iter++)
    {
        iter->VirtualAddress = PEAlign(iter->VirtualAddress,
            pNTHeader->OptionalHeader.SectionAlignment);

        iter->Misc.VirtualSize=PEAlign(iter->Misc.VirtualSize,
            pNTHeader->OptionalHeader.SectionAlignment);

        iter->PointerToRawData=PEAlign(iter->PointerToRawData,
            pNTHeader->OptionalHeader.FileAlignment);

        iter->SizeOfRawData=PEAlign(iter->SizeOfRawData,
            pNTHeader->OptionalHeader.FileAlignment);
    }
    pNTHeader->OptionalHeader.SizeOfImage=sectionHeader.end()->VirtualAddress+
        sectionHeader.end()->Misc.VirtualSize;
}
*/


/*In windows PE file, there are two alignments:
 1. SectionAlignment: the alignment of section in memory
 2. FileAlignment: the alignment of section in disk
 */
//this function is used to calculated the aligned address given dwAddr.
//which is applicable for either case.
Offset emitWin::PEAlign(Offset dwAddr,Offset dwAlign)
{   
    return(((dwAddr + dwAlign - 1) / dwAlign) * dwAlign);
}

//This function is used to calculate the maximum number of sections allowed to insert
//for the section header table, the usable space includes the usused space plus the dos stub area if any.
unsigned int emitWin::NumOfTotalAllowedSec(){
    unsigned int unusedSpaces = 0;
   
    //get the PE header pointer;
    PIMAGE_NT_HEADERS peHdrs = obj_nt -> GetImageHeader();
    //if no phdr, the
    if(peHdrs){
   
    }

    return unusedSpaces;
}

//to calculate the number of entries that can be inserted into section header table
//because of the fileAlignment, there could be unused space in section header table
unsigned int emitWin::NumOfAllowedSecInSectionTable(){

    //get base address of mapped file
    PIMAGE_NT_HEADERS peHdr = obj_nt -> GetImageHeader();
    IMAGE_OPTIONAL_HEADER optHdr = peHdr -> OptionalHeader;

    unsigned int NumOfDataDir = optHdr.NumberOfRvaAndSizes;
    //printf("NumOfDataDir: %lu\n", NumOfDataDir);
    assert( NumOfDataDir == 16);

    //retrieve information on bound import table
    //which is 12th entry (IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT) in data directory
    //Note: bound import table is right after section header table and before sections
    IMAGE_DATA_DIRECTORY bit = optHdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT];
    bit_addr = bit.VirtualAddress;
    bit_size = bit.Size;

    //printf("bound import table:0x%x, 0x%x\n", bit_addr, bit_size);

    //calculate the size of space between section header table and section
    PIMAGE_SECTION_HEADER pScnHdr = (PIMAGE_SECTION_HEADER)(((char*)peHdr) +
                                 sizeof(DWORD) + sizeof(IMAGE_FILE_HEADER) +
                                 peHdr->FileHeader.SizeOfOptionalHeader); //pointer of 1st section header
    unsigned int nSections = peHdr->FileHeader.NumberOfSections;

    DWORD dwFirstSectionHeaderOffset = ((PIMAGE_DOS_HEADER)base_addr)->e_lfanew + sizeof(IMAGE_NT_HEADERS);

    DWORD dwStuffSize = pScnHdr ->PointerToRawData - (dwFirstSectionHeaderOffset + nSections*sizeof(IMAGE_SECTION_HEADER));

    unsigned int ret = (dwStuffSize - bit_size)/SizeOfSecHeader;
    //printf("extra section headers: %lu\n", ret);
    return ret;

}

//to calculate the number of entries that can be inserted into Dos Stub area
//we don't care the Dos Stub, so just steal the space in that area for the additional section header
unsigned int emitWin::NumOfAllowedSecInDosHeader(){
	 //get base address of mapped file
    PIMAGE_NT_HEADERS peHdr = obj_nt -> GetImageHeader();
	//size of dos stub area
	unsigned int size = (char*)peHdr - (base_addr+sizeof(IMAGE_DOS_HEADER));
	unsigned int ret = size/SizeOfSecHeader;
	//printf("extra section headers in dos stub: %lu\n", ret);
	return ret;
}

void emitWin::log_winerror(void (*err_func)(const char *), const char* msg) {
    err_func(msg);
}
