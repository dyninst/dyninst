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

#include <algorithm>
#include "common/h/parseauxv.h"
#include "Symtab.h"
#include "emitElf.h"

#if defined(os_solaris)
#include <sys/link.h>
#endif


extern void pd_log_perror(const char *msg);
using namespace Dyninst;
using namespace Dyninst::SymtabAPI;
using namespace std;

extern const char *pdelf_get_shnames(Elf_X &elf);

struct sortByIndex
{
  bool operator ()(Symbol * lhs, Symbol* rhs) {
    return lhs->getIndex() < rhs->getIndex();
  }
};

/* Descriptor for data to be converted to or from memory format.  */
typedef struct
{
  void *d_buf;			/* Pointer to the actual data.  */
  Elf_Type d_type;		/* Type of this piece of data.  */
  unsigned int d_version;	/* ELF version.  */
  size_t d_size;		/* Size in bytes.  */
  off64_t d_off;			/* Offset into section.  */
  size_t d_align;		/* Alignment in section.  */
} Elf_Data64;

bool libelfso0Flag;
void setVersion(){
  libelfso0Flag = true;
#if !defined(os_solaris)
  unsigned nEntries;
  map_entries *maps = getLinuxMaps(getpid(), nEntries);
  for(unsigned i=0; i< nEntries; i++){
    if(strstr(maps[i].path, "libelf") && (strstr(maps[i].path,"1.so") ||strstr(maps[i].path,"so.1"))){
      libelfso0Flag = false;
      break;
    }
  }
#endif
}

unsigned int elfHash(const char *name)
{
  unsigned int h = 0, g;

  while (*name) {
    h = (h << 4) + *name++;
    if ((g = h & 0xf0000000))
      h ^= g >> 24;
    h &= ~g;
  }
  return h;
}
      
