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

// $Id: process.C,v 1.497 2004/04/21 14:25:49 chadd Exp $

#include <ctype.h>

#if defined(i386_unknown_solaris2_5)
#include <sys/procfs.h>
#endif
#include "common/h/headers.h"
#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/dyn_thread.h"
#include "dyninstAPI/src/dyn_lwp.h"
#include "dyninstAPI/src/signalhandler.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/util.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/instP.h"
#include "dyninstAPI/src/instPoint.h"
#include "dyninstAPI/src/dyninstP.h"
#include "dyninstAPI/src/os.h"
#include "dyninstAPI/src/showerror.h"
#include "dyninstAPI/src/dynamiclinking.h"
// #include "paradynd/src/mdld.h"
#include "common/h/Timer.h"
#include "common/h/Time.h"
#include "common/h/timing.h"

#include "dyninstAPI/src/rpcMgr.h"


#include "dyninstAPI/h/BPatch.h"

#if defined(sparc_sun_solaris2_4) || defined(i386_unknown_linux2_0)
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

#ifndef BPATCH_LIBRARY
extern void generateRPCpreamble(char *insn, Address &base, process *proc, 
                                unsigned offset, int tid, unsigned index);
#endif

#include "common/h/debugOstream.h"

#include "common/h/Timer.h"

unsigned enable_pd_attach_detach_debug = 0;

#if ENABLE_DEBUG_CERR == 1
#define attach_cerr if (enable_pd_attach_detach_debug) cerr
#else
#define attach_cerr if (0) cerr
#endif /* ENABLE_DEBUG_CERR == 1 */

unsigned enable_pd_inferior_rpc_debug = 0;

#if ENABLE_DEBUG_CERR == 1
#define inferiorrpc_cerr if (enable_pd_inferior_rpc_debug) cerr
#else
#define inferiorrpc_cerr if (0) cerr
#endif /* ENABLE_DEBUG_CERR == 1 */

unsigned enable_pd_shm_sampling_debug = 0;

#if ENABLE_DEBUG_CERR == 1
#define shmsample_cerr if (enable_pd_shm_sampling_debug) cerr
#else
#define shmsample_cerr if (0) cerr
#endif /* ENABLE_DEBUG_CERR == 1 */

unsigned enable_pd_fork_exec_debug = 0;

#if ENABLE_DEBUG_CERR == 1
#define forkexec_cerr if (enable_pd_fork_exec_debug) cerr
#else
#define forkexec_cerr if (0) cerr
#endif /* ENABLE_DEBUG_CERR == 1 */

unsigned enable_pd_signal_debug = 0;

#if ENABLE_DEBUG_CERR == 1
#define signal_cerr if (enable_pd_signal_debug) cerr
#else
#define signal_cerr if (0) cerr
#endif /* ENABLE_DEBUG_CERR == 1 */

unsigned enable_pd_sharedobj_debug = 0;

#if ENABLE_DEBUG_CERR == 1
#define sharedobj_cerr if (enable_pd_sharedobj_debug) cerr
#else
#define sharedobj_cerr if (0) cerr
#endif /* ENABLE_DEBUG_CERR == 1 */

#ifndef BPATCH_LIBRARY


unsigned enable_pd_metric_debug = 0;

#if ENABLE_DEBUG_CERR == 1
#define metric_cerr if (enable_pd_metric_debug) cerr
#else
#define metric_cerr if (0) cerr
#endif /* ENABLE_DEBUG_CERR == 1 */

unsigned enable_pd_samplevalue_debug = 0;

#if ENABLE_DEBUG_CERR == 1
#define sampleVal_cerr if (enable_pd_samplevalue_debug) cerr
#else
#define sampleVal_cerr if (0) cerr
#endif /* ENABLE_DEBUG_CERR == 1 */

unsigned enable_pd_aggregate_debug = 0;

#if ENABLE_DEBUG_CERR == 1
#define agg_cerr if (enable_pd_aggregate_debug) cerr
#else
#define agg_cerr if (0) cerr
#endif /* ENABLE_DEBUG_CERR == 1 */

#endif

unsigned MAX_NUMBER_OF_THREADS = 32;
#if defined(SHM_SAMPLING)
unsigned SHARED_SEGMENT_SIZE = 2097152;
#endif


#define FREE_WATERMARK (hp->totalFreeMemAvailable/2)
#define SIZE_WATERMARK 100
static const timeLength MaxWaitingTime(10, timeUnit::sec());
static const timeLength MaxDeletingTime(2, timeUnit::sec());
unsigned inferiorMemAvailable=0;

unsigned activeProcesses; // number of active processes
pdvector<process*> processVec;
pdstring process::programName;
pdstring process::pdFlavor;

#ifndef BPATCH_LIBRARY
extern pdstring osName;
#endif

#if defined (i386_unknown_linux2_0)
extern void cleanupVsysinfo(void *ehd);
#endif

// PARADYND_DEBUG_XXX
int pd_debug_infrpc=0;
int pd_debug_catchup=0;
//

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

ostream& operator<<(ostream&s, const Frame &f) {
   pd_Function *pcfunc = NULL;
   const char *funcname = "";
   if(f.thread_) {
      process *proc = f.thread_->get_proc();
      if(proc)
         pcfunc = proc->findFuncByAddr(f.pc_);
      if(pcfunc)
         funcname = pcfunc->prettyName().c_str();
   }

    fprintf(stderr, "PC: 0x%lx (%s), FP: 0x%lx, PID: %d",
            f.pc_, funcname, f.fp_, f.pid_);
    if (f.thread_)
        fprintf(stderr, ", TID: %d",
                f.thread_->get_tid());
    if (f.lwp_)
        fprintf(stderr, ", LWP: %d",
                f.lwp_->get_lwp_id());
    fprintf(stderr,"\n");
    
    /*
    s << ios::hex << "PC: " << f.pc_ << " FP: " << f.fp_
      << ios::dec << " PID: " << f.pid_;
  if (f.thread_)
    s << " TID: " << f.thread_->get_tid();
  if (f.lwp_)
    s << " LWP: " << f.lwp_->get_lwp_id();
    */  

  return s;
}

/* AIX method defined in aix.C; hijacked for IA-64's GP. */
#if !defined(rs6000_ibm_aix4_1) && !defined( ia64_unknown_linux2_4 )
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

  if (symbols->findFuncByOffset(dest))
    return (symbols->getObject()).getTOCoffset();
  if (shared_objects)
    for(u_int j=0; j < shared_objects->size(); j++)
      if (((*shared_objects)[j])->findFuncByAddress(dest))
#if ! defined(ia64_unknown_linux2_4)
        return (((*shared_objects)[j])->getImage()->getObject()).getTOCoffset();
#else
	{
	/* Entries in the .dynamic section are not relocated. */
	return (((*shared_objects)[j])->getImage()->getObject()).getTOCoffset() +
		((*shared_objects)[j])->getBaseAddress();
	}
#endif
  // Serious error! Assert?
  return 0;
}

#endif

#if defined(os_linux) && defined(arch_x86)
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
  
  if (!isStopped())
      return false;

#ifndef BPATCH_LIBRARY
  startTimingStackwalk();
#endif

#if defined(os_linux) 
  Address next_pc = currentFrame.getPC();

  // Do a special check for the vsyscall page.  Silently drop
  //  the page if it exists.
#if defined(arch_ia64)
  if (next_pc >= 0xffffffffffffe000 && next_pc < 0xfffffffffffff000) {
    fpOld = currentFrame.getSP();
    
    /* Suppress this frame; catch-up doesn't need it, and the user shouldn't see it. */
    currentFrame = currentFrame.getCallerFrame(this); 
  }
#elif defined(arch_x86)
  calcVSyscallFrame(this);
  if (next_pc >= getVsyscallStart() && next_pc < getVsyscallEnd()) {
     currentFrame = currentFrame.getCallerFrame(this);
  }
#endif

#endif
  // Do special check first time for leaf frames
  // Step through the stack frames
  while (!currentFrame.isLastFrame(this)) {
    
    // grab the frame pointer
    fpNew = currentFrame.getFP();

    // Check that we are not moving up the stack
    // successive frame pointers might be the same (e.g. leaf functions)
#if !defined(i386_unknown_linux2_0)
    if (fpOld > fpNew) {
      
      // AIX:
      // There's a signal function in the MPI library that we're not
      // handling properly. Instead of returning an empty stack,
      // return what we have.
      // One thing that makes me feel better: gdb is getting royally
      // confused as well. This sucks for catchup.
      
      #if defined( ia64_unknown_linux2_4 ) 
        /* My single-stepper needs to be able to continue past stackwalking errors. */
		/* DEBUG */ fprintf( stderr, "Terminating stackwalk early because fpOld (0x%lx) > fpNew (0x%lx).\n", fpOld, fpNew );
		break;
      #else
        // We should check to see if this early exit is warranted.
        return false;
      #endif
    }
#endif
    fpOld = fpNew;
    
    stackWalk.push_back(currentFrame);    
    currentFrame = currentFrame.getCallerFrame(this); 
  }
  // Clean up after while loop (push end frame)
  stackWalk.push_back(currentFrame);

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
      if (active == Frame())
	success = true;
      else
	activeFrames.push_back(active);
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


