/*
 * Copyright (c) 1996-2007 Barton P. Miller
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

/*
#if !defined(cap_libelf_so_0) && defined(os_linux)
#define _FILE_OFFSET_BITS 64
#endif
*/

#include "emitElf-64.h"
#include "Symtab.h"

extern void pd_log_perror(const char *msg);
using namespace Dyninst;
using namespace Dyninst::SymtabAPI;
using namespace std;

extern const char *pdelf_get_shnames(Elf_X &elf);
unsigned elf64_newdynstrIndex;
unsigned elf64_newdynsymIndex;
extern unsigned int elfHash(const char *name);

static int elfSymType(Symbol::SymbolType sType)
{
  switch (sType) {
    case Symbol::ST_MODULE: return STT_FILE;
    case Symbol::ST_OBJECT: return STT_OBJECT;
    case Symbol::ST_FUNCTION: return STT_FUNC;
    case Symbol::ST_NOTYPE : return STT_NOTYPE;
    default: return STT_SECTION;
  }
}

static int elfSymBind(Symbol::SymbolLinkage sLinkage)
{
  switch (sLinkage) {
    case Symbol::SL_LOCAL: return STB_LOCAL;
    case Symbol::SL_WEAK: return STB_WEAK;
    case Symbol::SL_GLOBAL: return STB_GLOBAL;
    default: return STB_LOPROC;
  }
}

emitElf64::emitElf64(Elf_X &oldElfHandle_, bool isStripped_, int BSSexpandflag_, void (*err_func)(const char *)) :
   oldElfHandle(oldElfHandle_), BSSExpandFlag(BSSexpandflag_), isStripped(isStripped_), err_func_(err_func)

{
   firstNewLoadSec = NULL;
   textData = NULL;
   symStrData = NULL;
   dynStrData = NULL;
   symTabData = NULL;
   hashData = NULL;
   dynsymData = NULL;
   rodata = NULL;
   dataData = NULL;
   
   if(BSSexpandflag_)
       addNewSegmentFlag = false;
   else    
       addNewSegmentFlag = true;
   oldElf = oldElfHandle.e_elfp();
   curVersionNum = 2;
}

bool emitElf64::getBackSymbol(Symbol *symbol, vector<string> &symbolStrs, unsigned &symbolNamesLength, vector<Elf64_Sym *> &symbols, bool dynSymFlag)
{
   Elf64_Sym *sym = new Elf64_Sym();
   sym->st_name = symbolNamesLength;
   symbolStrs.push_back(symbol->getName());
   symbolNamesLength += symbol->getName().length()+1;
   sym->st_value = symbol->getAddr();
   sym->st_size = symbol->getSize();
   sym->st_other = 0;
   sym->st_info = ELF64_ST_INFO(elfSymBind(symbol->getLinkage()), elfSymType (symbol->getType()));
   if(symbol->getSec())
	sym->st_shndx = symbol->getSec()->getRegionNumber();
   else if(symbol->getType() == Symbol::ST_MODULE || symbol->getType() == Symbol::ST_NOTYPE)
        sym->st_shndx = SHN_ABS;
   
   symbols.push_back(sym);
   //Do not emit aliases for dynamic symbols
   if(dynSymFlag){

#if !defined(os_solaris)
       string fileName;
       if(symbol->getLinkage() == Symbol::SL_WEAK){
           versionSymTable.push_back(0);
           return true;
       }
       
       if(!symbol->getVersionFileName(fileName))   //verdef entry
       {
           vector<string> *vers;
           if(!symbol->getVersions(vers))
                versionSymTable.push_back(1);
           else {
               if(vers->size() > 1){
                   if(versionNames.find((*vers)[0]) == versionNames.end())
                       versionNames[(*vers)[0]] = 0;
                   if(verdefEntries.find((*vers)[0]) != verdefEntries.end())
                        versionSymTable.push_back(verdefEntries[(*vers)[0]]);
                   else{
                       versionSymTable.push_back(curVersionNum);
                       verdefEntries[(*vers)[0]] = curVersionNum;
                       curVersionNum++;
                   }
               }
               for(unsigned i=1; i< vers->size(); i++){
                   if(versionNames.find((*vers)[i]) == versionNames.end())
                       versionNames[(*vers)[i]] = 0;
                   verdauxEntries[verdefEntries[(*vers)[0]]].push_back((*vers)[i]);
               }
           }
       }
       else {           //verneed entry
           vector<string> *vers;
           if(!symbol->getVersions(vers))
                versionSymTable.push_back(1);
           else{
               if(vers->size() == 1){        // There should only be one version string
                    //If the verison name already exists then add the same version number to the version symbol table
                    //Else give a new number and add it to the mapping.
                    if(versionNames.find((*vers)[0]) == versionNames.end())
                        versionNames[(*vers)[0]] = 0;
                    if(verneedEntries.find(fileName) != verneedEntries.end())
                    {
                        if(verneedEntries[fileName].find((*vers)[0]) != verneedEntries[fileName].end())
                            versionSymTable.push_back(verneedEntries[fileName][(*vers)[0]]);
                        else{
                            versionSymTable.push_back(curVersionNum);
                            verneedEntries[fileName][(*vers)[0]] = curVersionNum;
                            curVersionNum++;
                        }
                    }
                    else{
                        versionSymTable.push_back(curVersionNum);
                        verneedEntries[fileName][(*vers)[0]] = curVersionNum;
                        curVersionNum++;
                    }
                }
           }
       }
#endif
       return true;
   }
   std::vector<string> names = symbol->getAllMangledNames();
   for(unsigned i=1;i<names.size();i++)
   {
       	sym = new Elf64_Sym();
	    sym->st_name = symbolNamesLength;
    	symbolStrs.push_back(names[i]);
       	symbolNamesLength += names[i].length()+1;
    	sym->st_value = symbol->getAddr();
    	sym->st_size = 0;
       	sym->st_other = 0;
       	sym->st_info = ELF64_ST_INFO(elfSymBind(symbol->getLinkage()), elfSymType (symbol->getType()));
       	if(symbol->getSec())
    	    sym->st_shndx = symbol->getSec()->getRegionNumber();
    	else if(symbol->getType() == Symbol::ST_MODULE || symbol->getType() == Symbol::ST_NOTYPE)
    	    sym->st_shndx = SHN_ABS;
       	symbols.push_back(sym);
   }
   return true;
}

// Find the end of data/text segment
void emitElf64::findSegmentEnds()
{
    Elf64_Phdr *tmp = elf64_getphdr(oldElf);
    // Find the offset of the start of the text & the data segment
    // The first LOAD segment is the text & the second LOAD segment 
    // is the data
    int flag = 1;
    for(unsigned i=0;i<oldEhdr->e_phnum;i++)
    {
        if(tmp->p_type == PT_LOAD && flag == 1){
	    textSegEnd = tmp->p_vaddr + tmp->p_memsz;
        flag = 2;
	    tmp++;
	    continue;
	}
        if(tmp->p_type == PT_LOAD && flag == 2)
        {
            dataSegEnd = tmp->p_vaddr+tmp->p_memsz;
            break;
        }
        tmp++;
    }
}

