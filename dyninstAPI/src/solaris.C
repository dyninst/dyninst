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

// $Id: solaris.C,v 1.188 2005/11/03 05:21:07 jaw Exp $

#include "dyninstAPI/src/symtab.h"
#include "common/h/headers.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/dyn_thread.h"
#include "dyninstAPI/src/stats.h"
#include "common/h/Types.h"
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/termios.h>
#include <unistd.h>
#include "dyninstAPI/src/showerror.h"
#include "common/h/pathName.h" // concat_pathname_components()
#include "common/h/debugOstream.h"
#include "common/h/solarisKludges.h"
#include "dyninstAPI/src/editSharedLibrary.h" //ccw 11 mar 2005

#include "mapped_module.h"
#include "mapped_object.h"
#include "dynamiclinking.h"

#if defined (sparc_sun_solaris2_4)
#include "dyninstAPI/src/inst-sparc.h"
#else
#include "dyninstAPI/src/inst-x86.h"
#endif

#include "instPoint.h"
#include "baseTramp.h"

#include <procfs.h>
#include <stropts.h>
#include <link.h>
#include <dlfcn.h>
#include <strings.h> //ccw 11 mar 2005

#include "dyn_lwp.h"

#define DLOPEN_MODE (RTLD_NOW | RTLD_GLOBAL)

int (*P_native_demangle)(const char *, char *, size_t);

extern "C" {
extern long sysconf(int);
};

/*
   Define the indices of some registers to be used with pr_reg.
   These values are different on sparc and x86 platforms.
   RETVAL_REG: the registers that holds the return value of calls ($o0 on sparc,
               %eax on x86).
   PC_REG: program counter
   FP_REG: frame pointer (%i7 on sparc, %ebp on x86) 
*/
#ifdef sparc_sun_solaris2_4
#define PARAM1_REG (R_O0)
#define RETVAL_REG (R_O0)
#define PC_REG (R_PC)
#define FP_REG (R_O6)
#endif
#ifdef i386_unknown_solaris2_5
#define RETVAL_REG (EAX)
#define SP_REG (UESP)
#define PC_REG (EIP)
#define FP_REG (EBP)
#endif


// already setup on this FD.
// disconnect from controlling terminal 
void OS::osDisconnect(void) {
  //This is the POSIX-compliant way of disconnecting from the terminal 
  setpgrp();
}

/*
 * The set operations (set_entry_syscalls and set_exit_syscalls) are defined
 * in sol_proc.C
 */


// Compatibility for /proc
bool process::get_entry_syscalls(sysset_t *entry)
{
    pstatus_t status;
    if (!get_status(&status)) return false;
    
    memcpy(entry, &(status.pr_sysentry), sizeof(sysset_t));    
    return true;
}

bool process::get_exit_syscalls(sysset_t *exit)
{
    pstatus_t status;
    if (!get_status(&status)) return false;

    memcpy(exit, &(status.pr_sysexit), sizeof(sysset_t));
    return true;
}    

//TODO: This function should be converted to use process objects, not BPatch.
bool process::dldumpSharedLibrary(pdstring originalLibNameFullPath, char* dirName){
	BPatch_Vector<BPatch_snippet *> args;
	char *newLibName = saveWorldFindNewSharedLibraryName(originalLibNameFullPath,dirName);

   bool exists;
   BPatch_process *bproc = BPatch::bpatch->getProcessByPid(getPid(), &exists);

   assert(exists);
	BPatch_constExpr oldNameArg(originalLibNameFullPath.c_str());
	BPatch_constExpr newNameArg(newLibName);

	args.push_back(&oldNameArg);
	args.push_back(&newNameArg);
	
	BPatch_Vector<BPatch_function *> bpfv;
	
	if (((NULL == bproc->getImage()->findFunction("DYNINSTsaveRtSharedLibrary", bpfv) || !bpfv.size()))) {
		cout << __FILE__ << ":" << __LINE__ << ": FATAL:  Cannot find Internal Function " << "DYNINSTsaveRtSharedLibrary" << endl;
		if( newLibName){
			delete [] newLibName;
		}
		return false;
	}
	
	BPatch_function *dldump_func = bpfv[0]; 
	if (dldump_func == NULL) {
		if(newLibName){
			delete [] newLibName;
		}
		return false;
	}
	
	BPatch_funcCallExpr call_dldump(*dldump_func, args);
	
	/*fprintf(stderr,"CALLING dldump\n"); */
	if (!bproc->oneTimeCodeInternal(call_dldump, NULL, true)) {
                fprintf(stderr, "%s[%d]:  oneTimeCodeInternal failed\n", FILE__, __LINE__);
		// dldump FAILED
		// find the (global var) error string in the RT Lib and send it to the
		// error reporting mechanism
		BPatch_variableExpr *dlerror_str_var = bproc->getImage()->findVariable("gLoadLibraryErrorString");
		assert(NULL != dlerror_str_var);
	
		char dlerror_str[256];
		dlerror_str_var->readValue((void *)dlerror_str, 256);
		cerr << dlerror_str << endl;
		BPatch_reportError(BPatchWarning, 0, dlerror_str);
		if(newLibName){
			delete [] newLibName;
		}
		return false;
	}

	editSharedLibrary editSL;
	bool res = editSL.removeBSSfromSharedLibrary(newLibName);	
	delete [] newLibName;
	return res;
}


