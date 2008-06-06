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

#include <ar.h>

#include "symtabAPI/h/Symtab.h"
#include "symtabAPI/h/Archive.h"
#include "symtabAPI/src/Object.h"

using namespace Dyninst;
using namespace Dyninst::SymtabAPI;

extern char errorLine[];

static SymtabError serr;
static std::string errMsg;

SymtabError Archive::getLastError()
{
    return serr;
}

std::string Archive::printError(SymtabError serr)
{
   switch (serr){
      case Obj_Parsing:
         return "Failed to parse the Archive"+errMsg;
      case No_Such_Member:
	    	return "Member not found" + errMsg;
      case Not_An_Archive:
	    	return "File is not an archive";
      default:
         return "Unknown Error";
	}	
}		
		       
Archive::Archive(std::string &filename, bool &err) :
   basePtr(NULL)
{
   mf  = MappedFile::createMappedFile(filename);
   Elf_Cmd cmd = ELF_C_READ;
   Elf_Arhdr *archdr;;
   Elf_X *arf = new Elf_X(mf->getFD(), cmd);
   if(elf_kind(arf->e_elfp()) != ELF_K_AR){
       serr  = Not_An_Archive;
       err = false;
       return;
   }
   basePtr = (void *)arf;
   Elf_X *newelf = new Elf_X(mf->getFD(), cmd, arf);
   while(1){
       if(!newelf->e_elfp())
           break;
       archdr = elf_getarhdr(newelf->e_elfp());
       string member_name = archdr->ar_name;
       if(elf_kind(newelf->e_elfp()) == ELF_K_ELF){
           memberToOffsetMapping[member_name] = elf_getbase(newelf->e_elfp()) - sizeof(struct ar_hdr);
           //parse it lazily
           membersByName[member_name] = NULL;
       }
       Elf_X *elfhandle = arf->e_next(newelf);
       newelf->end();
       newelf = elfhandle;
   }
   err = true;
}

#if 0
//For elf looks like we need a file descriptor for parsing archives
//So return error unable to parse archive
Archive::Archive(char *mem_image, size_t size, bool &err) : basePtr(NULL)
{
    err = false;
}
#endif



Archive::~Archive()
{
   hash_map <std::string, Symtab *>::iterator iter = membersByName.begin();
   for (; iter!=membersByName.end();iter++) {
      if (iter->second)
         delete (iter->second);
   }
   memberToOffsetMapping.clear();
   for (unsigned i = 0; i < allArchives.size(); i++) {
      if (allArchives[i] == this)
         allArchives.erase(allArchives.begin()+i);
   }

   if (mf) {
      MappedFile::closeMappedFile(mf);
   }
}
