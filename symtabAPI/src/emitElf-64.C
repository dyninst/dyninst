#include "emitElf-64.h"
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

bool emitElf::getBackSymbol(Symbol *symbol)
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
	sym->st_shndx = symbol->getSec()->getSecNumber();
   else if(symbol->getType() == Symbol::ST_MODULE  || symbol->getType() == Symbol::ST_NOTYPE)
        sym->st_shndx = SHN_ABS;
   symbols.push_back(sym);
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

bool emitElf::driver(Symtab *obj, string fName){
    int newfd;
    Section *foundSec;
    unsigned pgSize = getpagesize();

    vector<Section *> newSecs;
    if(!obj->getAllNewSections(newSecs))
    {
    	log_elferror(err_func_, "No new sections to add");
	return false;
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
    
    //Section name index for new sections
    std::vector<string> loadSecNames;
    unsigned loadSecTotalSize = 0;
    unsigned NOBITStotalsize = 0;
    unsigned shStrTabSizeInc = 0;
    int dirtySecsInc = 0;
    unsigned extraAlignSize = 0;
    unsigned nonLoadableNamesSize = 0;
    
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

    /* flag the file for no auto-layout */
    if(addNewSegmentFlag)
    {
        newEhdr->e_phoff = sizeof(Elf64_Ehdr);
}
        elf_flagelf(newElf,ELF_C_SET,ELF_F_LAYOUT);  
 //   }	
    
    Elf_Scn *shstrtabSec = elf_getscn(oldElf, oldEhdr->e_shstrndx);
    Elf_Data *shstrtabData = elf_getdata(shstrtabSec, NULL);
    unsigned shstrtabDataSize = shstrtabData->d_size;
    
    Elf_Scn *scn = NULL, *newscn;
    Elf_Data *newdata = NULL, *olddata = NULL;
    Elf64_Shdr *newshdr, *shdr;
    
    unsigned scncount;
    for (scncount = 0; (scn = elf_nextscn(oldElf, scn)); scncount++) {

    	//copy sections from oldElf to newElf
        shdr = elf64_getshdr(scn);
	newscn = elf_newscn(newElf);
	newshdr = elf64_getshdr(newscn);
	newdata = elf_newdata(newscn);
	olddata = elf_getdata(scn,NULL);
	memcpy(newshdr, shdr, sizeof(Elf64_Shdr));
	memcpy(newdata,olddata, sizeof(Elf_Data));
													
	// resolve section name
        const char *name = &shnames[shdr->sh_name];
	obj->findSection(foundSec, name);
	
	if(foundSec->isDirty())
	{
	    newdata->d_buf = (char *)malloc(foundSec->getSecSize());
	    memcpy(newdata->d_buf, foundSec->getPtrToRawData(), foundSec->getSecSize());
	    newdata->d_size = foundSec->getSecSize();
	    newshdr->sh_size = foundSec->getSecSize();
	    dirtySecsInc += newshdr->sh_size - shdr->sh_size;
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
            dynStrData = newdata;
            //updateSymbols(dynsymData, dynStrData);
        }
	//Change sh_link for .symtab to point to .strtab
        if(!strcmp(name, ".symtab")){
            if(newshdr->sh_link >= insertPoint)
	    {
                newshdr->sh_link += loadSecNames.size();
	    }	
            symTabData = newdata;
        }
        if(!strcmp(name, ".dynsym")){
            dynsymData = newdata;
        }
	if(!strcmp(name, ".text")){
            textData = newdata;
        }
	// Add the new section names
        if(!strcmp(name, ".shstrtab")){
            addSectionNames(newdata,olddata, shstrtabDataSize, nonLoadableNamesSize, loadSecNames);
    	    newshdr->sh_size = newdata->d_size;
	    shStrTabSizeInc = newdata->d_size - olddata->d_size;
        }
        if(!strcmp(name, ".data")){
	    dataData = newdata;
	}
	// Change offsets of sections based on the newly added sections
	if(addNewSegmentFlag)
	    newshdr->sh_offset += pgSize;
	if(scncount > insertPoint)
	{
	    newshdr->sh_offset += loadSecTotalSize;
	    if(scncount>oldEhdr->e_shstrndx)
	    	newshdr->sh_offset += shStrTabSizeInc;
	}
	newshdr->sh_offset += (int) (dirtySecsInc + extraAlignSize);
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
            if(!createLoadableSections(newshdr, newSecs, loadSecNames, shstrtabDataSize, nonLoadableNamesSize, loadSecTotalSize))
	    	return false;
        }
	elf_update(newElf, ELF_C_NULL);
							
    }

    // Add non-loadable sections at the end of object file
    if(!createNonLoadableSections(newshdr, shstrtabDataSize, oldEhdr->e_shnum+loadSecNames.size()))
    	return false;

    scn = NULL;
    for (scncount = 0; (scn = elf_nextscn(newElf, scn)); scncount++) {
    	shdr = elf64_getshdr(scn);
	olddata = elf_getdata(scn,NULL);
    }
    newEhdr->e_shstrndx+=loadSecNames.size();

    // Move the section header to the end
    newEhdr->e_shoff =shdr->sh_offset+shdr->sh_size+1;
    if(addNewSegmentFlag)
        newEhdr->e_shoff += pgSize;

    //copy program headers
    oldPhdr = elf64_getphdr(oldElf);
    fixPhdrs(loadSecTotalSize);

    //Write the new Elf file
    if (elf_update(newElf, ELF_C_WRITE) < 0){
        int err;
	if ((err = elf_errno()) != 0)
   	{
		const char *msg = elf_errmsg(err);
		fprintf(stderr, "Error: Unable to write ELF file: %s\n", msg);
		/* print msg */
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

//This method updates the symbol table,
//it shifts each symbol address as necessary AND
//sets _end and _END_ to move the heap
void emitElf::updateSymbols(Elf_Data* symtabData,Elf_Data* strData, unsigned long loadSecsSize){
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

bool emitElf::createLoadableSections(Elf64_Shdr *shdr, std::vector<Section *>&newSecs, std::vector<string> &loadSecNames, unsigned &shstrtabDataSize, unsigned &nonLoadableNamesSize, unsigned &loadSecTotalSize)
{
    Elf_Scn *newscn;
    Elf_Data *newdata = NULL;
    Elf64_Shdr *newshdr;
    firstNewLoadSec = NULL;
    unsigned extraSize = 0;
    unsigned pgSize = getpagesize();
			
    for(unsigned i=0; i < newSecs.size(); i++)
    {
    	if(newSecs[i]->isLoadable())
	{
	    loadSecNames.push_back(newSecs[i]->getSecName());
    
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
	    newshdr = elf64_getshdr(newscn);
	    newshdr->sh_name = shstrtabDataSize;
	    newshdr->sh_flags = 0;
	    if(newSecs[i]->getFlags() && Section::textSection)
		newshdr->sh_flags |=  SHF_EXECINSTR | SHF_ALLOC;
	    if (newSecs[i]->getFlags() && Section::dataSection)    
	        newshdr->sh_flags |=  SHF_WRITE | SHF_ALLOC;
	    newshdr->sh_type = SHT_PROGBITS;
	    if(shdr->sh_type == SHT_NOBITS)
	    	newshdr->sh_offset = shdr->sh_offset;
	    else
	    	newshdr->sh_offset = shdr->sh_offset+shdr->sh_size;
	    newshdr->sh_addr = newSecs[i]->getSecAddr();

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
	    
//       newshdr->sh_offset = shdr->sh_offset;

	    newshdr->sh_link = SHN_UNDEF;
	    newshdr->sh_info = 0;
	    newshdr->sh_addralign = 4;
       	    newshdr->sh_entsize = 0;

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
    	    shstrtabDataSize += newSecs[i]->getSecName().size() + 1;
	    /* DEBUG */
	    fprintf(stderr, "Added New Section : secAddr 0x%lx, secOff 0x%lx, secsize 0x%lx, end 0x%lx\n",
	                                     newshdr->sh_addr, newshdr->sh_offset, newshdr->sh_size, newshdr->sh_offset + newshdr->sh_size );
	}
	else
	{
	    nonLoadableSecs.push_back(newSecs[i]);
	    nonLoadableNamesSize += newSecs[i]->getSecName().size()+1;
	} 
    }	
    return true;
}
	
void emitElf::addSectionNames(Elf_Data *&newdata, Elf_Data *olddata, unsigned shstrtabDataSize, unsigned nonLoadableNamesSize, std::vector<string> &loadSecNames)
{
    newdata->d_buf = (char *)malloc(shstrtabDataSize + nonLoadableNamesSize);
    memcpy(newdata->d_buf, olddata->d_buf, olddata->d_size);
    newdata->d_size = shstrtabDataSize + nonLoadableNamesSize;

    // change the shstrtab section data accordingly
    if(shstrtabDataSize > olddata->d_size)
    {
	char *ptr = (char *)newdata->d_buf+olddata->d_size;
    	for(unsigned i=0;i<loadSecNames.size(); i++)
	{
	    memcpy(ptr, loadSecNames[i].c_str(), loadSecNames[i].length());
	    memcpy(ptr+loadSecNames[i].length(), "\0", 1);
	    ptr += loadSecNames[i].length()+1;
	}    
    }
    if(nonLoadableNamesSize > 0)
    {
    	char *ptr = (char *)newdata->d_buf + shstrtabDataSize;
    	for(unsigned i=0;i<nonLoadableSecs.size(); i++)
	{
	    memcpy(ptr, nonLoadableSecs[i]->getSecName().c_str(), nonLoadableSecs[i]->getSecName().length());
	    memcpy(ptr+nonLoadableSecs[i]->getSecName().length(), "\0", 1);
	    ptr += nonLoadableSecs[i]->getSecName().length()+1;
	}    
    }
}

bool emitElf::createNonLoadableSections(Elf64_Shdr *shdr, unsigned shstrtabDataSize, unsigned newSecSize)
{
    Elf_Scn *newscn;
    Elf_Data *newdata = NULL;
    Elf64_Shdr *newshdr;
    
    Elf64_Shdr *prevshdr = shdr; 
    //All of them that are left are non-loadable. stack'em up at the end.
    int prevNameSize = 0;
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
	newshdr = elf64_getshdr(newscn);
	newshdr->sh_name = shstrtabDataSize + prevNameSize;
	if(nonLoadableSecs[i]->getFlags() && Section::textSection)		//Text Section
	{
	    newshdr->sh_type = SHT_PROGBITS;
	    newshdr->sh_flags = SHF_EXECINSTR | SHF_WRITE;
            newshdr->sh_entsize = 1;
	    newdata->d_type = ELF_T_BYTE;
	}
	else if(nonLoadableSecs[i]->getFlags() && Section::dataSection)	//Data Section
	{
	    newshdr->sh_type = SHT_PROGBITS;
	    newshdr->sh_flags = SHF_WRITE;
            newshdr->sh_entsize = 1;
	    newdata->d_type = ELF_T_BYTE;
	}
	else if(nonLoadableSecs[i]->getFlags() && Section::relocationSection)	//Relocatons section
	{
	    newshdr->sh_type = SHT_REL;
	    newshdr->sh_flags = SHF_WRITE;
            newshdr->sh_entsize = sizeof(Elf64_Rel);
	    newdata->d_type = ELF_T_BYTE;
	}
	else if(nonLoadableSecs[i]->getFlags() && Section::symtabSection)
	{
	    newshdr->sh_type = SHT_SYMTAB;
            newshdr->sh_entsize = sizeof(Elf64_Sym);
	    newdata->d_type = ELF_T_SYM;
	    newshdr->sh_link = newSecSize+i+1;   //.symtab section should have sh_link = index of .strtab
	    newshdr->sh_flags=  0;
	}
	else if(nonLoadableSecs[i]->getFlags() && Section::stringSection)	//String table Section
	{
	    newshdr->sh_type = SHT_STRTAB;
            newshdr->sh_entsize = 1;
	    newdata->d_type = ELF_T_BYTE;
	    newshdr->sh_link = SHN_UNDEF; 
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
	
	prevNameSize += nonLoadableSecs[i]->getSecName().length() + 1;
	prevshdr = newshdr;
    }	
    return true;
}    	

bool emitElf::checkIfStripped(Symtab *obj, std::vector<Symbol *>&functions, std::vector<Symbol *>&variables, std::vector<Symbol *>&mods, std::vector<Symbol *>&notypes)
{
    unsigned i;
//    if(!isStripped)
//    	return false;
    symbolNamesLength = 1;
    symbolStrs.push_back("");
    for(i=0; i<functions.size();i++)
        getBackSymbol(functions[i]);
    for(i=0; i<variables.size();i++)
        getBackSymbol(variables[i]);
    for(i=0; i<mods.size();i++)
        getBackSymbol(mods[i]);
    for(i=0; i<notypes.size();i++)
        getBackSymbol(notypes[i]);
    Elf64_Sym *syms = (Elf64_Sym *)malloc(symbols.size()* sizeof(Elf64_Sym));
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
    
    char *data = (char *)malloc(symbols.size()*sizeof(Elf64_Sym));
    memcpy(data,syms, symbols.size()*sizeof(Elf64_Sym));

    if(!isStripped)
    {
        Section *sec;
        obj->findSection(sec,".symtab");
	sec->setPtrToRawData(data, symbols.size()*sizeof(Elf64_Sym));
    }
    else
    	obj->addSection(0, data, symbols.size()*sizeof(Elf64_Sym), ".symtab", 8);

    char *strData = (char *)malloc(symbolNamesLength);
    memcpy(strData, str, symbolNamesLength);
    
    if(!isStripped)
    {
        Section *sec;
        obj->findSection(sec,".strtab");
	sec->setPtrToRawData(strData, symbolNamesLength);
    }
    else
        obj->addSection(0, strData, symbolNamesLength , ".strtab", 16);
    return true;
}

void emitElf::log_elferror(void (*err_func)(const char *), const char* msg) {
    const char* err = elf_errmsg(elf_errno());
    err = err ? err: "(bad elf error)";
    string str = string(err)+string(msg);
    err_func(str.c_str());
}