char* process::dumpPatchedImage(pdstring imageFileName){ //ccw 28 oct 2001

	writeBackElf *newElf;
	addLibrary *addLibraryElf;
   char *data, *paddedData;
	unsigned int errFlag=0;
	char name[50];	
	pdvector<imageUpdate*> compactedUpdates;
	pdvector<imageUpdate*> compactedHighmemUpdates;
	Address guardFlagAddr= trampGuardBase();
	char *mutatedSharedObjects=0;
	int mutatedSharedObjectsSize = 0, mutatedSharedObjectsIndex=0;
	char *directoryName = 0;
	mapped_object *sh_obj;
	if(!collectSaveWorldData){
		BPatch_reportError(BPatchSerious,122,"dumpPatchedImage: BPatch_thread::enableDumpPatchedImage() not called.  No mutated binary saved\n");
		return NULL;
	}


	directoryName = saveWorldFindDirectory();

	if(!directoryName){
		return NULL;
	}
	strcat(directoryName, "/");

	unsigned int dl_debug_statePltEntry = 0x00016574;//a pretty good guess
	unsigned int dyninst_SharedLibrariesSize = 0, mutatedSharedObjectsNumb;

	newElf = new writeBackElf((char*) mapped_objects[0]->fileName().c_str(),
		"/tmp/dyninstMutatee",errFlag);
	newElf->registerProcess(this);
	//add section that has, as its address, the original load address of
	//libdyninstAPI_RT.so.  The RT lib will check to see that it is loaded
	//in the correct place when the mutated binary is run.

	Address rtlibAddr;
	for(int i=0; i < mapped_objects.size() ; i++) {
            sh_obj = mapped_objects[i];
            if( strstr(sh_obj->fileName().c_str(),"libdyninstAPI_RT") ) {
                rtlibAddr = sh_obj->codeBase();
            }
	}

	/*fprintf(stderr,"SAVING RTLIB ADDR: %x\n",rtlibAddr);*/
	newElf->addSection(0,&rtlibAddr,sizeof(Address),"rtlib_addr",false);
/*
	//save the RT lib to the new directory using dldump in the RT lib.
	if(!dldumpSharedLibrary(dyninstRT_name, directoryName)){
		char *msg;
		msg = new char[dyninstRT_name.length() + 512];
		sprintf(msg,"dumpPatchedImage: libdyninstAPR_RT.so.1 not saved to %s.\nTry to use %s with the mutated binary.\n",directoryName,dyninstRT_name.c_str());

		BPatch_reportError(BPatchWarning,0,msg);
		delete [] msg;
	}
*/

	// DLDUMP every shared lib marked dirty or dirtycalled to the new directory:
	//	remove .bss section
	//	change filesz of Datasegment in PHT
	//	ehdr needs start of section headers updated
	// Always save the API_RT lib
        // Libraries: start at index 1
	for(unsigned i=i; i < mapped_objects.size() ; i++) {
            sh_obj = mapped_objects[i];
            if( sh_obj->isDirty() || sh_obj->isDirtyCalled() || strstr(sh_obj->fileName().c_str(),"libdyninstAPI_RT")){
                /*fprintf(stderr,"\nWRITE BACK SHARED OBJ %s\n", sh_obj->getName().c_str());*/
                
                if(!dldumpSharedLibrary(sh_obj->fileName(),directoryName)){
                    char *msg;
                    msg = new char[sh_obj->fileName().length() + strlen(directoryName)+128];
                    sprintf(msg,"dumpPatchedImage: %s not saved to %s.\n.\nTry to use the original shared library with the mutated binary.\n",sh_obj->fileName().c_str(),directoryName);
                    
                    BPatch_reportError(BPatchWarning,0,msg);
                    delete [] msg;
                }
            }
            
	}
        
	dl_debug_statePltEntry = saveWorldSaveSharedLibs(mutatedSharedObjectsSize, 
                                                         dyninst_SharedLibrariesSize,
                                                         directoryName, 
                                                         mutatedSharedObjectsNumb);
        
        
        
	// for each mutated shared object
	//	read the .text section from the new file, copy over the instrumentation code, 
	//	this allows subtest 6 to pass)
	//	add dyninst specific sections
	if(mutatedSharedObjectsSize){

		//UPDATED: 24 jul 2003 to include flag.
		//the flag denotes whether the shared lib is Dirty (1) or only DirtyCalled (0)
		// This is going to be a section that looks like this:
		// string
		// addr
		// flag
		// ...
		// string
		// addr
		// flag
		
		mutatedSharedObjectsSize += mutatedSharedObjectsNumb * sizeof(unsigned int);
		mutatedSharedObjects = new char[mutatedSharedObjectsSize ];
                // 1: shared library start (a.out is 0)
		for(unsigned i=1; i<mapped_objects.size() ; i++) {
                    sh_obj = mapped_objects[i];
                    //i ignore the dyninst RT lib here and in process::saveWorldSaveSharedLibs
                    if(sh_obj->isDirty() || sh_obj->isDirtyCalled()&& NULL==strstr(sh_obj->fileName().c_str(),"libdyninstAPI_RT")){ //ccw 24 jul 2003
                        memcpy(  & ( mutatedSharedObjects[mutatedSharedObjectsIndex]),
                                 sh_obj->fileName().c_str(),
                                 strlen(sh_obj->fileName().c_str())+1);
                        mutatedSharedObjectsIndex += strlen(
                                                            sh_obj->fileName().c_str())+1;
                        unsigned int baseAddr = sh_obj->getBaseAddress();
                        memcpy( & (mutatedSharedObjects[mutatedSharedObjectsIndex]),
                                &baseAddr, sizeof(unsigned int));
                        mutatedSharedObjectsIndex += sizeof(unsigned int);	
                        
                        //set flag
                        unsigned int tmpFlag = ((sh_obj->isDirty()
                                                 &&  NULL==strstr(sh_obj->fileName().c_str(),"libc")) ?1:0);	
                        memcpy( &(mutatedSharedObjects[mutatedSharedObjectsIndex]), &tmpFlag, sizeof(unsigned int));
                        mutatedSharedObjectsIndex += sizeof(unsigned int);	
                        
                    }
		}	
	}
	char *dyninst_SharedLibrariesData = saveWorldCreateSharedLibrariesSection(dyninst_SharedLibrariesSize);
        
	/*newElf = new writeBackElf(( char*) getImage()->file().c_str(),
          "/tmp/dyninstMutatee",errFlag);
          newElf->registerProcess(this);*/
        
	imageUpdates.sort(imageUpdateSort);// imageUpdate::mysort ); 

	newElf->compactLoadableSections(imageUpdates,compactedUpdates);

	highmemUpdates.sort(imageUpdateSort);

	newElf->compactSections(highmemUpdates, compactedHighmemUpdates);

	newElf->alignHighMem(compactedHighmemUpdates);

	int sectionsAdded = 0;
	unsigned int newSize, nextPage, paddedDiff;
	unsigned int pageSize = getpagesize();


	//This adds the LOADABLE HEAP TRAMP sections
	//if new platforms require this it will be moved
	//to process.C proper. Right now, only solaris uses it
	for(unsigned int i=0;i<compactedUpdates.size();i++) {
		data = new char[compactedUpdates[i]->size];
		readDataSpace((void*) compactedUpdates[i]->address, 
						  compactedUpdates[i]->size, data, true);

		//the TrampGuardFlag must be set to 1 to get the
		//tramps to run. it is not necessary set to 1 yet so
		//we set it to 1 before we save the file.
		if(guardFlagAddr){
			if(compactedUpdates[i]->address < guardFlagAddr &&
                        guardFlagAddr < compactedUpdates[i]->address+compactedUpdates[i]->size){
                                data[guardFlagAddr -
                                        compactedUpdates[i]->address ] = 0x1;
                        }
		}

		sprintf(name,"dyninstAPI_%08x",sectionsAdded);

		// what i want to do is this: 
		// I want MMAP rather than MEMCPY as much into place 
		// as i can. 
		// My assumptions:
		// there can be any number of heap trampoline sections
		// Only the first section could have valid (non heap tramp) data on the
		// beginning of its first page. Any subsequent sections will begin on
		// a different page than the previous page ends on [by the way 
		// compactLoadableSections works] and the heap has been moved
		// beyond the last heap tramp section so the mutatee cannot
		// put data there.
		// 
		// for the first section, i=0, make
		// two sections, where the first one is everything up to the first
		// page break, which will be memcpy'ed to memory
		// the second and subsequent sections will be aligned to be
		// mmaped ..... which means some sections will get bigger.
		
		if( i == 0 ){
			/* create memcpy section */
			nextPage = compactedUpdates[i]->address - (compactedUpdates[i]->address%pageSize);
			nextPage +=pageSize;
			newSize = nextPage - compactedUpdates[i]->address;	

			if( newSize > compactedUpdates[i]->size ){
				/* INSURE FIX CCW */
				/* 	changing newSize will cause us to read beyond the buffer data in addSection
					we should really realloc data to be of size newSize
				*/
				char *tmpData = new char[newSize];
				memcpy(tmpData,data,compactedUpdates[i]->size);
				delete data;
				data = tmpData;
				/* end realloc */
			}
			newElf->addSection(compactedUpdates[i]->address,data ,newSize,name);
			sectionsAdded ++;

			if(compactedUpdates[i]->size > newSize){
				/* only create this section if the size of the update
					spans beyond the first memcpy section*/ 
				/* create mmap section */
				sprintf(name,"dyninstAPI_%08x",sectionsAdded);
				newElf->addSection(nextPage, &(((char*) data)[newSize]) ,
					compactedUpdates[i]->size- newSize,name);
				sectionsAdded ++;
			}
		}else{
			/* create section padded out backwards to a page boundry */
			paddedDiff = (compactedUpdates[i]->address%pageSize);
			paddedData = new char[paddedDiff + compactedUpdates[i]->size];
			memset(paddedData, '\0', paddedDiff + compactedUpdates[i]->size); //necessary?
			memcpy(&( ((char*) paddedData)[paddedDiff]), data, compactedUpdates[i]->size);

			newElf->addSection(compactedUpdates[i]->address-paddedDiff, paddedData,
				paddedDiff + compactedUpdates[i]->size, name);
			delete [] (char*) paddedData;
				
		}
		delete [] (char*) data;
	}

	saveWorldCreateHighMemSections(compactedHighmemUpdates, highmemUpdates, (void*) newElf);

	if(mutatedSharedObjectsSize){
		newElf->addSection(0, mutatedSharedObjects, mutatedSharedObjectsSize,
								 "dyninstAPI_mutatedSO", false);
	}

	unsigned int k;
	
	for( k=0;k<imageUpdates.size();k++){
		delete imageUpdates[k];
	}

	for( k=0;k<highmemUpdates.size();k++){
		delete highmemUpdates[k];
	}
	for(  k=0;k<compactedUpdates.size();k++){
                delete compactedUpdates[k];
        }

        for(  k=0;k<compactedHighmemUpdates.size();k++){
                delete compactedHighmemUpdates[k];
        }

	//the following is for the dlopen problem 
	newElf->addSection(dl_debug_statePltEntry, dyninst_SharedLibrariesData, 
	dyninst_SharedLibrariesSize, "dyninstAPI_SharedLibraries", false);

	delete [] dyninst_SharedLibrariesData;


	//the following reloads any shared libraries loaded into the
	//mutatee using BPatch_thread::loadLibrary
	saveWorldAddSharedLibs((void*)newElf); // ccw 14 may 2002 

	saveWorldCreateDataSections((void*)newElf);
/*
	//add section that has, as its address, the original load address of
	//libdyninstAPI_RT.so.  The RT lib will check to see that it is loaded
	//in the correct place when the mutated binary is run.

	Address rtlibAddr;
	for(int i=0;mapped_objects && i<(int)mapped_objects->size() ; i++) {
		sh_obj = mapped_objects[i];
		if( strstr(sh_obj->fileName().c_str(),"libdyninstAPI_RT") ) {
			rtlibAddr = sh_obj->getBaseAddress();
		}
	}
*/
	/*fprintf(stderr,"SAVING RTLIB ADDR: %x\n",rtlibAddr);*/
//	newElf->addSection(0,&rtlibAddr,sizeof(Address),"rtlib_addr",false);

	newElf->createElf();
	char* fullName = new char[strlen(directoryName) + strlen ( (char*)imageFileName.c_str())+1];
	strcpy(fullName, directoryName);
	strcat(fullName, (char*)imageFileName.c_str());
	
	addLibraryElf = new addLibrary();
	elf_update(newElf->getElf(), ELF_C_WRITE);
	if(!addLibraryElf->driver(newElf->getElf(), fullName, "libdyninstAPI_RT.so.1")) {
		//ccw 27 jun 2003
		BPatch_reportError(BPatchSerious,122,"dumpPatchedImage: addLibraryElf() failed!  No mutated binary saved\n");
		delete [] directoryName;
                return NULL;

	}
	delete [] fullName;
	if(mutatedSharedObjects){
		delete [] mutatedSharedObjects;
	}
	delete addLibraryElf; //INSURE CCW 
	delete newElf; // INSURE CCW
	return directoryName;
}

