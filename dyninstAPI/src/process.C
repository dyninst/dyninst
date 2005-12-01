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

// $Id: process.C,v 1.562 2005/12/01 00:56:24 jaw Exp $

#include <ctype.h>

#if defined(i386_unknown_solaris2_5)
#include <sys/procfs.h>
#endif

#include <set>

#include <stdio.h>

#include "common/h/headers.h"
#include "dyninstAPI/src/function.h"
//#include "dyninstAPI/src/func-reloc.h"
#include "dyninstAPI/src/baseTramp.h"
#include "dyninstAPI/src/miniTramp.h"
#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/dyn_thread.h"
#include "dyninstAPI/src/dyn_lwp.h"
#include "dyninstAPI/src/signalhandler.h"
#include "dyninstAPI/src/mailbox.h"
#include "dyninstAPI/src/EventHandler.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/util.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/instP.h"
#include "dyninstAPI/src/instPoint.h"
#include "dyninstAPI/src/os.h"
#include "dyninstAPI/src/showerror.h"
#include "dyninstAPI/src/callbacks.h"
#include "dyninstAPI/src/dynamiclinking.h"
#include "dyninstAPI/src/BPatch_asyncEventHandler.h"
#include "dyninstAPI/src/debuggerinterface.h"
#include "dyninstAPI_RT/h/dyninstAPI_RT.h"
// #include "paradynd/src/mdld.h"
#include "common/h/Timer.h"
#include "common/h/Time.h"
#include "common/h/timing.h"

#include "dyninstAPI/src/rpcMgr.h"

#include "mapped_module.h"
#include "mapped_object.h"

#include "dyninstAPI/h/BPatch.h"

#if defined(sparc_sun_solaris2_4) \
 || defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */
#include "dyninstAPI/src/writeBackElf.h"
#include "dyninstAPI/src/saveSharedLibrary.h" 
#elif defined(rs6000_ibm_aix4_1)
#include "dyninstAPI/src/writeBackXCOFF.h"
#endif

#include "dyninstAPI/src/syscallNotification.h"

#if !defined(BPATCH_LIBRARY)
#include "rtinst/h/rtinst.h"
#include "rtinst/h/trace.h"
#include "paradynd/src/perfStream.h"
#include "paradynd/src/metricFocusNode.h"
#include "paradynd/src/costmetrics.h"
#include "paradynd/src/mdld.h"
#include "paradynd/src/main.h"
#include "paradynd/src/init.h"
#include "pdutil/h/pdDebugOstream.h"
#include "common/h/int64iostream.h"
#endif

#ifndef BPATCH_LIBRARY
#ifdef PAPI
#include "paradynd/src/papiMgr.h"
#endif
#endif

#include "common/h/debugOstream.h"

#include "common/h/Timer.h"

#define FREE_WATERMARK (hp->totalFreeMemAvailable/2)
#define SIZE_WATERMARK 100
static const timeLength MaxWaitingTime(10, timeUnit::sec());
static const timeLength MaxDeletingTime(2, timeUnit::sec());

unsigned activeProcesses; // number of active processes
pdvector<process*> processVec;


#ifndef BPATCH_LIBRARY
extern pdstring osName;
#endif

#if defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */
extern void cleanupVsysinfo(void *ehd);
#endif

pdvector<instMapping*> initialRequests;

void printLoadDyninstLibraryError() {
    cerr << "Paradyn/Dyninst failed to load the runtime library. This is normally caused by " << endl;
    cerr << "one of the following:" << endl;
    cerr << "Incorrect DYNINSTAPI_RT_LIB environment variable" << endl;
    cerr << "Missing RT library" << endl;
    cerr << "Unavailable dependency of the library" << endl;
#if defined(rs6000_ibm_aix4_1)
    cerr << "   libDyninstText.a must exist in a directory in the LIBPATH environment variable" << endl;
#endif
    cerr << "Please check your environment and try again." << endl;
}


void setLibState(libraryState_t &lib, libraryState_t state) {
    if (lib > state) cerr << "Error: attempting to revert library state" << endl;
    else lib = state;
}

/* AIX method defined in aix.C; hijacked for IA-64's GP. */
#if !defined(rs6000_ibm_aix4_1) \
 && !defined(ia64_unknown_linux2_4)
Address process::getTOCoffsetInfo(Address /*dest */)
{
  Address tmp = 0;
  assert(0 && "getTOCoffsetInfo not implemented");
  return tmp; // this is to make the nt compiler happy! - naim
}
#else
Address process::getTOCoffsetInfo(Address dest)
{
    // We have an address, and want to find the module the addr is
    // contained in. Given the probabilities, we (probably) want
    // the module dyninst_rt is contained in.
    // I think this is the right func to use
    
    // Find out which object we're in (by addr).
    codeRange *range = NULL;
    codeSections_.find(dest, range);
    if (!range)  // Try data?
        dataSections_.find(dest, range);
    if (!range)
        return 0;
    mapped_object *mobj = range->is_mapped_object();
    if (!mobj) {
        mappedObjData *tmp = dynamic_cast<mappedObjData *>(range);
        if (tmp)
            mobj = tmp->obj;
    }
    // Very odd case if this is not defined.
    assert(mobj); 
    
    Address TOCoffset = mobj->parse_img()->getObject().getTOCoffset();
    return TOCoffset + mobj->dataBase();
}

#endif

#if defined(os_linux) && (defined(arch_x86) || defined(arch_x86_64))
extern void calcVSyscallFrame(process *p);
#endif

// Windows NT has its own version of the walkStack function in pdwinnt.C

// Note: stack walks may terminate early. In this case, return what we can.
// Relies on the getCallerFrame method in the various <os>.C files

#if !defined(mips_unknown_ce2_11) && !defined(i386_unknown_nt4_0)
bool process::walkStackFromFrame(Frame startFrame,
				 pdvector<Frame> &stackWalk)
{
  Address fpOld   = 0;
  Address fpNew   = 0;

  Frame currentFrame = startFrame;
  
  if (!isStopped()) {
      fprintf(stderr, "%s[%d]:  walkStackFromFrame failing\n", FILE__, __LINE__);
      return false;
  }
#ifndef BPATCH_LIBRARY
  startTimingStackwalk();
#endif

#if defined( os_linux ) 
	/* Do a special check for the vsyscall page.  Silently drop
	   the page if it exists. */
#if defined( arch_ia64 )
	/* The IA-64 doesn't use DWARF to unwind out of the vsyscall page,
	   so calcVsyscallFrame() is overkill. */
	if( getVsyscallStart() == 0x0 ) {
		if( ! readAuxvInfo() ) {
			/* We're probably on Linux 2.4; use default values. */
			setVsyscallRange( 0xffffffffffffe000, 0xfffffffffffff000 );
			setVsyscallData( NULL );
			}
		}
#else
	calcVSyscallFrame( this );
#endif /* defined( arch_ia64 ) */
  
  Address next_pc = currentFrame.getPC();
  if (next_pc >= getVsyscallStart() && next_pc < getVsyscallEnd()) {
     currentFrame = currentFrame.getCallerFrame();
  }
#endif /* defined( os_linux ) */

  while (!currentFrame.isLastFrame()) {
    // grab the frame pointer
    fpNew = currentFrame.getFP();

    // Check that we are not moving up the stack
    // successive frame pointers might be the same (e.g. leaf functions)
#if ! defined( os_linux )
    if (fpOld > fpNew) {
      // AIX:
      // There's a signal function in the MPI library that we're not
      // handling properly. Instead of returning an empty stack,
      // return what we have.
      // One thing that makes me feel better: gdb is getting royally
      // confused as well. This sucks for catchup.
      
#if defined( ia64_unknown_linux2_4 )
        /* My single-stepper needs to be able to continue past stackwalking errors. */      
        // /* DEBUG */ fprintf( stderr, "Not terminating stackwalk early, even though fpOld (0x%lx) > fpNew (0x%lx).\n", fpOld, fpNew );
        // /* DEBUG */ for( unsigned int i = 0; i < stackWalk.size(); i++ ) {
        // /* DEBUG */ 	cerr << stackWalk[i] << endl;
        // /* DEBUG */ 	} 
        // /* DEBUG */	cerr << currentFrame << endl; 
        // /* DEBUG */ cerr << endl;
        break;
#else
        // We should check to see if this early exit is warranted.
        fprintf(stderr, "%s[%d]:  failing stackWalk here\n", FILE__, __LINE__);
        return false;
#endif
    }
#endif
    fpOld = fpNew;
    stackWalk.push_back(currentFrame);    
    currentFrame = currentFrame.getCallerFrame(); 
  }
  // Clean up after while loop (push end frame)
  // FIXME: get LastFrame on AMD64 des not work the same as on other platforms
  //        since the FP is not always zero
  if (currentFrame.getProc() != NULL) {
      stackWalk.push_back(currentFrame);
  }

#ifndef BPATCH_LIBRARY
  stopTimingStackwalk();
#endif

  return true;
}
#endif

// Return a vector (possibly with one object) of active frames
// in the process

bool process::getAllActiveFrames(pdvector<Frame> &activeFrames)
{
  Frame active;
  bool success = true;
  if (!threads.size()) { // Nothing defined in the thread data structures
    // So use the process LWP instead (Dyninst)
    active = getRepresentativeLWP()->getActiveFrame();
    if (active == Frame()) { // Hrm.. should getActive return a bool?
      return false;
    }
    activeFrames.push_back(active);
  }
  else { // Iterate through threads
    for (unsigned i = 0; i < threads.size(); i++) {
      active = threads[i]->getActiveFrame();
      if (active == Frame()) {
	success = true;
      }
      else {
	activeFrames.push_back(active);
      }
    }
  }
  return success;
}

bool process::walkStacks(pdvector<pdvector<Frame> >&stackWalks)
{
  pdvector<Frame> stackWalk;
  if (!threads.size()) { // Nothing defined in thread data structures
    if (!getRepresentativeLWP()->walkStack(stackWalk))
      return false;
    // Use the walk from the default LWP
    stackWalks.push_back(stackWalk);
  }
  else { // Have threads defined
    for (unsigned i = 0; i < threads.size(); i++) {
      if (!threads[i]->walkStack(stackWalk))
         return false;
      stackWalks.push_back(stackWalk);
      stackWalk.resize(0);
    }
  }
  return true;
}

extern "C" int heapItemCmpByAddr(const heapItem **A, const heapItem **B)
{
  heapItem *a = *(heapItem **)const_cast<heapItem **>(A);
  heapItem *b = *(heapItem **)const_cast<heapItem **>(B);

  if (a->addr < b->addr) {
      return -1;
  } else if (a->addr > b->addr) {
      return 1;
  } else {
      return 0;
  }
}

// For exec/process deletion
void inferiorHeap::clear() {
    Address addr;
    heapItem *heapItemPtr;

    dictionary_hash_iter<Address, heapItem *> activeIter(heapActive);
    while (activeIter.next(addr, heapItemPtr))
        delete heapItemPtr;
    heapActive.clear();
    
    for (unsigned i = 0; i < heapFree.size(); i++)
        delete heapFree[i];
    heapFree.clear();

    disabledList.clear();

    disabledListTotalMem = 0;
    totalFreeMemAvailable = 0;
    freed = 0;

    for (unsigned j = 0; j < bufferPool.size(); j++)
        delete bufferPool[j];
    bufferPool.clear();
}

void process::inferiorFreeCompact(inferiorHeap *hp)
{
  pdvector<heapItem *> &freeList = hp->heapFree;
  unsigned i, nbuf = freeList.size();

  /* sort buffers by address */
  VECTOR_SORT(freeList, heapItemCmpByAddr);

  /* combine adjacent buffers */
  bool needToCompact = false;
  for (i = 1; i < freeList.size(); i++) {
      heapItem *h1 = freeList[i-1];
      heapItem *h2 = freeList[i];
      assert(h1->length != 0);
      if (h1->addr + h1->length > h2->addr) {
          fprintf(stderr, "Error: heap 1 (0x%x to 0x%x) overlaps heap 2 (0x%x to 0x%x)\n",
                  h1->addr, h1->addr + h1->length,
                  h2->addr, h2->addr + h2->length);
      }
      assert(h1->addr + h1->length <= h2->addr);
      if (h1->addr + h1->length == h2->addr
          && h1->type == h2->type) {
          h2->addr = h1->addr;
          h2->length = h1->length + h2->length;
          h1->length = 0;
          nbuf--;
          needToCompact = true;
      }
  }

  /* remove any absorbed (empty) buffers */ 
  if (needToCompact) {
    pdvector<heapItem *> cleanList;
    unsigned end = freeList.size();
    for (i = 0; i < end; i++) {
      heapItem *h1 = freeList[i];
      if (h1->length != 0) {
        cleanList.push_back(h1);
      } else {
        delete h1;
      }
    }
    assert(cleanList.size() == nbuf);
    for (i = 0; i < nbuf; i++) {
      freeList[i] = cleanList[i];
    }
    freeList.resize(nbuf);
    assert(freeList.size() == nbuf);
  }
}

// Search an object for heapage

bool process::getInfHeapList(const mapped_object *obj,
                             pdvector<heapDescriptor> &infHeaps)
{

    pdvector<mapped_object::foundHeapDesc> foundHeaps;
    obj->getInferiorHeaps(foundHeaps);

    for (u_int j = 0; j < foundHeaps.size(); j++)
    {
        // The string layout is: DYNINSTstaticHeap_size_type_unique
        // Can't allocate a variable-size array on NT, so malloc
        // that sucker
        char *temp_str = (char *)malloc(strlen(foundHeaps[j].name.c_str())+1);
        strcpy(temp_str, foundHeaps[j].name.c_str());
        char *garbage_str = strtok(temp_str, "_"); // Don't care about beginning
        assert(!strcmp("DYNINSTstaticHeap", garbage_str));
        // Name is as is.
        // If address is zero, then skip (error condition)
        if (foundHeaps[j].addr == 0)
        {
            cerr << "Skipping heap " << foundHeaps[j].name.c_str()
                 << "with address 0" << endl;
            continue;
        }
        // Size needs to be parsed out (second item)
        // Just to make life difficult, the heap can have an optional
        // trailing letter (k,K,m,M,g,G) which indicates that it's in
        // kilobytes, megabytes, or gigabytes. Why gigs? I was bored.
        char *heap_size_str = strtok(NULL, "_"); // Second element, null-terminated
        unsigned heap_size = (unsigned) atol(heap_size_str);
        if (heap_size == 0)
            /* Zero size or error, either way this makes no sense for a heap */
        {
            free(temp_str);
            continue;
        }
        switch (heap_size_str[strlen(heap_size_str)-1])
        {
      case 'g':
      case 'G':
          heap_size *= 1024;
      case 'm':
      case 'M':
          heap_size *= 1024;
      case 'k':
      case 'K':
          heap_size *= 1024;
      default:
          break;
        }
        
        // Type needs to be parsed out. Can someone clean this up?
        inferiorHeapType heap_type;
        char *heap_type_str = strtok(NULL, "_");
        
        if (!strcmp(heap_type_str, "anyHeap"))
            heap_type = anyHeap;
        else if (!strcmp(heap_type_str, "lowmemHeap"))
            heap_type = lowmemHeap;
        else if (!strcmp(heap_type_str, "dataHeap"))
            heap_type = dataHeap;
        else if (!strcmp(heap_type_str, "textHeap"))
            heap_type = textHeap;
        else if (!strcmp(heap_type_str, "uncopiedHeap"))
            heap_type = uncopiedHeap;
        else
        {
            cerr << "Unknown heap string " << heap_type_str << " read from file!" << endl;
            free(temp_str);
            continue;
        }
        infHeaps.push_back(heapDescriptor(foundHeaps[j].name.c_str(),
                                          foundHeaps[j].addr, 
                                          heap_size, heap_type));
        free(temp_str);
    }
  return foundHeaps.size() > 0;
}

/*
 * Returns true if the address given is within the signal handler function,
 * otherwise returns false.
 */
bool process::isInSignalHandler(Address addr)
{
#if defined(arch_ia64)
    // We handle this elsewhere
    return false;
#endif
    codeRange *range;
    if (signalHandlerLocations_.find(addr, range))
        return true;
    return false;
}

/*
 * This function adds an item to the dataUpdates vector
 * which is used to maintain a list of variables that have
 * been written by the mutator //ccw 26 nov 2001
 */
 
#if defined(sparc_sun_solaris2_4) \
 || defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */ \
 || defined(rs6000_ibm_aix4_1)

void process::saveWorldData( Address address, int size, const void * src ) {
	if( collectSaveWorldData ) {
		dataUpdate *newData = new dataUpdate;
		newData->address= address;
		newData->size = size;
		newData->value = new char[size];
		memcpy(newData->value, src, size);
		dataUpdates.push_back(newData);
		}
	} /* end saveWorldData() */
	
#else

void process::saveWorldData( Address, int, const void* ) { ; }	  

#endif

#if defined(sparc_sun_solaris2_4) \
 || defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */ \
 || defined(rs6000_ibm_aix4_1)
/* || defined(rs6000_ibm_aix4_1)*/

char* process::saveWorldFindDirectory(){

	const char* directoryNameExt = "_dyninstsaved";
	int dirNo = 0;
/* ccw */
	char cwd[1024];
        char* directoryName;
	int lastChar;
	getcwd(cwd, 1024);
	lastChar = strlen(cwd);

	if( cwd[lastChar] != '/' && lastChar != 1023){
		cwd[lastChar] = '/';
		cwd[++lastChar] ='\0';
	}

	directoryName = new char[strlen(cwd) +
                        strlen(directoryNameExt) + 3+1+1];
/* ccw */
	sprintf(directoryName,"%s%s%x",cwd, directoryNameExt,dirNo);
        while(dirNo < 0x1000 && mkdir(directoryName, S_IRWXU) ){
                 if(errno == EEXIST){
                         dirNo ++;
                 }else{
                         BPatch_reportError(BPatchSerious, 122, "dumpPatchedImage: cannot open directory to store mutated binary. No files saved\n");
                         delete [] directoryName;
                         return NULL;
                 }
                 sprintf(directoryName, "%s%s%x",cwd,
                         directoryNameExt,dirNo);
        }
	if(dirNo == 0x1000){
	         BPatch_reportError(BPatchSerious, 122, "dumpPatchedImage: cannot open directory to store mutated binary. No files saved\n");
	         delete [] directoryName;
	         return NULL;
	}
	return directoryName;

}
#endif

#if defined(sparc_sun_solaris2_4) \
 || defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */


char *process::saveWorldFindNewSharedLibraryName(pdstring originalLibNameFullPath, char* dirName){
	const char *originalLibName = originalLibNameFullPath.c_str();
	unsigned int index=0;

	unsigned int nextIndex = 0;
	for(nextIndex = 0; nextIndex < originalLibNameFullPath.length() ;nextIndex++){
		if(originalLibName[nextIndex] == '/'){
			index = nextIndex +1;
		}
	}

	pdstring oldLibName = originalLibNameFullPath.substr(index,originalLibNameFullPath.length());
	char* newLibName = new char[strlen(dirName) + oldLibName.length()+1];
	memcpy(newLibName,dirName,strlen(dirName)+1);
	newLibName =strcat(newLibName, oldLibName.c_str());

	return newLibName;

	
}



unsigned int process::saveWorldSaveSharedLibs(int &mutatedSharedObjectsSize, 
                                 unsigned int &dyninst_SharedLibrariesSize, 
                                 char* directoryName, unsigned int &count) {

   unsigned int dl_debug_statePltEntry=0;
#if defined(sparc_sun_solaris2_4)
   unsigned int tmp_dlopen;
#endif
   bool dlopenUsed = false;
   
   //In the mutated binary we need to catch the dlopen events and adjust the
   //instrumentation of the shared libraries (and jumps into the shared
   //libraries) as the base address of the shared libraries different for the
   //base addresses during the original mutator/mutatee run.

   //the r_debug interface ensures that a change to the dynamic linking
   //information causes _dl_debug_state to be called.  This is because dlopen
   //is too small and odd to instrument/breakpoint.  So our code will rely on
   //this fact. (all these functions are contained in ld-linux.so)

   //Our method: The Procedure Linking Table (.plt) for ld-linux contains an
   //entry that jumps to a specified address in the .rel.plt table. To call a
   //function, the compiler generates a jump to the correct .plt entry which
   //reads its jump value out of the .rel.plt.

   //On the sly, secretly replace the entry in .rel.plt with folgers crystals
   //and poof, we jump to our own function in RTcommon.c
   //(dyninst_dl_debug_state) [actually we replace the entry in .rel.plt with
   //the address of dyninst_dl_debug_state].  To ensure correctness,
   //dyninst_dl_debug_state contains a call to the real _dl_debug_state
   //immediately before it returns, thus ensuring any code relying on that
   //fact that _dl_debug_state is actually run remains happy.

   //It is very important then, that we know the location of the entry in the
   //.rel.plt table.  We need to record the offset of this entry with respect
   //to the base address of ld-linux.so (for obvious reasons) This offset is
   //then sent to RTcommon.c, and here is the slick part, by assigning it to
   //the 'load address' of the section "dyninstAPI_SharedLibraries," which
   //contains the shared library/basei address pairs used to fixup the saved
   //binary. This way when checkElfFile() reads the section the offset will
   //be there in the section header.

   //neat, eh?  this is how it will work in the future, currently this is not
   //yet fully implemented and part of the cvs tree.

   //UPDATE: the above is implemented EXCEPT for adjusting the instrumentation
   //when shared libraries move. currently an error is thrown when a shared
   //lib is in the wrong place and execution is terminated!

	//I have now added the notion of DirtyCalled to a shared library.
	//This is a library that contains a function that is called by
	//instrumentation.  The shared lib may or may not be instrumented itself.
	//If it is not instrumented (Dirty) then it is NOT saved as a mutated 
	//shared library.  A flag in dyninstAPI_mutatedSO section that follows
	//the filename denotes whether the library is Dirty or merely DirtyCalled

   count = 0;
   for (unsigned i = 1; i < mapped_objects.size(); i++) {
     // We start at 1 because 0 is the a.out
     mapped_object *sh_obj = mapped_objects[i];

      //ccw 24 jul 2003
      if( (sh_obj->isDirty() || sh_obj->isDirtyCalled()) &&
		/* there are some libraries we should not save even if they are marked as mutated*/
		NULL==strstr(sh_obj->fileName().c_str(),"libdyninstAPI_RT") && 
		NULL== strstr(sh_obj->fileName().c_str(),"ld-linux.so") && 
		NULL==strstr(sh_obj->fileName().c_str(),"libc")){ //ccw 6 jul 2003
         count ++;
         if(!dlopenUsed && sh_obj->isopenedWithdlopen()){
            BPatch_reportError(BPatchWarning,123,"dumpPatchedImage: dlopen used by the mutatee, this may cause the mutated binary to fail\n");
            dlopenUsed = true;
         }			
         //bperr(" %s is DIRTY!\n", sh_obj->fileName().c_str());
        

         if( sh_obj->isDirty()){ 
            //if the lib is only DirtyCalled dont save it! //ccw 24 jul 2003
            Address textAddr, textSize;
            char *newName = saveWorldFindNewSharedLibraryName(sh_obj->fileName(),directoryName);

			/*new char[strlen(sh_obj->fileName().c_str()) + 
                                     strlen(directoryName) + 1];
            memcpy(newName, directoryName, strlen(directoryName)+1);
            const char *file = strrchr(sh_obj->fileName().c_str(), '/');
            strcat(newName, file);*/
           

		/* 	what i need to do:
			open the ORIGINAL shared lib --> sh_obj->fileName()
			read the text section out.
			reapply the instrumentation code
			save this new, instrumented text section back to the NEW DLDUMPED file in the _dyninstSaved# dir --> newName
		*/		 

            saveSharedLibrary *sharedObj =
               new saveSharedLibrary(sh_obj->getBaseAddress(),
                                     sh_obj->fileName().c_str(), newName);

            	sharedObj->openBothLibraries();
            
			sharedObj->getTextInfo(textAddr, textSize);
			char* textSection = sharedObj->getTextSection(); /* get the text section from the ORIGINAL library */

			if(textSection){

				applyMutationsToTextSection(textSection, textAddr, textSize);
            
	          	sharedObj->saveMutations(textSection);
     	       	sharedObj->closeNewLibrary();
	            /*			
     	       //this is for the dlopen problem....
          	  if(strstr(sh_obj->fileName().c_str(), "ld-linux.so") ){
	            //find the offset of _dl_debug_state in the .plt
     	       dl_debug_statePltEntry = 
          	  sh_obj->parse_img()->getObject().getPltSlot("_dl_debug_state");
	            }
     	       */			
				delete [] textSection;
			}else{
				char msg[strlen(sh_obj->fileName().c_str())+100];
				sprintf(msg,"dumpPatchedImage: could not retreive .text section for %s\n",sh_obj->fileName().c_str());
            		BPatch_reportError(BPatchWarning,123,msg);
				sharedObj->closeNewLibrary();
			}
			sharedObj->closeOriginalLibrary();
		  delete sharedObj;
            delete [] newName;
         }
         mutatedSharedObjectsSize += strlen(sh_obj->fileName().c_str()) +1 ;
         mutatedSharedObjectsSize += sizeof(int); //a flag to say if this is only DirtyCalled
      }
      //this is for the dlopen problem...
      if(strstr(sh_obj->fileName().c_str(), "ld-linux.so") ){
         //find the offset of _dl_debug_state in the .plt
         dl_debug_statePltEntry = 
            sh_obj->parse_img()->getObject().getPltSlot("_dl_debug_state");
      }
#if defined(sparc_sun_solaris2_4)
      
      if( ((tmp_dlopen = sh_obj->parse_img()->getObject().getPltSlot("dlopen")) && 
           !sh_obj->isopenedWithdlopen())){
         dl_debug_statePltEntry = tmp_dlopen + sh_obj->getBaseAddress();
      }
#endif
      //this is for the dyninst_SharedLibraries section we need to find out
      //the length of the names of each of the shared libraries to create the
      //data buffer for the section
      
      dyninst_SharedLibrariesSize += strlen(sh_obj->fileName().c_str())+1;
      //add the size of the address
      dyninst_SharedLibrariesSize += sizeof(unsigned int);
   }
#if defined(sparc_sun_solaris2_4)
   if( (tmp_dlopen = getAOut()->parse_img()->getObject().getPltSlot("dlopen"))) {
       dl_debug_statePltEntry = tmp_dlopen;
   }
   
   //dl_debug_statePltEntry = parse_img()->getObject().getPltSlot("dlopen");
#endif
   dyninst_SharedLibrariesSize += 1;//for the trailing '\0'
   
   return dl_debug_statePltEntry;
}
	
