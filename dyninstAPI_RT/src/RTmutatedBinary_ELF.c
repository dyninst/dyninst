/* $Id: RTmutatedBinary_ELF.c,v 1.5 2003/07/25 15:52:32 chadd Exp $ */

/* this file contains the code to restore the necessary
   data for a mutated binary 
 */

#include "dyninstAPI_RT/h/dyninstAPI_RT.h"
#include <unistd.h>
#include  <fcntl.h>
#include <string.h>

#if defined(sparc_sun_solaris2_4)
#include <libelf.h>
#elif defined(i386_unknown_linux2_0)
#include <libelf/libelf.h>
#define __USE_GNU
#endif

#include <sys/mman.h>
#include <dlfcn.h>
#include <link.h> /* ccw 23 jan 2002 */
#if defined(sparc_sun_solaris2_4) 
#include <sys/link.h>
#include <signal.h>
#endif
#include <limits.h>

#if defined(sparc_sun_solaris2_4)
extern void* _DYNAMIC;
#elif defined(i386_unknown_linux2_0)
extern ElfW(Dyn) _DYNAMIC[];

#endif
typedef struct {
      Elf32_Sword d_tag;
      union {
          Elf32_Sword d_val;
          Elf32_Addr d_ptr;
      } d_un;
  } __Elf32_Dyn;



unsigned int checkAddr;
/*extern int isMutatedExec;
char *buffer;
*/
struct link_map* map;
unsigned int dl_debug_state_addr;

#if defined(sparc_sun_solaris2_4)
struct r_debug _r_debug; /* ccw 2 apr 2002 */
extern unsigned int _dyninst_call_to_dlopen;
extern unsigned int __dyninst_jump_template__;
extern unsigned int __dyninst_jump_template__done__;
#endif

char *sharedLibraryInfo = NULL;
unsigned int originalInsnBkpt;
unsigned int addressBkpt;

/* 	this is not misnamed.  In the future, this function will contain
	code to patch the instrumentation of a shared library that has 
	been loaded into a different place during a mutated binary run.

	Now, it just exit()s, as you can see
*/

void fixInstrumentation(char* soName, unsigned int currAddr, unsigned int oldAddr){
	printf(" %s loaded at wrong address: 0x%x (expected at 0x%x) \n", soName, currAddr, oldAddr);
	printf(" This is an unrecoverable error, the instrumentation will not");
	printf("\n run correctly if shared libraries are loaded at a different address\n");
	printf("\n Exiting.....\n");
	fflush(stdout);
	exit(9);
}

/* 	this function checks the shared library (soName) to see if it
	is currently loaded (loadAddr) at the same place it was before (address).
	If the shared library is not found in the list (sharedLibraryInfo) that
	mean the shared library was *NOT* instrumented and can be loaded
	anywhere
*/
unsigned int checkSOLoadAddr(char *soName, unsigned int loadAddr){
	unsigned int result=0, found = 0;
	unsigned int address;
	char *ptr = sharedLibraryInfo;
	while(ptr &&  *ptr && !found ){
		/*fprintf(stderr," CHECKING FOR %s in %s\n", ptr, soName);*/

        	if(strstr(soName, ptr) || strstr(ptr,soName)){
                	found = 1;
			ptr += (strlen(ptr) +1);
			memcpy(&address, ptr, sizeof(unsigned int)); 
			/* previous line is done b/c of alignment issues on sparc*/
			if(loadAddr == address) {
				result = 0;
			}else{
				result = address;
			}	
		}

		ptr += (strlen(ptr) +1);
		ptr += sizeof(unsigned int);
		ptr += sizeof(unsigned int); /* for flag */


	}
	if(!found){
		result = 0;
		/*fprintf(stderr," NOT FOUND %s\n",soName);*/
	}

	/*fprintf(stderr," checkSOLoadAddr: %s %x %x\n", soName, loadAddr, result);*/
	return result;
}
#if defined(sparc_sun_solaris2_4)
unsigned int register_o7;