static int elfSymType(Symbol::SymbolType sType)
{
  switch (sType) {
  case Symbol::ST_MODULE: return STT_FILE;
  case Symbol::ST_SECTION: return STT_SECTION;
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

static int elfSymVisibility(Symbol::SymbolVisibility sVisibility)
{
  switch (sVisibility) {
  case Symbol::SV_DEFAULT: return STV_DEFAULT;
  case Symbol::SV_INTERNAL: return STV_INTERNAL;
  case Symbol::SV_HIDDEN: return STV_HIDDEN;
  case Symbol::SV_PROTECTED: return STV_PROTECTED;
  default: return STV_DEFAULT;
  }
}

emitElf::emitElf(Elf_X &oldElfHandle_, bool isStripped_, int BSSexpandflag_, void (*err_func)(const char *)) :
  oldElfHandle(oldElfHandle_), BSSExpandFlag(BSSexpandflag_), isStripped(isStripped_), err_func_(err_func)

{
  firstNewLoadSec = NULL;
  textData = NULL;
  symStrData = NULL;
  symTabData = NULL;
  hashData = NULL;
  rodata = NULL;
   
  if(BSSexpandflag_)
    addNewSegmentFlag = false;
  else    
    addNewSegmentFlag = true;
  oldElf = oldElfHandle.e_elfp();
  curVersionNum = 2;
  setVersion();
}

bool emitElf::createElfSymbol(Symbol *symbol, unsigned strIndex, vector<Elf32_Sym *> &symbols, bool dynSymFlag)
{
  Elf32_Sym *sym = new Elf32_Sym();
  sym->st_name = strIndex;

  sym->st_value = symbol->getAddr();
  sym->st_size = symbol->getSize();
  sym->st_other = ELF32_ST_VISIBILITY(elfSymVisibility(symbol->getVisibility()));
  sym->st_info = (unsigned char) ELF32_ST_INFO(elfSymBind(symbol->getLinkage()), elfSymType (symbol->getType()));

  if (symbol->getSec())
    {
#if defined(os_solaris)
      sym->st_shndx = (Elf32_Half) symbol->getSec()->getRegionNumber();
#else
      sym->st_shndx = (Elf32_Section) symbol->getSec()->getRegionNumber();
#endif
    }
  else if (symbol->isAbsolute())
    {
      sym->st_shndx = SHN_ABS;
    }
  else
    {
      sym->st_shndx = 0;
    }

  symbols.push_back(sym);

  if (dynSymFlag) 
    {
      //printf("dynamic symbol: %s\n", symbol->getName().c_str());

#if !defined(os_solaris)
      char msg[2048];
      char *mpos = msg;
      msg[0] = '\0';
      string fileName;

      if (!symbol->getVersionFileName(fileName))
	{
	  //verdef entry
	  vector<string> *vers;
	  if (!symbol->getVersions(vers))
	    {
	      if (symbol->getLinkage() == Symbol::SL_GLOBAL)
		{
		  versionSymTable.push_back(1);
		  mpos += sprintf(mpos, "  global\n");
		}
	      else
		{
		  versionSymTable.push_back(0);
		  mpos += sprintf(mpos, "  local\n");
		}
	    }
	  else 
	    {
	      if (vers->size() > 0)
		{
		  // new verdef entry
		  mpos += sprintf(mpos, "verdef: symbol=%s  version=%s ", symbol->getName().c_str(), (*vers)[0].c_str());
		  if (verdefEntries.find((*vers)[0]) != verdefEntries.end())
		    {
		      versionSymTable.push_back((unsigned short) verdefEntries[(*vers)[0]]);
		    }
		  else 
		    {
		      versionSymTable.push_back((unsigned short) curVersionNum);
		      verdefEntries[(*vers)[0]] = curVersionNum;
		      curVersionNum++;
		    }
		}
	      // add all versions to the verdef entry
	      for (unsigned i=0; i< vers->size(); i++)
		{
		  mpos += sprintf(mpos, "  {%s}", (*vers)[i].c_str());
		  if (versionNames.find((*vers)[i]) == versionNames.end())
		    {
		      versionNames[(*vers)[i]] = 0;
		    }

		  if (find( verdauxEntries[verdefEntries[(*vers)[0]]].begin(),
			    verdauxEntries[verdefEntries[(*vers)[0]]].end(),
			    (*vers)[i]) == verdauxEntries[verdefEntries[(*vers)[0]]].end())
		    {
		      verdauxEntries[verdefEntries[(*vers)[0]]].push_back((*vers)[i]);
		    }
		}
	      mpos += sprintf(mpos, "\n");
	    }
	}
      else 
	{           
	  //verneed entry
	  mpos += sprintf(mpos, "need: symbol=%s    filename=%s\n", 
			  symbol->getName().c_str(), fileName.c_str());

	  vector<string> *vers;

	  if (!symbol->getVersions(vers) || (vers && vers->size() != 1)) 
	    {
	      // add an unversioned dependency
	      if (fileName != "") 
		{
		  if (find(unversionedNeededEntries.begin(),
			   unversionedNeededEntries.end(),
			   fileName) == unversionedNeededEntries.end()) 
		    {
		      mpos += sprintf(mpos, "  new unversioned: %s\n", fileName.c_str());
		      unversionedNeededEntries.push_back(fileName);
		    }

		  if (symbol->getLinkage() == Symbol::SL_GLOBAL) {
		    mpos += sprintf(mpos, "  global (w/ filename)\n");
		    versionSymTable.push_back(1);
		  }
		  else {
		    mpos += sprintf(mpos, "  local (w/ filename)\n");
		    versionSymTable.push_back(0);
		  }
		}
	    } 
	  else 
	    {
	      if (!vers)
		{
		  fprintf(stderr, "%s[%d]:  weird inconsistency here...  getVersions returned NULL\n",
			  FILE__, __LINE__);
		}
	      else
		{
		  // There should only be one version string by this time
		  //If the verison name already exists then add the same version number to the version symbol table
		  //Else give a new number and add it to the mapping.
		  if (versionNames.find((*vers)[0]) == versionNames.end()) 
		    {
		      mpos += sprintf(mpos, "  new version name: %s\n", (*vers)[0].c_str());
		      versionNames[(*vers)[0]] = 0;
		    }

		  if (verneedEntries.find(fileName) != verneedEntries.end())
		    {
		      if (verneedEntries[fileName].find((*vers)[0]) != verneedEntries[fileName].end()) 
			{
			  mpos += sprintf(mpos, "  vernum: %d\n", verneedEntries[fileName][(*vers)[0]]);
			  versionSymTable.push_back((unsigned short) verneedEntries[fileName][(*vers)[0]]);
			}
		      else
			{
			  mpos += sprintf(mpos, "  new entry #%d: %s [%s]\n", 
					  curVersionNum, (*vers)[0].c_str(), fileName.c_str());
			  versionSymTable.push_back((unsigned short) curVersionNum);
			  verneedEntries[fileName][(*vers)[0]] = curVersionNum;
			  curVersionNum++;
			}
		    }
		  else
		    {
		      mpos += sprintf(mpos, "  new entry #%d: %s [%s]\n", 
				      curVersionNum, (*vers)[0].c_str(), fileName.c_str());
		      versionSymTable.push_back((unsigned short) curVersionNum);
		      verneedEntries[fileName][(*vers)[0]] = curVersionNum;
		      curVersionNum++;
		    }
		} 
	    }
	}
#ifdef BINEDIT_DEBUG
      printf("%s", msg);
#endif
#endif
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
  dataSegEnd = 0;
  for(unsigned i=0;i<oldEhdr->e_phnum;i++)
    {
      if(tmp->p_type == PT_LOAD)
        {
	  if (dataSegEnd < tmp->p_vaddr+tmp->p_memsz)
	    dataSegEnd = tmp->p_vaddr+tmp->p_memsz;
        }
      tmp++;
    }
}

// Rename an old section. Lengths of old and new names must match.
// Only renames the FIRST matching section encountered.
void emitElf::renameSection(const std::string &oldStr, const std::string &newStr, bool renameAll) {
  assert(oldStr.length() == newStr.length());
  for (unsigned k = 0; k < secNames.size(); k++) {
    if (secNames[k] == oldStr) {
      secNames[k].replace(0, oldStr.length(), newStr);
      if (!renameAll)
	break;
    }
  }
}

bool emitElf::driver(Symtab *obj, string fName){
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
    
  newEhdr->e_shnum = (Elf32_Half) (newEhdr->e_shnum +  newSecs.size());

  // Find the end of text and data segments
  findSegmentEnds();
  unsigned insertPoint = oldEhdr->e_shnum;
  unsigned NOBITSstartPoint = oldEhdr->e_shnum;

  if(addNewSegmentFlag)
    {
      newEhdr->e_phoff = sizeof(Elf32_Ehdr);
    }
    
  /* flag the file for no auto-layout */
  elf_flagelf(newElf,ELF_C_SET,ELF_F_LAYOUT);
    
  Elf_Scn *scn = NULL, *newscn;
  Elf_Data *newdata = NULL, *olddata = NULL;
  Elf32_Shdr *newshdr, *shdr = NULL;
  dyn_hash_map<unsigned, unsigned> secLinkMapping;
  dyn_hash_map<unsigned, unsigned> secInfoMapping;
  dyn_hash_map<unsigned, unsigned> changeMapping;
  dyn_hash_map<std::string, unsigned> newNameIndexMapping;
  dyn_hash_map<unsigned, std::string> oldIndexNameMapping;

  bool createdLoadableSections = false;
  unsigned scncount;
  unsigned sectionNumber = 0;
  for (scncount = 0; (scn = elf_nextscn(oldElf, scn)); scncount++) {
    //copy sections from oldElf to newElf
    shdr = elf32_getshdr(scn);

    // resolve section name
    const char *name = &shnames[shdr->sh_name];
    obj->findRegion(foundSec, shdr->sh_addr, shdr->sh_size);
    sectionNumber++;
    changeMapping[sectionNumber] = 0;
    oldIndexNameMapping[scncount+1] = string(name);
    newNameIndexMapping[string(name)] = sectionNumber;


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
	//printf("  SYMTAB: copy from NEW data [%s]  0x%lx to 0x%lx  sz=%d\n", 
	//name, foundSec->getPtrToRawData(), newshdr->sh_addr, foundSec->getDiskSize());
	newdata->d_buf = (char *)malloc(foundSec->getDiskSize());
	memcpy(newdata->d_buf, foundSec->getPtrToRawData(), foundSec->getDiskSize());
	newdata->d_size = foundSec->getDiskSize();
	newshdr->sh_size = foundSec->getDiskSize();
      }
    else if(olddata->d_buf)     //copy the data buffer from oldElf
      {
	//printf("  SYMTAB: copy from old data [%s]\n", name);
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
      
    vector <vector <unsigned long> > moveSecAddrRange = obj->getObject()->getMoveSecAddrRange();

    for (unsigned i = 0 ; i != moveSecAddrRange.size(); i++) {
      if ( (moveSecAddrRange[i][0] == shdr->sh_addr) ||
	   (shdr->sh_addr >= moveSecAddrRange[i][0] && shdr->sh_addr < moveSecAddrRange[i][1]) )
	{
	  newshdr->sh_type = SHT_PROGBITS;
	  changeMapping[sectionNumber] = 1;
	  string newName = ".o";
	  newName.append(name, 2, strlen(name));
	  renameSection((string)name, newName, false);
	}

    }
		
    if(obj->getObject()->getStrtabAddr() != 0 &&
       obj->getObject()->getStrtabAddr() == shdr->sh_addr)
      {
	symStrData = newdata;
	updateSymbols(symTabData, symStrData, loadSecTotalSize);
      }
	    
    //Change sh_link for .symtab to point to .strtab
    if(obj->getObject()->getSymtabAddr() != 0 && 
       obj->getObject()->getSymtabAddr() == shdr->sh_addr){
      newshdr->sh_link = secNames.size();
      symTabData = newdata;
    }

    if(obj->getObject()->getTextAddr() != 0 &&
       obj->getObject()->getTextAddr() == shdr->sh_addr){
      textData = newdata;
    }

    if(obj->getObject()->getDynamicAddr() != 0 &&
       obj->getObject()->getDynamicAddr() == shdr->sh_addr){
      dynData = newdata;
      dynSegOff = newshdr->sh_offset;
      dynSegAddr = newshdr->sh_addr;
      // Change the data to update the relocation addr
      newshdr->sh_type = SHT_PROGBITS;
      changeMapping[sectionNumber] = 1;
      string newName = ".o";
      newName.append(name, 2, strlen(name));
      renameSection((string)name, newName, false);
      //newSecs.push_back(new Section(oldEhdr->e_shnum+newSecs.size(),".dynamic", /*addr*/, newdata->d_size, dynData, Section::dynamicSection, true));
    }


    // Change offsets of sections based on the newly added sections
    if(addNewSegmentFlag) {
      if (newshdr->sh_offset > 0) 
	newshdr->sh_offset += pgSize;
    }		

    if(scncount > insertPoint && newshdr->sh_offset > 0)
      newshdr->sh_offset += loadSecTotalSize;

    if (newshdr->sh_offset > 0)
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
	
    secLinkMapping[sectionNumber] = shdr->sh_link; 
    secInfoMapping[sectionNumber] = shdr->sh_info; 


    //Insert new loadable sections at the end of data segment			
    if (shdr->sh_addr+shdr->sh_size == dataSegEnd && !createdLoadableSections) {
      createdLoadableSections = true;
      insertPoint = scncount;
      if(!createLoadableSections(newshdr, loadSecTotalSize, extraAlignSize, newNameIndexMapping, sectionNumber))
	return false;
    }

    elf_update(newElf, ELF_C_NULL);
  }

  // Add non-loadable sections at the end of object file
  if(!createNonLoadableSections(newshdr))
    return false;
   
  //Add the section header table right at the end        
  addSectionHeaderTable(newshdr);

  // Second iteration to fix the link fields to point to the correct section
  scn = NULL;

  for (scncount = 0; (scn = elf_nextscn(newElf, scn)); scncount++){
    shdr = elf32_getshdr(scn);
    if (changeMapping[scncount+1] == 0 && secLinkMapping[scncount+1] != 0) {
      unsigned linkIndex = secLinkMapping[scncount+1];
      string secName = oldIndexNameMapping[linkIndex];
      unsigned newLinkIndex = newNameIndexMapping[secName];
      shdr->sh_link = newLinkIndex;
    } 

    if (changeMapping[scncount+1] == 0 && secInfoMapping[scncount+1] != 0) {
      // For REL and RELA section, info field is a section index - hence of the index changes, the info field must change.
      // For SYMTAB and DYNSYM, info field is OS specific - so just copy it.
      // For others, info field is 0, so just copy it
      if (shdr->sh_type == SHT_REL || shdr->sh_type == SHT_RELA) {
	unsigned infoIndex = secInfoMapping[scncount+1];
	string secName = oldIndexNameMapping[infoIndex];
	unsigned newInfoIndex = newNameIndexMapping[secName];
	shdr->sh_info = newInfoIndex;
      } 
    }
	
  }


  newEhdr->e_shstrndx = (Elf32_Half) scncount;

  // Move the section header to the end
  newEhdr->e_shoff =shdr->sh_offset+shdr->sh_size;
  if(addNewSegmentFlag)
    newEhdr->e_shoff += pgSize;

  //copy program headers
  oldPhdr = elf32_getphdr(oldElf);
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

void emitElf::fixPhdrs(unsigned &loadSecTotalSize, unsigned &extraAlignSize)
{
  unsigned pgSize = getpagesize();
  Elf32_Phdr *tmp = oldPhdr;
  if(addNewSegmentFlag) {
    if(firstNewLoadSec)
      newEhdr->e_phnum= (Elf32_Half) (oldEhdr->e_phnum + 1);
    else
      newEhdr->e_phnum= oldEhdr->e_phnum;
  }
  if(BSSExpandFlag)
    newEhdr->e_phnum= oldEhdr->e_phnum;
    
  bool added_new_sec = false;
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
	newPhdr->p_memsz = dynSegSize;
	newPhdr->p_filesz = newPhdr->p_memsz;
      }
      else if(tmp->p_type == PT_PHDR){
	newPhdr->p_vaddr = tmp->p_vaddr - pgSize;
	newPhdr->p_paddr = newPhdr->p_vaddr;
	newPhdr->p_filesz = sizeof(Elf32_Phdr) * newEhdr->e_phnum;
	newPhdr->p_memsz = newPhdr->p_filesz;
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
	    if (tmp->p_vaddr > pgSize) {
	      newPhdr->p_vaddr = tmp->p_vaddr - pgSize;
	      newPhdr->p_paddr = newPhdr->p_vaddr;
	      newPhdr->p_filesz += pgSize;
	      newPhdr->p_memsz = newPhdr->p_filesz;
	    }
	  }
	// update first segment header with the page size offset
	if ((tmp->p_type == PT_LOAD && tmp->p_flags == 5 && tmp->p_vaddr == 0) ||
	    (tmp->p_type == PT_LOAD && tmp->p_flags == 6) || 
	    tmp->p_type == PT_NOTE || 
	    tmp->p_type == PT_INTERP)
	  newPhdr->p_offset += pgSize;
      } 
      newPhdr++;
      if(addNewSegmentFlag) {
	if(tmp->p_type == PT_LOAD && 
	   (tmp->p_flags == 6 || tmp->p_flags == 7) && 
	   firstNewLoadSec && !added_new_sec)
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
	    added_new_sec = true;
	    newPhdr++;
	  }
      }    
      tmp++;
    }
}