bool process::applyMutationsToTextSection(char* /*textSection*/, unsigned /*textAddr*/, 
                                          unsigned /*textSize*/)
{
    // Uhh... what does this do?
#if 0
	mutationRecord *mr = afterMutationList.getHead();

	while (mr != NULL) {
            if( mr->addr >= textAddr && mr->addr < (textAddr+textSize)){
                memcpy(&(textSection[mr->addr-textAddr]), mr->data, mr->size);
            }
            mr = mr->next;
	}
	return true;
#endif
   return true;
}

char* process::saveWorldCreateSharedLibrariesSection(int dyninst_SharedLibrariesSize){
	//dyninst_SharedLibraries
	//the SharedLibraries sections contains a list of all the shared libraries
	//that have been loaded and the base address for each one.
	//The format of the sections is:
	//
	//sharedlibraryName
	//baseAddr
	//...
	//sharedlibraryName
	//baseAddr
	//'\0'
	
	char *dyninst_SharedLibrariesData = new char[dyninst_SharedLibrariesSize];
	char *ptr= dyninst_SharedLibrariesData;
	//int size = mapped_objects->size() - 1; // a.out is included as well
	mapped_object *sh_obj;

	for(unsigned i=1; i < mapped_objects.size(); i++) {
	  sh_obj = mapped_objects[i];

		memcpy((void*) ptr, sh_obj->fileName().c_str(), strlen(sh_obj->fileName().c_str())+1);
		//fprintf(stderr,"loaded shared libs %s : ", ptr);
		ptr += strlen(sh_obj->fileName().c_str())+1;

		unsigned int baseAddr = sh_obj->getBaseAddress();
		/* 	LINUX PROBLEM. in the link_map structure the map->l_addr field is NOT
			the load address of the dynamic object, as the documentation says.  It is the
			RELOCATED address of the object. If the object was not relocated then the
			value is ZERO.

			So, on LINUX we check the address of the dynamic section, map->l_ld, which is
			correct.
		*/
#if defined(i386_unknown_linux2_0) || defined(x86_64_unknown_linux2_4)
		Symbol info;
		pdstring dynamicSection = "_DYNAMIC";
		sh_obj->getSymbolInfo(dynamicSection,info);
		baseAddr = sh_obj->getBaseAddress() + info.addr();
		//fprintf(stderr," %s DYNAMIC ADDR: %x\n",sh_obj->fileName().c_str(), baseAddr);
#endif

		memcpy( (void*)ptr, &baseAddr, sizeof(unsigned int));
		//fprintf(stderr," 0x%x \n", *(unsigned int*) ptr);
		ptr += sizeof(unsigned int);
	}
	memset( (void*)ptr, '\0' , 1);

	return dyninst_SharedLibrariesData;
}
#endif

#if defined(sparc_sun_solaris2_4) \
 || defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */ \
 || defined(rs6000_ibm_aix4_1)
void process::saveWorldCreateHighMemSections(
                        pdvector<imageUpdate*> &compactedHighmemUpdates, 
                        pdvector<imageUpdate*> &highmem_updates,
                        void *ptr) {

   unsigned int trampGuardValue;
   Address guardFlagAddr= trampGuardBase();

   unsigned int pageSize = getpagesize();
   unsigned int startPage, stopPage;
   unsigned int numberUpdates=1;
   int startIndex, stopIndex;
   void *data;
   char name[50];
#if defined(sparc_sun_solaris2_4) \
 || defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */
	writeBackElf *newFile = (writeBackElf*) ptr;
#elif defined(rs6000_ibm_aix4_1)
	writeBackXCOFF *newFile = (writeBackXCOFF*) ptr;

#endif

	/*fprintf(stderr,"guardFlagAddr %x\n",guardFlagAddr);*/
   	readDataSpace((void*) guardFlagAddr, sizeof(unsigned int),
                 (void*) &trampGuardValue, true);
   
   	bool err = writeDataSpace((void*)guardFlagAddr, sizeof(unsigned int),
                  (void*) &numberUpdates);
        if (!err) fprintf(stderr, "%s[%d][%s]:  writeDataSpace failed\n", __FILE__, __LINE__, getThreadStr(getExecThreadID()));
        assert(err);

	saveWorldData((Address) guardFlagAddr,sizeof(unsigned int), &numberUpdates); //ccw 6 jul 2003

   for(unsigned int j=0; j<compactedHighmemUpdates.size(); j++) {
      //the layout of dyninstAPIhighmem_%08x is:
      //pageData
      //address of update
      //size of update
      // ...
      //address of update
      //size of update
      //number of updates

      startPage = compactedHighmemUpdates[j]->address - 
                  compactedHighmemUpdates[j]->address % pageSize;
      stopPage = compactedHighmemUpdates[j]->address + 
                 compactedHighmemUpdates[j]->size -
                 (compactedHighmemUpdates[j]->address + 
                  compactedHighmemUpdates[j]->size) % pageSize;

      numberUpdates = 0;
      startIndex = -1;
      stopIndex = -1;
      
      for(unsigned index = 0;index < highmem_updates.size(); index++){
         //here we ignore anything with an address of zero.
         //these can be safely deleted in writeBackElf
         if( highmem_updates[index]->address && 
             startPage <= highmem_updates[index]->address &&
             highmem_updates[index]->address  < (startPage + pageSize /*compactedHighmemUpdates[j]->sizei*/)){
            numberUpdates ++;
            stopIndex = index;
            if(startIndex == -1){
               startIndex = index;
            }
           //bperr(" HighMemUpdates address 0x%x \n", highmem_updates[index]->address );
         }
	//bperr(" high mem updates: 0x%x", highmem_updates[index]->address);
      }
      unsigned int dataSize = compactedHighmemUpdates[j]->size + 
         sizeof(unsigned int) + 
         (2*(stopIndex - startIndex + 1) /*numberUpdates*/ * sizeof(unsigned int));

	//bperr("DATASIZE: %x : %x + 4 + ( 2*(%x - %x +1) * 4)\n", dataSize, compactedHighmemUpdates[j]->size, stopIndex, startIndex);
      
      data = new char[dataSize];
      
      //fill in pageData
      readDataSpace((void*) compactedHighmemUpdates[j]->address, 
                    compactedHighmemUpdates[j]->size, data, true);
      
      unsigned int *dataPtr = 
         (unsigned int*) ( (char*) data + compactedHighmemUpdates[j]->size);

      //fill in address of update
      //fill in size of update
      for(int index = startIndex; index<=stopIndex;index++){ 

         memcpy(dataPtr, &highmem_updates[index]->address,
                sizeof(unsigned int));
         dataPtr ++;
         memcpy(dataPtr, &highmem_updates[index]->size, sizeof(unsigned int));

         dataPtr++;
         //bperr("%d J %d ADDRESS: 0x%x SIZE 0x%x\n",index, j,
         //highmem_updates[index]->address, highmem_updates[index]->size);

	
      }
      //fill in number of updates
      memcpy(dataPtr, &numberUpdates, sizeof(unsigned int));

      //bperr(" NUMBER OF UPDATES 0x%x  %d %x\n\n",numberUpdates,dataSize,dataSize);
      sprintf(name,"dyninstAPIhighmem_%08x",j);
#if defined(sparc_sun_solaris2_4) \
 || defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */
      newFile->addSection(compactedHighmemUpdates[j]->address,data ,dataSize,name,false);
#elif defined(rs6000_ibm_aix4_1)
	  sprintf(name, "dyH_%03x",j);
      newFile->addSection(&(name[0]), compactedHighmemUpdates[j]->address,compactedHighmemUpdates[j]->address,
		dataSize, (char*) data );
#endif
      
      //lastCompactedUpdateAddress = compactedHighmemUpdates[j]->address+1;
      delete [] (char*) data;
   }
   err = writeDataSpace((void*)guardFlagAddr, sizeof(unsigned int), 
                  (void*)&trampGuardValue);
   if (!err) fprintf(stderr, "%s[%d][%s]:  writeDataSpace failed\n", __FILE__, __LINE__, getThreadStr(getExecThreadID()));
        assert(err);
}

void process::saveWorldCreateDataSections(void* ptr){

#if defined(sparc_sun_solaris2_4) \
 || defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */
	writeBackElf *newFile = (writeBackElf*) ptr;
#elif defined(rs6000_ibm_aix4_1)
	writeBackXCOFF *newFile = (writeBackXCOFF*) ptr;
#endif

	char *dataUpdatesData;
	int sizeofDataUpdatesData=0;
	for(unsigned int m=0;m<dataUpdates.size();m++){
		sizeofDataUpdatesData += (sizeof(int) + sizeof(Address)); //sizeof(size) +sizeof(Address);
		sizeofDataUpdatesData += dataUpdates[m]->size;
	}


	if(dataUpdates.size() > 0) {
		dataUpdatesData = new char[sizeofDataUpdatesData+(sizeof(int) + sizeof(Address))];
		char* ptr = dataUpdatesData;
		for(unsigned int k=0;k<dataUpdates.size();k++){
			memcpy(ptr, &dataUpdates[k]->size, sizeof(int));
			ptr += sizeof(int);
			memcpy(ptr, &dataUpdates[k]->address, sizeof(Address));
			ptr+=sizeof(Address);
			memcpy(ptr, dataUpdates[k]->value, dataUpdates[k]->size);
			ptr+=dataUpdates[k]->size;
			/*fprintf(stderr," DATA UPDATE : from: %x to %x , value %x\n", dataUpdates[k]->address, dataUpdates[k]->address+ dataUpdates[k]->size, (unsigned int) dataUpdates[k]->value);*/

		}
		*(int*) ptr=0;
		ptr += sizeof(int);
		*(unsigned int*) ptr=0;
#if defined(sparc_sun_solaris2_4) \
 || defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */
		newFile->addSection(0/*lastCompactedUpdateAddress*/, dataUpdatesData, 
			sizeofDataUpdatesData + (sizeof(int) + sizeof(Address)), "dyninstAPI_data", false);
#elif defined(rs6000_ibm_aix4_1)
		newFile->addSection("dyn_dat", 0/*lastCompactedUpdateAddress*/,0,
			sizeofDataUpdatesData + (sizeof(int) + sizeof(Address)), (char*) dataUpdatesData);

#endif

		delete [] (char*) dataUpdatesData;
	}

}
#endif

#if defined(sparc_sun_solaris2_4) \
 || defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */ \
 || defined(rs6000_ibm_aix4_1)

void process::saveWorldAddSharedLibs(void *ptr){ // ccw 14 may 2002 

	int dataSize=0;
	char *data, *dataptr;
#if defined(sparc_sun_solaris2_4) \
 || defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */
	writeBackElf *newFile = (writeBackElf*) ptr;
#elif defined(rs6000_ibm_aix4_1)
	writeBackXCOFF *newFile = (writeBackXCOFF*) ptr;
#endif

	for(unsigned i=0;i<loadLibraryUpdates.size();i++){
		dataSize += loadLibraryUpdates[i].length() + 1;
	}
	dataSize++;
	data = new char[dataSize];
	dataptr = data;
	/*bperr(" dataSize: %d\n", dataSize);*/

	for(unsigned j=0;j<loadLibraryUpdates.size();j++){
		memcpy( dataptr, loadLibraryUpdates[j].c_str(), loadLibraryUpdates[j].length()); 

		/*bperr("SAVING: %s %d\n", dataptr,dataSize);*/
		dataptr += loadLibraryUpdates[j].length();
		*dataptr = '\0';
		dataptr++; 
	}
	*dataptr = '\0'; //mark the end
	if(dataSize > 1){
#if defined(sparc_sun_solaris2_4) \
 || defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */
		newFile->addSection(0, data, dataSize, "dyninstAPI_loadLib", false);
#elif  defined(rs6000_ibm_aix4_1)
		newFile->addSection("dyn_lib",0,0, dataSize, data);
#endif
	}
	delete [] data;
}

#endif


/*
 * Given an image, add all static heaps inside it
 * (DYNINSTstaticHeap...) to the buffer pool.
 */

void process::addInferiorHeap(const mapped_object *obj)
{
    //fprintf(stderr, "Adding inferior heaps in %s\n", obj->fileName().c_str());
  pdvector<heapDescriptor> infHeaps;
  /* Get a list of inferior heaps in the new image */
  if (getInfHeapList(obj, infHeaps))
    {
      /* Add the vector to the inferior heap structure */
        for (u_int j=0; j < infHeaps.size(); j++)
        {
#ifdef DEBUG
            fprintf(stderr, "Adding heap at 0x%x to 0x%x, name %s\n",
                    infHeaps[j].addr(),
                    infHeaps[j].addr() + infHeaps[j].size(),
                    infHeaps[j].name().c_str());
#endif
#if defined(arch_power)
            // MT: I've seen problems writing into a "found" heap that
            // is in the application heap (IE a dlopen'ed
            // library). Since we don't have any problems getting
            // memory there, I'm skipping any heap that is in 0x2.....
            
            if ((infHeaps[j].addr() > 0x20000000) &&
                (infHeaps[j].addr() < 0xd0000000) &&
                (infHeaps[j].type() == uncopiedHeap))
                continue;
#endif            

            heapItem *h = new heapItem (infHeaps[j].addr(), infHeaps[j].size(),
                                        infHeaps[j].type(), false);
            heap.bufferPool.push_back(h);
            heapItem *h2 = new heapItem(h);
            h2->status = HEAPfree;
            heap.heapFree.push_back(h2);
            heap.totalFreeMemAvailable += h2->length;
        }
    }
  
}


/*
 * Called to (re)initialize the static inferior heap structure.
 * To incrementally add a static inferior heap (in a dlopen()d module,
 * for example), use addInferiorHeap(image *)
 */
void process::initInferiorHeap()
{
  // (re)initialize everything 
    heap.heapActive.clear();
    heap.heapFree.resize(0);
    heap.disabledList.resize(0);
    heap.disabledListTotalMem = 0;
    heap.freed = 0;
    heap.totalFreeMemAvailable = 0;

    // first initialization: add static heaps to pool
    for (unsigned i = 0; i < mapped_objects.size(); i++) {
        addInferiorHeap(mapped_objects[i]);
    }

    heapInitialized_ = true;
}

bool process::initTrampGuard()
{
  // This is slightly funky. Dyninst does not currently support
  // multiple threads -- so it uses a single tramp guard flag, 
  // which resides in the runtime library. However, this is not
  // enough for MT paradyn. So Paradyn overrides this setting as 
  // part of its initialization.
    const pdstring vrbleName = "DYNINST_tramp_guards";
    pdvector<int_variable *> vars;
    if (!findVarsByAll(vrbleName, vars)) 
    {
        return false;
    }
    assert(vars.size() == 1);

    readDataSpace((void *) vars[0]->getAddress(), sizeof(Address), &trampGuardBase_, 
                  true);
    return true;
}

// create a new inferior heap that is a copy of src. This is used when a process
// we are tracing forks.
inferiorHeap::inferiorHeap(const inferiorHeap &src):
    heapActive(addrHash16)
{
    for (unsigned u1 = 0; u1 < src.heapFree.size(); u1++) {
      heapFree.push_back(new heapItem(src.heapFree[u1]));
    }

    pdvector<heapItem *> items = src.heapActive.values();
    for (unsigned u2 = 0; u2 < items.size(); u2++) {
      heapActive[items[u2]->addr] = new heapItem(items[u2]);
    }
    
    for (unsigned u3 = 0; u3 < src.disabledList.size(); u3++) {
      disabledList.push_back(src.disabledList[u3]);
    }

    for (unsigned u4 = 0; u4 < src.bufferPool.size(); u4++) {
      bufferPool.push_back(new heapItem(src.bufferPool[u4]));
    }

    disabledListTotalMem = src.disabledListTotalMem;
    totalFreeMemAvailable = src.totalFreeMemAvailable;
    freed = 0;
}

//
// This function will return the index corresponding to the next position
// available in heapFree.
//
int process::findFreeIndex(unsigned size, int type, Address lo, Address hi)
{
    // type is a bitmask: match on any bit in the mask
  pdvector<heapItem *> &freeList = heap.heapFree;

  int best = -1;
  for (unsigned i = 0; i < freeList.size(); i++) {
      heapItem *h = freeList[i];
      // check if free block matches allocation constraints
      // Split out to facilitate debugging
      if (h->addr >= lo &&
          (h->addr + size - 1) <= hi &&
          h->length >= size &&
          h->type & type) {
          if (best == -1)
              best = i;
          // check for better match
          if (h->length < freeList[best]->length) best = i;
      }
  }
  return best;
}  

//
// dynamic inferior heap stuff
//
#if defined(USES_DYNAMIC_INF_HEAP)
#define HEAP_DYN_BUF_SIZE (0x100000)
// "imd_rpc_ret" = Inferior Malloc Dynamic RPC RETurn structure
typedef struct {
  bool ready;
  void *result;
} imd_rpc_ret;

bool inferiorMallocCallbackFlag = false;
void process::inferiorMallocCallback(process * /*p proc*/, unsigned /* rpc_id */,
                                     void *data, void *result)
{
  global_mutex->_Lock(__FILE__, __LINE__);
  inferiorrpc_printf("%s[%d]:  inside inferior malloc callback\n", __FILE__, __LINE__);
  imd_rpc_ret *ret = (imd_rpc_ret *)data;
  ret->result = result;
  ret->ready = true;
  inferiorMallocCallbackFlag = true;
  global_mutex->_Unlock(__FILE__, __LINE__);
}

void alignUp(int &val, int align)
{
  assert(val >= 0);
  assert(align >= 0);

  if (val % align != 0) {
    val = ((val / align) + 1) * align;
  }
}

// dynamically allocate a new inferior heap segment using inferiorRPC
void process::inferiorMallocDynamic(int size, Address lo, Address hi)
{
   /* 03/07/2001 - Jeffrey Shergalis TODO: This code was placed to prevent
    * the infinite recursion on the call to inferiorMallocDynamic,
    * unfortunately it makes paradyn break on Irix, temporarily fixed by the
    * #if !defined(mips..., but should be properly fixed in the future, just
    * no time now
    */
   inferiorrpc_printf("%s[%d]:  welcome to inferiorMallocDynamic\n", __FILE__, __LINE__);
#if !defined(mips_sgi_irix6_4)
  // Fun (not) case: there's no space for the RPC to execute.
  // It'll call inferiorMalloc, which will call inferiorMallocDynamic...
  // Avoid this with a static bool.
  if (inInferiorMallocDynamic) {
      fprintf(stderr, "%s[%d]:  recursion guard\n", __FILE__, __LINE__);
      return;
  }
  inInferiorMallocDynamic = true;
#endif

  // word-align buffer size 
  // (see "DYNINSTheap_align" in rtinst/src/RTheap-<os>.c)
  alignUp(size, 4);
  // build AstNode for "DYNINSTos_malloc" call
  pdstring callee = "DYNINSTos_malloc";
  pdvector<AstNode*> args(3);
  args[0] = new AstNode(AstNode::Constant, (void *)(Address)size);
  args[1] = new AstNode(AstNode::Constant, (void *)lo);
  args[2] = new AstNode(AstNode::Constant, (void *)hi);
  AstNode *code = new AstNode(callee, args);
  removeAst(args[0]);
  removeAst(args[1]);
  removeAst(args[2]);

  // issue RPC and wait for result
  imd_rpc_ret ret = { false, NULL };

 
  /* set lowmem to ensure there is space for inferior malloc */
  getRpcMgr()->postRPCtoDo(code, true, // noCost
                           &inferiorMallocCallback, &ret, 
                           true, // But use reserved memory
                           NULL, NULL); // process-wide
  bool wasRunning = (status() == running);

  // Specify that we want to wait for a RPCDone event
  eventType res = evtUndefined;

  inferiorMallocCallbackFlag = false;
     inferiorrpc_printf("%s[%d]:  waiting for rpc completion\n", FILE__, __LINE__);
  do {
     getRpcMgr()->launchRPCs(wasRunning);
     getMailbox()->executeCallbacks(FILE__, __LINE__);

     if(hasExited()) {
        fprintf(stderr, "%s[%d]:  BAD NEWS, process has exited\n", __FILE__, __LINE__);
        return;
     }
    if (inferiorMallocCallbackFlag) {
       break;
     }

     inferiorrpc_printf("%s[%d][%s]:  before wait for RPCDone, status == running is %s\n", 
                        FILE__, __LINE__, getThreadStr(getExecThreadID()), 
                        status() == running ? "true" : "false");

     res = getSH()->waitForEvent(evtRPCSignal, this, NULL /*lwp*/, statusRPCDone);
     getMailbox()->executeCallbacks(FILE__, __LINE__);
   } while (res != evtRPCSignal); // Loop until callback has fired.

  inferiorMallocCallbackFlag = false;

   switch ((int)(Address)ret.result) {
     case 0:
#ifdef DEBUG
        sprintf(errorLine, "DYNINSTos_malloc() failed\n");
        logLine(errorLine);
#endif
        break;
     case -1:
        // TODO: assert?
        sprintf(errorLine, "DYNINSTos_malloc(): unaligned buffer size\n");
        logLine(errorLine);
        break;
     default:
        // add new segment to buffer pool
         // FIXME
#if defined(os_aix)
         // for save the world...
        heapItem *h = new heapItem((Address)ret.result, size, dataHeap, true,
                                   HEAPfree);
#else
        heapItem *h = new heapItem((Address)ret.result, size, anyHeap, true,
                                   HEAPfree);
#endif


        heap.bufferPool.push_back(h);
        // add new segment to free list
        heapItem *h2 = new heapItem(h);
        heap.heapFree.push_back(h2);
        break;
   }
   
   /* 03/07/2001 - Jeffrey Shergalis
    * Part of the above #if !defined(mips... patch for the recursion problem
    * TODO: Need a better solution
    */
#if !defined(mips_sgi_irix6_4)
   inInferiorMallocDynamic = false;
#endif
}
#endif /* USES_DYNAMIC_INF_HEAP */

const Address ADDRESS_LO = ((Address)0);
const Address ADDRESS_HI = ((Address)~((Address)0));
//unsigned int totalSizeAlloc = 0;

