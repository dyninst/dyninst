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


#include "symtabAPI/h/Symtab.h"
#include "symtabAPI/h/Archive.h"
#include "symtabAPI/src/Object.h"

using namespace Dyninst;
using namespace Dyninst::SymtabAPI;

extern char errorLine[];

static SymtabError serr;
static std::string errMsg;

std::vector<Archive *> Archive::allArchives;

bool Archive::openArchive(Archive *&img, std::string filename)
{
 	bool err;
	for (unsigned i=0;i<allArchives.size();i++)
	{
      if (allArchives[i]->mf->pathname() == filename)
      {
	    	img = allArchives[i];
         return true;
      }
	}

	img = new Archive(filename ,err);
	if (err)	// No errors
		allArchives.push_back(img);	
	else {
      if (img)
         delete img;
		img = NULL;
   }
	return err;
}

#if 0 
bool Archive::openArchive(Archive *&img, char *mem_image, size_t size)
{
 	bool err;
	img = new Archive(mem_image, size, err);
	if(err == false)
		img = NULL;
	return err;
}
#endif

bool Archive::getMember(Symtab *&img, std::string member_name)
{
   if (membersByName.find(member_name) == membersByName.end())
   {
      serr = No_Such_Member;
      errMsg = "Member Does not exist";
      return false;
   }	
   img = membersByName[member_name];
   if (img == NULL) {
      bool err;
      if(mf->getFD() == -1)
          img = new Symtab((char *)mf->base_addr(),mf->size(), member_name, memberToOffsetMapping[member_name], err, basePtr);
      else
          img = new Symtab(mf->pathname(),member_name, memberToOffsetMapping[member_name], err, basePtr);
      membersByName[member_name] = img;
   }
   return true;
}

bool Archive::getAllMembers(std::vector <Symtab *> &members)
{
   hash_map <std::string, Symtab *>::iterator iter = membersByName.begin();
   for (; iter!=membersByName.end();iter++) {
      if (iter->second == NULL) {
         bool err;
         string member_name = iter->first;
         if(mf->getFD() == -1)
             iter->second = new Symtab((char *)mf->base_addr(), mf->size(), member_name, memberToOffsetMapping[iter->first], err, basePtr);
         else
             iter->second = new Symtab(mf->pathname(), member_name, memberToOffsetMapping[iter->first], err, basePtr);
      }
      members.push_back(iter->second);
   }
   return true;	
}

bool Archive::isMemberInArchive(std::string member_name)
{
   if(membersByName.find(member_name) != membersByName.end())
      return true;
   return false;
}

bool Archive::findMemberWithDefinition(Symtab *&obj, std::string name){
    vector<Symtab *>members;
    getAllMembers(members);

    for(unsigned i=0; i<members.size();i++){
        vector<Symbol *>syms;
        if(members[i]->findSymbolByType(syms, name, Symbol::ST_UNKNOWN, true)){
            obj = members[i];
            return true;
        }
    }
    return false;
}