bool emitElf64::driver(Symtab *obj, string fName){
    int newfd;
    Region *foundSec;
    unsigned pgSize = getpagesize();

    //open ELf File for writing
    if((newfd = (open(fName.c_str(), O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IXGRP)))==-1){ 
        log_elferror(err_func_, "error opening file to write symbols");
        return false;
    }
    if((newElf = elf_begin(newfd, ELF_C_WRITE, NULL)) == NULL){
        log_elferror(err_func_, "NEWELF_BEGIN_FAIL");
        fflush(stdout);
        return false;
    }
   
    //Section name index for all sections
    secNames.push_back("");
    secNameIndex = 1;
    //Section name index for new sections
    unsigned loadSecTotalSize = 0;
    unsigned NOBITStotalsize = 0;
    int dirtySecsChange = 0;
    unsigned extraAlignSize = 0;
    unsigned olddynstrIndex;
    unsigned olddynsymIndex;
    
    // ".shstrtab" section: string table for section header names
    const char *shnames = pdelf_get_shnames(oldElfHandle);
    if (shnames == NULL) {
        log_elferror(err_func_, ".shstrtab section");
    	return false;
    }	

    // Write the Elf header first!
    newEhdr= elf64_newehdr(newElf);
    if(!newEhdr){
        log_elferror(err_func_, "newEhdr failed\n");
        return false;
    }
    oldEhdr = elf64_getehdr(oldElf);
    memcpy(newEhdr, oldEhdr, sizeof(Elf64_Ehdr));
    
    newEhdr->e_shnum += newSecs.size();

    // Find the end of text and data segments
    findSegmentEnds();
    unsigned insertPoint = oldEhdr->e_shnum;
    unsigned NOBITSstartPoint = oldEhdr->e_shnum;

    if(addNewSegmentFlag)
    {
        newEhdr->e_phoff = sizeof(Elf64_Ehdr);
    }
    
    /* flag the file for no auto-layout */
    elf_flagelf(newElf,ELF_C_SET,ELF_F_LAYOUT);
    
    Elf_Scn *scn = NULL, *newscn;
    Elf_Data *newdata = NULL, *olddata = NULL;
    Elf64_Shdr *newshdr, *shdr;
    
    unsigned scncount;
    for (scncount = 0; (scn = elf_nextscn(oldElf, scn)); scncount++) {

    	//copy sections from oldElf to newElf
        shdr = elf64_getshdr(scn);
    	// resolve section name
        const char *name = &shnames[shdr->sh_name];
	    obj->findRegion(foundSec, name);
        // write the shstrtabsection at the end
        if(!strcmp(name, ".shstrtab"))
            continue;

    	newscn = elf_newscn(newElf);
    	newshdr = elf64_getshdr(newscn);
    	newdata = elf_newdata(newscn);
    	olddata = elf_getdata(scn,NULL);
    	memcpy(newshdr, shdr, sizeof(Elf64_Shdr));
    	memcpy(newdata,olddata, sizeof(Elf_Data));

        secNames.push_back(name);
        newshdr->sh_name = secNameIndex;
        secNameIndex += strlen(name) + 1;
    
    	if(foundSec->isDirty())
    	{
    	    newdata->d_buf = (char *)malloc(foundSec->getDiskSize());
    	    memcpy(newdata->d_buf, foundSec->getPtrToRawData(), foundSec->getDiskSize());
    	    newdata->d_size = foundSec->getDiskSize();
    	    newshdr->sh_size = foundSec->getDiskSize();
    	}
    	else if(olddata->d_buf)     //copy the data buffer from oldElf
    	{
            newdata->d_buf = (char *)malloc(olddata->d_size);
            memcpy(newdata->d_buf, olddata->d_buf, olddata->d_size);
    	}
	
	    if(BSSExpandFlag) {
	        // Add the expanded SHT_NOBITS section size if the section comes after those sections 
    	    if(scncount > NOBITSstartPoint)
    		newshdr->sh_offset += NOBITStotalsize;
	
	        // Expand the NOBITS sections in file & and change the type from SHT_NOBITS to SHT_PROGBITS
    	    if(shdr->sh_type == SHT_NOBITS)
    	    {
    	    	newshdr->sh_type = SHT_PROGBITS;
    	    	newdata->d_buf = (char *)malloc(shdr->sh_size);
    	    	memset(newdata->d_buf, '\0', shdr->sh_size);
    	    	newdata->d_size = shdr->sh_size;
    	    	if(NOBITSstartPoint == oldEhdr->e_shnum)
        		    NOBITSstartPoint = scncount;
    	        NOBITStotalsize += shdr->sh_size; 
	        }
    	}    
        
    	if(!strcmp(name,".strtab"))
    	{
    	    symStrData = newdata;
	        updateSymbols(symTabData, symStrData, loadSecTotalSize);
    	}
	    if(!strcmp(name, ".dynstr")){
            //Change the type of the original dynstr section if we are changing it.
    	    newshdr->sh_type = SHT_PROGBITS;
            olddynStrData = (char *)olddata->d_buf;
            dynStrData = newdata;
            olddynstrIndex = scncount+1;
            //updateSymbols(dynsymData, dynStrData);
        }
    	//Change sh_link for .symtab to point to .strtab
        if(!strcmp(name, ".symtab")){
            newshdr->sh_link = secNames.size();
            symTabData = newdata;
        }
        if(!strcmp(name, ".dynsym")){
            //Change the type of the original dynsym section if we are changing it.
    	    newshdr->sh_type = SHT_PROGBITS;
            dynsymData = newdata;
            olddynsymIndex = scncount+1;
        }
        if(!strcmp(name, ".gnu.version")){
            newshdr->sh_type = SHT_PROGBITS;
        }
        if(!strcmp(name, ".gnu.version_r")){
            newshdr->sh_type = SHT_PROGBITS;
        }
        if(!strcmp(name, ".gnu.version_d")){
            newshdr->sh_type = SHT_PROGBITS;
        }
    	if(!strcmp(name, ".text")){
            textData = newdata;
        }
        if(!strcmp(name, ".data")){
    	    dataData = newdata;
	    }
    	// Change offsets of sections based on the newly added sections
    	if(addNewSegmentFlag)
	        newshdr->sh_offset += pgSize;
    	if(scncount > insertPoint)
	        newshdr->sh_offset += loadSecTotalSize;
	    newshdr->sh_offset += (int) (dirtySecsChange + extraAlignSize);
        if(foundSec->isDirty()) 
            dirtySecsChange += newshdr->sh_size - shdr->sh_size;
        if(BSSExpandFlag && newshdr->sh_addr){
            unsigned newOff = newshdr->sh_offset - (newshdr->sh_offset & (pgSize-1)) + (newshdr->sh_addr & (pgSize-1));
            if(newOff < newshdr->sh_offset)
                newOff += pgSize;
            extraAlignSize += newOff - newshdr->sh_offset;
            newshdr->sh_offset = newOff;
        }
	    /* DEBUG */
    	fprintf(stderr, "Added Section %d(%s): secAddr 0x%lx, secOff 0x%lx, secsize 0x%lx, end 0x%lx\n",
	                        scncount, name, newshdr->sh_addr, newshdr->sh_offset, newshdr->sh_size, newshdr->sh_offset + newshdr->sh_size );

	    //Insert new loadable sections at the end of data segment			
    	if(shdr->sh_addr+shdr->sh_size == dataSegEnd){
	        insertPoint = scncount;
            if(!createLoadableSections(newshdr, loadSecTotalSize, extraAlignSize))
	        	return false;
        }

        if(!strcmp(name, ".dynamic")){
            dynData = newdata;
            dynSegOff = newshdr->sh_offset;
            dynSegAddr = newshdr->sh_addr;
            // Change the data to update the relocation addr
            newshdr->sh_type = SHT_PROGBITS;
            //newSecs.push_back(new Section(oldEhdr->e_shnum+newSecs.size(),".dynamic", /*addr*/, newdata->d_size, dynData, Section::dynamicSection, true));
	    }

	    elf_update(newElf, ELF_C_NULL);
    }

    // Add non-loadable sections at the end of object file
    if(!createNonLoadableSections(newshdr))
    	return false;
   
    //Add the section header table right at the end        
    addSectionHeaderTable(newshdr);

    // Second iteratioon to fix the link fields to point to the correct section
    scn = NULL;
    for (scncount = 0; (scn = elf_nextscn(newElf, scn)); scncount++)
    	shdr = elf64_getshdr(scn);
    newEhdr->e_shstrndx = scncount;

    // Move the section header to the end
    newEhdr->e_shoff =shdr->sh_offset+shdr->sh_size;
/*    if(addNewSegmentFlag)
        newEhdr->e_shoff += pgSize;
*/
    //copy program headers
    oldPhdr = elf64_getphdr(oldElf);
    fixPhdrs(loadSecTotalSize, extraAlignSize);

    //Write the new Elf file
    if (elf_update(newElf, ELF_C_WRITE) < 0){
        int err;
	    if ((err = elf_errno()) != 0)
       	{
	    	const char *msg = elf_errmsg(err);
    		/* print msg */
    		fprintf(stderr, "Error: Unable to write ELF file: %s\n", msg);
	    }
        log_elferror(err_func_, "elf_update failed");
        return false;
    }
    elf_end(newElf);
    close(newfd);

    return true;
}