Address process::inferiorMalloc(unsigned size, inferiorHeapType type, 
				Address near_, bool *err)
{
   inferiorHeap *hp = &heap;
   if (err) *err = false;
   assert(size > 0);
   
   // allocation range
   Address lo = ADDRESS_LO; // Should get reset to a more reasonable value
   Address hi = ADDRESS_HI; // Should get reset to a more reasonable value
   
#if defined(USES_DYNAMIC_INF_HEAP)
   inferiorMallocAlign(size); // align size
   // Set the lo/hi constraints (if necessary)
   inferiorMallocConstraints(near_, lo, hi, type);
#else
   /* align to cache line size (32 bytes on SPARC) */
   size = (size + 0x1f) & ~0x1f; 
#endif /* USES_DYNAMIC_INF_HEAP */


   // find free memory block (7 attempts)
   // attempt 0: as is
   // attempt 1: deferred free, compact free blocks
   // attempt 2: allocate new segment (1 MB, constrained)
   // attempt 3: allocate new segment (sized, constrained)
   // attempt 4: remove range constraints
   // attempt 5: allocate new segment (1 MB, unconstrained)
   // attempt 6: allocate new segment (sized, unconstrained)
   // attempt 7: deferred free, compact free blocks (why again?)
   int freeIndex = -1;
   int ntry = 0;
   for (ntry = 0; freeIndex == -1; ntry++) {
      switch(ntry) {
	case 0: // as is
	   break;
#if defined(USES_DYNAMIC_INF_HEAP)
	case 1: // compact free blocks
	  gcInstrumentation();
	  inferiorFreeCompact(hp);
	  break;
	case 2: // allocate new segment (1MB, constrained)
        inferiorMallocDynamic(HEAP_DYN_BUF_SIZE, lo, hi);
	   break;
	case 3: // allocate new segment (sized, constrained)
	   inferiorMallocDynamic(size, lo, hi);
	   break;
	case 4: // remove range constraints
	   lo = ADDRESS_LO;
	   hi = ADDRESS_HI;
	   if (err) {
              fprintf(stderr, "%s[%d]: ERROR!\n", __FILE__, __LINE__);
	      *err = true;
	   }
	   break;
	case 5: // allocate new segment (1MB, unconstrained)
	   inferiorMallocDynamic(HEAP_DYN_BUF_SIZE, lo, hi);
	   break;
	case 6: // allocate new segment (sized, unconstrained)
	   inferiorMallocDynamic(size, lo, hi);
	   break;
	case 7: // deferred free, compact free blocks
	   inferiorFreeCompact(hp);
	   break;
#else /* !(USES_DYNAMIC_INF_HEAP) */
	case 1: // deferred free, compact free blocks
	   inferiorFreeCompact(hp);
	   break;
#endif /* USES_DYNAMIC_INF_HEAP */
	   
	default: // error - out of memory
	   sprintf(errorLine, "***** Inferior heap overflow: %d bytes "
		   "freed, %d bytes requested \n", hp->freed, size);
	   logLine(errorLine);
	   showErrorCallback(66, (const char *) errorLine);    
              fprintf(stderr,"%s[%d]: ERROR!\n", __FILE__, __LINE__);
#if defined(BPATCH_LIBRARY)
	   return(0);
#else
	   P__exit(-1);
#endif
      }
      freeIndex = findFreeIndex(size, type, lo, hi);
//	bperr("  type %x",type);
   }

   // adjust active and free lists
   assert(freeIndex != -1);
   heapItem *h = hp->heapFree[freeIndex];
   assert(h);

   // remove allocated buffer from free list
   if (h->length != size) {
      // size mismatch: put remainder of block on free list
      heapItem *rem = new heapItem(h);
      rem->addr += size;
      rem->length -= size;
      hp->heapFree[freeIndex] = rem;
   } else {
      // size match: remove entire block from free list
      unsigned last = hp->heapFree.size();
      hp->heapFree[freeIndex] = hp->heapFree[last-1];
      hp->heapFree.resize(last-1);
   }
   // add allocated block to active list
   h->length = size;
   h->status = HEAPallocated;
   hp->heapActive[h->addr] = h;
   // bookkeeping
   hp->totalFreeMemAvailable -= size;
   assert(h->addr);
   
   // ccw: 28 oct 2001
   // create imageUpdate here:
   // imageUpdate(h->addr,size)
   
#ifdef BPATCH_LIBRARY
//	if( h->addr > 0xd0000000){
//		bperr(" \n ALLOCATION: %lx %lx ntry: %d\n", h->addr, size,ntry);
//		fflush(stdout);
//	}
#if defined(sparc_sun_solaris2_4) \
 || defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */ \
 || defined(rs6000_ibm_aix4_1)
   if(collectSaveWorldData){
      
#if defined(sparc_sun_solaris2_4)
      if(h->addr < 0xF0000000)
#elif defined(i386_unknown_linux2_0) \
   || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */
      if(h->addr == 0 ) //< 0x40000000) //ccw TEST TEST TEST
#elif defined(rs6000_ibm_aix4_1)
	if(h->addr < 0x20000000)
#endif	
      {
	 imageUpdate *imagePatch=new imageUpdate; 
	 imagePatch->address = h->addr;
	 imagePatch->size = size;
	 imageUpdates.push_back(imagePatch);
	 //totalSizeAlloc += size;
	 //bperr(" PUSHBACK %x %x --- \n", imagePatch->address, imagePatch->size); 		
	 //fprintf(stderr," PUSHBACK %x %x --- \n", imagePatch->address, imagePatch->size); 		
      } else {
	 //	totalSizeAlloc += size;
	 //fprintf(stderr,"HIGHMEM UPDATE %x %x \n", h->addr, size);
	 imageUpdate *imagePatch=new imageUpdate;
	 imagePatch->address = h->addr;
	 imagePatch->size = size;
	 highmemUpdates.push_back(imagePatch);
	 //bperr(" PUSHBACK %x %x\n", imagePatch->address, imagePatch->size);
      }
      //fflush(stdout);
   }
#endif
#endif
   return(h->addr);
}

/* returns true if memory was allocated for a variable starting at address
   "block", otherwise returns false
*/
bool isInferiorAllocated(process *p, Address block) {
  heapItem *h = NULL;  
  inferiorHeap *hp = &p->heap;
  return hp->heapActive.find(block, h);
}

void process::inferiorFree(Address block)
{
  inferiorHeap *hp = &heap;

  // find block on active list
  heapItem *h = NULL;  
  if (!hp->heapActive.find(block, h)) {
      // We can do this if we're at process teardown.
    return;
  }
  assert(h);

  // Remove from the active list
  hp->heapActive.undef(block);

  // Add to the free list
  h->status = HEAPfree;
  hp->heapFree.push_back(h);
  hp->totalFreeMemAvailable += h->length;
  hp->freed += h->length;
}
 

// Cleanup the process object. Delete all sub-objects to ensure we aren't 
// leaking memory and prepare for new versions. Useful when the process exits
// (and the process object is deleted) and when it execs

void process::deleteProcess() {

  // A lot of the behavior here is keyed off the current process status....
  // if it is exited we'll delete things without performing any operations
  // on the process. Otherwise we'll attempt to reverse behavior we've made.
    // For platforms that don't like repeated use of for (unsigned i...)
    unsigned iter = 0;

  // We may call this function multiple times... once when the process exits,
  // once when we delete the process object.
  if (status() == deleted) return;
    
  // If this assert fires check whether we're setting the status vrble correctly
  // before calling this function
  assert(!isAttached() || !reachedBootstrapState(bootstrapped_bs) || execing());

  // pid remains untouched
  // parent remains untouched
  for (iter = 0; iter < mapped_objects.size(); iter++) 
      delete mapped_objects[iter];
  mapped_objects.clear();
  runtime_lib = NULL;

  // Signal handlers...
  signalHandlerLocations_.clear();

  // creationMechanism_ remains untouched
  // stateWhenAttached_ remains untouched
  main_function = NULL;

  if (dyn) {
      delete dyn;
      dyn = NULL;
  }

  // Delete the thread and lwp structures
  dictionary_hash_iter<unsigned, dyn_lwp *> lwp_iter(real_lwps);
  dyn_lwp *lwp;
  unsigned index;
  
  // This differs on exec; we need to kill all but the representative LWP
  // Duplicate code for ease of reading

  if (execing()) {
      while (lwp_iter.next(index, lwp)) {
          if (process::IndependentLwpControl() &&
              (index == (unsigned)getPid())) {
              // Keep this around instead of nuking it -- this is our
              // handle to the "real" LWP
              representativeLWP = lwp;
          }
          else
              delete lwp;
      }
      real_lwps.clear();
      
      // DO NOT DELETE THE REPRESENTATIVE LWP

  }
  else { 
      while (lwp_iter.next(index, lwp)) {
          deleteLWP(lwp);
      }
      real_lwps.clear();
      
      if (representativeLWP) {
          delete representativeLWP;
          representativeLWP = NULL;
      }
  }
  
  // Blow away threads; we'll recreate later
  for (unsigned thr_iter = 0; thr_iter < threads.size(); thr_iter++) {
      delete threads[thr_iter];
  }
  threads.clear();


  deferredContinueProc = false;
  previousSignalAddr_ = 0;
  continueAfterNextStop_ = false;
  suppressCont_ = false;
  
  // Don't touch exec; statically allocated anyway.

  if (theRpcMgr) {
      delete theRpcMgr;
  }
  theRpcMgr = NULL;

  // Skipping saveTheWorld; don't know what to do with it.

  trampTrapMapping.clear();

  // Each instPoint may have several instances -- this gets instances and makes
  // a unique set
  std::set<instPoint *> allInstPoints;
  dictionary_hash_iter<Address, instPoint *> ipIter(instPMapping_);
  for (; ipIter; ipIter++) {
      instPoint *p = ipIter.currval();
      allInstPoints.insert(p);
  }

  // And this deletes the set.
  for (std::set<instPoint *>::iterator ip = allInstPoints.begin();
       ip != allInstPoints.end();
       ip++) {
      delete (*ip);
  }
  instPMapping_.clear();
  
  codeRangesByAddr_.clear();
  
  // Iterate and clear? instPoint deletion should handle it.
  // What about replaced functions?
  modifiedAreas_.clear();
  multiTrampDict.clear();

  // Blow away the replacedFunctionCalls list; codeGens are taken
  // care of statically and will deallocate
  dictionary_hash_iter<Address, replacedFunctionCall *> rfcIter(replacedFunctionCalls_);  
  for (; rfcIter; rfcIter++) {
      replacedFunctionCall *rfcVal = rfcIter.currval();
      assert(rfcVal->callAddr == rfcIter.currkey());
      assert(rfcVal);
      free(rfcVal);
  }
  replacedFunctionCalls_.clear();

  codeSections_.clear();
  dataSections_.clear();

  dyninstlib_brk_addr = 0;
  main_brk_addr = 0;

  heapInitialized_ = false;
  heap.clear();
  inInferiorMallocDynamic = false;

  // Get rid of our syscall tracing.
  if (tracedSyscalls_) {
      delete tracedSyscalls_;
      tracedSyscalls_ = NULL;
  }

  for (iter = 0; iter < syscallTraps_.size(); iter++) 
      delete syscallTraps_[iter];

  for (iter = 0; iter < tracingRequests.size(); iter++)
      delete tracingRequests[iter];
  tracingRequests.clear();

  // By definition, these are dangling.
  for (iter = 0; iter < pendingGCInstrumentation.size(); iter++) {
      delete pendingGCInstrumentation[iter];
  }
  pendingGCInstrumentation.clear();

  cumulativeObsCost = 0;
  lastObsCostLow = 0;
  costAddr_ = 0;
  threadIndexAddr = 0;
  trampGuardBase_ = 0;

#if defined(os_linux)
  vsyscall_start_ = 0;
  vsyscall_end_ = 0;
  vsyscall_text_ = 0;
  vsyscall_data_ = 0;
#endif
  
  set_status(deleted);
}

//
// cleanup when a process is deleted. Assumed we are detached or the process is exited.
//
process::~process()
{
    // Failed creation... nothing is here yet
    if (!reachedBootstrapState(initialized_bs))
        return;

    // We require explicit detaching if the process still exists.
    // On the other hand, if it never started...
    if (reachedBootstrapState(bootstrapped_bs))
        assert(!isAttached());

#if defined( ia64_unknown_linux2_4 )
	if( unwindProcessArg != NULL ) { getDBI()->UPTdestroy( unwindProcessArg ); }
	if( unwindAddressSpace != NULL ) { getDBI()->destroyUnwindAddressSpace( unwindAddressSpace ); }
#endif
    
    // Most of the deletion is encapsulated in deleteProcess
    deleteProcess();

    // We used to delete the particular process, but this created no end of problems
    // with people storing pointers into particular indices. We set the pointer to NULL.
    for (unsigned lcv=0; lcv < processVec.size(); lcv++) {
        if (processVec[lcv] == this) {
            processVec[lcv] = NULL;
        }        
    }

#if defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */
    cleanupVsysinfo(getVsyscallData());
#endif

}

// Default process class constructor. This handles both create,
// attach, and attachToCreated cases. We then call an auxiliary
// function (which can return an error value) to handle specific
// cases.
process::process(int ipid) :
    cached_result(not_cached), // MOVE ME
    pid(ipid),
    parent(NULL),
    runtime_lib(NULL),
    creationMechanism_(unknown_cm),
    stateWhenAttached_(unknown_ps),
    main_function(NULL),
    dyn(NULL),
    representativeLWP(NULL),
    real_lwps(CThash),
    max_number_of_threads(MAX_THREADS),
    deferredContinueProc(false),
    previousSignalAddr_(0),
    continueAfterNextStop_(false),
    suppressCont_(false),
    status_(neonatal),
    nextTrapIsExec(false),
    inExec_(false),
    theRpcMgr(NULL),
    collectSaveWorldData(true),
    requestTextMiniTramp(false),
    traceLink(0),
    trampTrapMapping(addrHash4),
    instPMapping_(addrHash4),
    multiTrampDict(intHash),
    replacedFunctionCalls_(addrHash4),
    bootstrapState(unstarted_bs),
    savedRegs(NULL),
    dyninstlib_brk_addr(0),
    main_brk_addr(0),
    runProcessAfterInit(false),
#if defined(os_windows)
    processHandle_(INVALID_HANDLE_VALUE),
    mainFileHandle_(INVALID_HANDLE_VALUE),
#endif
    splitHeaps(false),
    heapInitialized_(false),
    inInferiorMallocDynamic(false),
    tracedSyscalls_(NULL),
    cumulativeObsCost(0),
    lastObsCostLow(0),
    costAddr_(0),
    threadIndexAddr(0),
    trampGuardBase_(0)
#if defined(arch_ia64)
    , unwindAddressSpace(NULL) // Automatically created in getActiveFrame
    , unwindProcessArg(NULL) // And this one too
#endif
#if defined(os_linux)
    , vsyscall_start_(0)
    , vsyscall_end_(0)
    , vsyscall_text_(0)
    , vsyscall_data_(NULL)
#endif
{

    // Let's try to profile memory usage
#if defined(PROFILE_MEM_USAGE)
   void *mem_usage = sbrk(0);
   fprintf(stderr, "Process creation: sbrk %p\n", mem_usage);
#endif

    theRpcMgr = new rpcMgr(this);    
    dyn = new dynamic_linking(this);
    createRepresentativeLWP();

    // Not sure we need this anymore... on AIX you can run code
    // anywhere, so allocating by address _should_ be okay.
#if defined(rs6000_ibm_aix3_2) \
 || defined(rs6000_ibm_aix4_1) \
 || defined(alpha_dec_osf4_0)
    splitHeaps = true;
#endif

}

//
// Process "normal" (non-attach, non-fork) ctor equivalent, for when a
// new process is fired up by paradynd itself.  This ctor. is also
// used in the case that the application has been fired up by another
// process and stopped just after executing the execv() system call.
// In the "normal" case, the parameter iTraceLink will be a
// non-negative value which corresponds to the pipe descriptor of the
// trace pipe used between the paradynd and the application. In the
// later case, iTraceLink will be -1. This will be posteriorly used to
// properly initialize the createdViaAttachToCreated flag. - Ana
//


bool process::setupCreated(int iTraceLink) {
    traceLink = iTraceLink; // notice that tracelink will be -1 in the unique
    // case called "AttachToCreated" - Ana 
    // PARADYN ONLY

    creationMechanism_ = created_cm;
    
#if !defined(BPATCH_LIBRARY)
    if (iTraceLink == -1 ) {
        fprintf(stderr, "%s[%d]:  setting attachedToCreated\n", FILE__, __LINE__);
        creationMechanism_ = attachedToCreated_cm;
    }
#endif

    // Post-setup state variables
    runProcessAfterInit = false;
    stateWhenAttached_ = stopped; 
    
    startup_printf("Creation method: attaching to process\n");
    // attach to the child process (machine-specific implementation)
    if (!attach()) { // error check?
        status_ = detached;
         pdstring msg = pdstring("Warning: unable to attach to specified process :")
            + pdstring(pid);
        showErrorCallback(26, msg.c_str());
        return false;
    }
    startup_printf("Creation method: returning\n");
    return true;
}
    

// Attach version of the above: no trace pipe, but we assume that
// main() has been reached and passed. Someday we could unify the two
// if someone has a good way of saying "has main been reached".
bool process::setupAttached() {
    creationMechanism_ = attached_cm;
    // We're post-main... run the bootstrapState forward

#if !defined(os_windows)
    bootstrapState = initialized_bs;
#else
    // We need to wait for the CREATE_PROCESS debug event.
    // Set to "begun" here, and fix up in the signal loop
    bootstrapState = begun_bs;
#endif

   traceLink = -1; // will be set later, when the appl runs DYNINSTinit

   startup_printf("Attach method: attaching to process\n");

   // It is assumed that a call to attach() doesn't affect the running status
   // of the process.  But, unfortunately, some platforms may barf if the
   // running status is anything except paused. (How to deal with this?)
   // Note that solaris in particular seems able to attach even if the process
   // is running.
   if (!attach()) {
       status_ = detached;
       
      pdstring msg = pdstring("Warning: unable to attach to specified process: ")
                   + pdstring(pid);
      showErrorCallback(26, msg.c_str());
      return false;
   }

   startup_printf("[%d]: attached, getting current process state\n", getPid());

   // Record what the process was doing when we attached, for possible
   // use later.
   if (isRunning_()) {
       startup_printf("[%d]: process running when attached, pausing...\n", getPid());
       stateWhenAttached_ = running; 
       set_status(running);
       if (!pause())
           return false;
   }
   else {
       startup_printf("[%d]: attached to previously paused process\n", getPid());
       stateWhenAttached_ = stopped;
       set_status(stopped);
   }
   startup_printf("[%d]: setupAttached returning true\n", getPid());

   assert(status() == stopped);
   return true;
}

int HACKSTATUS = 0;

bool process::prepareExec() {
    ///////////////////////////// CONSTRUCTION STAGE ///////////////////////////
    // For all intents and purposes: a new id.
    // However, we don't want to make a new object since all sorts
    // of people have a pointer to this object. We could index by PID,
    // which would allow us to swap out processes; but hey.

    // We should be attached to the process

    // setupExec must get us to the point of putting a trap at the
    // beginning of main. We then continue. We might get called again,
    // or we might go into loading the dyninst lib; impossible to
    // tell.
    //

#if defined(AIX_PROC)
    // AIX oddly detaches from the process... fix that here
    // Actually, it looks like only the as FD is closed (probably because
    // the file it refers to is gone). Reopen.
   getRepresentativeLWP()->reopen_fds();
#endif

    // Revert the bootstrap state
    bootstrapState = attached_bs;

    // First, duplicate constructor.

    assert(theRpcMgr == NULL);
    assert(dyn == NULL);
    theRpcMgr = new rpcMgr(this);
    dyn = new dynamic_linking(this);
    int status = 0;

    // False: not waitin' for a signal (theoretically, we already got
    // it when we attached)
    fileDescriptor desc;
    if (!getExecFileDescriptor(execFilePath, 
                               pid,
                               false,
                               status, 
                               desc)) {
        cerr << "Failed to find exec descriptor" << endl;
        return false;
    }
    if (!setAOut(desc)) {
        return false;
    }

    // Probably not going to find anything (as we haven't loaded the
    // RT lib yet, and that's where most of the space is). However,
    // any shared object added after this point will have infHeaps
    // auto-added.
    startup_printf("Initializing vector heap\n");
    initInferiorHeap();

    // Now from setupGeneral...
    createInitialThread();

    // Status: stopped.
    set_status(stopped);

    // Annoying; most of our initialization code is in unix.C, and it
    // knows how to get us to main. Problem is, it triggers on a trap...
    // and guess what we just consumed. So replicate it manually.
    setBootstrapState(begun_bs);
    insertTrapAtEntryPointOfMain();
    continueProc();

    return true;
}

bool process::finishExec() {
    startup_printf("%s[%d]:  about toloadDyninstLib\n", FILE__, __LINE__);
    bool res = loadDyninstLib();
    if (!res)
        return false;
    
    getMailbox()->executeCallbacks(FILE__, __LINE__);
    while(!reachedBootstrapState(bootstrapped_bs)) {
        // We're waiting for something... so wait
        // true: block until a signal is received (efficiency)
        if(hasExited()) {
            return false;
        }
        fprintf(stderr, "%s[%d][%s]:  before waitForEvent(evtProcessInitDone)\n", 
                FILE__, __LINE__, getThreadStr(getExecThreadID()));

        getSH()->waitForEvent(evtProcessInitDone);
        getMailbox()->executeCallbacks(FILE__, __LINE__);
    }
    
    if(process::IndependentLwpControl())
        independentLwpControlInit();
    
    set_status(stopped); // was 'exited'
    
    inExec_ = false;
    BPatch::bpatch->registerExec(this);

    return true;
}

bool process::setupFork() {
    
    assert(parent);
    assert(parent->status() == stopped);

    // Do stuff....

    // Copy: 
    //   all mapped objects
    //   dynamic object tracer
    //   all threads
    //   all lwps
    //   rpc manager
    //   all vector heaps
    //   all defined instPoints
    //   all multiTramps
    //    ... and baseTramps, relocatedInstructions, trampEnds, miniTramps
    //   installed instrumentation
    //    ... including system call handling
    //   process state
    //   

    // Mapped objects first
    for (unsigned i = 0; i < parent->mapped_objects.size(); i++) {
        mapped_object *par_obj = parent->mapped_objects[i];
        mapped_object *child_obj = new mapped_object(par_obj, this);
        if (!child_obj) {
            delete child_obj;
            return false;
        }

        mapped_objects.push_back(child_obj);
        addCodeRange(child_obj);

        if ((par_obj->fileName() == dyninstRT_name) ||
            (par_obj->fullName() == dyninstRT_name))
            runtime_lib = child_obj;

        // This clones funcs, which then clone instPoints, which then 
        // clone baseTramps, which then clones miniTramps.
    }
    // And the main func and dyninst RT lib
    if (!setMainFunction())
        return false;
    if (parent->runtime_lib) {
        // This should be set by now...
        assert(runtime_lib);
    }
    
    /////////////////////////
    // Threads & LWPs
    /////////////////////////
    createRepresentativeLWP();
    if (!attach()) {
        status_ = detached;
        showErrorCallback(69, "Error in fork: cannot attach to child process");
        return false;
    }

    if(process::IndependentLwpControl())
        independentLwpControlInit();

    /////////////////////////
    // RPC manager
    /////////////////////////
    
    theRpcMgr = new rpcMgr(parent->theRpcMgr, this);
    assert(theRpcMgr);

    /////////////////////////
    // Find new threads
    /////////////////////////    

    recognize_threads(parent);


    /////////////////////////
    // Inferior heap
    /////////////////////////
    
    heap = inferiorHeap(parent->heap);

    /////////////////////////
    // Instrumentation (multiTramps on down)
    /////////////////////////

    dictionary_hash_iter<int, multiTramp *> multiTrampIter(parent->multiTrampDict);
    int mID;
    multiTramp *mTramp;
    for (; multiTrampIter; multiTrampIter++) {
        mID = multiTrampIter.currkey();
        mTramp = multiTrampIter.currval();
        assert(mTramp);
        multiTramp *newMulti = new multiTramp(mTramp, this);
        multiTrampDict[mID] = newMulti;
        addMultiTramp(newMulti);
    }
    // That will also create all baseTramps, miniTramps, ...

    // Copy the replacedFunctionCalls...
    dictionary_hash_iter<Address, replacedFunctionCall *> rfcIter(replacedFunctionCalls_);  
    Address rfcKey;
    replacedFunctionCall *rfcVal;
    for (; rfcIter; rfcIter++) {
        rfcKey = rfcIter.currkey();
        assert(rfcKey);
        rfcVal = rfcIter.currval();
        assert(rfcVal);
        assert(rfcVal->callAddr == rfcKey);
        // Mmm copy constructor
        replacedFunctionCall *newRFC = new replacedFunctionCall(*rfcVal);
        replacedFunctionCalls_[rfcKey] = newRFC;
    }

#if 0
    // We create instPoints as part of copying functions

    std::set<instPoint *> allInstPoints;
    dictionary_hash_iter<Address, instPoint *> ipIter(parent->instPMapping_);
    for (; ipIter; ipIter++) {
        instPoint *p = ipIter.currval();
        allInstPoints.insert(p);
    }
    
    for (std::set<instPoint *>::iterator ip = allInstPoints.begin();
         ip != allInstPoints.end();
         ip++) {
        instPoint *newIP = new instPoint(*ip, this);
        assert(newIP);
        // Adds to instPMapping_
    }
#endif

    // Tag the garbage collection list...
    for (unsigned ii = 0; ii < parent->pendingGCInstrumentation.size(); ii++) {
        // TODO. For now we'll just "leak"
    }

    // Now that we have instPoints, we can create the (possibly) instrumentation-
    // based tracing code

    /////////////////////////
    // Dynamic tracer
    /////////////////////////

    dyn = new dynamic_linking(parent->dyn, this);
    assert(dyn);

    /////////////////////////
    // Syscall tracer
    /////////////////////////

    tracedSyscalls_ = new syscallNotification(parent->tracedSyscalls_, this);

    // Copy signal handlers

    pdvector<codeRange *> sigHandlers;
    parent->signalHandlerLocations_.elements(sigHandlers);
    for (unsigned iii = 0; iii < sigHandlers.size(); iii++) {
        signal_handler_location *oldSig = dynamic_cast<signal_handler_location *>(sigHandlers[iii]);
        assert(oldSig);
        signal_handler_location *newSig = new signal_handler_location(*oldSig);
        signalHandlerLocations_.insert(newSig);
    }


#if defined(os_aix)
    // AIX doesn't copy memory past the ends of text segments, so we
    // do it manually here
    copyDanglingMemory(const_cast<process *>(parent));
#endif

    /////////////////////////
    // Process vector
    /////////////////////////

    processVec.push_back(this);
    activeProcesses++;
    return true;
}


