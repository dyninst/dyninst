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

#include "symtabAPI/h/Symtab.h"
#include "symtabAPI/h/Archive.h"
#include "symtabAPI/src/Object.h"

#include <iostream>

using namespace std;
using namespace Dyninst;
using namespace Dyninst::SymtabAPI;

/*
 * A generic interface for handling archives (aka .a files)
 *
 * Specifics of the format for the .a files are handled per platform.
 * See Archive-<platform> files for more info. 
 */

std::vector<Archive *> Archive::allArchives;
std::string Archive::errMsg;
SymtabError Archive::serr;

// Error messages used by printError
static const std::string PARSE_FAILURE = "Failed to parse the archive: ";
static const std::string NO_MEMBER = "Member not found: ";
static const std::string NOT_ARCHIVE = "File is not an archive";
static const std::string DUPLICATE_SYM = "Duplicate symbol found: ";
static const std::string UNKNOWN_ERR = "Unknown Error";

// Specific error cases
static const std::string MEMBER_DNE = "member does not exist";

SymtabError Archive::getLastError()
{
    return serr;
}

std::string Archive::printError(SymtabError err)
{
   switch (err) {
      case Obj_Parsing:
         return PARSE_FAILURE + errMsg;
      case No_Such_Member:
         return NO_MEMBER + errMsg;
      case Not_A_File:
         return errMsg;
      case Not_An_Archive:
         return NOT_ARCHIVE;
      case Duplicate_Symbol:
         return DUPLICATE_SYM + errMsg;
      default:
         return UNKNOWN_ERR;
   }
}

std::string Archive::name() {
    return mf->filename();
}

bool Archive::openArchive(Archive * &img, std::string filename)
{
    bool err = false;

    // Has the archive already been opened?
    std::vector<Archive *>::iterator ar_it;
    for (ar_it = allArchives.begin(); ar_it != allArchives.end(); ++ar_it) {
        assert( *ar_it != NULL );

        if( (*ar_it)->mf->filename() == filename ) {
            img = *ar_it;
            return true;
        }
    }

    img = new Archive(filename, err);
    if (err) {			
	allArchives.push_back(img);
    } else {
	if (img) {
            delete img;
            img = NULL;
        }
    }

    return err;
}

bool Archive::openArchive(Archive * &img, char *mem_image, size_t size)
{
    bool err;
    // Has the archive already been opened?
    std::vector<Archive *>::iterator ar_it;
    for (ar_it = allArchives.begin(); ar_it != allArchives.end(); ++ar_it) {
        assert( *ar_it != NULL );

        if( (*ar_it)->mf->base_addr() == (void *)mem_image ) {
            img = *ar_it;
            return true;
        }
    }

    img = new Archive(mem_image, size, err);
    if (err) {
        allArchives.push_back(img);
    } else {
        if (img) {
            delete img;
            img = NULL;
        }
    }

    return err;
}

bool Archive::getMember(Symtab *&img, string& member_name) 
{
    dyn_hash_map<string, ArchiveMember *>::iterator mem_it;
    mem_it = membersByName.find(member_name);
    if ( mem_it == membersByName.end() ) {
        serr = No_Such_Member;
        errMsg = MEMBER_DNE;
        return false;
    }

    img = mem_it->second->getSymtab();
    if( img == NULL ) {
        if( !parseMember(img, mem_it->second) ) {
            return false;
        }
    }

    return true;
}

bool Archive::getMemberByOffset(Symtab *&img, Offset memberOffset) 
{
    dyn_hash_map<Offset, ArchiveMember *>::iterator off_it;
    off_it = membersByOffset.find(memberOffset);
    if( off_it == membersByOffset.end() ) {
        serr = No_Such_Member;
        errMsg = MEMBER_DNE;
        return false;
    }

    img = off_it->second->getSymtab();
    if( img == NULL ) {
        if( !parseMember(img, off_it->second) ) {
            return false;
        }
    }

    return true;
}

