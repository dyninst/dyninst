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

/* $Id: RTmutatedBinary_XCOFF.c,v 1.8 2005/03/23 18:34:00 bernat Exp $ */


/* this file contains the code to restore the necessary
   data for a mutated binary in XCOFF format
 */


#include "dyninstAPI_RT/h/dyninstAPI_RT.h"
#include <unistd.h>
#include  <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <dlfcn.h>
#include <limits.h>
#include <sys/stat.h>
#include <xcoff.h>

/* Copied from RTheap-aix.c */
extern Address DYNINSTheap_loAddr;
extern Address DYNINSTheap_hiAddr;


unsigned int checkAddr;
/*char *buffer;
extern int isMutatedExec;
*/


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

void* loadFile(char *name, int *fd){

	struct stat statInfo;
	int statRet ;

	*fd = open( name, O_RDONLY);

	if( *fd == -1 ){
                /*printf("cannot open : %s\n",name);
    		fflush(stdout); */
		return 0;
	}


	statRet = fstat(*fd, &statInfo);	
	
	if( statRet == -1) {
		printf(" no stats found for %s \n",name);
		close(*fd);
		return 0;
	}
	
	return mmap(0x0, statInfo.st_size, PROT_READ, MAP_FILE, *fd,0x0); 

}

int checkMutatedFile(){


        int       cnt,fd;
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
	int sawFirstHeapTrampSection = 0;

	struct scnhdr *currScnhdr, *firstScnhdr;
	struct xcoffhdr *XCOFFhdr;
	char *data;
	void *XCOFFfile;
	unsigned int *numbNewHdrs;
	sprintf(execStr,"%s/dyninst_mutatedBinary",getenv("PWD"));


	XCOFFfile = loadFile(execStr, &fd);

	if(!XCOFFfile){
		/*printf("Cannot restore mutated binary\n\n");*/
		return 0;
	}

	/*fprintf(stderr," restoring mutated binary...");*/

	XCOFFhdr = (struct xcoffhdr*) XCOFFfile;


	/* 	WHAT IS GOING ON HERE?
		I cannot put the extra data (tramps, highmem data, shared lib info)
		in the middle of the file as in the ELF stuff.  I must place it at
		the end, AFTER the symbol table and string table.  Really this is
		like having two XCOFF files stuck together.  THe loader only pays
		attention to the first one (ie the one refered to by the file header
		info).  The stuff tacked on at the end is as follows:

		number of sections
		section header
		section header
		...
		section data
		section data
		...

		I need to find the end of the string table and then pull out the
		number of section headers tacked on to the end.  Then I can
		go through the section data a section at a time.
		
	*/

	/*fine end of symbol table*/
	firstScnhdr = (struct scnhdr*) ( ((char*) XCOFFfile) + XCOFFhdr->filehdr.f_symptr +
			XCOFFhdr->filehdr.f_nsyms * 18); /* 18 == sizeof symbol */
	/*find end of string table*/
	numbNewHdrs = (unsigned int*) ( ((char*) firstScnhdr ) + *((unsigned int*) firstScnhdr));
		/*(struct scnhdr*)  ( (char*) XCOFFfile + sizeof(struct xcoffhdr));*/

	firstScnhdr = (struct scnhdr *) ( sizeof(unsigned int) + ((char*) firstScnhdr ) + *((unsigned int*) firstScnhdr));

	currScnhdr = firstScnhdr;

	pageSize =  getpagesize();

   	for(currScnhdr = firstScnhdr, cnt=0; cnt < *numbNewHdrs; currScnhdr++, cnt++){
		if(!strcmp( currScnhdr->s_name, "dyn_lib")){
			/* use dlopen to load a list of shared libraries */

			int len;

			data = (char*) XCOFFfile + currScnhdr->s_scnptr;
			while(*data != '\0'){
				DYNINSTloadLibrary(data);
				data += (strlen(data) +1);
			}
		}else if(!strcmp( currScnhdr->s_name, "dyn_dat")){
			/* reload data */
			tmpPtr =  (char*) XCOFFfile + currScnhdr->s_scnptr;
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
			retVal = 1;

		}else if(!strncmp( currScnhdr->s_name, "dyn_",4)){

			/* THIS SHOULD NEVER OCCUR!*/

			/* heap tramps */
			char *tmpStr;
			printf("%s\n",  currScnhdr->s_name);
			retVal = 1; /* this is a restored run */

			tmpStr = &currScnhdr->s_name[4]; 
			if( *tmpStr>=0x30 && *tmpStr <= 0x39 ) {
				/* this is a heap tramp section */
				if( sawFirstHeapTrampSection ){

					result = (int)mmap((void*) currScnhdr->s_vaddr, currScnhdr->s_size, 
					PROT_READ|PROT_WRITE|PROT_EXEC, MAP_ANONYMOUS| MAP_FIXED,-1,0);
					if( result == -1){

						mprotect((void*) currScnhdr->s_vaddr, currScnhdr->s_size,
							PROT_READ|PROT_WRITE|PROT_EXEC);
						result = (int) mmap((void*) currScnhdr->s_vaddr, currScnhdr->s_size, 
						PROT_READ|PROT_WRITE|PROT_EXEC, MAP_ANONYMOUS| MAP_FIXED,-1,0);
						if( result == -1){
						perror("TRYING TO MMAP:");
						switch(errno){
							case EACCES:
								printf(" EACCES \n");
								break;
							case EBADF:
								printf(" EBADF \n");
								break;
							case EINVAL:
								printf(" EINVAL\n");
								break;
							case ENOMEM:
								printf(" NO MEM!\n");
								break;
							default:
								printf(" OTHER ERROR %d\n",errno);
						}
						}
					}

					printf(" MMAP! vaddr %x size %x scnptr %x : result %x",currScnhdr->s_vaddr,currScnhdr->s_size,currScnhdr->s_scnptr, result );
					fflush(stdout);

					memcpy(currScnhdr->s_vaddr, ((char*) XCOFFfile) + currScnhdr->s_scnptr,
						currScnhdr->s_size);
/*					result = mmap((void*) currScnhdr->s_vaddr, currScnhdr->s_size, 
					PROT_READ|PROT_WRITE|PROT_EXEC,
					MAP_FIXED|MAP_PRIVATE,fd,currScnhdr->s_scnptr);
					printf(" RESULT! %x", result);*/
				}else{

					printf(" mmecpy! %x %x %x",currScnhdr->s_vaddr,currScnhdr->s_size,
						((char*) XCOFFfile) + currScnhdr->s_scnptr );
					fflush(stdout);

					/*memcpy(currScnhdr->s_vaddr, ((char*) XCOFFfile) + currScnhdr->s_scnptr, currScnhdr->s_size);*/
					sawFirstHeapTrampSection = 1;
				}
			}

		}else if(!strncmp( currScnhdr->s_name, "dyH_", 4)){
			/* high mem tramps */

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
			int dataSize = currScnhdr->s_size;
			/*Dl_info dlip;*/

			retVal = 1; /* just to be sure */

			data = (char*) XCOFFfile + currScnhdr->s_scnptr	;
			
			numberUpdates = (unsigned int) ( ((unsigned int*) data)[
				(dataSize - sizeof(unsigned int))/ sizeof(unsigned int) ]);
			oldPageDataSize = dataSize-((2*numberUpdates+1)*
				sizeof(unsigned int)) ;
			oldPageData = (char*) malloc(oldPageDataSize);
			/*copy old page data */

			/* probe memory to see if we own it */
			checkAddr = 1;/*dladdr((void*)currScnhdr->s_vaddr, &dlip); */

			updateSize  = dataSize-((2*numberUpdates+1)* sizeof(unsigned int));

            /* Actually have a checkAddr... */
            if (currScnhdr->s_vaddr >= DYNINSTheap_loAddr &&
                currScnhdr->s_vaddr <= DYNINSTheap_hiAddr)
                checkAddr = 0;
            
			if(!checkAddr){ 
				/* we dont own it,mmap it!*/
                int fdzero = open("/dev/zero", O_RDWR);
                mmapAddr = 0;
                mmapAddr =(unsigned int) mmap((void*) currScnhdr->s_vaddr,
                                              oldPageDataSize,
                                              PROT_READ|PROT_WRITE|PROT_EXEC,
                                              MAP_FIXED|MAP_PRIVATE,
                                              fdzero,
                                              0);
                close(fdzero);
				mmapAddr = memcpy(oldPageData,
                                  (void*)currScnhdr->s_vaddr ,
                                  updateSize);
                checkAddr = 1;
			}else{
				/*we own it, finish the memcpy */
				mmapAddr = memcpy(oldPageData, (void*)currScnhdr->s_vaddr , updateSize);
				checkAddr = 1; 
			}
	
			dataPtr =(unsigned int*) &(data[oldPageDataSize]);	
			/*apply updates*/
			for(i = 0; i< numberUpdates; i++){
				updateAddress = *dataPtr; 
				updateSize = *(++dataPtr);

				updateOffset = updateAddress -  currScnhdr->s_vaddr;
				/*do update*/	
				memcpy(&( oldPageData[updateOffset]),
						&(data[updateOffset]) , updateSize);	
				dataPtr ++;
			} 
			if(!checkAddr){
				mmapAddr =  currScnhdr->s_scnptr;
				mmapAddr =(unsigned int) mmap((void*) currScnhdr->s_vaddr,oldPageDataSize, 
					PROT_READ|PROT_WRITE|PROT_EXEC, MAP_FIXED| MAP_PRIVATE,fd,mmapAddr);

			}else{
				if( currScnhdr->s_vaddr < 0xd0000000){
				/*mmapAddr = currScnhdr->s_vaddr - (currScnhdr->s_vaddr %pageSize);

				mmapAddr = mmap(mmapAddr,currScnhdr->s_vaddr - mmapAddr + oldPageDataSize, 
					PROT_READ|PROT_WRITE|PROT_EXEC, MAP_ANONYMOUS|MAP_FIXED |MAP_PRIVATE,-1,0);
				
				perror(" MMAP ");
				printf(" MMAP: %x \n", mmapAddr);
				*/
				mmapAddr = currScnhdr->s_vaddr - (currScnhdr->s_vaddr %pageSize);

				checkAddr = mprotect((void *)mmapAddr, currScnhdr->s_vaddr - mmapAddr + oldPageDataSize, 
					PROT_READ|PROT_WRITE|PROT_EXEC);
				/*printf(" MPROTECT: %x %lx : %lx \n", checkAddr, mmapAddr,currScnhdr->s_vaddr - mmapAddr + oldPageDataSize
 );*/

				memcpy((void*)currScnhdr->s_vaddr, oldPageData,oldPageDataSize );
				}
			}
		}
	}

	fflush(stdout);
        close(fd);

	return retVal;
}