unsigned process::getAddressWidth() { 
    if (mapped_objects.size() > 0) 
        return mapped_objects[0]->parse_img()->getObject().getAddressWidth(); 
    // We can call this before we've attached.. 
    return 4;
}

bool process::setAOut(fileDescriptor &desc) {
    assert(reachedBootstrapState(attached_bs));
    assert(mapped_objects.size() == 0);
    mapped_object *aout = mapped_object::createMappedObject(desc, this);
    if (!aout)
        return false;
    
    mapped_objects.push_back(aout);
    addCodeRange(aout);

    findSignalHandler(aout);

    // Find main
    return setMainFunction();
}

// Here's the list of functions to look for:
#define NUMBER_OF_MAIN_POSSIBILITIES 7
char main_function_names[NUMBER_OF_MAIN_POSSIBILITIES][20] = {
    "main",
    "DYNINST_pltMain",
    "_main",
    "WinMain",
    "_WinMain",
    "wWinMain",
    "_wWinMain"};

bool process::setMainFunction() {
    assert(!main_function);
    
    for (unsigned i = 0; i < NUMBER_OF_MAIN_POSSIBILITIES; i++) {
        main_function = findOnlyOneFunction(main_function_names[i]);
        if (main_function) break;
    }

    assert(main_function);
    return true;
}

bool process::setupGeneral() {
    // Need to have a.out at this point
    assert(mapped_objects.size() > 0);

    if (reachedBootstrapState(bootstrapped_bs)) 
        return true;

    // We should be paused; be sure.
    pause();
    
    // In the ST case, threads[0] (the only one) is effectively
    // a pass-through for process operations. In MT, it's the
    // main thread of the process and is handled correctly

    startup_printf("Creating initial thread...\n");

    createInitialThread();

    // Probably not going to find anything (as we haven't loaded the
    // RT lib yet, and that's where most of the space is). However,
    // any shared object added after this point will have infHeaps
    // auto-added.
    startup_printf("Initializing vector heap\n");
    initInferiorHeap();

#if defined(os_aix)
    if (HACKSTATUS == SIGTRAP) {
      EventRecord ev;
      ev.proc = this;
      ev.type = evtSignalled;
      ev.what = HACKSTATUS;
      ev.info = 0;
      //getSH()->handleSigTrap(ev);
      SignalHandlerUnix::handleSigTrap(ev);
    }
#endif
        

    startup_printf("%s[%d]: Loading DYNINST lib...\n", FILE__, __LINE__);
    // TODO: loadDyninstLib actually embeds a lot of startup material;
    // should move it up to this function to make things more obvious.
    bool res = loadDyninstLib();
    if(res == false) {
        return false;
    }
    startup_printf("Waiting for bootstrapped state...\n");
    while (!reachedBootstrapState(bootstrapped_bs)) {
       // We're waiting for something... so wait
       // true: block until a signal is received (efficiency)
       if(hasExited()) {
           return false;
       }
       startup_printf("Checking for process event...\n");
       getSH()->waitForEvent(evtProcessInitDone);
       getMailbox()->executeCallbacks(FILE__, __LINE__);
    }

    if(process::IndependentLwpControl())
        independentLwpControlInit();
    
    initTramps(multithread_capable());

    return true;
}

//
// Process "fork" ctor, for when a process which is already being monitored by
// paradynd executes the fork syscall.
//
// Needs to strictly duplicate all process information; this is a _lot_ of work.

process::process(const process *parentProc, int childPid, int childTrace_fd) : 
    cached_result(parentProc->cached_result), // MOVE ME
    pid(childPid),
    parent(parentProc),
    runtime_lib(NULL), // Set later
    dyninstRT_name(parentProc->dyninstRT_name),
    creationMechanism_(parentProc->creationMechanism_),
    stateWhenAttached_(parentProc->stateWhenAttached_),
    main_function(NULL), // Set later
    dyn(NULL),  // Set later
    representativeLWP(NULL), // Set later
    real_lwps(CThash),
    max_number_of_threads(parentProc->max_number_of_threads),
    deferredContinueProc(parentProc->deferredContinueProc),
    previousSignalAddr_(parentProc->previousSignalAddr_),
    continueAfterNextStop_(parentProc->continueAfterNextStop_),
    suppressCont_(parentProc->suppressCont_),
    status_(parentProc->status_),
    nextTrapIsExec(parentProc->nextTrapIsExec),
    inExec_(parentProc->inExec_),
    theRpcMgr(NULL), // Set later
    collectSaveWorldData(parentProc->collectSaveWorldData),
    requestTextMiniTramp(parentProc->requestTextMiniTramp),
    traceLink(childTrace_fd),
    trampTrapMapping(parentProc->trampTrapMapping),
    instPMapping_(addrHash4), // Later
    multiTrampDict(intHash), // Later
    replacedFunctionCalls_(addrHash4), // Also later
    bootstrapState(parentProc->bootstrapState),
    savedRegs(NULL), // Later
    dyninstlib_brk_addr(parentProc->dyninstlib_brk_addr),
    main_brk_addr(parentProc->main_brk_addr),
    runProcessAfterInit(parentProc->runProcessAfterInit),
    splitHeaps(parentProc->splitHeaps),
    heapInitialized_(parentProc->heapInitialized_),
    inInferiorMallocDynamic(parentProc->inInferiorMallocDynamic),
    tracedSyscalls_(NULL),  // Later
    cumulativeObsCost(parentProc->cumulativeObsCost),
    lastObsCostLow(parentProc->lastObsCostLow),
    costAddr_(parentProc->costAddr_),
    threadIndexAddr(parentProc->threadIndexAddr),
    trampGuardBase_(parentProc->trampGuardBase_)
#if defined(arch_ia64)
    , unwindAddressSpace(NULL) // Recreated automatically in getActiveFrame
    , unwindProcessArg(NULL) // And this
#endif
#if defined(os_linux)
    , vsyscall_start_(parentProc->vsyscall_start_)
    , vsyscall_end_(parentProc->vsyscall_end_)
    , vsyscall_text_(parentProc->vsyscall_text_)
    , vsyscall_data_(parentProc->vsyscall_data_)
#endif
{
}

static void cleanupBPatchHandle(int pid)
{
   BPatch::bpatch->unRegisterProcess(pid);
}


/*
 * Create a new instance of the named process.  Read the symbols and start
 *   the program
 */
process *ll_createProcess(const pdstring File, pdvector<pdstring> *argv,
			  pdvector<pdstring> *envp, const pdstring dir = "",
			  int stdin_fd=0, int stdout_fd=1, int stderr_fd=2)
{

	// prepend the directory (if any) to the filename,
	// unless the filename is an absolute pathname
	// 
	// The filename is an absolute pathname if it starts with a '/' on UNIX,
	// or a letter and colon pair on Windows.

  startup_cerr << "Creating process " << File << " in directory " << dir << endl;

  if (argv) {
    startup_cerr << "Arguments: (" << argv->size() << ")" << endl;
    for (unsigned a = 0; a < argv->size(); a++)
      startup_cerr << "   " << a << ": " << (*argv)[a] << endl;
  }
  if (envp) {
    startup_cerr << "Environment: (" << envp->size() << ")" << endl;
    for (unsigned e = 0; e < envp->size(); e++)
      startup_cerr << "   " << e << ": " << (*envp)[e] << endl;
  }
  startup_printf("Stdin: %d, stdout: %d, stderr: %d\n", stdin_fd, stdout_fd, stderr_fd);

    
   pdstring file = File;
	if( dir.length() > 0 )
	{
#if !defined(i386_unknown_nt4_0)
		if( !file.prefixed_by("/") )
		{
			// file does not start  with a '/', so it is a relative pathname
			// we modify it to prepend the given directory
			if( dir.suffixed_by("/") )
			{
				// the dir already has a trailing '/', so we can
				// just concatenate them to get an absolute path
				file = dir + file;
			}
			else
			{
				// the dir does not have a trailing '/', so we must
				// add a '/' to get the absolute path
				file = dir + "/" + file;
			}
		}
		else
		{
			// file starts with a '/', so it is an absolute pathname
			// DO NOT prepend the directory, regardless of what the
			// directory variable holds.
			// nothing to do in this case
		}

#else // !defined(i386_unknown_nt4_0)
		if( (file.length() < 2) ||	// file is too short to be a drive specifier
			!isalpha( file[0] ) ||	// first character in file is not a letter
			(file[1] != ':') )		// second character in file is not a colon
		{
			file = dir + "\\" + file;
		}
#endif // !defined(i386_unknown_nt4_0)
	}

#if defined(BPATCH_LIBRARY) && !defined(BPATCH_REDIRECT_IO)
    pdstring inputFile;
    pdstring outputFile;
#else
    // check for I/O redirection in arg list.
    pdstring inputFile;
    for (unsigned i1=0; i1<argv->size(); i1++) {
      if ((*argv)[i1] == "<") {
        inputFile = (*argv)[i1+1];
        for (unsigned j=i1+2, k=i1; j<argv->size(); j++, k++)
          (*argv)[k] = (*argv)[j];
        argv->resize(argv->size()-2);
      }
    }
    // TODO -- this assumes no more than 1 of each "<", ">"
    pdstring outputFile;
    for (unsigned i2=0; i2<argv->size(); i2++) {
      if ((*argv)[i2] == ">") {
        outputFile = (*argv)[i2+1];
        for (unsigned j=i2+2, k=i2; j<argv->size(); j++, k++)
          (*argv)[k] = (*argv)[j];
        argv->resize(argv->size()-2);
      }
    }


#endif /* BPATCH_LIBRARY */

    int traceLink = -1; // set by forkNewProcess, below.

    int pid;
    int tid;

    // NT
    int procHandle_temp;
    int thrHandle_temp;

    struct stat file_stat;
    int stat_result;

    stat_result = stat(file.c_str(), &file_stat);
    
    if(stat_result == -1) {
        startup_printf("%s[%d]:  failed to read file %s\n", __FILE__, __LINE__, file.c_str());
        pdstring msg = pdstring("Can't read executable file ") + file + (": ") + strerror(errno);
        showErrorCallback(68, msg.c_str());
        return(NULL);
    }

    if (!getDBI()->forkNewProcess(file, dir, argv, envp, inputFile, outputFile,
                        traceLink, pid, tid, procHandle_temp, thrHandle_temp,
                        stdin_fd, stdout_fd, stderr_fd)) {
        // forkNewProcess is responsible for displaying error messages
        // Note: if the fork succeeds, but exec fails, forkNew...
        // will return true. 
       fprintf(stderr, "[%s:%u] - Couldn't fork\n", __FILE__, __LINE__);
       return NULL;
    }

    startup_cerr << "Fork new process... succeeded" << endl;

#ifdef BPATCH_LIBRARY
    // Register the pid with the BPatch library (not yet associated with a
    // BPatch_thread object).
    assert(BPatch::bpatch != NULL);
    BPatch::bpatch->registerProvisionalThread(pid);
#endif

#if defined(i386_unknown_nt4_0)
    int status = procHandle_temp;
    // DEBUGGING
#else
    int status = pid;
#endif // defined(i386_unknown_nt4_0)
    
    statusLine("initializing process data structures");

    process *theProc = new process(pid);
    assert(theProc);
    //fprintf(stderr, "%s[%d]:  setting initial process state to running\n", FILE__, __LINE__);
    theProc->set_status(running);

    // We need to add this as soon as possible, since a _lot_ of things
    // do lookups.
    processVec.push_back(theProc);
    activeProcesses++;

    if (!theProc->setupCreated(traceLink)) {
        cleanupBPatchHandle(pid);
        processVec.pop_back();
        delete theProc;
        return NULL;
    }

    // AIX: wait for a trap so that we're sure the process is loaded
    // This is because we still read out of memory; bad idea, but...
    fileDescriptor desc;
    if (!process::getExecFileDescriptor(file, pid, true, status, desc)) {
        startup_cerr << "Failed to find exec descriptor" << endl;
        cleanupBPatchHandle(pid);
        processVec.pop_back();
        delete theProc;
        return NULL;
    }
    HACKSTATUS = status;

    if (!theProc->setAOut(desc)) {
        startup_printf("[%s:%u] - Couldn't setAOut\n", __FILE__, __LINE__);
        cleanupBPatchHandle(pid);
        processVec.pop_back();
        delete theProc;
        return NULL;
    }

    if (!theProc->setupGeneral()) {
        startup_printf("[%s:%u] - Couldn't setupGeneral\n", __FILE__, __LINE__);
        cleanupBPatchHandle(pid);
        processVec.pop_back();
        delete theProc;
        return NULL;
    }

    // Let's try to profile memory usage
#if defined(PROFILE_MEM_USAGE)
   void *mem_usage = sbrk(0);
   fprintf(stderr, "Post process: sbrk %p\n", mem_usage);
#endif

   return theProc;    
}


process *ll_attachProcess(const pdstring &progpath, int pid) 
{
  // implementation of dynRPC::attach() (the igen call)
  // This is meant to be "the other way" to start a process (competes w/ createProcess)
  
  // progpath gives the full path name of the executable, which we use ONLY to
  // read the symbol table.
  
  // We try to make progpath optional, since given pid, we should be able to
  // calculate it with a clever enough search of the process' PATH, examining
  // its argv[0], examining its current directory, etc.  /proc gives us this
  // information on solaris...not sure about other platforms...

    // No longer take the afterAttach argument. Instead, the process object records
    // the state of the process at attach, and the user can read and act on this as
    // they please -- bernat, JAN03

  startup_cerr << "welcome to attachProcess for pid " << pid << endl;
  startup_cerr << "Given program path: " << progpath << endl;

  // QUESTION: When we attach to a process, do we want to redirect its stdout/stderr
  //           (like we do when we fork off a new process the 'usual' way)?
  //           My first guess would be no.  -ari
  //           But although we may ignore the io, we still need the trace stream.
  
  // When we attach to a process, we don't fork...so this routine is much
  // simpler than its "competitor", ll_createProcess() (above).
  pdstring fullPathToExecutable = process::tryToFindExecutable(progpath, pid);

  if (!fullPathToExecutable.length()) {
      return NULL;
  }

  process *theProc = new process(pid);
  assert(theProc);

  // Add this as we can't do _anything_ without a process to look up.
  processVec.push_back(theProc);
  activeProcesses++;

  if (!theProc->setupAttached()) {
        processVec.pop_back();
      delete theProc;
      return NULL;
  }

#if defined(i386_unknown_nt4_0)
  int status = (int)INVALID_HANDLE_VALUE;	// indicates we need to obtain a valid handle
#else
  int status = pid;
#endif // defined(i386_unknown_nt4_0)

  fileDescriptor desc;
  if (!process::getExecFileDescriptor(fullPathToExecutable,
#if defined(os_windows)
                             (int)INVALID_HANDLE_VALUE,
#else
                             pid,
#endif
                             false,
                             status, 
                             desc)) {
        processVec.pop_back();
      delete theProc;
      return NULL;
  }
  
  if (!theProc->setAOut(desc)) {
        processVec.pop_back();
      delete theProc;
      return NULL;
  }
  if (!theProc->setupGeneral()) {
        processVec.pop_back();
      delete theProc;
      return NULL;
  }

  return theProc; // successful
}

/*
 * This function is needed in the unique case where we want to 
 * attach to an application which has been previously stopped
 * in the exec() system call as in the normal case (createProcess).
 * In this particular case, the SIGTRAP due to the exec() has been 
 * previously caught by another process, therefore we should taking into
 * account this issue in the necessary platforms. 
 * Basically, is an hybrid case between addprocess() and attachProcess().
 * 
 */

process *ll_attachToCreatedProcess(int pid, const pdstring &progpath)
{
    /* parent */
    statusLine("initializing process data structures");

    process *theProc = new process(pid);
    assert(theProc);

    // Add immediately
    processVec.push_back(theProc);
    activeProcesses++;

    // This is the same as attaching...
    if (!theProc->setupAttached()) {
        processVec.pop_back();
        delete theProc;
        return NULL;
    }
    // But here we reset the bootstrap state to indicate that we're between
    // exec and main
    theProc->resetBootstrapState(begun_bs);
    // Now, we missed the trap (because the creator already saw
    // it). However, we can always create a signal. We do that as soon
    // as we've parsed the image.

    pdstring fullPathToExecutable = process::tryToFindExecutable(progpath, pid);
    
    if (!fullPathToExecutable.length()) {
        return false;
    }  
    
    int status = pid;
    
    // Get the file descriptor for the executable file
    // "true" value is for AIX -- waiting for an initial trap
    // it's ignored on other platforms
    fileDescriptor desc;
    if (!process::getExecFileDescriptor(fullPathToExecutable, 
                               pid, 
                               false,
                               status, desc)) {
        processVec.pop_back();
        delete theProc;
        return false;
    } 

    if (!theProc->setAOut(desc)) {
        processVec.pop_back();
        delete theProc;
        return NULL;
    }

#if !defined(os_windows)    
    // Now, fake a trap signal and off to setupGeneral
    EventRecord ev;
    ev.proc = theProc;
    bool res = SignalHandlerUnix::handleSigTrap(ev);
    assert(res);
#endif

    // Process ran for about a second....
    // should now be stopped at main
    if (!theProc->setupGeneral()) {
        processVec.pop_back();
        delete theProc;
        return NULL;
    }
    
    return theProc;
} // end of AttachToCreatedProcess




/***************************************************************************
 **** Runtime library initialization code (Dyninst)                     ****
 ***************************************************************************/

/*
 * Gratuitously large comment. This diagrams the startup flow of 
 * messages between the mutator and mutatee. Entry points 
 * for create and attach process are both given.
 *     Mutator           Signal              Mutatee
 * Create:
 *     Fork/Exec
 *                     <-- Trap              Halted in exec
 *     Install trap in main             
 *                     <-- Trap              Halted in main
 *  Attach: (also paused, not in main)
 *     Install call to dlopen/
 *     LoadLibrary       
 *                     <-- Trap              In library load
 *     Set parameters in library
 *                     <-- Trap              Finished loading
 *     Restore code and leave paused
 *     Finalize library
 *       If finalizing fails, 
 *       init via iRPC
 *
 * Note: we've actually stopped trying to trap the library, since
 * setting arguments via inferior RPC is just as easy and a _lot_
 * cleaner.
 */

/*
 * In all cases, the process is left paused at the entry of main
 * (create) or where it was (attach). No permanent instrumentation
 * is inserted.
 */

// Load and initialize the runtime library: returns when the RT lib
// is both loaded and initialized.
// Return val: false=error condition

bool process::loadDyninstLib() {
  startup_printf("Entry to loadDyninstLib\n");
    // Wait for the process to get to an initialized (dlopen exists)
    // state
    while (!reachedBootstrapState(initialized_bs)) {
      startup_printf("Waiting for process to reach initialized state...\n");
       if(hasExited()) {
           return false;
       }
       getMailbox()->executeCallbacks(FILE__, __LINE__);
       getSH()->waitForEvent(evtProcessInit);
       getMailbox()->executeCallbacks(FILE__, __LINE__);
    }
    assert (isStopped());
    startup_printf("Stopped at entry of main\n");

    // We've hit the initialization trap, so load dyninst lib and
    // force initialization
    pdstring buffer = pdstring("PID=") + pdstring(pid);
    buffer += pdstring(", initializing shared objects");       
    statusLine(buffer.c_str());

    // Perform initialization...
    if (!dyn->initialize()) 
    {
       cerr << "Dyninst was unable to load the dyninst runtime library "
            << "into the application.  This may be caused by statically "
            << "linked executables, or by having dyninst linked against a "
            << "different version of libelf than it was built with." << endl;
       return false;
    }
    startup_printf("Initialized dynamic linking tracer\n");

    // And get the list of all shared objects in the process. More properly,
    // get the address of dlopen.
    if (!processSharedObjects()) {
      startup_printf("Failed to get initial shared objects\n");
      return false;
    }

    startup_printf("Processed initial shared objects:\n");

    for (unsigned debug_iter = 1; debug_iter < mapped_objects.size(); debug_iter++)
        startup_printf("%d: %s\n", debug_iter, 
                       mapped_objects[debug_iter]->debugString().c_str());

    startup_printf("----\n");

    if (dyninstLibAlreadyLoaded()) {
        logLine("ERROR: dyninst library already loaded, we missed initialization!");
	// TODO: We could handle this case better :)
        assert(0);
    }
    
    if (!getDyninstRTLibName()) {
        startup_printf("Failed to get Dyninst RT lib name\n");
        return false;
    }
    startup_printf("%s[%d]: Got Dyninst RT libname: %s\n", FILE__, __LINE__,
                   dyninstRT_name.c_str());

    // Force a call to dlopen(dyninst_lib)
    buffer = pdstring("PID=") + pdstring(pid);
    buffer += pdstring(", loading dyninst library");       
    statusLine(buffer.c_str());

    startup_printf("%s[%d]: Starting load of Dyninst library...\n", FILE__, __LINE__);
    loadDYNINSTlib();
    startup_printf("Think we have Dyninst RT lib set up...\n");
    
    setBootstrapState(loadingRT_bs);
    
    if (!continueProc()) {
        assert(0);
    }
    // Loop until the dyninst lib is loaded
    while (!reachedBootstrapState(loadedRT_bs)) {
        if(hasExited()) {
            startup_printf("Odd, process exited while waiting for Dyninst RT lib load\n");
            return false;
        }
        getSH()->waitForEvent(evtProcessLoadedRT);
    }
    getMailbox()->executeCallbacks(FILE__, __LINE__);
    // We haven't inserted a trap at dlopen yet (as we require the runtime lib for that)
    // So re-check all loaded libraries (and add to the list gotten earlier)
    // We force a compare even though the PC is not at the correct address.
    dyn->set_force_library_check();
    handleIfDueToSharedObjectMapping();
    dyn->unset_force_library_check();

    // Make sure the library was actually loaded
    if (!runtime_lib) {
        fprintf(stderr, "%s[%d]:  Don't have runtime library handle\n", __FILE__, __LINE__);
        return false;
    }
    
    buffer = pdstring("PID=") + pdstring(pid);
    buffer += pdstring(", initializing mutator-side structures");
    statusLine(buffer.c_str());    

    // The library is loaded, so do mutator-side initialization
    buffer = pdstring("PID=") + pdstring(pid);
    buffer += pdstring(", finalizing RT library");
    statusLine(buffer.c_str());    
    startup_printf("(%d) finalizing dyninst RT library\n", getPid());

    if (!finalizeDyninstLib())
      startup_printf("%s[%d]:  failed to finalize dyninst lib\n", __FILE__, __LINE__);

    if (!reachedBootstrapState(bootstrapped_bs)) {
        // For some reason we haven't run dyninstInit successfully.
        // Probably because we didn't set parameters before 
        // dyninstInit was automatically run. Catchup with
        // an inferiorRPC is the best bet.
        buffer = pdstring("PID=") + pdstring(pid);
        buffer += pdstring(", finalizing library via inferior RPC");
        statusLine(buffer.c_str());    
        iRPCDyninstInit();        
    }

    buffer = pdstring("PID=") + pdstring(pid);
    buffer += pdstring(", dyninst RT lib ready");
    statusLine(buffer.c_str());    

    return true;
}

// Set the shared object mapping for the RT library
bool process::setDyninstLibPtr(mapped_object *RTobj) {
    assert (!runtime_lib);

    runtime_lib = RTobj;
    return true;
}


// Set up the parameters for DYNINSTinit in the RT lib