void emitElf64::fixPhdrs(unsigned &loadSecTotalSize, unsigned &extraAlignSize)
{
    unsigned pgSize = getpagesize();
    Elf64_Phdr *tmp = oldPhdr;
    if(addNewSegmentFlag) {
        if(firstNewLoadSec)
            newEhdr->e_phnum= oldEhdr->e_phnum + 1;
        else
    	    newEhdr->e_phnum= oldEhdr->e_phnum;
    }
    if(BSSExpandFlag)
       newEhdr->e_phnum= oldEhdr->e_phnum;
    
    newPhdr=elf64_newphdr(newElf,newEhdr->e_phnum);

    Elf64_Phdr newSeg;
    for(unsigned i=0;i<oldEhdr->e_phnum;i++)
    {
        memcpy(newPhdr, tmp, oldEhdr->e_phentsize);
    	// Expand the data segment to include the new loadable sections
    	// Also add a executable permission to the segment
        if(tmp->p_type == PT_DYNAMIC){
            newPhdr->p_vaddr = dynSegAddr;
            newPhdr->p_paddr = dynSegAddr;
            newPhdr->p_offset = dynSegOff;
            newPhdr->p_memsz = dynSegSize;
            newPhdr->p_filesz = newPhdr->p_memsz;
        }
    	if(BSSExpandFlag) {
    	    if(tmp->p_type == PT_LOAD && (tmp->p_flags == 6 || tmp->p_flags == 7))
    	    {
	        	newPhdr->p_memsz += loadSecTotalSize + extraAlignSize;
    	    	newPhdr->p_filesz = newPhdr->p_memsz;
    	    	newPhdr->p_flags = 7;
	        }	
    	}    
	    if(addNewSegmentFlag) {
	        if(tmp->p_type == PT_LOAD && tmp->p_flags == 5)
            {
    	        newPhdr->p_vaddr = tmp->p_vaddr - pgSize;
        		newPhdr->p_paddr = newPhdr->p_vaddr;
        		newPhdr->p_filesz += pgSize;
        		newPhdr->p_memsz = newPhdr->p_filesz;
	        }
            if(tmp->p_type == PT_PHDR){
    	        newPhdr->p_vaddr = tmp->p_vaddr - pgSize;
        		newPhdr->p_paddr = newPhdr->p_vaddr;
            }
    	    if(tmp->p_type == PT_LOAD && tmp->p_flags == 6 || tmp->p_type == PT_NOTE || tmp->p_type == PT_INTERP)
	            newPhdr->p_offset += pgSize;
    	} 
    	fprintf(stderr, "Added New program header : offset 0x%lx,addr 0x%lx\n", newPhdr->p_offset, newPhdr->p_vaddr);
	    newPhdr++;
    	if(addNewSegmentFlag) {
	        if(tmp->p_type == PT_LOAD && (tmp->p_flags == 6 || tmp->p_flags == 7) && firstNewLoadSec)
            {
       	    	newSeg.p_type = PT_LOAD;
    	    	newSeg.p_offset = firstNewLoadSec->sh_offset;
        	   	newSeg.p_vaddr = newSegmentStart;
        	 	newSeg.p_paddr = newSeg.p_vaddr;
	        	newSeg.p_filesz = loadSecTotalSize - (newSegmentStart - firstNewLoadSec->sh_addr);
	    	    newSeg.p_memsz = newSeg.p_filesz;
    	    	newSeg.p_flags = PF_R+PF_W+PF_X;
	        	newSeg.p_align = tmp->p_align;
            	memcpy(newPhdr, &newSeg, oldEhdr->e_phentsize);
    	    	newPhdr++;
            }
	    }    
        tmp++;
    }
}

#if !defined(os_solaris)
//This method updates the .dynamic section to reflect the changes to the relocation section
void emitElf64::updateDynamic(unsigned tag, Elf64_Addr val){
    if(dynamicSecData.find(tag) == dynamicSecData.end())
        return;
    unsigned dtneeded = 1;
    
    switch(dynamicSecData[tag][0]->d_tag){
        case DT_STRSZ:
            dynamicSecData[tag][0]->d_un.d_val = val;
            break;
        case DT_SYMTAB:
        case DT_STRTAB:
        case DT_REL:
        case DT_RELA:
        case DT_JMPREL:
        case DT_VERSYM:
            dynamicSecData[tag][0]->d_un.d_ptr = val;
            break;
        case DT_VERNEED:
            dynamicSecData[tag][0]->d_un.d_ptr = val;
            dynamicSecData[DT_VERNEEDNUM][0]->d_un.d_val = verneednum;
            break;
        case DT_VERDEF:
            dynamicSecData[tag][0]->d_un.d_ptr = val;
            dynamicSecData[DT_VERDEFNUM][0]->d_un.d_val = verneednum;
            break;
    }
}
#endif    

