/*
 * Copyright (c) 1996 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
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

/* $Id: RTcommon.c,v 1.20 2002/02/12 15:42:06 chadd Exp $ */

#if defined(i386_unknown_nt4_0)
#include <process.h>
#define getpid _getpid
#else
#if !defined(mips_unknown_ce2_11) /*ccw 15 may 2000 : 29 mar 2001*/
#include <unistd.h>
#endif

#endif
#if !defined(mips_unknown_ce2_11) /*ccw 15 may 2000 : 29 mar 2001*/
#include <assert.h>
#endif

#include "dyninstAPI_RT/h/dyninstAPI_RT.h"


#if defined(sparc_sun_solaris2_4) || defined(i386_unknown_linux2_0) /* ccw 19 nov 2001*/
#include  <fcntl.h>

#if defined(sparc_sun_solaris2_4)
#include  <libelf.h>
#elif defined(i386_unknown_linux2_0)
#include <libelf/libelf.h>
#define __USE_GNU
#endif

#include <sys/mman.h>
#include <dlfcn.h>
#include <link.h> /* ccw 23 jan 2002 */
#if defined(sparc_sun_solaris2_4) 
#include <sys/link.h>
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


#endif

extern void DYNINSTbreakPoint();
extern void DYNINSTos_init(int calledByFork, int calledByAttach);

unsigned int DYNINSTversion = 1;
unsigned int DYNINSTobsCostLow;
unsigned int DYNINSThasInitialized = 0; /* 0 : has not initialized
					   2 : initialized by Dyninst
					   3 : initialized by Paradyn */

struct DYNINST_bootstrapStruct DYNINST_bootstrap_info ={0,0,0,'\0'} ; /*ccw 10 oct 2000 : 29 mar 2001*/

double DYNINSTglobalData[SYN_INST_BUF_SIZE/sizeof(double)];
double DYNINSTstaticHeap_32K_lowmemHeap_1[32*1024/sizeof(double)];
double DYNINSTstaticHeap_4M_anyHeap_1[4*1024*1024/sizeof(double)];

/* Written to by daemon just before launching an inferior RPC */
rpcInfo curRPC = { 0, 0, 0 };
unsigned pcAtLastIRPC;  /* just used to check for errors */
/* 1 = a trap was ignored and needs to be regenerated
   0 = there is not a trap that hasn't been processed */
int trapNotHandled = 0;

#ifdef DEBUG_PRINT_RT
int DYNINSTdebugPrintRT = 1;
#else
int DYNINSTdebugPrintRT = 0;
#endif

int DYNINST_mutatorPid = -1;

extern const char V_libdyninstAPI_RT[];

double DYNINSTdummydouble = 4321.71; /* Global so the compiler won't
					optimize away FP code in initFPU */
static void initFPU()
{
       /* Init the FPU.  We've seen bugs with Linux (e.g., Redhat 6.2
	  stock kernel on PIIIs) where processes started by Paradyn
	  started with FPU uninitialized. */
       double x = 17.1234;
       DYNINSTdummydouble *= x;
}

#if defined(i386_unknown_nt4_0)  /*ccw 13 june 2001*/
/* these variables are used by the mutator to pass values to the dll
 they are only used by the win2k/nt40 dyninstAPI*/
int libdyninstAPI_RT_DLL_localCause=-1, libdyninstAPI_RT_DLL_localPid=-1; /*ccw 2 may 2001*/
#endif


#if defined(sparc_sun_solaris2_4) || defined(i386_unknown_linux2_0)

unsigned int checkAddr;
int isElfFile=0;

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