bool process::setDyninstLibInitParams() {

   startup_cerr << "process::setDYNINSTinitArguments()" << endl;

   int pid = getpid();
   
   // Cause: 
   // 1 = created
   // 2 = forked
   // 3 = attached
   
   int cause;
   switch (creationMechanism_) {
   case created_cm:
       cause = 1;
       break;
   case attached_cm:
       cause = 3;
       break;
   case attachedToCreated_cm:
       // Uhhh....
       cause = 3;
       break;
   default:
       assert(0);
       break;
   }
   
   // Now we write these variables into the following global vrbles
   // in the dyninst library:
   // libdyninstAPI_RT_init_localCause
   // libdyninstAPI_RT_init_localPid

   pdvector<int_variable *> vars;

   if (!findVarsByAll("libdyninstAPI_RT_init_localCause",
                      vars,
                      dyninstRT_name))
       if (!findVarsByAll("_libdyninstAPI_RT_init_localCause",
                          vars))
           assert(0 && "Could not find necessary internal variable");
   assert(vars.size() == 1);
   writeDataSpace((void*)vars[0]->getAddress(), sizeof(int), (void *)&cause);
   vars.clear();

   if (!findVarsByAll("libdyninstAPI_RT_init_localPid", vars))
       if (!findVarsByAll("_libdyninstAPI_RT_init_localPid", vars))
           assert(0 && "Could not find necessary internal variable");
   assert(vars.size() == 1);
   writeDataSpace((void*)vars[0]->getAddress(), sizeof(int), (void *)&pid);
   vars.clear();   

   if (!findVarsByAll("libdyninstAPI_RT_init_maxthreads", vars))
       if (!findVarsByAll("_libdyninstAPI_RT_init_maxthreads", vars))
           assert(0 && "Could not find necessary internal variable");
   assert(vars.size() == 1);
   writeDataSpace((void*)vars[0]->getAddress(), sizeof(int), (void *) &max_number_of_threads);
   vars.clear();   

   startup_cerr << "process::installBootstrapInst() complete" << endl;   
   return true;
}

// Call DYNINSTinit via an inferiorRPC
bool process::iRPCDyninstInit() {
    startup_printf("[%s:%u] - Running DYNINSTinit via irpc\n", __FILE__, __LINE__);
    // Duplicates the parameter code in setDyninstLibInitParams()
    int pid = getpid();
    int maxthreads = maxNumberOfThreads();

    int cause = 0;
    switch (creationMechanism_) {
    case created_cm:
        cause = 1;
        break;
    case attached_cm:
        cause = 3;
        break;
    case attachedToCreated_cm:
        // Uhhh....
        cause = 3;
        break;
    default:
        assert(0);
        break;
   }

    pdvector<AstNode*> the_args(3);
    the_args[0] = new AstNode(AstNode::Constant, (void*)(Address)cause);
    the_args[1] = new AstNode(AstNode::Constant, (void*)(Address)pid);
    the_args[2] = new AstNode(AstNode::Constant, (void*)(Address)maxthreads);
    AstNode *dynInit = new AstNode("DYNINSTinit", the_args);
    removeAst(the_args[0]); removeAst(the_args[1]); removeAst(the_args[2]);
    getRpcMgr()->postRPCtoDo(dynInit,
                             true, // Don't update cost
                             process::DYNINSTinitCompletionCallback,
                             NULL, // No user data
                             true, // Use reserved memory
                             NULL, NULL);// No particular thread or LWP

    // We loop until dyninst init has run (check via the callback)
     inferiorrpc_printf("%s[%d]:  waiting for rpc completion\n", FILE__, __LINE__);
    while (!reachedBootstrapState(bootstrapped_bs)) {
        getRpcMgr()->launchRPCs(false); // false: not running
        if(hasExited()) {
            fprintf(stderr, "%s[%d][%s]:  unexpected exit\n", __FILE__, __LINE__, getThreadStr(getExecThreadID()));
           return false;
        }
        getMailbox()->executeCallbacks(FILE__, __LINE__);
        getSH()->waitForEvent(evtRPCSignal, this, NULL /*lwp*/, statusRPCDone);
        getMailbox()->executeCallbacks(FILE__, __LINE__);
    }
    startup_printf("%s[%d][%s]:  bootstrapped\n", __FILE__, __LINE__, getThreadStr(getExecThreadID()));
    startup_printf("[%s:%u] - Ran DYNINSTinit via irpc\n", __FILE__, __LINE__);
    return true;
}

bool process::attach() {
   dictionary_hash_iter<unsigned, dyn_lwp *> lwp_iter(real_lwps);
   dyn_lwp *lwp;
   unsigned index;
   assert(getRepresentativeLWP());  // we expect this to be defined
   // Though on linux, if process found to be MT, there will be no
   // representativeLWP since there is no lwp which controls the entire
   // process for MT linux.

   startup_printf("[%d]: attaching to representative LWP\n", getPid());

   if(! getRepresentativeLWP()->attach())
      return false;

   while (lwp_iter.next(index, lwp)) {
       startup_printf("[%d]: attaching to LWP %d\n", getPid(), index);

      if (!lwp->attach()) {
         deleteLWP(lwp);
         return false;
      }
   }
   // Fork calls attach() but is probably past this; silencing warning
   if (!reachedBootstrapState(attached_bs))
       setBootstrapState(attached_bs);
   getSH()->signalActiveProcess();
   startup_printf("[%d]: setting process flags\n", getPid());
   return setProcessFlags();
}


// Callback: finish mutator-side processing for dyninst lib

bool process::finalizeDyninstLib() 
{
   startup_printf("%s[%d]:  isAttached() = %s\n", __FILE__, __LINE__, isAttached() ? "true" : "false");
   startup_printf("%s[%d]: %s\n", __FILE__, __LINE__, getStatusAsString().c_str());
   
    assert (isStopped());
    if (reachedBootstrapState(bootstrapped_bs)) {
        return true;
    }

   DYNINST_bootstrapStruct bs_record;
   if (!extractBootstrapStruct(&bs_record))
      assert(false);

   // Read the structure; if event 0 then it's undefined! (not yet written)
   if (bs_record.event == 0)
   {
       startup_printf("[%s:%u] - bs_record.event is undefined\n");
       return false;
   }

   // Now that we have the dyninst library loaded, insert the hooks to dlopen/dlclose
   // (which may require the dyninst library)
    
   assert(bs_record.event == 1 || bs_record.event == 2 || bs_record.event==3);

   bool calledFromFork = (bs_record.event == 2);
   bool calledFromAttach = (bs_record.event == 3);

   pdvector<int_variable *> obsCostVec;
   if (!findVarsByAll("DYNINSTobsCostLow", obsCostVec))
       assert(0);
   assert(obsCostVec.size() == 1);

   costAddr_ = obsCostVec[0]->getAddress();

   assert(costAddr_);

   // Set breakpoints to detect (un)loaded libraries
   // Do this after the observed cost is set, in case we use instrumentation
   if (!dyn->installTracing()) assert (0 && "Failed to install library mapping hooks");

   if (!calledFromFork) {

       // Install initial instrumentation requests
       pdstring str=pdstring("PID=") + pdstring(bs_record.pid) + ", installing default (DYNINST) inst...";
       statusLine(str.c_str());
       
       extern pdvector<instMapping*> initialRequests; // init.C

       // Install any global instrumentation requests
       installInstrRequests(initialRequests);

       // Install process-specific instrumentation requests
       installInstrRequests(tracingRequests);
       
       // Install our system call tracing
       tracedSyscalls_ = new syscallNotification(this);

       //#ifdef i386_unknown_linux2_0
       //if(pdFlavor != "mpi") {
       //#endif
          // TODO: prefork and pre-exit should depend on whether a callback is defined
          if (!tracedSyscalls_->installPreFork()) 
             cerr << "Warning: failed pre-fork notification setup" << endl;
          if (!tracedSyscalls_->installPostFork()) 
             cerr << "Warning: failed post-fork notification setup" << endl;
          if (!tracedSyscalls_->installPreExec()) 
             cerr << "Warning: failed pre-exec notification setup" << endl;
          if (!tracedSyscalls_->installPostExec()) 
             cerr << "Warning: failed post-exec notification setup" << endl;
          if (!tracedSyscalls_->installPreExit()) 
             cerr << "Warning: failed pre-exit notification setup" << endl;
          if (!tracedSyscalls_->installPreLwpExit()) 
             cerr << "Warning: failed pre-lwp-exit notification setup" << endl;
          //#ifdef i386_unknown_linux2_0
          //}
          //#endif
   }
   else { // called from a forking process
       process *parentProcess = process::findProcess(bs_record.ppid);
       if (parentProcess) {
           if (parentProcess->status() == stopped) {
               if (!parentProcess->continueProc())
                   assert(false);
           }
           else
               parentProcess->continueAfterNextStop();
       }
       
   }

   startup_printf("Initializing tramp guard\n");
   if (!initTrampGuard())
      assert(0);

#if defined(cap_threads)
   if (multithread_capable())
   {
      initMT();
   }
#endif

   if (!calledFromAttach) {
       pdstring str=pdstring("PID=") + pdstring(bs_record.pid) + ", dyninst ready.";
       statusLine(str.c_str());
   }

   startup_printf("%s[%d]:  bootstrap done\n", __FILE__, __LINE__);
   // Ready to rock
   setBootstrapState(bootstrapped_bs);
   getSH()->signalEvent(evtProcessInitDone);

   return true;
}

void finalizeDyninstLibWrapper(process *p) 
{
  global_mutex->_Lock(FILE__, __LINE__);
  p->finalizeDyninstLib();
  global_mutex->_Unlock(FILE__, __LINE__);
}
void process::DYNINSTinitCompletionCallback(process* theProc,
                                            unsigned /* rpc_id */,
                                            void* /*userData*/, // user data
                                            void* /*ret*/) // return value from DYNINSTinit
{
    //global_mutex->_Lock(FILE__, __LINE__);
    startup_printf("%s[%d]:  about to finalize Dyninst Lib\n", __FILE__, __LINE__);
    theProc->finalizeDyninstLib();
    //FinalizeRTLibCallback *cbp = new FinalizeRTLibCallback(finalizeDyninstLibWrapper);
    //FinalizeRTLibCallback &cb = *cbp;
    //cb.setSynchronous(false);
    //cb(theProc);
    //global_mutex->_Unlock(FILE__, __LINE__);
}
/////////////////////////////////////////
// Function lookup...
/////////////////////////////////////////

bool process::findFuncsByAll(const pdstring &funcname,
                             pdvector<int_function *> &res,
                             const pdstring &libname) { // = "", btw
    
    unsigned starting_entries = res.size(); // We'll return true if we find something
    for (unsigned i = 0; i < mapped_objects.size(); i++) {
        if (libname == "" ||
            mapped_objects[i]->fileName() == libname ||
            mapped_objects[i]->fullName() == libname) {
            const pdvector<int_function *> *pretty = mapped_objects[i]->findFuncVectorByPretty(funcname);
            if (pretty) {
                // We stop at first match...
                for (unsigned pm = 0; pm < pretty->size(); pm++) {
                    res.push_back((*pretty)[pm]);
                }
            }
            else {
                const pdvector<int_function *> *mangled = mapped_objects[i]->findFuncVectorByMangled(funcname);
                if (mangled) {
                    for (unsigned mm = 0; mm < mangled->size(); mm++) {
                        res.push_back((*mangled)[mm]);
                    }
                }
            }
        }
    }
    return (res.size() != starting_entries);
}


bool process::findFuncsByPretty(const pdstring &funcname,
                             pdvector<int_function *> &res,
                             const pdstring &libname) { // = "", btw

    unsigned starting_entries = res.size(); // We'll return true if we find something

    for (unsigned i = 0; i < mapped_objects.size(); i++) {
        if (libname == "" ||
            mapped_objects[i]->fileName() == libname ||
            mapped_objects[i]->fullName() == libname) {
            const pdvector<int_function *> *pretty = mapped_objects[i]->findFuncVectorByPretty(funcname);
            if (pretty) {
                // We stop at first match...
                for (unsigned pm = 0; pm < pretty->size(); pm++) {
                    res.push_back((*pretty)[pm]);
                }
            }
        }
    }
    return res.size() != starting_entries;
}


bool process::findFuncsByMangled(const pdstring &funcname,
                                 pdvector<int_function *> &res,
                                 const pdstring &libname) { // = "", btw
    unsigned starting_entries = res.size(); // We'll return true if we find something

    for (unsigned i = 0; i < mapped_objects.size(); i++) {
        if (libname == "" ||
            mapped_objects[i]->fileName() == libname ||
            mapped_objects[i]->fullName() == libname) {
            const pdvector<int_function *> *mangled = 
               mapped_objects[i]->findFuncVectorByMangled(funcname);
            if (mangled) {
                for (unsigned mm = 0; mm < mangled->size(); mm++) {
                   res.push_back((*mangled)[mm]);
                }
            }
        }
    }
    return res.size() != starting_entries;
}

int_function *process::findOnlyOneFunction(const pdstring &name,
                                           const pdstring &lib) {
    pdvector<int_function *> allFuncs;
    if (!findFuncsByAll(name, allFuncs, lib))
        return NULL;
    if (allFuncs.size() > 1) {
        cerr << "Warning: multiple matches for " << name << ", returning first" << endl;
    }
    return allFuncs[0];
}

/////////////////////////////////////////
// Variable lookup...
/////////////////////////////////////////

bool process::findVarsByAll(const pdstring &varname,
                            pdvector<int_variable *> &res,
                            const pdstring &libname) { // = "", btw
    unsigned starting_entries = res.size(); // We'll return true if we find something
    
    for (unsigned i = 0; i < mapped_objects.size(); i++) {
        if (libname == "" ||
            mapped_objects[i]->fileName() == libname ||
            mapped_objects[i]->fullName() == libname) {
            const pdvector<int_variable *> *pretty = mapped_objects[i]->findVarVectorByPretty(varname);
            if (pretty) {
                // We stop at first match...
                for (unsigned pm = 0; pm < pretty->size(); pm++) {
                    res.push_back((*pretty)[pm]);
                }
            }
            else {
                const pdvector<int_variable *> *mangled = mapped_objects[i]->findVarVectorByMangled(varname);
                if (mangled) {
                    for (unsigned mm = 0; mm < mangled->size(); mm++) {
                        res.push_back((*mangled)[mm]);
                    }
                }
            }
        }
    }
    return res.size() != starting_entries;
}



// Get me a pointer to the instruction: the return is a local
// (mutator-side) store for the mutatee. This may duck into the local
// copy for images, or a relocated function's self copy.
// TODO: is this really worth it? Or should we just use ptrace?

void *process::getPtrToInstruction(Address addr) {
    codeRange *range;
    if (codeSections_.find(addr, range)) {
        return range->getPtrToInstruction(addr);
    }
    else if (dataSections_.find(addr, range)) {
        mappedObjData *data = dynamic_cast<mappedObjData *>(range);
        assert(data);
        return data->obj->getPtrToData(addr);
    }
    return NULL;
}

bool process::isValidAddress(Address addr) {
    // "Is this part of the process address space?"
    // We should codeRange data sections as well... since we don't, this is 
    // slow.
    codeRange *dontcare;
    if (codeSections_.find(addr, dontcare))
        return true;
    if (dataSections_.find(addr, dontcare))
        return true;
    fprintf(stderr, "Warning: address 0x%x not valid!\n",
            addr);
    return false;
}        

dyn_thread *process::STdyn_thread() { 
   assert(! multithread_capable());
   assert(threads.size()>0);
   return threads[0];
}

#if !defined(BPATCH_LIBRARY)
void process::processCost(unsigned obsCostLow,
                          timeStamp wallTime,
                          timeStamp processTime) {
  // wallTime and processTime should compare to DYNINSTgetWallTime() and
  // DYNINSTgetCPUtime().

  // check for overflow, add to running total, convert cycles to seconds, and
  // report.  Member vrbles of class process: lastObsCostLow and cumulativeObsCost
  // (the latter a 64-bit value).

  // code to handle overflow used to be in rtinst; we borrow it pretty much
  // verbatim. (see rtinst/RTposix.c)
  if (obsCostLow < lastObsCostLow) {
    // we have a wraparound
    cumulativeObsCost += ((unsigned)0xffffffff - lastObsCostLow) + obsCostLow + 1;
  }
  else
    cumulativeObsCost += (obsCostLow - lastObsCostLow);
  
  lastObsCostLow = obsCostLow;
  //  sampleVal_cerr << "processCost- cumulativeObsCost: " << cumulativeObsCost << "\n"; 
  timeLength observedCost((int64_t) cumulativeObsCost, getCyclesPerSecond());
  // timeUnit tu = getCyclesPerSecond(); // just used to print out
  //  sampleVal_cerr << "processCost: cyclesPerSecond=" << tu
  //		 << "; cum obs cost=" << observedCost << "\n";
  
  // Notice how most of the rest of this is copied from processCost() of
  // metric.C.  Be sure to keep the two "in sync"!

  extern costMetric *totalPredictedCost; // init.C
  extern costMetric *observed_cost;      // init.C
  
  const timeStamp lastProcessTime = 
    totalPredictedCost->getLastSampleProcessTime(this);
  //  sampleVal_cerr << "processCost- lastProcessTime: " <<lastProcessTime << "\n";
  // find the portion of uninstrumented time for this interval
  timeLength userPredCost = timeLength::sec() + getCurrentPredictedCost();
  //  sampleVal_cerr << "processCost- userPredCost: " << userPredCost << "\n";
  const double unInstTime = (processTime - lastProcessTime) / userPredCost; 
  //  sampleVal_cerr << "processCost- unInstTime: " << unInstTime << "\n";
  // update predicted cost
  // note: currentPredictedCost is the same for all processes
  //       this should be changed to be computed on a per process basis
  pdSample newPredCost = totalPredictedCost->getCumulativeValue(this);
  //  sampleVal_cerr << "processCost- newPredCost: " << newPredCost << "\n";
  timeLength tempPredCost = getCurrentPredictedCost() * unInstTime;
  //  sampleVal_cerr << "processCost- tempPredCost: " << tempPredCost << "\n";
  newPredCost += pdSample(tempPredCost.getI(timeUnit::ns()));
  //  sampleVal_cerr << "processCost- tempPredCost: " << newPredCost << "\n";
  totalPredictedCost->updateValue(this, wallTime, newPredCost, processTime);
  // update observed cost
  pdSample sObsCost(observedCost);
  observed_cost->updateValue(this, wallTime, sObsCost, processTime);
}
#endif
        
// If true is passed for ignore_if_mt_not_set, then an error won't be
// initiated if we're unable to determine if the program is multi-threaded.
// We are unable to determine this if the daemon hasn't yet figured out what
// libraries are linked against the application.  Currently, we identify an
// application as being multi-threaded if it is linked against a thread
// library (eg. libpthreads.a on AIX).  There are cases where we are querying
// whether the app is multi-threaded, but it can't be determined yet but it
// also isn't necessary to know.
bool process::multithread_capable(bool ignore_if_mt_not_set)
{
#if !defined(cap_threads)
   return false;
#endif

   if(cached_result != not_cached) {
       if(cached_result == cached_mt_true) {
           return true;
       } else {
           assert(cached_result == cached_mt_false);
           return false;
       }
   }
   
   if(mapped_objects.size() <= 1) {
       if(! ignore_if_mt_not_set) {
           cerr << "   can't query MT state, assert\n";
           assert(false);
       }
       return false;
   }

   if(findObject("libthread.so*", true) ||  // Solaris
      findObject("libpthreads.*", true)  ||  // AIX
      findObject("libpthread.so*", true))   // Linux
   {
       cached_result = cached_mt_true;
       return true;
   } else {
       cached_result = cached_mt_false;
       return false;
   }
}

void process::addThread(dyn_thread *thread)
{
   getRpcMgr()->addThread(thread);
   threads.push_back(thread);
}

bool process::multithread_ready(bool ignore_if_mt_not_set) {
   if (!multithread_capable(ignore_if_mt_not_set))
      return false;
   return isBootstrappedYet();
}

dyn_lwp *process::query_for_stopped_lwp() {
   dyn_lwp *foundLWP = NULL;
   dictionary_hash_iter<unsigned, dyn_lwp *> lwp_iter(real_lwps);
   dyn_lwp *lwp;
   unsigned index;

   if(IndependentLwpControl()) {
      while (lwp_iter.next(index, lwp)) {
         if(lwp->status() == stopped || lwp->status() == neonatal) {
            foundLWP = lwp;
            break;
         }
      }
      if(foundLWP == NULL  &&  getRepresentativeLWP() != NULL) {
         if(getRepresentativeLWP()->status() == stopped ||
            getRepresentativeLWP()->status() == neonatal)
            foundLWP = getRepresentativeLWP();
      }
   } else {
      if(this->status() == stopped || this->status() == neonatal) {
         foundLWP = getRepresentativeLWP();
      }
   }

   return foundLWP;
}

dyn_lwp *process::query_for_running_lwp() {
   dyn_lwp *foundLWP = NULL;
   dictionary_hash_iter<unsigned, dyn_lwp *> lwp_iter(real_lwps);
   dyn_lwp *lwp;
   unsigned index;

   if(IndependentLwpControl()) {
      while (lwp_iter.next(index, lwp)) {
         if(lwp->status() == running || lwp->status() == neonatal) {
            foundLWP = lwp;
            break;
         }
      }
      if(foundLWP == NULL  &&  getRepresentativeLWP() != NULL) {
         if(getRepresentativeLWP()->status() == running ||
            getRepresentativeLWP()->status() == neonatal)
            foundLWP = getRepresentativeLWP();
      }
   } else {
      if(this->status() == running || this->status() == neonatal) {
         foundLWP = getRepresentativeLWP();
      }
   }

   return foundLWP;
}

// first searches for stopped lwp and if not found explicitly stops an lwp
dyn_lwp *process::stop_an_lwp(bool *wasRunning) {
   dictionary_hash_iter<unsigned, dyn_lwp *> lwp_iter(real_lwps);
   dyn_lwp *lwp;
   dyn_lwp *stopped_lwp = NULL;
   unsigned index;
   if (!isAttached()) {
     fprintf(stderr, "%s[%d]:  cannot stop_an_lwp, process not attached\n", FILE__, __LINE__);
     return false;
   }

   if(IndependentLwpControl()) {
      while(lwp_iter.next(index, lwp)) {
         if(lwp->status() == stopped) {
            stopped_lwp = lwp;
            *wasRunning = false;
            break;
         }
         if(lwp->pauseLWP()) {
            stopped_lwp = lwp;
            *wasRunning = true;
            break;
         }
      }
      if(stopped_lwp == NULL) {
         if(getRepresentativeLWP()->status() == stopped) {
            *wasRunning = false;
         } else {
            getRepresentativeLWP()->pauseLWP();
            *wasRunning = true;
         }
         stopped_lwp = getRepresentativeLWP();
      }
   } else {
      if(getRepresentativeLWP()->status() == stopped)
         *wasRunning = false;
      else {
         getRepresentativeLWP()->pauseLWP();
         *wasRunning = true;
      }
      stopped_lwp = getRepresentativeLWP();
   }

   if (!stopped_lwp) {
     fprintf(stderr, "%s[%d][%s]:  stop_an_lwp failing\n", FILE__, __LINE__, getThreadStr(getExecThreadID()));
   }
   return stopped_lwp;
}

bool process::terminateProc() {
   if(status() == exited) {
     // "Sure, we terminated it... really!"
     return true;
   }
   terminateProcStatus_t retVal = terminateProc_();

   switch (retVal) {
   case terminateSucceeded:
     {
     // handle the kill signal on the process, which will dispatch exit callback
      signal_printf("%s[%d][%s]:  before waitForEvent(evtProcessExit)\n", 
              FILE__, __LINE__, getThreadStr(getExecThreadID()));
      getSH()->waitForEvent(evtProcessExit);
     return true;
     }
     break;
   case alreadyTerminated:
     // don't try to consume a signal (since we can't), 
     // just set process status to exited
     set_status(exited);
     return true;
     break;
   case terminateFailed:
     return false;
     break;

   }
   assert (0 && "Can't be reached");
   return false;
}

/*
 * Copy data from controller process to the named process.
 */
bool process::writeDataSpace(void *inTracedProcess, unsigned size,
                             const void *inSelf) {
   bool needToCont = false;

   //fprintf(stderr, "writeDataSpace to %p to %p, %d\n",
   //inTracedProcess, (int)inTracedProcess+size, size);

   if (!isAttached()) return false;

   dyn_lwp *stopped_lwp = query_for_stopped_lwp();
   if(stopped_lwp == NULL) {
      stopped_lwp = stop_an_lwp(&needToCont);
      if(stopped_lwp == NULL) {
         pdstring msg =
            pdstring("System error: unable to write to process data "
                     "space (WDS): couldn't stop an lwp\n");
         fprintf(stderr, "%s[%d]:  stop_an_lwp failed\n", __FILE__, __LINE__);
         showErrorCallback(38, msg);
         return false;
      }
   }
   
   bool res = stopped_lwp->writeDataSpace(inTracedProcess, size, inSelf);
   if (!res) {
       fprintf(stderr, "WDS: %d bytes from %p to %p, lwp %p\n",
               size, inSelf, inTracedProcess, stopped_lwp);
       cerr << endl;
       pdstring msg = pdstring("System error: unable to write to process data "
                               "space (WDS):") + pdstring(strerror(errno));
         fprintf(stderr, "%s[%d]:  wds failed\n", __FILE__, __LINE__);
       showErrorCallback(38, msg);
       return false;
   }

   if(needToCont) {
      return stopped_lwp->continueLWP();
   }
   return true;
}

