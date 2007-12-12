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

#include "emitElf.h"
#include "Symtab.h"

using namespace Dyninst;
using namespace Dyninst::SymtabAPI;
using namespace std;

extern const char *pdelf_get_shnames(Elf_X &elf);

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

emitElf::emitElf(Elf_X &oldElfHandle_, bool isStripped_, int BSSexpandflag_, void (*err_func)(const char *)) :
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
}

bool emitElf::getBackSymbol(Symbol *symbol, vector<string> &symbolStrs, unsigned &symbolNamesLength, vector<Elf32_Sym *> &symbols)
{
   Elf32_Sym *sym = new Elf32_Sym();
   sym->st_name = symbolNamesLength;
   symbolStrs.push_back(symbol->getName());
   symbolNamesLength += symbol->getName().length()+1;
   sym->st_value = symbol->getAddr();
   sym->st_size = symbol->getSize();
   sym->st_other = 0;
   sym->st_info = ELF32_ST_INFO(elfSymBind(symbol->getLinkage()), elfSymType (symbol->getType()));
   if(symbol->getSec())
	sym->st_shndx = symbol->getSec()->getSecNumber();
   else if(symbol->getType() == Symbol::ST_MODULE || symbol->getType() == Symbol::ST_NOTYPE)
        sym->st_shndx = SHN_ABS;
   
   symbols.push_back(sym);
   std::vector<string> names = symbol->getAllMangledNames();
   for(unsigned i=1;i<names.size();i++)
   {
       	sym = new Elf32_Sym();
	    sym->st_name = symbolNamesLength;
    	symbolStrs.push_back(names[i]);
       	symbolNamesLength += names[i].length()+1;
    	sym->st_value = symbol->getAddr();
    	sym->st_size = 0;
       	sym->st_other = 0;
       	sym->st_info = ELF32_ST_INFO(elfSymBind(symbol->getLinkage()), elfSymType (symbol->getType()));
       	if(symbol->getSec())
    	    sym->st_shndx = symbol->getSec()->getSecNumber();
    	else if(symbol->getType() == Symbol::ST_MODULE || symbol->getType() == Symbol::ST_NOTYPE)
    	    sym->st_shndx = SHN_ABS;
       	symbols.push_back(sym);
   }
   return true;
}