bool process::dumpImage(pdstring imageFileName) 
{
    int newFd;
    pdstring command;

    pdstring origFile = getAOut()->fileName();
   
    // first copy the entire image file
    command = "cp ";
    command += origFile;
    command += " ";
    command += imageFileName;
    system(command.c_str());

    // now open the copy
    newFd = open(imageFileName.c_str(), O_RDWR, 0);
    if (newFd < 0) {
	// log error
	return false;
    }

    Elf_X elf(newFd, ELF_C_READ);
    if (!elf.isValid()) return false;

    Elf_X_Shdr shstrscn = elf.get_shdr( elf.e_shstrndx() );
    Elf_X_Data shstrdata = shstrscn.get_data();
    const char* shnames = (const char *) shstrdata.get_string();

    Address baseAddr = 0;
    int length = 0;
    int offset = 0;
    for (int i = 0; i < elf.e_shnum(); ++i) {
	Elf_X_Shdr shdr = elf.get_shdr(i);
	const char *name = (const char *) &shnames[shdr.sh_name()];

	if (!P_strcmp(name, ".text")) {
	    offset = shdr.sh_offset();
	    length = shdr.sh_size();
	    baseAddr = shdr.sh_addr();
	    break;
	}
    }

    char *tempCode = new char[length];
    bool ret = readTextSpace((void *) baseAddr, length, tempCode);
    if (!ret) {
	// log error

	delete[] tempCode;
	elf.end();
	P_close(newFd);

	return false;
    }

    lseek(newFd, offset, SEEK_SET);
    write(newFd, tempCode, length);

    // Cleanup
    delete[] tempCode;
    elf.end();
    P_close(newFd);

    return true;
}