#if !defined(os_solaris)
//This method updates the .dynamic section to reflect the changes to the relocation section
void emitElf::updateDynamic(unsigned tag, Elf32_Addr val){
  if(dynamicSecData.find(tag) == dynamicSecData.end()) {
    //printf(" Error updateDynamic - cannot find tag \n");
    return;
  }
    
  switch(dynamicSecData[tag][0]->d_tag){
  case DT_STRSZ:
  case DT_RELSZ:
  case DT_RELASZ:
    dynamicSecData[tag][0]->d_un.d_val = val;
    break;
  case DT_HASH:
  case 0x6ffffef5: // DT_GNU_HASH (not defined on all platforms)
  case DT_SYMTAB:
  case DT_STRTAB:
  case DT_REL:
  case DT_RELA:
  case DT_VERSYM:
    dynamicSecData[tag][0]->d_un.d_ptr = val;
    break;
  case DT_VERNEED:
    dynamicSecData[tag][0]->d_un.d_ptr = val;
    dynamicSecData[DT_VERNEEDNUM][0]->d_un.d_val = verneednum;
    break;
  case DT_VERDEF:
    dynamicSecData[tag][0]->d_un.d_ptr = val;
    dynamicSecData[DT_VERDEFNUM][0]->d_un.d_val = verdefnum;
    break;
  }
}
#endif    

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

