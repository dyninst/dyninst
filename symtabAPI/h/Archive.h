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

#ifndef Archive_h
#define Archive_h
 
#include "util.h" 
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <vector>
#include <string>
#include <ext/hash_map>
 
using namespace std;
using namespace __gnu_cxx; 

namespace Dyninst{
namespace SymtabAPI{

class Symtab;

class Archive{
 public:
 	Archive() {}
	static bool openArchive(Archive *&img, std::string filename);
	static bool openArchive(Archive *&img, char *mem_image, size_t image_size);

	bool getMember(Symtab *&img, std::string memberName);
	bool getMemberByOffset(Symtab *&img, Offset memberOffset);
	bool getAllMembers(std::vector <Symtab *> &members);
	bool isMemberInArchive(std::string member_name);

	static SymtabError getLastError();
	static std::string printError(SymtabError serr);
	std::string file() { return filename_; }
	~Archive();

 private:
 	Archive(std::string &filename, bool &err);
	Archive(char *mem_image, size_t image_size, bool &err);
 
 private:
 	std::string filename_;
	char *mem_image_;
	hash_map <std::string, Symtab *> membersByName;
	hash_map <Offset, Symtab *> membersByOffset;

	// A vector of all Archive. Used to avoid duplicating
   // a Archive that already exists.
   static std::vector<Archive *> allArchives;
};

}//namespace SymtabAPI
}//namespace Dyninst
#endif