bool process::readDataSpace(const void *inTracedProcess, unsigned size,
                            void *inSelf, bool displayErrMsg) {
   bool needToCont = false;

   if (!isAttached()) {
      fprintf(stderr, "%s[%d][%s]:  readDataSpace() failing, not attached\n",
             __FILE__, __LINE__, getThreadStr(getExecThreadID()));
      return false;
   }
   dyn_lwp *stopped_lwp = query_for_stopped_lwp();
   if(stopped_lwp == NULL) {
      stopped_lwp = stop_an_lwp(&needToCont);
      if(stopped_lwp == NULL) {
         pdstring msg =
            pdstring("System error: unable to read to process data "
                     "space: couldn't stop an lwp\n");
         fprintf(stderr, "%s[%d]:  stop_an_lwp failed\n", FILE__, __LINE__);
         showErrorCallback(38, msg);
         return false;
      }
   }

   bool res = stopped_lwp->readDataSpace(inTracedProcess, size, inSelf);
   if (!res) {
      if (displayErrMsg) {
         sprintf(errorLine, "System error: "
                 "<>unable to read %d@%s from process data space: %s (pid=%d)",
                 size, Address_str((Address)inTracedProcess), 
                 strerror(errno), getPid());
         pdstring msg(errorLine);
         fprintf(stderr, "%s[%d]:  readDataSpace failed\n", FILE__, __LINE__);
         showErrorCallback(38, msg);
      }
      return false;
   }

   if (needToCont) {
      return stopped_lwp->continueLWP();
   }

   return true;
}

bool process::writeTextWord(caddr_t inTracedProcess, int data) {
   bool needToCont = false;

   if (!isAttached()) return false;

   dyn_lwp *stopped_lwp = query_for_stopped_lwp();
   if(stopped_lwp == NULL) {
      stopped_lwp = stop_an_lwp(&needToCont);
      if(stopped_lwp == NULL) {
         pdstring msg =
            pdstring("System error: unable to write word to process text "
                     "space: couldn't stop an lwp\n");
         fprintf(stderr, "%s[%d]:  stop_an_lwp failed\n", FILE__, __LINE__);
         showErrorCallback(38, msg);
         return false;
      }
   }

  bool res = stopped_lwp->writeTextWord(inTracedProcess, data);
  if (!res) {
     pdstring msg = pdstring("System error: unable to write word to process "
                             "text space:") + pdstring(strerror(errno));
         fprintf(stderr, "%s[%d]:  writeDataSpace failed\n", FILE__, __LINE__);
     showErrorCallback(38, msg);
     return false;
  }

  if(needToCont) {
     return stopped_lwp->continueLWP();
  }
  return true;
}

bool process::writeTextSpace(void *inTracedProcess, u_int amount, 
                             const void *inSelf) {
   bool needToCont = false;
   /*
   fprintf(stderr, "writeTextSpace to %p to %p, %d\n",
           inTracedProcess,
           (char *)inTracedProcess + amount, amount);
   */
   if (!isAttached()) return false;
   dyn_lwp *stopped_lwp = query_for_stopped_lwp();
   if(stopped_lwp == NULL) {
      stopped_lwp = stop_an_lwp(&needToCont);
      if(stopped_lwp == NULL) {
         pdstring msg =
            pdstring("System error: unable to write to process text "
                     "space (WTS): couldn't stop an lwp\n");
         fprintf(stderr, "%s[%d]:  stop_an_lwp failed\n", FILE__, __LINE__);
         showErrorCallback(38, msg);
         return false;
      }
   }

  bool res = stopped_lwp->writeTextSpace(inTracedProcess, amount, inSelf);

  if (!res) {
     pdstring msg = pdstring("System error: unable to write to process text "
                             "space (WTS):") + pdstring(strerror(errno));
         fprintf(stderr, "%s[%d]:  writeTextSpace failed\n", FILE__, __LINE__);
     showErrorCallback(38, msg);
     return false;
  }
  
  if(needToCont) {
     return stopped_lwp->continueLWP();
  }
  return true;
}

// InsrucIter uses readTextSpace
bool process::readTextSpace(const void *inTracedProcess, u_int amount,
                            const void *inSelf)
{
   bool needToCont = false;

   if (!isAttached()) return false;

   dyn_lwp *stopped_lwp = query_for_stopped_lwp();
   if(stopped_lwp == NULL) {
      stopped_lwp = stop_an_lwp(&needToCont);
      if(stopped_lwp == NULL) {
         pdstring msg =
            pdstring("System error: unable to read to process text "
                     "space: couldn't stop an lwp\n");
         showErrorCallback(39, msg);
         return false;
      }
   }

   bool res = stopped_lwp->readTextSpace(const_cast<void*>(inTracedProcess),
                                         amount, inSelf);

   if (!res) {
      sprintf(errorLine, "System error: "
              "<>unable to read %d@%s from process text space: %s (pid=%d)",
              amount, Address_str((Address)inTracedProcess), 
              strerror(errno), getPid());
      pdstring msg(errorLine);
      showErrorCallback(38, msg);
         fprintf(stderr, "%s[%d]:  readTextSpace failed\n", FILE__, __LINE__);
      return false;
   }

   if (needToCont) {
      return stopped_lwp->continueLWP();
   }

   return true;
}

void process::set_status(processState st) {
   // update the process status
   status_ = st;

   pdvector<dyn_thread *>::iterator iter = threads.begin();
   
   dyn_lwp *proclwp = getRepresentativeLWP();
   if(proclwp) proclwp->internal_lwp_set_status___(st);
   
   while(iter != threads.end()) {
      dyn_thread *thr = *(iter);
      dyn_lwp *lwp = thr->get_lwp();
      assert(lwp);
      lwp->internal_lwp_set_status___(st);
      iter++;
   }
}

void process::set_lwp_status(dyn_lwp *whichLWP, processState lwp_st) {
   // any lwp status = stopped, means proc status = stopped

   assert(whichLWP != NULL);

   // update the process status
   if(lwp_st == stopped)
      status_ = stopped;

   whichLWP->internal_lwp_set_status___(lwp_st);

   if(IndependentLwpControl()) {
      // all lwp status = running, means proc status = running
      bool stopped_lwp_exists = false;
      pdvector<dyn_thread *>::iterator iter = threads.begin();
      if(lwp_st == running) {
         while(iter != threads.end()) {
            dyn_thread *thr = *(iter);
            dyn_lwp *lwp = thr->get_lwp();
            assert(lwp);
            if(lwp->status() == stopped)
               stopped_lwp_exists = true;
            iter++;
         }
      }
      if(!stopped_lwp_exists && lwp_st==running) {
         status_ = running;
      }
   } else {
      // if can't do independent lwp control, should only be able to set
      // lwp status for representative lwp
      assert(whichLWP == getRepresentativeLWP());
      set_status(lwp_st);  // sets process status and all lwp statuses
   }
}

bool process::pause() {
  bool result;
  if (!isAttached()) {
    bperr( "Warning: pause attempted on non-attached process\n");
    return false;
  }
      
   // The only remaining combination is: status==running but haven't yet
   // reached first break.  We never want to pause before reaching the first
   // break (trap, actually).  But should we be returning true or false in
   // this case?

  if(! reachedBootstrapState(initialized_bs)) {
    return true;
  }


#if defined(sparc_sun_solaris2_4)
     inferiorrpc_printf("%s[%d]:  waiting for rpc completion\n", FILE__, __LINE__);
   while (getRpcMgr()->existsRunningIRPC()) {
      fprintf(stderr, "%s[%d][%s]:  before waitForEvent(evtRPCSignal,...,statusRPCDone)\n", 
              FILE__, __LINE__, getThreadStr(getExecThreadID()));
     getSH()->waitForEvent(evtRPCSignal, this, NULL /*lwp*/, statusRPCDone);
   }
#endif

   // Let's try having stopped mean all lwps stopped and running mean
   // atleast one lwp running.
   
   if (status_ == stopped || status_ == neonatal) {
      return true;
   }

   result = stop_();
   if (!result) {
       bperr ("Warning: low-level paused failed, process is not paused\n");
     return false;
   }
   status_ = stopped;
   return true;
}

//process::stop_ is only different on linux
#if !defined(os_linux)
bool process::stop_()
{
   assert(status_ == running);      
   bool res = getRepresentativeLWP()->pauseLWP(true);
   if (!res) {
      sprintf(errorLine,
              "warn : in process::pause, pause_ unable to pause process\n");
      logLine(errorLine);
      return false;
   }
   return true;
}
#endif

// handleIfDueToSharedObjectMapping: if a trap instruction was caused by
// a dlopen or dlclose event then return true
bool process::handleIfDueToSharedObjectMapping()
{
   if(!dyn) { 
       bperr( "No dyn object, returning false\n");
       return false;
   }
   pdvector<mapped_object *> changed_objs;

   u_int change_type = 0;

   if (!dyn->handleIfDueToSharedObjectMapping(changed_objs,
                                              change_type)) {
       // Not the right addr, I guess
       return false;
   }
   // if this trap was due to dlopen or dlclose, and if something changed
   // then figure out how it changed and either add or remove shared objects
   // if something was added then call process::addASharedObject with
   // each element in the vector of changed_objects
   if (change_type == SHAREDOBJECT_ADDED) {
       signal_printf("%s[%d]:  SHAREDOBJECT_ADDED\n", FILE__, __LINE__);
       assert(changed_objs.size());

       for(u_int i=0; i < changed_objs.size(); i++) {
 	   if (changed_objs[i]->fileName().length()==0) {
               cerr << "Warning: new shared object with no name!" << endl;
               continue;
           }
           // addASharedObject is where all the interesting stuff happens.
           if(!addASharedObject(changed_objs[i]))
               cerr << "Failed to add library " << changed_objs[i]->fullName() << endl;
       }
       return true;
   } else if (change_type == SHAREDOBJECT_REMOVED) {
       signal_printf("%s[%d]: ... removed object...\n", FILE__, __LINE__);
       // TODO: handle this case
       // if something was removed then call process::removeASharedObject
       // with each element in the vector of changed_objects
       // for now, just delete shared_objects to avoid memory leeks
       for(u_int i=0; i < changed_objs.size(); i++){
           // Remove from mapped_objects list
           for (unsigned j = 0; j < mapped_objects.size(); j++) {
               if (changed_objs[i] == mapped_objects[j]) {
                   assert(j != 0); // Better not delete the a.out. That makes things unhappy.
                   mapped_objects[j] = mapped_objects.back();
                   mapped_objects.pop_back();
                   break;
               }
           }
           delete changed_objs[i];
       }
       return true;
   }
   else {
       signal_printf("%s[%d]:  UNKNOWN\n", __FILE__, __LINE__);
       // ... okay, we handled something...
       return true;
   }
   
   return false;
}

/* Checks whether the shared object SO is the runtime instrumentation
   library.  The test looks for "libdyninst" in the library name (both
   the Dyninst and Paradyn rtinst libs have this substring on every
   platform).

   Returns:
   0  if SO is not the runtime library
   1  if SO is the runtime library, but it has not been initialized
   2  if SO is the runtime library, and it has been initialized by Dyninst
   3  if SO is the runtime library, and it has been initialized by Paradyn
 */
static int
check_rtinst(process *proc, mapped_object *so)
{
     const char *name, *p;
     static const char *libdyn = "libdyninst";
     static int len = 10; /* length of libdyn */

     name = (so->fileName()).c_str();

     p = strrchr(name, '/');
     if (!p)
	  /* name is relative to current directory */
	  p = name;
     else
	  ++p; /* skip '/' */

     if (0 != strncmp(p, libdyn, len)) {
	  return 0;
     }

     /* Now we check if the library has initialized */

     const pdvector<int_variable *> *vars = so->findVarVectorByPretty("DYNINSThasInitialized");
     if (!vars) return 0;
     if (vars->size() == 0) return 0;
     
     unsigned int val;
     if (! proc->readDataSpace((void*)((*vars)[0]->getAddress()), sizeof(val), (void*)&val, true)) {
         return 0;
     }
     if (val == 0) {
         /* The library has been loaded, but not initialized */
         return 1;
     } else
         return val;
}

// addASharedObject: This routine is called whenever a new shared object
// has been loaded by the run-time linker
// It processes the image, creates new resources
bool process::addASharedObject(mapped_object *new_obj) {
    assert(new_obj);
    // Add to mapped_objects
    // Add to codeRange tree
    // Make library callback (will trigger BPatch adding the lib)
    // Perform platform-specific lookups (e.g., signal handler)
    
    mapped_objects.push_back(new_obj);
    addCodeRange(new_obj);

    findSignalHandler(new_obj);

    pdstring msg;

    //if(new_obj->fileName().length() == 0) {
    //return false;
    //}

#ifdef NOTDEF// PDSEP
    const char *dn = dyninstRT_name.c_str();
    const char *fn = new_obj->fileName().c_str();
#endif
    parsing_printf("Adding shared object %s, addr range 0x%x to 0x%x\n",
           new_obj->fileName().c_str(), 
           new_obj->getBaseAddress(),
           new_obj->get_size_cr());
    // TODO: check for "is_elf64" consistency (Object)

    // If the static inferior heap has been initialized, then 
    // add whatever heaps might be in this guy to the known heap space.
    if (heapInitialized_) {
        // initInferiorHeap was run already, so hit this guy
        addInferiorHeap(new_obj);
    }

#if defined(os_windows)
    char dllFilename[_MAX_FNAME];
    _splitpath( dyninstRT_name.c_str(),
                NULL, NULL, dllFilename, NULL);
    pdstring shortname = pdstring(dllFilename);    
#else
    pdstring shortname = new_obj->fileName();
#endif
    pdstring longname = new_obj->fullName();

    //fprintf(stderr, "%s[%d]:  shortname = %s, longname = %s, RTlib = %s\n ", FILE__, __LINE__, shortname.c_str(), longname.c_str(), dyninstRT_name.c_str());
    if ((shortname == dyninstRT_name) 
        || (longname == dyninstRT_name)) {
      startup_printf("%s[%d]:  handling init of dyninst RT library\n", FILE__, __LINE__);
      if (!setDyninstLibPtr(new_obj)) {
        fprintf(stderr, "%s[%d]:  FATAL, failing to set dyninst lib\n", FILE__, __LINE__);
        assert(0);
      }
      if (!setDyninstLibInitParams()) {
        fprintf(stderr, "%s[%d]:  FATAL, failing to init dyninst lib\n", FILE__, __LINE__);
        assert(0);
      }
    }

#if !defined(i386_unknown_nt4_0) \
 && !(defined mips_unknown_ce2_11) //ccw 20 july 2000 : 29 mar 2001
    int ret = 0;
    /* If we're not currently trying to load the runtime library,
       check whether this shared object is the runtime lib. */
    if (!(bootstrapState == loadingRT_bs)
        && (ret = check_rtinst(this, new_obj))) {
        if (ret == 1) {
            /* The runtime library has been loaded, but not initialized.
               Proceed anyway. */
            msg = pdstring("Application was linked with Dyninst/Paradyn runtime library -- this is not necessary");
            statusLine(msg.c_str());
        } else {
            /* The runtime library has been loaded into the inferior
               and previously initialized, probably by a previous
               run or Dyninst or Paradyn.  Bail.  */
	      if (ret == 2)
		   msg = pdstring("This process was previously modified by Dyninst -- cannot reattach");
	      else if (ret == 3)
		   msg = pdstring("This process was previously modified by Paradyn -- cannot reattach");
	      else
		   assert(0);
	      showErrorCallback(26, msg);
	      return false;
	 }
    }
#endif
        BPatch_process *bProc = BPatch::bpatch->getProcessByPid(pid);
        if (!bProc) return true; // Done
        BPatch_image *bImage = bProc->getImage();
        assert(bImage); // This we can assert to be true
    
        const pdvector<mapped_module *> &modlist = new_obj->getModules();

        // This should all be moved to the bpatch layer
        if (modlist.size()) {
            for (unsigned i = 0; i < modlist.size(); i++) {
                mapped_module *curr = modlist[i];
                
                BPatch_module *bpmod = bImage->findOrCreateModule(curr);
                
                pdvector<CallbackBase *> cbs;

                if (! getCBManager()->dispenseCallbacksMatching(evtLoadLibrary, cbs)) {
                   return true;
                 }

                for (unsigned int i = 0; i < cbs.size(); ++i) {
                  DynLibraryCallback &cb = *((DynLibraryCallback *) cbs[i]);
                  cb(bProc->threads[0], bpmod, true);
                 }
            }
        }
    
    return true;
}

// processSharedObjects: This routine is called before main() or on attach
// to an already running process.  It gets and process all shared objects
// that have been mapped into the process's address space
bool process::processSharedObjects() 
{
    if (mapped_objects.size() > 1) {
        // Already called... probably don't want to call again
        return true;
    }

    pdvector<mapped_object *> shared_objs;
    if (!dyn->getSharedObjects(shared_objs)) {
        startup_printf("dyn failed to get shared objects");
        return false;
    }
    statusLine("parsing shared object files");
#ifndef BPATCH_LIBRARY
    tp->resourceBatchMode(true);
#endif
    // for each element in shared_objects list process the 
    // image file to find new instrumentaiton points
    for(u_int j=0; j < shared_objs.size(); j++){
        // pdstring temp2 = pdstring(j);
        // 	    temp2 += pdstring(" ");
        // 	    temp2 += pdstring("the shared obj, addr: ");
        // 	    temp2 += pdstring(((*shared_objects)[j])->getBaseAddress());
        // 	    temp2 += pdstring(" name: ");
        // 	    temp2 += pdstring(((*shared_objects)[j])->getName());
        // 	    temp2 += pdstring("\n");
        // 	    logLine(P_strdup(temp2.c_str()));
        if (!addASharedObject(shared_objs[j]))
           logLine("Error after call to addASharedObject\n");
    }
#ifndef BPATCH_LIBRARY
    tp->resourceBatchMode(false);
#endif
    return true;
}

// findOneFunction: returns the function associated with func  
// this routine checks both the a.out image and any shared object
// images for this resource.
// Semantics of excluded functions - Once "exclude" works for both
// static and dynamically linked objects, this should return NULL
// if the function being sought is excluded....
#if 0
#ifndef BPATCH_LIBRARY
#include "paradynd/src/resource.h"
int_function *process::findOnlyOneFunction(resource *func, resource *mod){
    if((!func) || (!mod)) { return 0; }
    if(func->mdlType() != MDL_T_PROCEDURE) { return 0; }
    if(mod->mdlType() != MDL_T_MODULE) { return 0; }

    const pdvector<pdstring> &f_names = func->names();
    const pdvector<pdstring> &m_names = mod->names();
    pdstring func_name = f_names[f_names.size() -1]; 
    pdstring mod_name = m_names[m_names.size() -1]; 

    return findOnlyOneFunction(func_name, mod_name);
}

#if 0
bool process::findAllFuncsByName(resource *func, resource *mod, 
                                 pdvector<int_function *> &res) {
   
  pdvector<int_function *> *pdfv=NULL;
  if((!func) || (!mod)) { return 0; }
    if(func->mdlType() != MDL_T_PROCEDURE) { return 0; }
    if(mod->mdlType() != MDL_T_MODULE) { return 0; }

    const pdvector<pdstring> &f_names = func->names();
    const pdvector<pdstring> &m_names = mod->names();
    pdstring func_name = f_names[f_names.size() -1]; 
    pdstring mod_name = m_names[m_names.size() -1]; 

    //cerr << "process::findOneFunction called.  function name = " 
    //   << func_name.c_str() << endl;
    
    // KLUDGE: first search any shared libraries for the module name 
    //  (there is only one module in each shared library, and that 
    //  is the library name)
    if(dynamiclinking && shared_objects){
        for(u_int j=0; j < shared_objects->size(); j++){
            module *next = 0;
            next = ((*shared_objects)[j])->findModule(mod_name);

            if(next){
               if (NULL != (pdfv = ((*shared_objects)[j])->findFuncVectorByPretty(func_name))) {
                 for (unsigned int i = 0; i < pdfv->size(); ++i) {
                    res.push_back((*pdfv)[i]);
                 }
                 return true;
               }
            }
        }
    }

    if (NULL != (pdfv = symbols->findFuncVectorByPretty(func_name))) {
       for (unsigned int i = 0; i < pdfv->size(); ++i) {
          res.push_back((*pdfv)[i]);
       }
       return true;
    }
    return false;
}
#endif

#endif /* BPATCH_LIBRARY */
#endif

// Parse name into library name and function name. If no library is specified,
// lib gets the empty string "". 
//
// e.g. libc.so/read => lib_name = libc.so, func_name = read
//              read => lib_name = "", func_name = read
// We need to be careful about parsing for slashes -- "operator /" -- 
void process::getLibAndFunc(const pdstring &name, pdstring &lib_name, pdstring &func_name) {

  unsigned index = 0;
  unsigned length = name.length();
  bool hasNoLib = true;
 
  // clear the strings 
  lib_name = "";
  func_name = "";

  for (unsigned i = length-1; i > 0 && hasNoLib; i--) {
    if (name[i] == '/') {
      index = i;
      hasNoLib = false; 
    } 
  }

  if (hasNoLib) {
    // No library specified  
    func_name = name;
  } else { 
      // Grab library name and function name 
      lib_name = name.substr(0, index);
      func_name = name.substr(index+1, length-index);
  }
}


// Return true if library specified by lib_name matched name.
// This check is done ignoring the version number of name.
// Thus, for lib_name = libc.so, and name = libc.so.6, this function
// will return true (a match was made).
bool matchLibName(pdstring &lib_name, const pdstring &name) {

  // position in string "lib_name" where version information begins
  unsigned ln_index = lib_name.length();

  // position in string "name" where version information begins
  unsigned n_index = name.length();

  // Walk backwards from end of name, passing over the version number.
  // e.g. isolate the libc.so in libc.so.6
  while (isdigit(name[n_index-1]) || name[n_index-1] == '.') {
    n_index--;
  }

  // Walk backwards from end of lib_name, passing over the version number.
  // e.g. isolate the libc.so in libc.so.6
  while (isdigit(lib_name[ln_index-1]) || lib_name[ln_index-1] == '.') {
    ln_index--;
  }
 
  // If lib_name is the same as name (minus the version information, 
  // return true
  if ((lib_name.substr(0, ln_index)).wildcardEquiv(name.substr(0, n_index))) {
    return true;
  }  
 
  return false;
}

#if 0

// findOneFunction: returns the function associated with func  
// this routine checks both the a.out image and any shared object
// images for this resource
int_function *process::findOnlyOneFunction(const pdstring &name) const {

    pdstring lib_name;
    pdstring func_name;
    int_function *pdf, *ret = NULL;

    // Split name into library and function
    getLibAndFunc(name, lib_name, func_name);

    // If no library was specified, grab the first function we find
    if (lib_name == "") {
        // first check a.out for function symbol
        pdf = symbols->findOnlyOneFunction(func_name);
        if(pdf) {
            ret = pdf;
        }
        // search any shared libraries for the file name 
        if(dynamiclinking && shared_objects){
            for(u_int j=0; j < shared_objects->size(); j++){
                pdf = ((*shared_objects)[j])->findOnlyOneFunction(func_name);
                if(pdf){
                    // fail if we already found a match
                    if (ret) {
                        cerr << __FILE__ << ":" << __LINE__ << ": ERROR:  findOnlyOneFunction"
                             << " found more than one match for function " << func_name <<endl;
                        return NULL;
                    }
                    ret = pdf;
                }
                else {
                    // cerr << __FILE__ << ":" << __LINE__ << ": WARNING:  findOnlyOneFunction"
                    //<< " could not find function " << func_name << endl;
                }
            }
        }
    } else {
        
        // Search specified shared library for function 
        if(dynamiclinking && shared_objects){ 
            for(u_int j=0; j < shared_objects->size(); j++){
                shared_object *so = (*shared_objects)[j];
                
                // Add prefix wildcard to make name matching easy
                if (!lib_name.prefixed_by("*"))
                    lib_name = "*" + lib_name;             
                
                if(matchLibName(lib_name, so->getName())) {
                    int_function *fb = so->findOnlyOneFunction(func_name);
                    if (fb) {
                        if (ret) {
                            cerr << __FILE__ << ":" << __LINE__ << ": ERROR:  findOnlyOneFunction"
                                 << " found more than one match for function " << func_name <<endl;
                            return NULL;
                        }
                        ret = fb;
                        //cerr << "Found " << func_name << " in " << lib_name << endl;
                    }
                    else {
                        //cerr << __FILE__ << ":" << __LINE__ << ": WARNING:  findOnlyOneFunction"
                        //  << " could not find function " << func_name << " in module " << so->getName()<<endl;
                    }
                }
            }
        }
    }

    return ret;
}