bool emitElf::createLoadableSections(Elf32_Shdr* &shdr, unsigned &loadSecTotalSize, unsigned &extraAlignSize, dyn_hash_map<std::string, unsigned> &newNameIndexMapping, unsigned &sectionNumber)
{
  Elf_Scn *newscn;
  Elf_Data *newdata = NULL;
  Elf_Data64 *newdata64 = NULL;
  Elf32_Shdr *newshdr;
  std::vector<Elf32_Shdr *> updateDynLinkShdr;
  std::vector<Elf32_Shdr *> updateStrLinkShdr;
  firstNewLoadSec = NULL;
  unsigned pgSize = getpagesize();
  unsigned strtabIndex = 0;
  unsigned dynsymIndex = 0;
  Elf32_Shdr *prevshdr = NULL;


  for(unsigned i=0; i < newSecs.size(); i++)
    {
      if(newSecs[i]->isLoadable())
    	{
	  secNames.push_back(newSecs[i]->getRegionName());
	  newNameIndexMapping[newSecs[i]->getRegionName()] = secNames.size() -1;
	  sectionNumber++;
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
	  if(!libelfso0Flag)
	    newdata64 = (Elf_Data64 *)malloc(sizeof(Elf_Data64));

	  // Fill out the new section header	
	  newshdr = elf32_getshdr(newscn);
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
	      newshdr->sh_entsize = sizeof(Elf32_Rel);
	      updateDynLinkShdr.push_back(newshdr);
	      if(!libelfso0Flag) {
		newdata64->d_type = ELF_T_REL;
		newdata64->d_align = 4;
	      }
	      else {
		newdata->d_type = ELF_T_REL;
		newdata->d_align = 4;
	      }
#if !defined(os_solaris)
	      updateDynamic(DT_REL, newshdr->sh_addr);
#endif
            }
	  else if(newSecs[i]->getRegionType() == Region::RT_RELA)    //Relocation section
            {
	      newshdr->sh_type = SHT_RELA;
	      newshdr->sh_flags = SHF_ALLOC;
	      newshdr->sh_entsize = sizeof(Elf32_Rela);
	      updateDynLinkShdr.push_back(newshdr);
	      newdata->d_type = ELF_T_RELA;
	      newdata->d_align = 4;
#if !defined(os_solaris)
	      updateDynamic(DT_RELA, newshdr->sh_addr);
#endif
            }
	  else if(newSecs[i]->getRegionType() == Region::RT_STRTAB)    //String table Section
            {
	      newshdr->sh_type = SHT_STRTAB;
	      newshdr->sh_entsize = 0;
	      if(!libelfso0Flag) {
		newdata64->d_type = ELF_T_BYTE;
		newdata64->d_align = 1;
	      }
	      else {
		newdata->d_type = ELF_T_BYTE;
		newdata->d_align = 1;
	      }
	      newshdr->sh_link = SHN_UNDEF;
	      newshdr->sh_flags=  SHF_ALLOC;
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
	      newshdr->sh_entsize = sizeof(Elf32_Sym);
	      if(!libelfso0Flag) {
		newdata64->d_type = ELF_T_SYM;
		newdata64->d_align = 4;
	      }
	      else {
		newdata->d_type = ELF_T_SYM;
		newdata->d_align = 4;
	      }
	      newshdr->sh_link = secNames.size();   //.symtab section should have sh_link = index of .strtab for .dynsym
	      newshdr->sh_flags = SHF_ALLOC ;
	      newshdr->sh_info = 1;
	      dynsymIndex = secNames.size()-1;
#if !defined(os_solaris)
	      updateDynamic(DT_SYMTAB, newshdr->sh_addr);
#endif
            }
	  else if(newSecs[i]->getRegionType() == Region::RT_DYNAMIC)
            {
#if !defined(os_solaris)
	      newshdr->sh_entsize = sizeof(Elf32_Dyn);
#endif            
	      newshdr->sh_type = SHT_DYNAMIC;
	      if(!libelfso0Flag) {
		newdata64->d_type = ELF_T_DYN;
		newdata64->d_align = 4;
	      }
	      else {
		newdata->d_type = ELF_T_DYN;
		newdata->d_align = 4;
	      }
	      updateStrLinkShdr.push_back(newshdr);
	      newshdr->sh_flags=  SHF_ALLOC | SHF_WRITE;
	      dynSegOff = newshdr->sh_offset;
	      dynSegAddr = newshdr->sh_addr;
	      dynSegSize = newSecs[i]->getDiskSize();
            }
	  else if(newSecs[i]->getRegionType() == Region::RT_HASH)
            {
	      newshdr->sh_entsize = sizeof(Elf32_Word);
	      newshdr->sh_type = SHT_HASH;
	      if(!libelfso0Flag) {
		newdata64->d_type = ELF_T_WORD;
		newdata64->d_align = 4;
	      } else {
		newdata->d_type = ELF_T_WORD;
		newdata->d_align = 4;
	      }
	      updateDynLinkShdr.push_back(newshdr);
	      newshdr->sh_flags=  SHF_ALLOC;
	      newshdr->sh_info = 0;
#if !defined(os_solaris)
	      updateDynamic(DT_HASH, newshdr->sh_addr);
#endif
            }
#if !defined(os_solaris)
	  else if(newSecs[i]->getRegionType() == Region::RT_SYMVERSIONS)
            {
	      newshdr->sh_type = SHT_GNU_versym;
	      newshdr->sh_entsize = sizeof(Elf32_Half);
	      newshdr->sh_addralign = 2;
	      if(!libelfso0Flag) {
		newdata64->d_type = ELF_T_HALF;
		newdata64->d_align = 2;
	      }
	      else {
		newdata->d_type = ELF_T_HALF;
		newdata->d_align = 2;
	      }
	      updateDynLinkShdr.push_back(newshdr);
	      newshdr->sh_flags = SHF_ALLOC ;
	      updateDynamic(DT_VERSYM, newshdr->sh_addr);
            }
	  else if(newSecs[i]->getRegionType() == Region::RT_SYMVERNEEDED)
            {
	      newshdr->sh_type = SHT_GNU_verneed;
	      newshdr->sh_entsize = 0;
	      newshdr->sh_addralign = 4;
	      if(!libelfso0Flag) {
		newdata64->d_type = ELF_T_VNEED;
		newdata64->d_align = 4;
	      }
	      else {
		newdata->d_type = ELF_T_VNEED;
		newdata->d_align = 4;
	      }
	      updateStrLinkShdr.push_back(newshdr);
	      newshdr->sh_flags = SHF_ALLOC ;
	      newshdr->sh_info = verneednum;
	      updateDynamic(DT_VERNEED, newshdr->sh_addr);
            }
	  else if(newSecs[i]->getRegionType() == Region::RT_SYMVERDEF)
            {
	      newshdr->sh_type = SHT_GNU_verdef;
	      newshdr->sh_entsize = 0;
	      if(!libelfso0Flag) {
		newdata64->d_type = ELF_T_VDEF;
		newdata64->d_align = 4;
	      }
	      else {
		newdata->d_type = ELF_T_VDEF;
		newdata->d_align = 4;
	      }
	      updateStrLinkShdr.push_back(newshdr);
	      newshdr->sh_flags = SHF_ALLOC ;
	      newshdr->sh_info = verdefnum;
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
	  if(!libelfso0Flag) {
	    newdata64->d_buf = malloc(newSecs[i]->getDiskSize());
	    memcpy(newdata64->d_buf, newSecs[i]->getPtrToRawData(), newSecs[i]->getDiskSize());
	    newdata64->d_off = 0;
	    newdata64->d_version = 1;
	    newdata64->d_size = newSecs[i]->getDiskSize();
	    newshdr->sh_size = newdata64->d_size;
	    memcpy(newdata, newdata64, sizeof(Elf_Data));
	  }
	  else {
	    newdata->d_buf = malloc(newSecs[i]->getDiskSize());
	    memcpy(newdata->d_buf, newSecs[i]->getPtrToRawData(), newSecs[i]->getDiskSize());
	    newdata->d_off = 0;
	    newdata->d_version = 1;
	    newdata->d_size = newSecs[i]->getDiskSize();
	    newshdr->sh_size = newdata->d_size;
	  }

	  loadSecTotalSize += newshdr->sh_size;
	  elf_update(newElf, ELF_C_NULL);

	  shdr = newshdr;
	  if(!firstNewLoadSec)
	    firstNewLoadSec = shdr;
	  secNameIndex += newSecs[i]->getRegionName().size() + 1;
	  prevshdr = newshdr;
	}
      else
	nonLoadableSecs.push_back(newSecs[i]);
    }	
    
  for(unsigned i=0; i < updateDynLinkShdr.size(); i++) {
    newshdr = updateDynLinkShdr[i];
    newshdr->sh_link = dynsymIndex;   
  }
    
  for(unsigned i=0; i < updateStrLinkShdr.size(); i++) {
    newshdr = updateStrLinkShdr[i];
    newshdr->sh_link = strtabIndex;   
  }
   
  return true;
}
	
bool emitElf::addSectionHeaderTable(Elf32_Shdr *shdr) {
  Elf_Scn *newscn;
  Elf_Data *newdata = NULL;
  Elf_Data64 *newdata64 = NULL;
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
  if(!libelfso0Flag)
    newdata64 = (Elf_Data64 *) malloc(sizeof(Elf_Data64));
    
  //Fill out the new section header
  newshdr = elf32_getshdr(newscn);
  newshdr->sh_name = secNameIndex;
  secNames.push_back(".shstrtab");
  secNameIndex += 10;
  newshdr->sh_type = SHT_STRTAB;
  newshdr->sh_entsize = 1;
  newshdr->sh_link = SHN_UNDEF; 
  newshdr->sh_flags=  0;

  newshdr->sh_offset = shdr->sh_offset+shdr->sh_size;
  newshdr->sh_addr = 0;
  newshdr->sh_info = 0;
  newshdr->sh_addralign = 4;

  char *ptr;
  //Set up the data
  if(!libelfso0Flag){
    newdata64->d_buf = (char *)malloc(secNameIndex);
    ptr = (char *)newdata64->d_buf;
  }
  else{
    newdata->d_buf = (char *)malloc(secNameIndex);
    ptr = (char *)newdata->d_buf;
  }
  for(unsigned i=0;i<secNames.size(); i++)
    {
      memcpy(ptr, secNames[i].c_str(), secNames[i].length());
      memcpy(ptr+secNames[i].length(), "\0", 1);
      ptr += secNames[i].length()+1;
    }    

  if(!libelfso0Flag){
    newdata64->d_type = ELF_T_BYTE;
    newdata64->d_size = secNameIndex;
    newdata64->d_align = 4;
    newdata64->d_version = 1;
    newdata64->d_off = 0;
    newshdr->sh_size = newdata64->d_size;
    memcpy(newdata, newdata64, sizeof(Elf_Data));
  }
  else {
    newdata->d_type = ELF_T_BYTE;
    newdata->d_size = secNameIndex;
    newdata->d_align = 4;
    newdata->d_version = 1;
    newdata->d_off = 0;
    newshdr->sh_size = newdata->d_size;
  }


  //elf_update(newElf, ELF_C_NULL);

  return true;
}