//This method updates the symbol table,
//it shifts each symbol address as necessary AND
//sets _end and _END_ to move the heap
void emitElf64::updateSymbols(Elf_Data* symtabData,Elf_Data* strData, unsigned long loadSecsSize){
    if( symtabData && strData && loadSecsSize){
        Elf64_Sym *symPtr=(Elf64_Sym*)symtabData->d_buf;
        for(unsigned int i=0;i< symtabData->d_size/(sizeof(Elf64_Sym));i++,symPtr++){
            if(!(strcmp("_end", (char*) strData->d_buf + symPtr->st_name))){
    	        //newHeapAddrIncr = newHeapAddr - symPtr->st_value ;
                symPtr->st_value += loadSecsSize;
            }
            if(!(strcmp("_END_", (char*) strData->d_buf + symPtr->st_name))){
	            //newHeapAddrIncr = newHeapAddr - symPtr->st_value ;
	            symPtr->st_value += loadSecsSize;
            }
    	}    
    }
}

bool emitElf64::createLoadableSections(Elf64_Shdr *shdr, unsigned &loadSecTotalSize, unsigned &extraAlignSize)
{
    Elf_Scn *newscn;
    Elf_Data *newdata = NULL;
    Elf64_Shdr *newshdr;
    firstNewLoadSec = NULL;
    unsigned pgSize = getpagesize();
    unsigned strtabIndex = 0;
    unsigned dynsymIndex = 0;
    Elf64_Shdr *prevshdr = NULL;

    for(unsigned i=0; i < newSecs.size(); i++)
    {
    	if(newSecs[i]->isLoadable())
    	{
	        secNames.push_back(newSecs[i]->getRegionName());
            if(newSecs[i]->getRegionName() == ".dynstr")
                elf64_newdynstrIndex = secNames.size() - 1;
            else if(newSecs[i]->getRegionName() == ".dynsym")
                elf64_newdynsymIndex = secNames.size() - 1;
    
            // Add a new loadable section
	        if((newscn = elf_newscn(newElf)) == NULL)
    	    {
	            log_elferror(err_func_, "unable to create new function");	
 	            return false;
    	    }	
   	        if ((newdata = elf_newdata(newscn)) == NULL)
	        {
                log_elferror(err_func_, "unable to create section data");	
           		return false;
	        } 
            memset(newdata, 0, sizeof(Elf_Data));

    	    // Fill out the new section header	
	        newshdr = elf64_getshdr(newscn);
	        newshdr->sh_name = secNameIndex;
    	    newshdr->sh_flags = 0;
            switch(newSecs[i]->getRegionType()){
                case Region::RT_TEXTDATA:
                    newshdr->sh_flags = SHF_EXECINSTR | SHF_ALLOC | SHF_WRITE;
                    break;
                case Region::RT_TEXT:
                    newshdr->sh_flags = SHF_EXECINSTR | SHF_ALLOC;
                    break;
                case Region::RT_DATA:
                    newshdr->sh_flags = SHF_WRITE | SHF_ALLOC;
                    break;
                default:
                    break;
            }
    	    newshdr->sh_type = SHT_PROGBITS;

            // TODO - compute the correct offset && address. This is wrong!!
	        if(shdr->sh_type == SHT_NOBITS)
	        	newshdr->sh_offset = shdr->sh_offset;
    	    else
	        	newshdr->sh_offset = shdr->sh_offset+shdr->sh_size;
            if(newSecs[i]->getDiskOffset())
    	        newshdr->sh_addr = newSecs[i]->getDiskOffset();
            else{
                newshdr->sh_addr = prevshdr->sh_addr+ prevshdr->sh_size;
            }
    	    
            newshdr->sh_link = SHN_UNDEF;
	        newshdr->sh_info = 0;
	        newshdr->sh_addralign = 4;
       	    newshdr->sh_entsize = 0;
            
            if(newSecs[i]->getRegionType() == Region::RT_REL)    //Relocation section
            {
                newshdr->sh_type = SHT_REL;
                newshdr->sh_flags = SHF_ALLOC;
                newshdr->sh_entsize = sizeof(Elf64_Rel);
                newshdr->sh_link = dynsymIndex;   //.rel.plt section should have sh_link = index of .dynsym
                newdata->d_type = ELF_T_REL;
	            newdata->d_align = 4;
#if !defined(os_solaris)
                if(newSecs[i]->getRegionName() == ".rel.plt")
                    updateDynamic(DT_JMPREL, newshdr->sh_addr);
                else
                    updateDynamic(DT_REL, newshdr->sh_addr);
#endif
            }
            else if(newSecs[i]->getRegionType() == Region::RT_STRTAB)    //String table Section
            {
                newshdr->sh_type = SHT_STRTAB;
                newshdr->sh_entsize = 1;
                newdata->d_type = ELF_T_BYTE;
                newshdr->sh_link = SHN_UNDEF;
                newshdr->sh_flags=  SHF_ALLOC;
	            newdata->d_align = 1;
                strtabIndex = secNames.size()-1;
                newshdr->sh_addralign = 1;
#if !defined(os_solaris)
                updateDynamic(DT_STRTAB, newshdr->sh_addr);
                updateDynamic(DT_STRSZ, newSecs[i]->getDiskSize());
#endif
            }
            else if(newSecs[i]->getRegionType() == Region::RT_SYMTAB)
            {
                newshdr->sh_type = SHT_DYNSYM;
                newshdr->sh_entsize = sizeof(Elf64_Sym);
                newdata->d_type = ELF_T_SYM;
	            newdata->d_align = 4;
                newshdr->sh_link = secNames.size();   //.symtab section should have sh_link = index of .strtab for .dynsym
                newshdr->sh_flags = SHF_ALLOC ;
                dynsymIndex = secNames.size()-1;
#if !defined(os_solaris)
                updateDynamic(DT_SYMTAB, newshdr->sh_addr);
#endif
            }
            else if(newSecs[i]->getRegionType() == Region::RT_DYNAMIC)
            {
#if !defined(os_solaris)
                newshdr->sh_entsize = sizeof(Elf64_Dyn);
#endif            
                newshdr->sh_type = SHT_DYNAMIC;
                newdata->d_type = ELF_T_DYN;
	            newdata->d_align = 4;
                newshdr->sh_link = strtabIndex;   //.dynamic section should have sh_link = index of .strtab for .dynsym
                newshdr->sh_flags=  SHF_ALLOC | SHF_WRITE;
                dynSegOff = newshdr->sh_offset;
                dynSegAddr = newshdr->sh_addr;
                dynSegSize = newSecs[i]->getDiskSize();
            }
#if !defined(os_solaris)
            else if(newSecs[i]->getRegionType() == Region::RT_SYMVERSIONS)
            {
                newshdr->sh_type = SHT_GNU_versym;
                newshdr->sh_entsize = sizeof(Elf64_Half);
                newshdr->sh_addralign = 2;
                newdata->d_type = ELF_T_HALF;
	            newdata->d_align = 2;
                newshdr->sh_link = dynsymIndex;   //.symtab section should have sh_link = index of .strtab for .dynsym
                newshdr->sh_flags = SHF_ALLOC ;
                updateDynamic(DT_VERSYM, newshdr->sh_addr);
            }
            else if(newSecs[i]->getRegionType() == Region::RT_SYMVERNEEDED)
            {
                newshdr->sh_type = SHT_GNU_verneed;
                newshdr->sh_entsize = 0;
                newshdr->sh_addralign = 4;
                newdata->d_type = ELF_T_VNEED;
	            newdata->d_align = 4;
                newshdr->sh_link = strtabIndex;   //.symtab section should have sh_link = index of .strtab for .dynsym
                newshdr->sh_flags = SHF_ALLOC ;
                updateDynamic(DT_VERNEED, newshdr->sh_addr);
            }
            else if(newSecs[i]->getRegionType() == Region::RT_SYMVERDEF)
            {
                newshdr->sh_type = SHT_GNU_verdef;
                newshdr->sh_entsize = 0;
                newdata->d_type = ELF_T_VDEF;
	            newdata->d_align = 4;
                newshdr->sh_link = strtabIndex;   //.symtab section should have sh_link = index of .strtab for .dynsym
                newshdr->sh_flags = SHF_ALLOC ;
                updateDynamic(DT_VERDEF, newshdr->sh_addr);
            }
#endif

    	    if(addNewSegmentFlag)
	        {
	            // Check to make sure the (vaddr for the start of the new segment - the offset) is page aligned
    	        if(!firstNewLoadSec)
	            {
                    newSegmentStart = newshdr->sh_addr;
                    Offset newoff = newshdr->sh_offset  - (newshdr->sh_offset & (pgSize-1)) + (newshdr->sh_addr & (pgSize-1));
                    if(newoff < newshdr->sh_offset)
                        newoff += pgSize;
                    extraAlignSize += newoff - newshdr->sh_offset;
                    newshdr->sh_offset = newoff;

                	/* // Address or Offset
                    newSegmentStart = newshdr->sh_addr  - (newshdr->sh_addr & (pgSize-1)) + (newshdr->sh_offset & (pgSize-1));
    		        if(newSegmentStart < newshdr->sh_addr)
    	            {
	    	            newSegmentStart += pgSize;
        		        extraAlignSize += newSegmentStart - newshdr->sh_addr;
        		        newshdr->sh_addr = newSegmentStart;
	        	    } 
                    */
		        }    
    	    }	
            else{
                Offset newoff = newshdr->sh_offset  - (newshdr->sh_offset & (pgSize-1)) + (newshdr->sh_addr & (pgSize-1));
                if(newoff < newshdr->sh_offset)
                    newoff += pgSize;
                extraAlignSize += newoff - newshdr->sh_offset;
                newshdr->sh_offset = newoff;
            }
            // Why is this being done -giri??	
            // newshdr->sh_offset = shdr->sh_offset;

    	    //Set up the data
            newdata->d_buf = malloc(newSecs[i]->getDiskSize());
    	    memcpy(newdata->d_buf, newSecs[i]->getPtrToRawData(), newSecs[i]->getDiskSize());
            newdata->d_off = 0;
	        newdata->d_size = newSecs[i]->getDiskSize();
	        newshdr->sh_size = newdata->d_size;
    	    loadSecTotalSize += newshdr->sh_size;
	    
    	    newdata->d_version = 1;

    	    elf_update(newElf, ELF_C_NULL);

	        shdr = newshdr;
    	    if(!firstNewLoadSec)
	        	firstNewLoadSec = shdr;
            secNameIndex += newSecs[i]->getRegionName().size() + 1;
	        /* DEBUG */
            fprintf(stderr, "Added New Section(%s) : secAddr 0x%lx, secOff 0x%lx, secsize 0x%lx, end 0x%lx\n",
                    newSecs[i]->getRegionName().c_str(), newshdr->sh_addr, newshdr->sh_offset, newshdr->sh_size, newshdr->sh_offset + newshdr->sh_size );
            prevshdr = newshdr;
	    }
	    else
	        nonLoadableSecs.push_back(newSecs[i]);
    }	
    return true;
}
	