bool process::findAllFuncsByName(const pdstring &name, pdvector<int_function *> &res)
{
   pdstring lib_name;
   pdstring func_name;
   pdvector<int_function *> *pdfv=NULL;
   
   // Split name into library and function
   getLibAndFunc(name, lib_name, func_name);
   
   // If no library was specified, grab the first function we find
   if (lib_name == "") {
      // first check a.out for function symbol
      if (NULL != (pdfv = symbols->findFuncVectorByPretty(func_name))) {
         for (unsigned int i = 0; i < pdfv->size(); ++i)
            res.push_back((*pdfv)[i]);
      }
      
      // search any shared libraries for the file name 
      if(dynamiclinking && shared_objects){
         for(u_int j=0; j < shared_objects->size(); j++){
            if (NULL != (pdfv = (*shared_objects)[j]->findFuncVectorByPretty(func_name))) {
                for (unsigned int i = 0; i < pdfv->size(); ++i) {
                    res.push_back((*pdfv)[i]);
                }
            }
            //pdf=static_cast<int_function *>(((*shared_objects)[j])->findFuncByName(func_name));
         }
      }      
   } else {
      // Library was specified, search lib for function func_name 
      // Add prefix wildcard to make name matching easy
       if (!lib_name.prefixed_by("*"))
           lib_name = "*" + lib_name;             
       
       if(dynamiclinking && shared_objects) { 
           for(u_int j=0; j < shared_objects->size(); j++){
               shared_object *so = (*shared_objects)[j];
               
               if(matchLibName(lib_name, so->getName())) {
                   if (NULL != (pdfv = so->findFuncVectorByPretty(func_name))) {
                       for (unsigned int i = 0; i < pdfv->size(); ++i) {
                           res.push_back((*pdfv)[i]);
                       }
                       
                   }
                   
                   //int_function *fb = static_cast<int_function *>(so->findFuncByName(func_name));
               }
           }
       }
   }
   
   if (res.size())
      return true; 

   //  Last ditch:  maybe the name was a mangled name

   if (NULL != (pdfv = symbols->findFuncVectorByMangled(func_name))) {
     for (unsigned i = 0; i < pdfv->size(); i++) 
       res.push_back((*pdfv)[i]);
   }
   
   // search any shared libraries for the file name 
   if(dynamiclinking && shared_objects){
     for(u_int j=0; j < shared_objects->size(); j++){
       if (NULL != (pdfv = (*shared_objects)[j]->findFuncVectorByMangled(func_name))) {
	 for (unsigned i = 0; i < pdfv->size(); i++) 
	   res.push_back((*pdfv)[i]);
       }
     }
   }

   if (res.size())
     return true; 

   return false;
}
#endif


// Returns the named symbol from the image or a shared object
bool process::getSymbolInfo( const pdstring &name, Symbol &ret ) 
{
    for (unsigned i = 0; i < mapped_objects.size(); i++) {
        bool sflag;
        sflag = mapped_objects[i]->getSymbolInfo( name, ret );
        
        if( sflag ) {
            return true;
        }
    }
    return false;
}

// findRangeByAddr: finds the object (see below) that corresponds with
// a given absolute address. This includes:
//   Functions (non-relocated)
//   Base tramps
//   Mini tramps
//   Relocated functions
//
// The process class keeps a tree of objects occupying the address space.
// This top-level tree includes trampolines, relocated functions, and the
// application and shared objects. The search starts at the top of this tree.
// If the address resolves to a base tramp, mini tramp, or relocated function,
// that is returned. If the address resolves within the range of an shared
// object, the search recurses into the object (the offset into the object
// is calculated and the function lookup works from the offset). If the offset
// is within the a.out, we look up the function assuming the address given is
// the offset. 

codeRange *process::findCodeRangeByAddress(Address addr) {

   codeRange *range = NULL;

   if (!codeRangesByAddr_.find(addr, range)) {
       return NULL;
   }
   
   assert(range);

   bool in_range = (addr >= range->get_address_cr() &&
                    addr <= (range->get_address_cr() + range->get_size_cr()));
   assert(in_range); // Supposed to return NULL if this is the case

   // The top level tree doesn't go into mapped_objects, which is not
   // what we want; so if we're in a mapped_object, poke inside.
   // However, if we're in a function (int_function), minitramp,
   // basetramp, ... return that right away.

   mapped_object *mobj = range->is_mapped_object();
   if (mobj) {
       codeRange *obj_range = mobj->findCodeRangeByAddress(addr);
       if (obj_range) range = obj_range;
   }

   return range;
}

int_function *process::findFuncByAddr(Address addr) {
    codeRange *range = findCodeRangeByAddress(addr);
    if (!range) return NULL;
    
    int_function *func_ptr = range->is_function();
    multiTramp *multi = range->is_multitramp();
    miniTrampInstance *mti = range->is_minitramp();

    if(func_ptr) {
       return func_ptr;
    }
    else if (multi) {
        return multi->func();
    }
    else if (mti) {
        return mti->baseTI->multiT->func();
    }
    else {
        return NULL;
    }
}

bool process::addCodeRange(codeRange *codeobj) {

   codeRangesByAddr_.insert(codeobj);
#if 0
   fprintf(stderr, "addCodeRange for %p\n", codeobj);
   if (dynamic_cast<multiTramp *>(codeobj))
       fprintf(stderr, "... multiTramp\n");
   if (dynamic_cast<miniTrampInstance *>(codeobj))
       fprintf(stderr, "... miniTramp\n");
   if (dynamic_cast<mapped_object *>(codeobj))
       fprintf(stderr, "... mobj\n");
#endif
   if (codeobj->is_mapped_object() ||
       codeobj->is_multitramp() ||
       codeobj->is_minitramp() ||
       codeobj->is_basicBlockInstance()) {
       // Chunk of executable code - add to codeSections_
       codeSections_.insert(codeobj);
#if 0
       codeRange *temp;
       if (!codeSections_.find(codeobj->get_address_cr(),
                               temp)) {
           codeSections_.insert(codeobj);
       }
#endif
       if (codeobj->is_mapped_object()) {
           // Hack... add data range
           mappedObjData *data = new mappedObjData(codeobj->is_mapped_object());
           dataSections_.insert(data);
       }
   }
   return true;
}

bool process::deleteCodeRange(Address addr) {
    codeRangesByAddr_.remove(addr);
    // Don't do this -- we clean that at exit/exec.
    //codeSections_.remove(addr);
    // Need to nuke data section as well...

    return true;
}

// Given an address, find the multiTramp covering that addr. 
multiTramp *process::findMultiTramp(Address addr) {
    codeRange *range;

    if (!modifiedAreas_.find(addr, range))
        return false;
    
    instArea *area = dynamic_cast<instArea *>(range);
    
    if (area)
        return area->multi;
    return NULL;
}

void process::addMultiTramp(multiTramp *newMulti) {
    assert(newMulti);
    assert(newMulti->instAddr());

    codeRange *range;
    if (modifiedAreas_.find(newMulti->instAddr(), range)) {
        // We're overriding. Keep the instArea but update pointer.
        instArea *area = dynamic_cast<instArea *>(range);
        // It could be something else, which should have been
        // caught already
        if (!area) {
            fprintf(stderr, "ERROR: failed to find mt at 0x%lx\n",
                    newMulti->instAddr());
        }
        assert(area);
        area->multi = newMulti;
        return;
    }
    else {
        instArea *area = new instArea(newMulti);
        modifiedAreas_.insert(area);
    }
}

void process::removeMultiTramp(multiTramp *oldMulti) {
    if (!oldMulti) return;

    assert(oldMulti->instAddr());

    // Already gone.
    if (findMultiTramp(oldMulti->instAddr()) != oldMulti) {
        return;
    }
    // Pull the corresponding instArea out of the tree.
    modifiedAreas_.remove(oldMulti->instAddr());
}

void process::addModifiedCallsite(replacedFunctionCall *RFC) {
    codeRange *range;
    if (modifiedAreas_.find(RFC->get_address_cr(), range)) {
        assert(dynamic_cast<replacedFunctionCall *>(range));
        modifiedAreas_.remove(RFC->get_address_cr());
        delete range;
    }
    assert(RFC);
    modifiedAreas_.insert(RFC);
    replacedFunctionCalls_[RFC->callAddr] = RFC;
}

void process::addFunctionReplacement(functionReplacement *fr,
                                     pdvector<codeRange *> &overwrittenObjs) {
    assert(fr);
    Address currAddr = fr->get_address_cr();
    codeRange *range;
    while (modifiedAreas_.find(currAddr, range)) {
        overwrittenObjs.push_back(range);
        modifiedAreas_.remove(currAddr);
        currAddr += range->get_size_cr();
    }

    modifiedAreas_.insert(fr);
}

codeRange *process::findModifiedPointByAddr(Address addr) {
    codeRange *range = NULL;
    if (modifiedAreas_.find(addr, range)) {
        assert(range);
        return range;
    }
    return NULL;
}

void process::removeModifiedPoint(Address addr) {
    modifiedAreas_.remove(addr);
}

Address process::getReplacedCallAddr(Address origAddr) const {
    if (replacedFunctionCalls_.defines(origAddr))
        return replacedFunctionCalls_[origAddr]->newTargetAddr;
    return 0;
}

bool process::wasCallReplaced(Address origAddr) const {
    if (replacedFunctionCalls_.find(origAddr))
        return true;
    else
        return false;
}

instPoint *process::findInstPByAddr(Address addr) {
    if (instPMapping_.defines(addr))
        return instPMapping_[addr];
    else 
        return NULL;
}

    
// findModule: returns the module associated with mod_name 
// this routine checks both the a.out image and any shared object
// images for this resource
mapped_module *process::findModule(const pdstring &mod_name, bool wildcard)
{
    // KLUDGE: first search any shared libraries for the module name 
    //  (there is only one module in each shared library, and that 
    //  is the library name)
    for(u_int j=0; j < mapped_objects.size(); j++){
        mapped_module *mod = mapped_objects[j]->findModule(mod_name, wildcard);
        if (mod) {
            return (mod);
        }
    }
    return NULL;
}

// findObject: returns the object associated with obj_name 
// This just iterates over the mapped object vector
mapped_object *process::findObject(const pdstring &obj_name, bool wildcard)
{
    for(u_int j=0; j < mapped_objects.size(); j++){
        if (mapped_objects[j]->fileName() == obj_name ||
            mapped_objects[j]->fullName() == obj_name ||
            (wildcard &&
             (obj_name.wildcardEquiv(mapped_objects[j]->fileName()) ||
              obj_name.wildcardEquiv(mapped_objects[j]->fullName()))))
            return mapped_objects[j];
    }
    return NULL;
}

// getAllFunctions: returns a vector of all functions defined in the
// a.out and in the shared objects

void process::getAllFunctions(pdvector<int_function *> &funcs) {
    for (unsigned i = 0; i < mapped_objects.size(); i++) {
        mapped_objects[i]->getAllFunctions(funcs);
    }
}
      
// getAllModules: returns a vector of all modules defined in the
// a.out and in the shared objects

void process::getAllModules(pdvector<mapped_module *> &mods){
    for (unsigned i = 0; i < mapped_objects.size(); i++) {
        const pdvector<mapped_module *> &obj_mods = mapped_objects[i]->getModules();
        for (unsigned j = 0; j < obj_mods.size(); j++) {
            mods.push_back(obj_mods[j]);
        }
    }
}

void process::addSignalHandler(Address addr, unsigned size) {
    signal_handler_location *newSig = new signal_handler_location(addr, 
                                                                  size);
    signalHandlerLocations_.insert(newSig);
}

// We keep a vector of all signal handler locations
void process::findSignalHandler(mapped_object *obj){
    assert(obj);
#if 0
    // For some reason by-func search is breaking. Argh. This is the
    // better way, too...
    const pdvector<int_function *> *funcs;
    pdstring signame(SIGNAL_HANDLER);
    funcs = obj->findFuncVectorByMangled(signame);
    if (funcs) {
        for (unsigned i = 0; i < funcs->size(); i++) {
            signal_handler_location *newSig = new signal_handler_location((*funcs)[i]->getAddress(),
                                                                          (*funcs)[i]->getSize());
            signalHandlerLocations_.insert(newSig);
        }
    }
#endif
    // Old skool
    Symbol sigSym;
    pdstring signame(SIGNAL_HANDLER);

    if (obj->getSymbolInfo(signame, sigSym)) {
        // Symbols often have a size of 0. This b0rks the codeRange code,
        // so override to 1 if this is true...
        unsigned size_to_use = sigSym.size();
        if (!size_to_use) size_to_use = 1;

        addSignalHandler(sigSym.addr(), size_to_use);
    }

}

bool process::continueProc(int signalToContinueWith) 
{
   
   signal_printf("%s[%d]: continuing process %d\n", FILE__, __LINE__, getPid());
  if (!isAttached()) {
    signal_printf("%s[%d]: warning continue on non-attached %d\n", 
                  FILE__, __LINE__, getPid());
    bpwarn( "Warning: continue attempted on non-attached process\n");
    fprintf(stderr, "%s[%d]:  continue attempted on non-attached process\n", FILE__, __LINE__);
    return false;
  }

  if (suppressEventConts()) {
    fprintf(stderr, "%s[%d][%s]:  suppressing continue\n", __FILE__, __LINE__, getThreadStr(getExecThreadID()));
    return false;
  }

  bool res = continueProc_(signalToContinueWith);
  if (!res) 
  {
         fprintf(stderr, "%s[%d]:  continueProc_ failed\n", FILE__, __LINE__);
    showErrorCallback(38, "System error: can't continue process");
    return false;
  }

  if (status_ != exited)
     status_ = running;

  if (getExecThreadID() != getSH()->getThreadID()) {
    signal_printf("%s[%d][%s]:  signalling active process\n", 
                  FILE__, __LINE__, getThreadStr(getExecThreadID()));
    getSH()->signalActiveProcess();
  }
  return true;
}

//Only different on Linux
#if !defined(os_linux)
bool process::continueProc_(int sig)
{
  if (status_ == running)
    return true;
  bool ret =  getRepresentativeLWP()->continueLWP(sig);
  getSH()->signalActiveProcess();
  return ret;
}
#endif

bool process::detachProcess(const bool leaveRunning) {
    // First, remove all syscall tracing and notifications
    delete tracedSyscalls_;
    tracedSyscalls_ = NULL;
    // Next, delete the dynamic linker instrumentation
    delete dyn;
    dyn = NULL;
    // Unset process flags
    unsetProcessFlags();
    // Detach from the application
    if (!detach(leaveRunning)) {
        fprintf(stderr, "Failed detach!\n");
        return false;
    }
    
    set_status(detached);
    // deleteProcess does the right thing depending on the status vrble
    deleteProcess();
    return true;
}

    

// Note: this may happen when the process is already gone. 
bool process::detach(const bool leaveRunning ) {

#if !defined(i386_unknown_linux2_0) \
 && !defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */ \
 && !defined(ia64_unknown_linux2_4)
    // Run the process if desired
    // Linux does not support this for two reasons: 1) the process must be paused
    // when we detach, and 2) the process is _always_ continued when we detach
    if (leaveRunning)
        continueProc();
#endif

    // Detach by detaching each LWP handle

    // On linux, if process found to be MT, there will be no representativeLWP
    // since there is no lwp which controls the entire process for MT linux.
    if(getRepresentativeLWP()) 
        getRepresentativeLWP()->detach();   
    
    dictionary_hash_iter<unsigned, dyn_lwp *> lwp_iter(real_lwps);
    dyn_lwp *lwp;
    unsigned index;
    while (lwp_iter.next(index, lwp)) {
        lwp->detach();
    }

#if defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */ \
 || defined(ia64_unknown_linux2_4)
    // On Lee-nucks the process occasionally does _not_ continue. On the
    // other hand, we've detached from it. So here we send a SIGCONT
    // to continue the proc (if the user wants it) and SIGSTOP to stop it
    if (leaveRunning)
        kill(getPid(), SIGCONT);
    else
        kill(getPid(), SIGSTOP);
#endif

    return true;
}

/*
 * Generic syscall exit handling.
 * Returns true if handling was done
 */

#if defined(alpha_dec_osf4_0)
bool process::handleSyscallExit(eventWhat_t syscall,
                                dyn_lwp *lwp_with_event)
#else
bool process::handleSyscallExit(eventWhat_t, dyn_lwp *lwp_with_event)
#endif
{
   // For each thread:
   // Get the LWP associated with the thread
   // Check to see if the LWP is at a syscall exit trap
   // Check to see if the trap is desired
   // Return the first thread to match the above conditions.
   bool lwp_with_trap_event = false;
   for (unsigned iter = 0; iter < threads.size(); iter++) {
      dyn_thread *thr = threads[iter];
      dyn_lwp *lwp = thr->get_lwp();
#if !defined(rs6000_ibm_aix4_1) || defined(AIX_PROC)  // non AIX-PTRACE
      if(lwp != lwp_with_event)
         continue;
#endif

      int match_type = lwp->hasReachedSyscallTrap();
      if (match_type == 0)
         continue; // No match
      else if (match_type == 1) {
         // Uhh... crud. 
         lwp->stepPastSyscallTrap();
         // Question: what to return? The trap was incorrect,
         // and as such should silently disappear. For now, return
         // the thread that hit the trap, and the caller should 
         // determine there is nothing to be done.
         //return true;  ... don't return here, need to look through
         //                  all lwps for match_type of 2
         lwp_with_trap_event = true;
      }
      else {
         lwp->clearSyscallExitTrap();
         lwp_with_trap_event = true;
      }
   }
   
   return lwp_with_trap_event;
}

void process::triggerNormalExitCallback(int exitCode) 
{
   // special case where can't wait to continue process
   if (status() == exited) {
      fprintf(stderr, "%s[%d]:  cannot trigger exit callback, process is gone...\n", __FILE__, __LINE__);
      return;
   }
   BPatch::bpatch->registerNormalExit(this, exitCode);
}

void process::triggerSignalExitCallback(int signalnum) 
{
   // special case where can't wait to continue process
   if (status() == exited) {
      //fprintf(stderr, "%s[%d]:  cannot trigger exit callback, process already exited\n", 
      //        FILE__, __LINE__);
      return;
   }
   BPatch::bpatch->registerSignalExit(this, signalnum);
}

bool process::handleProcessExit() 
{

   // special case where can't wait to continue process
   if (status() == exited) {
      signal_printf("%s[%d]:  cannot detach from process, process already exited\n", 
              FILE__, __LINE__);
      return true;
     //return false;
   }
   --activeProcesses;

   // make exited is set, so detach doesn't try impossible things
   set_status(exited);

   if (!detach(false)) return false;

   return true;
}
        

/*
 * handleForkEntry: do anything necessary when a fork is entered
 */

bool process::handleForkEntry() {

    // Make bpatch callbacks as well
    BPatch::bpatch->registerForkingProcess(getPid(), NULL);
    return true;
}

bool process::handleForkExit(process *child) {
    BPatch::bpatch->registerForkedProcess(getPid(), child->getPid(), child);
    return true;
}

bool process::handleExecEntry(char *arg0) {
    nextTrapIsExec = true;
    execPathArg = "";
    // The arg0 is an address in the mutatee's space
    char temp[512];
    if (!readDataSpace(arg0, 512, temp, false)) {
        cerr << "Failed to read exec argument!" << endl;
        return false;
    }
    else
        execPathArg = temp;
    // /* DEBUG */ cerr << "Exec path arg is " << execPathArg << endl;
   return true;

    // We now wait for exec to finish. We often see very many exec
    // entries (with attempted paths), but only one exit.
}

/* process::handleExecExit: called when a process successfully exec's.
   Parse the new image, disable metric instances on the old image, create a
   new (and blank) shm segment.  The process hasn't yet bootstrapped, so we
   mustn't try to enable anything...
   We finish by bootstrapping here (actually, we call setupExec)
*/
bool process::handleExecExit() 
{
    fprintf(stderr, "%s[%d]:  welcome to handleExecExit\n", FILE__, __LINE__);
    inExec_ = true;
    // NOTE: for shm sampling, the shm segment has been removed, so we
    //       mustn't try to disable any dataReqNodes in the standard way...
    nextTrapIsExec = false;

   // Should probably be renamed to clearProcess... deletes anything
   // unnecessary
   deleteProcess();

   prepareExec();
   // The companion finishExec gets called from unix.C...

   // Do not call this here; instead we call it at the end of finalizeDyninstLib
   // so that dyninsts are available
   //BPatch::bpatch->registerExec(this);

   return true;
}

bool process::checkTrappedSyscallsInternal(Address syscall)
{
    for (unsigned i = 0; i < syscallTraps_.size(); i++) {
        if (syscall == syscallTraps_[i]->syscall_id)
            return true;
    }
    
    return false;
}

#if defined(rs6000_ibm_aix4_1) && defined(BPATCH_LIBRARY)
//When we save the world on AIX we must instrument the
//beginning of main() to call dlopen() to load the
//dyninst rt shared library.  The file format of an
//AIX exec does not easily allow dyninst to add the
//dyninst rt lib to the list of libraries to load upon
//startup.  This is why we use dlopen().  If the
//instrumentation that calls DYNINSTinit() is placed
//at the beginning of main() when this dlopen() call
//is inserted, bad things happen.  So we remove
//the call to DYNINSTinit() and replace it with the
//call to dlopen() when we save the world.  
//The handle declared below denotes the DYNINSTinit()
//instrumentation.  It is deleted once the mutatee
//has executed DYNINSTinit() and is ready to execute the
//start of main().

BPatchSnippetHandle *handle; //ccw 17 jul 2002
#endif

void process::registerInstPointAddr(Address addr, instPoint *inst) {
    if (instPMapping_.defines(addr)) {
        // No silently overriding instPoints
        assert(instPMapping_[addr] == inst);
    }
    else {
        instPMapping_[addr] = inst;
    }
}

void process::unregisterInstPointAddr(Address addr, instPoint *inst) {
    if (instPMapping_.defines(addr)) {
        // No silently overriding instPoints
        assert(instPMapping_[addr] == inst);
        instPMapping_.undef(addr);
    }
}

void process::installInstrRequests(const pdvector<instMapping*> &requests) {
    for (unsigned lcv=0; lcv < requests.size(); lcv++) {
        instMapping *req = requests[lcv];
        
        if(!multithread_capable() && req->is_MTonly())
            continue;

        pdvector<int_function *> matchingFuncs;
        
        findFuncsByAll(req->func, matchingFuncs);

        for (unsigned funcIter = 0; funcIter < matchingFuncs.size(); funcIter++) {
         int_function *func = matchingFuncs[funcIter];
         if (!func) {
            continue;  // probably should have a flag telling us whether errors
         }
         
         // should be silently handled or not
         AstNode *ast;
         if ((req->where & FUNC_ARG) && req->args.size()>0) {
            ast = new AstNode(req->inst, req->args);
         } else {
            AstNode *tmp = new AstNode(AstNode::Constant, (void*)0);
            ast = new AstNode(req->inst, tmp);
            removeAst(tmp);
         }
         if (req->where & FUNC_EXIT) {
             const pdvector<instPoint*> func_rets = func->funcExits();
             bool mtramp = BPatch::bpatch->isMergeTramp();
             BPatch::bpatch->setMergeTramp(false);
             for (unsigned j=0; j < func_rets.size(); j++) {
                 instPoint *func_ret = func_rets[j];
                 miniTramp *mt = func_ret->instrument(ast,
                                                      req->when,
                                                      req->order,
                                                      (!req->useTrampGuard),
                                                      false);
                 if (mt)
                     req->miniTramps.push_back(mt);
             }

             BPatch::bpatch->setMergeTramp(mtramp);             
         }
         
         if (req->where & FUNC_ENTRY) {
             const pdvector<instPoint *> func_entries = func->funcEntries();
             bool mtramp = BPatch::bpatch->isMergeTramp();
             BPatch::bpatch->setMergeTramp(false);
             for (unsigned k=0; k < func_entries.size(); k++) {
                 instPoint *func_ent = func_entries[k];
                 miniTramp *mt = func_ent->instrument(ast,
                                                      req->when,
                                                      req->order,
                                                      (!req->useTrampGuard),
                                                      false); // nocost
                 if (mt)
                     req->miniTramps.push_back(mt);
             }
             BPatch::bpatch->setMergeTramp(mtramp);
         }
         
         if (req->where & FUNC_CALL) {
             pdvector<instPoint*> func_calls = func->funcCalls();
             bool mtramp = BPatch::bpatch->isMergeTramp();
             BPatch::bpatch->setMergeTramp(false);

             for (unsigned l=0; l < func_calls.size(); l++) {
                 miniTramp *mt = func_calls[l]->instrument(ast,
                                                           req->when,
                                                           req->order,
                                                           (!req->useTrampGuard),
                                                           false);
                 if (mt) req->miniTramps.push_back(mt);
             }
             BPatch::bpatch->setMergeTramp(mtramp);
         }
         removeAst(ast);
        }
    }
}


bool process::extractBootstrapStruct(DYNINST_bootstrapStruct *bs_record)
{
  const pdstring vrbleName = "DYNINST_bootstrap_info";

  pdvector<int_variable *> bootstrapInfoVec;
  if (!findVarsByAll(vrbleName, bootstrapInfoVec))
      assert(0);
  assert(bootstrapInfoVec.size() == 1);

  Address symAddr = bootstrapInfoVec[0]->getAddress();

  // bulk read of bootstrap structure
  if (!readDataSpace((const void*)symAddr, sizeof(*bs_record), bs_record, true)) {
    cerr << "extractBootstrapStruct failed because readDataSpace failed" << endl;
    return false;
  }
  return true;
}