unsigned int loadAddr;
/*	this function is not a signal handler. it was originally but now is 
	not, it is called below in dyninst_jump_template
*/ 
void pseudoSigHandler(int sig){

	map = _r_debug.r_map; /* ccw 22 jul 2003*/
	if(_r_debug.r_state == 0){
		do{
			if(map->l_next){
				map = map->l_next;
			}
			loadAddr = checkSOLoadAddr(map->l_name, map->l_addr);
			if(loadAddr){
				fixInstrumentation(map->l_name, map->l_addr, loadAddr);
			}

		}while(map->l_next);

	}
}

unsigned int loadAddr;
void dyninst_jump_template(){
/* THE PLAN:

	The Solaris loader/ELF file works as follows:
	
	A call to dlopen jumps to the Procedure Linking Table 
	slot holding the dlopen information.  This slot consists of
	three instructions:
	
	sethi r1, 0xb4
	ba (to another PLT slot)
	nop

	The second PLT slot contains three instructions:
	save
	call (address of dlopen)
	nop

	dlopen returns directly to where it was called from, not to
	either of the PLT slots.  The address from which it was called
	is located in %o7 when the call to dlopen in the second PLT
	slot is made. dlopen returns to %o7 +4.
	

	What our code does:

	The goals is to intercept this call to dlopen by overwritting
	the first PLT slot to jump to __dyninst_jump_template__ then we
	can jump to code that will check the addresses of the loaded
	shared libraries.

	first we must preserver %o7 so we know where to go back to.
	This is done with the first two instructions in __dyninst_jump_template__
	These are written as nops BUT are overwritten in the SharedLibraries 
	branch in checkElfFile.  %o7 is saved in register_o7 declared above.
	This address is not available until run time so we generate these
	instructions on the fly.

	Next, we CALL the second PLT slot as normal.  We use the delay
	slot to run the sethi instruction from the first PLT slot. These
	instructions are generated at runtime. 

	dlopen will eventually be called and will return to the nop after
	the sethi.  Now we need to call our function to check the
	shared library address.  This is pseudoSigHandler.  We must
	preserve the data returned by dlopen so we do a save to
	push the register onto the stack before we call our function.
	We call our function, and then do a restore to retreive the 
	saved information.

	At __dyninst_jump_template__done__ we want to restore the
	value in register_o7 to %o7 so when we do a retl we will
	jump back to where the mutated binary originally called 
	dlopen.
	The sethi and ld instructions are generated on the fly just
	as the first sethi and st pair that saved the value of %o7.
	The retl used %o7 to jump back to the original place 
	the mutatee called dlopen. We are done. Whew.

	Note that I know the address of the first PLT slot because
	the mutator found it and saved it in the mutated binary.
	The address of the second PLT slot is determined by looking
	at the instructions in the first PLT slot.
*/
	

	asm("nop");
	asm("__dyninst_jump_template__:");

	asm("nop"); 	/*sethi hi(register_o7), %g1 GENERATED BELOW*/
	asm("nop");	/*st %o7 GENERATED BELOW*/
	asm("nop");	/*call plt GENERATED BELOW*/
	asm("nop");     /*sethi r1, b4 GENERATED BELOW*/

	asm("nop");
	asm("save %sp, -104, %sp");
	asm("nop");
	pseudoSigHandler(0);
	asm("nop");
	asm("restore");
	asm("nop");
	asm("nop");
	asm("__dyninst_jump_template__done__:");
	asm("nop"); 	/*sethi hi(register_o7), %g1 GENERATED BELOW*/
	asm("nop"); 	/* ld [register_o7], %o7 GENERATED BELOW*/
	asm("retl");

	asm("nop"); 	/* this will be filled in below */
	asm("nop");

}

#endif

#if defined(i386_unknown_linux2_0)
unsigned int loadAddr;
void dyninst_dl_debug_state(){
	asm("nop");
	if(_r_debug.r_state == 1){
	do {
			if(map->l_next){
				map = map->l_next;
			}
			loadAddr = checkSOLoadAddr(map->l_name, map->l_addr);
			if(loadAddr){
				fixInstrumentation(map->l_name, map->l_addr, loadAddr);
			}
		}while(map->l_next);

	}

	/* the following call is used to call
	 * _dl_debug_state to ensure correctness (if
	 * someone relies on it being called it is
	 * execuated after this function)
	 * The value stored in dl_debug_state_addr is
	 * the address of the function _dl_debug_state
	 * and is set in checkElfFile
	 */
	asm("nop");
	asm("nop");
	asm("nop");
	asm("call *dl_debug_state_addr");
	asm("nop");

}