bool emitElf64::addSectionHeaderTable(Elf64_Shdr *shdr) {
   Elf_Scn *newscn;
   Elf_Data *newdata = NULL;
   Elf64_Shdr *newshdr;
   
   if((newscn = elf_newscn(newElf)) == NULL)
   {
      log_elferror(err_func_, "unable to create new function");	
      return false;
	}	
   if ((newdata = elf_newdata(newscn)) == NULL)
   {
      log_elferror(err_func_, "unable to create section data");	
      return false;
	} 
   //Fill out the new section header
   newshdr = elf64_getshdr(newscn);
   newshdr->sh_name = secNameIndex;
   secNames.push_back(".shstrtab");
   secNameIndex += 10;
   newshdr->sh_type = SHT_STRTAB;
   newshdr->sh_entsize = 1;
   newdata->d_type = ELF_T_BYTE;
   newshdr->sh_link = SHN_UNDEF; 
   newshdr->sh_flags=  0;

   newshdr->sh_offset = shdr->sh_offset+shdr->sh_size;
   newshdr->sh_addr = 0;
   newshdr->sh_info = 0;
   newshdr->sh_addralign = 4;
   
   //Set up the data
   newdata->d_buf = (char *)malloc(secNameIndex);
   char *ptr = (char *)newdata->d_buf;
   for(unsigned i=0;i<secNames.size(); i++)
	{
	    memcpy(ptr, secNames[i].c_str(), secNames[i].length());
        memcpy(ptr+secNames[i].length(), "\0", 1);
	    ptr += secNames[i].length()+1;
    }    
    
    newdata->d_size = secNameIndex;
   	newshdr->sh_size = newdata->d_size;
    fprintf(stderr, "Added New Section(%s) : secAddr 0x%lx, secOff 0x%lx, secsize 0x%lx, end 0x%lx\n",
                                    ".shstrtab", newshdr->sh_addr, newshdr->sh_offset, newshdr->sh_size, newshdr->sh_offset + newshdr->sh_size );
    //elf_update(newElf, ELF_C_NULL);
   
   newdata->d_align = 4;
   newdata->d_version = 1;
   return true;
}