bool process::isAttached() const {
    if (status_ == exited ||
        status_ == detached ||
	status_ == deleted)
        return false;
    else
        return true;
}

bool process::isStopped() const {
    if (!isAttached()) return false;
    if (status_ == running) return false;
    // What about "neonatal"?
    return true;
}

bool process::hasExited() const {
    return (status_ == exited ||
	    status_ == deleted);
}

bool process::dumpCore(const pdstring fileOut) {
  bool res = dumpCore_(fileOut);
  if (!res) {
    return false;
  }
  return true;
}

process *process::findProcess(int pid) {
  unsigned size=processVec.size();
  for (unsigned u=0; u<size; u++)
    if (processVec[u] && processVec[u]->getPid() == pid)
      return processVec[u];
  return NULL;
}

pdstring process::getBootstrapStateAsString() const {
   // useful for debugging
   switch(bootstrapState) {
     case unstarted_bs:
        return "unstarted";
     case begun_bs:
        return "begun";
   case attached_bs:
       return "attached";
     case initialized_bs:
        return "initialized";
     case loadingRT_bs:
        return "loadingRT";
     case loadedRT_bs:
        return "loadedRT";
     case bootstrapped_bs:
        return "bootstrapped";
   }
   assert(false);
   return "???";
}

pdstring process::getStatusAsString() const {
   // useful for debugging
   if (status_ == neonatal)
      return "neonatal";
   if (status_ == stopped)
      return "stopped";
   if (status_ == running)
      return "running";
   if (status_ == exited)
      return "exited";
   if (status_ == detached)
       return "detached";
   assert(false);
   return "???";
}

bool process::uninstallMutations() {
    // For each multiTramp drop in the original instructions
    dictionary_hash_iter<int, multiTramp *> multiTrampIter(multiTrampDict);
    multiTramp *mTramp;
    for (; multiTrampIter; multiTrampIter++) {
        mTramp = multiTrampIter.currval();
        if (mTramp) {
            mTramp->disable();
        }
        
    }

    dictionary_hash_iter<Address, replacedFunctionCall *> rfcIter(replacedFunctionCalls_);
    Address rfcAddr;
    replacedFunctionCall *rfcVal;
    for (; rfcIter; rfcIter++) {
        rfcAddr = rfcIter.currkey();
        rfcVal = rfcIter.currval();
        assert(rfcAddr);
        assert(rfcVal);
        assert(rfcAddr == rfcVal->callAddr);
        writeDataSpace((void *)rfcAddr,
                       rfcVal->oldCall.used(),
                       rfcVal->oldCall.start_ptr());
    }

    return true;
}

bool process::reinstallMutations() {
    // For each multiTramp drop in the jump instructions
    dictionary_hash_iter<int, multiTramp *> multiTrampIter(multiTrampDict);
    multiTramp *mTramp;
    for (; multiTrampIter; multiTrampIter++) {
        mTramp = multiTrampIter.currval();
        if (mTramp) 
            mTramp->enable();
    }

    dictionary_hash_iter<Address, replacedFunctionCall *> rfcIter(replacedFunctionCalls_);
    Address rfcAddr;
    replacedFunctionCall *rfcVal;
    for (; rfcIter; rfcIter++) {
        rfcAddr = rfcIter.currkey();
        rfcVal = rfcIter.currval();
        assert(rfcAddr);
        assert(rfcVal);
        assert(rfcAddr == rfcVal->callAddr);
        writeDataSpace((void *)rfcAddr,
                       rfcVal->newCall.used(),
                       rfcVal->newCall.start_ptr());
    }

    return true;
}

// Add it at the bottom...
void process::deleteGeneratedCode(generatedCodeObject *delInst)
{
  // Add to the list and deal with it later.
  // The question is then, when to GC. I'd suggest
  // when we try to allocate memory, and leave
  // it a public member that can be called when
  // necessary

#if 0
    fprintf(stderr, "Deleting generated code %p, which is a:\n",
            delInst);
    if (dynamic_cast<multiTramp *>(delInst)) {
        fprintf(stderr, "   multiTramp\n");
    }
    else if (dynamic_cast<baseTrampInstance *>(delInst)) {
        fprintf(stderr, "   baseTramp\n");
    } 
    else if (dynamic_cast<miniTrampInstance *>(delInst)) {
        fprintf(stderr, "   miniTramp\n");
    }
    else {
        fprintf(stderr, "   unknown\n");
    }
#endif    
    // Make sure we don't double-add
    for (unsigned i = 0; i < pendingGCInstrumentation.size(); i++)
        if (pendingGCInstrumentation[i] == delInst)
            return;

    pendingGCInstrumentation.push_back(delInst);
}

// Function relocation requires a version of process::convertPCsToFuncs 
// in which null functions are not passed into ret. - Itai 
pdvector<int_function *> process::pcsToFuncs(pdvector<Frame> stackWalk) {
    pdvector <int_function *> ret;
    unsigned i;
    int_function *fn;
    for(i=0;i<stackWalk.size();i++) {
        fn = (int_function *)findFuncByAddr(stackWalk[i].getPC());
        // no reason to add a null function to ret
        if (fn != 0) ret.push_back(fn);
    }
    return ret;
}

// garbage collect instrumentation
void process::gcInstrumentation()
{
  // The without-a-passed-in-stackwalk version. Walk the stack
  // and pass it down.
  // First, idiot check...
  if (status() == exited)
    return; 

  if (pendingGCInstrumentation.size() == 0)
    return;

  // We need to pause the process. Otherwise we could have an incorrect
  // stack walk, etc. etc. etc. blahblahblah

  bool wasPaused = true;

  if (status() == running) wasPaused = false;
 
  if (!wasPaused && !pause()) {
    return;
  }

  pdvector< pdvector<Frame> > stackWalks;
  if (!walkStacks(stackWalks)) return;

  gcInstrumentation(stackWalks);
  if(!wasPaused) {
    continueProc();
  }
}

// garbage collect instrumentation
void process::gcInstrumentation(pdvector<pdvector<Frame> > &stackWalks)
{
  // Go through the list and try to clear out any
  // instInstances that are freeable.
  if (status() == exited) return;

  // This is seriously optimizable -- go by the stack walks first,
  // and label each item as to whether it is deletable or not,
  // then handle them all at once.

  if (pendingGCInstrumentation.size() == 0) return;

  for (unsigned deletedIter = 0; 
       deletedIter < pendingGCInstrumentation.size(); 
       deletedIter++) {

      generatedCodeObject *deletedInst = pendingGCInstrumentation[deletedIter];
      bool safeToDelete = true;
      
      for (unsigned threadIter = 0;
           threadIter < stackWalks.size(); 
           threadIter++) {
          pdvector<Frame> stackWalk = stackWalks[threadIter];
          for (unsigned walkIter = 0;
               walkIter < stackWalk.size();
               walkIter++) {
              
              Frame frame = stackWalk[walkIter];
              codeRange *range = frame.getRange();
              
              if (!range) {
                  // Odd... couldn't find a match at this PC
                  // Do we want to skip GCing in this case? Problem
                  // is, we often see garbage at the end of stack walks.
                  continue;
              }
              safeToDelete = deletedInst->safeToFree(range);
              
              // If we can't delete, don't bother to continue checking
              if (!safeToDelete)
                  break;
          }
          // Same as above... pop out.
          if (!safeToDelete)
              break;
      }
      if (safeToDelete) {
          // Delete from list of GCs
          // Vector deletion is slow... so copy the last item in the list to
          // the current position. We could also set this one to NULL, but that
          // means the GC vector could get very, very large.
          pendingGCInstrumentation[deletedIter] = 
              pendingGCInstrumentation.back();
          // Lop off the last one
          pendingGCInstrumentation.pop_back();
          // Back up iterator to cover the fresh one
          deletedIter--;
          delete deletedInst;
      }
  }
}

dyn_thread *process::createInitialThread() 
{
#if defined(os_windows)
    // Windows has its own startup technique...
    assert(threads.size() > 0);
    return threads[0];
#else
   assert(threads.size() == 0);
   dyn_thread *initialThread = new dyn_thread(this);
   return initialThread;
#endif
}

dyn_thread *process::getThread(dynthread_t tid) {
   dyn_thread *thr;
   for(unsigned i=0; i<threads.size(); i++) {
      thr = threads[i];
      if(thr->get_tid() == tid)
         return thr;
   }
   return NULL;
}

// Question: if we don't find, do we create?
// For now, yes.

dyn_lwp *process::getLWP(unsigned lwp_id)
{
  dyn_lwp *foundLWP;
  if(real_lwps.find(lwp_id, foundLWP)) {
    return foundLWP;
  }
  if (representativeLWP && representativeLWP->get_lwp_id() == lwp_id)
    return representativeLWP;

  foundLWP = createRealLWP(lwp_id, lwp_id);

  if (!foundLWP->attach()) {
     deleteLWP(foundLWP);
     return NULL;
  }

  if (IndependentLwpControl())
  {
     if (status_ == running)
        set_lwp_status(foundLWP, running);
     else if (status_ == stopped)
        set_lwp_status(foundLWP, stopped);
  }
  return foundLWP;
}

dyn_lwp *process::lookupLWP(unsigned lwp_id) {
   dyn_lwp *foundLWP = NULL;
   bool found = real_lwps.find(lwp_id, foundLWP);
   if(! found) {
      dyn_lwp *repLWP = getRepresentativeLWP();
      if(repLWP && lwp_id == repLWP->get_lwp_id())
         foundLWP = getRepresentativeLWP();
   }
   return foundLWP;
}

// fictional lwps aren't saved in the real_lwps vector
dyn_lwp *process::createFictionalLWP(unsigned lwp_id) {
   dyn_lwp *lwp = new dyn_lwp(lwp_id, this);
   if (theRpcMgr) // We may not have a manager yet (fork case)
       theRpcMgr->addLWP(lwp);
   return lwp;
}

dyn_lwp *process::createRealLWP(unsigned lwp_id, int /*lwp_index*/) {
   dyn_lwp *lwp = new dyn_lwp(lwp_id, this);
   real_lwps[lwp_id] = lwp;
   if (theRpcMgr) // We may not have a manager (fork case)
       theRpcMgr->addLWP(lwp);
   return lwp;
}

void process::deleteLWP(dyn_lwp *lwp_to_delete) {
   if(real_lwps.size() > 0 && lwp_to_delete!=NULL) {
      theRpcMgr->deleteLWP(lwp_to_delete);
      lwp_to_delete->get_lwp_id();
      real_lwps.undef(lwp_to_delete->get_lwp_id());
   }
   if (lwp_to_delete == representativeLWP)
      representativeLWP = NULL;
   delete lwp_to_delete;
}


// Called for new threads
void process::deleteThread(dynthread_t tid)
{
  processState newst = running;
  pdvector<dyn_thread *>::iterator iter = threads.end();
  while(iter != threads.begin()) {
    dyn_thread *thr = *(--iter);
    dyn_lwp *lwp = thr->get_lwp();
    //Find the deleted thread
    if(thr->get_tid() != tid) 
    {
      //Update the process state, since deleting a thread may change it.
      if (lwp->status() == stopped)
         newst = stopped;
      continue;
    } 
    //Delete the thread
    getRpcMgr()->deleteThread(thr);
    delete thr;

    //Delete the lwp below the thread
    deleteLWP(lwp);

    threads.erase(iter);  
  }
  
  if (threads.size() && (status_ == running || status_ == stopped))
    status_ = newst;
}

// Pull whatever is in the slot out of the inferior process
unsigned process::getIndexToThread(unsigned index) {
    unsigned val;
    
    readDataSpace((void *)(threadIndexAddr + (index * sizeof(unsigned))),
                  sizeof(unsigned),
                  (void *)&val,
                  true);
    return val;
}

void process::setIndexToThread(unsigned index, unsigned value) {
   bool err =  writeDataSpace((void *)(threadIndexAddr + (index * sizeof(unsigned))),
                   sizeof(unsigned),
                   (void *)&value);
   if (!err) fprintf(stderr, "%s[%d][%s]:  writeDataSpace failed\n", __FILE__, __LINE__, getThreadStr(getExecThreadID()));
        assert(err);
}

void process::updateThreadIndexAddr(Address addr) {
    threadIndexAddr = addr;
}

//used to pass multiple values with the doneRegistering callback
typedef struct done_reg_bundle_t {
   pdvector<int> *lwps;
   pdvector<int> *indexes;
   unsigned *num_completed;
   int this_lwp;
} done_reg_bundle_t;

static void doneRegistering(process *, unsigned, void *data, void *result) 
{
   done_reg_bundle_t *pairs = (done_reg_bundle_t *) data;
   
   int index = (int) result;
   int lwp_id = pairs->this_lwp;

   pairs->lwps->push_back(lwp_id);
   pairs->indexes->push_back(index);
   (*pairs->num_completed)++;
   free(pairs);
}

void process::recognize_threads(const process *parent) 
{
  pdvector<unsigned> lwp_ids;
  unsigned i;
  int result;

  if (!multithread_capable()) {
     // Easy case
     if (parent) {
        assert(parent->threads.size() == 1);
        new dyn_thread(parent->threads[0], this);
        //The constructor automatically adds the thread to the process list,
        // thus it's safe to not keep a pointer to the new thread
     }
     return;
  }

  result = determineLWPs(lwp_ids);
  if (!result)
  {
     forkexec_printf("Error. Recognize_threads couldn't determine LWPs\n");
     return;
  }     

  //If !parent, then we're not a forked process and we can just assign the threads to
  //LWPs arbitrarly.  This is most used in the attach/create cases. 
  if (!parent)
  {
     unsigned num_completed = 0;
     //Parallel arrays for simplicity
     pdvector<int> ret_indexes;
     pdvector<int> ret_lwps;
     assert(status() == stopped);


     /**
      * Step 1: Find the lwp ids running in this process 
      *         (done above in determineLWPs)
      * Step 2: Run DYNINSTthreadIndex as an iRPC on each lwp
      * Step 3: Have doneRegistering() map the return values of 
      *         DYNINSTthreadIndex to the lwp we ran the iRPC on
      * Step 4: Create (if it doesn't exist) threads using the index and
      *         lwps
      **/
     for (i = 0; i < lwp_ids.size(); i++)
     {
        unsigned lwp_id = lwp_ids[i];
        dyn_lwp *lwp = getLWP(lwp_id);
        
        pdvector<AstNode *> ast_args;
        AstNode *ast = new AstNode("DYNINSTthreadIndex", ast_args);

        done_reg_bundle_t *bundle = (done_reg_bundle_t*) 
           malloc(sizeof(done_reg_bundle_t));
        bundle->indexes = &ret_indexes;
        bundle->lwps = &ret_lwps;
        bundle->this_lwp = lwp_id;
        bundle->num_completed = &num_completed;
        getRpcMgr()->postRPCtoDo(ast, true, doneRegistering, bundle, false, 
                                 NULL, lwp);
     }

     inferiorrpc_printf("%s[%d]:  waiting for rpc completion\n", FILE__, __LINE__);
     while(num_completed != lwp_ids.size())
     {
        getRpcMgr()->launchRPCs(false);
        getMailbox()->executeCallbacks(FILE__, __LINE__);
        if(hasExited()) {
           fprintf(stderr, "%s[%d]:  unexpected process exit\n", FILE__, __LINE__);
           return;
        }
        getSH()->waitForEvent(evtRPCSignal, this, NULL /*lwp*/, statusRPCDone);
        getMailbox()->executeCallbacks(FILE__, __LINE__);
     }

     assert(ret_lwps.size() == lwp_ids.size());
     assert(ret_lwps.size() == ret_indexes.size());

     assert(status() == stopped);
 
   for (i = 0; i < ret_lwps.size(); ++i) {
     BPatch_process *bproc = BPatch::bpatch->getProcessByPid(getPid());
     BPatch_thread *bpthrd = bproc->getThreadByIndex(ret_indexes[i]);
     if (bpthrd && bpthrd->updated) {
        //Already 
     }
     else {
       //fprintf(stderr, "%s[%d]:  before createOrUpdateBPThread: index %d\n", FILE__, __LINE__,ret_indexes[i]); 
       bproc->createOrUpdateBPThread(ret_lwps[i], (dynthread_t) -1, ret_indexes[i], 0, 0);
     }
   }

    assert(status() == stopped);
    return;
  }
  
  // We have LWPs with objects. The parent has a vector of threads.
  // Hook them up.
  for (unsigned thr_iter = 0; thr_iter < parent->threads.size(); thr_iter++) {
     unsigned matching_lwp = 0;
     dyn_thread *par_thread = parent->threads[thr_iter];
     forkexec_printf("Updating thread %d (tid %d)\n",
                     thr_iter, par_thread->get_tid());
     threads.clear();
  
     for (unsigned lwp_iter = 0; lwp_iter < lwp_ids.size(); lwp_iter++) {
        if (lwp_ids[lwp_iter] == 0) continue;
        dyn_lwp *lwp = getLWP(lwp_ids[lwp_iter]);
        
        forkexec_printf("... checking against LWP %d\n", lwp->get_lwp_id());
        
        if (par_thread->get_lwp()->executingSystemCall()) {
           // Must be the forking thread. Look for an LWP in a matching call
           Address par_syscall = par_thread->get_lwp()->getCurrentSyscall();
           if (!lwp->executingSystemCall())
              continue;
           Address cur_syscall = lwp->getCurrentSyscall();
           if (par_syscall == cur_syscall) {
              matching_lwp = lwp_ids[lwp_iter];
              forkexec_printf("... Match: syscall %d = %d\n",
                              par_syscall, cur_syscall);
              break;
           }
        }
        else {
           // Not in a system call, match active frames
           Frame parFrame = par_thread->get_lwp()->getActiveFrame();
           Frame lwpFrame = lwp->getActiveFrame();
           if ((parFrame.getPC() == lwpFrame.getPC()) &&
               (parFrame.getFP() == lwpFrame.getFP()) &&
               (parFrame.getSP() == lwpFrame.getSP())) {
              forkexec_cerr << "... Match: " << lwpFrame << endl;
              matching_lwp = lwp_ids[lwp_iter];
              break;
           }
        }
     }
     if (matching_lwp) {
        // Make a new thread with details from the old
        dyn_thread *new_thr = new dyn_thread(par_thread, this);
        new_thr->update_lwp(getLWP(matching_lwp));
        threads.push_back(new_thr);
        forkexec_printf("Creating new thread %d (tid %d) on lwp %d, parent was on %d\n",
                        thr_iter, new_thr->get_tid(), new_thr->get_lwp()->get_lwp_id(),
                        par_thread->get_lwp()->get_lwp_id());
     }
     else {
        forkexec_printf("Failed to find match for thread %d (tid %d), assuming deleted\n",
                        thr_iter, par_thread->get_tid());
     }
  }
  return;
}

int process::maxNumberOfThreads()
{
   if (!multithread_capable())
      return 1;
   return max_number_of_threads; 
}
// vim:set ts=5:


bool process::isBootstrappedYet() const {
   return bootstrapState == bootstrapped_bs;
}

static void mapIndexToTid_cb(process *, unsigned, void *data, void *result)
{
   dynthread_t *tid = (dynthread_t *) data;
   *tid = (dynthread_t) result;
}

//Turn a thread index into a tid via an iRPC
dynthread_t process::mapIndexToTid(int index)
{
   dynthread_t tid = (dynthread_t) -1;
   pdvector<AstNode *> ast_args;
   AstNode arg1(AstNode::Constant, (void *) index);
   ast_args.push_back(&arg1);
   AstNode call_get_tid("DYNINST_getThreadFromIndex", ast_args);

   getRpcMgr()->postRPCtoDo(&call_get_tid, true, mapIndexToTid_cb, &tid,
                            false, NULL, NULL);

     inferiorrpc_printf("%s[%d]:  waiting for rpc completion\n", FILE__, __LINE__);
   while (tid == -1)
   {
      getRpcMgr()->launchRPCs(false);
      getMailbox()->executeCallbacks(FILE__, __LINE__);
      if(hasExited()) return (dynthread_t) -1;
      getSH()->waitForEvent(evtRPCSignal, this, NULL /*lwp*/, statusRPCDone);
      getMailbox()->executeCallbacks(FILE__, __LINE__);
      //getSH()->checkForAndHandleProcessEvents(false);
   }

   return tid;
}

void process::stepi() {
   //Not using default parameters due to problems with gdb, which these
   //functions were meant to be called from.
   stepi(true, -1);
}

void process::stepi(int lwp) {
   //Not using default parameters due to problems with gdb, which these
   //functions were meant to be called from.
   stepi(true, lwp);
}

Address process::stepi(bool verbose, int lwp) {
   /**
    * Safety checking and warning messages
    **/
   if (status() == detached || status() == exited)
   {
      if (verbose) {
         fprintf(stderr, "[stepi @ %s:%u] - Error. Process %d no longer " 
                 "exists.\n",  __FILE__, __LINE__, getPid());
      }
      return (Address) -1;
   }
   if (status() == running)
   {
      if (verbose) {
         fprintf(stderr, "[stepi @ %s:%u] - Warning. Process %d was running.\n",
                 __FILE__, __LINE__, getPid());
      }
      bool result = pause();
      if (!result)
      {
         if (verbose) {
            fprintf(stderr, "[stepi @ %s:%u] - Error. Couldn't stop %d.\n" 
                    __FILE__, __LINE__, getPid());
         }
         return (Address) -1;
      }
   }
   
   /**
    * Step the process forward one instruction.  If we're being verbose
    * then get a code range for the next instruction and print some information.
    **/
   dyn_lwp *lwp_to_step;
   if (lwp == -1)
      lwp_to_step = getRepresentativeLWP();
   else {
      lwp_to_step = lookupLWP(lwp);
      if (!lwp_to_step) {
         if (verbose)
            fprintf(stderr, "[step @ %s:%u] - Couldn't find lwp %d\n",
                    __FILE__, __LINE__, lwp);
         return (Address) -1;
      }
   }
      
   Address nexti = lwp_to_step->step_next_insn();
   
   if ((nexti == (Address) -1) || lwp_to_step->status() != stopped)
   {
      if (verbose) {
         fprintf(stderr, "[stepi @ %s:%u] - Warning. Couldn't step.\n",
                 __FILE__, __LINE__, getPid());
      }
   }

   /**
    * Print the results if they're wanted.
    **/
   if (!verbose)
      return nexti;
   
   codeRange *range = findCodeRangeByAddress(nexti);

   fprintf(stderr, "0x%lx ", nexti);
   if (range)
      range->print_range();
   else
      fprintf(stderr, "\n");
   return nexti;
}

void process::disass(Address start, Address end) {
   disass(start, end, false);
}

void process::disass(Address start, Address end, bool leave_files) {
   int size = end - start;
   if (size < 0)
      return;

   unsigned char *buffer = (unsigned char *) malloc(size);
   readDataSpace((const void *)start, size, buffer, true);
   print_instrucs(buffer, size, leave_files);
}

#define MAXLINE 128
void process::print_instrucs(unsigned char *buffer, unsigned size, 
                                    bool leave_files) 
{
   FILE *f;
   char *result;
   bool success;
   char line[MAXLINE];
   char cname[32] = "dyndisXXXXXX";
   char oname[32] = "dynobjXXXXXX"; 
   char tname[32] = "dyntmpXXXXXX";
   OS::make_tempfile(cname);
   OS::make_tempfile(oname);
   OS::make_tempfile(tname);
   strcat(cname, ".c");
   strcat(oname, ".o");
   strcat(tname, ".tmp");

   f = fopen(cname, "w");
   if (!f) { 
      fprintf(stderr, "%s ", cname);
      perror("couldn't be opened");
      return;
   }

   fprintf(f, "unsigned char DyninstDisass[] = { ");
   for (unsigned i=0; i<size-1; i++)
      fprintf(f, "%u, ", (unsigned) buffer[i]);
   fprintf(f, "%u };\n", (unsigned) buffer[size-1]);
   fclose(f);

   sprintf(line, "gcc -c -o %s %s", oname, cname);
   success = OS::execute_file(line);
   if (!success) {
      return;
   }

#if !defined(os_windows)
   sprintf(line, "objdump -D %s > %s", oname, tname);
#else
   sprintf(line, "objdump -D %s", oname);
#endif
   success = OS::execute_file(line);
   if (!success) {
      return;
   }
   
   f = fopen(tname, "r");
   if (f)
   {
      while (true) {
         result = fgets(line, MAXLINE, f);
         if (!result)
            break;
         if (strstr(line, "DyninstDisass"))
            break;
      }
      while (true) {
         result = fgets(line, MAXLINE, f);
         if (!result)
            break;
         if (strstr(line, "Disassembly of section"))
            break;
         fprintf(stderr, "%s", line);
      }
   }
   if (!leave_files) {
      OS::unlink(oname);
      OS::unlink(cname);
      OS::unlink(tname);   
   }
   else
   {
      fprintf(stderr, "Leaving disassembly in %s, built from %s\n",
              oname, cname);
   }
}