void hack_ld_linux_plt(unsigned int pltEntryAddr){ 
/* this is ugly.
 * save the world needs to check each shared library
 * that is loaded to ensure that it is loaded at the
 * same base address it was loaded at when the mutator/mutatee
 * pair ran.  
 * So, we know dlopen calls _dl_debug_state per the r_debug
 * interface to let the process know a shared library has changed
 * state.
 * with this function we change the Procedure Linkage Table (.plt)
 * for ld-linux.so so that the entry that used to point to
 * _dl_debug_state points to dyninst_dl_debug_state.
 *
 * dyninst_dl_debug_state then calls _dl_debug_state before
 * exiting 
 *
 * dont try this at home
 */
	unsigned int mprotectAddr = pltEntryAddr - (pltEntryAddr % getpagesize());	
	unsigned int newTarget = (unsigned int) &dyninst_dl_debug_state ;
	
	mprotect( (void*) mprotectAddr, pltEntryAddr - mprotectAddr + 4, 
				PROT_READ|PROT_WRITE|PROT_EXEC);

	memcpy( (void*) &dl_debug_state_addr, (void*) pltEntryAddr, 4); 

	memcpy( (void*) pltEntryAddr, &newTarget, 4);
}
#endif


int checkSO(char* soName){
	Elf32_Shdr *shdr;
        Elf32_Ehdr *   ehdr;
        Elf *          elf;
        int       fd;
        Elf_Data *strData;
        Elf_Scn *scn;
	int result = 0;

 	if((fd = (int) open(soName, O_RDONLY)) == -1){
                RTprintf("cannot open : %s\n",soName);
    		fflush(stdout); 
		return;
        }
        if((elf = elf_begin(fd, ELF_C_READ, NULL)) ==NULL){
                RTprintf("%s %s \n",soName, elf_errmsg(elf_errno()));
                RTprintf("cannot elf_begin\n");
		fflush(stdout);
                close(fd);
                return;
        }

        ehdr = elf32_getehdr(elf);
        scn = elf_getscn(elf, ehdr->e_shstrndx);
        strData = elf_getdata(scn,NULL);
   	for( scn = NULL; !result && (scn = elf_nextscn(elf, scn)); ){
                shdr = elf32_getshdr(scn);
		if(!strcmp((char *)strData->d_buf + shdr->sh_name, ".dyninst_mutated")) {
			result = 1;
		}
	}
        elf_end(elf);
        close(fd);

	return result;
}