int checkElfFile(){


	Elf32_Shdr *shdr;
        Elf32_Ehdr *   ehdr;
        Elf *          elf;
        int       cnt,fd;
        Elf_Data *elfData,*strData;
        Elf_Scn *scn;
        char execStr[256];
	int retVal = 0;
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
                RTprintf("%s %s \n",execStr, elf_errmsg(elf_errno()));
                RTprintf("cannot elf_begin\n");
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
				dataSize = *(int*) tmpPtr;
				tmpPtr+=sizeof(int);
				dataAddress = *(Address*) tmpPtr;
				tmpPtr += sizeof(Address);
				if(dataAddress){
					memcpy((char*) dataAddress, tmpPtr, dataSize);
					tmpPtr += dataSize;
				}
			}

		}else if(!strncmp((char *)strData->d_buf + shdr->sh_name, "dyninstAPI_",11)){
			retVal = 1; /* this is a restored run */
		}
		if(!strcmp((char *)strData->d_buf + shdr->sh_name, "dyninstAPI_mutatedSO")){
			/* make sure the mutated SOs are loaded, not the original ones */
			char *soNames;
			int totallen=0;
			__Elf32_Dyn *_dyn = (__Elf32_Dyn*)& _DYNAMIC;	
#if defined(sparc_sun_solaris2_4)
			Link_map *lmap=0;
#elif defined(i386_unknown_linux2_0)
			struct link_map *lmap=0;
#endif
			struct r_debug *_r_debug;
			char *loadedname, *dyninstname;

			elfData = elf_getdata(scn, NULL);

			while(_dyn && _dyn->d_tag != 0 && _dyn->d_tag != 21){
				_dyn ++;
			}
			if(_dyn && _dyn->d_tag != 0){
				_r_debug = _dyn->d_un.d_ptr;
				lmap = _r_debug->r_map;
		
				for(soNames = (char*) elfData->d_buf ; totallen<elfData->d_size; 
					soNames = &((char*) elfData->d_buf)[strlen(soNames)+1] ){
					totallen += strlen(soNames) + 1;
					lmap = _r_debug->r_map;
					while(lmap){
						loadedname = strrchr(lmap->l_name,'/');
						dyninstname =  strrchr(soNames,'/');
						if(loadedname == 0){
							loadedname = lmap->l_name;
						}
						if(dyninstname == 0){
							dyninstname = soNames;
						}	
						if(!strcmp(loadedname, dyninstname)) {
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
			oldPageDataSize = shdr->sh_size-((2*numberUpdates+1)*
				sizeof(unsigned int)) ;
			oldPageData = (char*) malloc(oldPageDataSize);
			/*copy old page data */

			/* probe memory to see if we own it */
			checkAddr = dladdr((void*)shdr->sh_addr, &dlip); 

			updateSize  = shdr->sh_size-((2*numberUpdates+1)* sizeof(unsigned int));
		
			if(!checkAddr){ 
				/* we dont own it,mmap it!*/

                        	mmapAddr = shdr->sh_offset + pageSize- (shdr->sh_offset % pageSize);
                        	mmapAddr =(unsigned int) mmap((void*) shdr->sh_addr,sizeof(oldPageData),
                                	PROT_READ|PROT_WRITE|PROT_EXEC,MAP_FIXED|MAP_PRIVATE,fd,mmapAddr);
			}else{
				/*we own it, finish the memcpy */
				memcpy(oldPageData, (void*) shdr->sh_addr, updateSize);
			}
	
			dataPtr =(unsigned int*) &(((char*)  elfData->d_buf)[oldPageDataSize]);	
			/*apply updates*/
			for(i = 0; i< numberUpdates; i++){
				updateAddress = *dataPtr; 
				updateSize = *(++dataPtr);

				updateOffset = updateAddress - shdr->sh_addr;
				/*do update*/	
				/*printf("i %d  updateAddress 0x%x updateSize 0x%x updateOffset 0x%x totalSize: %x\n",
						i,updateAddress,updateSize,updateOffset, oldPageDataSize);*/
				memcpy(&( oldPageData[updateOffset]),
						&(((char*)elfData->d_buf)[updateOffset]) , updateSize);	
				dataPtr ++;
			} 
			if(!checkAddr){
				mmapAddr = shdr->sh_offset + pageSize- (shdr->sh_offset % pageSize);
				mmapAddr =(unsigned int) mmap((void*) shdr->sh_addr,sizeof(oldPageData), 
					PROT_READ|PROT_WRITE|PROT_EXEC, MAP_FIXED| MAP_PRIVATE,fd,mmapAddr);
			}else{
				memcpy((void*) shdr->sh_addr, oldPageData,oldPageDataSize );
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

#endif

#if defined(i386_unknown_linux2_0) 
/* with solaris, the mutatee has a jump from
 * main() to a trampoline that calls DYNINSTinit() the
 * trampoline resides in the area that was previously
 * the heap, this trampoline is loaded as part of the
 * data segment
 *
 * with linux the trampolines are ALL in the big
 * array at the top of this file and so are not loaded
 * by the loader as part of the data segment. this
 * needs to be called to map in everything before
 * main() jumps to the big array
 */ 
void _init(){

	isElfFile =checkElfFile();
}
#endif


/*
 * The Dyninst API arranges for this function to be called at the entry to
 * main().
 */
void DYNINSTinit(int cause, int pid)
{
    int calledByFork = 0, calledByAttach = 0;
	int isRestart=0;

    initFPU();


#if defined(sparc_sun_solaris2_4) || defined(i386_unknown_linux2_0)
	/* this checks to see if this is a restart or a
	  	normal attach  ccw 19 nov 2001*/
    	if(!isElfFile){ /* on linux, dont check this twice (first in _init() ) */
		isRestart = checkElfFile();	
	}else{
		isRestart = isElfFile;
	}
	if(isRestart){
		fflush(stdout);
		cause = 9;
	}
#endif

    DYNINSThasInitialized = 2;

   if (cause == 2) calledByFork = 1;
   else if (cause == 3) calledByAttach = 1;

    /* sanity check */
#if !defined(mips_unknown_ce2_11) /*ccw 15 may 2000 : 29 mar 2001*/
    assert(sizeof(int64_t) == 8);
    assert(sizeof(int32_t) == 4);
#endif

#ifndef mips_unknown_ce2_11 /*ccw 23 july 2001*/
    RTprintf("%s\n", V_libdyninstAPI_RT);
#endif

    DYNINSTos_init(calledByFork, calledByAttach);

#if !defined(mips_unknown_ce2_11) /*ccw 16 may 2000 : 29 mar 2001*/
    DYNINST_bootstrap_info.pid = getpid();
#endif
    DYNINST_bootstrap_info.ppid = pid;
    DYNINST_bootstrap_info.event = cause;

    DYNINST_mutatorPid = pid;
	
#ifndef i386_unknown_nt4_0 /*ccw 13 june 2001*/

#ifndef mips_unknown_ce2_11 
	if(isRestart==0){ /*ccw 19 nov 2001 */
	   DYNINSTbreakPoint();
	}
#else
	__asm("break 1"); /*ccw 25 oct 2000 : 29 mar 2001*/
	__asm("nop");
#endif

#endif

}

#if defined(i386_unknown_nt4_0)  /*ccw 13 june 2001*/
#include <windows.h>

/* this function is automatically called when windows loads this dll
 if we are launching a mutatee to instrument, dyninst will place
 the correct values in libdyninstAPI_RT_DLL_localPid and
 libdyninstAPI_RT_DLL_localCause and they will be passed to
 DYNINSTinit to correctly initialize the dll.  this keeps us
 from having to instrument two steps from the mutator (load and then 
 the execution of DYNINSTinit()
*/
BOOL WINAPI DllMain(
  HINSTANCE hinstDLL,  /* handle to DLL module */
  DWORD fdwReason,     /* reason for calling function */
  LPVOID lpvReserved   /* reserved */
){

	if(libdyninstAPI_RT_DLL_localPid != -1 || libdyninstAPI_RT_DLL_localCause != -1){
		DYNINSTinit(libdyninstAPI_RT_DLL_localCause,libdyninstAPI_RT_DLL_localPid);
	}
	return 1; 
}
 

#endif

#if !defined(i386_unknown_nt4_0)  && !defined(mips_unknown_ce2_11)/*ccw 2 may 2000 : 29 mar 2001*/
/*
 * handle vfork special case
 */
void DYNINSTvfork(int parent)
{
    /* sanity check */
    assert(sizeof(int64_t)  == 8);
    assert(sizeof(int32_t)  == 4);

    if (parent != getpid()) {
	DYNINST_bootstrap_info.pid = getpid();
	DYNINST_bootstrap_info.ppid = getppid();
	DYNINST_bootstrap_info.event = 3;
    } else {
	DYNINSTbreakPoint();
	DYNINST_bootstrap_info.event = 0;
    }
}
#endif