/* Auxiliary function */
bool checkAllThreadsForBreakpoint(process *proc, Address break_addr)
{
   pdvector<Frame> activeFrames;
   if (!proc->getAllActiveFrames(activeFrames)) return false;
   for(unsigned frame_iter = 0; frame_iter < activeFrames.size(); frame_iter++)
   {
      if (activeFrames[frame_iter].getPC() == break_addr) {
         return true;
      }
   }
   
   return false;
}

bool process::trapAtEntryPointOfMain(dyn_lwp *, Address)
{
    if (main_brk_addr == 0x0) return false;
    return checkAllThreadsForBreakpoint(this, main_brk_addr);
}

bool process::handleTrapAtEntryPointOfMain(dyn_lwp *)
{
    assert(main_brk_addr);
    
  // restore original instruction 
    writeDataSpace((void *)main_brk_addr, sizeof(savedCodeBuffer), 
                   (char *)savedCodeBuffer);
    main_brk_addr = 0;
    return true;
}

bool process::insertTrapAtEntryPointOfMain()
{

    int_function *f_main = 0;
    pdvector<int_function *> funcs;
    
    //first check a.out for function symbol   
    bool res = findFuncsByPretty("main", funcs);
    if (!res)
    {
        logLine( "a.out has no main function. checking for PLT entry\n" );
        //we have not found a "main" check if we have a plt entry
	res = findFuncsByPretty( "DYNINST_pltMain", funcs );
 
	if (!res) {
            logLine( "no PLT entry for main found\n" );
            return false;
	  }       
    }
    
    if( funcs.size() > 1 ) {
        cerr << __FILE__ << __LINE__ 
             << ": found more than one main! using the first" << endl;
    }
    f_main = funcs[0];
    assert(f_main);

    Address addr = f_main->getAddress(); 

    codeGen gen(instruction::size());
    instruction::generateTrap(gen);

    // save original instruction first
    readDataSpace((void *)addr, sizeof(savedCodeBuffer), savedCodeBuffer, true);
    
    writeDataSpace((void *)addr, gen.used(), gen.start_ptr());

    main_brk_addr = addr;
    
    return true;
}