// Find the end of data/text segment
void emitElf::findSegmentEnds()
{
    Elf32_Phdr *tmp = elf32_getphdr(oldElf);
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

bool emitElf::driver(Symtab *obj, string fName){
    int newfd;
    Section *foundSec;
    unsigned pgSize = getpagesize();

    vector<Section *> newSecs;
    if(!obj->getAllNewSections(newSecs))
    {
    	log_elferror(err_func_, "No new sections to add");
//	    return false;
    }	

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
    
    // ".shstrtab" section: string table for section header names
    const char *shnames = pdelf_get_shnames(oldElfHandle);
    if (shnames == NULL) {
        log_elferror(err_func_, ".shstrtab section");
    	return false;
    }	

    // Write the Elf header first!
    newEhdr= elf32_newehdr(newElf);
    if(!newEhdr){
        log_elferror(err_func_, "newEhdr failed\n");
        return false;
    }
    oldEhdr = elf32_getehdr(oldElf);
    memcpy(newEhdr, oldEhdr, sizeof(Elf32_Ehdr));
    
    newEhdr->e_shnum += newSecs.size();

    // Find the end of text and data segments
    findSegmentEnds();
    unsigned insertPoint = oldEhdr->e_shnum;
    unsigned NOBITSstartPoint = oldEhdr->e_shnum;

    /* flag the file for no auto-layout */
    if(addNewSegmentFlag)
    {
        newEhdr->e_phoff = sizeof(Elf32_Ehdr);
    }
    elf_flagelf(newElf,ELF_C_SET,ELF_F_LAYOUT);  
    
    Elf_Scn *scn = NULL, *newscn;
    Elf_Data *newdata = NULL, *olddata = NULL;
    Elf32_Shdr *newshdr, *shdr;
    
    unsigned scncount;
    for (scncount = 0; (scn = elf_nextscn(oldElf, scn)); scncount++) {

    	//copy sections from oldElf to newElf
        shdr = elf32_getshdr(scn);
    	// resolve section name
        const char *name = &shnames[shdr->sh_name];
	    obj->findSection(foundSec, name);
        // write the shstrtabsection at the end
        if(!strcmp(name, ".shstrtab"))
            continue;

    	newscn = elf_newscn(newElf);
    	newshdr = elf32_getshdr(newscn);
    	newdata = elf_newdata(newscn);
    	olddata = elf_getdata(scn,NULL);
    	memcpy(newshdr, shdr, sizeof(Elf32_Shdr));
    	memcpy(newdata,olddata, sizeof(Elf_Data));

        secNames.push_back(name);
        newshdr->sh_name = secNameIndex;
        secNameIndex += strlen(name) + 1;
    
    	if(foundSec->isDirty())
    	{
    	    newdata->d_buf = (char *)malloc(foundSec->getSecSize());
    	    memcpy(newdata->d_buf, foundSec->getPtrToRawData(), foundSec->getSecSize());
    	    newdata->d_size = foundSec->getSecSize();
    	    newshdr->sh_size = foundSec->getSecSize();
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
    	    //newshdr->sh_type = SHT_PROGBITS;
            dynStrData = newdata;
            //updateSymbols(dynsymData, dynStrData);
        }
    	//Change sh_link for .symtab to point to .strtab
        if(!strcmp(name, ".symtab")){
            newshdr->sh_link = secNames.size();
            symTabData = newdata;
        }
        if(!strcmp(name, ".dynsym")){
            //Change the type of the original dynsym section if we are changing it.
    	    //newshdr->sh_type = SHT_PROGBITS;
            newshdr->sh_link = secNames.size();
            dynsymData = newdata;
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
    	fprintf(stderr, "Added Section %d: secAddr 0x%lx, secOff 0x%lx, secsize 0x%lx, end 0x%lx\n",
	                        scncount, newshdr->sh_addr, newshdr->sh_offset, newshdr->sh_size, newshdr->sh_offset + newshdr->sh_size );

	    //Insert new loadable sections at the end of data segment			
    	if(shdr->sh_addr+shdr->sh_size == dataSegEnd){
	        insertPoint = scncount;
            if(!createLoadableSections(newshdr, newSecs, loadSecTotalSize))
	        	return false;
        }

        if(!strcmp(name, ".dynamic")){
            dynData = newdata;
            dynSegOff = newshdr->sh_offset;
            dynSegAddr = newshdr->sh_addr;
            // Change the data to update the relocation addr
            //newshdr->sh_type = SHT_PROGBITS;
            //newSecs.push_back(new Section(oldEhdr->e_shnum+newSecs.size(),".dynamic", /*addr*/, newdata->d_size, dynData, Section::dynamicSection, true));
	    }

	    elf_update(newElf, ELF_C_NULL);
    }

    // Add non-loadable sections at the end of object file
    if(!createNonLoadableSections(newshdr))
    	return false;
   
    //Add the section header table right at the end        
    addSectionHeaderTable(newshdr);

    scn = NULL;
    for (scncount = 0; (scn = elf_nextscn(newElf, scn)); scncount++) {
    	shdr = elf32_getshdr(scn);
    	olddata = elf_getdata(scn,NULL);
    }
    newEhdr->e_shstrndx = scncount;

    // Move the section header to the end
    newEhdr->e_shoff =shdr->sh_offset+shdr->sh_size+1;
    if(addNewSegmentFlag)
        newEhdr->e_shoff += pgSize;

    //copy program headers
    oldPhdr = elf32_getphdr(oldElf);
    fixPhdrs(loadSecTotalSize);

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

void emitElf::fixPhdrs(unsigned loadSecTotalSize)
{
    unsigned pgSize = getpagesize();
    Elf32_Phdr *tmp = oldPhdr;
    if(addNewSegmentFlag) {
        if(firstNewLoadSec)
	    newEhdr->e_phnum= oldEhdr->e_phnum + 1;
        else
    	    newEhdr->e_phnum= oldEhdr->e_phnum;
    }
    if(BSSExpandFlag)
       newEhdr->e_phnum= oldEhdr->e_phnum;
    
    newPhdr=elf32_newphdr(newElf,newEhdr->e_phnum);

    Elf32_Phdr newSeg;
    for(unsigned i=0;i<oldEhdr->e_phnum;i++)
    {
        memcpy(newPhdr, tmp, oldEhdr->e_phentsize);
    	// Expand the data segment to include the new loadable sections
    	// Also add a executable permission to the segment
        if(tmp->p_type == PT_DYNAMIC){
            newPhdr->p_vaddr = dynSegAddr;
            newPhdr->p_paddr = dynSegAddr;
            newPhdr->p_offset = dynSegOff;
        }
    	if(BSSExpandFlag) {
    	    if(tmp->p_type == PT_LOAD && (tmp->p_flags == 6 || tmp->p_flags == 7))
    	    {
	        	newPhdr->p_memsz += loadSecTotalSize;
    	    	newPhdr->p_filesz = newPhdr->p_memsz;
    	    	newPhdr->p_flags = 7;
	        }	
    	}    
	    if(addNewSegmentFlag) {
	        if(tmp->p_type == PT_LOAD && tmp->p_flags == 5)
            {
    	        newPhdr->p_vaddr = 0x08047000;
        		newPhdr->p_paddr = newPhdr->p_vaddr;
        		newPhdr->p_filesz += pgSize;
        		newPhdr->p_memsz = newPhdr->p_filesz;
	        }
    	    if(tmp->p_type == PT_LOAD && tmp->p_flags == 6 || tmp->p_type == PT_NOTE || tmp->p_type == PT_INTERP || tmp->p_type == PT_DYNAMIC)
	            newPhdr->p_offset += pgSize;
    	} 
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

//This method updates the .dynamic section to reflect the changes to the relocation section
void emitElf::updateDynamic(Elf_Data* dynData,  Elf32_Addr relAddr){
#if !defined(os_solaris)
    Elf32_Dyn *dyns = (Elf32_Dyn *)dynData->d_buf;
    int count = dynData->d_size/sizeof(Elf32_Dyn);
    for(unsigned i = 0; i< count;i++){
        switch(dyns[i].d_tag){
            case DT_REL:
            case DT_RELA:
                dyns[i].d_un.d_ptr = relAddr;
                break;
        }
    }
#endif    
}

//This method updates the symbol table,
//it shifts each symbol address as necessary AND
//sets _end and _END_ to move the heap
void emitElf::updateSymbols(Elf_Data* symtabData,Elf_Data* strData, unsigned long loadSecsSize){
    if( symtabData && strData && loadSecsSize){
        Elf32_Sym *symPtr=(Elf32_Sym*)symtabData->d_buf;
        for(unsigned int i=0;i< symtabData->d_size/(sizeof(Elf32_Sym));i++,symPtr++){
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

bool emitElf::createLoadableSections(Elf32_Shdr *shdr, std::vector<Section *>&newSecs, unsigned &loadSecTotalSize)
{
    Elf_Scn *newscn;
    Elf_Data *newdata = NULL;
    Elf32_Shdr *newshdr;
    firstNewLoadSec = NULL;
    unsigned extraSize = 0;
    unsigned pgSize = getpagesize();
    unsigned strtabIndex = 0;
    Elf32_Shdr *prevshdr = NULL;

    for(unsigned i=0; i < newSecs.size(); i++)
    {
    	if(newSecs[i]->isLoadable())
    	{
	        secNames.push_back(newSecs[i]->getSecName());
    
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

    	    // Fill out the new section header	
	        newshdr = elf32_getshdr(newscn);
	        newshdr->sh_name = secNameIndex;
    	    newshdr->sh_flags = 0;
    	    if(newSecs[i]->getFlags() && Section::textSection)
	        	newshdr->sh_flags |=  SHF_EXECINSTR | SHF_ALLOC;
    	    if (newSecs[i]->getFlags() && Section::dataSection)    
	            newshdr->sh_flags |=  SHF_WRITE | SHF_ALLOC;
    	    newshdr->sh_type = SHT_PROGBITS;

            // TODO - compute the correct offset && address. This is wrong!!
	        if(shdr->sh_type == SHT_NOBITS)
	        	newshdr->sh_offset = shdr->sh_offset;
    	    else
	        	newshdr->sh_offset = shdr->sh_offset+shdr->sh_size;
            if(newSecs[i]->getSecAddr())
    	        newshdr->sh_addr = newSecs[i]->getSecAddr();
            else{
                newshdr->sh_addr = prevshdr->sh_addr+ prevshdr->sh_size;
            }
    	    
            newshdr->sh_link = SHN_UNDEF;
	        newshdr->sh_info = 0;
	        newshdr->sh_addralign = 4;
       	    newshdr->sh_entsize = 0;
            
            if(newSecs[i]->getFlags() & Section::relocationSection)    //Relocatons section
            {
                newshdr->sh_type = SHT_REL;
                newshdr->sh_flags = SHF_WRITE;
                newshdr->sh_entsize = sizeof(Elf32_Rel);
                newshdr->sh_link = strtabIndex;   //.rel.plt section should have sh_link = index of .strtab for dynsym
                newdata->d_type = ELF_T_REL;
                updateDynamic(dynData, newshdr->sh_addr);
            }
            else if(newSecs[i]->getFlags() & Section::stringSection)    //String table Section
            {
                newshdr->sh_type = SHT_STRTAB;
                newshdr->sh_entsize = 1;
                newdata->d_type = ELF_T_BYTE;
                newshdr->sh_link = SHN_UNDEF;
                newshdr->sh_flags=  0;
                strtabIndex = secNames.size()-1;
            }
            else if(newSecs[i]->getFlags() & Section::dynsymtabSection)
            {
                newshdr->sh_type = SHT_DYNSYM;
                newshdr->sh_entsize = sizeof(Elf32_Sym);
                newdata->d_type = ELF_T_SYM;
                newshdr->sh_link = secNames.size();   //.symtab section should have sh_link = index of .strtab for .dynsym
                newshdr->sh_flags=  0;
            }
            else if(newSecs[i]->getFlags() & Section::dynamicSection)
            {
    #if !defined(os_solaris)
                newshdr->sh_entsize = sizeof(Elf32_Dyn);
    #endif            
                newshdr->sh_type = SHT_DYNAMIC;
                newdata->d_type = ELF_T_DYN;
                newshdr->sh_link = strtabIndex;   //.dynamic section should have sh_link = index of .strtab for .dynsym
                newshdr->sh_flags=  0;
                dynSegOff = newshdr->sh_offset;
                dynSegAddr = newshdr->sh_addr;
            }

    	    if(addNewSegmentFlag)
	        {
	            // Check to make sure the (vaddr for the start of the new segment - the offset) is page aligned
    	        if(!firstNewLoadSec)
	            {
	        	    newSegmentStart = newshdr->sh_addr  - (newshdr->sh_addr & (pgSize-1)) + (newshdr->sh_offset & (pgSize-1));
    		        if(newSegmentStart < newshdr->sh_addr)
    	            {
	    	            newSegmentStart += pgSize;
        		        extraSize = newSegmentStart - newshdr->sh_addr;
        		        newshdr->sh_addr = newSegmentStart;
	        	    } 
		        }    
    	    }	
            // Why is this being done -giri??	
            // newshdr->sh_offset = shdr->sh_offset;

    	    //Set up the data
            newdata->d_buf = malloc(newSecs[i]->getSecSize());
    	    memcpy(newdata->d_buf, newSecs[i]->getPtrToRawData(), newSecs[i]->getSecSize());
	        newdata->d_size = newSecs[i]->getSecSize();
	        newshdr->sh_size = newdata->d_size;
    	    loadSecTotalSize += newshdr->sh_size;
	    
	        newdata->d_type = ELF_T_BYTE;
	        newdata->d_align = 4;
    	    newdata->d_version = 1;
	    
    	    elf_update(newElf, ELF_C_NULL);

	        shdr = newshdr;
    	    if(!firstNewLoadSec)
	        	firstNewLoadSec = shdr;
            secNameIndex += newSecs[i]->getSecName().size() + 1;
	        /* DEBUG */
    	    fprintf(stderr, "Added New Section : secAddr 0x%lx, secOff 0x%lx, secsize 0x%lx, end 0x%lx\n",
	                                     newshdr->sh_addr, newshdr->sh_offset, newshdr->sh_size, newshdr->sh_offset + newshdr->sh_size );
            prevshdr = newshdr;
	    }
	    else
	        nonLoadableSecs.push_back(newSecs[i]);
    }	
    return true;
}
	
bool emitElf::addSectionHeaderTable(Elf32_Shdr *shdr) {
    Elf_Scn *newscn;
    Elf_Data *newdata = NULL;
    Elf32_Shdr *newshdr;
    
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
    newshdr = elf32_getshdr(newscn);
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
    //elf_update(newElf, ELF_C_NULL);
   
   	newdata->d_align = 4;
    newdata->d_version = 1;
}

bool emitElf::createNonLoadableSections(Elf32_Shdr *shdr)
{
    Elf_Scn *newscn;
    Elf_Data *newdata = NULL;
    Elf32_Shdr *newshdr;
    
    Elf32_Shdr *prevshdr = shdr; 
    //All of them that are left are non-loadable. stack'em up at the end.
    for(unsigned i = 0; i < nonLoadableSecs.size(); i++)
    {
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
    	newshdr = elf32_getshdr(newscn);
	    newshdr->sh_name = secNameIndex;
    	secNameIndex += nonLoadableSecs[i]->getSecName().length() + 1;
    	if(nonLoadableSecs[i]->getFlags() & Section::textSection)		//Text Section
	    {
    	    newshdr->sh_type = SHT_PROGBITS;
    	    newshdr->sh_flags = SHF_EXECINSTR | SHF_WRITE;
            newshdr->sh_entsize = 1;
	        newdata->d_type = ELF_T_BYTE;
    	}
    	else if(nonLoadableSecs[i]->getFlags() & Section::dataSection)	//Data Section
	    {
    	    newshdr->sh_type = SHT_PROGBITS;
    	    newshdr->sh_flags = SHF_WRITE;
            newshdr->sh_entsize = 1;
    	    newdata->d_type = ELF_T_BYTE;
    	}
	    else if(nonLoadableSecs[i]->getFlags() & Section::relocationSection)	//Relocatons section
    	{
	        newshdr->sh_type = SHT_REL;
            newshdr->sh_flags = SHF_WRITE;
            newshdr->sh_entsize = sizeof(Elf32_Rel);
	        newdata->d_type = ELF_T_BYTE;
    	}
    	else if(nonLoadableSecs[i]->getFlags() & Section::symtabSection)
	    {
    	    newshdr->sh_type = SHT_SYMTAB;
            newshdr->sh_entsize = sizeof(Elf32_Sym);
    	    newdata->d_type = ELF_T_SYM;
    	    //newshdr->sh_link = newSecSize+i+1;   //.symtab section should have sh_link = index of .strtab
    	    newshdr->sh_flags=  0;
	    }
    	else if(nonLoadableSecs[i]->getFlags() & Section::stringSection)	//String table Section
	    {
    	    newshdr->sh_type = SHT_STRTAB;
            newshdr->sh_entsize = 1;
    	    newdata->d_type = ELF_T_BYTE;
    	    newshdr->sh_link = SHN_UNDEF; 
	        newshdr->sh_flags=  0;
    	}
	    else if(nonLoadableSecs[i]->getFlags() & Section::dynsymtabSection)
    	{
	        newshdr->sh_type = SHT_DYNSYM;
            newshdr->sh_entsize = sizeof(Elf32_Sym);
	        newdata->d_type = ELF_T_SYM;
    	   //newshdr->sh_link = newSecSize+i+1;   //.symtab section should have sh_link = index of .strtab
	        newshdr->sh_flags=  0;
	    }
    	newshdr->sh_offset = prevshdr->sh_offset+prevshdr->sh_size;
	    newshdr->sh_addr = 0;
    	newshdr->sh_info = 0;
	    newshdr->sh_addralign = 4;

        //Set up the data
    	newdata->d_buf = nonLoadableSecs[i]->getPtrToRawData();
	    newdata->d_size = nonLoadableSecs[i]->getSecSize();
    	newshdr->sh_size = newdata->d_size;
	    //elf_update(newElf, ELF_C_NULL);
	    
    	newdata->d_align = 4;
	    newdata->d_version = 1;
	    
	    prevshdr = newshdr;
    }	
    shdr = prevshdr;
    return true;
}    	

/* Regenerates the .symtab, .strtab sections from the symbols
 * Add new .dynsym, .dynstr sections for the newly added dynamic symbols
 * Method - For every symbol call getBackSymbol to get a Elf32_Sym corresposnding 
 *          to a Symbol object. Accumulate all and their names to form the sections
 *          and add them to the list of new sections
 */
bool emitElf::checkIfStripped(Symtab *obj, vector<Symbol *>&functions, vector<Symbol *>&variables, vector<Symbol *>&mods, vector<Symbol *>&notypes, std::vector<relocationEntry> &fbt)
{
    unsigned i;
//    if(!isStripped)
//    	return false;

    //Symbol table(.symtab) symbols
    std::vector<Elf32_Sym *> symbols;

    //Symbol table(.dynsymtab) symbols
    std::vector<Elf32_Sym *> dynsymbols;

    unsigned symbolNamesLength = 1, dynsymbolNamesLength = 0;
    vector<string> symbolStrs, dynsymbolStrs;
    symbolStrs.push_back("");
    for(i=0; i<functions.size();i++) {
        if(functions[i]->isInSymtab())
            getBackSymbol(functions[i], symbolStrs, symbolNamesLength, symbols);
        if(functions[i]->isInDynSymtab())
            getBackSymbol(functions[i], dynsymbolStrs, dynsymbolNamesLength, dynsymbols);
    }
    for(i=0; i<variables.size();i++) {
        if(variables[i]->isInSymtab())
            getBackSymbol(variables[i], symbolStrs, symbolNamesLength, symbols);
        if(variables[i]->isInDynSymtab())
            getBackSymbol(variables[i], dynsymbolStrs, dynsymbolNamesLength, dynsymbols);
    }
    for(i=0; i<mods.size();i++) {
        if(mods[i]->isInSymtab())
            getBackSymbol(mods[i], symbolStrs, symbolNamesLength, symbols);
        if(mods[i]->isInDynSymtab())
            getBackSymbol(mods[i], dynsymbolStrs, dynsymbolNamesLength, dynsymbols);
    }
    for(i=0; i<notypes.size();i++) {
        if(notypes[i]->isInSymtab())
            getBackSymbol(notypes[i], symbolStrs, symbolNamesLength, symbols);
        if(notypes[i]->isInDynSymtab())
            getBackSymbol(notypes[i], dynsymbolStrs, dynsymbolNamesLength, dynsymbols);
    }

    //reconstruct .symtab section
    Elf32_Sym *syms = (Elf32_Sym *)malloc(symbols.size()* sizeof(Elf32_Sym));
    for(i=0;i<symbols.size();i++)
        syms[i] = *(symbols[i]);
    
    --symbolNamesLength;
    char *str = (char *)malloc(symbolNamesLength);
    unsigned cur=0;
    for(i=0;i<symbolStrs.size();i++)
    {
        strcpy(&str[cur],symbolStrs[i].c_str());
        cur+=symbolStrs[i].length()+1;
    }
    
    char *data = (char *)malloc(symbols.size()*sizeof(Elf32_Sym));
    memcpy(data,syms, symbols.size()*sizeof(Elf32_Sym));

    if(!isStripped)
    {
        Section *sec;
        obj->findSection(sec,".symtab");
	    sec->setPtrToRawData(data, symbols.size()*sizeof(Elf32_Sym));
    }
    else
    	obj->addSection(0, data, symbols.size()*sizeof(Elf32_Sym), ".symtab", Section::symtabSection);

    //reconstruct .strtab section
    char *strData = (char *)malloc(symbolNamesLength);
    memcpy(strData, str, symbolNamesLength);
    
    if(!isStripped)
    {
        Section *sec;
        obj->findSection(sec,".strtab");
	    sec->setPtrToRawData(strData, symbolNamesLength);
    }
    else
        obj->addSection(0, strData, symbolNamesLength , ".strtab", Section::stringSection);

    //reconstruct .rel
    for(i=0;i<fbt.size();i++) {
    }
    
    //reconstruct .dynsym section
    Elf32_Sym *dynsyms = (Elf32_Sym *)malloc(dynsymbols.size()* sizeof(Elf32_Sym));
    for(i=0;i<dynsymbols.size();i++)
        dynsyms[i] = *(dynsymbols[i]);
   
    if(!dynsymbolNamesLength)
        return true; 
    --dynsymbolNamesLength;
    char *dynstr = (char *)malloc(dynsymbolNamesLength);
    cur=0;
    for(i=0;i<dynsymbolStrs.size();i++)
    {
        strcpy(&dynstr[cur],dynsymbolStrs[i].c_str());
        cur+=dynsymbolStrs[i].length()+1;
    }
    
    data = (char *)malloc(dynsymbols.size()*sizeof(Elf32_Sym));
    memcpy(data, dynsyms, dynsymbols.size()*sizeof(Elf32_Sym));

//    if(!isStripped)
//    {
//        Section *sec;
//        obj->findSection(sec,".dynsym");
//	    sec->setPtrToRawData(data, dynsymbols.size()*sizeof(Elf32_Sym));
//    }
//    else
    	obj->addSection(0, data, dynsymbols.size()*sizeof(Elf32_Sym), ".dynsym", Section::dynsymtabSection, true);

    //reconstruct .dynstr section
    strData = (char *)malloc(dynsymbolNamesLength);
    memcpy(strData, dynstr, dynsymbolNamesLength);
    
//    if(!isStripped)
//    {
//        Section *sec;
//        obj->findSection(sec,".dynstr");
//	    sec->setPtrToRawData(strData, dynsymbolNamesLength);
//    }
//    else
        obj->addSection(0, strData, dynsymbolNamesLength , ".dynstr", Section::stringSection, true);
    return true;
}

void emitElf::log_elferror(void (*err_func)(const char *), const char* msg) {
    const char* err = elf_errmsg(elf_errno());
    err = err ? err: "(bad elf error)";
    string str = string(err)+string(msg);
    err_func(str.c_str());
}
