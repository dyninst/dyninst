/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
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
#include "../common/src/parseauxv.h"
#include "Object.h"
#include "Object-nt.h"
#include "emitWin.h"
#include "Symtab.h"
#include <iostream>
#include <dbghelp.h>


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

// based on the dyninst section being the last section
static Region *getDyninstSection(Symtab *st)
{
   vector<Region*> regs;
   st->getAllRegions(regs);
   return regs.back();
}

static Offset copy_ILT_or_ALT(unsigned int idtIdx, 
                             vector<pair<string,IMAGE_IMPORT_DESCRIPTOR> > &idt, 
                             const vector<void*> &iltEntries, 
                             const unsigned char *ptrDyn,
                             Offset secOff, 
                             Offset ptrToWrite,
                             int addrWidth,
                             bool isILT)
{
   assert(idtIdx < idt.size());
   if (isILT) {
      idt[idtIdx].second.OriginalFirstThunk = secOff + ptrToWrite;
   } else {
      idt[idtIdx].second.FirstThunk = secOff + ptrToWrite;;
   }
   idtIdx++;

   for (vector<void*>::const_iterator eit = iltEntries.begin();
        eit != iltEntries.end(); 
        eit++)
   {
      if (0 == (*eit) && idtIdx < idt.size()) {
         if (isILT) {
            idt[idtIdx].second.OriginalFirstThunk = secOff + ptrToWrite + addrWidth;
         } else {
            idt[idtIdx].second.FirstThunk = secOff + ptrToWrite + addrWidth;
         }
         idtIdx++;
      }
      memcpy((void*)(ptrDyn + ptrToWrite), &(*eit), addrWidth);
      ptrToWrite += addrWidth;
   }
   return ptrToWrite;
}


/* Things that need to change:
 *
 * Tables
 *  - Move import descriptor table even if we're not adding new libraries 
      (orig. ILT, IAT, HNT tables are full, must point to additional entries)
 *  - Add extra import lookup table entries (per-lib ILT's are NULL terminated)
 *  - Add extra Hint/Name table entries (1 table for whole binary, but can split)
 *  - Add extra import address table entries (can't move original entries)
 *  - Add extra library names (duplicates are OK)
 * PE Header
 *  - fix Import (descriptor) table pointer
 *  - fix Import address table pointer
 */
