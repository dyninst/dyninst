/* $Id: RTmutatedBinary_XCOFF.c,v 1.2 2003/02/04 15:19:06 bernat Exp $ */


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
		close(fd);
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
	sprintf(execStr,"./dyninst_mutatedBinary");


	XCOFFfile = loadFile(execStr, &fd);

	if(!XCOFFfile){
		/*printf("Cannot restore mutated binary\n\n");*/
		return 0;
	}

	/*printf(" restoring mutated binary...");*/

	XCOFFhdr = (struct xcoffhdr*) XCOFFfile;
	/*fine end of symbol table*/
	firstScnhdr = (struct scnhdr*) ( ((char*) XCOFFfile) + XCOFFhdr->filehdr.f_symptr +
			XCOFFhdr->filehdr.f_nsyms * 18); /* 18 == sizeof symbol */
	/*find end of string table*/
	firstScnhdr = (struct scnhdr *) ( ((char*) firstScnhdr ) + *((unsigned int*) firstScnhdr));
		/*(struct scnhdr*)  ( (char*) XCOFFfile + sizeof(struct xcoffhdr));*/

	currScnhdr = firstScnhdr;

	pageSize =  getpagesize();
   	for(currScnhdr = firstScnhdr, cnt=0; cnt < XCOFFhdr->filehdr.f_nscns; currScnhdr++, cnt++){
		if(!strcmp( currScnhdr->s_name, "dyn_dat")){
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

					result = mmap((void*) currScnhdr->s_vaddr, currScnhdr->s_size, 
					PROT_READ|PROT_WRITE|PROT_EXEC, MAP_ANONYMOUS| MAP_FIXED,-1,0);
					if( result == -1){

						mprotect((void*) currScnhdr->s_vaddr, currScnhdr->s_size,
							PROT_READ|PROT_WRITE|PROT_EXEC);
						result = mmap((void*) currScnhdr->s_vaddr, currScnhdr->s_size, 
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

					//memcpy(currScnhdr->s_vaddr, ((char*) XCOFFfile) + currScnhdr->s_scnptr, currScnhdr->s_size);
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
			//Dl_info dlip;

			retVal = 1; /* just to be sure */

			data = (char*) XCOFFfile + currScnhdr->s_scnptr;
			
			numberUpdates = (unsigned int) ( ((unsigned int*) data)[
				(dataSize - sizeof(unsigned int))/ sizeof(unsigned int) ]);
			oldPageDataSize = dataSize-((2*numberUpdates+1)*
				sizeof(unsigned int)) ;
			oldPageData = (char*) malloc(oldPageDataSize);
			/*copy old page data */

			/* probe memory to see if we own it */
			checkAddr = 1;//dladdr((void*)currScnhdr->s_vaddr, &dlip); 

			updateSize  = dataSize-((2*numberUpdates+1)* sizeof(unsigned int));
		
			if(!checkAddr){ 
				/* we dont own it,mmap it!*/
				mmapAddr = currScnhdr->s_scnptr;

                        	mmapAddr =(unsigned int) mmap((void*) currScnhdr->s_vaddr,oldPageDataSize,
                                	PROT_READ|PROT_WRITE|PROT_EXEC,MAP_FIXED|MAP_PRIVATE,fd,mmapAddr);
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

				checkAddr = mprotect(mmapAddr, currScnhdr->s_vaddr - mmapAddr + oldPageDataSize, 
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


