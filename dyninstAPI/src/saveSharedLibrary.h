/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

#if defined(sparc_sun_solaris2_4) \
 || defined(i386_unknown_linux2_0) \
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
		Elf *oldElf;
		Elf *newElf;
		int oldfd, newfd;
		Address textSize;
		Address textAddr;
		Elf_Data* textData;

		bool openElf();
		void copyElf();
		void closeElf();	
		bool readNewLib();
	public:
		saveSharedLibrary(sharedLibrary sharedLib, char* newname=NULL){
			soObject.setpathname(sharedLib.getpathname());
			soObject.setbase_addr(sharedLib.getbase_addr());
			if(newname){
				newpathname = new char[strlen(newname) +1];
				strcpy(newpathname, newname);	
			}else{
				newpathname = NULL;
			}	
		}

		saveSharedLibrary(Address baseAddr, const char* oldname,
					char* newname){
			soObject.setbase_addr(baseAddr);
			soObject.setpathname(oldname);
			newpathname = new char[strlen(newname) +1];
			strcpy(newpathname, newname);	
		}
		saveSharedLibrary(): newpathname(NULL) {}

		~saveSharedLibrary(){
			if(newpathname){
				delete []newpathname;
			}
		}

		void setnewname(char *newname){
			if(newpathname){
				delete [] newpathname;
			}
			newpathname = new char[strlen(newname) +1];
			strcpy(newpathname, newname);	
		}

		void openBothLibraries();
		void closeOriginalLibrary();

		void getTextInfo(Address &textScnAddr, Address &textScnSize){
			textScnAddr = textAddr;
			textScnSize = textSize;
		}
		void closeNewLibrary();
		void saveMutations(char *textInsn);

		char* getTextSection();
};

#endif


