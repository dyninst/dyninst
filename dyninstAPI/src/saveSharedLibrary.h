/*
 * Copyright (c) 1996-2011 Barton P. Miller
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#if defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */


#include  <fcntl.h>
#include  <stdio.h>
#include  <libelf.h>
#include  <stdlib.h>
#include  <string.h>
#include <unistd.h>
#include "../../common/h/Types.h"

class sharedLibrary{

	private:
		Address base_addr;
		char *pathname;
		
	public:
		sharedLibrary(){
			base_addr = 0;
			pathname = 0;
		};

		sharedLibrary(Address baseAddr, char* name) : 
		base_addr(baseAddr){
			pathname = new char[strlen(name)+1];
			strcpy(pathname, name);
		} ;
	
		~sharedLibrary(){
			if(pathname){
				delete [] pathname;
			}
		}
	
		char* getpathname(){
			return pathname;
		}

		Address getbase_addr(){
			return base_addr;
		}

		void setpathname(const char *name){
			if(pathname){
				free(pathname);
			}
			pathname = new char[strlen(name)+1];
			strcpy(pathname, name);

		}

		void setbase_addr(Address baseAddr){
			base_addr = baseAddr;
		}	
};


class saveSharedLibrary {


	private:

		sharedLibrary soObject;
		char* newpathname;
		//Elf *oldElf;
		Elf *newElf;
		int /*oldfd, */ newfd;
		Address textSize;
		Address textAddr;
		Elf_Data* textData;

		bool openElf();
		void copyElf();
		void closeElf();	
		bool readNewLib();

		char *newShstrtabData_d_buf;
	public:
		saveSharedLibrary(sharedLibrary sharedLib, char* newname=NULL);

		saveSharedLibrary(Address baseAddr, const char* oldname,
					char* newname);
		saveSharedLibrary(); 

		~saveSharedLibrary();

		void setnewname(char *newname);

		void openBothLibraries();
		void closeOriginalLibrary();

		void getTextInfo(Address &textScnAddr, Address &textScnSize);
		void closeNewLibrary();
		void saveMutations(char *textInsn);

		//char* getTextSection();
};

#endif