bool emitElf::createNonLoadableSections(Elf32_Shdr *&shdr)
{
  Elf_Scn *newscn;
  Elf_Data *newdata = NULL;
  Elf_Data64 *newdata64 = NULL;
  Elf32_Shdr *newshdr;
    
  Elf32_Shdr *prevshdr = shdr; 
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
      newdata64 = (Elf_Data64 *)malloc(sizeof(Elf_Data64));
    	
      //Fill out the new section header
      newshdr = elf32_getshdr(newscn);
      newshdr->sh_name = secNameIndex;
      secNameIndex += nonLoadableSecs[i]->getRegionName().length() + 1;
      if(nonLoadableSecs[i]->getRegionType() == Region::RT_TEXT)		//Text Section
	{
	  newshdr->sh_type = SHT_PROGBITS;
	  newshdr->sh_flags = SHF_EXECINSTR | SHF_WRITE;
	  newshdr->sh_entsize = 1;
	  if(!libelfso0Flag)
	    newdata64->d_type = ELF_T_BYTE;
	  else
	    newdata->d_type = ELF_T_BYTE;
    	}
      else if(nonLoadableSecs[i]->getRegionType() == Region::RT_DATA)	//Data Section
	{
	  newshdr->sh_type = SHT_PROGBITS;
	  newshdr->sh_flags = SHF_WRITE;
	  newshdr->sh_entsize = 1;
	  if(!libelfso0Flag)
	    newdata64->d_type = ELF_T_BYTE;
	  else
	    newdata->d_type = ELF_T_BYTE;
    	}
      else if(nonLoadableSecs[i]->getRegionType() == Region::RT_REL)	//Relocatons section
    	{
	  newshdr->sh_type = SHT_REL;
	  newshdr->sh_flags = SHF_WRITE;
	  newshdr->sh_entsize = sizeof(Elf32_Rel);
	  if(!libelfso0Flag)
	    newdata64->d_type = ELF_T_BYTE;
	  else
	    newdata->d_type = ELF_T_BYTE;
    	}
      else if(nonLoadableSecs[i]->getRegionType() == Region::RT_RELA)	//Relocatons section
    	{
	  newshdr->sh_type = SHT_RELA;
	  newshdr->sh_flags = SHF_WRITE;
	  newshdr->sh_entsize = sizeof(Elf32_Rela);
	  if(!libelfso0Flag)
	    newdata64->d_type = ELF_T_BYTE;
	  else
	    newdata->d_type = ELF_T_BYTE;
    	}

      else if(nonLoadableSecs[i]->getRegionType() == Region::RT_SYMTAB)
	{
	  newshdr->sh_type = SHT_SYMTAB;
	  newshdr->sh_entsize = sizeof(Elf32_Sym);
	  if(!libelfso0Flag)
	    newdata64->d_type = ELF_T_SYM;
	  else
	    newdata->d_type = ELF_T_SYM;
	  newshdr->sh_link = secNames.size();   //.symtab section should have sh_link = index of .strtab 
	  newshdr->sh_flags=  0;
	}
      else if(nonLoadableSecs[i]->getRegionType() == Region::RT_STRTAB)	//String table Section
	{
	  newshdr->sh_type = SHT_STRTAB;
	  newshdr->sh_entsize = 0;
	  if(!libelfso0Flag)
	    newdata64->d_type = ELF_T_BYTE;
	  else
	    newdata->d_type = ELF_T_BYTE;
	  newshdr->sh_link = SHN_UNDEF; 
	  newshdr->sh_flags=  0;
    	}
      /*    
	    else if(nonLoadableSecs[i]->getFlags() & Section::dynsymtabSection)
	    {
	    newshdr->sh_type = SHT_DYNSYM;
            newshdr->sh_entsize = sizeof(Elf32_Sym);
            if(!libelfso0Flag)
	    newdata64->d_type = ELF_T_SYM;
            else
	    newdata->d_type = ELF_T_SYM;
	    //newshdr->sh_link = newSecSize+i+1;   //.symtab section should have sh_link = index of .strtab
	    newshdr->sh_flags=  SHF_ALLOC | SHF_WRITE;
	    }*/
      newshdr->sh_offset = prevshdr->sh_offset+prevshdr->sh_size;
      newshdr->sh_addr = 0;
      newshdr->sh_info = 0;
      newshdr->sh_addralign = 4;

      //Set up the data
      if(!libelfso0Flag) {
	newdata64->d_buf = nonLoadableSecs[i]->getPtrToRawData();
	newdata64->d_size = nonLoadableSecs[i]->getDiskSize();
	newshdr->sh_size = newdata64->d_size;
	newdata64->d_align = 4;
	newdata64->d_off = 0;
	newdata64->d_version = 1;
	memcpy(newdata, newdata64, sizeof(Elf_Data));
      }
      else{
	newdata->d_buf = nonLoadableSecs[i]->getPtrToRawData();
	newdata->d_size = nonLoadableSecs[i]->getDiskSize();
	newshdr->sh_size = newdata->d_size;
	newdata->d_align = 4;
	newdata->d_off = 0;
	newdata->d_version = 1;
      }
	    
      //elf_update(newElf, ELF_C_NULL);
      prevshdr = newshdr;
    }	
  shdr = prevshdr;
  return true;
}    	

/* Regenerates the .symtab, .strtab sections from the symbols
 * Add new .dynsym, .dynstr sections for the newly added dynamic symbols
 * Method - For every symbol call createElfSymbol to get a Elf32_Sym corresposnding 
 *          to a Symbol object. Accumulate all and their names to form the sections
 *          and add them to the list of new sections
 */