bool emitElf64::createNonLoadableSections(Elf64_Shdr *&shdr)
{
    Elf_Scn *newscn;
    Elf_Data *newdata = NULL;
    Elf64_Shdr *newshdr;
    
    Elf64_Shdr *prevshdr = shdr; 
    //All of them that are left are non-loadable. stack'em up at the end.
    for(unsigned i = 0; i < nonLoadableSecs.size(); i++)
    {
        secNames.push_back(nonLoadableSecs[i]->getRegionName());
        // Add a new non-loadable section
        if((newscn = elf_newscn(newElf)) == NULL)
        {
            log_elferror(err_func_, "unable to create new function");	
            return false;
        }	
        if ((newdata = elf_newdata(newscn)) == NULL)
        {
            log_elferror(err_func_, "unable to create section data");	
            return false;
        } 

        //Fill out the new section header
        newshdr = elf64_getshdr(newscn);
        newshdr->sh_name = secNameIndex;
        secNameIndex += nonLoadableSecs[i]->getRegionName().length() + 1;
        if(nonLoadableSecs[i]->getRegionType() == Region::RT_TEXT)		//Text Section
        {
            newshdr->sh_type = SHT_PROGBITS;
            newshdr->sh_flags = SHF_EXECINSTR | SHF_WRITE;
            newshdr->sh_entsize = 1;
            newdata->d_type = ELF_T_BYTE;
        }
        else if(nonLoadableSecs[i]->getRegionType() == Region::RT_DATA)	//Data Section
        {
            newshdr->sh_type = SHT_PROGBITS;
            newshdr->sh_flags = SHF_WRITE;
            newshdr->sh_entsize = 1;
            newdata->d_type = ELF_T_BYTE;
        }
        else if(nonLoadableSecs[i]->getRegionType() == Region::RT_REL)	//Relocations section
        {
            newshdr->sh_type = SHT_REL;
            newshdr->sh_flags = SHF_WRITE;
            newshdr->sh_entsize = sizeof(Elf64_Rel);
            newdata->d_type = ELF_T_BYTE;
        }
        else if(nonLoadableSecs[i]->getRegionType() == Region::RT_SYMTAB)
        {
            newshdr->sh_type = SHT_SYMTAB;
            newshdr->sh_entsize = sizeof(Elf64_Sym);
            newdata->d_type = ELF_T_SYM;
            newshdr->sh_link = secNames.size();   //.symtab section should have sh_link = index of .strtab 
            newshdr->sh_flags=  0;
        }
        else if(nonLoadableSecs[i]->getRegionType() == Region::RT_STRTAB)	//String table Section
        {
            newshdr->sh_type = SHT_STRTAB;
            newshdr->sh_entsize = 1;
            newdata->d_type = ELF_T_BYTE;
            newshdr->sh_link = SHN_UNDEF; 
            newshdr->sh_flags=  0;
        }
        /*
        else if(nonLoadableSecs[i]->getFlags() & Section::dynsymtabSection)
        {
            newshdr->sh_type = SHT_DYNSYM;
            newshdr->sh_entsize = sizeof(Elf64_Sym);
            newdata->d_type = ELF_T_SYM;
            //newshdr->sh_link = newSecSize+i+1;   //.symtab section should have sh_link = index of .strtab
            newshdr->sh_flags=  SHF_ALLOC | SHF_WRITE;
        }*/
        newshdr->sh_offset = prevshdr->sh_offset+prevshdr->sh_size;
        newshdr->sh_addr = 0;
        newshdr->sh_info = 0;
        newshdr->sh_addralign = 4;

        //Set up the data
        newdata->d_buf = nonLoadableSecs[i]->getPtrToRawData();
        newdata->d_size = nonLoadableSecs[i]->getDiskSize();
        newshdr->sh_size = newdata->d_size;
        //elf_update(newElf, ELF_C_NULL);

        newdata->d_align = 4;
        newdata->d_off = 0;
        newdata->d_version = 1;
        /* DEBUG */
        fprintf(stderr, "Added New Section(%s) : secAddr 0x%lx, secOff 0x%lx, secsize 0x%lx, end 0x%lx\n",
                nonLoadableSecs[i]->getRegionName().c_str(), newshdr->sh_addr, newshdr->sh_offset, newshdr->sh_size, newshdr->sh_offset + newshdr->sh_size );

        prevshdr = newshdr;
    }	
    shdr = prevshdr;
    return true;
}    	

/* Regenerates the .symtab, .strtab sections from the symbols
 * Add new .dynsym, .dynstr sections for the newly added dynamic symbols
 * Method - For every symbol call getBackSymbol to get a Elf64_Sym corresposnding 
 *          to a Symbol object. Accumulate all and their names to form the sections
 *          and add them to the list of new sections
 */