bool process::getDyninstRTLibName() {
   if (dyninstRT_name.length() == 0) {
      // Get env variable
      if (getenv("DYNINSTAPI_RT_LIB") != NULL) {
         dyninstRT_name = getenv("DYNINSTAPI_RT_LIB");
      }
      else {
         pdstring msg = pdstring("Environment variable ") +
                        pdstring("DYNINSTAPI_RT_LIB") +
                        pdstring(" has not been defined for process ") +
                        pdstring(pid);
         showErrorCallback(101, msg);
         return false;
      }
   }
   // Check to see if the library given exists.
   if (access(dyninstRT_name.c_str(), R_OK)) {
      pdstring msg = pdstring("Runtime library ") + dyninstRT_name +
                     pdstring(" does not exist or cannot be accessed!");
      showErrorCallback(101, msg);
      return false;
   }
   return true;
}

bool process::loadDYNINSTlib() {
    // we will write the following into a buffer and copy it into the
    // application process's address space
    // [....LIBRARY's NAME...|code for DLOPEN]
    
    // write to the application at codeOffset. This won't work if we
    // attach to a running process.
    //Address codeBase = this->getImage()->codeOffset();
    // ...let's try "_start" instead
    int_function *_startfn;
    
    pdvector<int_function *> funcs;
    bool res = findFuncsByPretty("_start", funcs);
    if (!res) {
        // we can't instrument main - naim
        showErrorCallback(108,"_start() unfound");
        return false;
    }

    if( funcs.size() > 1 ) {
        cerr << __FILE__ << __LINE__ 
             << ": found more than one _start! using the first" << endl;
    }
    _startfn = funcs[0];

    Address codeBase = _startfn->getAddress();
    assert(codeBase);
    
    // Or should this be readText... it seems like they are identical
    // the remaining stuff is thanks to Marcelo's ideas - this is what 
    // he does in NT. The major change here is that we use AST's to 
    // generate code for dlopen.
    
    // savedCodeBuffer[BYTES_TO_SAVE] is declared in process.h
    readDataSpace((void *)codeBase, sizeof(savedCodeBuffer), savedCodeBuffer, true);
    
    codeGen scratchCodeBuffer(BYTES_TO_SAVE);

    // First we write in the dyninst lib string. Vewy simple.
    Address dyninstlib_addr = codeBase;
    
    scratchCodeBuffer.copy(dyninstRT_name.c_str(), dyninstRT_name.length()+1);

    // Were we're calling into
    Address dlopencall_addr = codeBase + scratchCodeBuffer.used();
    /*
      fprintf(stderr, "dlopen call addr at 0x%x, for codeBase of 0x%x\n",
            dlopencall_addr, codeBase);
    */

    // deadList and deadListSize are also used in inst-sparc.C
    // registers 8 to 15: out registers 
    // registers 16 to 22: local registers
    Register deadList[10] = { 16, 17, 18, 19, 20, 21, 22, 0, 0, 0 };
    unsigned dead_reg_count = 7;
    if(! multithread_capable()) {
        deadList[7] = 23;
        dead_reg_count++;
    }
    
    registerSpace *dlopenRegSpace =
        new registerSpace(dead_reg_count, deadList, (unsigned)0,
                          NULL, multithread_capable());
    dlopenRegSpace->resetSpace();

    pdvector<AstNode*> dlopenAstArgs(2);
    AstNode *dlopenAst;
    
    // We call directly into ld.so.1. This used to be handled in 
    // process::findInternalSymbols, which made it very difficult
    // to figure out what was going on.
    Address dlopen_func_addr = dyn->get_dlopen_addr();
    assert(dlopen_func_addr);

    //fprintf(stderr, "We want to call 0x%x\n", dlopen_func_addr);
    // See if we can get a function for it.
    int_function *dlopenFunc = findFuncByAddr(dlopen_func_addr);
    //fprintf(stderr, "Attempt to find func pointer: %p\n", dlopenFunc);

    dlopenAstArgs[0] = new AstNode(AstNode::Constant, (void *)(dyninstlib_addr));
    dlopenAstArgs[1] = new AstNode(AstNode::Constant, (void*)DLOPEN_MODE);
    dlopenAst = new AstNode(dlopen_func_addr, dlopenAstArgs);
    removeAst(dlopenAstArgs[0]);
    removeAst(dlopenAstArgs[1]);

    dlopenAst->generateCode(this, dlopenRegSpace, scratchCodeBuffer,
                            true, true);
    removeAst(dlopenAst);

    // Slap in a breakpoint
    dyninstlib_brk_addr = codeBase + scratchCodeBuffer.used();
    instruction::generateTrap(scratchCodeBuffer);
    
    writeDataSpace((void *)codeBase, scratchCodeBuffer.used(), 
                   scratchCodeBuffer.start_ptr());
    
    //fprintf(stderr, "Breakpoint at 0x%x\n", dyninstlib_brk_addr);
    
    // save registers
    savedRegs = new dyn_saved_regs;
    bool status = getRepresentativeLWP()->getRegisters(savedRegs);
    assert(status == true);
    
    if (!getRepresentativeLWP()->changePC(dlopencall_addr, NULL)) {
        logLine("WARNING: changePC failed in loadDYNINSTlib\n");
        assert(0);
    }
    setBootstrapState(loadingRT_bs);
    return true;
}