bool Archive::getMemberByGlobalSymbol(Symtab *&img, string& symbol_name) 
{
    if( !symbolTableParsed ) {
       if( !parseSymbolTable() ) {
            return false;
       }
    }

    std::pair<std::multimap<string, ArchiveMember *>::iterator,
              std::multimap<string, ArchiveMember *>::iterator> range_it;
    range_it = membersBySymbol.equal_range(symbol_name);

    // Symbol not found in symbol table
    if( range_it.first == range_it.second ) {
        serr = No_Such_Member;
        errMsg = MEMBER_DNE;
        return false;
    }
    ArchiveMember *foundMember = range_it.first->second;

    // Duplicate symbol found in symbol table
    ++(range_it.first);
    if( range_it.first != range_it.second ) {
        serr = Duplicate_Symbol;
        errMsg = symbol_name;
        return false;
    }

    img = foundMember->getSymtab();
    if( img == NULL ) {
        if( !parseMember(img, foundMember) ) {
            return false;
        }
    }

    return true;
}

bool Archive::getMembersBySymbol(std::string name,
                                 std::vector<Symtab *> &matches) {
   if (!symbolTableParsed && !parseSymbolTable())
      return false;
   
   std::pair<std::multimap<string, ArchiveMember *>::iterator,
      std::multimap<string, ArchiveMember *>::iterator> range_it;
   
   range_it = membersBySymbol.equal_range(name);
   auto begin = range_it.first;
   auto end = range_it.second;

   for (; begin != end; ++begin) {
      ArchiveMember *member = begin->second;
      Symtab *img = member->getSymtab();
      if (!img && !parseMember(img, member)) return false;
      matches.push_back(img);
   }
         
   return true;
}

bool Archive::getAllMembers(vector<Symtab *> &members) 
{
    dyn_hash_map<string, ArchiveMember *>::iterator mem_it;
    for(mem_it = membersByName.begin(); mem_it != membersByName.end(); ++mem_it) {
        Symtab *img = mem_it->second->getSymtab();
        if( img == NULL) {
            if( !parseMember(img, mem_it->second) ) {
                return false;
            }
        }
        members.push_back(mem_it->second->getSymtab());
    }
    return true;
}

bool Archive::isMemberInArchive(std::string& member_name) 
{
    if (membersByName.count(member_name)) return true;
    return false;
}

/**
 * This method differs from getMemberByGlobalSymbol in that it searches the
 * underlying Symtab objects in the archive for the symbol while
 * getMemberByGlobalSymbol searches the Archive's symbol table.
 *
 * The reasoning behind this is that searching the Archive's symbol table will
 * be faster than using the Symtab interface because the underlying Symtab
 * object is only created when the symbol is found in the Archive's symbol table.
 * Contrary to this, findMemberWithDefinition requires creating Symtab objects
 * for every member in an Archive and then searching each of these Symtab objects
 * for the symbol.
 */
bool Archive::findMemberWithDefinition(Symtab * &obj, std::string& name)
{
    std::vector<Symtab *> members;
    if( !getAllMembers(members) ) {
        return false;
    }

    std::vector<Symtab *>::iterator obj_it;
    for (obj_it = members.begin(); obj_it != members.end(); ++obj_it) {
        std::vector<Symbol *> syms;
	if ((*obj_it)->findSymbol(syms, name, Symbol::ST_UNKNOWN)) {
            obj = *obj_it;
            return true;
	}
    }

    serr = No_Such_Member;
    errMsg = MEMBER_DNE;
    return false;
}

Archive::~Archive()
{
    dyn_hash_map<string, ArchiveMember *>::iterator it;
    for (it = membersByName.begin(); it != membersByName.end(); ++it) {
        if (it->second) delete it->second;
    }

    for (unsigned i = 0; i < allArchives.size(); i++) {
        if (allArchives[i] == this)
            allArchives.erase(allArchives.begin()+i);
    }

    if (mf) {
      MappedFile::closeMappedFile(mf);
    }
    // who allocated basePtr? they should delete it...
}