bool emitElf::createSymbolTables(Symtab *obj, vector<Symbol *>&allSymbols, std::vector<relocationEntry> &relocation_table)
{
  unsigned i;

  //Symbol table(.symtab) symbols
  std::vector<Elf32_Sym *> symbols;

  //Symbol table(.dynsymtab) symbols
  std::vector<Elf32_Sym *> dynsymbols;

  unsigned symbolNamesLength = 1, dynsymbolNamesLength = 1;
  dyn_hash_map<string, unsigned> dynSymNameMapping;
  std::vector<std::string> symbolStrs, dynsymbolStrs;
  std::vector<Symbol *> dynsymVector;
  std::vector<Symbol *> allDynSymbols;
  std::vector<Symbol *> allSymSymbols;

  dyn_hash_map<int, Region*> secTagRegionMapping = obj->getObject()->getTagRegionMapping();

  Region *sec;
  if (secTagRegionMapping.find(DT_STRTAB) != secTagRegionMapping.end()) { 
    // .dynstr
    sec = secTagRegionMapping[DT_STRTAB];
    olddynStrData = (char *)(sec->getPtrToRawData());
    olddynStrSize = sec->getRegionSize();
    dynsymbolNamesLength = olddynStrSize+1;
  }

  // recreate a "dummy symbol"
  Elf32_Sym *sym = new Elf32_Sym();
  symbolStrs.push_back("");
  sym->st_name = 0;
  sym->st_value = 0;
  sym->st_size = 0;
  sym->st_other = 0;
  sym->st_info = (unsigned char) ELF32_ST_INFO(elfSymBind(Symbol::SL_LOCAL), elfSymType (Symbol::ST_NOTYPE));
  sym->st_shndx = SHN_UNDEF;

  symbols.push_back(sym);
  dynsymbols.push_back(sym);
  dynsymVector.push_back(Symbol::magicEmitElfSymbol());
  versionSymTable.push_back(0);

  for(i=0; i<allSymbols.size();i++) {
    if(allSymbols[i]->isInSymtab()) {
      allSymSymbols.push_back(allSymbols[i]);
    }	
    if(allSymbols[i]->isInDynSymtab()) {
      allDynSymbols.push_back(allSymbols[i]);
    }	
  }
 
  int max_index = -1;
  for(i = 0; i < allDynSymbols.size();i++) {
    if (max_index < allDynSymbols[i]->getIndex()) 
      max_index = allDynSymbols[i]->getIndex();
  }
  for(i=0; i<allDynSymbols.size(); i++) {
    if (allDynSymbols[i]->getIndex() == -1) {
      max_index++;
      allDynSymbols[i]->setIndex(max_index);
    }

    if (allDynSymbols[i]->getStrIndex() == -1) {
      // New Symbol - append to the list of strings
      dynsymbolStrs.push_back( allDynSymbols[i]->getName().c_str());
      allDynSymbols[i]->setStrIndex(dynsymbolNamesLength);
      dynsymbolNamesLength += allDynSymbols[i]->getName().length() + 1;
    } 

  }	
   
  max_index = -1;
  for(i = 0; i < allSymSymbols.size();i++) {
    if (max_index < allSymSymbols[i]->getIndex()) 
      max_index = allSymSymbols[i]->getIndex();
  }

  for(i=0; i<allSymSymbols.size(); i++) {
    if (allSymSymbols[i]->getIndex() == -1) {
      max_index++;
      allSymSymbols[i]->setIndex(max_index);
    }
  }	

  std::sort(allDynSymbols.begin(), allDynSymbols.end(), sortByIndex());
  std::sort(allSymSymbols.begin(), allSymSymbols.end(), sortByIndex());

  /* We regenerate symtab and symstr section. We do not 
     maintain the order of the strings and symbols as it was in
     the original binary. Hence, the strings in symstr have new order and 
     new index.
     On the other hand, we do not regenerate dynsym and dynstr section. We copy over
     old symbols and string in the original order as it was in the 
     original binary. We preserve sh_index of Elf symbols (from Symbol's strIndex). We append 
     new symbols and string that we create for the new binary (targ*, versions etc).
  */  

  for(i=0; i<allSymSymbols.size();i++) {
    //allSymSymbols[i]->setStrIndex(symbolNamesLength);
    createElfSymbol(allSymSymbols[i], symbolNamesLength, symbols);
    symbolStrs.push_back(allSymSymbols[i]->getName());
    symbolNamesLength += allSymSymbols[i]->getName().length()+1;
  }
  for(i=0; i<allDynSymbols.size();i++) {
    createElfSymbol(allDynSymbols[i], allDynSymbols[i]->getStrIndex(), dynsymbols, true);
    dynSymNameMapping[allDynSymbols[i]->getName().c_str()] = allDynSymbols[i]->getIndex();
    dynsymVector.push_back(allDynSymbols[i]);
  }

  //reconstruct .symtab section
  Elf32_Sym *syms = (Elf32_Sym *)malloc(symbols.size()* sizeof(Elf32_Sym));
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
      sec->setPtrToRawData(syms, symbols.size()*sizeof(Elf32_Sym));
    }
  else
    obj->addRegion(0, syms, symbols.size()*sizeof(Elf32_Sym), ".symtab", Region::RT_SYMTAB);

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

  if(dynsymbols.size() == 1)
    return true;

  //reconstruct .dynsym and .dynstr sections
  Elf32_Sym *dynsyms = (Elf32_Sym *)malloc(dynsymbols.size()* sizeof(Elf32_Sym));
  for(i=0;i<dynsymbols.size();i++)
    dynsyms[i] = *(dynsymbols[i]);

#if !defined(os_solaris)
  Elf32_Half *symVers;
  char *verneedSecData, *verdefSecData;
  unsigned verneedSecSize = 0, verdefSecSize = 0;
               
  createSymbolVersions(obj, symVers, verneedSecData, verneedSecSize, verdefSecData, verdefSecSize, dynsymbolNamesLength, dynsymbolStrs);
  // build new .hash section
  Elf32_Word *hashsecData;
  unsigned hashsecSize = 0;
  createHashSection(hashsecData, hashsecSize, dynsymVector);
  if(hashsecSize) {
    string name; 
    if (secTagRegionMapping.find(DT_HASH) != secTagRegionMapping.end()) {
      name = secTagRegionMapping[DT_HASH]->getRegionName();
      obj->addRegion(0, hashsecData, hashsecSize*sizeof(Elf32_Word), name, Region::RT_HASH, true);
    } else if (secTagRegionMapping.find(0x6ffffef5) != secTagRegionMapping.end()) { 
      // GNU_HASH - should not come to case as we implicitly change GNU_HASH to HASH in Object-elf.C
      name = secTagRegionMapping[0x6ffffef5]->getRegionName();
      obj->addRegion(0, hashsecData, hashsecSize*sizeof(Elf32_Word), name, Region::RT_HASH, true);
    } else {
      name = ".hash";
      obj->addRegion(0, hashsecData, hashsecSize*sizeof(Elf32_Word), name, Region::RT_HASH, true);
    }	
  }

  Elf32_Dyn *dynsecData;
  unsigned dynsecSize = 0;
  if(obj->findRegion(sec, ".dynamic")) {
    createDynamicSection(sec->getPtrToRawData(), sec->getDiskSize(), dynsecData, dynsecSize, dynsymbolNamesLength, dynsymbolStrs);
  }  
#endif
   
  // build map of dynamic symbol names to symbol table index (for
  // relocations)
  if(!dynsymbolNamesLength)
    return true; 

  char *dynstr = (char *)malloc(dynsymbolNamesLength);
  memcpy((void *)dynstr, (void *)olddynStrData, olddynStrSize);
  cur = olddynStrSize+1;
  for(i=0;i<dynsymbolStrs.size();i++)
    {
      strcpy(&dynstr[cur],dynsymbolStrs[i].c_str());
      cur+=dynsymbolStrs[i].length()+1;
      if ( dynSymNameMapping.find(dynsymbolStrs[i]) == dynSymNameMapping.end()) {
	dynSymNameMapping[dynsymbolStrs[i]] = allDynSymbols.size()+i;
      }
    }

  string name; 
  if (secTagRegionMapping.find(DT_SYMTAB) != secTagRegionMapping.end()) {
    name = secTagRegionMapping[DT_SYMTAB]->getRegionName();
  } else {
    name = ".dynsym";
  }
  obj->addRegion(0, dynsyms, dynsymbols.size()*sizeof(Elf32_Sym), name, Region::RT_SYMTAB, true);

  if (secTagRegionMapping.find(DT_STRTAB) != secTagRegionMapping.end()) {
    name = secTagRegionMapping[DT_STRTAB]->getRegionName();
  } else {
    name = ".dynstr";
  }
  obj->addRegion(0, dynstr, dynsymbolNamesLength , name, Region::RT_STRTAB, true);

#if !defined(os_solaris)
  //add .gnu.version, .gnu.version_r, and .gnu.version_d sections
  if (secTagRegionMapping.find(DT_VERSYM) != secTagRegionMapping.end()) {
    name = secTagRegionMapping[DT_VERSYM]->getRegionName();
  } else {
    name = ".gnu.version";
  }
  obj->addRegion(0, symVers, versionSymTable.size() * sizeof(Elf32_Half), name, Region::RT_SYMVERSIONS, true);

  if(verneedSecSize) {
    if (secTagRegionMapping.find(DT_VERNEED) != secTagRegionMapping.end()) {
      name = secTagRegionMapping[DT_VERNEED]->getRegionName();
    } else {
      name = ".gnu.version_r";
    }
    obj->addRegion(0, verneedSecData, verneedSecSize, name, Region::RT_SYMVERNEEDED, true);
  }

  if(verdefSecSize) {
    obj->addRegion(0, verdefSecData, verdefSecSize, ".gnu.version_d", Region::RT_SYMVERDEF, true);
  } 