bool emitWin::writeImpTable(Symtab* obj)
{
   bool ret = true; 

   // populate Import descriptor table with existing entries
   vector<pair<string,IMAGE_IMPORT_DESCRIPTOR> > & idt = obj_nt->getImportDescriptorTable();
   int origIDTSize = idt.size();

   //get the dyninst section
   Region *dynSec = getDyninstSection(obj);
   assert(dynSec);

   std::map<string, map<Offset, string> > & ref = obj_nt->getRefs();

   // for each new library in ref, create new IDT entry,
   // for each new function in ref, create new HNT entry
   map<string, map<string, WORD> > hnt;
   for(map<string, map<Offset, string> >::iterator lit=ref.begin(); 
       lit != ref.end(); 
       lit++)
   {
      // add IDT entry
      IMAGE_IMPORT_DESCRIPTOR newID;
      newID.FirstThunk = NULL;       // IAT offset,      don't know yet
      newID.ForwarderChain=0;
      newID.Name = NULL;             // lib name offset, don't know yet
      newID.OriginalFirstThunk=NULL; // ILT offset,      don't know yet
      newID.TimeDateStamp=0;
      idt.push_back(std::pair<string, IMAGE_IMPORT_DESCRIPTOR>(lit->first, newID));

      hnt[lit->first]; // add hnt entry for the library if needed 
      map<string, map<string, WORD> >::iterator libHNT = hnt.find(lit->first);

      // for each new function in ref, create new HNT entry
      for(map<Offset, string>::iterator fit = lit->second.begin(); 
          fit != lit->second.end(); 
          fit++)
      {
         if (libHNT->second.end() == libHNT->second.find(fit->second)) {
            // add missing HNT entry
            (libHNT->second)[fit->second] = 0; // index hint = 0
         }
      }
   }

   //create space to hold the .dyninst section and updated import table
   unsigned int numImports = 0;
   // calculate hint/name table, number of import funcs, the concatenated length of lib names
   unsigned int hntSize = 0;
   unsigned int libNameSize = 0;
   for (map<string, map<string, WORD> >::iterator hit = hnt.begin();
        hit != hnt.end();
        hit++)
   {
      numImports += hit->second.size();
      libNameSize += hit->first.size() + 1;
      for (map<string, WORD>::iterator fit = hit->second.begin();
           fit != hit->second.end();
           fit++)
      {
         int nameLen = fit->first.size();
         hntSize += 2 + nameLen + 1 + ((nameLen+1) % 2); // hint + nameLen + '\0' + padding
      }
   }
   unsigned int idtSize = sizeof(IMAGE_IMPORT_DESCRIPTOR) * (idt.size() + 1);
   unsigned int iltSize = (numImports + hnt.size()) * obj->getAddressWidth();
   unsigned int iatSize = iltSize;
   unsigned int secSize = dynSec->getDiskSize() + idtSize + iltSize + hntSize + iatSize + libNameSize;
   unsigned char* ptrDyn = (unsigned char*) GlobalAlloc(GMEM_FIXED|GMEM_ZEROINIT, secSize);

   // initialize the new .dyninst section
   Offset ptrToWrite = 0;

   // a. copy the existing section, set dynSec data pointer
   memcpy(ptrDyn + ptrToWrite,
          dynSec->getPtrToRawData(), 
          dynSec->getDiskSize());
   ptrToWrite += dynSec->getDiskSize();
   dynSec->setPtrToRawData(ptrDyn, secSize);

   // b. copy the HNT, set (ILT/IAT)->HNT pointers
   assert(sizeof(void*) == obj->getAddressWidth());
   vector<void*> iltEntries;
   for (map<string, map<string, WORD> >::iterator hit = hnt.begin();
        hit != hnt.end();
        hit++)
   {
      for (map<string, WORD>::iterator fit = hit->second.begin();
           fit != hit->second.end();
           fit++)
      {
         // set ILT pointer
         iltEntries.push_back((void*)(dynSec->getMemOffset() + ptrToWrite));

         // copy hint
         memcpy(ptrDyn + ptrToWrite,
                &(fit->second), 
                2);
         ptrToWrite += 2;

         // copy func name
         int nameLen = fit->first.size() + 1; // include the ending \0
         memcpy(ptrDyn + ptrToWrite,
                fit->first.c_str(), 
                nameLen);
         ptrToWrite += nameLen;

         // copy padding
         int padLen = nameLen % 2;
         if (padLen) { // padLen is 0 or 1, write if 1
            ptrDyn[ptrToWrite] = '\0';
            ptrToWrite += padLen;
         }
      }

      // add null entry to close library ILT
      iltEntries.push_back(0);
   }

   // c. copy the ILT, set IDT pointers
   ptrToWrite = copy_ILT_or_ALT(origIDTSize, 
                                idt, 
                                iltEntries, 
                                ptrDyn,
                                dynSec->getMemOffset(), 
                                ptrToWrite, 
                                obj->getAddressWidth(), 
                                true); 

   // d. copy the IAT (identical to ILT), set IDT pointers
   Offset iatOffset = dynSec->getMemOffset() + ptrToWrite;
   ptrToWrite = copy_ILT_or_ALT(origIDTSize, 
                                idt, 
                                iltEntries, 
                                ptrDyn,
                                dynSec->getMemOffset(), 
                                ptrToWrite, 
                                obj->getAddressWidth(), 
                                false); 
   assert(iatSize == (dynSec->getMemOffset() + ptrToWrite - iatOffset));

   // e. create list of library names, set IDT pointers
   for (int idtIdx = origIDTSize; idtIdx  != idt.size(); idtIdx++) {
      idt[idtIdx].second.Name = dynSec->getMemOffset() + ptrToWrite;
      int nameLen = idt[idtIdx].first.size() + 1; // include terminating \0
      memcpy(ptrDyn + ptrToWrite,
             idt[idtIdx].first.c_str(), 
             nameLen);
      ptrToWrite += nameLen;
   }

   // f. copy the null-terminated IDT
   Offset idtOffset = dynSec->getMemOffset() + ptrToWrite;
   for (vector<pair<string,IMAGE_IMPORT_DESCRIPTOR> >::iterator dit = idt.begin(); 
        dit != idt.end();
        dit++)
   {
      memcpy(ptrDyn + ptrToWrite,
             (void*)& dit->second, 
             sizeof(IMAGE_IMPORT_DESCRIPTOR));
      ptrToWrite += sizeof(IMAGE_IMPORT_DESCRIPTOR);
   }
   memset(ptrDyn + ptrToWrite, 0, sizeof(IMAGE_IMPORT_DESCRIPTOR));
   ptrToWrite += sizeof(IMAGE_IMPORT_DESCRIPTOR);
   assert(idtSize == (dynSec->getMemOffset() + ptrToWrite - idtOffset));

   assert(ptrToWrite == secSize);

   // fix PE header import descriptor table address & size
   if (obj_nt->getPEHdr()->OptionalHeader.NumberOfRvaAndSizes > IMAGE_DIRECTORY_ENTRY_IMPORT) {
      obj_nt->getPEHdr()->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress = idtOffset;
      obj_nt->getPEHdr()->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size = idtSize;
   } else {
      ret = false;
   }

   return ret;
} // end writeImpTable