bool process::trapDueToDyninstLib(dyn_lwp *)
{
  if (dyninstlib_brk_addr == 0) return(false);
  return checkAllThreadsForBreakpoint(this, dyninstlib_brk_addr);
}

bool process::loadDYNINSTlibCleanup(dyn_lwp *)
{
  // rewrite original instructions in the text segment we use for 
  // the inferiorRPC - naim
  unsigned count = sizeof(savedCodeBuffer);
  //Address codeBase = getImage()->codeOffset();

  int_function *_startfn;

    pdvector<int_function *> funcs;
    bool res = findFuncsByPretty("_start", funcs);
    if (!res) {
        // we can't instrument main - naim
        showErrorCallback(108,"_start() unfound");
        return false;
    }

    if( funcs.size() > 1 ) {
        cerr << __FILE__ << __LINE__ 
             << ": found more than one main! using the first" << endl;
    }
    _startfn = funcs[0];

    Address codeBase = _startfn->getAddress();
    assert(codeBase);
    if (!writeDataSpace((void *)codeBase, count, (char *)savedCodeBuffer))
        return false;

    // restore registers
    if (!getRepresentativeLWP()->restoreRegisters(*savedRegs))
     return false;
    delete savedRegs;
    savedRegs = NULL;
    return true;
}

bool process::getExecFileDescriptor(pdstring filename,
                                    int /*pid*/,
                                    bool /*whocares*/,
                                    int &,
                                    fileDescriptor &desc)
{
    desc = fileDescriptor(filename, 0, 0, false);
    return true;
}

#if defined(USES_DYNAMIC_INF_HEAP)
static const Address lowest_addr = 0x0;
void process::inferiorMallocConstraints(Address near, Address &lo, Address &hi,
                                        inferiorHeapType /* type */)
{
  if (near)
    {
      lo = region_lo(near);
      hi = region_hi(near);  
    }
}

void process::inferiorMallocAlign(unsigned &size)
{
     /* 32 byte alignment.  Should it be 64? */
  size = (size + 0x1f) & ~0x1f;
}
#endif

bool process::dumpCore_(const pdstring coreName) 
{
  char command[100];

  sprintf(command, "gcore %d 2> /dev/null; mv core.%d %s", getPid(), getPid(), 
          coreName.c_str());

  detach(false);
  system(command);
  attach();

  return false;
}

int getNumberOfCPUs()
{
  // _SC_NPROCESSORS_CONF is the number of processors configured in the
  // system and _SC_NPROCESSORS_ONLN is the number of those processors that
  // are online.
  int numberOfCPUs;
  numberOfCPUs = (int) sysconf(_SC_NPROCESSORS_ONLN);
  if (numberOfCPUs) 
    return(numberOfCPUs);
  else 
    return(1);
}  

Frame Frame::getCallerFrame()
{
  Address newPC=0;
  Address newFP=0;
  Address newSP=0;

  //fprintf(stderr, "Frame::getCallerFrame for %p\n", this);

  if (uppermost_) {
    codeRange *range = getRange();
    int_function *func = range->is_function();
    if (func) {
      if (func->hasNoStackFrame()) { // formerly "isLeafFunc()"
	struct dyn_saved_regs regs;
	bool status;
	if (lwp_)
	  status = lwp_->getRegisters(&regs);
	else
	  status = getProc()->getRepresentativeLWP()->getRegisters(&regs);

	assert(status == true);
	newPC = regs.theIntRegs[R_O7] + 8;
	newFP = fp_; // frame pointer unchanged
	return Frame(newPC, newFP, newSP, 0, this);
      }
    }
  }
  //
  // For the sparc, register %i7 is the return address - 8 and the fp is
  // register %i6. These registers can be located in %fp+14*5 and
  // %fp+14*4 respectively, but to avoid two calls to readDataSpace,
  // we bring both together (i.e. 8 bytes of memory starting at %fp+14*4
  // or %fp+56).
  // These values are copied to the stack when the application is paused,
  // so we are assuming that the application is paused at this point
  
  struct {
    Address fp;
    Address rtn;
  } addrs;
  
  if (getProc()->readDataSpace((caddr_t)(fp_ + 56), 2*sizeof(int),
			       (caddr_t)&addrs, true))
    {

      newFP = addrs.fp;
      newPC = addrs.rtn + 8;

      if (isSignalFrame()) {
         // get the value of the saved PC: this value is stored in the
         // address specified by the value in register i2 + 44. Register i2
         // must contain the address of some struct that contains, among
         // other things, the saved PC value.
         u_int reg_i2;
         if (getProc()->readDataSpace((caddr_t)(fp_+40), sizeof(u_int),
                              (caddr_t)&reg_i2,true)) {
            Address saved_pc;
            if (getProc()->readDataSpace((caddr_t) (reg_i2+44), sizeof(int),
                                 (caddr_t) &saved_pc,true)) {
               
               int_function *func = getProc()->findFuncByAddr(saved_pc);
               
               newPC = saved_pc;
               if (func && func->hasNoStackFrame())
                  newFP = fp_;
            }
         }
	 else {
	   return Frame();
	 }
      }
      

      if(getProc()->multithread_capable()) {
         // MT thread adds another copy of the start function
         // to the top of the stack... this breaks instrumentation
         // since we think we're at a function entry.
         if (newFP == 0) newPC = 0;
      }
      Frame ret = Frame(newPC, newFP, 0, 0, this);

      // If we're in a base tramp, skip this frame (return getCallerFrame)
      // as we only return minitramps
      codeRange *range = getRange();
      if (range->is_multitramp()) {
          // If we're inside instrumentation only....
          multiTramp *multi = range->is_multitramp();
          baseTrampInstance *bti = multi->getBaseTrampInstanceByAddr(getPC());
          if (bti &&
              bti->isInInstru(getPC()))
              return ret.getCallerFrame();
      }
      return ret;
    }
   return Frame(); // zero frame
}