#endif

  createRelocationSections(obj, relocation_table, dynSymNameMapping);
   
#if !defined(os_solaris)
  //add .dynamic section
  if(dynsecSize)
    obj->addRegion(0, dynsecData, dynsecSize*sizeof(Elf64_Dyn), ".dynamic", Region::RT_DYNAMIC, true);
#endif 

  if(!obj->getAllNewRegions(newSecs))
    log_elferror(err_func_, "No new sections to add");

  return true;
}

void emitElf::createRelocationSections(Symtab *obj, std::vector<relocationEntry> &relocation_table, dyn_hash_map<std::string, unsigned> &dynSymNameMapping) {
  unsigned i,j,k;

  vector<relocationEntry> newRels;
  if(newSecs.size())
    newRels = newSecs[0]->getRelocations();
    
  Elf32_Rel *rels = (Elf32_Rel *)malloc(sizeof(Elf32_Rel) * (relocation_table.size()+newRels.size()));
  Elf32_Rela *relas = (Elf32_Rela *)malloc(sizeof(Elf32_Rela) * (relocation_table.size()+newRels.size()));
  j=0; k=0;
  //reconstruct .rel
  for(i=0;i<relocation_table.size();i++) 
    {
      if (relocation_table[i].regionType() == Region::RT_REL) {
	rels[j].r_offset = relocation_table[i].rel_addr();
	if(dynSymNameMapping.find(relocation_table[i].name()) != dynSymNameMapping.end()) {
	  rels[j].r_info = ELF32_R_INFO(dynSymNameMapping[relocation_table[i].name()], relocation_table[i].getRelType());
	} else {
	  fprintf(stderr, "%s[%d]:  relocation symbol not found: %s\n", FILE__, __LINE__,
		  relocation_table[i].name().c_str());
	}
	j++;
      } else {
	relas[k].r_offset = relocation_table[i].rel_addr();
	relas[k].r_addend = relocation_table[i].addend();
	if(dynSymNameMapping.find(relocation_table[i].name()) != dynSymNameMapping.end()) {
	  relas[k].r_info = ELF32_R_INFO(dynSymNameMapping[relocation_table[i].name()], relocation_table[i].getRelType());
	} else {
	  fprintf(stderr, "%s[%d]:  relocation symbol not found: %s\n", FILE__, __LINE__,
		  relocation_table[i].name().c_str());
	}
	k++;
      }
    }
  for(i=0;i<newRels.size();i++) 
    {
      if (newRels[i].regionType() == Region::RT_REL) {
	rels[j].r_offset = newRels[i].rel_addr();
	if(dynSymNameMapping.find(newRels[i].name()) != dynSymNameMapping.end()) {
#if defined(arch_x86)
	  rels[j].r_info = ELF32_R_INFO(dynSymNameMapping[newRels[i].name()], R_386_GLOB_DAT);
#elif defined(arch_sparc)
	  //            rels[j].r_info = ELF32_R_INFO(dynSymNameMapping[newRels[i].name()], R_SPARC_GLOB_DAT);
#elif defined(arch_x86_64)
	  rels[j].r_info = ELF32_R_INFO(dynSymNameMapping[newRels[i].name()], R_X86_64_GLOB_DAT);
#elif defined(arch_power)
	  rels[j].r_info = ELF32_R_INFO(dynSymNameMapping[newRels[i].name()], R_PPC_GLOB_DAT);
#endif
	} else {
	  fprintf(stderr, "%s[%d]:  relocation symbol not found: %s\n", FILE__, __LINE__,
		  newRels[i].name().c_str());
	}
	j++;
      } else {
	relas[k].r_offset = newRels[i].rel_addr();
	relas[k].r_addend = newRels[i].addend();
	if(dynSymNameMapping.find(newRels[i].name()) != dynSymNameMapping.end()) {
#if defined(arch_x86)
	  relas[k].r_info = ELF32_R_INFO(dynSymNameMapping[newRels[i].name()], R_386_GLOB_DAT);
#elif defined(arch_sparc)
	  //            relas[k].r_info = ELF32_R_INFO(dynSymNameMapping[newRels[i].name()], R_SPARC_GLOB_DAT);
#elif defined(arch_x86_64)
	  relas[k].r_info = ELF32_R_INFO(dynSymNameMapping[newRels[i].name()], R_X86_64_GLOB_DAT);
#elif defined(arch_power)
	  relas[k].r_info = ELF32_R_INFO(dynSymNameMapping[newRels[i].name()], R_PPC_GLOB_DAT);
#endif
	} else {
	  fprintf(stderr, "%s[%d]:  relocation symbol not found: %s\n", FILE__, __LINE__,
		  newRels[i].name().c_str());
	}
	k++;
      }
    }

#if defined (os_solaris)
  fprintf(stderr, "%s[%d]:  FIXME:  This does not work on solaris\n", FILE__, __LINE__);
#else
  dyn_hash_map<int, Region*> secTagRegionMapping = obj->getObject()->getTagRegionMapping();
  if (obj->hasReldyn()) {
    string name;
    if (secTagRegionMapping.find(DT_REL) != secTagRegionMapping.end()) {
      name = secTagRegionMapping[DT_REL]->getRegionName();
    } else {
      name = ".rel.dyn";
    }
    obj->addRegion(0, rels, j*sizeof(Elf32_Rel), name, Region::RT_REL, true);
    updateDynamic(DT_RELSZ, j*sizeof(Elf32_Rel));
  }
  if (obj->hasReladyn()) {
    string name;
    if (secTagRegionMapping.find(DT_RELA) != secTagRegionMapping.end()) {
      name = secTagRegionMapping[DT_REL]->getRegionName();
    } else {
      name = ".rela.dyn";
    }
 
    obj->addRegion(0, relas, k*sizeof(Elf32_Rela), name, Region::RT_RELA, true);
    updateDynamic(DT_RELASZ, k*sizeof(Elf32_Rela));
  }
#endif

} 