int checkMutatedFile(){


	Elf32_Shdr *shdr;
        Elf32_Ehdr *   ehdr;
        Elf *          elf;
        int       cnt,fd;
        Elf_Data *elfData,*strData;
        Elf_Scn *scn;
        char execStr[256];
	int retVal = 0, result;
	unsigned int mmapAddr;
	int pageSize;
	Address dataAddress;
	int dataSize;
       	char* tmpPtr;
        unsigned int updateAddress, updateSize, updateOffset;
        unsigned int *dataPtr;
 	unsigned int numberUpdates,i ;
	char* oldPageData;
	Dl_info dlip;
	int soError = 0; 
	int sawFirstHeapTrampSection = 0;
	elf_version(EV_CURRENT);

#if defined(sparc_sun_solaris2_4)
        sprintf(execStr,"/proc/%d/object/a.out",getpid());
#elif defined(i386_unknown_linux2_0)
	sprintf(execStr,"/proc/%d/exe",getpid());
#endif

        if((fd = (int) open(execStr, O_RDONLY)) == -1){
                printf("cannot open : %s\n",execStr);
    		fflush(stdout); 
		return;
        }
        if((elf = elf_begin(fd, ELF_C_READ, NULL)) ==NULL){
                printf("%s %s \n",execStr, elf_errmsg(elf_errno()));
                printf("cannot elf_begin\n");
		fflush(stdout);
                close(fd);
                return;
        }

        ehdr = elf32_getehdr(elf);
        scn = elf_getscn(elf, ehdr->e_shstrndx);
        strData = elf_getdata(scn,NULL);
	pageSize =  getpagesize();
   	for(cnt = 0, scn = NULL; !soError &&  (scn = elf_nextscn(elf, scn));cnt++){
                shdr = elf32_getshdr(scn);
		if(!strncmp((char *)strData->d_buf + shdr->sh_name, "dyninstAPI_data", 15)) {
			elfData = elf_getdata(scn, NULL);
			tmpPtr = elfData->d_buf;
			dataAddress = -1;
			while( dataAddress != 0 ) { 
				/*tmpPtr may not be aligned on the correct boundry
				so use memcpy to set dataSize
				dataSize = *(int*) tmpPtr;*/
				memcpy((char*) & dataSize, tmpPtr, sizeof(int));

				tmpPtr+=sizeof(int);
				memcpy( (char*) & dataAddress, tmpPtr, sizeof(Address));

				tmpPtr += sizeof(Address);
				if(dataAddress){
					memcpy((char*) dataAddress, tmpPtr, dataSize);

					tmpPtr += dataSize;
				}
			}

		}else if(!strncmp((char *)strData->d_buf + shdr->sh_name, "dyninstAPI_",11)){ 
			char *tmpStr = strchr((char *)strData->d_buf + shdr->sh_name, (int)'_'); ;

			tmpStr ++;


#if defined(sparc_sun_solaris2_4)
			{
				/* this moved up incase there is no dyninstAPI_### section, map and
				_r_debug are still set correctly. */
				/* solaris does not make _r_debug available by
				default, we have to find it in the _DYNAMIC table */

				__Elf32_Dyn *_dyn = (__Elf32_Dyn*)& _DYNAMIC;	
	
				while(_dyn && _dyn->d_tag != 0 && _dyn->d_tag != 21){
					_dyn ++;
				}

				if(_dyn && _dyn->d_tag != 0){
					_r_debug = *(struct r_debug*) _dyn->d_un.d_ptr;
				}
				map = _r_debug.r_map;
			}

#endif

			if( *tmpStr>=0x30 && *tmpStr <= 0x39 ) {
				/* we dont want to do this unless this is a dyninstAPI_### section
					specifically, dont do this for dyninstAPI_SharedLibraries*/
				retVal = 1; /* this is a restored run */

				if( *tmpStr>=0x30 && *tmpStr <= 0x39 ) {
					/* this is a heap tramp section */
					if( sawFirstHeapTrampSection ){
						result = (int) mmap((void*) shdr->sh_addr, shdr->sh_size, 
	        	                           PROT_READ|PROT_WRITE|PROT_EXEC,
                        		           MAP_FIXED|MAP_PRIVATE,fd,shdr->sh_offset);
					}else{
						elfData = elf_getdata(scn, NULL);
						memcpy((void*)shdr->sh_addr, elfData->d_buf, shdr->sh_size);
						sawFirstHeapTrampSection = 1;
					}
				}
			}
		}
		if(!strcmp((char *)strData->d_buf + shdr->sh_name, "dyninstAPI_mutatedSO")){
			/* make sure the mutated SOs are loaded, not the original ones */
			char *soNames;
			int mutatedFlag = 0;
			int totallen=0;
			__Elf32_Dyn *_dyn = (__Elf32_Dyn*)& _DYNAMIC;	
#if defined(sparc_sun_solaris2_4)
			Link_map *lmap=0;
#elif defined(i386_unknown_linux2_0)
			struct link_map *lmap=0;
#endif
			char *loadedname, *dyninstname;

			elfData = elf_getdata(scn, NULL);

			sharedLibraryInfo = (char*) malloc(elfData->d_size);
			memcpy(sharedLibraryInfo, elfData->d_buf, elfData->d_size);
			lmap = _r_debug.r_map;
		
			for(soNames = (char*) elfData->d_buf ; totallen<elfData->d_size; 
				soNames = &((char*) elfData->d_buf)[strlen(soNames)+1+sizeof(unsigned int) +sizeof(unsigned int)]){
				/* added a +sizeof(unsigned int) above for flag */
				totallen += strlen(soNames) + 1 + sizeof(unsigned int) +sizeof(unsigned int); /*for flag*/
				memcpy(&mutatedFlag, &((char*) elfData->d_buf)[totallen-sizeof(unsigned int)], sizeof(unsigned int));
				lmap = _r_debug.r_map;
				while(lmap){
					loadedname = strrchr(lmap->l_name,'/');
					dyninstname =  strrchr((const char *)soNames,(int)'/');
					if(loadedname == 0){
						loadedname = lmap->l_name;
					}
					if(dyninstname == 0){
						dyninstname = soNames;
					}	
					if(mutatedFlag && !strcmp(loadedname, dyninstname)) {
						if(!checkSO(lmap->l_name)){
			printf("ERROR: %s was mutated during saveworld and",lmap->l_name);
			printf(" the currently loaded %s has not been mutated\n", lmap->l_name);
			printf(" check your LD path to be sure the mutated %s is visible\n", soNames);
							soError = 1;
		
						}

		
					}
					lmap = lmap->l_next;
				}
			}
		}
		if(!strcmp((char *)strData->d_buf + shdr->sh_name, "dyninstAPI_SharedLibraries")){
			unsigned int diffAddr;
			unsigned int ld_linuxBaseAddr, baseAddr, size;
#if defined(sparc_sun_solaris2_4)
			unsigned int *overWriteInsn;
			unsigned int *pltEntry, *PLTEntry, *dyninst_jump_templatePtr, pltInsn;
			unsigned int BA_MASK = 0x003fffff;
			unsigned int offset, callInsn;
			struct sigaction  mysigact, oldsigact;
#endif
			char *ptr;
			int foundLib = 0, result;
			int done = 0;
			elfData = elf_getdata(scn, NULL);

			ptr = elfData->d_buf;
		
			map = _r_debug.r_map;

			while(map && !done){
				if( * map->l_name ){
					diffAddr = checkSOLoadAddr(map->l_name, map->l_addr);
					if(diffAddr){
						fixInstrumentation(map->l_name, map->l_addr, diffAddr);
					}
#if defined(i386_unknown_linux2_0)
					if(strstr(map->l_name, "ld-linux.so")){
						ld_linuxBaseAddr =map->l_addr;
					}	
#endif
				}
				/* check every loaded SO but leave map such that map->l_next == NULL.
					The next time a SO is loaded it will be placed at 
					map->l_next, so keep a tail pointer such that we 
					dont need to loop through the entire list again
				*/
				if(map->l_next){
					map = map->l_next;
				}else{
					done = 1;
				}
			}
			if( shdr->sh_addr != 0){
				/* if the addr is zero, then there is 
					no PLT entry for dlopen.  if there is
					no entry for dlopen the mutatee must not
					call it.  -- what about calling it from
					a shared lib that is statically loaded?
				*/

			/* WHY IS THERE A POUND DEFINE HERE? 
				
				well, we need to intercept the dlopen calls from the mutated binary
				because our trampolines expect the shared libraries to be in
				a particular location and if they are not where they are expected
				our trampolines can jump off into nothingness, or even worse, some
				random bit of executable code.  

				So we must intercept the dlopen call and then check to be sure
				the shared libraries are loaded in the same place as before.  If
				they are not we exit with a message to the user saying this is
				a fatal error.
		
				Note, only shared libraries that have been instrumented are checked
				here.  
			*/

#if defined(sparc_sun_solaris2_4)
			/* 
				For a description of what is going on here read
				the comment in dyninst_jump_template above.

				This code generated all the instructions refered
				to in that comment as "GENERATED BELOW".

			*/
			pltEntry = (unsigned int*) shdr->sh_addr;
			pltInsn = *pltEntry; /* save insn for later */
			pltEntry += 1;
			offset = (*pltEntry) & BA_MASK;
			if(offset & 0x00200000){
				/* negative so sign extend */
				offset = 0xffc00000 | offset;
			}
			PLTEntry = pltEntry;
			
			PLTEntry += (offset*4)/sizeof(PLTEntry); /* move PLTEntry offset*4 bytes!*/
			dyninst_jump_templatePtr = (unsigned int*) & __dyninst_jump_template__;

			baseAddr = ((unsigned int) dyninst_jump_templatePtr)  -
				( ((unsigned int) dyninst_jump_templatePtr)% getpagesize());
			size =  (unsigned int) dyninst_jump_templatePtr  - baseAddr + 80;
			result = mprotect((void*)baseAddr , size, 
                           PROT_READ|PROT_WRITE|PROT_EXEC);

			/* build sethi hi(register_o7), %g1 */
			*dyninst_jump_templatePtr = 0x03000000;
			*dyninst_jump_templatePtr |= ( (((unsigned int ) & register_o7)& 0xfffffc00) >> 10); /*0xffffe000 */

			dyninst_jump_templatePtr ++;

			/* build st %o7, &register_o7 */
			*dyninst_jump_templatePtr = 0xde206000;
			*dyninst_jump_templatePtr |=  ( ((unsigned int ) & register_o7) & 0x000003ff ); /*0x00001fff*/

			dyninst_jump_templatePtr ++;

			/* build call PLTEntry */
			*dyninst_jump_templatePtr = 0x40000000;
			*dyninst_jump_templatePtr |= ( ((unsigned int) (PLTEntry)-  ((unsigned int) dyninst_jump_templatePtr)) >>2);
			
			dyninst_jump_templatePtr ++;

			/* copy from plt */
			*dyninst_jump_templatePtr = pltInsn;
			dyninst_jump_templatePtr ++;


			/* advance past call to pseudoSigHandler */
			dyninst_jump_templatePtr = (unsigned int*) &__dyninst_jump_template__done__ ;
	
			/* build sethi hi(register_o7), %g1 */
			*dyninst_jump_templatePtr = 0x03000000;
			*dyninst_jump_templatePtr |= ( (((unsigned int ) & register_o7)& 0xfffffc00) >> 10); /*0xffffe000*/

			dyninst_jump_templatePtr ++;

			/* build ld %o7, register_o7 */
			*dyninst_jump_templatePtr = 0xde006000;
			*dyninst_jump_templatePtr |=  ( ((unsigned int ) & register_o7) & 0x000003ff );  /*0x00001fff*/


			/* THIS ENABLES THE JUMP */
			/* edit plt to jump to __dyninst_jump_template__ */
			baseAddr = ((unsigned int) pltEntry)  -
				( ((unsigned int) pltEntry)% getpagesize());
			size =  (unsigned int) pltEntry  - baseAddr + 8;
			result = mprotect((void*)baseAddr , size,
                           PROT_READ|PROT_WRITE|PROT_EXEC);

			/* build sethi hi(&__dyninst_jump_template__), %g1 */
			pltEntry --;

			*pltEntry = 0x03000000;
			*pltEntry |= ( (((unsigned int ) &__dyninst_jump_template__) )>> 10);
			pltEntry ++;
	
			/* build jmpl %g1, %r0 */	
			*pltEntry = 0x81c06000;
			*pltEntry |=  ( ((unsigned int ) &__dyninst_jump_template__ ) & 0x00003ff );

#elif defined(i386_unknown_linux2_0)
			/* install jump to catch call to _dl_debug_state */
			/* see comment int hack_ld_linux_plt for explainations */
			hack_ld_linux_plt(ld_linuxBaseAddr + shdr->sh_addr); 
#endif
		}/* shdr->sh_addr != 0 */ 
		}
		if(!strncmp((char *)strData->d_buf + shdr->sh_name, "dyninstAPIhighmem_",18)){
			/*the layout of dyninstAPIhighmem_ is:
			pageData
			address of update
			size of update
			...	
			address of update
			size of update	
			number of updates
	
			we must ONLY overwrite the updates, the other
			areas of the page may be important (and different than
			the saved data in the file.  we first copy out the
			page, the apply the updates to it, and then
			write it back.
			*/

			int oldPageDataSize;

			retVal = 1; /* just to be sure */
			elfData = elf_getdata(scn, NULL);
			numberUpdates = (unsigned int) ( ((unsigned int*) elfData->d_buf)[
				(elfData->d_size - sizeof(unsigned int))/ sizeof(unsigned int) ]);

			/*fprintf(stderr," numberUpdates: %d :: (%d - 4) / 4  %x\n", numberUpdates, elfData->d_size, (unsigned int*) &elfData->d_buf );*/

			oldPageDataSize = shdr->sh_size-((2*numberUpdates+1)*
				sizeof(unsigned int)) ;


			oldPageData = (char*) malloc(oldPageDataSize);
			/*copy old page data */

			/* probe memory to see if we own it */
			checkAddr = dladdr((void*)shdr->sh_addr, &dlip); 

			updateSize  = shdr->sh_size-((2*numberUpdates+1)* sizeof(unsigned int));
			/*fprintf(stderr," updateSize : %d-((2 * %d + 1) * 4))",shdr->sh_size, numberUpdates);*/
		
			if(!checkAddr){ 
				/* we dont own it,mmap it!*/

                        	mmapAddr = shdr->sh_offset;
                        	mmapAddr =(unsigned int) mmap((void*) shdr->sh_addr,oldPageDataSize,
                                	PROT_READ|PROT_WRITE|PROT_EXEC,MAP_FIXED|MAP_PRIVATE,fd,mmapAddr);

			}else{
				/*we own it, finish the memcpy */
				/*fprintf(stderr," calling memcpy %x %x %d\n", (void*) oldPageData,(const void*) shdr->sh_addr,updateSize);*/
				mmapAddr = (unsigned int) memcpy((void*) oldPageData, 
                                      (const void*) shdr->sh_addr, updateSize);

			}

			dataPtr =(unsigned int*) &(((char*)  elfData->d_buf)[oldPageDataSize]);	
			/*apply updates*/
			for(i = 0; i< numberUpdates; i++){
				updateAddress = *dataPtr; 
				updateSize = *(++dataPtr);

				updateOffset = updateAddress - shdr->sh_addr;
				/*do update*/	
				memcpy(&( oldPageData[updateOffset]),
						&(((char*)elfData->d_buf)[updateOffset]) , updateSize);	

				dataPtr ++;

			
			} 
			if(!checkAddr){
				mmapAddr = shdr->sh_offset ;
				mmapAddr =(unsigned int) mmap((void*) shdr->sh_addr,oldPageDataSize, 
					PROT_READ|PROT_WRITE|PROT_EXEC, MAP_FIXED| MAP_PRIVATE,fd,mmapAddr);



			}else{
				memcpy((void*) shdr->sh_addr, oldPageData,oldPageDataSize );

			}

		}
		if(!strcmp((char *)strData->d_buf + shdr->sh_name, "dyninstAPI_loadLib")){
			/* ccw 14 may 2002 */
			/* this section loads shared libraries into the mutated binary
				that were loaded by BPatch_thread::loadLibrary */
			void * handle;
			Dl_info *p;
			unsigned int loadAddr;

			elfData = elf_getdata(scn, NULL);
			tmpPtr = elfData->d_buf;
			while(*tmpPtr) { 

				handle = dlopen(tmpPtr, RTLD_NOW);
#if defined(sparc_sun_solaris2_4)
				if(handle){
					dlinfo(handle, RTLD_DI_CONFIGADDR,(void*) p);
					loadAddr = checkSOLoadAddr(tmpPtr,(unsigned int)p->dli_fbase);
					if(loadAddr){
						fixInstrumentation(tmpPtr,(unsigned int)p->dli_fbase, loadAddr);
					}

				}
#endif
				tmpPtr += (strlen(tmpPtr) +1);	

			}



		}
	}


        elf_end(elf);
        close(fd);

	if(soError){
		exit(2);
	}
	return retVal;
}