bool Frame::setPC(Address newpc) {
  fprintf(stderr, "Implement me! Changing frame PC from %x to %x\n",
	  pc_, newpc);
  return false;
}


void print_read_error_info(const relocationEntry entry, 
                           int_function *&target_pdf, Address base_addr) {

   sprintf(errorLine, "  entry      : target_addr 0x%lx\n",
           entry.target_addr());
   logLine(errorLine);
   sprintf(errorLine, "               rel_addr 0x%lx\n", entry.rel_addr());
   logLine(errorLine);
   sprintf(errorLine, "               name %s\n", (entry.name()).c_str());
   logLine(errorLine);
   
   if (target_pdf) {
      sprintf(errorLine, "  target_pdf : symTabName %s\n",
              (target_pdf->symTabName()).c_str());
      logLine(errorLine);    
      sprintf(errorLine , "              prettyName %s\n",
              (target_pdf->symTabName()).c_str());
      logLine(errorLine);
      sprintf(errorLine , "              addr 0x%lx\n",
              target_pdf->getAddress());
      logLine(errorLine);
   }

   sprintf(errorLine, "  base_addr  0x%lx\n", base_addr);
   logLine(errorLine);
}

// hasBeenBound: returns true if the runtime linker has bound the
// function symbol corresponding to the relocation entry in at the address
// specified by entry and base_addr.  If it has been bound, then the callee 
// function is returned in "target_pdf", else it returns false.
bool process::hasBeenBound(const relocationEntry &entry, 
			   int_function *&target_pdf, Address base_addr) {

// TODO: the x86 and sparc versions should really go in seperate files 
#if defined(i386_unknown_solaris2_5)

    if (status() == exited) return false;

    // if the relocationEntry has not been bound yet, then the value
    // at rel_addr is the address of the instruction immediately following
    // the first instruction in the PLT entry (which is at the target_addr) 
    // The PLT entries are never modified, instead they use an indirrect 
    // jump to an address stored in the _GLOBAL_OFFSET_TABLE_.  When the 
    // function symbol is bound by the runtime linker, it changes the address
    // in the _GLOBAL_OFFSET_TABLE_ corresponding to the PLT entry

    Address got_entry = entry.rel_addr() + base_addr;
    Address bound_addr = 0;
    if(!readDataSpace((const void*)got_entry, sizeof(Address), 
			&bound_addr, true)){
        sprintf(errorLine, "read error in process::hasBeenBound "
		"addr 0x%lx, pid=%d\n (readDataSpace returns 0)",
		got_entry, pid);
	logLine(errorLine);
	print_read_error_info(entry, target_pdf, base_addr);
        return false;
    }

    if( !( bound_addr == (entry.target_addr()+6+base_addr)) ) {
        // the callee function has been bound by the runtime linker
	// find the function and return it
        target_pdf = findFuncByAddr(bound_addr);
	if(!target_pdf){
            return false;
	}
        return true;	
    }
    return false;

#else
    // if the relocationEntry has not been bound yet, then the second instr 
    // in this PLT entry branches to the fist PLT entry.  If it has been   
    // bound, then second two instructions of the PLT entry have been changed 
    // by the runtime linker to jump to the address of the function.  
    // Here is an example:   
    //     before binding			after binding
    //	   --------------			-------------
    //     sethi  %hi(0x15000), %g1		sethi  %hi(0x15000), %g1
    //     b,a  <_PROCEDURE_LINKAGE_TABLE_>     sethi  %hi(0xef5eb000), %g1
    //	   nop					jmp  %g1 + 0xbc ! 0xef5eb0bc

    unsigned int insnBuf;

    Address next_insn_addr = entry.target_addr() + base_addr + instruction::size(); 
    if( !(readDataSpace((caddr_t)next_insn_addr, instruction::size(), 
                        (char *)&insnBuf, true)) ) {
        sprintf(errorLine, "read error in process::hasBeenBound addr 0x%lx"
                " (readDataSpace next_insn_addr returned 0)\n",
		next_insn_addr);
	logLine(errorLine);
	print_read_error_info(entry, target_pdf, base_addr);
    }
    instruction next_insn(insnBuf);
    // if this is a b,a instruction, then the function has not been bound
    if(((*next_insn).branch.op == FMT2op)  && ((*next_insn).branch.op2 == BICCop2) 
       && ((*next_insn).branch.anneal == 1) && ((*next_insn).branch.cond == BAcond)) {
	return false;
    } 

    // if this is a sethi instruction, then it has been bound...get target_addr
    Address third_addr = entry.target_addr() + base_addr + 8; 
    if( !(readDataSpace((caddr_t)third_addr, instruction::size(),
		       (char *)&insnBuf, true)) ) {
        sprintf(errorLine, "read error in process::hasBeenBound addr 0x%lx"
                " (readDataSpace third_addr returned 0)\n",
		third_addr);
	logLine(errorLine);
	print_read_error_info(entry,target_pdf, base_addr);
    }

    instruction third_insn(insnBuf);

    // get address of bound function, and return the corr. int_function
    if(((*next_insn).sethi.op == FMT2op) && ((*next_insn).sethi.op2 == SETHIop2)
	&& ((*third_insn).rest.op == RESTop) && ((*third_insn).rest.i == 1)
	&& ((*third_insn).rest.op3 == JMPLop3)) {
        
	Address new_target = ((*next_insn).sethi.imm22 << 10) & 0xfffffc00; 
	new_target |= (*third_insn).resti.simm13;

        target_pdf = findFuncByAddr(new_target);
	if(!target_pdf){
            return false;
	}
	return true;
    }
    // this is a messed up entry
    return false;
#endif

}