#if !defined(os_solaris)
void emitElf::createSymbolVersions(Symtab *obj, Elf32_Half *&symVers, char*&verneedSecData, unsigned &verneedSecSize, char *&verdefSecData, unsigned &verdefSecSize, unsigned &dynSymbolNamesLength, vector<string> &dynStrs){

  //Add all names to the new .dynstr section
  map<string, unsigned>::iterator iter = versionNames.begin();
  for(;iter!=versionNames.end();iter++){
    iter->second = dynSymbolNamesLength;
    dynStrs.push_back(iter->first);
    dynSymbolNamesLength+= iter->first.size()+1;
  }

  //reconstruct .gnu_version section
  symVers = (Elf32_Half *)malloc(versionSymTable.size() * sizeof(Elf32_Half));
  for(unsigned i=0; i<versionSymTable.size(); i++)
    symVers[i] = versionSymTable[i];

  //reconstruct .gnu.version_r section
  verneedSecSize = 0;
  map<string, map<string, unsigned> >::iterator it = verneedEntries.begin();
  for(; it != verneedEntries.end(); it++)
    verneedSecSize += sizeof(Elf32_Verneed) + sizeof(Elf32_Vernaux) * it->second.size();

  verneedSecData = (char *)malloc(verneedSecSize);
  unsigned curpos = 0;
  verneednum = 0;
  std::vector<std::string>::iterator dit;
  for(dit = unversionedNeededEntries.begin(); dit != unversionedNeededEntries.end(); dit++) {
    // account for any substitutions due to rewriting a shared lib
    std::string name = obj->getDynLibSubstitution(*dit);
    // no need for self-references
    if (!(obj->name() == name)) {
      //printf("adding unversioned entry: %s [%s]\n", name.c_str(), obj->name().c_str());
      versionNames[name] = dynSymbolNamesLength;
      dynStrs.push_back(name);
      dynSymbolNamesLength+= (name).size()+1;
      if(find(DT_NEEDEDEntries.begin(), DT_NEEDEDEntries.end(), name) == DT_NEEDEDEntries.end())
	DT_NEEDEDEntries.push_back(name);
    }
  }
  for(it = verneedEntries.begin(); it != verneedEntries.end(); it++){
    //printf("  verneed entry: %s ", it->first.c_str());
    Elf32_Verneed *verneed = (Elf32_Verneed *)(void*)(verneedSecData+curpos);
    verneed->vn_version = 1;
    verneed->vn_cnt = (Elf32_Half) it->second.size();
    verneed->vn_file = dynSymbolNamesLength;
    versionNames[it->first] = dynSymbolNamesLength;
    dynStrs.push_back(it->first);
    dynSymbolNamesLength+= it->first.size()+1;
    if(find(DT_NEEDEDEntries.begin(), DT_NEEDEDEntries.end(), it->first) == DT_NEEDEDEntries.end())
      DT_NEEDEDEntries.push_back(it->first);
    verneed->vn_aux = sizeof(Elf32_Verneed);
    verneed->vn_next = sizeof(Elf32_Verneed) + it->second.size()*sizeof(Elf32_Vernaux);
    if(curpos + verneed->vn_next == verneedSecSize)
      verneed->vn_next = 0;
    verneednum++;
    int i = 0;
    for(iter = it->second.begin(); iter!= it->second.end(); iter++){
      //printf(" {%s}", iter->first.c_str());
      Elf32_Vernaux *vernaux = (Elf32_Vernaux *)(void*)(verneedSecData + curpos + verneed->vn_aux + i*sizeof(Elf32_Vernaux));
      vernaux->vna_hash = elfHash(iter->first.c_str());
      vernaux->vna_flags = 0;
      vernaux->vna_other = (Elf32_Half) iter->second;
      vernaux->vna_name = versionNames[iter->first];
      if(i == verneed->vn_cnt-1)
	vernaux->vna_next = 0;
      else
	vernaux->vna_next = sizeof(Elf32_Vernaux);
      i++;
    }
    //printf("\n");
    curpos += verneed->vn_next;
  }

  //reconstruct .gnu.version_d section
  verdefSecSize = 0;
  for(iter = verdefEntries.begin(); iter != verdefEntries.end(); iter++)
    verdefSecSize += sizeof(Elf32_Verdef) + sizeof(Elf32_Verdaux) * verdauxEntries[iter->second].size();

  verdefSecData = (char *)malloc(verdefSecSize);
  curpos = 0;
  verdefnum = 0;
  for(iter = verdefEntries.begin(); iter != verdefEntries.end(); iter++){
    //printf("  verdef entry: %s [cnt=%d] ", iter->first.c_str(), verdauxEntries[iter->second].size());
    Elf32_Verdef *verdef = (Elf32_Verdef *)(void*)(verdefSecData+curpos);
    verdef->vd_version = 1;
    // should the flag = 1 for filename versions?
    verdef->vd_flags = 0;
    verdef->vd_ndx = (Elf32_Half) iter->second;
    verdef->vd_cnt = (Elf32_Half) verdauxEntries[iter->second].size();
    verdef->vd_hash = elfHash(iter->first.c_str());
    verdef->vd_aux = sizeof(Elf32_Verdef);
    verdef->vd_next = sizeof(Elf32_Verdef) + verdauxEntries[iter->second].size()*sizeof(Elf32_Verdaux);
    if(curpos + verdef->vd_next == verdefSecSize)
      verdef->vd_next = 0;
    verdefnum++;
    for(unsigned i = 0; i< verdauxEntries[iter->second].size(); i++){
      //printf(" {%s}", verdauxEntries[iter->second][i].c_str());
      Elf32_Verdaux *verdaux = (Elf32_Verdaux *)(void*)(verdefSecData + curpos +verdef->vd_aux + i*sizeof(Elf32_Verdaux));
      verdaux->vda_name = versionNames[verdauxEntries[iter->second][i]];
      if(i == (unsigned) verdef->vd_cnt-1)
	verdaux->vda_next = 0;
      else
	verdaux->vda_next = sizeof(Elf32_Verdaux);
    }
    //printf("\n");
    curpos += verdef->vd_next;
  }
  return;
}

void emitElf::createHashSection(Elf32_Word *&hashsecData, unsigned &hashsecSize, vector<Symbol *>&dynSymbols)
{
  vector<Symbol *>::iterator iter;
  dyn_hash_map<unsigned, unsigned> lastHash; // bucket number to symbol index
  unsigned nbuckets = (unsigned)dynSymbols.size()*2/3;
  if (nbuckets % 2 == 0)
    nbuckets--;
  if (nbuckets < 1)
    nbuckets = 1;
  unsigned nchains = (unsigned)dynSymbols.size();
  hashsecSize = 2 + nbuckets + nchains;
  hashsecData = (Elf32_Word *)malloc(hashsecSize*sizeof(Elf32_Word));
  unsigned i=0, key;
  for (i=0; i<hashsecSize; i++) {
    hashsecData[i] = STN_UNDEF;
  }
  hashsecData[0] = (Elf32_Word)nbuckets;
  hashsecData[1] = (Elf32_Word)nchains;
  i = 0;
  for (iter = dynSymbols.begin(); iter != dynSymbols.end(); iter++) {
    key = elfHash((*iter)->getName().c_str()) % nbuckets;
    //printf("hash entry:  %s  =>  %u\n", (*iter)->getName().c_str(), key);
    if (lastHash.find(key) != lastHash.end()) {
      hashsecData[2+nbuckets+lastHash[key]] = i;
    }
    else {
      hashsecData[2+key] = i;
    }
    lastHash[key] = i;
    hashsecData[2+nbuckets+i] = STN_UNDEF;
    i++;
  }
}

void emitElf::createDynamicSection(void *dynData, unsigned size, Elf32_Dyn *&dynsecData, unsigned &dynsecSize, unsigned &dynSymbolNamesLength, std::vector<std::string> &dynStrs) {
  dynamicSecData.clear();
  Elf32_Dyn *dyns = (Elf32_Dyn *)dynData;
  unsigned count = size/sizeof(Elf32_Dyn);
  dynsecSize = 2*count+ DT_NEEDEDEntries.size();    //We don't know the size before hand. So allocate the maximum possible size;
  dynsecData = (Elf32_Dyn *)malloc(dynsecSize*sizeof(Elf32_Dyn));
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
    case 0x6ffffef5: // DT_GNU_HASH (not defined on all platforms)
      dynsecData[curpos].d_tag = DT_HASH;
      dynsecData[curpos].d_un.d_ptr =dyns[i].d_un.d_ptr ;
      dynamicSecData[DT_HASH].push_back(dynsecData+curpos);
      curpos++;
      break;
    case DT_HASH: 
      dynsecData[curpos].d_tag = dyns[i].d_tag;
      dynsecData[curpos].d_un.d_ptr =dyns[i].d_un.d_ptr ;
      dynamicSecData[dyns[i].d_tag].push_back(dynsecData+curpos);
      curpos++;
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
      memcpy(dynsecData+curpos, dyns+i, sizeof(Elf32_Dyn));
      dynamicSecData[dyns[i].d_tag].push_back(dynsecData+curpos);
      curpos++;
      break;
    }
  }
  dynsecData[curpos].d_tag = DT_NULL;
  dynsecData[curpos].d_un.d_val = 0;
  curpos++;
  dynsecSize = curpos+1;                            //assign size to the correct number of entries
}
#endif

void emitElf::log_elferror(void (*err_func)(const char *), const char* msg) {
  const char* err = elf_errmsg(elf_errno());
  err = err ? err: "(bad elf error)";
  string str = string(err)+string(msg);
  err_func(str.c_str());
}