static Address alignAddress(Address addr, unsigned align) {
  Address skew = addr % align;
  return (skew) ? (((addr/align)+1)*align) : addr;
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

// Get inferior heaps from every library currently loaded.
// This is done at startup by initInferiorHeap().
// There's also another one which takes a shared library and
// only looks in there for the heaps

bool process::getInfHeapList(pdvector<heapDescriptor> &infHeaps)
{
  bool foundHeap = false;
  // First check the program (without shared libs)
  foundHeap = getInfHeapList(symbols, infHeaps, 0);
  
  // Now iterate through shared libs
  if (shared_objects)
      for(u_int j=0; j < shared_objects->size(); j++)
      {
          if(((*shared_objects)[j]->getImage())){
              if (getInfHeapList(((*shared_objects)[j])->getImage(), infHeaps, 0))
                  foundHeap = true;
	      }
	}

  return foundHeap;
}

bool process::getInfHeapList(const image *theImage, // okay, boring name
                             pdvector<heapDescriptor> &infHeaps,
                             Address baseAddr)
{

  // First we get the list of symbols we're interested in, then
  // we go through and add them to the heap list. This is done
  // for two reasons: first, the symbol address might be off (depends
  // on the image), and this lets us do some post-processing.
    bool foundHeap = false;
    pdvector<Symbol> heapSymbols;
    // For maximum flexibility the findInternalByPrefix function
    // returns a list of symbols
    
    foundHeap = theImage->findInternalByPrefix(pdstring("DYNINSTstaticHeap"),
                                               heapSymbols);
    if (!foundHeap)
        // Some platforms preface with an underscore
        foundHeap = theImage->findInternalByPrefix(pdstring("_DYNINSTstaticHeap"),
                                                   heapSymbols);
    // The address in the symbol isn't necessarily absolute
    if (!baseAddr) {
        // Possible that this call will fail if we're looking for
        // heaps before this image has been added to the shared objects
        // list - hence the handed-in parameter.
        getBaseAddress(theImage, baseAddr);
    }
    
    for (u_int j = 0; j < heapSymbols.size(); j++)
    {
        // The string layout is: DYNINSTstaticHeap_size_type_unique
        // Can't allocate a variable-size array on NT, so malloc
        // that sucker
        char *temp_str = (char *)malloc(strlen(heapSymbols[j].name().c_str())+1);
        strcpy(temp_str, heapSymbols[j].name().c_str());
        char *garbage_str = strtok(temp_str, "_"); // Don't care about beginning
        assert(!strcmp("DYNINSTstaticHeap", garbage_str));
        // Name is as is.
        // If address is zero, then skip (error condition)
        if (heapSymbols[j].addr() == 0)
        {
            cerr << "Skipping heap " << heapSymbols[j].name().c_str()
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
        infHeaps.push_back(heapDescriptor(heapSymbols[j].name(),
#ifdef mips_unknown_ce2_11 //ccw 13 apr 2001
                                          heapSymbols[j].addr(), 
#else
                                          heapSymbols[j].addr()+baseAddr, 
#endif
                                          heap_size, heap_type));
        free(temp_str);
    }
  return foundHeap;
}

/*
 * Returns true if the address given is within the signal handler function,
 * otherwise returns false.
 */
bool process::isInSignalHandler(Address addr)
{
#if defined(os_linux) && defined(arch_x86)
  for (unsigned int i = 0; i < signal_restore.size(); i++)
  {
    if (addr == signal_restore[i])
      return true;
  }
  return false;
#elif defined( os_linux ) && defined( arch_ia64 )
  assert( 0 );
#else
  if (!signal_handler)
      return false;

  const image *sig_image = (signal_handler->file())->exec();

  Address sig_addr;
  if(getBaseAddress(sig_image, sig_addr)){
    sig_addr += signal_handler->getAddress(this);
  } else {
    sig_addr = signal_handler->getAddress(this);
  }

  if (addr >= sig_addr && addr < sig_addr + signal_handler->get_size())
    return true;
  else
    return false;
#endif
}

/*
 * This function adds an item to the dataUpdates vector
 * which is used to maintain a list of variables that have
 * been written by the mutator //ccw 26 nov 2001
 */
#ifdef BPATCH_LIBRARY
void process::saveWorldData(Address address, int size, const void* src){
#else
void process::saveWorldData(Address, int, const void*){
#endif
#ifdef BPATCH_LIBRARY
#if defined(sparc_sun_solaris2_4) || defined(i386_unknown_linux2_0) || defined(rs6000_ibm_aix4_1)
	if(collectSaveWorldData){  //ccw 16 jul 2002 : NEW LINE
		dataUpdate *newData = new dataUpdate;
		newData->address= address;
		newData->size = size;
		newData->value = new char[size];
		memcpy(newData->value, src, size);
		dataUpdates.push_back(newData);
	}
#else /* Get rid of annoying warnings */
	Address tempaddr = address;
	int tempsize = size;
	const void *bob = src;
#endif
#endif
}

#if defined(sparc_sun_solaris2_4) || defined(i386_unknown_linux2_0)  || defined(rs6000_ibm_aix4_1)
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

#if defined(sparc_sun_solaris2_4) || defined(i386_unknown_linux2_0)  
unsigned int process::saveWorldSaveSharedLibs(int &mutatedSharedObjectsSize, 
                                 unsigned int &dyninst_SharedLibrariesSize, 
                                 char* directoryName, unsigned int &count) {
   shared_object *sh_obj;
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
   for(int i=0;shared_objects && i<(int)shared_objects->size() ; i++) {
      sh_obj = (*shared_objects)[i];
      //ccw 24 jul 2003
      if( (sh_obj->isDirty() || sh_obj->isDirtyCalled()) &&
		/* there are some libraries we should not save even if they are marked as mutated*/
		NULL==strstr(sh_obj->getName().c_str(),"libdyninstAPI_RT") && 
		NULL== strstr(sh_obj->getName().c_str(),"ld-linux.so") && 
		NULL==strstr(sh_obj->getName().c_str(),"libc")){ //ccw 6 jul 2003
         count ++;
         if(!dlopenUsed && sh_obj->isopenedWithdlopen()){
            BPatch_reportError(BPatchWarning,123,"dumpPatchedImage: dlopen used by the mutatee, this may cause the mutated binary to fail\n");
            dlopenUsed = true;
         }			
         //bperr(" %s is DIRTY!\n", sh_obj->getName().c_str());
        

         if( sh_obj->isDirty()){ 
            //if the lib is only DirtyCalled dont save it! //ccw 24 jul 2003
            Address textAddr, textSize;
            char *newName = new char[strlen(sh_obj->getName().c_str()) + 
                                     strlen(directoryName) + 1];
            memcpy(newName, directoryName, strlen(directoryName)+1);
            const char *file = strrchr(sh_obj->getName().c_str(), '/');
            strcat(newName, file);
            
            saveSharedLibrary *sharedObj =
               new saveSharedLibrary(sh_obj->getBaseAddress(),
                                     sh_obj->getName().c_str(), newName);
            sharedObj->writeLibrary();
            
            sharedObj->getTextInfo(textAddr, textSize);
            
            char *textSection = new char[textSize];
            readDataSpace((void*) (textAddr+ sh_obj->getBaseAddress()),
                          textSize,(void*)textSection, true);
            
            sharedObj->saveMutations(textSection);
            sharedObj->closeLibrary();
            /*			
            //this is for the dlopen problem....
            if(strstr(sh_obj->getName().c_str(), "ld-linux.so") ){
            //find the offset of _dl_debug_state in the .plt
            dl_debug_statePltEntry = 
            sh_obj->getImage()->getObject().getPltSlot("_dl_debug_state");
            }
            */			
            delete [] textSection;
            delete [] newName;
         }
         mutatedSharedObjectsSize += strlen(sh_obj->getName().c_str()) +1 ;
         mutatedSharedObjectsSize += sizeof(int); //a flag to say if this is only DirtyCalled
      }
      //this is for the dlopen problem....
      if(strstr(sh_obj->getName().c_str(), "ld-linux.so") ){
         //find the offset of _dl_debug_state in the .plt
         dl_debug_statePltEntry = 
            sh_obj->getImage()->getObject().getPltSlot("_dl_debug_state");
      }
#if defined(sparc_sun_solaris2_4)
      
      if( ((tmp_dlopen = sh_obj->getImage()->getObject().getPltSlot("dlopen")) && !sh_obj->isopenedWithdlopen())){
         dl_debug_statePltEntry = tmp_dlopen + sh_obj->getBaseAddress();
      }
#endif
      //this is for the dyninst_SharedLibraries section we need to find out
      //the length of the names of each of the shared libraries to create the
      //data buffer for the section
      
      dyninst_SharedLibrariesSize += strlen(sh_obj->getName().c_str())+1;
      //add the size of the address
      dyninst_SharedLibrariesSize += sizeof(unsigned int);
   }
#if defined(sparc_sun_solaris2_4)
   if( (tmp_dlopen = getImage()->getObject().getPltSlot("dlopen"))) {
      dl_debug_statePltEntry = tmp_dlopen;
   }
   
   //dl_debug_statePltEntry = getImage()->getObject().getPltSlot("dlopen");
#endif
   dyninst_SharedLibrariesSize += 1;//for the trailing '\0'
   
   return dl_debug_statePltEntry;
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
	int size = shared_objects->size();
	shared_object *sh_obj;

	for(int i=0;shared_objects && i<size ; i++) {
		sh_obj = (*shared_objects)[i];

		memcpy((void*) ptr, sh_obj->getName().c_str(), strlen(sh_obj->getName().c_str())+1);
		//bperr(" %s : ", ptr);
		ptr += strlen(sh_obj->getName().c_str())+1;

		unsigned int baseAddr = sh_obj->getBaseAddress();
		memcpy( (void*)ptr, &baseAddr, sizeof(unsigned int));
		//bperr(" 0x%x \n", *(unsigned int*) ptr);
		ptr += sizeof(unsigned int);
	}
       	memset( (void*)ptr, '\0' , 1);

	return dyninst_SharedLibrariesData;
}
#endif

#if defined(sparc_sun_solaris2_4) || defined(i386_unknown_linux2_0)  || defined(rs6000_ibm_aix4_1)
void process::saveWorldCreateHighMemSections(
                        pdvector<imageUpdate*> &compactedHighmemUpdates, 
                        pdvector<imageUpdate*> &highmem_updates,
                        void *ptr) {

   unsigned int trampGuardValue;
   Address guardFlagAddr= trampGuardAddr();

   unsigned int pageSize = getpagesize();
   unsigned int startPage, stopPage;
   unsigned int numberUpdates=1;
   int startIndex, stopIndex;
   void *data;
   char name[50];
#if defined(sparc_sun_solaris2_4) || defined(i386_unknown_linux2_0)
	writeBackElf *newFile = (writeBackElf*) ptr;
#elif defined(rs6000_ibm_aix4_1)
	writeBackXCOFF *newFile = (writeBackXCOFF*) ptr;
#endif

   readDataSpace((void*) guardFlagAddr, sizeof(unsigned int),
                 (void*) &trampGuardValue, true);
   
   writeDataSpace((void*)guardFlagAddr, sizeof(unsigned int),
                  (void*) &numberUpdates);

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
#if defined(sparc_sun_solaris2_4) || defined(i386_unknown_linux2_0)
      newFile->addSection(compactedHighmemUpdates[j]->address,data ,dataSize,name,false);
#elif defined(rs6000_ibm_aix4_1)
	  sprintf(name, "dyH_%03x",j);
      newFile->addSection(&(name[0]), compactedHighmemUpdates[j]->address,compactedHighmemUpdates[j]->address,
		dataSize, (char*) data );
#endif
      
      //lastCompactedUpdateAddress = compactedHighmemUpdates[j]->address+1;
      delete [] (char*) data;
   }
   writeDataSpace((void*)guardFlagAddr, sizeof(unsigned int), 
                  (void*)&trampGuardValue);
}

void process::saveWorldCreateDataSections(void* ptr){

#if defined(sparc_sun_solaris2_4) || defined(i386_unknown_linux2_0)
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
			//bperr(" DATA UPDATE : from: %x to %x , value %x\n", dataUpdates[k]->address,
		//	dataUpdates[k]->address+ dataUpdates[k]->size, (unsigned int) dataUpdates[k]->value);

		}
		*(int*) ptr=0;
		ptr += sizeof(int);
		*(unsigned int*) ptr=0;
#if defined(sparc_sun_solaris2_4) || defined(i386_unknown_linux2_0)
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

#if defined(sparc_sun_solaris2_4) || defined(i386_unknown_linux2_0) || defined(rs6000_ibm_aix4_1)

void process::saveWorldAddSharedLibs(void *ptr){ // ccw 14 may 2002 

	int dataSize=0;
	char *data, *dataptr;
#if defined(sparc_sun_solaris2_4) || defined(i386_unknown_linux2_0)
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
#if defined(sparc_sun_solaris2_4) || defined(i386_unknown_linux2_0)
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

void process::addInferiorHeap(const image *theImage, Address baseAddr)
{
  pdvector<heapDescriptor> infHeaps;
  /* Get a list of inferior heaps in the new image */
  if (getInfHeapList(theImage, infHeaps, baseAddr))
    {
      /* Add the vector to the inferior heap structure */
        for (u_int j=0; j < infHeaps.size(); j++)
        {
            heapItem *h = new heapItem (infHeaps[j].addr(), infHeaps[j].size(),
                                        infHeaps[j].type(), false);
            heap.bufferPool.push_back(h);
            heapItem *h2 = new heapItem(h);
            heap.heapFree.push_back(h2);
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
  assert(this->symbols);
  inferiorHeap *hp = &heap;
  pdvector<heapDescriptor> infHeaps;

  // first initialization: add static heaps to pool
  if (hp->bufferPool.size() == 0) {
    bool err;
    Address heapAddr=0;
    int staticHeapSize = alignAddress(SYN_INST_BUF_SIZE, 32);

    // Get the inferior heap list
    getInfHeapList(infHeaps);
    
    bool lowmemHeapAdded = false;
    bool heapAdded = false;
    
    for (u_int j=0; j < infHeaps.size(); j++)
    {
        hp->bufferPool.push_back(new heapItem (infHeaps[j].addr(), infHeaps[j].size(),
                                               infHeaps[j].type(), false));
        heapAdded = true;
        if (infHeaps[j].type() == lowmemHeap)
            lowmemHeapAdded = true;
    }
    if (!heapAdded)
    {
        // No heap added. Check for the old DYNINSTdata heap
        unsigned LOWMEM_HEAP_SIZE=(32*1024);
        cerr << "No heap found of the form DYNINSTstaticHeap_<size>_<type>_<unique>." << endl;
        cerr << "Attempting to use old DYNINSTdata inferior heap..." << endl;
        heapAddr = findInternalAddress(pdstring("DYNINSTdata"), true, err);
        assert(heapAddr);
        hp->bufferPool.push_back(new heapItem(heapAddr, staticHeapSize - LOWMEM_HEAP_SIZE,
                                              anyHeap, false));
        hp->bufferPool.push_back(new heapItem(heapAddr + staticHeapSize - LOWMEM_HEAP_SIZE,
                                              LOWMEM_HEAP_SIZE, lowmemHeap, false));
        heapAdded = true; 
        lowmemHeapAdded = true;
    }
    if (!lowmemHeapAdded)
    {
        // Didn't find the low memory heap. 
        // Handle better?
        // Yeah, gripe like hell
        cerr << "No lowmem heap found (DYNINSTstaticHeap_*_lowmem), inferior RPCs" << endl;
        cerr << "will probably fail" << endl;
    }
    
  }
  
  // (re)initialize everything 
  hp->heapActive.clear();
  hp->heapFree.resize(0);
  hp->disabledList.resize(0);
  hp->disabledListTotalMem = 0;
  hp->freed = 0;
  hp->totalFreeMemAvailable = 0;
  
  /* add dynamic heap segments to free list */
  for (unsigned i = 0; i < hp->bufferPool.size(); i++) {
    heapItem *hi = new heapItem(hp->bufferPool[i]);
    hi->status = HEAPfree;
    hp->heapFree.push_back(hi);
    hp->totalFreeMemAvailable += hi->length;     
  }
  inferiorMemAvailable = hp->totalFreeMemAvailable;
}

bool process::initTrampGuard()
{
  // This is slightly funky. Dyninst does not currently support
  // multiple threads -- so it uses a single tramp guard flag, 
  // which resides in the runtime library. However, this is not
  // enough for MT paradyn. So Paradyn overrides this setting as 
  // part of its initialization.
  // Repeat: OVERRIDDEN LATER FOR PARADYN

  const pdstring vrbleName = "DYNINSTtrampGuard";
  internalSym sym;
  bool flag = findInternalSymbol(vrbleName, true, sym);
  assert(flag);
  trampGuardAddr_ = sym.getAddr();
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
    inferiorMemAvailable = totalFreeMemAvailable;
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

void process::inferiorMallocCallback(process * /*proc*/, unsigned /* rpc_id */,
                                     void *data, void *result)
{
  imd_rpc_ret *ret = (imd_rpc_ret *)data;
  ret->result = result;
  ret->ready = true;
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
#if !defined(mips_sgi_irix6_4)
  // Fun (not) case: there's no space for the RPC to execute.
  // It'll call inferiorMalloc, which will call inferiorMallocDynamic...
  // Avoid this with a static bool.
  if (inInferiorMallocDynamic) {
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
  args[0] = new AstNode(AstNode::Constant, (void *)size);
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

  do {
      getRpcMgr()->launchRPCs(wasRunning);
      if(hasExited()) return;
      getSH()->checkForAndHandleProcessEvents(false);
   } while (!ret.ready); // Loop until callback has fired.

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
   inferiorMemAvailable = hp->totalFreeMemAvailable;
   assert(h->addr);
   
   // ccw: 28 oct 2001
   // create imageUpdate here:
   // imageUpdate(h->addr,size)
   
#ifdef BPATCH_LIBRARY
//	if( h->addr > 0xd0000000){
//		bperr(" \n ALLOCATION: %lx %lx ntry: %d\n", h->addr, size,ntry);
//		fflush(stdout);
//	}
#if defined(sparc_sun_solaris2_4 ) || defined(i386_unknown_linux2_0) || defined(rs6000_ibm_aix4_1)
   if(collectSaveWorldData){
      
#if defined(sparc_sun_solaris2_4)
      if(h->addr < 0xF0000000)
#elif defined(i386_unknown_linux2_0)
      if(h->addr < 0x40000000)
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
      } else {
	 //	totalSizeAlloc += size;
	 //bperr(" HIGHMEM UPDATE %x %x \n", h->addr, size);
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
    showErrorCallback(96,"Internal error: "
        "attempt to free non-defined heap entry.");
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
  inferiorMemAvailable = hp->totalFreeMemAvailable;
}
 

// Cleanup the process object. Delete all sub-objects to ensure we aren't 
// leaking memory and prepare for new versions. Useful when the process exits
// (and the process object is deleted) and when it execs

void process::deleteProcess() {
    // A lot of the behavior here is keyed off the current process status....
    // if it is exited we'll delete things without performing any operations
    // on the process. Otherwise we'll attempt to reverse behavior we've made.
    
    // If this assert fires check whether we're setting the status vrble correctly
    // before calling this function
    assert(!isAttached());
           
    // Get rid of our syscall tracing.
    if (tracedSyscalls_) {
        delete tracedSyscalls_;
        tracedSyscalls_ = NULL;
    }
    // Delete the base (and minitramp) handles if the process execed or exited
    // We keep them around if we detached.
    if (status_ != detached) {        
        dictionary_hash_iter<const instPoint *, trampTemplate *>baseMap_iter(baseMap);
        for (; baseMap_iter; baseMap_iter++) {
            // WE ARE NOT DELETING INSTPOINTS as they are shared between processes.
            // const instPoint *p = baseMap_iter.currkey();
            // delete p;
            // Note: we do not remove the base and mini tramps from the codeRange address
            // mapping, since we'll clear it later.
            trampTemplate *t = baseMap_iter.currval();
            delete t;
        }
        // instpoints to base tramps
        baseMap.clear();
        // Address to instpoint (for arb. instpoints)
        instPointMap.clear();
        // pdFunctions to bpatch 'parents'
        PDFuncToBPFuncMap.clear();
        trampTrapMapping.clear();
    }
    
#if defined(SHM_SAMPLING)
    delete shMetaOffsetData;
    shMetaOffsetData = NULL;
    delete shmMetaData;
    shmMetaData = NULL;
#endif

    // Our modifications to the dynamic linker so that we're made aware
    // of new shared libraries
    if (dyn) {
        delete dyn;
        dyn = NULL;
    }
    
    // Delete all shared_object objects
    if (shared_objects) {
        for (unsigned shared_objects_iter = 0;
             shared_objects_iter < (*shared_objects).size();
             shared_objects_iter++)
            delete (*shared_objects)[shared_objects_iter];
        // Note: we don't remove the shared objects from the codeRange address
        // mapping, as we clear it later.
        delete shared_objects;
        shared_objects = NULL;
    }
    runtime_lib = NULL;
    
    // Clear the codeRange address mapping
    if (codeRangesByAddr_) {
        delete codeRangesByAddr_;
        codeRangesByAddr_ = NULL;
    }

    // Signal handler
#if !(defined(os_linux) && defined(arch_x86))
   signal_handler = 0;
#endif

   
   // pdFunctions should be deleted as part of the image class... when the reference count
   // hits 0... this is TODO
   delete some_modules; some_modules = NULL;
   delete some_functions; some_functions = NULL;
   delete all_functions; all_functions = NULL;
   delete all_modules; all_modules = NULL;
   
   if (status_ == exited || status_ == detached) {
// Delete the thread and lwp structures
       dictionary_hash_iter<unsigned, dyn_lwp *> lwp_iter(real_lwps);
       dyn_lwp *lwp;
       unsigned index;
       
       while (lwp_iter.next(index, lwp)) {
           deleteLWP(lwp);
       }
       real_lwps.clear();
       
       for (unsigned thr_iter = 0; thr_iter < threads.size(); thr_iter++) {
           delete threads[thr_iter];
       }
       threads.clear();
       // Delete the RPC manager
       if (theRpcMgr) delete theRpcMgr;
       theRpcMgr = NULL;
   }

   // TODO: thread and lwp cleanup when we exec.

}

//
// cleanup when a process is deleted. Assumed we are detached or the process is exited.
//
process::~process()
{
    // We require explicit detaching if the process still exists.

    assert(!isAttached());
    
    // Most of the deletion is encapsulated in deleteProcess
    deleteProcess();

    // We used to delete the particular process, but this created no end of problems
    // with people storing pointers into particular indices. We set the pointer to NULL.
    for (unsigned lcv=0; lcv < processVec.size(); lcv++) {
        if (processVec[lcv] == this) {
            processVec[lcv] = NULL;
        }        
    }

#if defined(i386_unknown_linux2_0)
    cleanupVsysinfo(getVsyscallData());
#endif

#if defined( ia64_unknown_linux2_4 )
     _UPT_destroy( unwindProcessArg );
     unw_destroy_addr_space( unwindAddressSpace );
#endif
}

unsigned hash_bp(function_base * const &bp ) { return(addrHash4((Address) bp)); }

//
// Process "normal" (non-attach, non-fork) ctor, for when a new process
// is fired up by paradynd itself.
// This ctor. is also used in the case that the application has been fired up
// by another process and stopped just after executing the execv() system call.
// In the "normal" case, the parameter iTraceLink will be a non-negative value which
// corresponds to the pipe descriptor of the trace pipe used between the paradynd
// and the application. In the later case, iTraceLink will be -1. This will be 
// posteriorly used to properly initialize the createdViaAttachToCreated flag. - Ana
//

//removed all ioLink related code for output redirection
process::process(int iPid, image *iImage, int iTraceLink 
#ifdef SHM_SAMPLING
                 , key_t theShmKey
#endif
) :
  collectSaveWorldData(true),
#if defined(BPATCH_LIBRARY) && defined(rs6000_ibm_aix4_1)
 requestTextMiniTramp(0), //ccw 30 jul 2002
#endif
  bpatch_thread( NULL ),
  baseMap(ipHash),
  PDFuncToBPFuncMap(hash_bp),
  instPointMap(hash_address),
  trampTrapMapping(addrHash4),
  suppressCont_(false),
  loadLibraryCallbacks_(pdstring::hash),
  cached_result(not_cached),
#ifndef BPATCH_LIBRARY
  shmMetaData(NULL), shMetaOffsetData(NULL),
#endif
  savedRegs(NULL),
  pid(iPid), // needed in fastInferiorHeap ctors below
#if !defined(BPATCH_LIBRARY)
  previous(0),
#endif
#if defined(i386_unknown_nt4_0) || (defined mips_unknown_ce2_11)
  windows_termination_requested(false),
#endif
  representativeLWP(NULL),
#if ! defined( ia64_unknown_linux2_4 )  
  real_lwps(CThash)  
#else
  real_lwps(CThash), unwindAddressSpace( NULL ), unwindProcessArg( NULL )
#endif
{
#if !defined(BPATCH_LIBRARY) //ccw 22 apr 2002 : SPLIT
	PARADYNhasBootstrapped = false;
#endif
    tracedSyscalls_ = NULL;
    codeRangesByAddr_ = NULL;

   shared_objects = 0;
   invalid_thr_create_msgs = 0;

   parentPid = childPid = 0;
   dyninstlib_brk_addr = 0;
    
   main_brk_addr = 0;

   nextTrapIsFork = false;
   nextTrapIsExec = false;
   // Instrumentation points for tracking syscalls
   preForkInst = postForkInst = preExecInst = postExecInst = preExitInst = NULL;

   LWPstoppedFromForkExit = 0;  

   wasRunningWhenAttached_ = false;
   bootstrapState = unstarted;

   createdViaAttach = false;
   createdViaFork = false;
   createdViaAttachToCreated = false; 
   wasRunningWhenAttached_ = false; // Technically true...

#ifndef BPATCH_LIBRARY
   if (iTraceLink == -1 ) createdViaAttachToCreated = true;
   // indicates the unique case where paradynd is attached to
   // a stopped application just after executing the execv() --Ana
#endif
   needToContinueAfterDYNINSTinit = false;  //Wait for press of "RUN" button

   symbols = iImage;
   // The a.out is a special case... all addresses in it are absolutes,
   // but we only want it to occupy the area in our memory map that the
   // functions take up. So we insert it at the codeOffset, and then
   // _don't_ pass in offsets for recursed function lookups
   codeRangesByAddr_ = new codeRangeTree;
   addCodeRange(symbols->codeOffset(), symbols);
    
   mainFunction = NULL; // set in platform dependent function heapIsOk

   theRpcMgr = new rpcMgr(this);
   set_status(neonatal);
   previousSignalAddr_ = 0;
   continueAfterNextStop_ = 0;

#ifndef BPATCH_LIBRARY
   theSharedMemMgr = new shmMgr(this, theShmKey, SHARED_SEGMENT_SIZE);
   shmMetaData = 
      new sharedMetaData(*theSharedMemMgr, MAX_NUMBER_OF_THREADS); 
   // previously was using maxNumberOfThreads() instead of
   // MAX_NUMBER_OF_THREADS.  Unfortunately, maxNumberOfThreads
   // calls multithread_capable(), which isn't able to be
   // called yet since the modules aren't parsed.
   // We'll use the larger size for the ST case.  The increased
   // size is fairly small, perhaps 2 KB.
#endif
   parent = NULL;
   inExec = false;

   cumObsCost = 0;
   lastObsCostLow = 0;

    
   dynamiclinking = false;
   dyn = new dynamic_linking(this);
   runtime_lib = 0;
   all_functions = 0;
   all_modules = 0;
   some_modules = 0;
   some_functions = 0;
#if !(defined(os_linux) && defined(arch_x86))
   signal_handler = 0;
#endif
   execed_ = false;

   inInferiorMallocDynamic = false;

#ifdef SHM_SAMPLING
#ifdef sparc_sun_sunos4_1_3
   kvmHandle = kvm_open(0, 0, 0, O_RDONLY, 0);
   if (kvmHandle == NULL) {
      perror("could not map child's uarea; kvm_open");
      exit(5);
   }

   //   childUareaPtr = tryToMapChildUarea(iPid);
   childUareaPtr = NULL;
#endif
#endif

#if defined(i386_unknown_linux2_0)
    vsyscall_start_ = 0x0;
    vsyscall_end_ = 0x0;
    vsyscall_text_ = 0x0;
    vsyscall_data_= NULL;
#endif

   splitHeaps = false;

#if defined(rs6000_ibm_aix3_2) || defined(rs6000_ibm_aix4_1) || defined(alpha_dec_osf4_0)
   // XXXX - move this to a machine dependant place.

   // create a seperate text heap.
   //initInferiorHeap(true);
   splitHeaps = true;
#endif
   traceLink = iTraceLink; // notice that tracelink will be -1 in the unique
   // case called "AttachToCreated" - Ana
          
   bufStart = 0;
   bufEnd = 0;
   //removed for output redirection
   //ioLink = iIoLink;


   createRepresentativeLWP();

   // attach to the child process (machine-specific implementation)
   if (!attach()) { // error check?
       status_ = detached;
       
      pdstring msg = pdstring("Warning: unable to attach to specified process :")
         + pdstring(pid);
      showErrorCallback(26, msg.c_str());
      cerr << "Process: failed attach!" << endl;
   }

#ifndef BPATCH_LIBRARY
#ifdef PAPI
   if (isPapiInitialized()) {
      papi = new papiMgr(this);
   }
#endif
#endif
} // end of normal constructor


//
// Process "attach" ctor, for when paradynd is attaching to an already-existing
// process. 
//
//
process::process(int iPid, image *iSymbols,
                 bool &success
#if !defined(BPATCH_LIBRARY)
                 , key_t theShmKey
#endif
                 ) :
  collectSaveWorldData(true),
#if defined(BPATCH_LIBRARY) && defined(rs6000_ibm_aix4_1)
  requestTextMiniTramp(0), //ccw 30 jul 2002
#endif
  bpatch_thread( NULL ),
  baseMap(ipHash),
  PDFuncToBPFuncMap(hash_bp),
  instPointMap(hash_address),
  trampTrapMapping(addrHash4),
  suppressCont_(false),
  loadLibraryCallbacks_(pdstring::hash),
  cached_result(not_cached),
#ifndef BPATCH_LIBRARY
  shmMetaData(NULL), shMetaOffsetData(NULL),
  PARADYNhasBootstrapped(false),
#endif
  savedRegs(NULL),
  pid(iPid),
#if !defined(BPATCH_LIBRARY)  && defined(i386_unknown_nt4_0)
  previous(0), //ccw 8 jun 2002
#endif
#if defined(i386_unknown_nt4_0) || (defined mips_unknown_ce2_11)
  windows_termination_requested(false),
#endif
  representativeLWP(NULL),
#if ! defined( ia64_unknown_linux2_4 )  
  real_lwps(CThash)  
#else
  real_lwps(CThash), unwindAddressSpace( NULL ), unwindProcessArg( NULL )
#endif
{
    tracedSyscalls_ = NULL;
    codeRangesByAddr_ = NULL;

    parentPid = childPid = 0;
    dyninstlib_brk_addr = 0;
    main_brk_addr = 0;
    
    nextTrapIsFork = false;
    nextTrapIsExec = false;

    invalid_thr_create_msgs = 0;

    createdViaAttach = true;

#if !defined(i386_unknown_nt4_0)
    bootstrapState = initialized;
#else
    // We need to wait for the CREATE_PROCESS debug event.
    // Set to "begun" here, and fix up in the signal loop
    bootstrapState = begun;
#endif

    createdViaFork = false;
    createdViaAttachToCreated = false; 
    
    symbols = iSymbols;
    mainFunction = NULL; // set in platform dependent function heapIsOk
    
    set_status(neonatal);
    previousSignalAddr_ = 0;
    continueAfterNextStop_ = 0;

    LWPstoppedFromForkExit = 0;

    inInferiorMallocDynamic = false;
    theRpcMgr = new rpcMgr(this);
#ifndef BPATCH_LIBRARY
    theSharedMemMgr = new shmMgr(this, theShmKey, SHARED_SEGMENT_SIZE);
    shmMetaData = new sharedMetaData(*theSharedMemMgr, MAX_NUMBER_OF_THREADS);
               // previously was using maxNumberOfThreads() instead of
               // MAX_NUMBER_OF_THREADS.  Unfortunately, maxNumberOfThreads
               // calls multithread_capable(), which isn't able to be
               // called yet since the modules aren't parsed.
               // We'll use the larger size for the ST case.  The increased
               // size is fairly small, perhaps 2 KB.
#endif

    parent = NULL;
    inExec = false;

    // Instrumentation points for tracking syscalls
    preForkInst = postForkInst = preExecInst = postExecInst = preExitInst = NULL;
    cumObsCost = 0;
    lastObsCostLow = 0;

    dynamiclinking = false;
    dyn = new dynamic_linking(this);
    shared_objects = 0;
    runtime_lib = 0;
    all_functions = 0;
    all_modules = 0;
    some_modules = 0;
    some_functions = 0;
#if !(defined(os_linux) && defined(arch_x86))
    signal_handler = 0;
#endif
    execed_ = false;

    splitHeaps = false;
#if defined(rs6000_ibm_aix3_2) || defined(rs6000_ibm_aix4_1) || defined(alpha_dec_osf4_0)
        // XXXX - move this to a machine dependant place.

        // create a seperate text heap.
        //initInferiorHeap(true);
        splitHeaps = true;
#endif

#if defined(i386_unknown_linux2_0)
    vsyscall_start_ = 0x0;
    vsyscall_end_ = 0x0;
    vsyscall_text_ = 0x0;
    vsyscall_data_= NULL;
#endif

   traceLink = -1; // will be set later, when the appl runs DYNINSTinit
   bufStart = 0;
   bufEnd = 0;

   //removed for output redirection
   //ioLink = -1; // (ARGUABLY) NOT YET IMPLEMENTED...MAYBE WHEN WE ATTACH WE DON'T WANT
                // TO REDIRECT STDIO SO WE CAN LEAVE IT AT -1.

   // Now the actual attach...the moment we've all been waiting for

   attach_cerr << "process attach ctor: about to attach to pid " << getPid() << endl;
   createRepresentativeLWP();

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
      success = false;
      return;
   }

#if defined(rs6000_ibm_aix3_2) || defined(rs6000_ibm_aix4_1)
   // Now that we're attached, we can reparse the image with correct
   // settings.
   // Process is paused 
   int status = pid;
   fileDescriptor *desc = getExecFileDescriptor(symbols->pathname(), status, false);
   if (!desc) {
      pdstring msg = pdstring("Warning: unable to parse to specified process: ")
                   + pdstring(pid);
      showErrorCallback(26, msg.c_str());
      success = false;
      return;
   }

   image *theImage = image::parseImage(desc);
   if (theImage == NULL) {
      pdstring msg = pdstring("Warning: unable to parse to specified process: ")
                   + pdstring(pid);
      showErrorCallback(26, msg.c_str());
      success = false;
      return;
   }
   // this doesn't leak, since the old theImage was deleted. 
   symbols = theImage;
#endif
   codeRangesByAddr_ = new codeRangeTree;
   addCodeRange(symbols->codeOffset(), symbols);

   // Record what the process was doing when we attached, for possible
   // use later.
   wasRunningWhenAttached_ = isRunning_();
   if (wasRunningWhenAttached_) 
      set_status(running);
   else
      set_status(stopped);

   // Does attach() send a SIGTRAP, a la the initial SIGTRAP sent at the
   // end of exec?  It seems that on some platforms it does; on others
   // it doesn't.  Ick.  On solaris, it doesn't.

   // note: we don't call getSharedObjects() yet; that happens once DYNINSTinit
   //       finishes (initSharedObjects)


#ifndef BPATCH_LIBRARY
#ifdef PAPI
   if (isPapiInitialized()) {
     papi = new papiMgr(this);
   }
#endif
#endif

   // Everything worked
   success = true;
}

//
// Process "fork" ctor, for when a process which is already being monitored by
// paradynd executes the fork syscall.
//

process::process(const process &parentProc, int iPid, int iTrace_fd) :
  collectSaveWorldData(true),
#if defined(BPATCH_LIBRARY) && defined(rs6000_ibm_aix4_1)
  requestTextMiniTramp(0), //ccw 30 jul 2002
#endif
  baseMap(ipHash), // could change to baseMap(parentProc.baseMap)
  PDFuncToBPFuncMap(hash_bp),
  instPointMap(hash_address),
  trampTrapMapping(parentProc.trampTrapMapping),
  suppressCont_(false),
  loadLibraryCallbacks_(pdstring::hash),
  cached_result(parentProc.cached_result),
#ifndef BPATCH_LIBRARY
  shmMetaData(NULL), shMetaOffsetData(NULL),
  PARADYNhasBootstrapped(false),
#endif
  savedRegs(NULL),
#ifdef SHM_SAMPLING
  previous(0),
#endif
#if defined(i386_unknown_nt4_0) || (defined mips_unknown_ce2_11)
  windows_termination_requested(false),
#endif
  representativeLWP(NULL),
#if ! defined( ia64_unknown_linux2_4 )  
  real_lwps(CThash)  
#else
  real_lwps(CThash), unwindAddressSpace( NULL ), unwindProcessArg( NULL )
#endif
{
   // This is the "fork" ctor
   bootstrapState = initialized;

   invalid_thr_create_msgs = 0;

   createdViaAttachToCreated = false;
   createdViaFork = true;
   createdViaAttach = parentProc.createdViaAttach;
   wasRunningWhenAttached_ = true;
   needToContinueAfterDYNINSTinit = true;

   symbols = parentProc.symbols->clone();
   symbols->updateForFork(this, &parentProc);
   mainFunction = parentProc.mainFunction;

   LWPstoppedFromForkExit = 0;

   traceLink = iTrace_fd;
   bufStart = 0;
   bufEnd = 0;

   //removed for output redireciton
   //ioLink = -1; // when does this get set?

   set_status(neonatal); // is neonatal right?
   previousSignalAddr_ = 0;
   continueAfterNextStop_ = 0;

   pid = iPid; 

   // Copy over the base tramp data structure
   {
       dictionary_hash_iter<const instPoint *, trampTemplate *> baseMap_iter(parentProc.baseMap);
       trampTemplate *t;
       for (; baseMap_iter; baseMap_iter++) {
           const instPoint *p = baseMap_iter.currkey();
           t = baseMap_iter.currval();
           // Also copies internal minitramp lists
           baseMap[p] = new trampTemplate(t, this);
       }
   }

   codeRangesByAddr_ = new codeRangeTree(*(parentProc.codeRangesByAddr_));


   // Copy over the system call notifications and reinitialize (if necessary)
   tracedSyscalls_ = new syscallNotification(parentProc.tracedSyscalls_, this);

   theRpcMgr = new rpcMgr(this);

   parent = const_cast<process*>(&parentProc);
    
   parentPid = childPid = 0;
   dyninstlib_brk_addr = 0;

   main_brk_addr = 0;
   nextTrapIsExec = false;
   
   //bootstrapState = initialized;
   bootstrapState = bootstrapped;

   splitHeaps = parentProc.splitHeaps;

   heap = inferiorHeap(parentProc.heap);

   inExec = false;

   cumObsCost = 0;
   lastObsCostLow = 0;

#if defined(i386_unknown_linux2_0)
    vsyscall_start_ = 0x0;
    vsyscall_end_ = 0x0;
    vsyscall_text_ = 0x0;
    vsyscall_data_= NULL;
#endif

   inInferiorMallocDynamic = false;

   dynamiclinking = parentProc.dynamiclinking;
   dyn = new dynamic_linking(this, parentProc.dyn);
   runtime_lib = parentProc.runtime_lib;

   shared_objects = 0;

   // make copy of parent's shared_objects vector
   if (parentProc.shared_objects) {
      shared_objects = new pdvector<shared_object*>;
      for (unsigned u1 = 0; u1 < parentProc.shared_objects->size(); u1++){
         (*shared_objects).push_back(
                                     new shared_object(*(*parentProc.shared_objects)[u1]));
      }
   }

   all_functions = 0;
   if (parentProc.all_functions) {
      all_functions = new pdvector<function_base *>;
      for (unsigned u2 = 0; u2 < parentProc.all_functions->size(); u2++)
         (*all_functions).push_back((*parentProc.all_functions)[u2]);
   }

   all_modules = 0;
   if (parentProc.all_modules) {
      all_modules = new pdvector<module *>;
      for (unsigned u3 = 0; u3 < parentProc.all_modules->size(); u3++)
         (*all_modules).push_back((*parentProc.all_modules)[u3]);
   }

   some_modules = 0;
   if (parentProc.some_modules) {
      some_modules = new pdvector<module *>;
      for (unsigned u4 = 0; u4 < parentProc.some_modules->size(); u4++)
         (*some_modules).push_back((*parentProc.some_modules)[u4]);
   }
    
   some_functions = 0;
   if (parentProc.some_functions) {
      some_functions = new pdvector<function_base *>;
      for (unsigned u5 = 0; u5 < parentProc.some_functions->size(); u5++)
         (*some_functions).push_back((*parentProc.some_functions)[u5]);
   }

#if defined(os_linux) && defined(arch_x86)
   signal_restore = parentProc.signal_restore;
#else
   signal_handler = parentProc.signal_handler;
#endif
   execed_ = false;

#if defined(SHM_SAMPLING) && defined(sparc_sun_sunos4_1_3)
   childUareaPtr = NULL;
#endif

   createRepresentativeLWP();

   if (!attach()) {     // moved from ::forkProcess
       status_ = detached;
      showErrorCallback(69, "Error in fork: cannot attach to child process");
      set_status(exited);
      return;
   }

   if(! multithread_capable()) {
      // for single thread, add the single thread
      assert(parentProc.threads.size() == 1);
      dyn_thread *parent_thr = parentProc.threads[0];
      dyn_thread *new_thr = new dyn_thread(this, parent_thr);
      threads.push_back(new_thr);      
   }
   // the threads for MT programs are added through later, through function
   // process::recognize_threads

   if( isRunning_() )
      set_status(running);
   else
      set_status(stopped);
   // would neonatal be more appropriate?  Nah, we've reached the first trap

#ifndef BPATCH_LIBRARY
#ifdef PAPI
   if (isPapiInitialized()) {
      papi = new papiMgr(this);
   }
#endif
#endif

   initTrampGuard();
}

// #endif

#ifdef SHM_SAMPLING
void process::registerInferiorAttachedSegs(void *inferiorAttachedAtPtr) {
   shmsample_cerr << "process pid " << getPid() << ": welcome to register with inferiorAttachedAtPtr=" << inferiorAttachedAtPtr << endl;

   theSharedMemMgr->registerInferiorAttachedAt(inferiorAttachedAtPtr);
}

// returns the offset address (ie. the offset in the shared memory manager
// segment) of the offset meta offset data (used to communicate the location
// of the shared meta data to the RTinst library)
Address process::initSharedMetaData() {
   shmMetaData->mallocInShm();
   shmMetaData->initialize(theSharedMemMgr->cookie, getpid(), getPid());
   assert(shMetaOffsetData == NULL);
   shMetaOffsetData = new sharedMetaOffsetData(*theSharedMemMgr, 
					       maxNumberOfThreads());
   shmMetaData->saveOffsetsIntoRTstructure(shMetaOffsetData);

   Address offsetOfShMetaOffsetData = shMetaOffsetData->getAddrInDaemon()
                          - (Address) theSharedMemMgr->getBaseAddrInDaemon();
   return offsetOfShMetaOffsetData;
}
#endif


#if defined(rs6000_ibm_aix4_1)
void aix_pre_allocate(process *theProc) {
    // Pre-emptively allocate a HUGE chunk of memory right below
    // the shared memory segment -- we need this to instrument 
    // libraries anyway, so grab it now. This massively simplifies
    // life. 
    bool err = false;
    Address alloc;
    pdvector<Address> allocations;
    do {
        alloc = theProc->inferiorMalloc((unsigned) 1024*1024, 
                                        (inferiorHeapType) anyHeap, 
                                        (Address) 0xd0000000, 
                                        &err);    
        allocations.push_back(alloc);
    } while (alloc > (0xd0000000 - 0x02000000));
    // 0x02... is POWER's branch range. Basically, slurp the memory
    // below the shared library segment (pre-allocate). 
    for (unsigned i = 0; i < allocations.size(); i++)
        if (allocations[i])
            theProc->inferiorFree(allocations[i]);
}
#endif

extern bool forkNewProcess(pdstring &file, pdstring dir,
                           pdvector<pdstring> argv, pdstring inputFile,
                           pdstring outputFile, int &traceLink, int &pid,
                           int &tid, int &procHandle, int &thrHandle,
                           int stdin_fd, int stdout_fd, int stderr_fd);

/*
 * Create a new instance of the named process.  Read the symbols and start
 *   the program
 */
process *ll_createProcess(const pdstring File, pdvector<pdstring> argv, 
                       const pdstring dir = "", int stdin_fd=0,
                       int stdout_fd=1, int stderr_fd=2)
{
	// prepend the directory (if any) to the filename,
	// unless the filename is an absolute pathname
	// 
	// The filename is an absolute pathname if it starts with a '/' on UNIX,
	// or a letter and colon pair on Windows.
    
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
    for (unsigned i1=0; i1<argv.size(); i1++) {
      if (argv[i1] == "<") {
        inputFile = argv[i1+1];
        for (unsigned j=i1+2, k=i1; j<argv.size(); j++, k++)
          argv[k] = argv[j];
        argv.resize(argv.size()-2);
      }
    }
    // TODO -- this assumes no more than 1 of each "<", ">"
    pdstring outputFile;
    for (unsigned i2=0; i2<argv.size(); i2++) {
      if (argv[i2] == ">") {
        outputFile = argv[i2+1];
        for (unsigned j=i2+2, k=i2; j<argv.size(); j++, k++)
          argv[k] = argv[j];
        argv.resize(argv.size()-2);
      }
    }


#endif /* BPATCH_LIBRARY */

    int traceLink = -1;
    //remove all ioLink related code for output redirection
    //int ioLink = stdout_fd;

    int pid;
    int tid;
    // Ignored except on NT (where we modify in forkNewProcess, and ignore the result???)
    int procHandle_temp;
    int thrHandle_temp;

#ifndef BPATCH_LIBRARY

    struct stat file_stat;
    int stat_result;

    stat_result = stat(file.c_str(), &file_stat);
    
    if(stat_result == -1) {
        pdstring msg = pdstring("Can't read executable file ") + file + (": ") + strerror(errno);
        showErrorCallback(68, msg.c_str());
        return(NULL);
    }

#endif

    if (!forkNewProcess(file, dir, argv, inputFile, outputFile,
                        traceLink, pid, tid, procHandle_temp, thrHandle_temp,
                        stdin_fd, stdout_fd, stderr_fd)) {
        // forkNewProcess is responsible for displaying error messages
        // Note: if the fork succeeds, but exec fails, forkNew...
        // will return true. 
        return NULL;
    }
#ifdef BPATCH_LIBRARY
    // Register the pid with the BPatch library (not yet associated with a
    // BPatch_thread object).
    assert(BPatch::bpatch != NULL);
    BPatch::bpatch->registerProvisionalThread(pid);
#endif

#if defined(i386_unknown_nt4_0)
    int status = procHandle_temp;
#else
    int status = pid;
#endif // defined(i386_unknown_nt4_0)
    
    // Get the file descriptor for the executable file
    // "true" value is for AIX -- waiting for an initial trap
    // it's ignored on other platforms
    fileDescriptor *desc = getExecFileDescriptor(file, status, true);
    if (!desc) {
        cerr << "Failed to find exec descriptor" << endl;
        return NULL;
    }
    // What a hack... getExecFileDescriptor waits for a trap
    // signal on AIX and returns the code in status. So we basically
    // have a pending TRAP that we need to handle, but not right
    // now.
#if defined(rs6000_ibm_aix4_1)
    int fileDescSignal = status;
#endif

    image *img = image::parseImage(desc);
    if (!img) {
      // For better error reporting, two failure return values would be
      // useful.  One for simple error like because-file-not-because.
      // Another for serious errors like found-but-parsing-failed 
      //    (internal error; please report to paradyn@cs.wisc.edu)
      pdstring msg = pdstring("Unable to parse image: ") + file;
      showErrorCallback(68, msg.c_str());
      // destroy child process
      OS::osKill(pid);
      return(NULL);
    }

    /* parent */
    statusLine("initializing process data structures");

    process *theProc = new process(pid, img, traceLink

#ifdef SHM_SAMPLING
			       , 7000 // shm seg key to try first
#endif
			       );
    // change this to a ctor that takes in more args
    assert(theProc);

    img->defineModules(theProc);

#ifdef mips_unknown_ce2_11 //ccw 27 july 2000 : 29 mar 2001
    //the MIPS instruction generator needs the Gp register value to
    //correctly calculate the jumps.  In order to get it there it needs
    //to be visible in Object-nt, and must be taken from process.
    void *cont;
    //DebugBreak();
    cont = theProc->GetRegisters(thrHandle_temp); //ccw 10 aug 2000 : ADD thrHandle HERE!
    img->getObjectNC().set_gp_value(((w32CONTEXT*) cont)->IntGp);
#endif
    processVec.push_back(theProc);
    activeProcesses++;

    // find the signal handler function
    theProc->findSignalHandler(); // should this be in the ctor?

    // In the ST case, threads[0] (the only one) is effectively
    // a pass-through for process operations. In MT, it's the
    // main thread of the process and is handled correctly

    theProc->createInitialThread();

#if defined(rs6000_ibm_aix3_2) || defined(rs6000_ibm_aix4_1)
    // XXXX - this is a hack since getExecFileDescriptor needed to wait for
    //    the TRAP signal.
    // We really need to move most of the above code (esp parse image)
    //    to the TRAP signal handler.  The problem is that we don't
    //    know the base addresses until we get the load info via ptrace.
    //    In general it is even harder, since dynamic libs can be loaded
    //    at any time.
    procevent ev;
    ev.proc = theProc;
    ev.why  = procSignalled;
    ev.what = fileDescSignal;
    ev.info = 0;
    getSH()->handleProcessEvent(ev);    
#endif

    bool res = theProc->loadDyninstLib();
    if(res == false) {
        // Error message printout macro
        printLoadDyninstLibraryError();
        theProc->detachProcess(false);
        delete theProc;
        return NULL;
    }

    while (!theProc->reachedBootstrapState(bootstrapped)) {
       // We're waiting for something... so wait
       // true: block until a signal is received (efficiency)
       if(theProc->hasExited()) {
           delete theProc;
          return NULL;
       }

       getSH()->checkForAndHandleProcessEvents(true);
    }

#if defined(rs6000_ibm_aix4_1)
    if(theProc->multithread_capable()) {
#if !defined(AIX_PROC)
        // Unneeded with aix's /proc -- we don't need to allocate
        // base tramps within inferior RPCs
       aix_pre_allocate(theProc);
#endif
    }
#endif

    if(process::IndependentLwpControl())
       theProc->independentLwpControlInit();

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

  attach_cerr << "welcome to attachProcess for pid " << pid << endl;

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

#if defined(i386_unknown_nt4_0)
  int status = (int)INVALID_HANDLE_VALUE;	// indicates we need to obtain a valid handle
#else
  int status = pid;
#endif // defined(i386_unknown_nt4_0)

  fileDescriptor *desc = getExecFileDescriptor(fullPathToExecutable,
					       status, false);
  if (!desc) {
      return NULL;
  }

  image *theImage = image::parseImage(desc);

  if (theImage == NULL) {
    // two failure return values would be useful here, to differentiate
    // file-not-found vs. catastrophic-parse-error.
    pdstring msg = pdstring("Unable to parse image: ") + fullPathToExecutable;
    showErrorCallback(68, msg.c_str());
    return NULL;
  }

  // NOTE: the actual attach happens in the process "attach" constructor:
  bool success=false;
  process *theProc = new process(pid, theImage, success
#ifdef SHM_SAMPLING
				 ,7000 // shm seg key to try first
#endif                            
				 );
  assert(theProc);

  if (!success) {
    // XXX Do we need to do something to get rid of theImage, too?
    delete theProc;
    return NULL;
  }

  theImage->defineModules(theProc);

  // Note: it used to be that the attach ctor called pause()...not anymore...so
  // the process is probably running even as we speak.
  
  processVec.push_back(theProc);
  activeProcesses++;

  theProc->createInitialThread();

  // we now need to dynamically load libdyninstRT.so.1 - naim
  if (!theProc->pause()) {
    logLine("WARNING: pause failed\n");
    assert(0);
  }

  // find the signal handler function
  theProc->findSignalHandler(); // shouldn't this be in the ctor?

  if (!theProc->loadDyninstLib()) {
      printLoadDyninstLibraryError();
      delete theProc;
      return NULL;
  }

  // The process is paused at this point. Run if appropriate.

#if defined(alpha_dec_osf4_0)
  // need to perform this after dyninst Heap is present and happy

    // Actually, we need to perform this after DYNINSTinit() has
    // been invoked in the mutatee.  So, the following line was
    // moved to BPatch_thread.C.   RSC 08-26-2002
  //theProc->getDyn()->setMappingHooks(theProc);
#endif

  return theProc; // successful
}




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
    // Wait for the process to get to an initialized (dlopen exists)
    // state
    while (!reachedBootstrapState(initialized)) {
       if(hasExited()) {
           cerr << "Process exited" << endl;
           return false;
       }
       getSH()->checkForAndHandleProcessEvents(true);
    }
    assert (isStopped());

    // We've hit the initialization trap, so load dyninst lib and
    // force initialization
    pdstring buffer = pdstring("PID=") + pdstring(pid);
    buffer += pdstring(", initializing shared objects");       
    statusLine(buffer.c_str());

    // Perform initialization...
    if (!dyn->initialize()) assert (0 && "Static executable: unhandled");

    // And get the list of all shared objects in the process. More properly,
    // get the address of dlopen.
    if (!getSharedObjects()) assert (0 && "Failed to get shared objects in initialization");

    if (dyninstLibAlreadyLoaded()) {
        logLine("ERROR: dyninst library already loaded, we missed initialization!");
        assert(0);
    }
    
    if (!getDyninstRTLibName()) return false;
    
    // Set up a callback to be run when dyninst lib is loaded
    // Windows has some odd naming problems, so we only use the root
#if defined(i386_unknown_nt4_0)
    char dllFilename[_MAX_FNAME];
    _splitpath( dyninstRT_name.c_str(),
                NULL, NULL, dllFilename, NULL);
    
    registerLoadLibraryCallback(pdstring(dllFilename), dyninstLibLoadCallback, NULL);
#else
    registerLoadLibraryCallback(dyninstRT_name, dyninstLibLoadCallback, NULL);
#endif // defined(i386_unknown_nt4_0)
    
    // Force a call to dlopen(dyninst_lib)
    buffer = pdstring("PID=") + pdstring(pid);
    buffer += pdstring(", loading dyninst library");       
    statusLine(buffer.c_str());
    loadDYNINSTlib();
    setBootstrapState(loadingRT);
    
    if (!continueProc()) {
        assert(0);
    }

    // Loop until the dyninst lib is loaded
    while (!reachedBootstrapState(loadedRT)) {
        if(hasExited()) {
            cerr << "Process exited" << endl;
            return false;
        }
        getSH()->checkForAndHandleProcessEvents(true);
    }

    // We haven't inserted a trap at dlopen yet (as we require the runtime lib for that)
    // So re-check all loaded libraries (and add to the list gotten earlier)
    // We force a compare even though the PC is not at the correct address.
    dyn->set_force_library_check();
    handleIfDueToSharedObjectMapping();
    dyn->unset_force_library_check();

    // Make sure the library was actually loaded
    if (!runtime_lib) {
        cerr << "Don't have runtime library handle" << endl;
        return false;
    }
    

    // Get rid of the callback
    unregisterLoadLibraryCallback(dyninstRT_name);

    buffer = pdstring("PID=") + pdstring(pid);
    buffer += pdstring(", initializing mutator-side structures");
    statusLine(buffer.c_str());    

    // The following calls depend on the RT library being parsed,
    // but not on dyninstInit being run
    initInferiorHeap();
    // This must be done after the inferior heap is initialized
    initTrampGuard();
    extern pdvector<sym_data> syms_to_find;
    if (!heapIsOk(syms_to_find)) {
        bperr( "heap not okay\n");
        return false;
    }
    
    // The library is loaded, so do mutator-side initialization
    buffer = pdstring("PID=") + pdstring(pid);
    buffer += pdstring(", finalizing RT library");
    statusLine(buffer.c_str());    
    int result = finalizeDyninstLib();

    if (!result) {
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
bool process::setDyninstLibPtr(shared_object *RTobj) {
    assert (!runtime_lib);
    
    runtime_lib = RTobj;
    return true;
}


// Set up the parameters for DYNINSTinit in the RT lib

bool process::setDyninstLibInitParams() {

   attach_cerr << "process::setDYNINSTinitArguments()" << endl;

   int pid = getpid();
   
   // Cause: 
   // 1 = created
   // 2 = forked
   // 3 = attached
   
   int cause;
   if (createdViaAttach) {
       cause = 3; 
   }
   else if (createdViaFork) {
       cause = 2;
   }
   
   else 
       cause = 1;
   
   // Now we write these variables into the following global vrbles
   // in the dyninst library:
   // libdyninstAPI_RT_init_localCause
   // libdyninstAPI_RT_init_localPid

   Symbol causeSym;
   if (!getSymbolInfo("libdyninstAPI_RT_init_localCause", causeSym))
       if (!getSymbolInfo("_libdyninstAPI_RT_init_localCause", causeSym))
           assert(0 && "Could not find symbol libdyninstAPI_RT_init_localCause");
   assert(causeSym.type() != Symbol::PDST_FUNCTION);
   writeDataSpace((void*)causeSym.addr(), sizeof(int), (void *)&cause);
   
   Symbol pidSym;
   if (!getSymbolInfo("libdyninstAPI_RT_init_localPid", pidSym))
       if (!getSymbolInfo("_libdyninstAPI_RT_init_localPid", pidSym))
           assert(0 && "Could not find symbol libdyninstAPI_RT_init_localPid");
   assert(pidSym.type() != Symbol::PDST_FUNCTION);
   writeDataSpace((void*)pidSym.addr(), sizeof(int), (void *)&pid);
   
   attach_cerr << "process::installBootstrapInst() complete" << endl;
   
   return true;
}

// Callback for the above
void process::dyninstLibLoadCallback(process *p, pdstring /*ignored*/, shared_object *libobj, void * /*ignored*/) {
    p->setDyninstLibPtr(libobj);
    p->setDyninstLibInitParams();
}

// Call DYNINSTinit via an inferiorRPC
bool process::iRPCDyninstInit() {
    // Duplicates the parameter code in setDyninstLibInitParams()
    int cause;
    int pid = getpid();

    if (createdViaAttach)
        cause = 3;
    else if (createdViaFork)
        cause = 2;
    else 
        cause = 1;

    pdvector<AstNode*> the_args(2);
    the_args[0] = new AstNode(AstNode::Constant, (void*)cause);
    the_args[1] = new AstNode(AstNode::Constant, (void*)pid);
    AstNode *dynInit = new AstNode("DYNINSTinit", the_args);
    removeAst(the_args[0]); removeAst(the_args[1]);
    getRpcMgr()->postRPCtoDo(dynInit,
                             true, // Don't update cost
                             process::DYNINSTinitCompletionCallback,
                             NULL, // No user data
                             true, // Use reserved memory
                             NULL, NULL);// No particular thread or LWP

    // We loop until dyninst init has run (check via the callback)
    while (!reachedBootstrapState(bootstrapped)) {
        getRpcMgr()->launchRPCs(false); // false: not running
        if(hasExited())
           return false;
        getSH()->checkForAndHandleProcessEvents(true);
    }
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

   if(! getRepresentativeLWP()->attach())
      return false;

   while (lwp_iter.next(index, lwp)) {
      if (!lwp->attach()) {
         deleteLWP(lwp);
         return false;
      }
   }
   return setProcessFlags();
}

void process::DYNINSTinitCompletionCallback(process* theProc,
                                            unsigned /* rpc_id */,
                                            void* /*userData*/, // user data
                                            void* /*ret*/) // return value from DYNINSTinit
{
    theProc->finalizeDyninstLib();
}

// Callback: finish mutator-side processing for dyninst lib

bool process::finalizeDyninstLib() {
    assert (isStopped());
   if (reachedBootstrapState(bootstrapped)) {
       return true;
   }

   DYNINST_bootstrapStruct bs_record;
   if (!extractBootstrapStruct(&bs_record))
      assert(false);

   // Read the structure; if event 0 then it's undefined! (not yet written)
   if (bs_record.event == 0){
       return false;
   }

   // Now that we have the dyninst library loaded, insert the hooks to dlopen/dlclose
   // (which may require the dyninst library)

   // Set breakpoints to detect (un)loaded libraries
   if (!dyn->installTracing()) assert (0 && "Failed to install library mapping hooks");
    
   assert(bs_record.event == 1 || bs_record.event == 2 || bs_record.event==3);

   bool calledFromFork = (bs_record.event == 2);
   bool calledFromAttach = (bs_record.event == 3);
   
   if (!calledFromFork) {
       getObservedCostAddr();

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

       //#ifdef i386_unknown_linux2_4
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
          //#ifdef i386_unknown_linux2_4
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
   
   if (!calledFromAttach) {
       pdstring str=pdstring("PID=") + pdstring(bs_record.pid) + ", dyninst ready.";
       statusLine(str.c_str());
   }

   // Ready to rock
   setBootstrapState(bootstrapped);
   
   if (wasExeced()) {
       BPatch::bpatch->registerExec(bpatch_thread);
   }
   
   return true;
}

dyn_thread *process::STdyn_thread() { 
   assert(! multithread_capable());
   assert(threads.size()>0);
   return threads[0];
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

bool AttachToCreatedProcess(int pid,const pdstring &progpath)
{


    int traceLink = -1;

    pdstring fullPathToExecutable = process::tryToFindExecutable(progpath, pid);
    
    if (!fullPathToExecutable.length()) {
      return false;
    }  

#ifdef BPATCH_LIBRARY
    // Register the pid with the BPatch library (not yet associated with a
    // BPatch_thread object).
    assert(BPatch::bpatch != NULL);
    BPatch::bpatch->registerProvisionalThread(pid);
#endif

    int status = pid;
    
    // Get the file descriptor for the executable file
    // "true" value is for AIX -- waiting for an initial trap
    // it's ignored on other platforms
    fileDescriptor *desc = 
        getExecFileDescriptor(fullPathToExecutable, status, true);
    // What a hack... getExecFileDescriptor waits for a trap
    // signal on AIX and returns the code in status. So we basically
    // have a pending TRAP that we need to handle, but not right
    // now.
#if defined(rs6000_ibm_aix4_1)
    int fileDescSignal = status;
#endif 

    if (!desc) {
      return false;
    }

#ifndef BPATCH_LIBRARY

// We bump up batch mode here; the matching bump-down occurs after 
// shared objects are processed (after receiving the SIGSTOP indicating
// the end of running DYNINSTinit; more specifically, 
// finalizeDyninstInit(). Prevents a diabolical w/w deadlock on 
// solaris --ari
    tp->resourceBatchMode(true);

#endif

    image *img = image::parseImage(desc);

    if (img==NULL) {

        // For better error reporting, two failure return values would be
        // useful.  One for simple error like because-file-not-because.
        // Another for serious errors like found-but-parsing-failed 
        //    (internal error; please report to paradyn@cs.wisc.edu)

        pdstring msg = pdstring("Unable to parse image: ") + fullPathToExecutable;
        showErrorCallback(68, msg.c_str());
        // destroy child process
        OS::osKill(pid);
        return(false);
    }

    /* parent */
    statusLine("initializing process data structures");

    // The same process ctro. is used as in the "normal" case but
    // here, traceLink is -1 instead of a positive value.
    process *ret = new process(pid, img, traceLink

#ifdef SHM_SAMPLING
			       , 7000  // shm seg key to try first
#endif
                                   );

    assert(ret);

    img->defineModules(ret);

#ifdef mips_unknown_ce2_11 //ccw 27 july 2000 : 29 mar 2001

    //the MIPS instruction generator needs the Gp register value to
    //correctly calculate the jumps.  In order to get it there it needs
    //to be visible in Object-nt, and must be taken from process.
    void *cont;
    //DebugBreak();
    //ccw 10 aug 2000 : ADD thrHandle HERE
    
    cont = ret->GetRegisters(thrHandle_temp);

    img->getObjectNC().set_gp_value(((w32CONTEXT*) cont)->IntGp);
#endif

    processVec.push_back(ret);
    activeProcesses++;

#ifndef BPATCH_LIBRARY
    if (!costMetric::addProcessToAll(ret)) {
        assert(false);
    }
#endif

    // find the signal handler function
    ret->findSignalHandler(); // should this be in the ctor?

    // initializing vector of threads - thread[0] is really the 
    // same process

    ret->createInitialThread();

    // initializing hash table for threads. This table maps threads to
    // positions in the superTable - naim 4/14/97

#if defined(rs6000_ibm_aix3_2) || defined(rs6000_ibm_aix4_1)
    // XXXX - this is a hack since getExecFileDescriptor needed to wait for
    //        the TRAP signal.
    // We really need to move most of the above code (esp parse image)
    // to the TRAP signal handler.  The problem is that we don't
    // know the base addresses until we get the load info via ptrace.
    // In general it is even harder, since dynamic libs can be loaded
    // at any time.
    procevent ev;
    ev.proc = ret;
    ev.why  = procSignalled;
    ev.what = fileDescSignal;
    ev.info = 0;
    getSH()->handleProcessEvent(ev);
#endif
    
    return(true);

} // end of AttachToCreatedProcess


#ifdef SHM_SAMPLING
void process::processCost(unsigned obsCostLow,
                          timeStamp wallTime,
                          timeStamp processTime) {
  // wallTime and processTime should compare to DYNINSTgetWallTime() and
  // DYNINSTgetCPUtime().

  // check for overflow, add to running total, convert cycles to seconds, and
  // report.  Member vrbles of class process: lastObsCostLow and cumObsCost
  // (the latter a 64-bit value).

  // code to handle overflow used to be in rtinst; we borrow it pretty much
  // verbatim. (see rtinst/RTposix.c)
  if (obsCostLow < lastObsCostLow) {
    // we have a wraparound
    cumObsCost += ((unsigned)0xffffffff - lastObsCostLow) + obsCostLow + 1;
  }
  else
    cumObsCost += (obsCostLow - lastObsCostLow);
  
  lastObsCostLow = obsCostLow;
  //  sampleVal_cerr << "processCost- cumObsCost: " << cumObsCost << "\n"; 
  timeLength observedCost(cumObsCost, getCyclesPerSecond());
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
#ifdef BPATCH_LIBRARY
bool process::multithread_capable(bool)
#else
bool process::multithread_capable(bool ignore_if_mt_not_set)
#endif
{
#ifdef BPATCH_LIBRARY
   return false;
#else
   if(cached_result != not_cached) {
      if(cached_result == cached_mt_true) {
         return true;
      } else {
         assert(cached_result == cached_mt_false);
         return false;
      }
   }

   if(!symbols || !shared_objects) {
      if(! ignore_if_mt_not_set) {
         cerr << "   can't query MT state, assert\n";
         assert(false);
      }
      return false;
   }

   if(findModule("libthread.so.1", true) ||  // Solaris
      findModule("libpthreads.a", true)  ||  // AIX
      findModule("libpthread.so.0", true))   // Linux
   {
      cached_result = cached_mt_true;
      return true;
   } else {
      cached_result = cached_mt_false;
      return false;
   }
#endif
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
      if(foundLWP == NULL  &&  getRepresentativeLWP() != NULL)
         if(getRepresentativeLWP()->status() == stopped ||
            getRepresentativeLWP()->status() == neonatal)
            foundLWP = getRepresentativeLWP();
   } else {
      if(this->status() == stopped || this->status() == neonatal) {
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

   return stopped_lwp;
}

bool process::terminateProc() {
   if(status() == exited) {
      return false;
   }
   bool retVal = terminateProc_();

   // handle the kill signal on the process, which will dispatch exit callback
   getSH()->checkForAndHandleProcessEvents(true);
   return retVal;
}

/*
 * Copy data from controller process to the named process.
 */
bool process::writeDataSpace(void *inTracedProcess, unsigned size,
                             const void *inSelf) {
   bool needToCont = false;

   if (!isAttached()) return false;

   dyn_lwp *stopped_lwp = query_for_stopped_lwp();
   if(stopped_lwp == NULL) {
      stopped_lwp = stop_an_lwp(&needToCont);
      if(stopped_lwp == NULL) {
         pdstring msg =
            pdstring("System error: unable to write to process data "
                     "space (WDS): couldn't stop an lwp\n");
         showErrorCallback(38, msg);
         return false;
      }
   }
   
   bool res = stopped_lwp->writeDataSpace(inTracedProcess, size, inSelf);
   if (!res) {
      pdstring msg = pdstring("System error: unable to write to process data "
                              "space (WDS):") + pdstring(strerror(errno));
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

   if (!isAttached()) return false;

   dyn_lwp *stopped_lwp = query_for_stopped_lwp();
   if(stopped_lwp == NULL) {
      stopped_lwp = stop_an_lwp(&needToCont);
      if(stopped_lwp == NULL) {
         pdstring msg =
            pdstring("System error: unable to read to process data "
                     "space: couldn't stop an lwp\n");
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
         showErrorCallback(38, msg);
         return false;
      }
   }

#ifdef BPATCH_SET_MUTATIONS_ACTIVE
  if (!isAddrInHeap((Address)inTracedProcess)) {
    if (!saveOriginalInstructions((Address)inTracedProcess, sizeof(int)))
        return false;
    afterMutationList.insertTail((Address)inTracedProcess, sizeof(int), &data);
  }
#endif

  bool res = stopped_lwp->writeTextWord(inTracedProcess, data);
  if (!res) {
     pdstring msg = pdstring("System error: unable to write word to process "
                             "text space:") + pdstring(strerror(errno));
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

   if (!isAttached()) return false;
   dyn_lwp *stopped_lwp = query_for_stopped_lwp();
   if(stopped_lwp == NULL) {
      stopped_lwp = stop_an_lwp(&needToCont);
      if(stopped_lwp == NULL) {
         pdstring msg =
            pdstring("System error: unable to write to process text "
                     "space (WTS): couldn't stop an lwp\n");
         showErrorCallback(38, msg);
         return false;
      }
   }

#ifdef BPATCH_SET_MUTATIONS_ACTIVE
  if (!isAddrInHeap((Address)inTracedProcess)) {
    if (!saveOriginalInstructions((Address)inTracedProcess, amount))
        return false;
    afterMutationList.insertTail((Address)inTracedProcess, amount, inSelf);
  }
#endif

  bool res = stopped_lwp->writeTextSpace(inTracedProcess, amount, inSelf);

  if (!res) {
     pdstring msg = pdstring("System error: unable to write to process text "
                             "space (WTS):") + pdstring(strerror(errno));
     showErrorCallback(38, msg);
     return false;
  }
  
  if(needToCont) {
     return stopped_lwp->continueLWP();
  }
  return true;
}

// InsrucIter uses readTextSpace
//#ifdef BPATCH_SET_MUTATIONS_ACTIVE
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
      return false;
   }

   if (needToCont) {
      return stopped_lwp->continueLWP();
   }

   return true;
}
//#endif /* BPATCH_SET_MUTATIONS_ACTIVE */

void process::clearProcessEvents() {
   // first coded for Solaris, trying it out on other platforms

   // Make sure process isn't already stopped from an event. If it is,
   // handle the event.  An example where this happens is if the process is
   // stopped at a trap that signals the end of an rpc.  The loop is because
   // there are actually 2 successive traps at the end of an rpc.
   bool gotEvent = false;
   do {
      pdvector<procevent *> foundEvents;
      gotEvent = getSH()->checkForProcessEvents(&foundEvents, getPid(), false);
      getSH()->handleProcessEvents(foundEvents);
   } while(gotEvent == true);  // keep checking if we handled an event
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
      if(!stopped_lwp_exists && lwp_st==running)
         status_ = running;
   } else {
      // if can't do independent lwp control, should only be able to set
      // lwp status for representative lwp
      assert(whichLWP == getRepresentativeLWP());
      set_status(lwp_st);  // sets process status and all lwp statuses
   }
}

void process::clearCachedRegister() {
   pdvector<dyn_thread *>::iterator iter = threads.begin();
   
   dyn_lwp *proclwp = getRepresentativeLWP();
   if(proclwp) proclwp->clearCachedRegister();
   
   while(iter != threads.end()) {
      dyn_thread *thr = *(iter);
      dyn_lwp *lwp = thr->get_lwp();
      assert(lwp);
      lwp->clearCachedRegister();
      iter++;
   }   
}

bool process::pause() {

    if (!isAttached()) {
        bperr( "Warning: pause attempted on non-attached process\n");
        return false;
    }
      
   // The only remaining combination is: status==running but haven't yet
   // reached first break.  We never want to pause before reaching the first
   // break (trap, actually).  But should we be returning true or false in
   // this case?
   if(! reachedBootstrapState(initialized)) {
      return true;
   }

#if defined(sparc_sun_solaris2_4)
   clearProcessEvents();
#endif

   /*   DEBUGGING
      pdvector<dyn_thread *>::iterator iterA = threads.begin();

      while(iterA != threads.end()) {
         dyn_thread *thr = *(iterA);
         dyn_lwp *lwp = thr->get_lwp();
         assert(lwp);
         if(lwp->status() == stopped)
            cerr << "  lwp " << lwp->get_lwp_id() << " is stopped\n";
         else if(lwp->status() == running)
            cerr << "  lwp " << lwp->get_lwp_id() << " is running\n";
         else if(lwp->status() == neonatal)
            cerr << "  lwp " << lwp->get_lwp_id() << " is neonatal\n";
         iterA++;
      }
   */

   // Let's try having stopped mean all lwps stopped and running mean
   // atleast one lwp running.
   
   if (status_ == stopped || status_ == neonatal) {
      return true;
   }

   if(IndependentLwpControl()) {      
     setSuppressEventConts(true);
     pdvector<dyn_thread *>::iterator iter = threads.begin();
     while(iter != threads.end()) {
       dyn_thread *thr = *(iter);
       dyn_lwp *lwp = thr->get_lwp();
       assert(lwp);
       lwp->pauseLWP(true);
       iter++;
     }
     setSuppressEventConts(false);
   } else {
      assert(status_ == running);      
      bool res = getRepresentativeLWP()->pauseLWP(true);
      if (!res) {
         sprintf(errorLine,
                 "warn : in process::pause, pause_ unable to pause process\n");
         logLine(errorLine);
         return false;
      }
   }

   status_ = stopped;
   return true;
}

// handleIfDueToSharedObjectMapping: if a trap instruction was caused by
// a dlopen or dlclose event then return true
bool process::handleIfDueToSharedObjectMapping(){
   if(!dyn) { 
       bperr( "No dyn object, returning false\n");
       return false;
   }
   pdvector<shared_object *> *changed_objects = 0;
   u_int change_type = 0;
   bool error_occured = false;
   bool ok = dyn->handleIfDueToSharedObjectMapping(&changed_objects,
                                                   change_type,error_occured);
   // if this trap was due to dlopen or dlclose, and if something changed
   // then figure out how it changed and either add or remove shared objects
   if(ok && !error_occured && (change_type != SHAREDOBJECT_NOCHANGE)) {

      // if something was added then call process::addASharedObject with
      // each element in the vector of changed_objects
      if((change_type == SHAREDOBJECT_ADDED) && changed_objects) {
         for(u_int i=0; i < changed_objects->size(); i++) {
             // TODO: currently we aren't handling dlopen because  
             // we don't have the code in place to modify existing metrics
             // This is what we really want to do:
             // Paradyn -- don't add new symbols unless it's the runtime
             // library
             // UGLY.
             if(addASharedObject((*changed_objects)[i],
                                 (*changed_objects)[i]->getBaseAddress()))
             {
                 (*shared_objects).push_back((*changed_objects)[i]);
                 addCodeRange((*changed_objects)[i]->getBaseAddress() +
                              (*changed_objects)[i]->getImage()->codeOffset(),
                              (*changed_objects)[i]);
                 
                 // Check to see if there is a callback registered for this
                 // library, and if so call it.
                 
                 pdstring libname =  (*changed_objects)[i]->getName();
                 runLibraryCallback(libname, (*changed_objects)[i]);;
                 
             } // addASharedObject, above
             else {
                 //logLine("Error after call to addASharedObject\n");
                 delete (*changed_objects)[i];
             }
         }
         delete changed_objects;
         
      } else if((change_type == SHAREDOBJECT_REMOVED) && (changed_objects)) { 

         // TODO: handle this case
         // if something was removed then call process::removeASharedObject
         // with each element in the vector of changed_objects
         // for now, just delete shared_objects to avoid memory leeks
         for(u_int i=0; i < changed_objects->size(); i++){
            delete (*changed_objects)[i];
         }

         delete changed_objects;
      }

      // TODO: add support for adding or removing new code resource once the 
      // process has started running...this means that currently collected
      // metrics may have to have aggregate components added or deleted
      // this should be added to process::addASharedObject and 
      // process::removeASharedObject  
   }

   return ok;
}

// Register a callback to be made when a library is detected
// in handleSharedObjectMapping (above). In many cases this allows
// a user to modify the library before any associated _init function
// is run. Note: ordering is NOT guaranteed. This is best-effort.

bool process::registerLoadLibraryCallback(pdstring libname, 
                                          loadLibraryCallbackFunc callback,
                                          void *data) {
    if (loadLibraryCallbacks_.defines(libname)) {
        cerr << "Possible error: predefined callback for "
             << libname << " being overwritten!" << endl;
        // Return false here? For now, continue
    }
    libraryCallback *lib = new libraryCallback();
    lib->callback = callback;
    lib->data = data;
    loadLibraryCallbacks_[libname] = lib;
    return true;    
}

// Unregister the above callback. Arbitrary decision:
// callbacks are persistent until unregistered.
bool process::unregisterLoadLibraryCallback(pdstring libname) {
    if (loadLibraryCallbacks_.defines(libname)) {
        libraryCallback *lib = loadLibraryCallbacks_[libname];
        loadLibraryCallbacks_.undef(libname);
        delete lib;
        return true;
    }
    return false;
}

//
bool process::runLibraryCallback(pdstring libname, shared_object *libobj) {
    if (loadLibraryCallbacks_.defines(libname)) {
        libraryCallback *lib = loadLibraryCallbacks_[libname];
        (lib->callback)(this, libname, libobj, lib->data);
    }
    return true;
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
check_rtinst(process *proc, shared_object *so)
{
     const char *name, *p;
     static const char *libdyn = "libdyninst";
     static int len = 10; /* length of libdyn */

     name = (so->getName()).c_str();

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
     Symbol sym;
     if (! so->getSymbolInfo("DYNINSThasInitialized", sym)) {
	  return 0;
     }
     Address addr = sym.addr() + so->getBaseAddress();
     unsigned int val;
     if (! proc->readDataSpace((void*)addr, sizeof(val), (void*)&val, true)) {
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
bool process::addASharedObject(shared_object *new_obj, Address newBaseAddr){
    int ret;
    pdstring msg;

    if(new_obj->getName().length() == 0) {
        fprintf(stderr, "Null name on object\n");
        return false;
    }

    image *img = image::parseImage(new_obj->getFileDesc(),newBaseAddr); 
    
    if(!img){
        //logLine("error parsing image in addASharedObject\n");
        bperr( "No image: failed parse\n");
        return false;
    }

    img->defineModules(this);

    new_obj->addImage(img);

        ///ccw 20 apr 2004 : test4 linux bug hack
        /* what is going on here? If you relocate a function in a shared library,
           then call exec (WITHOUT CALLING FORK) the function will continue to be
           marked as relocated BY THE EXEC'ED PROCESS even though the shared 
           library will have been reloaded.

           So, to fix this, we look to see if each function is marked as relocated 
	   by the exec'ed process and remove the relocation tag connecting it with 
	   the said process.  

           The function unrelocatedByProcess was added to pd_Function in symtab.h

           This only effects exec and not fork since fork creates a new process
           and exec does not.  check to see how pd_Function::hasBeenRelocated()
           works for more info.
        */
                   
        const pdvector<pd_Function*> *allFuncs = new_obj->getAllFunctions();

        for(int funcIndex=0; execed_ && funcIndex<allFuncs->size();funcIndex++){
                if( (*allFuncs)[funcIndex]->hasBeenRelocated(this) ){
                        (*allFuncs)[funcIndex]->unrelocatedByProcess(this);
                }
        }

    // TODO: check for "is_elf64" consistency (Object)

    // If the static inferior heap has been initialized, then 
    // add whatever heaps might be in this guy to the known heap space.
    if (heap.bufferPool.size() != 0)
      // Need to search/add heaps here, instead of waiting for
      // initInferiorHeap.
        addInferiorHeap(img, newBaseAddr);


#if !defined(i386_unknown_nt4_0)  && !(defined mips_unknown_ce2_11) //ccw 20 july 2000 : 29 mar 2001
    /* If we're not currently trying to load the runtime library,
       check whether this shared object is the runtime lib. */
    if (!(bootstrapState == loadingRT)
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

    // if the list of all functions and all modules have already been 
    // created for this process, then the functions and modules from this
    // shared object need to be added to those lists 
    if(all_modules){
      pdvector<module *> *vptr = const_cast< pdvector<module *> *>(reinterpret_cast< const pdvector<module *> *>(new_obj->getModules()));
      VECTOR_APPEND(*all_modules, *vptr);
    }
    if(all_functions){
      pdvector<function_base *> *normal_funcs = (pdvector<function_base *> *)
                const_cast< pdvector<pd_Function *> *>(new_obj->getAllFunctions());
	// need to concat two vectors ...
        VECTOR_APPEND(*all_functions, *normal_funcs); 
        normal_funcs = 0;
    }

    // if the signal handler function has not yet been found search for it
#if defined(i386_unknown_linux2_0)
    if(signal_restore.size() == 0){
      Symbol s;
      if (img->symbol_info(SIGNAL_HANDLER, s)) {
	  // Add base address of shared library
	signal_restore.push_back(s.addr() + new_obj->getBaseAddress());
      }
    }
#else
    //  JAW -- 04-03  SIGNAL HANDLER seems to only be validly defined for solaris/linux/mips
    //      I don't know if this is correct, but given this fact, I'm putting in the following #if
#if defined(sparc_sun_solaris2_4)  \
    || defined(sparc_sun_solaris2_5)
 
    if(!signal_handler){
      pdvector<pd_Function *> *pdfv;
      if (NULL == (pdfv = img->findFuncVectorByPretty(pdstring(SIGNAL_HANDLER)))
	  || ! pdfv->size()) {
	//cerr << __FILE__ << __LINE__ << ": findFuncVectorByPretty could not find "
	//    << pdstring(SIGNAL_HANDLER) << endl;
	// what to do here?

	// JAW: ANSWER, for now:  NOTHING
	//signal_handler will eventually be found (hopefully)
	// the search for SIGNAL_HANDLER should probably be moved somewhere else
	// but, for the time being, not finding it here is not an error
      }
      else {
	if (pdfv->size() > 1){
	  cerr << __FILE__ << __LINE__ << ": findFuncVectorByPretty found " << pdfv->size()
	       <<" functions called "<< pdstring(SIGNAL_HANDLER) << endl;
	}
	//cerr << SIGNAL_HANDLER << " found." << endl;
        signal_handler = (*pdfv)[0];
      }
    }
#endif // sparc-mips
#endif // !linux

    // clear the include_funcs flag if this shared object should not be
    // included in the some_functions and some_modules lists
#ifndef BPATCH_LIBRARY
    pdvector<pdstring> lib_constraints;
    if(mdl_get_lib_constraints(lib_constraints)){
        for(u_int j=0; j < lib_constraints.size(); j++){
           char *where = 0; 
           // if the lib constraint is not of the form "module/function" and
           // if it is contained in the name of this object, then exclude
           // this shared object
           char *obj_name = P_strdup(new_obj->getName().c_str());
           char *lib_name = P_strdup(lib_constraints[j].c_str());
           if(obj_name && lib_name && (where=P_strstr(obj_name, lib_name))){
              new_obj->changeIncludeFuncs(false); 
           }
           if(lib_name) free(lib_name);
           if(obj_name) free(obj_name);
        }
    }

    // This looks a bit wierd at first glance, but apparently is ok -
    //  A shared object has 1 module.  If that module is excluded,
    //  then new_obj->includeFunctions() should return FALSE.  As such,
    //  the some_modules += new_obj->getModules() is OK as long as
    //  shared objects have ONLY 1 module.
    if(new_obj->includeFunctions()){
        if(some_modules) {
            *some_modules += *((const pdvector<module *> *)(new_obj->getModules()));
        }
        if(some_functions) {
            // gets only functions not excluded by mdl "exclude_node" option
            *some_functions += 
                *((pdvector<function_base *> *)(new_obj->getIncludedFunctions()));
        }
    }
#endif /* BPATCH_LIBRARY */


#ifdef BPATCH_LIBRARY

    assert(BPatch::bpatch);
    const pdvector<pdmodule *> *modlist = new_obj->getModules();
    if (modlist != NULL) {
      for (unsigned i = 0; i < modlist->size(); i++) {
        pdmodule *curr = (*modlist)[i];
        pdstring name = curr->fileName();

	BPatch_thread *thr = BPatch::bpatch->getThreadByPid(pid);
	if(!thr)
	  continue;  //There is no BPatch_thread yet, so nothing else to do
	// this occurs in the attach case - jdd 6/30/99
	
	BPatch_image *image = thr->getImage();
	assert(image);

	BPatch_module *bpmod = NULL;
	if ((name != "DYN_MODULE") && (name != "LIBRARY_MODULE")) {
	  if( image->ModuleListExist() )
	    //cout << __FILE__ << ":" << __LINE__ << ": creating module " << name << endl;
	    bpmod = new BPatch_module(this, curr, image);
	}
	// add to module list
	if( bpmod){
	  //cout<<"Module: "<<name<<" in Process.C"<<endl;
	  image->addModuleIfExist(bpmod);
	}

        // XXX - jkh Add the BPatch_funcs here

        if (BPatch::bpatch->dynLibraryCallback) {
          BPatch::bpatch->dynLibraryCallback(thr, bpmod, true);
        }
      }
    }
    
#endif /* BPATCH_LIBRARY */

    return true;
}

// getSharedObjects: This routine is called before main() or on attach
// to an already running process.  It gets and process all shared objects
// that have been mapped into the process's address space
bool process::getSharedObjects() {
    
    if (!shared_objects) {
        // First time we were called: get all shared objects.
        shared_objects = dyn->getSharedObjects();        
        if(shared_objects){
            statusLine("parsing shared object files");
#ifndef BPATCH_LIBRARY
            tp->resourceBatchMode(true);
#endif
            // for each element in shared_objects list process the 
            // image file to find new instrumentaiton points
            for(u_int j=0; j < shared_objects->size(); j++){
	    // pdstring temp2 = pdstring(j);
// 	    temp2 += pdstring(" ");
// 	    temp2 += pdstring("the shared obj, addr: ");
// 	    temp2 += pdstring(((*shared_objects)[j])->getBaseAddress());
// 	    temp2 += pdstring(" name: ");
// 	    temp2 += pdstring(((*shared_objects)[j])->getName());
// 	    temp2 += pdstring("\n");
// 	    logLine(P_strdup(temp2.c_str()));
                if(addASharedObject((*shared_objects)[j],
                                    (*shared_objects)[j]->getBaseAddress())){
                    addCodeRange((*shared_objects)[j]->getBaseAddress() +
                                 (*shared_objects)[j]->getImage()->codeOffset(),
                                 (*shared_objects)[j]);
                    
                }
                else
                    logLine("Error after call to addASharedObject\n");
                
            }
#ifndef BPATCH_LIBRARY
            tp->resourceBatchMode(false);
#endif
            return true;
        }
        else {
            // else this a.out does not have a .dynamic section
            dynamiclinking = false;
            
            return false;
        }
    }
    else {
        // Already have shared object vector... but from where?
#if defined(os_windows)
        // We make it when we're notified of the first library debug
        // message
        return true;
#endif
        cerr << "Warning: shared_object vector already exists!" << endl;
        return true;
    }
    assert(0 && "UNREACHABLE");
    return true;
}

// findOneFunction: returns the function associated with func  
// this routine checks both the a.out image and any shared object
// images for this resource.
// Semantics of excluded functions - Once "exclude" works for both
// static and dynamically linked objects, this should return NULL
// if the function being sought is excluded....
#ifndef BPATCH_LIBRARY
function_base *process::findOnlyOneFunction(resource *func, resource *mod){
    if((!func) || (!mod)) { return 0; }
    if(func->mdlType() != MDL_T_PROCEDURE) { return 0; }
    if(mod->mdlType() != MDL_T_MODULE) { return 0; }

    const pdvector<pdstring> &f_names = func->names();
    const pdvector<pdstring> &m_names = mod->names();
    pdstring func_name = f_names[f_names.size() -1]; 
    pdstring mod_name = m_names[m_names.size() -1]; 

    return findOnlyOneFunction(func_name, mod_name);
}

function_base *process::findOnlyOneFunction(pdstring func_name,
                                            pdstring mod_name) {
    //cerr << "process::findOneFunction called.  function name = " 
    //   << func_name.c_str() << endl;
    
    // KLUDGE: first search any shared libraries for the module name 
    //  (there is only one module in each shared library, and that 
    //  is the library name)
    if(dynamiclinking && shared_objects){
        for(u_int j=0; j < shared_objects->size(); j++){
            module *next = 0;
            next = ((*shared_objects)[j])->findModule(mod_name,true);

            if(next){
                if(((*shared_objects)[j])->includeFunctions()){ 
                  //cerr << "function found in module " << mod_name.c_str() << endl;
		  //return(((*shared_objects)[j])->findFuncByName(func_name));
		  return (*shared_objects)[j]->findOnlyOneFunction(func_name);
		} 
                else { 
                  //cerr << "function found in module " << mod_name.c_str()
                  //    << " that module excluded" << endl;
                  return 0;
                } 
            }
        }
    }

    return symbols->findOnlyOneFunction(func_name);
}

bool process::findAllFuncsByName(resource *func, resource *mod, 
                                 pdvector<function_base *> &res) {
   
  pdvector<pd_Function *> *pdfv=NULL;
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
            next = ((*shared_objects)[j])->findModule(mod_name,true);

            if(next){
                if(((*shared_objects)[j])->includeFunctions()){ 
                  //cerr << "function found in module " << mod_name.c_str() << endl;
                   if (NULL != (pdfv = ((*shared_objects)[j])->findFuncVectorByPretty(func_name))) {
                      for (unsigned int i = 0; i < pdfv->size(); ++i) {
                         res.push_back((*pdfv)[i]);
                      }
                      return true;
                   }
                }
                else { 
                  //cerr << "function found in module " << mod_name.c_str()
                  //    << " that module excluded" << endl;
                  return false;
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

#endif /* BPATCH_LIBRARY */

#ifndef BPATCH_LIBRARY
// returns all the functions in the module "mod" that are not excluded by
// exclude_lib or exclude_func
// return 0 on error.
pdvector<function_base *> *process::getIncludedFunctions(module *mod) {

    if((!mod)) { return 0; }

    //cerr << "process::getIncludedFunctions(" << mod->fileName() << ") called" << endl;

    // KLUDGE: first search any shared libraries for the module name 
    //  (there is only one module in each shared library, and that 
    //  is the library name)
    if(dynamiclinking && shared_objects){
        for(u_int j=0; j < shared_objects->size(); j++){
            module *next = 0;
            next = ((*shared_objects)[j])->findModule(mod->fileName(), true);
            if(next){
                if(((*shared_objects)[j])->includeFunctions()){ 
                    return((pdvector<function_base *> *)
                           ((*shared_objects)[j])->getIncludedFunctions());
                } 
                else { return 0;} 
            }
        }
    }

    // this must be an a.out module so just return the list associated
    // with the module.
    // Now that exclude should work for both static and dynamically
    // linked executables, probably need to either filter excluded
    // files here, or let the module do it and pass the information not
    // to include excluded functions along with this proc call....
    // mcheyney 970927
    return(mod->getIncludedFunctions());
}
#endif /* BPATCH_LIBRARY */


// Parse name into library name and function name. If no library is specified,
// lib gets the empty string "". 
//
// e.g. libc.so/read => lib_name = libc.so, func_name = read
//              read => lib_name = "", func_name = read
void getLibAndFunc(const pdstring &name, pdstring &lib_name, pdstring &func_name) {

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

// findOneFunction: returns the function associated with func  
// this routine checks both the a.out image and any shared object
// images for this resource
function_base *process::findOnlyOneFunction(const pdstring &name) const {

    pdstring lib_name;
    pdstring func_name;
    function_base *pdf, *ret = NULL;

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
                    function_base *fb = so->findOnlyOneFunction(func_name);
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

// findOneFunction: returns the function associated with func  
// this routine checks both the a.out image and any shared object
// images for this resource
function_base *process::findOnlyOneFunctionFromAll(const pdstring &name) const {

  pdstring func_name;
  function_base *pdf, *ret = NULL;

  // first check a.out for function symbol
  if (NULL != (pdf = symbols->findOnlyOneFunctionFromAll(name)))
    ret = pdf;
  
  // search any shared libraries for the file name 
  if(dynamiclinking && shared_objects){
    for(u_int j=0; j < shared_objects->size(); j++){
      pdf = ((*shared_objects)[j])->findOnlyOneFunctionFromAll(name);
      if(pdf){
	// fail if we already found a match
	if (ret) {
	  char msg[256];
	  sprintf(msg, "%s[%d]:  ERROR:  findOnlyOneFunction "
		  " found more than one match for function %s",
		  __FILE__, __LINE__, name.c_str());
	  logLine(msg);
	  return NULL;
	}
	ret = pdf;
      }
      else {
	//  do we want to warn here?
      }
    }
  }

  return ret;
}

bool process::findAllFuncsByName(const pdstring &name, pdvector<function_base *> &res)
{
   pdstring lib_name;
   pdstring func_name;
   pdvector<pd_Function *> *pdfv=NULL;
   
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
            //pdf=static_cast<function_base *>(((*shared_objects)[j])->findFuncByName(func_name));
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
                   
                   //function_base *fb = static_cast<function_base *>(so->findFuncByName(func_name));
               }
           }
       }
   }
   
   if (res.size())
      return true; 

   //  Last ditch:  maybe the name was a mangled name
   pd_Function *pdf;

   if (NULL != (pdf = symbols->findFuncByMangled(func_name))) {
       res.push_back(pdf);
   }
   
   // search any shared libraries for the file name 
   if(dynamiclinking && shared_objects){
     for(u_int j=0; j < shared_objects->size(); j++){
       if (NULL != (pdf = (*shared_objects)[j]->findFuncByMangled(func_name))) {
	 res.push_back(pdf);
	 
       }
       //pdf=static_cast<function_base *>(((*shared_objects)[j])->findFuncByName(func_name));
     }
   }

   if (res.size())
     return true; 

   return false;
}

// Returns the named symbol from the image or a shared object
bool process::getSymbolInfo( const pdstring &name, Symbol &ret ) 
{
   if(!symbols)
      abort();
   
   bool sflag;
   sflag = symbols->symbol_info( name, ret );

   if(sflag)
      return true;
  
   if( dynamiclinking && shared_objects ) {
      for( u_int j = 0; j < shared_objects->size(); ++j ) {
         if (!(*shared_objects)[j])
            abort();

         sflag = ((*shared_objects)[j])->getSymbolInfo( name, ret );

         if( sflag ) {
            // NT already has base address added in
             ret.setAddr( ret.addr() + (*shared_objects)[j]->getBaseAddress());
             return true;
         }
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
   if (!codeRangesByAddr_->precessor(addr, range))
      return NULL;

   assert(range);

   bool in_range = (addr >= range->get_address() &&
                    addr <= (range->get_address() + range->get_size()));
   if(! in_range) {
      range = NULL;
   }

   shared_object *sharedobject_ptr = range->is_shared_object();
   if(sharedobject_ptr) {
      if (!sharedobject_ptr->isProcessed()) {
         // Very odd case....
         return false;
      }
      Address inImage = (addr - sharedobject_ptr->getBaseAddress());        
      const image *img = sharedobject_ptr->getImage();
        
      if (!img) {
         // Looks to be caused by a bug where not deregistering sharedobjs
         // from the code range data when we call delete on the sharedobj
         // (for example when an exec occurs).  General bug should be
         // fixed.
         return false;
      }
      if (inImage > (img->get_address() + img->get_size())) {
         /*
           bperr( "Warning: addr 0x%x not in code range (closest: shobj from 0x%x to 0x%x\n",
           addr, img->codeOffset() +
           range->sharedobject_ptr->getBaseAddress(),
           img->codeOffset()+img->codeLength() +
           range->sharedobject_ptr->getBaseAddress());
         */
         range = NULL;
      }
   }

   if (!range) return false;
   
   image *image_ptr = range->is_image();

   // If we're talking the a.out or a shared object, recurse....
   if(image_ptr) {
      // Assumes the base addr of the image is 0!
      // Fill in the function part as well for complete info
      pd_Function *function_ptr = image_ptr->findFuncByOffset(addr);
      range = function_ptr;
   }
   else if (sharedobject_ptr) {
       pd_Function *function_ptr = sharedobject_ptr->findFuncByAddress(addr);
       range = function_ptr;
    }
    
   return range;
}

pd_Function *process::findFuncByAddr(Address addr) {
    codeRange *range = findCodeRangeByAddress(addr);
    if (!range) return NULL;
    
    pd_Function *func_ptr = range->is_pd_Function();
    trampTemplate *basetramp_ptr = range->is_basetramp();
    miniTrampHandle *minitramp_ptr = range->is_minitramp();
    relocatedFuncInfo *reloc_ptr = range->is_relocated_func();

    if(func_ptr) {
       return func_ptr;
    }
    else if(basetramp_ptr) {
        return basetramp_ptr->location->pointFunc();
    }
    else if(minitramp_ptr) {   
        return minitramp_ptr->baseTramp->location->pointFunc();
    }
    else if (reloc_ptr) {
        return reloc_ptr->func();
    }
    else {
        return NULL;
    }
}

bool process::addCodeRange(Address addr, codeRange *codeobj) {
   codeRangesByAddr_->insert(addr, codeobj);
   return true;
}

bool process::deleteCodeRange(Address addr) {
    codeRangesByAddr_->remove(addr);
    return true;
}

    
// findModule: returns the module associated with mod_name 
// this routine checks both the a.out image and any shared object
// images for this resource
// if check_excluded is true it checks to see if the module is excluded
// and if it is it returns 0.  If check_excluded is false it doesn't check
#ifndef BPATCH_LIBRARY
pdmodule *process::findModule(const pdstring &mod_name, bool check_excluded) {
   // KLUDGE: first search any shared libraries for the module name 
   //  (there is only one module in each shared library, and that 
   //  is the library name)
   if(dynamiclinking && shared_objects){
      for(u_int j=0; j < shared_objects->size(); j++){
         pdmodule *next = ((*shared_objects)[j])->findModule(mod_name,
                                                           check_excluded);
         if(next) {
            return(next);
         }
      }
   }

   // check a.out for function symbol
   //  Note that symbols is data member of type image* (comment says
   //  "information related to the process"....
   pdmodule *mret = symbols->findModule(mod_name, check_excluded);
   return mret;
}
#else
pdmodule *process::findModule(const pdstring &mod_name)
{
   // KLUDGE: first search any shared libraries for the module name 
   //  (there is only one module in each shared library, and that 
   //  is the library name)
   if(dynamiclinking && shared_objects){
      for(u_int j=0; j < shared_objects->size(); j++){
         pdmodule *next = ((*shared_objects)[j])->findModule(mod_name);
         if(next) {
            return(next);
         }
      }
   }

   // check a.out for function symbol
   //  Note that symbols is data member of type image* (comment says
   //  "information related to the process"....
   pdmodule *mret = symbols->findModule(mod_name);
   return mret;
}
#endif

// getSymbolInfo:  get symbol info of symbol associated with name n
// this routine starts looking a.out for symbol and then in shared objects
// baseAddr is set to the base address of the object containing the symbol.
// This function appears to return symbol info even if module/function
// is excluded.  In extending excludes to statically linked executable,
// we preserve these semantics....
bool process::getSymbolInfo(const pdstring &name, Symbol &info, 
                            Address &baseAddr) const 
{
   // first check a.out for symbol
   if(symbols->symbol_info(name,info))
      return getBaseAddress(symbols, baseAddr);
   
   // next check shared objects
   if(dynamiclinking && shared_objects) {
      for(u_int j=0; j < shared_objects->size(); j++) {
         if(((*shared_objects)[j])->getSymbolInfo(name,info)) {
            return getBaseAddress(((*shared_objects)[j])->getImage(), 
                                  baseAddr); 
         }
      }
   }
    
   return false;
}


// getAllFunctions: returns a vector of all functions defined in the
// a.out and in the shared objects
// TODO: what to do about duplicate function names?
pdvector<function_base *> *process::getAllFunctions(){

    // if this list has already been created, return it
    if(all_functions) 
        return all_functions;

    // else create the list of all functions
    all_functions = new pdvector<function_base *>;
    const pdvector<function_base *> &blah = 
    (pdvector<function_base *> &)(symbols->getAllFunctions());

    VECTOR_APPEND(*all_functions,blah);

    if(dynamiclinking && shared_objects){
        for(u_int j=0; j < shared_objects->size(); j++){
           pdvector<function_base *> *funcs = (pdvector<function_base *> *) 
                        const_cast< pdvector<pd_Function *> *>
                        (((*shared_objects)[j])->getAllFunctions());

           if(funcs){
               VECTOR_APPEND(*all_functions,*funcs); 
           }
        }
    }
    return all_functions;
}
      
// getAllModules: returns a vector of all modules defined in the
// a.out and in the shared objects
// Includes "excluded" modules....
pdvector<module *> *process::getAllModules(){

    // if the list of all modules has already been created, the return it
    if(all_modules) return all_modules;

    // else create the list of all modules
    all_modules = new pdvector<module *>;
#ifdef BPATCH_LIBRARY
    VECTOR_APPEND(*all_modules,*((const pdvector<module *> *)(&(symbols->getModules()))));
#else
    VECTOR_APPEND(*all_modules,*((const pdvector<module *> *)(&(symbols->getAllModules()))));
#endif
    if(dynamiclinking && shared_objects){
        for(u_int j=0; j < shared_objects->size(); j++){
           const pdvector<module *> *mods = (const pdvector<module *> *)
                        (((*shared_objects)[j])->getModules());
           if(mods) {
               VECTOR_APPEND(*all_modules,*mods); 
           }
    } } 
    return all_modules;
}

#ifndef BPATCH_LIBRARY
// getIncludedFunctions: returns a vector of all functions defined in the
// a.out and in the shared objects
// TODO: what to do about duplicate function names?
pdvector<function_base *> *process::getIncludedFunctions(){
    //cerr << "process " << programName << " :: getIncludedFunctions() called"
    //   << endl;
    // if this list has already been created, return it
    if(some_functions) 
        return some_functions;

    // else create the list of all functions
    some_functions = new pdvector<function_base *>;
    const pdvector<function_base *> &incl_funcs = 
        (pdvector<function_base *> &)(symbols->getIncludedFunctions());
        *some_functions += incl_funcs;

    //cerr << " (process::getIncludedFunctions), about to add incl_funcs to some_functions, incl_funcs = " << endl;
    //print_func_vector_by_pretty_name(pdstring(">>>"), &incl_funcs);

    if(dynamiclinking && shared_objects){
        for(u_int j=0; j < shared_objects->size(); j++){
            if(((*shared_objects)[j])->includeFunctions()){
                // kludge: can't assign a vector<derived_class *> to 
                // a vector<base_class *> so recast
                pdvector<function_base *> *funcs = (pdvector<function_base *> *)
                        (((*shared_objects)[j])->getIncludedFunctions());
                if(funcs) { 
                    *some_functions += (*funcs); 
                } 
            } 
    } } 

    //cerr << " (process::getIncludedFunctions()) about to return fucntion list : ";
    //print_func_vector_by_pretty_name(pdstring("  "), some_functions);

    return some_functions;
}

// getIncludedModules: returns a vector of all modules defined in the
// a.out and in the shared objects that are included as specified in
// the mdl

pdvector<module *> *process::getIncludedModules(){

    //cerr << "process::getIncludedModules called" << endl;

    // if the list of all modules has already been created, the return it
    if(some_modules) {
        //cerr << "some_modules already created, returning it:" << endl;
        //print_module_vector_by_short_name(pdstring("  "), (vector<pdmodule*>*)some_modules);
        return some_modules;
    }

    // else create the list of all modules
    some_modules = new pdvector<module *>;
    *some_modules += *((const pdvector<module *> *)(&(symbols->getIncludedModules())));

    if(dynamiclinking && shared_objects){
        for(u_int j=0; j < shared_objects->size(); j++){
            if(((*shared_objects)[j])->includeFunctions()){
               const pdvector<module *> *mods = (const pdvector<module *> *) 
                        (((*shared_objects)[j])->getModules());
               if(mods) {
                   *some_modules += *mods; 
               }
           }
    } } 

    //cerr << "some_modules newly created, returning it:" << endl;
    //print_module_vector_by_short_name(pdstring("  "),
    //    (vector<pdmodule*>*)some_modules);
    return some_modules;
}
#endif /* BPATCH_LIBRARY */

// getBaseAddress: sets baseAddress to the base address of the 
// image corresponding to which.  It returns true  if image is mapped
// in processes address space, otherwise it returns 0
bool process::getBaseAddress(const image *which, Address &baseAddress) const {

  if((Address)(symbols) == (Address)(which)){
      baseAddress = 0; 
      return true;
  }
  else if (shared_objects) {  
      // find shared object corr. to this image and compute correct address
      for(unsigned j=0; j < shared_objects->size(); j++){ 
          if(((*shared_objects)[j])->isMapped()){
              if(((*shared_objects)[j])->getImage() == which) { 
                  baseAddress = ((*shared_objects)[j])->getBaseAddress();
                  return true;
              }
          }
      }
  }
  else {
      bperr( "shared_objects not defined\n");
  }
  
  return false;
}

#if defined(i386_unknown_linux2_0)
// findSignalHandler: if signal_restore is empty, then it checks all images
// associated with this process for the signal restore function.
// Otherwise, the signal restore function has already been found
void process::findSignalHandler(){
  if (signal_restore.size() == 0) {
    Symbol s;
    if (getSymbolInfo(SIGNAL_HANDLER, s))
	signal_restore.push_back(s.addr());

    signal_cerr << "process::findSignalHandler <" << SIGNAL_HANDLER << ">";
    if (signal_restore.size() == 0) signal_cerr << " NOT";
    signal_cerr << " found." << endl;
  }
}
#else
// findSignalHandler: if signal_handler is 0, then it checks all images
// associated with this process for the signal handler function.
// Otherwise, the signal handler function has already been found
void process::findSignalHandler(){
  
  pdvector<pd_Function *> *pdfv;

    if(SIGNAL_HANDLER == 0) return;
    if(!signal_handler) { 
        // first check a.out for signal handler function
      if (NULL != (pdfv = symbols->findFuncVectorByPretty(SIGNAL_HANDLER)) && pdfv->size()) {
	if (pdfv->size() > 1) {
	  cerr << __FILE__ << __LINE__ << ": findFuncVectorByPretty found " << pdfv->size()
	       <<" functions called "<< SIGNAL_HANDLER << endl;
	}
	signal_handler = (*pdfv)[0];
      }
      
      // search any shared libraries for signal handler function
      if(!signal_handler && dynamiclinking && shared_objects) { 
	for(u_int j=0;(j < shared_objects->size()) && !signal_handler; j++){
	  if (NULL != (pdfv = (*shared_objects)[j]->findFuncVectorByPretty(SIGNAL_HANDLER)) && pdfv->size()) {
	    if (pdfv->size() > 1) {
	      cerr << __FILE__ << __LINE__ << ": findFuncVectorByPretty found " << pdfv->size()
		   <<" functions called "<< SIGNAL_HANDLER << endl;
	    }
	    signal_handler = (*pdfv)[0];
	  }
	} 
      }
 
      signal_cerr << "process::findSignalHandler <" << SIGNAL_HANDLER << ">";
      if (!signal_handler) signal_cerr << " NOT";
      signal_cerr << " found." << endl;
    }
}
#endif


bool process::findInternalSymbol(const pdstring &name, bool warn,
                                 internalSym &ret_sym) const
{
     // On some platforms, internal symbols may be dynamic linked
     // so we search both the a.out and the shared objects
     Symbol sym;
     Address baseAddr;
     static const pdstring underscore = "_";

     if (getSymbolInfo(name, sym, baseAddr)
         || getSymbolInfo(underscore+name, sym, baseAddr)) {
#if defined(mips_unknown_ce2_11)
        ret_sym = internalSym(sym.addr(), name);
#else
        ret_sym = internalSym(baseAddr+sym.addr(), name);
#endif
        return true;
     }

     if (warn) {
        pdstring msg;
        msg = pdstring("Unable to find symbol: ") + name;
        statusLine(msg.c_str());
        showErrorCallback(28, msg);
     }
     return false;
}

Address process::findInternalAddress(const pdstring &name, bool warn, bool &err) const {
     // On some platforms, internal symbols may be dynamic linked
     // so we search both the a.out and the shared objects
     Symbol sym;
     Address baseAddr;
     static const pdstring underscore = "_";
#if !defined(i386_unknown_linux2_0) \
 && !defined(ia64_unknown_linux2_4) \
 && !defined(alpha_dec_osf4_0) \
 && !defined(i386_unknown_nt4_0) \
 && !defined(mips_unknown_ce2_11) //ccw 20 july 2000
     // we use "dlopen" because we took out the leading "_"'s from the name
     if (name==pdstring("dlopen")) {
       // if the function is dlopen, we use the address in ld.so.1 directly
       baseAddr = dyn->get_dlopen_addr();
       if (baseAddr != (Address)NULL) {
         err = false;
         return baseAddr;
       } else {
         err = true;
         return 0;
       }
       // replace above with tighter code below
       //err = (baseAddr != 0);
       //return baseAddr;
     }
#endif

     if (getSymbolInfo(name, sym, baseAddr)
         || getSymbolInfo(underscore+name, sym, baseAddr)) {
        err = false;
#ifdef mips_unknown_ce2_11 //ccw 29 mar 2001
        return sym.addr();//+baseAddr; ///ccw 28 oct 2000 THIS ADDS TOO MUCH TO THE LIBDYNINSTAPI.DLL!
#else
        return sym.addr()+baseAddr;
#endif
     }

     if (warn) {
        pdstring msg;
        msg = pdstring("Unable to find symbol: ") + name;
        statusLine(msg.c_str());
        showErrorCallback(28, msg);
     }
     err = true;
     return 0;
}

bool process::continueProc(int signalToContinueWith) {
    if (!isAttached()) {
        bpwarn( "Warning: continue attempted on non-attached process\n");
        return false;
    }

    if (suppressEventConts())
    {
      return false;
    }

   if(IndependentLwpControl()) {
      pdvector<dyn_thread *>::iterator iter = threads.begin();
      while(iter != threads.end()) {
         dyn_thread *thr = *(iter);
         dyn_lwp *lwp = thr->get_lwp();
         
          if(lwp && lwp->status() != running)  {// the thread might not be scheduled
            lwp->continueLWP(signalToContinueWith);
	       }

         iter++;
      }
   } else {
      if (status_ == running)
	return true;
      bool res = getRepresentativeLWP()->continueLWP(signalToContinueWith);
      if (!res) {
         showErrorCallback(38, "System error: can't continue process");
         return false;
      }
   }
    
   status_ = running;
   return true;
}

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
        return false;
    }
    
    set_status(detached);

    // deleteProcess does the right thing depending on the status vrble
    deleteProcess();
    return true;
}

    

// Note: this may happen when the process is already gone. 
bool process::detach(const bool leaveRunning ) {

#if !defined(i386_unknown_linux2_0) && !defined(ia64_unknown_linux2_4)
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

#if defined(i386_unknown_linux2_0) || defined(ia64_unknown_linux2_4)
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
bool process::handleSyscallExit(procSignalWhat_t syscall,
                                dyn_lwp *lwp_with_event)
#else
bool process::handleSyscallExit(procSignalWhat_t, dyn_lwp *lwp_with_event)
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
#if !defined(rs6000_ibm_aix4_1)  || defined(AIX_PROC)  // non AIX-PTRACE
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

void process::triggerNormalExitCallback(int exitCode) {
   // special case where can't wait to continue process
   if (status() == exited) {
      return;
   }

   BPatch::bpatch->registerNormalExit(bpatch_thread, exitCode);
   
   // And continue the process so that it exits normally
   continueProc();
}

void process::triggerSignalExitCallback(int signalnum) {
   // special case where can't wait to continue process
   if (status() == exited) {
      return;
   }

   BPatch::bpatch->registerSignalExit(bpatch_thread, signalnum);
}

void process::handleProcessExit() {

   // special case where can't wait to continue process
   if (status() == exited) {
      return;
   }

   --activeProcesses;

   // Set exited first, so detach doesn't try impossible things
   set_status(exited);
   detach(false);

  // Perhaps these lines can be un-commented out in the future, but since
  // cleanUpAndExit() does the same thing, and it always gets called
  // (when paradynd detects that paradyn died), it's not really necessary
  // here.  -ari
//  for (unsigned lcv=0; lcv < processVec.size(); lcv++)
//     if (processVec[lcv] == proc) {
//        delete proc; // destructor removes shm segments...
//      processVec[lcv] = NULL;
//     }
}
        

/*
 * handleForkEntry: do anything necessary when a fork is entered
 */

void process::handleForkEntry() {

    // On some platforms we can't tell the fork exit trap, so we 
    // set a flag that is detected
    nextTrapIsFork = true;
    
    // Make bpatch callbacks as well
    BPatch::bpatch->registerForkingThread(getPid(), NULL);
}

void process::handleForkExit(process *child) {
    nextTrapIsFork = false;

#if !defined(BPATCH_LIBRARY)
    // shared memory will probably become part of dyninst soon
    child->init_shared_memory(this);
#endif

#if defined(os_aix)
    // AIX doesn't copy memory past the ends of text segments, so we
    // do it manually here
    copyDanglingMemory(child);
#endif
    
#if !defined(BPATCH_LIBRARY)
    if(pdFlavor == "mpi")
       child->detachProcess(true);
    else
#endif
       BPatch::bpatch->registerForkedThread(getPid(), child->getPid(), child);
}

void process::handleExecEntry(char *arg0) {
    nextTrapIsExec = true;
    execPathArg = "";
    // The arg0 is an address in the mutatee's space
    char temp[512];
    if (!readDataSpace(arg0, 512, temp, false))
        cerr << "Failed to read exec argument!" << endl;
    else
        execPathArg = temp;
}

/* process::handleExecExit: called when a process successfully exec's.
   Parse the new image, disable metric instances on the old image, create a
   new (and blank) shm segment.  The process hasn't yet bootstrapped, so we
   mustn't try to enable anything...
*/
void process::handleExecExit() {
    // NOTE: for shm sampling, the shm segment has been removed, so we
    //       mustn't try to disable any dataReqNodes in the standard way...
    nextTrapIsExec = false;
#if !defined(BPATCH_LIBRARY) //ccw 22 apr 2002 : SPLIT
	PARADYNhasBootstrapped = false;
#endif
   // all instrumentation that was inserted in this process is gone.
   // set exited here so that the disables won't try to write to process
   set_status(execing);

   deleteProcess();

   ///////////////////////////// CONSTRUCTION STAGE ///////////////////////////
   dyn = new dynamic_linking(this);

   codeRangesByAddr_ = new codeRangeTree;
   
#ifdef SHM_SAMPLING
   shmMetaData = 
      new sharedMetaData(*theSharedMemMgr, MAX_NUMBER_OF_THREADS); 
#endif
   
   int status = pid;

   // Before we parse the file descriptor, re-open the /proc/pid/as file handle
#if defined(rs6000_ibm_aix4_1) && defined(AIX_PROC)
   getRepresentativeLWP()->reopen_fds();
#endif

   fileDescriptor *desc = getExecFileDescriptor(execFilePath,
                                                status,
                                                false);
   if (!desc) return;

   // Clear out any previous version of this path
   image::removeImage(desc);
   image *img = image::parseImage(desc);
   
   if (!img) {
       // For better error reporting, two failure return values would be useful
       // One for simple error like because-file-not-found
       // Another for serious errors like found-but-parsing-failed (internal error;
       //    please report to paradyn@cs.wisc.edu)

       pdstring msg = pdstring("Unable to parse image: ") + execFilePath;
       showErrorCallback(68, msg.c_str());
       OS::osKill(pid);
          // err..what if we had attached?  Wouldn't a detach be appropriate in this case?
       return;
    }

   img->defineModules(this);

    // delete proc->symbols ???  No, the image can be shared by more
    // than one process...images and instPoints can not be deleted...
   if (!symbols->destroy())
     image::removeImage(symbols);
   
    symbols = img;
    addCodeRange(symbols->codeOffset(), symbols);

    // see if new image contains the signal handler function
    this->findSignalHandler();

	// release our info about the heaps that exist in the process
	// (we will eventually call initInferiorHeap to rebuild the list and
	// reset our notion of which are available and which are in use)
	unsigned int heapIdx;
	for( heapIdx = 0; heapIdx < heap.bufferPool.size(); heapIdx++ )
	{
		delete heap.bufferPool[heapIdx];
	}
	heap.bufferPool.resize(0);

    // initInferiorHeap can only be called after symbols is set!
#if defined(i386_unknown_nt4_0) || (defined mips_unknown_ce2_11) //ccw 20 july 2000 : 29 mar 2001
    initInferiorHeap();
#endif

    bootstrapState = unstarted;
    createdViaFork = false;
    createdViaAttach = false;
    // This doesn't exist, but would be appropriate
    // createdViaExec = true;
    createdViaAttachToCreated = true;
    

    /* update process status */

    set_status(stopped); // was 'exited'

   // TODO: We should remove (code) items from the where axis, if the exec'd process
   // was the only one who had them.

   // the exec'd process has the same fd's as the pre-exec, so we don't need
   // to re-initialize traceLink (is this right???)

   // we don't need to re-attach after an exec (is this right???)
 
#ifdef SHM_SAMPLING
   theSharedMemMgr->handleExec();
      // reuses the shm seg (paradynd's already attached to it); resets applic-attached-
      // at to NULL.  Quite similar to the (non-fork) ctor, really.

   //theVariableMgr->handleExec();
#endif

   inExec = false;
   execed_ = true;

   // We don't make the exec callback here since this can be called
   // more than once if we get multiple exec "exits", plus the proces
   // isn't Dyninst-ready. We make the callback once the RT library is
   // loaded
}

bool process::checkTrappedSyscallsInternal(Address syscall)
{
    for (unsigned i = 0; i < syscallTraps_.size(); i++) {
        if (syscall == syscallTraps_[i]->syscall_id)
            return true;
    }
    
    return false;
}


/*
If you want to check that ignored traps associated with irpcs are getting
regenerated, you can use this code in the metric::enableDataCollection
functions, after the process has been continued.

    if(procsToContinue.size()>0) {
      sleep(1);
      procsToContinue[0]->CheckAppTrapIRPCInfo();
    }
*/

#ifdef INFERIOR_RPC_DEBUG
void process::CheckAppTrapIRPCInfo() {
   cout << "CheckAppTrapIRPCInfo() - Entering\n";
   int trapNotHandled;
   bool err = false;
   Address addr = 0;
   addr = findInternalAddress("trapNotHandled",true, err);
   assert(err==false);
   if (!readDataSpace((caddr_t)addr, sizeof(int), &trapNotHandled, true))
     return;  // readDataSpace has it's own error reporting

   if(trapNotHandled)
     cerr << "!!! Error, previously ignored trap not regenerated (ABE)!!!\n"; 
   else
     cerr << "CheckAppTrapIRPCInfo() - trap got handled correctly (ABE).\n";
   cout << "CheckAppTrapIRPCInfo() - Leaving\n";
}
#endif    

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

void process::installInstrRequests(const pdvector<instMapping*> &requests) {
    for (unsigned lcv=0; lcv < requests.size(); lcv++) {
        instMapping *req = requests[lcv];
        
        if(!multithread_capable() && req->is_MTonly())
            continue;

        pdstring func_name;
        pdstring lib_name;
        pdvector<function_base *> matchingFuncs;
        
        getLibAndFunc(req->func, lib_name, func_name);

        if ((lib_name != "*") && (lib_name != "")) {
            function_base *func2 = static_cast<function_base *>(findOnlyOneFunction(req->func));
            if(func2 != NULL)
                matchingFuncs.push_back(func2);
            //else
            //cerr << "couldn't find initial function " << req->func << "\n";
        }
        else {
            // Wildcard: grab all functions matching this name
            findAllFuncsByName(func_name, matchingFuncs);
        }
        for (unsigned funcIter = 0; funcIter < matchingFuncs.size(); funcIter++) {
         function_base *func = matchingFuncs[funcIter];
         
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
            const pdvector<instPoint*> func_rets = func->funcExits(this);
            for (unsigned j=0; j < func_rets.size(); j++) {
               instPoint *func_ret = const_cast<instPoint *>(func_rets[j]);
               miniTrampHandle *mtHandle;
               // We ignore the mtHandle return, which is okay -- it's
               // also stored with the base tramp. 
               loadMiniTramp_result opResult = addInstFunc(this,
                                                           mtHandle,
                                                           func_ret, ast,
                                                           req->when, req->order, false, 
                                                           (!req->useTrampGuard));
               assert( opResult == success_res );
               req->mtHandles.push_back(mtHandle);
            }
	  
         }
         
         if (req->where & FUNC_ENTRY) {
            instPoint *func_entry = const_cast<instPoint *>(func->funcEntry(this));
            miniTrampHandle *mtHandle;
            loadMiniTramp_result opResult = addInstFunc(this, mtHandle,
                                                        func_entry, ast,
                                                        req->when, req->order, false,
                                                        (!req->useTrampGuard));
            assert( opResult == success_res );
            req->mtHandles.push_back(mtHandle);
         }
         
         if (req->where & FUNC_CALL) {
            pdvector<instPoint*> func_calls = func->funcCalls(this);
            if (func_calls.size() == 0)
               continue;
            
            for (unsigned j=0; j < func_calls.size(); j++) {
               miniTrampHandle *mtHandle;
               loadMiniTramp_result opResult = addInstFunc(this, mtHandle,
                                                func_calls[j], ast,
                                                req->when, req->order, false, 
                                                (!req->useTrampGuard));
               assert( opResult == success_res);
               req->mtHandles.push_back(mtHandle);
            }
         }
         
         removeAst(ast);
        }
    }
}


bool process::extractBootstrapStruct(DYNINST_bootstrapStruct *bs_record)
{
  const pdstring vrbleName = "DYNINST_bootstrap_info";
  internalSym sym;
  bool flag = findInternalSymbol(vrbleName, true, sym);
  assert(flag);
  Address symAddr = sym.getAddr();

  // bulk read of bootstrap structure
  if (!readDataSpace((const void*)symAddr, sizeof(*bs_record), bs_record, true)) {
    cerr << "extractBootstrapStruct failed because readDataSpace failed" << endl;
    return false;
  }
  return true;
}

void process::getObservedCostAddr() {

#if !defined(SHM_SAMPLING) || 1 //ccw 19 apr 2002 : SPLIT
    bool err;
    costAddr_ = findInternalAddress("DYNINSTobsCostLow", true, err);
    if (err) {
        sprintf(errorLine,"Internal error: unable to find addr of DYNINSTobsCostLow\n");
        logLine(errorLine);
        showErrorCallback(79,errorLine);
        P_abort();
    }
#else
    costAddr_ = (Address)getObsCostLowAddrInApplicSpace();
#endif
}

bool process::isAttached() const {
    if (status_ == exited ||
        status_ == detached ||
        status_ == execing)
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
    return (status_ == exited);
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

#ifdef DEBUG
pdstring process::getBootstrapStateAsString() const {
   // useful for debugging
   switch(bootstrapState) {
     case unstarted:
        return "unstarted";
     case begun:
        return "begun";
     case initialized:
        return "initialized";
     case loadingRT:
        return "loadingRT";
     case loadedRT:
        return "loadedRT";
     case bootstrapped:
        return "bootstrapped";
   }
   assert(false);
   return "???";
}
#endif

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

#ifdef BPATCH_SET_MUTATIONS_ACTIVE
bool process::saveOriginalInstructions(Address addr, int size) {
    char *data = new char[size];
    assert(data);

    if (!readTextSpace((const void *)addr, size, data))
        return false;

    beforeMutationList.insertHead(addr, size, data);

    delete[] data;
    
    return true;
}

bool process::writeMutationList(mutationList &list) {
   bool needToCont = false;

   if (!isAttached())
      return false;

   dyn_lwp *stopped_lwp = query_for_stopped_lwp();
   if(stopped_lwp == NULL) {
      stopped_lwp = stop_an_lwp(&needToCont);
      if(stopped_lwp == NULL) {
         pdstring msg =
            pdstring("System error: unable to write mutation list "
                     ": couldn't stop an lwp\n");
         showErrorCallback(38, msg);
         return false;
      }
   }

   mutationRecord *mr = list.getHead();

   while (mr != NULL) {
      bool res = stopped_lwp->writeTextSpace((void *)mr->addr, mr->size,
                                             mr->data);
      if (!res) {
         // XXX Should we do something special when an error occurs, since
         //     it could leave the process with only some mutations
         //     installed?
         pdstring msg =
            pdstring("System error: unable to write to process text space (WML): ")
            + pdstring(strerror(errno));
         showErrorCallback(38, msg); // XXX Own error number?
         return false;
      }
      mr = mr->next;
   }

   if(needToCont) {
      return stopped_lwp->continueLWP();
   }
   return true;
}

bool process::uninstallMutations() {
    return writeMutationList(beforeMutationList);
}

bool process::reinstallMutations() {
    return writeMutationList(afterMutationList);
}

mutationRecord::mutationRecord(Address _addr, int _size, const void *_data) {
    prev = NULL;
    next = NULL;
    addr = _addr;
    size = _size;
    data = new char[size];
    assert(data);
    memcpy(data, _data, size);
}

mutationRecord::~mutationRecord()
{
    // allocate it as a char[], might as well delete it as a char[]...
    delete [] static_cast<char *>(data);
}

mutationList::~mutationList() {
    mutationRecord *p = head;

    while (p != NULL) {
        mutationRecord *n = p->next;
        delete p;
        p = n;
    }
}

void mutationList::insertHead(Address addr, int size, const void *data) {
    mutationRecord *n = new mutationRecord(addr, size, data);
    
    assert((head == NULL && tail == NULL) || (head != NULL && tail != NULL));

    n->next = head;
    if (head == NULL)
        tail = n;
    else
        head->prev = n;
    head = n;
}

void mutationList::insertTail(Address addr, int size, const void *data) {
    mutationRecord *n = new mutationRecord(addr, size, data);
    
    assert((head == NULL && tail == NULL) || (head != NULL && tail != NULL));

    n->prev = tail;
    if (tail == NULL)
        head = n;
    else
        tail->next = n;
    tail = n;
}
#endif /* BPATCH_SET_MUTATIONS_ACTIVE */


BPatch_point *process::findOrCreateBPPoint(BPatch_function *bpfunc,
					   instPoint *ip,
					   BPatch_procedureLocation pointType)
{
   Address addr = ip->pointAddr();

   if (ip->getOwner() != NULL) {
      Address baseAddr;
      if (getBaseAddress(ip->getOwner(), baseAddr)) {
         addr += baseAddr;
      }
   }

   if (instPointMap.defines(addr)) {
      return instPointMap[addr];
   } else {
      if (bpfunc == NULL) {
         const pd_Function *fc =
            dynamic_cast<const pd_Function *>(ip->pointFunc());
         pd_Function *f = const_cast<pd_Function *>(fc);
         const BPatch_function *ptr =
            dynamic_cast<const BPatch_function *>(findOrCreateBPFunc(f));
         bpfunc = const_cast<BPatch_function *>(ptr);
      }

      BPatch_point *pt = new BPatch_point(this, bpfunc, ip, pointType);
      instPointMap[addr] = pt;
      return pt;
   }
}

BPatch_function *process::findOrCreateBPFunc(pd_Function* pdfunc,
					     BPatch_module *bpmod)
{
   if (PDFuncToBPFuncMap.defines(pdfunc))
      return PDFuncToBPFuncMap[pdfunc];

   assert(bpatch_thread);

   // Find the module that contains the function
   if (bpmod == NULL && pdfunc->file() != NULL) {
      BPatch_Vector<BPatch_module *> &mods =
         *(bpatch_thread->getImage()->getModules());
      for (unsigned int i = 0; i < mods.size(); i++) {
         if (mods[i]->mod == pdfunc->file()) {
            bpmod = mods[i];
            break;
         }
      }
      // The BPatch_function may have been created as a side effect
      // of the above
      if (PDFuncToBPFuncMap.defines(pdfunc))
         return PDFuncToBPFuncMap[pdfunc];
   }
   
   BPatch_function *ret = new BPatch_function(this, pdfunc, bpmod);
   
   return ret;
}

// Add it at the bottom...
void process::deleteMiniTramp(miniTrampHandle *delInst)
{
  // Add to the list and deal with it later.
  // The question is then, when to GC. I'd suggest
  // when we try to allocate memory, and leave
  // it a public member that can be called when
  // necessary
  struct instPendingDeletion *toBeDeleted = new instPendingDeletion();
  toBeDeleted->oldMini = delInst;
  toBeDeleted->oldBase = NULL;
  pendingGCInstrumentation.push_back(toBeDeleted);
}

bool process::checkIfMiniTrampAlreadyDeleted(miniTrampHandle *delInst)
{
  for (unsigned i = 0; i < pendingGCInstrumentation.size(); i++)
    if (pendingGCInstrumentation[i]->oldMini == delInst)
      return true;
  return false;
}

void process::deleteBaseTramp(trampTemplate *baseTramp)
{
  struct instPendingDeletion *toBeDeleted = new instPendingDeletion();
  toBeDeleted->oldMini = NULL;
  toBeDeleted->oldBase = baseTramp;
  pendingGCInstrumentation.push_back(toBeDeleted);
}

// Function relocation requires a version of process::convertPCsToFuncs 
// in which null functions are not passed into ret. - Itai 
pdvector<pd_Function *> process::pcsToFuncs(pdvector<Frame> stackWalk) {
    pdvector <pd_Function *> ret;
    unsigned i;
    pd_Function *fn;
    for(i=0;i<stackWalk.size();i++) {
        fn = (pd_Function *)findFuncByAddr(stackWalk[i].getPC());
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

  inferiorHeap *hp = &heap;

  if (pendingGCInstrumentation.size() == 0) return;

  for (unsigned deletedIter = 0; 
       deletedIter < pendingGCInstrumentation.size(); 
       deletedIter++) {
    instPendingDeletion *deletedInst = pendingGCInstrumentation[deletedIter];
    bool safeToDelete = true;

    for (unsigned threadIter = 0;
         threadIter < stackWalks.size(); 
         threadIter++) {
        pdvector<Frame> stackWalk = stackWalks[threadIter];
        for (unsigned walkIter = 0;
             walkIter < stackWalk.size();
             walkIter++) {
            Frame frame = stackWalk[walkIter];
            codeRange *range = findCodeRangeByAddress(frame.getPC());
            trampTemplate *basetramp_ptr = range->is_basetramp();
            miniTrampHandle *minitramp_ptr = range->is_minitramp();
                
            if (!range) {
                // Odd... couldn't find a match at this PC
                // Do we want to skip GCing in this case? Problem
                // is, we often see garbage at the end of stack walks.
                continue;
            }
            if (deletedInst->oldBase) {
                // If we're in the base tramp we can't delete
                if (basetramp_ptr == deletedInst->oldBase)
                    safeToDelete = false;
                // If we're in a child minitramp, we also can't delete
                miniTrampHandle *mt = range->is_minitramp();
                if (mt && (mt->baseTramp == deletedInst->oldBase))
                    safeToDelete = false;
            }
            else {
                assert(deletedInst->oldMini);
                if (minitramp_ptr == deletedInst->oldMini)
                    safeToDelete = false;
            }
            // If we can't delete, don't bother to continue checking
            if (!safeToDelete)
                break;
        }
        // Same as above... pop out.
        if (!safeToDelete)
            break;
    }
    if (safeToDelete) {
        heapItem *ptr = NULL;
        Address baseAddr;
        if (deletedInst->oldBase)
            baseAddr = deletedInst->oldBase->baseAddr;
        else
            baseAddr = deletedInst->oldMini->miniTrampBase;
        if (!hp->heapActive.find(baseAddr, ptr)) {
            sprintf(errorLine,"Warning: attempt to free undefined heap entry 0x%p (pid=%d, heapActive.size()=%d)\n", 
                    (void*)baseAddr, getPid(), 
                    hp->heapActive.size());
            logLine(errorLine);
            // Skip to next item on the list
            continue;
        }
        inferiorFree(baseAddr);

        // Delete from list of GCs
        // Vector deletion is slow... so copy the last item in the list to
        // the current position. We could also set this one to NULL, but that
        // means the GC vector could get very, very large.
        pendingGCInstrumentation[deletedIter] = 
          pendingGCInstrumentation[pendingGCInstrumentation.size()-1];
        // Lop off the last one
        pendingGCInstrumentation.resize(pendingGCInstrumentation.size()-1);
        // Back up iterator to cover the fresh one
        deletedIter--;
        
        // Delete from the codeRange tree
        deleteCodeRange(baseAddr);
        
        if (deletedInst->oldMini)
            delete deletedInst->oldMini;
        if (deletedInst->oldBase)
            delete deletedInst->oldBase;
        delete deletedInst;
    }
  }
}

dyn_thread *process::createInitialThread() {
   assert(threads.size() == 0);
   dyn_thread *initialThread = new dyn_thread(this);
   threads.push_back(initialThread);
   return initialThread;
}


dyn_thread *process::getThread(unsigned tid) {
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

  foundLWP = createRealLWP(lwp_id, lwp_id);

  if (!foundLWP->attach()) {
     deleteLWP(foundLWP);
     return NULL;
  }
  return foundLWP;
}

dyn_lwp *process::lookupLWP(unsigned lwp_id) {
   dyn_lwp *foundLWP = NULL;
   bool found = real_lwps.find(lwp_id, foundLWP);
   if(! found) {
      if(lwp_id == getRepresentativeLWP()->get_lwp_id())
         foundLWP = getRepresentativeLWP();
   }
   return foundLWP;
}

// fictional lwps aren't saved in the real_lwps vector
dyn_lwp *process::createFictionalLWP(unsigned lwp_id) {
   dyn_lwp *lwp = new dyn_lwp(lwp_id, this);
   theRpcMgr->addLWP(lwp);
   return lwp;
}

dyn_lwp *process::createRealLWP(unsigned lwp_id, int /*lwp_index*/) {
   dyn_lwp *lwp = new dyn_lwp(lwp_id, this);
   real_lwps[lwp_id] = lwp;
   theRpcMgr->addLWP(lwp);
   return lwp;
}

void process::deleteLWP(dyn_lwp *lwp_to_delete) {
   if(real_lwps.size() > 0 && lwp_to_delete!=NULL) {
      theRpcMgr->deleteLWP(lwp_to_delete);
      unsigned index = lwp_to_delete->get_lwp_id();
      if (index)
          real_lwps.undef(index);
   }
   delete lwp_to_delete;
}

void read_variables_after_fork(process *proc, int *shm_key,
                               void **DYNINST_shmSegAttachedPtr) {
   bool err = false;
   Address addr = proc->findInternalAddress("DYNINST_shmSegKey", true, err);
   assert(err==false);
   if (!proc->readDataSpace((caddr_t)addr, sizeof(int), shm_key, true))
      return;  // readDataSpace has it's own error reporting

   addr = proc->findInternalAddress("DYNINST_shmSegAttachedPtr", true, err);
   assert(err==false);
   if (!proc->readDataSpace((caddr_t)addr, sizeof(int),
                            DYNINST_shmSegAttachedPtr, true))
      return;  // readDataSpace has it's own error reporting
}

void call_PARADYN_init_child_after_fork(process * /*theProc*/, 
                                        unsigned /*rpc_id*/,
                                        void * /*userData*/,
                                        void *returnVal) {
   //cerr << "called call_PARADYN_init callback, returnVal: " << returnVal 
   //     << ", pid: " << theProc->getPid() << ", rpc_id: " << rpc_id << endl;
   if(reinterpret_cast<long>(returnVal) != 123)
      cerr << "WARNING, PARADYN_init_child_after_fork unsuccessful\n";
}

#if !defined(BPATCH_LIBRARY)
void process::init_shared_memory(process *parentProc) {
   pdvector<AstNode *> ast_args;
   AstNode *ast = new AstNode("PARADYN_init_child_after_fork", ast_args);

   // Use the lwp that is actually stopped at the fork exit if can. Seems to
   // get around restoreRegister - device busy problem.
   const int UNSET_LWP_STOPPED_FROM_EXIT_VAL = 0;
   int lwp_stopped_from_exit = parentProc->getLWPStoppedFromForkExit();
   dyn_lwp *lwp_to_use;
   if(lwp_stopped_from_exit == UNSET_LWP_STOPPED_FROM_EXIT_VAL)
      lwp_to_use = getRepresentativeLWP();
   else
      lwp_to_use = lookupLWP(lwp_stopped_from_exit);

   unsigned rpc_id =
      getRpcMgr()->postRPCtoDo(ast, false, call_PARADYN_init_child_after_fork,
                               this, true, NULL, lwp_to_use);
   
   bool wasRunning = false;  // child should be paused after fork

   do {
       getRpcMgr()->launchRPCs(wasRunning);
       if(hasExited()) return;
       getSH()->checkForAndHandleProcessEvents(false);
       // loop until the rpc has completed
   } while(getRpcMgr()->getRPCState(rpc_id) != irpcNotValid);

   int shm_key;
   void *shmSegAttachedPtr;
   read_variables_after_fork(this, &shm_key, &shmSegAttachedPtr);

   // since the child process inherits the parents instrumentation we'll
   // need to inherit the parent process's data also
   theSharedMemMgr = 
      new shmMgr(*parentProc->theSharedMemMgr, shm_key, shmSegAttachedPtr,
                 this);
   shmMetaData = new sharedMetaData(*(parentProc->shmMetaData), 
                                    *theSharedMemMgr);
   shMetaOffsetData = 
      new sharedMetaOffsetData(*theSharedMemMgr, 
                               *(parentProc->shMetaOffsetData));

   shmMetaData->adjustToNewBaseAddr(reinterpret_cast<Address>(
                         (theSharedMemMgr->getBaseAddrInDaemon())));
   shmMetaData->initializeForkedProc(theSharedMemMgr->cookie, getPid());
}
#endif

// MT section (move to processMT.C?)
#if !defined(BPATCH_LIBRARY)
// Called for new threads

dyn_thread *process::createThread(
  int tid, 
  unsigned pos, 
  unsigned stackbase, 
  unsigned startpc, 
  void* resumestate_p,  
  bool /*bySelf*/)
{
  dyn_thread *thr;
  //bperr( "Received notice of new thread.... tid %d, pos %d, stackbase 0x%x, startpc 0x%x\n", tid, pos, stackbase, startpc);
  // creating new thread
  thr = new dyn_thread(this, tid, pos, NULL);
  threads += thr;

  thr->updateLWP();

  thr->update_resumestate_p(resumestate_p);
  function_base *pdf ;

  if (startpc) {
    thr->update_stack_addr(stackbase) ;
    thr->update_start_pc(startpc) ;
    codeRange *range = findCodeRangeByAddress(startpc);
    pdf = range->is_pd_Function();
    thr->update_start_func(pdf) ;
  } else {
    pdf = findOnlyOneFunction("main");
    assert(pdf);
    //thr->update_start_pc(pdf->addr()) ;
    thr->update_start_pc(0);
    thr->update_start_func(pdf);
    thr->update_stack_addr(stackbase);
  }

  //sprintf(errorLine,"+++++ creating new thread{%s/0x%x}, pos=%u, tid=%d, stack=0x%x, resumestate=0x%x, by[%s]\n",
  //pdf->prettyName().c_str(), startpc, pos,tid,stackbase,(unsigned)resumestate_p, bySelf?"Self":"Parent");
//logLine(errorLine);
  return(thr);
}

//
// CALLED for mainThread
//
void process::updateThread(dyn_thread *thr, int tid, 
			   unsigned index, void* resumestate_p)
{
  assert(thr);
  thr->update_tid(tid);
  thr->update_index(index);
  thr->update_resumestate_p(resumestate_p);
  function_base *f_main = findOnlyOneFunction("main");
  assert(f_main);

  //unsigned addr = f_main->addr();
  //thr->update_start_pc(addr) ;
  thr->update_start_pc(0) ;
  thr->update_start_func(f_main) ;

  thr->updateLWP();

  //sprintf(errorLine,"+++++ updateThread--> creating new thread{main}, index=%u, tid=%d, resumestate=0x%x\n", index,tid, (unsigned) resumestate_p);
  //logLine(errorLine);
}

//
// CALLED from Attach
//
void process::updateThread(
  dyn_thread *thr, 
  int tid, 
  unsigned index, 
  unsigned stackbase, 
  unsigned startpc, 
  void* resumestate_p) 
{
  assert(thr);
  //  
  sprintf(errorLine," updateThread(tid=%d, index=%d, stackaddr=0x%x, startpc=0x%x)\n",
	 tid, index, stackbase, startpc);
  logLine(errorLine);

  thr->update_tid(tid);
  thr->update_index(index);
  thr->update_resumestate_p(resumestate_p);

  function_base *pdf;

  if(startpc) {
    thr->update_start_pc(startpc) ;
    codeRange *range = findCodeRangeByAddress(startpc);
    pdf = range->is_pd_Function();
    thr->update_start_func(pdf) ;
    thr->update_stack_addr(stackbase) ;
  } else {
    pdf = findOnlyOneFunction("main");
    assert(pdf);
    thr->update_start_pc(startpc) ;
    //thr->update_start_pc(pdf->addr()) ;
    thr->update_start_func(pdf);
    thr->update_stack_addr(stackbase);
  } //else

  //sprintf(errorLine,"+++++ creating new thread{%s/0x%x}, index=%u, tid=%d, stack=0x%xs, resumestate=0x%x\n",
  //pdf->prettyName().c_str(), startpc, index, tid, stackbase, (unsigned) resumestate_p);
  //logLine(errorLine);
}

void process::deleteThread(int tid)
{
   pdvector<dyn_thread *>::iterator iter = threads.end();
   while(iter != threads.begin()) {
      dyn_thread *thr = *(--iter);
      if(thr->get_tid() != (unsigned) tid)  continue;

      // ===  Found It  ==========================
      // Set the INDEX to "reusable"
      // Note: we don't acquire a lock. This is okay, because we're simply
      //       clearing the bit, which was not usable before now anyway.
      assert(shmMetaData->getIndexToThread(thr->get_index()) 
             == THREAD_AWAITING_DELETION);
      shmMetaData->setIndexToThread(thr->get_index(), 0);

      getRpcMgr()->deleteThread(thr);

      delete thr;    
      //sprintf(errorLine,"----- deleting thread, tid=%d, threads.size()=%d\n",
      //        tid, threads.size());
      //logLine(errorLine);

      threads.erase(iter);
   }
}

#endif /* MT*/