bool emitElf64::checkIfStripped(Symtab *obj, vector<Symbol *>&functions, vector<Symbol *>&variables, vector<Symbol *>&mods, vector<Symbol *>&notypes, std::vector<relocationEntry> &relocation_table, std::vector<relocationEntry> &fbt)
{
    unsigned i;

    //Symbol table(.symtab) symbols
    std::vector<Elf64_Sym *> symbols;

    //Symbol table(.dynsymtab) symbols
    std::vector<Elf64_Sym *> dynsymbols;

    unsigned symbolNamesLength = 1, dynsymbolNamesLength = 1;
    vector<string> symbolStrs, dynsymbolStrs;
    symbolStrs.push_back("");
    dynsymbolStrs.push_back("");

    Elf64_Sym *sym = new Elf64_Sym();
    sym->st_name = 0;
    sym->st_value = 0;
    sym->st_size = 0;
    sym->st_other = 0;
    sym->st_info = ELF64_ST_INFO(elfSymBind(Symbol::SL_LOCAL), elfSymType (Symbol::ST_NOTYPE));
    sym->st_shndx = SHN_ABS;

    symbols.push_back(sym);
    dynsymbols.push_back(sym);
    versionSymTable.push_back(0);

    for(i=0; i<notypes.size();i++) {
        if(notypes[i]->isInSymtab())
            getBackSymbol(notypes[i], symbolStrs, symbolNamesLength, symbols);
        if(notypes[i]->isInDynSymtab())
            getBackSymbol(notypes[i], dynsymbolStrs, dynsymbolNamesLength, dynsymbols, true);
    }
    for(i=0; i<functions.size();i++) {
        if(functions[i]->isInSymtab())
            getBackSymbol(functions[i], symbolStrs, symbolNamesLength, symbols);
        if(functions[i]->isInDynSymtab())
            getBackSymbol(functions[i], dynsymbolStrs, dynsymbolNamesLength, dynsymbols, true);
    }
    for(i=0; i<variables.size();i++) {
        if(variables[i]->isInSymtab())
            getBackSymbol(variables[i], symbolStrs, symbolNamesLength, symbols);
        if(variables[i]->isInDynSymtab())
            getBackSymbol(variables[i], dynsymbolStrs, dynsymbolNamesLength, dynsymbols, true);
    }
    for(i=0; i<mods.size();i++) {
        if(mods[i]->isInSymtab())
            getBackSymbol(mods[i], symbolStrs, symbolNamesLength, symbols);
        if(mods[i]->isInDynSymtab())
            getBackSymbol(mods[i], dynsymbolStrs, dynsymbolNamesLength, dynsymbols, true);
    }

    //reconstruct .symtab section
    Elf64_Sym *syms = (Elf64_Sym *)malloc(symbols.size()* sizeof(Elf64_Sym));
    for(i=0;i<symbols.size();i++)
        syms[i] = *(symbols[i]);
    
    --symbolNamesLength;
    char *str = (char *)malloc(symbolNamesLength+1);
    unsigned cur=0;
    for(i=0;i<symbolStrs.size();i++)
    {
        strcpy(&str[cur],symbolStrs[i].c_str());
        cur+=symbolStrs[i].length()+1;
    }
    
    if(!isStripped)
    {
        Region *sec;
        obj->findRegion(sec,".symtab");
	    sec->setPtrToRawData(syms, symbols.size()*sizeof(Elf64_Sym));
    }
    else
    	obj->addRegion(0, syms, symbols.size()*sizeof(Elf64_Sym), ".symtab", Region::RT_SYMTAB);

    //reconstruct .strtab section
    if(!isStripped)
    {
        Region *sec;
        obj->findRegion(sec,".strtab");
	    sec->setPtrToRawData(str, symbolNamesLength);
    }
    else
        obj->addRegion(0, str, symbolNamesLength , ".strtab", Region::RT_STRTAB);
    
    if(!obj->getAllNewRegions(newSecs))
    	log_elferror(err_func_, "No new sections to add");

    //reconstruct .dynsym section
    Elf64_Sym *dynsyms = (Elf64_Sym *)malloc(dynsymbols.size()* sizeof(Elf64_Sym));
    for(i=0;i<dynsymbols.size();i++)
        dynsyms[i] = *(dynsymbols[i]);

#if !defined(os_solaris)
    Elf64_Half *symVers;
    char *verneedSecData, *verdefSecData;
    unsigned verneedSecSize = 0, verdefSecSize = 0, dynsecSize = 0;
               
    createSymbolVersions(symVers, verneedSecData, verneedSecSize, verdefSecData, verdefSecSize, dynsymbolNamesLength, dynsymbolStrs);
    Region *sec;
    if(obj->findRegion(sec, ".dynstr"))
        olddynStrData = (char *)sec->getPtrToRawData();

    Elf64_Dyn *dynsecData;
    if(obj->findRegion(sec, ".dynamic"))
        createDynamicSection(sec->getPtrToRawData(), sec->getDiskSize(), dynsecData, dynsecSize, dynsymbolNamesLength, dynsymbolStrs);
#endif
   
    if(!dynsymbolNamesLength)
        return true; 
    --dynsymbolNamesLength;
    char *dynstr = (char *)malloc(dynsymbolNamesLength+1);
    cur=0;
    hash_map<string, unsigned> dynSymNameMapping;
    for(i=0;i<dynsymbolStrs.size();i++)
    {
        strcpy(&dynstr[cur],dynsymbolStrs[i].c_str());
        cur+=dynsymbolStrs[i].length()+1;
        dynSymNameMapping[dynsymbolStrs[i]] = i;
    }
    
  	obj->addRegion(0, dynsyms, dynsymbols.size()*sizeof(Elf64_Sym), ".dynsym", Region::RT_SYMTAB, true);

    //reconstruct .dynstr section
    obj->addRegion(0, dynstr, dynsymbolNamesLength+1 , ".dynstr", Region::RT_STRTAB, true);

#if !defined(os_solaris)
    //add .gnu.version section
    obj->addRegion(0, symVers, versionSymTable.size() * sizeof(Elf64_Half), ".gnu.version", Region::RT_SYMVERSIONS, true);
    //add .gnu.version_r section
    if(verneedSecSize)
        obj->addRegion(0, verneedSecData, verneedSecSize, ".gnu.version_r", Region::RT_SYMVERNEEDED, true);
    if(verdefSecSize)
        obj->addRegion(0, verdefSecData, verdefSecSize, ".gnu.version_d", Region::RT_SYMVERDEF, true);
#endif

    Elf64_Rel *relplts = (Elf64_Rel *)malloc(sizeof(Elf64_Rel) * (fbt.size()));
    //reconstruct .rel.plt
    for(i=0;i<fbt.size();i++) 
    {
        if(dynSymNameMapping.find(fbt[i].name()) != dynSymNameMapping.end()) {
            relplts[i].r_offset = fbt[i].rel_addr();
            relplts[i].r_info = ELF64_R_INFO(dynSymNameMapping[fbt[i].name()], fbt[i].getRelType());
            //rels[i].r_addend = 0;
        }
    }
   	obj->addRegion(0, relplts, fbt.size()*sizeof(Elf64_Rel), ".rel.plt", Region::RT_REL, true);
    
    vector<relocationEntry> newRels = newSecs[0]->getRelocations();
    
    Elf64_Rel *rels = (Elf64_Rel *)malloc(sizeof(Elf64_Rel) * (relocation_table.size()+newRels.size()));
    //reconstruct .rel
    for(i=0;i<relocation_table.size();i++) 
    {
        if(dynSymNameMapping.find(relocation_table[i].name()) != dynSymNameMapping.end()) {
            rels[i].r_offset = relocation_table[i].rel_addr();
            rels[i].r_info = ELF64_R_INFO(dynSymNameMapping[relocation_table[i].name()], relocation_table[i].getRelType());
            //rels[i].r_addend = 0;
        }
    }
    for(i=0;i<newRels.size();i++) 
    {
        if(dynSymNameMapping.find(newRels[i].name()) != dynSymNameMapping.end()) {
            rels[i].r_offset = newRels[i].rel_addr();
            //rels[i].r_addend = 0;
#if defined(arch_x86)
            rels[i].r_info = ELF64_R_INFO(dynSymNameMapping[newRels[i].name()], R_386_64);
#elif defined(arch_sparc)
//            rels[i].r_info = ELF64_R_INFO(dynSymNameMapping[newRels[i].name()], R_SPARC_64);
#elif defined(arch_x86_64)
            rels[i].r_info = ELF64_R_INFO(dynSymNameMapping[newRels[i].name()], R_X86_64_64);
#elif defined(arch_power)
//            rels[i].r_info = ELF64_R_INFO(dynSymNameMapping[newRels[i].name()], R_PPC_ADDR64);
#endif
        }
    }
   	obj->addRegion(0, rels, (relocation_table.size()+newRels.size())*sizeof(Elf64_Rel), ".rel.dyn", Region::RT_REL, true);
    
#if !defined(os_solaris)
    //add .dynamic section
    if(dynsecSize)
        obj->addRegion(0, dynsecData, dynsecSize*sizeof(Elf64_Dyn), ".dynamic", Region::RT_DYNAMIC, true);
#endif 

    if(!obj->getAllNewRegions(newSecs))
    	log_elferror(err_func_, "No new sections to add");

    return true;
}