bool emitWin::driver(Symtab *obj, std::string fName){
	//if external references exist, write the import info.
	if(!obj_nt->getRefs().empty())
		writeImpTable(obj);

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
        printf("%s, diskOff=%x, diskSize=%x, memoff=%x, memsize=%x\n", 
               (const char*)secHdrs[i]->Name,secHdrs[i]->PointerToRawData, 
               secHdrs[i]->SizeOfRawData, secHdrs[i]->VirtualAddress, 
               secHdrs[i]->Misc.VirtualSize);
    }

    //resize the last section to fit label and trap table pointer
    PIMAGE_SECTION_HEADER lastSec = secHdrs[secHdrs.size()-1];
    const int REWRITE_LABEL_SIZE = obj->getAddressWidth() + 16;
    if (obj->getObject()->trapHeader()) {
        lastSec->SizeOfRawData = PEAlign(
            lastSec->SizeOfRawData + REWRITE_LABEL_SIZE, 
            pehdr->OptionalHeader.FileAlignment);
        lastSec->Misc.VirtualSize = PEAlign(
            lastSec->SizeOfRawData + REWRITE_LABEL_SIZE, 
            pehdr->OptionalHeader.SectionAlignment);
    }

    //calculate size of file
    DWORD dwFileSize = lastSec->PointerToRawData + lastSec->SizeOfRawData;
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
    NTHeader->FileHeader.NumberOfSections = (WORD)regs.size();
    //update SizeOfImage
    NTHeader->OptionalHeader.SizeOfImage = secHdrs[secHdrs.size()-1]->VirtualAddress + 
        secHdrs[secHdrs.size()-1]->Misc.VirtualSize;

	//if move the import descriptor table, update its address
	if(!obj_nt->getRefs().empty() && NTHeader->OptionalHeader.NumberOfRvaAndSizes > IMAGE_DIRECTORY_ENTRY_IMPORT) {
       Offset newOff = obj_nt->getPEHdr()->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
       Offset newSize = obj_nt->getPEHdr()->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size;
		NTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress = newOff;
		NTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size = newSize;
        printf("new import descriptor table address: %lu size: %lu\n", newOff, newSize);
	}
	//if move the import address table, update its address
	if(!obj_nt->getRefs().empty() && NTHeader->OptionalHeader.NumberOfRvaAndSizes > IMAGE_DIRECTORY_ENTRY_IAT) {
       Offset newOff = obj_nt->getPEHdr()->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT].VirtualAddress;
       Offset newSize = obj_nt->getPEHdr()->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT].Size;
		NTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT].VirtualAddress = newOff;
		NTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT].Size = newSize;
		printf("new import address table address: %lu size: %lu\n", newOff, newSize);
	}

    memcpy(pMem+writeOffset, NTHeader,sizeof(IMAGE_NT_HEADERS));
    //if bound import table is not empty, update its virtual address
    if(bit_addr != 0){
        assert(NTHeader->OptionalHeader.NumberOfRvaAndSizes > IMAGE_DIRECTORY_ENTRY_IMPORT);
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
    unsigned int numSecns = secHdrs.size();
    for(unsigned int i=0; i<numSecns; i++){
        memcpy(pMem+dwFirstSectionHeaderOffset + i*sizeof(IMAGE_SECTION_HEADER),secHdrs[i],
            sizeof(IMAGE_SECTION_HEADER));
        memcpy(pMem+secHdrs[i]->PointerToRawData, regs[i]->getPtrToRawData(),regs[i]->getDiskSize());
    }

    // copy trap-table header & DYNINST_REWRITE to end of last section
    Address trapHead = obj->getObject()->trapHeader();
    Address writeTarg = (Address) (pMem 
        + lastSec->PointerToRawData 
        + lastSec->SizeOfRawData 
        - REWRITE_LABEL_SIZE);
    memcpy((void*)writeTarg, (void*) &trapHead, obj->getAddressWidth());
    memcpy((void*)(writeTarg + obj->getAddressWidth()), "DYNINST_REWRITE", 16);

    //write bound import table info
    if(bit_addr != 0){
        memcpy(pMem+dwFirstSectionHeaderOffset + secHdrs.size()*sizeof(IMAGE_SECTION_HEADER),
            (char*)(base_addr+bit_addr), bit_size);
    }   

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
