#if defined(BPATCH_LIBRARY)
#if defined(sparc_sun_solaris2_4)


#include  <fcntl.h>
#include  <stdio.h>
#include  <libelf.h>
#include  <stdlib.h>
#include  <string.h>
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
				free(pathname);
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

		void openElf();
		void copyElf();
		void closeElf();	
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
				free(newpathname);
			}
		}

		void setnewname(char *newname){
			if(newpathname){
				free(newpathname);
			}
			newpathname = new char[strlen(newname) +1];
			strcpy(newpathname, newname);	
		}

		void writeLibrary(char* newname=NULL);

		void getTextInfo(Address &textScnAddr, Address &textScnSize){
			textScnAddr = textAddr;
			textScnSize = textSize;
		}
		void closeLibrary();
		void saveMutations(char *textInsn);
};

#endif
#endif

