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

#ifndef Dyn_Archive_h
#define Dyn_Archive_h
 
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <vector>
#include <string>
#include <ext/hash_map>
 
 
#include "Dyn_Symtab.h"
using namespace std;
using namespace __gnu_cxx; 
 
class Dyn_Archive{
 public:
 	Dyn_Archive() {}
	static bool openArchive(string &filename, Dyn_Archive *&img);
	static bool openArchive(char *mem_image, size_t image_size, Dyn_Archive *&img);

	bool getMember(string &member_name,Dyn_Symtab *&img);
	bool getMemberByOffset(OFFSET &memberOffset, Dyn_Symtab *&img);
	bool getAllMembers(vector <Dyn_Symtab *> &members);
	bool isMemberInArchive(string &member_name);

	static SymtabError getLastError();
	static string printError(SymtabError serr);
	string file() { return filename_; }
	~Dyn_Archive();

 private:
 	Dyn_Archive(string &filename, bool &err);
	Dyn_Archive(char *mem_image, size_t image_size, bool &err);
 
 private:
 	string filename_;
	char *mem_image_;
	hash_map <string, Dyn_Symtab *> membersByName;
	hash_map <OFFSET, Dyn_Symtab *> membersByOffset;

	// A vector of all Dyn_Archive. Used to avoid duplicating
   // a Dyn_Archive that already exists.
   static vector<Dyn_Archive *> allArchives;
};

#endif