#if !defined(os_solaris)
void emitElf64::createSymbolVersions(Elf64_Half *&symVers, char*&verneedSecData, unsigned &verneedSecSize, char *&verdefSecData, unsigned &verdefSecSize, unsigned &dynSymbolNamesLength, vector<string> &dynStrs){

    //Add all names to the new .dynstr section
    map<string, unsigned>::iterator iter = versionNames.begin();
    for(;iter!=versionNames.end();iter++){
        iter->second = dynSymbolNamesLength;
        dynStrs.push_back(iter->first);
        dynSymbolNamesLength+= iter->first.size()+1;
    }

    //reconstruct .gnu_version section
    symVers = (Elf64_Half *)malloc(versionSymTable.size() * sizeof(Elf64_Half));
    for(unsigned i=0; i<versionSymTable.size(); i++)
        symVers[i] = versionSymTable[i];

    //reconstruct .gnu.version_r section
    verneedSecSize = 0;
    map<string, map<string, unsigned> >::iterator it = verneedEntries.begin();
    for(; it != verneedEntries.end(); it++)
        verneedSecSize += sizeof(Elf64_Verneed) + sizeof(Elf64_Vernaux) * it->second.size();

    verneedSecData = (char *)malloc(verneedSecSize);
    unsigned curpos = 0;
    verneednum = 0;
    for(it = verneedEntries.begin(); it != verneedEntries.end(); it++){
        Elf64_Verneed *verneed = (Elf64_Verneed *)(verneedSecData+curpos);
        verneed->vn_version = 1;
        verneed->vn_cnt = it->second.size();
        verneed->vn_file = dynSymbolNamesLength;
        versionNames[it->first] = dynSymbolNamesLength;
        dynStrs.push_back(it->first);
        dynSymbolNamesLength+= it->first.size()+1;
        if(find(DT_NEEDEDEntries.begin(), DT_NEEDEDEntries.end(), it->first) == DT_NEEDEDEntries.end())
            DT_NEEDEDEntries.push_back(it->first);
        verneed->vn_aux = sizeof(Elf64_Verneed);
        verneed->vn_next = sizeof(Elf64_Verneed) + it->second.size()*sizeof(Elf64_Vernaux);
        if(curpos + verneed->vn_next == verneedSecSize)
            verneed->vn_next = 0;
        verneednum++;
        unsigned i = 0;
        for(iter = it->second.begin(); iter!= it->second.end(); iter++){
            Elf64_Vernaux *vernaux = (Elf64_Vernaux *)(verneedSecData + curpos + verneed->vn_aux + i*sizeof(Elf64_Vernaux));
            vernaux->vna_hash = elfHash(iter->first.c_str());
            vernaux->vna_flags = 1;
            vernaux->vna_other = iter->second;
            vernaux->vna_name = versionNames[iter->first];
            if(i == verneed->vn_cnt-1)
                vernaux->vna_next = 0;
            else
                vernaux->vna_next = sizeof(Elf64_Vernaux);
            i++;
        }
        curpos += verneed->vn_next;
    }

    //reconstruct .gnu.version_d section
    verdefSecSize = 0;
    for(iter = verdefEntries.begin(); iter != verdefEntries.end(); iter++)
        verdefSecSize += sizeof(Elf64_Verdef) + sizeof(Elf64_Verdaux) * verdauxEntries[iter->second].size();

    verdefSecData = (char *)malloc(verdefSecSize);
    curpos = 0;
    for(iter = verdefEntries.begin(); iter != verdefEntries.end(); iter++){
        Elf64_Verdef *verdef = (Elf64_Verdef *)(verdefSecData+curpos);
        verdef->vd_version = 1;
        verdef->vd_flags = 1;
        verdef->vd_ndx = iter->second;
        verdef->vd_cnt = verdauxEntries[iter->second].size();
        verdef->vd_hash = elfHash(iter->first.c_str());
        verdef->vd_aux = sizeof(Elf64_Verdef);
        verdef->vd_next = sizeof(Elf64_Verdef) + verdauxEntries[iter->second].size()*sizeof(Elf64_Verdaux);
        if(curpos + verdef->vd_next == verdefSecSize)
            verdef->vd_next = 0;
        for(unsigned i = 0; i< verdauxEntries[iter->second].size(); i++){
            Elf64_Verdaux *verdaux = (Elf64_Verdaux *)(verdefSecData + curpos + verdef->vd_aux + i*sizeof(Elf64_Verdaux));
            verdaux->vda_name = versionNames[verdauxEntries[iter->second][0]];
            if(i == verdef->vd_cnt-1)
                verdaux->vda_next = 0;
            else
                verdaux->vda_next = sizeof(Elf64_Verdaux);
            i++;
        }
        curpos += verdef->vd_next;
    }
    return;
}

void emitElf64::createDynamicSection(void *dynData, unsigned size, Elf64_Dyn *&dynsecData, unsigned &dynsecSize, unsigned &dynSymbolNamesLength, std::vector<std::string> &dynStrs) {
    dynamicSecData.clear();
    Elf64_Dyn *dyns = (Elf64_Dyn *)dynData;
    unsigned count = size/sizeof(Elf64_Dyn);
    dynsecSize = 2*count+ DT_NEEDEDEntries.size();    //We don't know the size before hand. So allocate the maximum possible size;
    dynsecData = (Elf64_Dyn *)malloc(dynsecSize*sizeof(Elf64_Dyn));
    unsigned curpos = 0;
    string rpathstr;
    for(unsigned i = 0; i< DT_NEEDEDEntries.size(); i++){
        dynsecData[curpos].d_tag = DT_NEEDED;
        dynsecData[curpos].d_un.d_val = versionNames[DT_NEEDEDEntries[i]];
        dynamicSecData[DT_NEEDED].push_back(dynsecData+curpos);
        curpos++;
    }
    for(unsigned i = 0; i< count;i++){
        switch(dyns[i].d_tag){
            case DT_NULL:
                break;
            case DT_NEEDED:
                rpathstr = &olddynStrData[dyns[i].d_un.d_val];
                if(find(DT_NEEDEDEntries.begin(), DT_NEEDEDEntries.end(), rpathstr) != DT_NEEDEDEntries.end())
                    break;
                dynsecData[curpos].d_tag = dyns[i].d_tag;
                dynsecData[curpos].d_un.d_val = dynSymbolNamesLength;
                dynStrs.push_back(rpathstr);
                dynSymbolNamesLength += rpathstr.size() + 1;
                dynamicSecData[dyns[i].d_tag].push_back(dynsecData+curpos);
                curpos++;
                break;
            case DT_RPATH:
            case DT_RUNPATH:
                dynsecData[curpos].d_tag = dyns[i].d_tag;
                dynsecData[curpos].d_un.d_val = dynSymbolNamesLength;
                rpathstr = &olddynStrData[dyns[i].d_un.d_val];
                dynStrs.push_back(rpathstr);
                dynSymbolNamesLength += rpathstr.size() + 1;
                dynamicSecData[dyns[i].d_tag].push_back(dynsecData+curpos);
                curpos++;
                break;
            default:
                memcpy(dynsecData+curpos, dyns+i, sizeof(Elf64_Dyn));
                dynamicSecData[dyns[i].d_tag].push_back(dynsecData+curpos);
                curpos++;
                break;
        }
    }
    dynsecData[curpos].d_tag = DT_NULL;
    dynsecData[curpos].d_un.d_val = 0;
    dynsecSize = curpos+1;                            //assign size to the correct number of entries
}
#endif


void emitElf64::log_elferror(void (*err_func)(const char *), const char* msg) {
    const char* err = elf_errmsg(elf_errno());
    err = err ? err: "(bad elf error)";
    string str = string(err)+string(msg);
    err_func(str.c_str());
}