// findCallee: finds the function called by the instruction corresponding
// to the instPoint "instr". If the function call has been bound to an
// address, then the callee function is returned in "target" and the 
// instPoint "callee" data member is set to pt to callee's int_function.  
// If the function has not yet been bound, then "target" is set to the 
// int_function associated with the name of the target function (this is 
// obtained by the PLT and relocation entries in the image), and the instPoint
// callee is not set.  If the callee function cannot be found, (ex. function
// pointers, or other indirect calls), it returns false.
// Returns false on error (ex. process doesn't contain this instPoint).
//
int_function *instPoint::findCallee() {

   if(callee_) {
       return callee_;
   }

       if (ipType_ != callSite) {
        return NULL;
    }

    if (isDynamicCall()) { 
        return NULL;
    }

    // Check if we parsed an intra-module static call
    assert(img_p_);
    image_func *icallee = img_p_->getCallee();
    if (icallee) {
        // Now we have to look up our specialized version
        // Can't do module lookup because of DEFAULT_MODULE...
        const pdvector<int_function *> *possibles = func()->obj()->findFuncVectorByMangled(icallee->symTabName());
        if (!possibles) {
            return NULL;
        }
        for (unsigned i = 0; i < possibles->size(); i++) {
          if ((*possibles)[i]->ifunc() == icallee) {
                callee_ = (*possibles)[i];
                return callee_;
            }
        }
        // No match... very odd
        assert(0);
        return NULL;
    }

    
    // get the target address of this function
    Address target_addr = img_p_->callTarget();
    //    Address insn_addr = instr.pointAddr(); 
    
    if(!target_addr) {  
        // this is either not a call instruction or an indirect call instr
        // that we can't get the target address
        return NULL;
    }
    
    
    // else, get the relocation information for this image
    const Object &obj = func()->obj()->parse_img()->getObject();
    const pdvector<relocationEntry> *fbt;
    if(!obj.get_func_binding_table_ptr(fbt)) {
        return false; // target cannot be found...it is an indirect call.
    }
    
    // find the target address in the list of relocationEntries
    Address base_addr = func()->obj()->codeBase();
    for(u_int i=0; i < fbt->size(); i++) {
        if((*fbt)[i].target_addr() == target_addr) {
            // check to see if this function has been bound yet...if the
            // PLT entry for this function has been modified by the runtime
            // linker
            int_function *target_pdf = 0;
            if(proc()->hasBeenBound((*fbt)[i], target_pdf, base_addr)) {
                callee_ = target_pdf;
                return callee_;
            } 
            else {
                // just try to find a function with the same name as entry 
                pdvector<int_function *> pdfv;
                bool found = proc()->findFuncsByMangled((*fbt)[i].name(), pdfv);
                if(found) {
                    assert(pdfv.size());
#ifdef BPATCH_LIBRARY
                    if(pdfv.size() > 1)
                        cerr << __FILE__ << ":" << __LINE__ 
                             << ": WARNING:  findAllFuncsByName found " 
                             << pdfv.size() << " references to function " 
                             << (*fbt)[i].name() << ".  Using the first.\n";
#endif
                    callee_ = pdfv[0];
                    return callee_;
                }
            }
            return NULL;
        }
    }
    return NULL;
}

void loadNativeDemangler() {
  
  P_native_demangle = NULL;
  void *hDemangler = dlopen("libdemangle.so", DLOPEN_MODE);
  if (hDemangler != NULL)
    P_native_demangle = (int (*) (const char *, char *, size_t)) 
      dlsym(hDemangler, "cplus_demangle");
}
// vim:ts=5:

bool process::initMT()
{
   return true;
}

#include <sched.h>
void dyninst_yield()
{
   sched_yield();
}
