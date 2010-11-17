/*
 * Copyright (c) 1996-2009 Barton P. Miller
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

#include <ctype.h>

#if defined(i386_unknown_solaris2_5)
#include <sys/procfs.h>
#endif

#include <set>
#include <string>

#include <stdio.h>

#include "common/h/headers.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/function.h"
#include "symtabAPI/h/Symtab.h"
//#include "dyninstAPI/src/func-reloc.h"
#include "dyninstAPI/src/baseTramp.h"
#include "dyninstAPI/src/miniTramp.h"
#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/dyn_thread.h"
#include "dyninstAPI/src/dyn_lwp.h"
#include "dyninstAPI/src/signalhandler.h"
#include "dyninstAPI/src/signalgenerator.h"
#include "dyninstAPI/src/mailbox.h"
#include "dyninstAPI/src/EventHandler.h"
#include "dyninstAPI/src/util.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/instP.h"
#include "dyninstAPI/src/instPoint.h"
#include "dyninstAPI/src/os.h"
#include "dyninstAPI/src/debug.h"
#include "dyninstAPI/src/dynamiclinking.h"
#include "dyninstAPI/src/BPatch_asyncEventHandler.h"
#include "dyninstAPI/src/debuggerinterface.h"
#include "dyninstAPI_RT/h/dyninstAPI_RT.h"
#include "dyninstAPI/src/ast.h"
#include "MemoryEmulator/memEmulator.h"
#include <boost/tuple/tuple.hpp>

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

#include "common/h/debugOstream.h"

#include "common/h/Timer.h"

#include "dyninstAPI_RT/h/dyninstAPI_RT.h"
using namespace Dyninst;

#define P_offsetof(s, m) (Address) &(((s *) NULL)->m)

#define FREE_WATERMARK (hp->totalFreeMemAvailable/2)
#define SIZE_WATERMARK 100 
static const timeLength MaxWaitingTime(10, timeUnit::sec());
static const timeLength MaxDeletingTime(2, timeUnit::sec());

unsigned activeProcesses; // number of active processes
pdvector<process*> processVec;

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

/* AIX method defined in aix.C; hijacked for IA-64's GP. */
#if !defined(arch_power)
Address process::getTOCoffsetInfo(Address /*dest */)
{
  Address tmp = 0;
  assert(0 && "getTOCoffsetInfo not implemented");
  return tmp; // this is to make the nt compiler happy! - naim
}
#else
Address process::getTOCoffsetInfo(Address dest)
{
   // Linux-power-32 bit: return 0 here, as it doesn't use the TOC.
   // Linux-power-64 does. Lovely. 
#if defined(arch_power) && defined(os_linux)
   if (getAddressWidth() == 4)
      return 0;
#endif

    // We have an address, and want to find the module the addr is
    // contained in. Given the probabilities, we (probably) want
    // the module dyninst_rt is contained in.
    // I think this is the right func to use
    
    // Find out which object we're in (by addr).
    codeRange *range = NULL;
    textRanges_.find(dest, range);
    if (!range)  // Try data?
        dataRanges_.find(dest, range);
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
    Address TOCOffset = mobj->parse_img()->getObject()->getTOCoffset(); 

    if (!TOCOffset)
       return 0;
    return TOCOffset + mobj->dataBase();

}

Address process::getTOCoffsetInfo(int_function *func) {

#if defined(arch_power) && defined(os_linux)
   // See comment above.
   if (getAddressWidth() == 4)
      return 0;
#endif

    mapped_object *mobj = func->obj();

    return mobj->parse_img()->getObject()->getTOCoffset() + mobj->dataBase();
}

#endif

#if defined(os_linux) && (defined(arch_x86) || defined(arch_x86_64))
extern void calcVSyscallFrame(process *p);
#endif

// Note: stack walks may terminate early. In this case, return what we can.
// Relies on the getCallerFrame method in the various <os>.C files
#if !defined(os_linux) 
Frame process::preStackWalkInit(Frame startFrame) {
    return startFrame;
}
#endif

bool process::walkStackFromFrame(Frame startFrame,
                                 pdvector<Frame> &stackWalk)
{
#if !defined( arch_x86) && !defined(arch_x86_64)
  Address fpOld   = 0;
#else
  Address spOld = 0;
#endif
  if (!isStopped())
      return false;

  mal_printf("Invoked a stackwalk %s[%d]\n",FILE__,__LINE__);

  Frame currentFrame = preStackWalkInit(startFrame);

  while (!currentFrame.isLastFrame()) {

#if !defined( arch_x86) && !defined(arch_x86_64)
    // Check that we are not moving up the stack.  Not relevant on x86,
    // since the frame pointer may be used for data.
    // successive frame pointers might be the same (e.g. leaf functions)
    if (fpOld > currentFrame.getFP())
        return false;
    fpOld = currentFrame.getFP();
#else
    if (spOld > currentFrame.getSP())
       return (stackWalk.size() != 0);
    spOld = currentFrame.getSP();
#endif

    stackWalk.push_back(currentFrame);
    currentFrame = currentFrame.getCallerFrame();
  }
  if (currentFrame.getProc() != NULL)
      stackWalk.push_back(currentFrame);

  return true;
}

FILE *debugfd = NULL;

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
	bool needToContinue = false;
	bool retval = true;
	
	if (!isStopped()) {
		needToContinue = true;
		pause();
	}
	
  	pdvector<Frame> stackWalk;
  	if (!threads.size()) { // Nothing defined in thread data structures
    	if (!getRepresentativeLWP()->walkStack(stackWalk)) {
			retval = false;
		}
    	else {
			// Use the walk from the default LWP
    		stackWalks.push_back(stackWalk);
		}
  	}
  	else { // Have threads defined
    	for (unsigned i = 0; i < threads.size(); i++) {
      		if (!threads[i]->walkStack(stackWalk)) {
				retval = false;
			}
      		else {
				stackWalks.push_back(stackWalk);
      			stackWalk.clear();
			}
    	}
  	}
	if (needToContinue)
		continueProc();
		
	return retval;
}

// Search an object for heapage

bool process::getInfHeapList(mapped_object *obj,
                             pdvector<heapDescriptor> &infHeaps)
{

    vector<pair<string,Address> > foundHeaps;

    obj->getInferiorHeaps(foundHeaps);

    for (u_int j = 0; j < foundHeaps.size(); j++)
    {
        // The string layout is: DYNINSTstaticHeap_size_type_unique
        // Can't allocate a variable-size array on NT, so malloc
        // that sucker
        char *temp_str = (char *)malloc(strlen(foundHeaps[j].first.c_str())+1);
        strcpy(temp_str, foundHeaps[j].first.c_str());
        char *garbage_str = strtok(temp_str, "_"); // Don't care about beginning
        assert(!strcmp("DYNINSTstaticHeap", garbage_str));
        // Name is as is.
        // If address is zero, then skip (error condition)
        if (foundHeaps[j].second == 0)
        {
            cerr << "Skipping heap " << foundHeaps[j].first.c_str()
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
        infHeaps.push_back(heapDescriptor(foundHeaps[j].first.c_str(),
                                          foundHeaps[j].second, 
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

#if defined (cap_save_the_world) 

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
#endif

#if defined (cap_save_the_world)
#if defined(sparc_sun_solaris2_4) \
 || defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */


char *process::saveWorldFindNewSharedLibraryName(string originalLibNameFullPath, char* dirName){
	const char *originalLibName = originalLibNameFullPath.c_str();
	unsigned int index=0;

	unsigned int nextIndex = 0;
	for(nextIndex = 0; nextIndex < originalLibNameFullPath.length() ;nextIndex++){
		if(originalLibName[nextIndex] == '/'){
			index = nextIndex +1;
		}
	}

	string oldLibName = originalLibNameFullPath.substr(index,originalLibNameFullPath.length());
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
			//fprintf(stderr," SAVING SHARED LIB: %s\n", sh_obj->fileName().c_str());
            //if the lib is only DirtyCalled dont save it! //ccw 24 jul 2003
            Address textAddr, textSize;
            char *newName = saveWorldFindNewSharedLibraryName(sh_obj->fileName(),directoryName);

		/* 	what i need to do:
			open the ORIGINAL shared lib --> sh_obj->fileName()
			read the text section out.
			reapply the instrumentation code
			save this new, instrumented text section back to the NEW DLDUMPED file in the _dyninstSaved# dir --> newName
		*/		 

            saveSharedLibrary *sharedObj =
               new saveSharedLibrary(sh_obj->getBaseAddress(),
                                     sh_obj->fullName().c_str(), newName);

            	sharedObj->openBothLibraries();
            
		sharedObj->getTextInfo(textAddr, textSize);
		char* textSection ;//= sharedObj->getTextSection(); /* get the text section from the ORIGINAL library */
		textSection = new char[textSize]; //ccw 14 dec 2005

		if(textSection){

			//applyMutationsToTextSection(textSection, textAddr, textSize);

			readDataSpace((void*)textAddr, textSize, (void*)textSection, true); //ccw 14 dec 2005
	          	sharedObj->saveMutations(textSection);
     		       	sharedObj->closeNewLibrary();
			delete [] textSection;
		}else{
			char msg[strlen(sh_obj->fileName().c_str())+100];
			sprintf(msg,"dumpPatchedImage: could not retreive .text section for %s\n",sh_obj->fileName().c_str());
       			BPatch_reportError(BPatchWarning,123,msg);
			sharedObj->closeNewLibrary();
		}
		//sharedObj->closeOriginalLibrary();
	  	delete sharedObj;
       		delete [] newName;
         }
         mutatedSharedObjectsSize += strlen(sh_obj->fileName().c_str()) +1 ;
         mutatedSharedObjectsSize += sizeof(int); //a flag to say if this is only DirtyCalled
      }
      //this is for the dlopen problem...
      if(strstr(sh_obj->fileName().c_str(), "ld-linux.so") ){
         //find the offset of _dl_debug_state in the .plt
	 Symtab *obj = sh_obj->parse_img()->getObject();
	 vector<relocationEntry> fbt;
	 obj->getFuncBindingTable(fbt);
	 dl_debug_statePltEntry = 0;
	 for(unsigned i=0; i<fbt.size();i++)
	 {
	 	if(fbt[i].name() == "_dl_debug_state")
			dl_debug_statePltEntry = fbt[i].rel_addr();
	 }
      }
#if defined(sparc_sun_solaris2_4)
      relocationEntry re;
      tmp_dlopen = 0;
      vector<relocationEntry> fbt;
      sh_obj->parse_img()->getObject()->getFuncBindingTable(fbt);
      for( unsigned int i = 0; i < fbt.size(); i++ )
      {
	if("dlopen" == fbt[i].name())
		tmp_dlopen =  fbt[i].rel_addr();
      }
      
      if( tmp_dlopen && (!sh_obj->isopenedWithdlopen()))
      {
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
   if(tmp_dlopen) {
       dl_debug_statePltEntry = tmp_dlopen;
   }
   
   //dl_debug_statePltEntry = parse_img()->getObject()->getPltSlot("dlopen");
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
                int_symbol info;
		std::string dynamicSection = "_DYNAMIC";
		sh_obj->getSymbolInfo(dynamicSection,info);
		baseAddr = sh_obj->getBaseAddress() + info->getAddr();
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
#endif

#if defined (cap_save_the_world)
#if defined(sparc_sun_solaris2_4) \
 || defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */ \
 || defined(rs6000_ibm_aix4_1)
void process::saveWorldCreateHighMemSections(
                        pdvector<imageUpdate*> &compactedHighmemUpdates, 
                        pdvector<imageUpdate*> &highmem_updates,
                        void *ptr) {

   Address guardFlagAddr= trampGuardBase();

   unsigned int pageSize = getpagesize();
   unsigned int startPage, stopPage;
   unsigned int numberUpdates=1;
   int startIndex, stopIndex;
   char *data;
   char name[50];
#if defined(sparc_sun_solaris2_4) \
 || defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */
	writeBackElf *newFile = (writeBackElf*) ptr;
#elif defined(rs6000_ibm_aix4_1)
	writeBackXCOFF *newFile = (writeBackXCOFF*) ptr;

#endif

#if 0
   unsigned int trampGuardValue;
   	bool err ;
//#if !defined(rs6000_ibm_aix4_1)

	/*fprintf(stderr,"guardFlagAddr %x\n",guardFlagAddr);*/
//   	readDataSpace((void*) guardFlagAddr, sizeof(unsigned int),
//                (void*) &trampGuardValue, true);
   

	for(int i=0;i< max_number_of_threads;i++){
		err = writeDataSpace((void*) &( ((int *)guardFlagAddr)[i]), sizeof(unsigned int),
                  (void*) &numberUpdates);
        	if (!err) fprintf(stderr, "%s[%d][%s]:  writeDataSpace failed\n", FILE__, __LINE__, getThreadStr(getExecThreadID()));
        		assert(err);

		saveWorldData( (Address) &( ((int *)guardFlagAddr)[i]),sizeof(unsigned int), &numberUpdates); //ccw 7 jul 2003
	}
#endif

      sprintf(name,"dyninstAPItrampguard");

	data = new char[sizeof(max_number_of_threads)*max_number_of_threads];
	memcpy(data, &max_number_of_threads,sizeof(max_number_of_threads));
	//fprintf(stderr,"WRITING: max_number_of_threads %d\n",max_number_of_threads);
	//ccw 14 dec 2005
#if defined(sparc_sun_solaris2_4) \
 || defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */
      newFile->addSection(guardFlagAddr,data,sizeof(max_number_of_threads),name,false);
#elif defined(rs6000_ibm_aix4_1)
	sprintf(name,"trampg");
	//fprintf(stderr," trampg 0x%x\n",guardFlagAddr);
	newFile->addSection(name,guardFlagAddr,guardFlagAddr,sizeof(unsigned int) *max_number_of_threads,data);
	//newFile->setDataEnd(guardFlagAddr+sizeof(unsigned int) *max_number_of_threads);
#endif
	delete []data;

		
#if  defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) \
 || defined(sparc_sun_solaris2_4)

#if defined(sparc_sun_solaris2_4)
	if( imageUpdates.size() == 0 ){
#endif
	//fprintf(stderr,"LOADING trampgu 0x%x\n",guardFlagAddr);
	data = new char[sizeof(unsigned int) * max_number_of_threads];
	memset(data,1,sizeof(unsigned int) * max_number_of_threads);		
	newFile->addSection(guardFlagAddr,data,sizeof(unsigned int) * max_number_of_threads,"dyninstAPI_1",false);
	delete []data;
#if defined(sparc_sun_solaris2_4)
	}
#endif
#endif

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
#if 0
   err = writeDataSpace((void*)guardFlagAddr, sizeof(unsigned int), 
                  (void*)&trampGuardValue);
   if (!err) fprintf(stderr, "%s[%d][%s]:  writeDataSpace failed\n", FILE__, __LINE__, getThreadStr(getExecThreadID()));
        assert(err);
#endif 
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
#endif

#if defined (cap_save_the_world)
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
		dataSize += loadLibraryUpdates[i].length() + 1 + sizeof(void *);
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
                void *tmp_brk = loadLibraryBRKs[j];
                memcpy( dataptr, &tmp_brk, sizeof(void *));
                dataptr += sizeof(void *);
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
#endif

/*
 * Given an image, add all static heaps inside it
 * (DYNINSTstaticHeap...) to the buffer pool.
 */

void process::addInferiorHeap(mapped_object *obj)
{
  pdvector<heapDescriptor> infHeaps;
  /* Get a list of inferior heaps in the new image */
  if (getInfHeapList(obj, infHeaps))
    {
      /* Add the vector to the inferior heap structure */
        for (u_int j=0; j < infHeaps.size(); j++)
        {
            infmalloc_printf("%s[%d]: adding heap at 0x%lx to 0x%lx, name %s\n",
                             FILE__, __LINE__,
                             infHeaps[j].addr(),
                             infHeaps[j].addr() + infHeaps[j].size(),
                             infHeaps[j].name().c_str());
#if defined(os_aix)
            // MT: I've seen problems writing into a "found" heap that
            // is in the application heap (IE a dlopen'ed
            // library). Since we don't have any problems getting
            // memory there, I'm skipping any heap that is in 0x2.....
            
            if ((infHeaps[j].addr() > 0x20000000) &&
                (infHeaps[j].addr() < 0xd0000000) &&
                (infHeaps[j].type() == uncopiedHeap)) {
                infmalloc_printf("... never mind, AIX skipped heap\n");
                continue;
            }
#endif            

            heapItem *h = new heapItem (infHeaps[j].addr(), infHeaps[j].size(),
                                        infHeaps[j].type(), false);

            infmalloc_printf("%s[%d]: Adding heap from 0x%lx - 0x%lx (%d bytes, type %d) from mapped object %s\n",
                             FILE__, __LINE__, 
                             infHeaps[j].addr(),
                             infHeaps[j].addr() + infHeaps[j].size(),
                             infHeaps[j].size(),
                             infHeaps[j].type(),
                             obj->fileName().c_str());
                             
            addHeap(h);

            // set rtlib heaps (runtime_lib hasn't been set yet)
            if ( ! obj->fullName().compare( dyninstRT_name ) ) {
                dyninstRT_heaps.push_back(h);
            }
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
    initializeHeap();

    // first initialization: add static heaps to pool
    for (unsigned i = 0; i < mapped_objects.size(); i++) {
        addInferiorHeap(mapped_objects[i]);
    }
}

bool process::initTrampGuard()
{
  // This is slightly funky. Dyninst does not currently support
  // multiple threads -- so it uses a single tramp guard flag, 
  // which resides in the runtime library. However, this is not
  // enough for MT paradyn. So Paradyn overrides this setting as 
  // part of its initialization.
    const std::string vrbleName = "DYNINST_tramp_guards";
    pdvector<int_variable *> vars;
    if (!findVarsByAll(vrbleName, vars)) 
    {
        fprintf(stderr, "ERROR: failed to initialize tramp guards!\n");
        return false;
    }
    assert(vars.size() == 1);

    Address allocedTrampAddr = 0;
    
    if (getAddressWidth() == 4) {
	// Don't write directly into trampGuardBase_ as a buffer,
	//   in case we're on a big endian architechture.
      unsigned int value;
      readDataWord((void *)vars[0]->getAddress(), 4, &value, true);
      allocedTrampAddr = value;

    } else if (getAddressWidth() == 8) {
	readDataWord((void *)vars[0]->getAddress(), 8, &allocedTrampAddr, true);
    } else assert(0 && "Incompatible mutatee address width");

    trampGuardBase_ = getAOut()->getDefaultModule()->createVariable("DYNINST_tramp_guard", allocedTrampAddr, getAddressWidth());

    return true;
}


//
// dynamic inferior heap stuff
//
#if defined(cap_dynamic_heap)

#if defined(os_vxworks)
#include "vxworks.h"
#define HEAP_DYN_BUF_SIZE (0x4000)
#else
#define HEAP_DYN_BUF_SIZE (0x100000)
#endif
// "imd_rpc_ret" = Inferior Malloc Dynamic RPC RETurn structure
typedef struct {
  bool ready;
  void *result;
  bool completed;
} imd_rpc_ret;

int process::inferiorMallocCallback(process * /*p proc*/, unsigned /* rpc_id */,
                                     void *data, void *result)
{
  global_mutex->_Lock(FILE__, __LINE__);
  inferiorrpc_printf("%s[%d]:  inside inferior malloc callback\n", FILE__, __LINE__);
  imd_rpc_ret *ret = (imd_rpc_ret *)data;
  ret->result = result;
  ret->ready = true;
  ret->completed = true;
  global_mutex->_Unlock(FILE__, __LINE__);
  return 0;
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
   inferiorrpc_printf("%s[%d]:  welcome to inferiorMallocDynamic\n", FILE__, __LINE__);
#if !defined(mips_sgi_irix6_4)
  // Fun (not) case: there's no space for the RPC to execute.
  // It'll call inferiorMalloc, which will call inferiorMallocDynamic...
  // Avoid this with a static bool.
  if (inInferiorMallocDynamic) {
      fprintf(stderr, "%s[%d]:  recursion guard\n", FILE__, __LINE__);
      return;
  }
  inInferiorMallocDynamic = true;
#endif

  // word-align buffer size 
  // (see "DYNINSTheap_align" in rtinst/src/RTheap-<os>.c)
  alignUp(size, 4);
  // build AstNode for "DYNINSTos_malloc" call
  std::string callee = "DYNINSTos_malloc";
  pdvector<AstNodePtr> args(3);
  args[0] = AstNode::operandNode(AstNode::Constant, (void *)(Address)size);
  args[1] = AstNode::operandNode(AstNode::Constant, (void *)lo);
  args[2] = AstNode::operandNode(AstNode::Constant, (void *)hi);
  AstNodePtr code = AstNode::funcCallNode(callee, args);

  // issue RPC and wait for result
  imd_rpc_ret ret = { false, NULL, false };

  bool wasRunning = (status() == running);
 
  /* set lowmem to ensure there is space for inferior malloc */
  getRpcMgr()->postRPCtoDo(code, true, // noCost
                           &inferiorMallocCallback, &ret, 
                           wasRunning, // run when finished?
                           true, // But use reserved memory
                           NULL, NULL); // process-wide

  // Specify that we want to wait for a RPCDone event
  eventType res = evtUndefined;

  inferiorrpc_printf("%s[%d]:  waiting for rpc completion\n", FILE__, __LINE__);
  // Aggravation....
  // We need to override the BPatch paused behavior; we may be BPatch-paused,
  // but we _really_ need to run the process here.

  processRunState_t oldState = sh->overrideSyncContinueState(ignoreRequest);

  do {
      bool rpcNeedsContinue = false;
      getRpcMgr()->launchRPCs(rpcNeedsContinue,
                              wasRunning);
      if( rpcNeedsContinue ) { continueProc(); }
      
      getMailbox()->executeCallbacks(FILE__, __LINE__);
      
      if(hasExited()) {
          fprintf(stderr, "%s[%d]:  BAD NEWS, process has exited\n", FILE__, __LINE__);
          return;
      }
      if (ret.completed) {
          break;
      }
      
      inferiorrpc_printf("%s[%d][%s]:  before wait for RPCDone, status == running is %s\n", 
                         FILE__, __LINE__, getThreadStr(getExecThreadID()), 
                         status() == running ? "true" : "false");
      
      res = sh->waitForEvent(evtRPCSignal, this, NULL /*lwp*/, statusRPCDone);
      getMailbox()->executeCallbacks(FILE__, __LINE__);
      /* If, as we used to, we loop while res != evtRPCSignal, we could
         be kicked out of the loop by some other (random) RPC completing,
         rather than the malloc() call, and we'll fail based on the random
         garbage in the return structure that the callback hasn't filled in yet. */
  } while( ! ret.completed );
  
  sh->overrideSyncContinueState(oldState);

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

        addHeap(h);
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
#endif 

const Address ADDRESS_LO = ((Address)0);
const Address ADDRESS_HI = ((Address)~((Address)0));
//unsigned int totalSizeAlloc = 0;

void process::inferiorFree(Address item) {
  inferiorFreeInternal(item);
}

bool process::inferiorRealloc(Address item, unsigned newSize)
{
  return inferiorReallocInternal(item, newSize);
}

Address process::inferiorMalloc(unsigned size, inferiorHeapType type, 
				Address near_, bool *err)
{
    Address ret = 0;

   if (err) *err = false;
   assert(size > 0);
   

   // allocation range
   Address lo = ADDRESS_LO; // Should get reset to a more reasonable value
   Address hi = ADDRESS_HI; // Should get reset to a more reasonable value
   
#if defined(cap_dynamic_heap)
   inferiorMallocAlign(size); // align size
   // Set the lo/hi constraints (if necessary)
   inferiorMallocConstraints(near_, lo, hi, type);
#else
   /* align to cache line size (32 bytes on SPARC) */
   size = (size + 0x1f) & ~0x1f; 
#endif

   infmalloc_printf("%s[%d]: inferiorMalloc entered; size %d, type %d, near 0x%lx (0x%lx to 0x%lx)\n",
                    FILE__, __LINE__, size, type, near_, lo, hi);

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
#if defined(cap_dynamic_heap)
	case 1: // compact free blocks
            infmalloc_printf("%s[%d]: garbage collecting and compacting\n",
                             FILE__, __LINE__);
#if defined(cap_garbage_collection)
            gcInstrumentation();
#endif
            inferiorFreeCompact();
	  break;
	case 2: // allocate new segment (1MB, constrained)
            infmalloc_printf("%s[%d]: inferiorMallocDynamic for %d (0x%x) bytes between 0x%lx - 0x%lx\n",
                             FILE__, __LINE__, HEAP_DYN_BUF_SIZE, HEAP_DYN_BUF_SIZE, lo, hi);
            inferiorMallocDynamic(HEAP_DYN_BUF_SIZE, lo, hi);
            break;
	case 3: // allocate new segment (sized, constrained)
            infmalloc_printf("%s[%d]: inferiorMallocDynamic for %d (0x%x) bytes between 0x%lx - 0x%lx\n",
                             FILE__, __LINE__, size, size, lo, hi);
	   inferiorMallocDynamic(size, lo, hi);
	   break;
	case 4: // remove range constraints
            infmalloc_printf("%s[%d]: inferiorMalloc: removing range constraints\n",
                             FILE__, __LINE__);
	   lo = ADDRESS_LO;
	   hi = ADDRESS_HI;
	   if (err) {
              fprintf(stderr, "%s[%d]: ERROR!\n", FILE__, __LINE__);
	      *err = true;
	   }
	   break;
	case 5: // allocate new segment (1MB, unconstrained)
            infmalloc_printf("%s[%d]: inferiorMallocDynamic for %d (0x%x) bytes between 0x%lx - 0x%lx\n",
                             FILE__, __LINE__, HEAP_DYN_BUF_SIZE, HEAP_DYN_BUF_SIZE, lo, hi);
	   inferiorMallocDynamic(HEAP_DYN_BUF_SIZE, lo, hi);
	   break;
	case 6: // allocate new segment (sized, unconstrained)
            infmalloc_printf("%s[%d]: inferiorMallocDynamic for %d (0x%x) bytes between 0x%lx - 0x%lx\n",
                             FILE__, __LINE__, size, size, lo, hi);
	   inferiorMallocDynamic(size, lo, hi);
	   break;
	case 7: // deferred free, compact free blocks
            infmalloc_printf("%s[%d]: inferiorMalloc: recompacting\n", FILE__, __LINE__);
            inferiorFreeCompact();
	   break;
#else /* !(cap_dynamic_heap) */
      case 1: // deferred free, compact free blocks
#if defined(cap_garbage_collection)
	  gcInstrumentation();
#endif
          inferiorFreeCompact();
          break;
#endif /* cap_dynamic_heap */
	   
	default: // error - out of memory
	   return(0);
      }
      ret = inferiorMallocInternal(size, lo, hi, type);
      if (ret) break;
   }
   infmalloc_printf("%s[%d]: inferiorMalloc, returning address 0x%lx\n", FILE__, __LINE__, ret);
   return ret;
}

// Cleanup the process object. Delete all sub-objects to ensure we aren't 
// leaking memory and prepare for new versions. Useful when the process exits
// (and the process object is deleted) and when it execs

void process::deleteProcess() 
{
  assert(this);

  // A lot of the behavior here is keyed off the current process status....
  // if it is exited we'll delete things without performing any operations
  // on the process. Otherwise we'll attempt to reverse behavior we've made.
    // For platforms that don't like repeated use of for (unsigned i...)
    unsigned iter = 0;

  // We may call this function multiple times... once when the process exits,
  // once when we delete the process object.
  if (status() == deleted) {
      return;
  }

  // If this assert fires check whether we're setting the status vrble correctly
  // before calling this function
  assert(!isAttached() || !reachedBootstrapState(bootstrapped_bs) || execing());

  // Cancel the BPatch layer's control of anything below this point
  if (sh) {
      sh->overrideSyncContinueState(ignoreRequest);
      sh->clearCachedLocations();
  }


  for (iter = 0; iter < deleted_objects.size(); iter++)
      delete deleted_objects[iter];
  deleted_objects.clear();

  runtime_lib.clear();

  // Signal handlers...
  signalHandlerLocations_.clear();

  // creationMechanism_ remains untouched
  // stateWhenAttached_ remains untouched
  main_function = NULL;
  thread_index_function = NULL;

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

  // Blow away dyn_lwp's that we delayed delete'ing
  for (unsigned lwp_ndx = 0; lwp_ndx < lwps_to_delete.size(); lwp_ndx++) {
      delete lwps_to_delete[lwp_ndx];
  }
  lwps_to_delete.clear();
  
  // Blow away threads; we'll recreate later
  for (unsigned thr_iter = 0; thr_iter < threads.size(); thr_iter++) {
      delete threads[thr_iter];
  }
  threads.clear();


  deferredContinueProc = false;
  previousSignalAddr_ = 0;
  continueAfterNextStop_ = false;
  
  // Don't touch exec; statically allocated anyway.

  if (theRpcMgr) {
      delete theRpcMgr;
  }
  theRpcMgr = NULL;

  // Skipping saveTheWorld; don't know what to do with it.

  dyninstlib_brk_addr = 0;
  main_brk_addr = 0;

  inInferiorMallocDynamic = false;

  // Get rid of our syscall tracing.
  if (tracedSyscalls_) {
      delete tracedSyscalls_;
      tracedSyscalls_ = NULL;
  }

  for (iter = 0; iter < syscallTraps_.size(); iter++) { 
      if(syscallTraps_[iter] != NULL)
          delete syscallTraps_[iter];
  }
  syscallTraps_.clear();

  trapMapping.clearTrapMappings();

  for (iter = 0; iter < tracingRequests.size(); iter++) {
      if(tracingRequests[iter] != NULL)
          delete tracingRequests[iter];
  }
  tracingRequests.clear();

  deleteAddressSpace();


#if defined(os_linux)
  vsyscall_start_ = 0;
  vsyscall_end_ = 0;
  vsyscall_text_ = 0;
  vsyscall_obj = NULL;
#endif

  set_status(deleted);
  
}

//
// cleanup when a process is deleted. Assumed we are detached or the process is exited.
//
process::~process()
{
    // Failed creation... nothing is here yet
    if (!reachedBootstrapState(initialized_bs)) {
        if (sh) SignalGeneratorCommon::stopSignalGenerator(sh);
        sh = NULL;
        return;
    }

    // We require explicit detaching if the process still exists.
    // On the other hand, if it never started...
    if (reachedBootstrapState(bootstrapped_bs)) {
        assert(!isAttached());
    }

    // Most of the deletion is encapsulated in deleteProcess
    deleteProcess();

    /*
      // Removed; if we're detached (or the proc exited) this trick won't work anyway. 
      // And when the process dies the signal generator will clean itself up.

    if (sh) {
      signal_printf("%s[%d]:  removing signal handler for process\n", FILE__, __LINE__);
      SignalGeneratorCommon::stopSignalGenerator(sh);
    }
    */

    // We used to delete the particular process, but this created no end of problems
    // with people storing pointers into particular indices. We set the pointer to NULL.
    for (unsigned lcv=0; lcv < processVec.size(); lcv++) {
        if (processVec[lcv] == this) {
            processVec[lcv] = NULL;
        }        
    }

    if (sh) {
        // Set this to TRUE before setting proc to null so that it knows to go away...
        sh->stop_request = true;
        sh->proc = NULL;
    }
}

// Default process class constructor. This handles both create,
// attach, and attachToCreated cases. We then call an auxiliary
// function (which can return an error value) to handle specific
// cases.
process::process(SignalGenerator *sh_, BPatch_hybridMode mode) :
    cached_result(not_cached), // MOVE ME
    analysisMode_(mode),
    //isAMcacheValid(false),
    RT_address_cache_addr(0),
    parent(NULL),
    sh(sh_),
    creationMechanism_(unknown_cm),
    stateWhenAttached_(unknown_ps),
    main_function(NULL),
    thread_index_function(NULL),
    lastForkingThread(NULL),
    dyn(NULL),
    interpreter_name_(NULL),
    interpreter_base_(0x0),
    representativeLWP(NULL),
    real_lwps(CThash),
    max_number_of_threads(MAX_THREADS),
    thread_structs_base(0),
    deferredContinueProc(false),
    previousSignalAddr_(0),
    continueAfterNextStop_(false),
    status_(neonatal),
    exiting_(false),
    nextTrapIsExec(false),
    inExec_(false),
    theRpcMgr(NULL),
    collectSaveWorldData(true),
    requestTextMiniTramp(false),
    traceLink(0),
    bootstrapState(unstarted_bs),
    suppress_bpatch_callbacks_(false),
    loadDyninstLibAddr(0),
#if defined(os_windows)
    main_breaks(addrHash4),
#endif
    savedRegs(NULL),
    dyninstlib_brk_addr(0),
    main_brk_addr(0),
    systemPrelinkCommand(NULL),
#if defined(os_windows)
    processHandle_(INVALID_HANDLE_VALUE),
    mainFileHandle_(INVALID_HANDLE_VALUE),
#endif
    inInferiorMallocDynamic(false),
    tracedSyscalls_(NULL),
    traceSysCalls_(false),
    traceState_(noTracing_ts),
    libcstartmain_brk_addr(0)
#if defined(os_linux)
    , vsys_status_(vsys_unknown)
    , vsyscall_start_(0)
    , vsyscall_end_(0)
    , vsyscall_text_(0)
    , auxv_parser(NULL)
    , vsyscall_obj(NULL)
    , started_stopped(false)
#endif
{
    // Let's try to profile memory usage
#if defined(PROFILE_MEM_USAGE)
   void *mem_usage = sbrk(0);
   fprintf(stderr, "Process creation: sbrk %p\n", mem_usage);
#endif

   // initialize memory page size
#if defined (os_windows) 
    SYSTEM_INFO sysinfo;
    GetSystemInfo((LPSYSTEM_INFO) &sysinfo);
    memoryPageSize_ = sysinfo.dwPageSize;
#else
    memoryPageSize_ = getpagesize();
#endif

    theRpcMgr = new rpcMgr(this);    
    dyn = new dynamic_linking(this);
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


bool process::setupCreated(int iTraceLink) 
{
    traceLink = iTraceLink; // notice that tracelink will be -1 in the unique
    // case called "AttachToCreated" - Ana 
    // PARADYN ONLY

    creationMechanism_ = created_cm;
    
    // Post-setup state variables
    stateWhenAttached_ = stopped; 
    
    startup_printf("%s[%d]: Creation method: attaching to process\n", FILE__, __LINE__);
    // attach to the child process (machine-specific implementation)
    if (!attach()) { // error check?
        status_ = detached;
         fprintf(stderr, "%s[%d] attach failing here\n", FILE__, __LINE__);
         std::string msg = std::string("Warning: unable to attach to specified process :")
            + utos(getPid());
        showErrorCallback(26, msg.c_str());
        return false;
    }
    startup_printf("%s[%d]: Creation method: returning\n", FILE__, __LINE__);
    return true;
}
    

// Attach version of the above: no trace pipe, but we assume that
// main() has been reached and passed. Someday we could unify the two
// if someone has a good way of saying "has main been reached".
bool process::setupAttached() 
{
    creationMechanism_ = attached_cm;
    // We're post-main... run the bootstrapState forward

#if !defined(os_windows)
    bootstrapState = initialized_bs;
#else
    // We need to wait for the CREATE_PROCESS debug event.
    // Set to "begun" here, and fix up in the signal loop
    bootstrapState = attached_bs;
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
       
         fprintf(stderr, "%s[%d] attach failing here\n", FILE__, __LINE__);
      std::string msg = std::string("Warning: unable to attach to specified process: ")
                   + utos(getPid());
      showErrorCallback(26, msg.c_str());
      return false;
   }

   startup_printf("%s[%d]: attached, getting current process state\n", FILE__, __LINE__);

   // Record what the process was doing when we attached, for possible
   // use later.
   if (isRunning_()) {
       startup_printf("%s[%d]: process running when attached, pausing...\n", FILE__, __LINE__);
       stateWhenAttached_ = running; 
       set_status(running);
       if (!pause())
           return false;
   }
   else {
       startup_printf("%s[%d]: attached to previously paused process\n", FILE__, __LINE__);
       stateWhenAttached_ = stopped;
       set_status(stopped);
   }
   startup_printf("%s[%d]: setupAttached returning true\n",FILE__, __LINE__);

   assert(status() == stopped);
   return true;
}

int HACKSTATUS = 0;

bool process::prepareExec(fileDescriptor &desc) 
{
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

#if defined(os_aix) && defined(cap_proc)
    // AIX oddly detaches from the process... fix that here
    // Actually, it looks like only the as FD is closed (probably because
    // the file it refers to is gone). Reopen.
   getRepresentativeLWP()->reopen_fds();
#endif
#if defined(os_linux)
   if (auxv_parser) {
      auxv_parser->deleteAuxvParser();
      auxv_parser = NULL;
   }
#endif

    // Revert the bootstrap state
    bootstrapState = attached_bs;

    // Reset whether this process is multithread capable -- it may have changed
    cached_result = not_cached;

    // The tramp guard ast cannot be reused for the new process image
    trampGuardAST_ = AstNodePtr();

    // First, duplicate constructor.

    assert(theRpcMgr == NULL);
    assert(dyn == NULL);
    theRpcMgr = new rpcMgr(this);

    dictionary_hash<unsigned, dyn_lwp *>::iterator lwp_iter = real_lwps.begin();
    for (; lwp_iter != real_lwps.end(); lwp_iter++) 
       theRpcMgr->addLWP(*lwp_iter);
    if (representativeLWP)
       theRpcMgr->addLWP(representativeLWP);

    dyn = new dynamic_linking(this);

    startup_printf("%s[%d]: exec exit, setting a.out to %s:%s\n",
                   FILE__, __LINE__, desc.file().c_str(), desc.member().c_str());

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
    set_status(stopped, true, true); // Revert and ignore

    // Annoying; most of our initialization code is in unix.C, and it
    // knows how to get us to main. Problem is, it triggers on a trap...
    // and guess what we just consumed. So replicate it manually.
    setBootstrapState(begun_bs);
    insertTrapAtEntryPointOfMain();

    return true;
}

bool process::finishExec() 
{
    startup_printf("%s[%d]:  about to load DyninstLib\n", FILE__, __LINE__);
    forkexec_printf("%s[%d]:  about to load DyninstLib\n", FILE__, __LINE__);
    async_printf("%s[%d]:  about to load DyninstLib\n", FILE__, __LINE__);
    bool res = loadDyninstLib();
    if (!res)
	{
		fprintf(stderr, "%s[%d]:  FAILED to loadDyninstLib in exec process\n", FILE__, __LINE__);
        return false;
	}
    
    getMailbox()->executeCallbacks(FILE__, __LINE__);
    while (!reachedBootstrapState(bootstrapped_bs)) 
	{
        // We're waiting for something... so wait
        // true: block until a signal is received (efficiency)
        if (hasExited()) 
		{
            return false;
        }

        sh->waitForEvent(evtProcessInitDone);
        getMailbox()->executeCallbacks(FILE__, __LINE__);
    }
    
    if (process::IndependentLwpControl())
        independentLwpControlInit();
    
    set_status(stopped); // was 'exited'
    
    inExec_ = false;
    BPatch::bpatch->registerExecExit(this);

    sh->continueProcessAsync();

    return true;
}

bool process::setupFork() 
{
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

    copyAddressSpace(parent);

    assert(mapped_objects.size() == parent->mapped_objects.size());

    for (unsigned i = 0; i < mapped_objects.size(); i++) {        
        if ((parent->mapped_objects[i]->fileName() == dyninstRT_name.c_str()) ||
            (parent->mapped_objects[i]->fullName() == dyninstRT_name.c_str()))
            runtime_lib.insert(mapped_objects[i]); 
    }

    // And the main func and dyninst RT lib
    if (!setMainFunction())
        return false;
    if (parent->runtime_lib.size()) {
        // This should be set by now...
        assert(runtime_lib.size());
    }
    
    /////////////////////////
    // Threads & LWPs
    /////////////////////////

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

    if (!recognize_threads(parent))
        return false;

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

Architecture
process::getArch() const {
    if(mapped_objects.size() > 0)
        return mapped_objects[0]->parse_img()->codeObject()->cs()->getArch();
    // as with getAddressWidth(), we can call this before we've attached.. 
    return Arch_none;
}

unsigned process::getAddressWidth() const { 
    if (mapped_objects.size() > 0)
        return mapped_objects[0]->parse_img()->codeObject()->cs()->getAddressWidth(); 
    // We can call this before we've attached.. 
    return 4;
}

Address process::offset() const {
    fprintf(stderr,"process::offset() unimpl\n");
    return 0;
}

Address process::length() const {
    fprintf(stderr,"process::length() unimpl\n");
    return 0;
}

bool process::setAOut(fileDescriptor &desc) 
{
   startup_printf("%s[%d]:  enter setAOut\n", FILE__, __LINE__);
    assert(reachedBootstrapState(attached_bs));
    assert(mapped_objects.size() == 0);
    mapped_object *aout = mapped_object::createMappedObject
        (desc, this, getHybridMode());
    if (!aout) {
       startup_printf("%s[%d]:  fail setAOut\n", FILE__, __LINE__);
        return false;
    }
    
    mapped_objects.push_back(aout);

   startup_printf("%s[%d]:  setAOut: adding range\n", FILE__, __LINE__);
    addOrigRange(aout);

   startup_printf("%s[%d]:  setAOut: finding signal handler\n", FILE__, __LINE__);
    findSignalHandler(aout);

    // Find main
   startup_printf("%s[%d]:  leave setAOut/setting main\n", FILE__, __LINE__);
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

bool process::setMainFunction() 
{
    assert(!main_function);
    
    for (unsigned i = 0; i < NUMBER_OF_MAIN_POSSIBILITIES; i++) {
        main_function = findOnlyOneFunction(main_function_names[i]);
        if (main_function) break;
    }

    return true;
}

bool process::setupGeneral() 
{
#if !defined(os_vxworks)
    // Need to have a.out at this point, except on vxWorks.
    assert(mapped_objects.size() > 0);
#endif

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

    startup_printf("%s[%d]: Loading DYNINST lib...\n", FILE__, __LINE__);
    // TODO: loadDyninstLib actually embeds a lot of startup material;
    // should move it up to this function to make things more obvious.
    bool res = loadDyninstLib();
    if(res == false) {
        startup_printf("%s[%d]: ERROR: failed to load DYNINST lib\n", FILE__, __LINE__);
        return false;
    }
    startup_printf("%s[%d]: Waiting for bootstrapped state...\n", FILE__, __LINE__);
    while (!reachedBootstrapState(bootstrapped_bs)) {
       // We're waiting for something... so wait
       // true: block until a signal is received (efficiency)
       if(hasExited()) {
           return false;
       }
       startup_printf("Checking for process event...\n");
       sh->waitForEvent(evtProcessInitDone);
       getMailbox()->executeCallbacks(FILE__, __LINE__);
    }

    if(process::IndependentLwpControl())
        independentLwpControlInit();

    // use heuristics to set hybrid analysis mode 
    if (BPatch_heuristicMode == analysisMode_) {
        if (getAOut()->parse_img()->codeObject()->defensiveMode()) {
            analysisMode_ = BPatch_defensiveMode;
        } else {
            analysisMode_ = BPatch_normalMode;
        }
    }

   return true;
}

//
// Process "fork" ctor, for when a process which is already being monitored by
// paradynd executes the fork syscall.
//
// Needs to strictly duplicate all process information; this is a _lot_ of work.

process::process(process *parentProc, SignalGenerator *sg_, int childTrace_fd) : 
    cached_result(parentProc->cached_result), // MOVE ME
    analysisMode_(parentProc->analysisMode_),
    memoryPageSize_(parentProc->memoryPageSize_),
    //isAMcacheValid(parentProc->isAMcacheValid),
    RT_address_cache_addr(parentProc->RT_address_cache_addr),
    parent(parentProc),
    sh(sg_),
    creationMechanism_(parentProc->creationMechanism_),
    stateWhenAttached_(parentProc->stateWhenAttached_),
    main_function(NULL), // Set later
    thread_index_function(NULL),
    dyn(NULL),  // Set later
    interpreter_name_(NULL),
    interpreter_base_(0x0),
    representativeLWP(NULL), // Set later
    real_lwps(CThash),
    max_number_of_threads(parentProc->max_number_of_threads),
    thread_structs_base(parentProc->thread_structs_base),
    deferredContinueProc(parentProc->deferredContinueProc),
    previousSignalAddr_(parentProc->previousSignalAddr_),
    continueAfterNextStop_(parentProc->continueAfterNextStop_),
    status_(parentProc->status_),
    exiting_(parentProc->exiting_),
    nextTrapIsExec(parentProc->nextTrapIsExec),
    inExec_(parentProc->inExec_),
    theRpcMgr(NULL), // Set later
    collectSaveWorldData(parentProc->collectSaveWorldData),
    requestTextMiniTramp(parentProc->requestTextMiniTramp),
    traceLink(childTrace_fd),
    bootstrapState(parentProc->bootstrapState),
    suppress_bpatch_callbacks_(parentProc->suppress_bpatch_callbacks_),
    loadDyninstLibAddr(0),
#if defined(os_windows)
    main_breaks(addrHash4),
#endif
    savedRegs(NULL), // Later
    dyninstlib_brk_addr(parentProc->dyninstlib_brk_addr),
    main_brk_addr(parentProc->main_brk_addr),
    systemPrelinkCommand(NULL),
    inInferiorMallocDynamic(parentProc->inInferiorMallocDynamic),
    tracedSyscalls_(NULL),  // Later
    traceSysCalls_(parentProc->getTraceSysCalls()),
    traceState_(parentProc->getTraceState()),
    libcstartmain_brk_addr(parentProc->getlibcstartmain_brk_addr())
#if defined(os_linux)
    , vsyscall_start_(parentProc->vsyscall_start_)
    , vsyscall_end_(parentProc->vsyscall_end_)
    , vsyscall_text_(parentProc->vsyscall_text_)
    , auxv_parser(NULL)
    , vsyscall_obj(parentProc->vsyscall_obj)
    , started_stopped(false)
#endif
{
   dyninstRT_name = parentProc->dyninstRT_name;
}

static void cleanupBPatchHandle(int pid)
{
   BPatch::bpatch->unRegisterProcess(pid, NULL);
}


/*
 * Create a new instance of the named process.  Read the symbols and start
 *   the program
 */
process *ll_createProcess(const std::string File, pdvector<std::string> *argv,
			  BPatch_hybridMode &analysisMode, 
			  pdvector<std::string> *envp, const std::string dir = "",
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

  //    int traceLink = -1; // set by forkNewProcess, below.

  //    int pid = -1;
  //    int tid;

    // NT
    //    int thrHandle_temp;

    process *theProc = SignalGeneratorCommon::newProcess(File, dir,
                                                         argv, envp,
                                                         stdin_fd, stdout_fd, 
                                                         stderr_fd, 
                                                         analysisMode);


   if (!theProc || !theProc->sh) {
       startup_printf("%s[%d]: For new process... failed (theProc %p, SH %p)\n", FILE__, __LINE__,
                      theProc, theProc ? theProc->sh : NULL);
       getMailbox()->executeCallbacks(FILE__, __LINE__);
       return NULL;
   }

    startup_printf( "%s[%d]:  Fork new process... succeeded",FILE__, __LINE__);

    // Register the pid with the BPatch library (not yet associated with a
    // BPatch_thread object).
    assert(BPatch::bpatch != NULL);
    BPatch::bpatch->registerProvisionalThread(theProc->sh->getPid());

    theProc->set_status(running);

    // We need to add this as soon as possible, since a _lot_ of things
    // do lookups.
    processVec.push_back(theProc);
    activeProcesses++;

    statusLine("initializing process data structures");

    if (!theProc->setupGeneral()) {
        startup_printf("[%s:%u] - Couldn't setupGeneral\n", FILE__, __LINE__);
        if (theProc->sh)
        cleanupBPatchHandle(theProc->sh->getPid());
        processVec.pop_back();
        delete theProc;
        return NULL;
    }

    // set the stack_addr for the main thread
    if (BPatch_defensiveMode == theProc->getHybridMode()) {
        for (unsigned tidx=0; tidx < theProc->threads.size(); tidx++) {
            Address sp = theProc->threads[tidx]->getActiveFrame().getSP();
            theProc->threads[tidx]->update_stack_addr
                (  ( sp / theProc->getMemoryPageSize() + 1 ) 
                 * theProc->getMemoryPageSize() );
        }
    }

    // Let's try to profile memory usage
#if defined(PROFILE_MEM_USAGE)
   void *mem_usage = sbrk(0);
   fprintf(stderr, "Post process: sbrk %p\n", mem_usage);
#endif

   assert(theProc->reachedBootstrapState(bootstrapped_bs));
   startup_printf("%s[%d]:  process state: %s\n\n\n\n", FILE__, __LINE__, theProc->getBootstrapStateAsString().c_str());
   return theProc;    
}


process *ll_attachProcess(const std::string &progpath, int pid, void *container_proc_,
                          BPatch_hybridMode &analysisMode) 
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
  std::string fullPathToExecutable = process::tryToFindExecutable(progpath, pid);

  if (!fullPathToExecutable.length()) {
      return NULL;
  }

  process *theProc = SignalGeneratorCommon::newProcess(fullPathToExecutable,
                                                       pid, 
                                                       analysisMode);
  if (!theProc || !theProc->sh) {
       startup_printf("%s[%d]: Fork new process... failed\n", FILE__, __LINE__);
    getMailbox()->executeCallbacks(FILE__, __LINE__);
    return NULL;
  }
  theProc->set_up_ptr(container_proc_);

  // Add this as we can't do _anything_ without a process to look up.
  processVec.push_back(theProc);
  activeProcesses++;

  if (!theProc->setupGeneral()) {
        processVec.pop_back();
      delete theProc;
      return NULL;
  }

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
   startup_printf("%s[%d]: Entry to loadDyninstLib\n", FILE__, __LINE__);
   // Wait for the process to get to an initialized (dlopen exists)
   // state
#if !defined(os_windows)    
   if (!reachedBootstrapState(initialized_bs) && wasCreatedViaAttach())
   {
      insertTrapAtEntryPointOfMain();
      continueProc();
   }
#endif
   while (!reachedBootstrapState(libcLoaded_bs)) {
      startup_printf("%s[%d]: Waiting for process to load libc or reach "
                     "initialized state...\n", FILE__, __LINE__);
      if(hasExited()) {
#if defined(os_linux)
         bperr("The process exited during startup.  This is likely due to one " 
               "of two reasons:\n"
               "A). The application is mis-built and unable to load.  Try " 
               "running the application outside of Dyninst and see if it " 
               "loads properly.\n"
               "B). libdyninstAPI_RT is mis-built.  Try loading the library " 
               "into another application and see if it reports any errors.  "
               "Ubuntu users - You may need to rebuild the RT library "
               "with the DISABLE_STACK_PROT line enabled in " 
               "core/make.config.local");
#endif
         startup_printf("%s[%d] Program exited early, never reached " 
                        "initialized state\n", FILE__, __LINE__);
         startup_printf("Error is likely due to the application or RT " 
                        "library having missing symbols or dependencies\n");
         return false;
      }
      getMailbox()->executeCallbacks(FILE__, __LINE__);
      pdvector<eventType> evts;
      evts.push_back(evtProcessAttach); 
      evts.push_back(evtProcessInit); 
      evts.push_back(evtLibcLoaded);
      evts.push_back(evtProcessExit); 
      if (reachedBootstrapState(libcLoaded_bs)) break;
      sh->waitForOneOf(evts);
      getMailbox()->executeCallbacks(FILE__, __LINE__);
   }
   startup_printf("%s[%d]: Ready to initialize dynamic linking tracer\n", 
                  FILE__, __LINE__);

   // We've hit the initialization trap, so load dyninst lib and
   // force initialization
   std::string buffer = std::string("PID=") + utos(getPid());
   buffer += std::string(", initializing shared objects");       
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

   if (!getDyninstRTLibName()) {
      startup_printf("Failed to get Dyninst RT lib name\n");
      return false;
   }
   startup_printf("%s[%d]: Got Dyninst RT libname: %s\n", FILE__, __LINE__,
                  dyninstRT_name.c_str());

   // if tracing system calls then put a breakpoint at __libc_start_main
   if (getTraceState() >= instrumentLibc_ts) {

      // instrument libc
      instrumentLibcStartMain();

      // wait for process to reach initialized bootstrap state 
      while (!reachedBootstrapState(initialized_bs)) {
         startup_printf("%s[%d]: Waiting for process to reach initialized state...\n", 
                        FILE__, __LINE__);
         if(hasExited()) {
            return false;
         }
         getMailbox()->executeCallbacks(FILE__, __LINE__);
         pdvector<eventType> evts;
         evts.push_back(evtProcessAttach); 
         evts.push_back(evtProcessInit); 
         evts.push_back(evtProcessExit); 
         if (reachedBootstrapState(initialized_bs)) break;
         sh->waitForOneOf(evts);
         getMailbox()->executeCallbacks(FILE__, __LINE__);
      }
   }

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
      // LD_PRELOAD worked or we are attaching for the second time
      // or app was linked with RTlib
      setBootstrapState(loadedRT_bs);
      if (main_brk_addr) {
         //The trap at the entry of is usually cleaned when we handle a RT
         // library load.  Since that's not going to happen, clean it manually.
         handleTrapAtEntryPointOfMain(getInitialLwp());
      }
   }
   else {
      // Force a call to dlopen(dyninst_lib)
      buffer = std::string("PID=") + utos(getPid());
      buffer += std::string(", loading dyninst library");       
      statusLine(buffer.c_str());
       
      startup_printf("%s[%d]: Starting load of Dyninst library...\n",
                     FILE__, __LINE__);
      if (!loadDYNINSTlib()) {
         fprintf(stderr, "%s[%d]:  FIXME:  loadDYNINSTlib failed\n", FILE__, __LINE__);
      }
      startup_printf("%s[%d]: Think we have Dyninst RT lib set up...\n", FILE__, __LINE__);
       
      setBootstrapState(loadingRT_bs);

#if !defined(os_vxworks)
      if (!sh->continueProcessAsync()) {
         assert(0);
      }
#endif
       
      // Loop until the dyninst lib is loaded
      while (!reachedBootstrapState(loadedRT_bs)) {
         if(hasExited()) {
            startup_printf("Odd, process exited while waiting for "
                           "Dyninst RT lib load\n");
            return false;
         }
          
         sh->waitForEvent(evtProcessLoadedRT);
      }
      getMailbox()->executeCallbacks(FILE__, __LINE__);

      // We haven't inserted a trap at dlopen yet (as we require the
      // runtime lib for that) So re-check all loaded libraries (and
      // add to the list gotten earlier) We force a compare even
      // though the PC is not at the correct address.
      dyn->set_force_library_check();
      EventRecord load_rt_event;
      load_rt_event.proc = this;
      load_rt_event.lwp = NULL;
      load_rt_event.type = evtLoadLibrary;
      load_rt_event.what = SHAREDOBJECT_ADDED;
      if (!handleChangeInSharedObjectMapping(load_rt_event)) {
         fprintf(stderr, "%s[%d]:  handleChangeInSharedObjectMapping "
                 "failed!\n", FILE__, __LINE__);
      }
      dyn->unset_force_library_check();

      // Make sure the library was actually loaded
      if (!runtime_lib.size()) {
         fprintf(stderr, "%s[%d]:  Don't have runtime library handle\n",
                 FILE__, __LINE__);
         return false;
      }
   }    
   buffer = std::string("PID=") + utos(getPid());
   buffer += std::string(", initializing mutator-side structures");
   statusLine(buffer.c_str());    

   // The library is loaded, so do mutator-side initialization
   buffer = std::string("PID=") + utos(getPid());
   buffer += std::string(", finalizing RT library");
   statusLine(buffer.c_str());    
   startup_printf("%s[%d]: (%d) finalizing dyninst RT library\n", FILE__, __LINE__, getPid());

   if (!finalizeDyninstLib())
      startup_printf("%s[%d]:  failed to finalize dyninst lib\n", FILE__, __LINE__);
      //fprintf(stderr, "%s[%d]:  failed to finalize dyninst lib\n", FILE__, __LINE__);

   if (!reachedBootstrapState(bootstrapped_bs)) {
      // For some reason we haven't run dyninstInit successfully.
      // Probably because we didn't set parameters before 
      // dyninstInit was automatically run. Catchup with
      // an inferiorRPC is the best bet.
      buffer = std::string("PID=") + utos(getPid());
      buffer += std::string(", finalizing library via inferior RPC");
      statusLine(buffer.c_str());    
      iRPCDyninstInit();        
   }

   buffer = std::string("PID=") + utos(getPid());
   buffer += std::string(", dyninst RT lib ready");
   statusLine(buffer.c_str());    

   return true;
}


// Set the shared object mapping for the RT library
bool process::setDyninstLibPtr(mapped_object *RTobj) 
{
    runtime_lib.insert(RTobj);
    return true;
}


// Set up the parameters for DYNINSTinit in the RT lib

bool process::setDyninstLibInitParams() 
{
   startup_cerr << "process::setDYNINSTinitArguments()" << endl;

   int pid = P_getpid();
   
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
         if (!findVarsByAll("libdyninstAPI_RT_init_localCause",
                            vars))
            
            assert(0 && "Could not find necessary internal variable");
   assert(vars.size() == 1);
   if (!writeDataWord((void*)vars[0]->getAddress(), sizeof(int), (void *)&cause))
      fprintf(stderr, "%s[%d]:  writeDataWord failed\n", FILE__, __LINE__);
   vars.clear();
   
   if (!findVarsByAll("libdyninstAPI_RT_init_localPid", vars))
      if (!findVarsByAll("_libdyninstAPI_RT_init_localPid", vars))
         assert(0 && "Could not find necessary internal variable");
   assert(vars.size() == 1);
   if (!writeDataWord((void*)vars[0]->getAddress(), sizeof(int), (void *)&pid))
      fprintf(stderr, "%s[%d]:  writeDataWord failed\n", FILE__, __LINE__);
   vars.clear();   
   
   if (!findVarsByAll("libdyninstAPI_RT_init_maxthreads", vars))
      if (!findVarsByAll("_libdyninstAPI_RT_init_maxthreads", vars))
         assert(0 && "Could not find necessary internal variable");
   assert(vars.size() == 1);
   if (!writeDataWord((void*)vars[0]->getAddress(), sizeof(int), (void *) &max_number_of_threads))
      fprintf(stderr, "%s[%d]:  writeDataWord failed\n", FILE__, __LINE__);
   vars.clear();   
   
   extern int dyn_debug_rtlib;
   if (!findVarsByAll("libdyninstAPI_RT_init_debug_flag", vars))
       if (!findVarsByAll("_libdyninstAPI_RT_init_debug_flag", vars))
           assert(0 && "Could not find necessary internal variable");
   assert(vars.size() == 1);
   if (!writeDataWord((void*)vars[0]->getAddress(), sizeof(int), (void *) &dyn_debug_rtlib))
     fprintf(stderr, "%s[%d]:  writeDataWord failed\n", FILE__, __LINE__);
   vars.clear();   
   if (dyn_debug_rtlib) {
      fprintf(stderr, "%s[%d]:  set var in RTlib for debug...\n", FILE__, __LINE__);
   }

   startup_cerr << "process::installBootstrapInst() complete" << endl;   
   return true;
}

// Call DYNINSTinit via an inferiorRPC
bool process::iRPCDyninstInit() 
{
    startup_printf("%s[%d]: Running DYNINSTinit via irpc\n", FILE__, __LINE__);
    // Duplicates the parameter code in setDyninstLibInitParams()
    int pid = P_getpid();
    int maxthreads = maxNumberOfThreads();
    extern int dyn_debug_rtlib;

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

    pdvector<AstNodePtr> the_args(4);
    the_args[0] = AstNode::operandNode(AstNode::Constant, (void*)(Address)cause);
    the_args[1] = AstNode::operandNode(AstNode::Constant, (void*)(Address)pid);
    the_args[2] = AstNode::operandNode(AstNode::Constant, (void*)(Address)maxthreads);
    the_args[3] = AstNode::operandNode(AstNode::Constant, (void*)(Address)dyn_debug_rtlib);
    AstNodePtr dynInit = AstNode::funcCallNode("DYNINSTinit", the_args);
    getRpcMgr()->postRPCtoDo(dynInit,
                             true, // Don't update cost
                             process::DYNINSTinitCompletionCallback,
                             NULL, // No user data
                             false, // Don't run when done
                             true, // Use reserved memory
                             NULL, NULL);// No particular thread or LWP

    startup_printf("%s[%d][%s]:  posted RPC\n", FILE__, __LINE__, getThreadStr(getExecThreadID()));
    // We loop until dyninst init has run (check via the callback)
     inferiorrpc_printf("%s[%d]:  waiting for rpc completion\n", FILE__, __LINE__);

     bool rpcNeedsContinue = false;
     getRpcMgr()->launchRPCs(rpcNeedsContinue,
                             false); // false: not running
     assert(rpcNeedsContinue);
     continueProc();

    while (!reachedBootstrapState(bootstrapped_bs)) {
    startup_printf("%s[%d][%s]:  waiting for RPC completion\n", FILE__, __LINE__, getThreadStr(getExecThreadID()));
        if (hasExited()) {
            fprintf(stderr, "%s[%d][%s]:  unexpected exit\n", FILE__, __LINE__, getThreadStr(getExecThreadID()));
           return false;
        }
        getMailbox()->executeCallbacks(FILE__, __LINE__);
        // This needs to be before after _anything_ that can give up the global lock.
        if (reachedBootstrapState(bootstrapped_bs)) break;

        //sh->waitForEvent(evtRPCSignal, this, NULL /*lwp*/, statusRPCDone);
        sh->waitForEvent(evtAnyEvent, this, NULL /*lwp*/);
        getMailbox()->executeCallbacks(FILE__, __LINE__);
    }
    startup_printf("%s[%d][%s]:  bootstrapped\n", FILE__, __LINE__, getThreadStr(getExecThreadID()));
    startup_printf("%s[%u]: - Ran DYNINSTinit via irpc\n", FILE__, __LINE__);
    return true;
}

bool process::attach() 
{
   dictionary_hash_iter<unsigned, dyn_lwp *> lwp_iter(real_lwps);
   dyn_lwp *lwp;
   unsigned index;
   assert(getRepresentativeLWP());  // we expect this to be defined
   // Though on linux, if process found to be MT, there will be no
   // representativeLWP since there is no lwp which controls the entire
   // process for MT linux.

   startup_printf("[%d]: attaching to representative LWP\n", getPid());

   if ( !getRepresentativeLWP()->attach()) 
   {
      startup_printf("%s[%d]:  failed to attach to rep lwp\n", FILE__, __LINE__);
      return false;
   }

   while (lwp_iter.next(index, lwp)) 
   {
       startup_printf("[%d]: attaching to LWP %d\n", getPid(), index);

      if (!lwp->attach()) 
	  {
         deleteLWP(lwp);
		 fprintf(stderr, "%s[%d]:  failed to attach to rep lwp\n", FILE__, __LINE__);
         return false;
      }
   }

   // Fork calls attach() but is probably past this; silencing warning
   if (!reachedBootstrapState(attached_bs))
       setBootstrapState(attached_bs);
   signal_printf("%s[%d]: calling signalActiveProcess from attach\n",
                 FILE__, __LINE__);
   //sh->signalActiveProcess();
   startup_printf("[%d]: setting process flags\n", getPid());

   bool ret =  setProcessFlags();
   //if (!ret)
   //  fprintf(stderr, "%s[%d]:  failed to set process flags\n", FILE__, __LINE__);

   return ret;
}


// Callback: finish mutator-side processing for dyninst lib

bool process::finalizeDyninstLib() 
{
   startup_printf("%s[%d]:  isAttached() = %s\n", FILE__, __LINE__, isAttached() ? "true" : "false");
   startup_printf("%s[%d]: %s\n", FILE__, __LINE__, getStatusAsString().c_str());

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
       startup_printf("%s[%d]: - bs_record.event is undefined\n", FILE__, __LINE__);
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
       std::string str=std::string("PID=") + utos(bs_record.pid) + ", installing default (DYNINST) inst...";
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
       std::string str=std::string("PID=") + utos(bs_record.pid) + ", dyninst ready.";
       statusLine(str.c_str());
   }

   startup_printf("%s[%d]:  bootstrap done\n", FILE__, __LINE__);
   // Ready to rock
   setBootstrapState(bootstrapped_bs);
   sh->signalEvent(evtProcessInitDone);

   return true;
}

void finalizeDyninstLibWrapper(process *p) 
{
  global_mutex->_Lock(FILE__, __LINE__);
  p->finalizeDyninstLib();
  global_mutex->_Unlock(FILE__, __LINE__);
}

int process::DYNINSTinitCompletionCallback(process* theProc,
                                            unsigned /* rpc_id */,
                                            void* /*userData*/, // user data
                                            void* /*ret*/) // return value from DYNINSTinit
{
    //global_mutex->_Lock(FILE__, __LINE__);
    startup_printf("%s[%d]:  about to finalize Dyninst Lib\n", FILE__, __LINE__);
    theProc->finalizeDyninstLib();
    //FinalizeRTLibCallback *cbp = new FinalizeRTLibCallback(finalizeDyninstLibWrapper);
    //FinalizeRTLibCallback &cb = *cbp;
    //cb.setSynchronous(false);
    //cb(theProc);
    //global_mutex->_Unlock(FILE__, __LINE__);
    return 0;
}

dyn_thread *process::STdyn_thread() { 
   assert(! multithread_capable());
   assert(threads.size()>0);
   return threads[0];
}


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

#if defined(os_windows)
   return true;
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

void process::updateThreadIndex(dyn_thread *thread, int index) {
    assert(thread->get_index() == -1 && index != -1);
    thread->update_index(index);
    getRpcMgr()->addThread(thread);
}

void process::addThread(dyn_thread *thread)
{
   if (thread->get_index() != -1) {
     getRpcMgr()->addThread(thread);
   }
   threads.push_back(thread);
}

bool process::multithread_ready(bool ignore_if_mt_not_set) {
    if (thread_index_function != NULL)
        return true;
    if (!multithread_capable(ignore_if_mt_not_set))
        return false;
    if (!reachedBootstrapState(loadedRT_bs))
        return false;

    thread_index_function = findOnlyOneFunction("DYNINSTthreadIndex");

    return thread_index_function != NULL;
}

dyn_lwp *process::query_for_stopped_lwp() 
{
   dyn_lwp *foundLWP = NULL;
   dictionary_hash_iter<unsigned, dyn_lwp *> lwp_iter(real_lwps);
   dyn_lwp *lwp, *backup_lwp = NULL;
   unsigned index;

   if(IndependentLwpControl()) {
      while (lwp_iter.next(index, lwp)) {
         if (lwp->status() == stopped || lwp->status() == neonatal)
         {
            if (lwp->isDoingAttach_) {
               backup_lwp = lwp;
               continue;
            }
            foundLWP = lwp;
            break;
         }
      }
      if (!foundLWP && backup_lwp) {
         foundLWP = backup_lwp;
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

dyn_lwp *process::query_for_running_lwp() 
{
   dyn_lwp *foundLWP = NULL;
   dictionary_hash_iter<unsigned, dyn_lwp *> lwp_iter(real_lwps);
   dyn_lwp *lwp;
   unsigned index;

   if(IndependentLwpControl()) {
      while (lwp_iter.next(index, lwp)) {
         if (!lwp) continue;
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
dyn_lwp *process::stop_an_lwp(bool *wasRunning) 
{
   dictionary_hash_iter<unsigned, dyn_lwp *> lwp_iter(real_lwps);
   dyn_lwp *lwp;
   dyn_lwp *stopped_lwp = NULL;
   unsigned index;
   if (!isAttached()) {
     fprintf(stderr, "%s[%d]:  cannot stop_an_lwp, process not attached\n", 
             FILE__, __LINE__);
     return NULL;
   }

   if(IndependentLwpControl()) {
      while (lwp_iter.next(index, lwp)) {
         if (lwp->status() == exited) 
            continue;
         if (lwp->status() == stopped) {
             stopped_lwp = lwp;
             if (wasRunning)
                 *wasRunning = false;
             break;
         }
         if (lwp->pauseLWP()) {
             if (lwp->status() != stopped) 
                 continue;
            stopped_lwp = lwp;
            if (wasRunning)
                *wasRunning = true;
            break;
         }
      }
      if(stopped_lwp == NULL) {
         if (!getRepresentativeLWP()) return NULL;
         if(getRepresentativeLWP()->status() == stopped) {
             if (wasRunning)
                 *wasRunning = false;
         } else {
            getRepresentativeLWP()->pauseLWP();
            if (wasRunning)
                *wasRunning = true;
         }
         stopped_lwp = getRepresentativeLWP();
      }
   } else {
       // We whole-process pause....
      if(status() == stopped) {
          if (wasRunning)
              *wasRunning = false;
      }
      else {
          if (wasRunning)
              *wasRunning = true;
      }

      processRunState_t oldState = sh->overrideSyncContinueState(stopRequest);
      sh->pauseProcessBlocking();
      sh->overrideSyncContinueState(oldState);
      stopped_lwp = getRepresentativeLWP();
   }

   if (!stopped_lwp) {
     fprintf(stderr, "%s[%d][%s]:  stop_an_lwp failing\n", FILE__, __LINE__, getThreadStr(getExecThreadID()));
   }
   return stopped_lwp;
}

bool process::terminateProc() 
{
    if(status() == exited || status() == deleted) {
        return true;
    }
    if (status() == detached || !sh->isRunning()) {
        set_status(exited);
        return true;
    }
    terminateProcStatus_t retVal = terminateProc_();
    switch (retVal) {
    case terminateSucceeded: {
     // handle the kill signal on the process, which will dispatch exit callback
      signal_printf("%s[%d][%s]:  before waitForEvent(evtProcessExit)\n", 
              FILE__, __LINE__, getThreadStr(getExecThreadID()));

      // Let it run so we can see it die...      
      set_status(running);

      if (getExecThreadID() != sh->getThreadID()) {
          signal_printf("%s[%d][%s]:  signalling active process from termination\n", 
                        FILE__, __LINE__, getThreadStr(getExecThreadID()));
          sh->signalActiveProcess();
      }
      sh->waitForEvent(evtProcessExit);
      if (status() != deleted)
          set_status(exited);
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
       set_status(exited);
     return false;
     break;

   }
   assert (0 && "Can't be reached");
   return false;
}

void process::writeDebugDataSpace(void *inTracedProcess, u_int amount, 
				  const void *inSelf)
{
  static unsigned write_no = 0;

  if (!dyn_debug_write)
    return;
  write_printf("char ");
#if defined(arch_x86_64)
  if (getAddressWidth() == 4)
    write_printf("x86_");
  else
    write_printf("amd64_");
#elif defined(arch_x86)
  write_printf("x86_");
#elif defined(arch_power)
  write_printf("power_");
#elif defined(arch_sparc)
  write_printf("sparc_");
#else
  write_printf("unknown_");
#endif
  write_printf("%lx_%d_%u[] = {", inTracedProcess, getPid(), write_no++);

  const unsigned char *buffer = (const unsigned char *) inSelf;
  for (unsigned i=0; i < amount-1; i++) {
    if (amount && !(amount % 10))
      write_printf("\n");
    write_printf("0x%hhx, ", buffer[i]);
  }
  if (amount)
    write_printf("0x%hhx", buffer[amount-1]);
  write_printf(" };\n", buffer[amount-1]);
}

/*
 * Copy data from controller process to the named process.
 */
#if defined(endian_mismatch)
bool process::writeDataWord(void *inTracedProcess, unsigned size,
                            const void *inSelf)
{
    if (size != 2 && size != 4 && size != 8) return false;

    char buf[8];
    const char *word = (const char *)inSelf;
    unsigned int head, tail;
    for (head = 0, tail = size - 1; head < size; ++head, --tail)
        buf[head] = word[tail];

    return writeDataSpace(inTracedProcess, size, buf);
}
#else
bool process::writeDataWord(void *inTracedProcess, unsigned size,
                            const void *inSelf)
{ return writeDataSpace(inTracedProcess, size, inSelf); }
#endif

bool process::writeDataSpace(void *inTracedProcess, unsigned size,
                             const void *inSelf) 
{
   bool needToCont = false;

   if (!isAttached()) return false;

   dyn_lwp *stopped_lwp = query_for_stopped_lwp();
   if(stopped_lwp == NULL) {
      stopped_lwp = stop_an_lwp(&needToCont);
      if(stopped_lwp == NULL) {
         std::string msg =
            std::string("System error: unable to write to process data "
                     "space (WDS): couldn't stop an lwp\n");
         fprintf(stderr, "%s[%d]:  stop_an_lwp failed\n", FILE__, __LINE__);
         showErrorCallback(38, msg);
         return false;
      }
   }
   
   bool res = stopped_lwp->writeDataSpace(inTracedProcess, size, inSelf);
   if (!res) {
       fprintf(stderr, "%s[%d]: WDS failure - %d bytes from %p to %p, lwp %p\n",
               FILE__, __LINE__, size, inSelf, inTracedProcess, stopped_lwp);
       std::string msg = std::string("System error: unable to write to process data "
                               "space (WDS):") + std::string(strerror(errno));
       showErrorCallback(38, msg);
       return false;
   }

   if (dyn_debug_write)
     writeDebugDataSpace(inTracedProcess, size, inSelf);

   if(needToCont) {
      return stopped_lwp->continueLWP();
   }
   return true;
}

#if defined(endian_mismatch)
bool process::readDataWord(const void *inTracedProcess, u_int size,
                           void *inSelf, bool displayErrMsg)
{
    if ((size != 2 && size != 4 && size != 8) ||
        !readDataSpace(inTracedProcess, size, inSelf, displayErrMsg))
        return false;

    char *buf = (char *)inSelf;
    unsigned int head, tail;
    for (head = 0, tail = size - 1; tail > head; ++head, --tail) {
        buf[head] = buf[head] ^ buf[tail];
        buf[tail] = buf[head] ^ buf[tail];
        buf[head] = buf[head] ^ buf[tail];
    }
    return true;
}
#else
bool process::readDataWord(const void *inTracedProcess, u_int size,
                           void *inSelf, bool displayErrMsg)
{ return readDataSpace(inTracedProcess, size, inSelf, displayErrMsg); }
#endif

bool process::readDataSpace(const void *inTracedProcess, u_int size,
                            void *inSelf, bool displayErrMsg) 
{
   bool needToCont = false;

   if (!isAttached()) {
      fprintf(stderr, "%s[%d][%s]:  readDataSpace() failing, not attached\n",
            FILE__, __LINE__, getThreadStr(getExecThreadID()));
      return false;
   }

   dyn_lwp *stopped_lwp = query_for_stopped_lwp();
   if(stopped_lwp == NULL) {
      stopped_lwp = stop_an_lwp(&needToCont);
      if (stopped_lwp == NULL) {
         std::string msg =
            std::string("System error: unable to read to process data "
                  "space: couldn't stop an lwp\n");
         fprintf(stderr, "%s[%d]:  stop_an_lwp failed\n", FILE__, __LINE__);
         showErrorCallback(38, msg);
         return false;
      }
   }

   errno = 0;
   bool res = stopped_lwp->readDataSpace(inTracedProcess, size, inSelf);
   if (!res) {
      if (displayErrMsg) {
         sprintf(errorLine, "System error: "
               "<>unable to read %d@%s from process data space: %s (pid=%d)",
               size, Address_str((Address)inTracedProcess), 
               strerror(errno), getPid());
         fprintf(stderr, "%s[%d]: Failed to read %d from %p: LWP %d\n", 
               FILE__, __LINE__, size, inTracedProcess, stopped_lwp->get_lwp_id());

         std::string msg(errorLine);
         showErrorCallback(38, msg);
      }
   }

   if (needToCont) {
      stopped_lwp->continueLWP();
   }

   return res;
}

#if defined(endian_mismatch)
bool process::writeTextWord(void *inTracedProcess, u_int amount,
                            const void *inSelf)
{
    if (amount != 2 && amount != 4 && amount != 8) return false;

    char buf[8];
    const char *word = (const char *)inSelf;
    unsigned int head, tail;
    for (head = 0, tail = amount - 1; head < amount; ++head, --tail)
        buf[head] = word[tail];

    return writeTextSpace(inTracedProcess, amount, buf);
}
#else
bool process::writeTextWord(void *inTracedProcess, u_int amount,
                            const void *inSelf)
{ return writeTextSpace(inTracedProcess, amount, inSelf); }
#endif

#if 0
bool process::writeTextWord(caddr_t inTracedProcess, int data) {
   bool needToCont = false;

   if (!isAttached()) return false;

   dyn_lwp *stopped_lwp = query_for_stopped_lwp();
   if(stopped_lwp == NULL) {
      stopped_lwp = stop_an_lwp(&needToCont);
      if(stopped_lwp == NULL) {
         std::string msg =
            std::string("System error: unable to write word to process text "
                     "space: couldn't stop an lwp\n");
         fprintf(stderr, "%s[%d]:  stop_an_lwp failed\n", FILE__, __LINE__);
         showErrorCallback(38, msg);
         return false;
      }
   }

  bool res = stopped_lwp->writeTextWord(inTracedProcess, data);
  if (!res) {
     std::string msg = std::string("System error: unable to write word to process "
                             "text space:") + std::string(strerror(errno));
         fprintf(stderr, "%s[%d]:  writeDataSpace failed\n", FILE__, __LINE__);
     showErrorCallback(38, msg);
     return false;
  }

  if (dyn_debug_write)
     writeDebugDataSpace(inTracedProcess, sizeof(int), &data);

  if(needToCont) {
     return stopped_lwp->continueLWP();
  }
  return true;
}
#endif

bool process::writeTextSpace(void *inTracedProcess, u_int amount, 
                             const void *inSelf) 
{
   assert(inTracedProcess);
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
         std::string msg =
            std::string("System error: unable to write to process text "
                     "space (WTS): couldn't stop an lwp\n");
         fprintf(stderr, "%s[%d]:  stop_an_lwp failed\n", FILE__, __LINE__);
         showErrorCallback(38, msg);
         return false;
      }
   }

  bool res = stopped_lwp->writeTextSpace(inTracedProcess, amount, inSelf);

  if (!res) {
     std::string msg = std::string("System error: unable to write to process text "
                             "space (WTS):") + std::string(strerror(errno));
         fprintf(stderr, "%s[%d]:  writeTextSpace failed\n", FILE__, __LINE__);
     showErrorCallback(38, msg);
     return false;
  }

   if (dyn_debug_write)
     writeDebugDataSpace(inTracedProcess, amount, inSelf);
  
  if(needToCont) {
     return stopped_lwp->continueLWP();
  }
  return true;
}

#if defined(endian_mismatch)
bool process::readTextWord(const void *inTracedProcess, u_int amount,
                           void *inSelf)
{
    if ((amount != 2 && amount != 4 && amount != 8) ||
        !readTextSpace(inTracedProcess, amount, inSelf))
        return false;

    char *buf = (char *)inSelf;
    unsigned int head, tail;
    for (head = 0, tail = amount - 1; tail > amount; ++head, --tail) {
        buf[head] = buf[head] ^ buf[tail];
        buf[tail] = buf[head] ^ buf[tail];
        buf[head] = buf[head] ^ buf[tail];
    }
    return true;
}
#else
bool process::readTextWord(const void *inTracedProcess, u_int amount,
                           void *inSelf)
{ return readTextSpace(inTracedProcess, amount, inSelf); }
#endif

// InsrucIter uses readTextSpace
bool process::readTextSpace(const void *inTracedProcess, u_int amount,
                            void *inSelf)
{
   bool needToCont = false;

   if (!isAttached()) return false;

   dyn_lwp *stopped_lwp = query_for_stopped_lwp();
   if(stopped_lwp == NULL) {
      stopped_lwp = stop_an_lwp(&needToCont);
      if(stopped_lwp == NULL) {
         std::string msg =
            std::string("System error: unable to read to process text "
                     "space: couldn't stop an lwp\n");
         showErrorCallback(39, msg);
         return false;
      }
   }

   bool res = stopped_lwp->readTextSpace(inTracedProcess, amount, inSelf);
   if (!res) {
      sprintf(errorLine, "System error: "
              "<>unable to read %d@%s from process text space: %s (pid=%d)",
              amount, Address_str((Address)inTracedProcess), 
              strerror(errno), getPid());
      std::string msg(errorLine);
      showErrorCallback(38, msg);
      fprintf(stderr, "%s[%d]:  readTextSpace failed\n", FILE__, __LINE__);

      return false;
   }

   if (needToCont) {
      return stopped_lwp->continueLWP();
   }

   return true;
}

void process::set_status(processState st,
			 bool global_st /* = true */,
			 bool override /* = false */) 
{
    // There's a state machine:
    // neonatal
    // running <-> stopped
    // detached
    // deleted
    // exited
    // Make sure we never regress on that...

    if (override) {
        status_ = st;
    } 
    else {
        switch (status_) {
        case neonatal:
            status_ = st;
            break;
        case running:
        case stopped:
            if (st == neonatal) {
                fprintf(stderr, "%s[%d]: REGRESSION OF STATUS: %s to %s\n",
                        FILE__, __LINE__, processStateAsString(status_), 
                        processStateAsString(st));
            }
            else
                status_ = st;
            break;
        case detached:
            if ((st == neonatal) ||
                (st == running) ||
                (st == stopped)) {
                fprintf(stderr, "%s[%d]: REGRESSION OF STATUS: %s to %s\n",
                        FILE__, __LINE__, processStateAsString(status_), 
                        processStateAsString(st));
            }
            else
                status_ = st;
            break;
        case exited:
            if ((st == neonatal) ||
                (st == running) ||
                (st == stopped) ||
                (st == detached)) {
                fprintf(stderr, "%s[%d]: REGRESSION OF STATUS: %s to %s\n",
                        FILE__, __LINE__, processStateAsString(status_), 
                        processStateAsString(st));
            }
            else
                status_ = st;
            break;
        case deleted:
            if ((st == neonatal) ||
                (st == running) ||
                (st == stopped) ||
                (st == detached) ||
                (st == exited)) {
                fprintf(stderr, "%s[%d]: REGRESSION OF STATUS: %s to %s\n",
                        FILE__, __LINE__, processStateAsString(status_), 
                        processStateAsString(st));
            }
            else
                status_ = st;
            break;
        default:
            // ???
            assert(0);
            break;
        };
    }

    if (!global_st) return;

   proccontrol_printf("[%s:%u] - Setting everyone to state %s\n",
                      FILE__, __LINE__, 
                      processStateAsString(status_));

   pdvector<dyn_thread *>::iterator iter = threads.begin();
   
   dyn_lwp *proclwp = getRepresentativeLWP();
   if(proclwp) proclwp->internal_lwp_set_status___(status_);
   
   while(iter != threads.end()) {
      dyn_thread *thr = *(iter);
      dyn_lwp *lwp = thr->get_lwp();
      assert(lwp);
      lwp->internal_lwp_set_status___(status_);
      iter++;
   }
}

void process::set_lwp_status(dyn_lwp *whichLWP, processState lwp_st) 
{
   // any lwp status = stopped, means proc status = stopped

   assert(whichLWP != NULL);

   // update the process status
   if(lwp_st == stopped) {
     set_status(stopped, false);
   }

   proccontrol_printf("[%s:%u] - Setting %d to state %s (%d)\n",
                      FILE__, __LINE__, whichLWP->get_lwp_id(),
                      lwp_st == running ? "running" : 
                      lwp_st == stopped ? "stopped" : 
                      lwp_st == exited ? "exited" : "other",
                      lwp_st);
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
	set_status(running, false);
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

   // Let's try having stopped mean all lwps stopped and running mean
   // atleast one lwp running.
   
   if (status_ == neonatal) {
      return true;
   }

   //if (status_ == stopped) {
   //   signal_printf("%s[%d]: tried to stop a stopped process\n", FILE__, __LINE__);
   //   return true;
   //}

   signal_printf("%s[%d]: stopping process\n", FILE__, __LINE__);

   result = stop_();
   if (!result) {
       bperr ("Warning: low-level paused failed, process is not paused\n");
       fprintf(stderr, "%s[%d]:  pause() failing here\n", FILE__, __LINE__);
     return false;
   }
   set_status(stopped, false);

   signal_printf("%s[%d]: process stopped\n", FILE__, __LINE__);

   return true;
}

//process::stop_ is only different on linux
#if !defined(os_linux) && !defined(os_windows)
bool process::stop_(bool waitUntilStop)
{
   if (status_ == stopped) {
       return true;
   }

   assert(status_ == running);      

   bool res = getRepresentativeLWP()->pauseLWP(waitUntilStop);
   if (!res) {
      sprintf(errorLine,
              "warn : in process::pause, pause_ unable to pause process\n");
      logLine(errorLine);
      return false;
   }
   return true;
}
#endif

// There also exists the possibility of objects having been both
// loaded and unloaded (especially at startup) in which case the
// options for ev.type and ev.what are inadequate, but it doesn't
// really matter since both library loads and unloads are processed by
// the same event handler
bool process::decodeIfDueToSharedObjectMapping(EventRecord &ev)
{
   if(!dyn) { 
       fprintf(stderr, "%s[%d]:  no dyn objects, failing ...\n", FILE__, __LINE__);
       return false;
   }

   u_int change_type = 0;
   if (!dyn->decodeIfDueToSharedObjectMapping(ev,change_type))
     return false;

   ev.what = change_type;
   switch(change_type) {
     case SHAREDOBJECT_ADDED:
        ev.type = evtLoadLibrary;
        signal_printf("%s[%d]:  ADDED\n", FILE__, __LINE__);
        return true;
     case SHAREDOBJECT_REMOVED:
        ev.type = evtUnloadLibrary;
        signal_printf("%s[%d]:  REMOVED\n", FILE__, __LINE__);
        return true;
     case SHAREDOBJECT_NOCHANGE:
        signal_printf("%s[%d]:  NOCHANGE\n", FILE__, __LINE__);
        ev.type = evtLoadLibrary;
        return true;
     default:
       fprintf(stderr, "%s[%d]:  WEIRD ERROR, decodeIfDueToSharedObjectMapping"
               "must be broken\n", FILE__, __LINE__);
   };
   return false;
}

/* Uses dynamiclinking::handleIfDueToSharedObjectMapping to figure out
 * what objects were loaded / removed and to create mapped_objects for
 * them.  If an object was loaded we add it using addASharedObject.
 * If one was removed, we call removeASharedObject, which currently
 * doesn't do much.
 */
bool process::handleChangeInSharedObjectMapping(EventRecord &ev)
{
   pdvector<mapped_object *> changed_objs;
   pdvector<bool> is_new_object;
   if(!dyn) { 
       fprintf(stderr, "%s[%d]:  no dyn objects, failing ...\n", FILE__, __LINE__);
       return false;
   }
   if (!dyn->handleIfDueToSharedObjectMapping(ev,changed_objs, is_new_object)) {
       startup_printf("%s[%d]: change in mapping but no changed objs??\n", FILE__, __LINE__);
       return false;
   }
   // figure out how the list changed and either add or remove shared objects
   for (unsigned int i=0; i < changed_objs.size(); i++) {
      if(is_new_object[i]) {// SHAREDOBJECT_ADDED
         signal_printf("%s[%d]:  SHAREDOBJECT_ADDED\n", FILE__, __LINE__);
         const string filename = changed_objs[i]->fileName();
         if (filename.length()==0) {
            cerr << "Warning: new shared object with no name!" << endl;
            continue;
         }
         // addASharedObject is where all the interesting stuff happens.
         if(!addASharedObject(changed_objs[i])) {
            cerr << "Failed to add library " << changed_objs[i]->fullName() << endl;
         }
      } else {// SHAREDOBJECT_REMOVED
         signal_printf("%s[%d]:  SHAREDOBJECT_REMOVED...\n", FILE__, __LINE__);
         if (!removeASharedObject(changed_objs[i])) {
            cerr << "Failed to remove library " << changed_objs[i]->fullName() << endl;
         }
      }
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
     if (! proc->readDataWord((void*)((*vars)[0]->getAddress()), sizeof(val), (void*)&val, true)) {
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
bool process::addASharedObject(mapped_object *new_obj) 
{
    assert(new_obj);
    // Add to mapped_objects
    // Add to codeRange tree
    // Make library callback (will trigger BPatch adding the lib)
    // Perform platform-specific lookups (e.g., signal handler)
    mapped_objects.push_back(new_obj);
    addOrigRange(new_obj);

    findSignalHandler(new_obj);
    std::string msg;

    //if(new_obj->fileName().length() == 0) {
    //return false;
    //}

    signal_printf("Adding shared object %s, addr range 0x%x to 0x%x\n",
                   new_obj->fileName().c_str(),
                   new_obj->getBaseAddress(),
                   new_obj->getBaseAddress() + new_obj->get_size());
    parsing_printf("Adding shared object %s, addr range 0x%x to 0x%x\n",
           new_obj->fileName().c_str(), 
           new_obj->getBaseAddress(),
           new_obj->getBaseAddress() + new_obj->get_size());
    // TODO: check for "is_elf64" consistency (Object)

    // If the static inferior heap has been initialized, then 
    // add whatever heaps might be in this guy to the known heap space.
    if (heapInitialized_) {
        // initInferiorHeap was run already, so hit this guy
        addInferiorHeap(new_obj);
    }
    else {
        startup_printf("%s[%d]: skipping check for new inferior heaps, heap uninitialized\n",
                       FILE__, __LINE__);
    }

	std::string dyninstRT_shortname;
	const char *last_slash;
#if defined(os_windows)
    last_slash = strrchr(new_obj->fileName().c_str(), '\\');
	if (!last_slash) last_slash = strrchr(new_obj->fileName().c_str(), '/');
    std::string shortname = last_slash ? std::string(last_slash +1) : new_obj->fileName().c_str();
#else
    std::string shortname = new_obj->fileName().c_str();
#endif
	
	if (dyninstRT_name.c_str()) {
		last_slash = strrchr(dyninstRT_name.c_str(), '\\');
		if (!last_slash) last_slash = strrchr(dyninstRT_name.c_str(), '/');
		dyninstRT_shortname = last_slash ? std::string(last_slash +1) : dyninstRT_name.c_str();
	}
    std::string longname = new_obj->fullName().c_str();

    if ((shortname == dyninstRT_name) || 
        (longname == dyninstRT_name) ||
        (shortname == dyninstRT_shortname))
    {
      startup_printf("%s[%d]:  handling init of dyninst RT library\n", FILE__, __LINE__);
      if (!setDyninstLibPtr(new_obj)) {
        fprintf(stderr, "%s[%d]:  FATAL, failing to set dyninst lib\n", FILE__, __LINE__);
        assert(0);
      }
      if (!setDyninstLibInitParams()) {
        fprintf(stderr, "%s[%d]:  FATAL, failing to init dyninst lib\n", FILE__, __LINE__);
        assert(0);
      }
      // Clean this up some...
      addAllocatedRegion(new_obj->codeAbs(),
                         new_obj->imageSize());
      addAllocatedRegion(new_obj->dataAbs(),
                         new_obj->dataSize());

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
            msg = std::string("Application was linked with Dyninst/Paradyn runtime library -- this is not necessary");
            statusLine(msg.c_str());
        } else {
            /* The runtime library has been loaded into the inferior
               and previously initialized, probably by a previous
               run or Dyninst or Paradyn.  Bail.  */
	      if (ret == 2)
		   msg = std::string("This process was previously modified by Dyninst -- cannot reattach");
	      else if (ret == 3)
		   msg = std::string("This process was previously modified by Paradyn -- cannot reattach");
	      else
		   assert(0);
	      showErrorCallback(26, msg);
	      return false;
	 }
    }
#endif

    if (!reachedBootstrapState(bootstrapped_bs) || suppress_bpatch_callbacks_) 
        // Don't let BPatch know about modules before we've
        // loaded
        return true;
    

    const pdvector<mapped_module *> &modlist = new_obj->getModules();

    for (unsigned i = 0; i < modlist.size(); i++) {
        mapped_module *curr = modlist[i];
        BPatch::bpatch->registerLoadedModule(this, curr);
    }
    return true;
}

bool process::removeASharedObject(mapped_object *obj) {
    // Remove from mapped_objects list
    for (unsigned j = 0; j < mapped_objects.size(); j++) {
        if (obj == mapped_objects[j]) {
            mapped_objects[j] = mapped_objects.back();
            mapped_objects.pop_back();
            deleted_objects.push_back(obj);
            break;
        }
    }

    if (runtime_lib.end() != runtime_lib.find(obj)) {
        runtime_lib.erase( runtime_lib.find(obj) );
    }
    signal_printf("Removing shared object %s, addr range 0x%x to 0x%x\n",
                   obj->fileName().c_str(),
                   obj->getBaseAddress(),
                   obj->get_size());

    removeOrigRange(obj);

    // Signal handler...
    // TODO


    const pdvector<mapped_module *> &modlist = obj->getModules();

    for (unsigned i = 0; i < modlist.size(); i++) {
        mapped_module *curr = modlist[i];

        BPatch::bpatch->registerUnloadedModule(this, curr);
    }

    return true;
}    
    


// processSharedObjects: This routine is called before main() or on attach
// to an already running process.  It gets and process all shared objects
// that have been mapped into the process's address space
bool process::processSharedObjects() 
{
    if (mapped_objects.size() > 1 && getTraceState() == noTracing_ts) {
        // Already called... probably don't want to call again
        return true;
    }

    pdvector<mapped_object *> shared_objs;
    if (!dyn->getSharedObjects(shared_objs)) {
        startup_printf("dyn failed to get shared objects\n");
        return false;
    }
    statusLine("parsing shared object files");

    // for each element in shared_objects list process the 
    // image file to find new instrumentaiton points
    for(u_int j=0; j < shared_objs.size(); j++){
        // std::string temp2 = std::string(j);
        // 	    temp2 += std::string(" ");
        // 	    temp2 += std::string("the shared obj, addr: ");
        // 	    temp2 += std::string(((*shared_objects)[j])->getBaseAddress());
        // 	    temp2 += std::string(" name: ");
        // 	    temp2 += std::string(((*shared_objects)[j])->getName());
        // 	    temp2 += std::string("\n");
        // 	    logLine(P_strdup(temp2.c_str()));
        if (!addASharedObject(shared_objs[j]))
           logLine("Error after call to addASharedObject\n");
    }

    return true;
}

bool process::dumpMemory(void * addr, unsigned nbytes)
{
    unsigned char *buf = new unsigned char[nbytes];
    memset(buf, 0, nbytes);
    assert(buf);

    if (!readDataSpace((void *)((long)addr-32), nbytes, buf, true)) {
       fprintf(stderr, "%s[%d]:  dumpMemory failing, cannot read\n", FILE__, __LINE__);
       return false;
    }

    fprintf(stderr, "## 0x%p:\n", (void *)((long)addr-32));
    for (unsigned u = 0; u < nbytes; u++)
    {
            fprintf(stderr, " %x", buf[u]);
    }
    fprintf( stderr, "\n" );

    if (!readDataSpace(addr, nbytes, buf, true)) {
       fprintf(stderr, "%s[%d]:  dumpMemory failing, cannot read\n", FILE__, __LINE__);
       return false;
    }

    fprintf(stderr, "## 0x%p:\n", addr);
    for (unsigned u1 = 0; u1 < nbytes; u1++)
    {
            fprintf(stderr, " %x", buf[u1]);
    }
    fprintf(stderr, "\n" );

    return true;
}

void process::addSignalHandler(Address addr, unsigned size) {
    codeRange *handlerLoc;
    if (signalHandlerLocations_.find(addr, handlerLoc)) {
        return; // we're already tracking this location
    }
    handlerLoc = new signal_handler_location(addr, size);
    signalHandlerLocations_.insert((signal_handler_location*)handlerLoc);
}

// We keep a vector of all signal handler locations
void process::findSignalHandler(mapped_object *obj)
{
   startup_printf("%s[%d]: findSignalhandler(%p)\n", FILE__, __LINE__, obj);
    assert(obj);

    // Old skoon
    int_symbol sigSym;
    std::string signame(SIGNAL_HANDLER);

   startup_printf("%s[%d]: findSignalhandler(%p): gettingSymbolInfo\n", FILE__, __LINE__, obj);
    if (obj->getSymbolInfo(signame, sigSym)) {
        // Symbols often have a size of 0. This b0rks the codeRange code,
        // so override to 1 if this is true...
        unsigned size_to_use = sigSym.getSize();
        if (!size_to_use) 
           size_to_use = 1;
        startup_printf("%s[%d]: findSignalhandler(%p): addingSignalHandler(%p, %d)\n", FILE__, __LINE__, obj, (void *) sigSym.getAddr(), size_to_use);
        addSignalHandler(sigSym.getAddr(), size_to_use);
    }

   startup_printf("%s[%d]: leaving findSignalhandler(%p)\n", FILE__, __LINE__, obj);
}

bool process::continueProc(int signalToContinueWith) 
{   
  signal_printf("%s[%d]: continuing process %d\n", FILE__, __LINE__, getPid());
  if (!isAttached()) {
    signal_printf("%s[%d]: warning continue on non-attached %d\n", 
                  FILE__, __LINE__, getPid());
    bpwarn( "Warning: continue attempted on non-attached process\n");
    return false;
  }

  if (sh->waitingForStop()) {
    return false;
  }

  // Asynchronously signals "make my people run".
  sh->continueProcessAsync(signalToContinueWith);

  return true;
}

//Only different on Linux and Windows
#if !defined(os_linux) && !defined(os_windows)
bool process::continueProc_(int sig)
{
    if (status_ == running) {
        signal_printf("%s[%d]: Continue of already running process, skipping\n",
                      FILE__, __LINE__);
    return true;
    }
  bool ret =  getRepresentativeLWP()->continueLWP(sig);
  return ret;
}
#endif

bool process::detachProcess(const bool leaveRunning) 
{
    // BPatch calls this in the process destructor. 
    if ((status() == detached) ||
        (status() == exited) ||
        (status() == deleted))
        return true;

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
bool process::detach(const bool leaveRunning ) 
{

#if !defined(i386_unknown_linux2_0) \
 && !defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */
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

    // Get rid of everything that could contain a trap that we'll no longer handle.
    // This means the dynamic linker trap, syscall traps, and trap-based instrumentation.
    if(dyn) {
        dyn->uninstallTracing();
    }

    if (tracedSyscalls_) {
        delete tracedSyscalls_;
        tracedSyscalls_ = NULL;
    }

#if defined(cap_syscall_trap)
    for (unsigned iter = 0; iter < syscallTraps_.size(); iter++) {
        if(syscallTraps_[iter] != NULL) {
            clearSyscallTrapInternal(syscallTraps_[iter]);
            delete syscallTraps_[iter];
        }
    }
    syscallTraps_.clear();
#endif

#if defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */
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

void process::triggerNormalExitCallback(int exitCode) 
{
    // Err... why not here
    print_stats();


    sh->overrideSyncContinueState(stopRequest);

    exiting_ = true;
    // special case where can't wait to continue process
    if (status() == exited) {
        fprintf(stderr, "%s[%d]:  cannot trigger exit callback, process is gone...\n", FILE__, __LINE__);
        return;
    }

    for (unsigned i = 1; i < threads.size(); i++) {
        BPatch::bpatch->registerThreadExit(this, threads[i]->get_tid(), true);
    }

    BPatch::bpatch->registerNormalExit(this, exitCode);
    sh->overrideSyncContinueState(ignoreRequest);
}

void process::triggerSignalExitCallback(int signalnum) 
{
    // Err... why not here
    print_stats();
    exiting_ = true;

   // special case where can't wait to continue process
#if 0
  // Removed; we set status to exited before calling this so that
  // the user callbacks don't start doing dangerous things - the process
  // is gone and can't be modified.
   if (status() == exited) {
      //fprintf(stderr, "%s[%d]:  cannot trigger exit callback, process already exited\n", 
      //        FILE__, __LINE__);
      return;
   }
#endif
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

bool process::handleForkEntry() 
{
    // Make bpatch callbacks as well
    BPatch::bpatch->registerForkingProcess(getPid(), NULL);
    return true;
}

bool process::handleForkExit(process *child) 
{
    BPatch::bpatch->registerForkedProcess(this, child);
    return true;
}

bool process::handleExecEntry(char *arg0) 
{
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
bool process::handleExecExit(fileDescriptor &desc) 
{
    inExec_ = true;
    // NOTE: for shm sampling, the shm segment has been removed, so we
    //       mustn't try to disable any dataReqNodes in the standard way...
    nextTrapIsExec = false;

   // Should probably be renamed to clearProcess... deletes anything
   // unnecessary
    BPatch::bpatch->registerExecCleanup(this, NULL);
   deleteProcess();

   prepareExec(desc);
   // The companion finishExec gets called from unix.C...

   // Do not call this here; instead we call it at the end of finalizeDyninstLib
   // so that dyninsts are available
   //BPatch::bpatch->registerExec(this);

   return true;
}


dictionary_hash< Address, unsigned > process::stopThread_callbacks( addrHash );
int process::stopThread_ID_counter = 0;

int process::getStopThreadCB_ID(const Address cb)
{
    if (stopThread_callbacks.defines((Address)cb)) {
        return stopThread_callbacks[(Address)cb];
    }
    else {
        int cb_id = ++stopThread_ID_counter;
        stopThread_callbacks[(Address)cb] = cb_id;
        return cb_id;
    }
}

bool process::isRuntimeHeapAddr(Address addr)
{
    for (unsigned hidx=0; hidx < dyninstRT_heaps.size(); hidx++) {
        if (addr >= dyninstRT_heaps[hidx]->addr &&
            addr < dyninstRT_heaps[hidx]->addr + dyninstRT_heaps[hidx]->length) {
            return true;
        }
    }
    return false;
}

/* resolves jump and return transfers into rtlib */
Address process::resolveJumpIntoRuntimeLib(instPoint *srcPt, Address reloc)
{
    Address orig = 0;
    std::list<Address> srcRelocs;
    getRelocAddrs(srcPt->addr(), srcPt->func(), srcRelocs, false);
    for(list<Address>::iterator rit= srcRelocs.begin(); 
        rit != srcRelocs.end(); 
        rit++) 
    {
        Offset adjustment;
        if ( (*rit) > srcPt->addr() ) {
            adjustment = (*rit) - srcPt->addr();
        } else {
            adjustment = srcPt->addr() - (*rit);
        }
        mal_printf("translated indir target %lx to %lx %s[%d]\n",
                   reloc, reloc - adjustment, FILE__,__LINE__);
        if (srcPt->func()->obj() == findObject(reloc - adjustment)) {
            orig = reloc - adjustment;
        }
    }
    return orig;
}


/*    If calculation is a relocated address, translate it to the original addr
 *    case 1: The point is at a return instruction
 *    case 2: The point is a control transfer into the runtime library
 *    Mark returning functions as returning
 *    Save the targets of indirect control transfers (not regular returns)
 */
Address process::stopThreadCtrlTransfer
       (instPoint* intPoint, 
        Address target)
{
    Address pointAddr = intPoint->addr();

    // if the point is a real return instruction and its target is a stack 
    // address, get the return address off of the stack 
    if ( intPoint->isReturnInstruction() && 
         ! intPoint->func()->isSignalHandler() ) 
    {
        mal_printf("%s[%d]: return address is %lx\n", FILE__,
                    __LINE__,target);
    }

    Address unrelocTarget = target;

    if ( isRuntimeHeapAddr( target ) ) {
        // case 1: The point is at a return instruction
        if (intPoint->getPointType() == functionExit) {

            // get unrelocated target address, there are three possibilities
            // a. We're in post-call padding, and targBBI is the call block
            // b. We're in an analyzed fallthrough block
            // c. The stack was tampered with and we need the (mod_pc - pc) 
            //    offset to figure out where we should be

            instPoint *callPt = NULL;
            bblInstance *callBBI = NULL;
            bool tampered = false;
            if ( reverseDefensiveMap_.find(target,callPt) ) {
                // a. 
                callBBI = callPt->block()->origInstance();
            }
            else {
                // b. 
                // if we're in the fallthrough block, match to call block, 
                // and if necessary, add fallthrough edge
                int_function *targFunc = NULL;
                baseTrampInstance *bti = NULL;
                bool hasFT = getRelocInfo(target, unrelocTarget, targFunc, bti);
                assert(hasFT); // otherwise we should be in the defensive map
                
                bblInstance *targBBI = targFunc->findBlockInstanceByAddr
                    (unrelocTarget);
                callBBI = targFunc->findBlockInstanceByAddr
                    (targBBI->firstInsnAddr() -1);
                if (callBBI) {
                    // if necessary, add the fallthrough edge
                    using namespace ParseAPI;
                    Block::edgelist &edges = callBBI->block()->llb()->targets();
                    Block::edgelist::iterator eit = edges.begin();
                    for (; eit != edges.end(); eit++)
                        if (CALL_FT == (*eit)->type())
                            break;
                    if (eit == edges.end()) {
                        // add ft edge
                        vector<Block*>  srcs; 
                        vector<Address> trgs;
                        vector<EdgeTypeEnum> etypes;
                        srcs.push_back(callBBI->block()->llb());
                        trgs.push_back(callBBI->block()->llb()->end());
                        etypes.push_back(CALL_FT);
                        callBBI->func()->ifunc()->img()->codeObject()->
                            parseNewEdges(srcs,trgs,etypes);
                    }
                } 
                else { 
                    tampered = true;
                }
            }
            if (!tampered) {
                unrelocTarget = callBBI->endAddr();
            } 
            else {
                // resolve target subtracting the difference between the address 
                // of the relocated intPoint and its original counterpart
                unrelocTarget = resolveJumpIntoRuntimeLib(intPoint, target);
                if (0 == unrelocTarget) {
                    mal_printf("ERROR: stopThread caught a return target in "
                               "the rtlib heap that it couldn't translate "
                               "%lx=>%lx %s[%d]\n", pointAddr, 
                               target, FILE__, __LINE__);
                    assert(0 && "need to change relocated return addr to orig");
                }
            }
        }
        // case 2: The point is a control transfer into the runtime library
        else {
            // This is an indirect jump or call into the dyninst runtime 
            // library heap, meaning that the indirect target is calculated 
            // by basing off of the program counter at the source block, so
            // adjust the target by comparing to the uninstrumented address of
            // the source instruction
            unrelocTarget = resolveJumpIntoRuntimeLib(intPoint, target);
            if (0 == unrelocTarget) {
                mal_printf("WARNING: stopThread caught an indirect "
                        "call or jump whose target is an unresolved "
                        "runtime library heap address %lx=>%lx %s[%d]\n",
                        pointAddr, target, FILE__, __LINE__);
                assert(0);
            } else {
                fprintf(stderr,"ERROR: jump %lx=>[%lx][%lx] is going to jump/"
                        "call to the rtlib heap address where there is no "
                        "code, need to modify the target somehow %s[%d]\n", 
                        intPoint->addr(), unrelocTarget, 
                        target, FILE__, __LINE__);
            }
        }
        mal_printf("translated target %lx to %lx %s[%d]\n",
                   target, unrelocTarget, FILE__, __LINE__);
    }
    else { // target is not relocated, nothing to do but find the 
           // mapped_object, creating one if necessary, for transfers
           // into memory regions that are allocated at runtime
        mapped_object *obj = findObject(target);
        if (!obj) {
            obj = createObjectNoFile(target);
            if (!obj) {
                fprintf(stderr,"ERROR, point %lx has target %lx that responds "
                        "to no object %s[%d]\n", pointAddr, target, 
                        FILE__,__LINE__);
                assert(0 && "stopThread snippet has an invalid target");
                return 0;
            }
        }
    }

    // save the targets of indirect control transfers, save them in the point
    if (intPoint->isDynamic() && ! intPoint->isReturnInstruction()) {
        intPoint->setSavedTarget(unrelocTarget);
    }
    else if ( ! intPoint->isDynamic() && 
              functionExit != intPoint->getPointType()) 
    {
        // remove unresolved status from point if it is a static ctrl transfer
        if ( ! intPoint->setResolved() ) {
            fprintf(stderr,"We seem to have tried resolving this point[0x%lx] "
                    "twice, why? %s[%d]\n", pointAddr, FILE__,__LINE__);
        }
    }
    return unrelocTarget;
} 

/* This is the simple version
 * 1. Need three pieces of information: 
 * 1a. The instrumentation point that triggered the stopThread event
 * 1b. The ID of the callback function given at the registration 
 *     of the stopThread snippet
 * 1c. The result of the snippet calculation that was given by the user,
 *     if the point is a return instruction, read the return address
 * 2. If the calculation is an address that is meant to be interpreted, do that
 * 3. Invoke the callback
 */
bool process::handleStopThread(EventRecord &ev) 
{
    // 1. Need three pieces of information: 

    /* 1a. The instrumentation point that triggered the stopThread event */
    Address relocPointAddr = // arg1
#if defined (os_windows)
       (Address) ev.fd;
#else
       (Address) ev.info;
#endif

    int_function *pointFunc = NULL;
    Address pointAddr = relocPointAddr;
    baseTrampInstance *pointbti = NULL;
    bool success = getRelocInfo(relocPointAddr, pointAddr, pointFunc, pointbti);
    if (!success) {
        assert(0);
        return false;
    }
    
    instPoint *intPoint = pointFunc->findInstPByAddr(pointAddr);
    if (!intPoint) { 
        assert(0);
        return false; 
    }

    // Read args 2,3 from the runtime library, as in decodeRTSignal,
    // didn't do it there since this is the only RT library event that
    // makes use of them 
    int callbackID = 0x0; //arg2
    void *calculation = 0x0; // arg3

/* 1b. The ID of the callback function given at the registration 
       of the stopThread snippet */
    pdvector<int_variable *> vars;
    // get runtime library arg2 address from runtime lib
    if (sh->sync_event_arg2_addr == 0) {
        std::string arg_str ("DYNINST_synch_event_arg2");
        if (!findVarsByAll(arg_str, vars)) {
            fprintf(stderr, "%s[%d]:  cannot find var %s\n", 
                    FILE__, __LINE__, arg_str.c_str());
            return false;
        }
        if (vars.size() != 1) {
            fprintf(stderr, "%s[%d]:  ERROR:  %d vars matching %s, not 1\n", 
                    FILE__, __LINE__, (int)vars.size(), arg_str.c_str());
            return false;
        }
        sh->sync_event_arg2_addr = vars[0]->getAddress();
    }
    //read arg2 (callbackID)
    if (!readDataSpace((void *)sh->sync_event_arg2_addr, 
                             sizeof(int), &callbackID, true)) {
        fprintf(stderr, "%s[%d]:  readDataSpace failed\n", FILE__, __LINE__);
        return false;
    }

/* 1c. The result of the snippet calculation that was given by the user, 
       if the point is a return instruction, read the return address */
    // get runtime library arg3 address from runtime lib
    if (sh->sync_event_arg3_addr == 0) {
        std::string arg_str ("DYNINST_synch_event_arg3");
        vars.clear();
        if (!findVarsByAll(arg_str, vars)) {
            fprintf(stderr, "%s[%d]:  cannot find var %s\n", 
                    FILE__, __LINE__, arg_str.c_str());
            return false;
        }
        if (vars.size() != 1) {
            fprintf(stderr, "%s[%d]:  ERROR:  %d vars matching %s, not 1\n", 
                    FILE__, __LINE__, (int)vars.size(), arg_str.c_str());
            return false;
        }
        sh->sync_event_arg3_addr = vars[0]->getAddress();
    }
    //read arg3 (calculation)
    if (!readDataSpace((void *)sh->sync_event_arg3_addr, 
                       getAddressWidth(), &calculation, true)) 
    {
        fprintf(stderr, "%s[%d]:  readDataSpace failed\n", FILE__, __LINE__);
        return false;
    }

    mal_printf("handling stopThread %lx[%lx]=>%lx %s[%d]\n", 
               pointAddr, relocPointAddr, (long)calculation, FILE__,__LINE__); 
/* 2. If the callbackID is negative, the calculation is meant to be
      interpreted as the address of code, so we call stopThreadCtrlTransfer
      to translate the target to an unrelocated address */
    bool interpFlag = false;
    if (callbackID < 0) {
        interpFlag = true;
        callbackID *= -1;
        calculation = (void*) 
            stopThreadCtrlTransfer(intPoint, (Address)calculation);
    }

/* 3. Trigger the callback for the stopThread
      using the correct snippet instance ID & event type */
    ((BPatch_process*)up_ptr())->triggerStopThread
        (intPoint, pointFunc, callbackID, (void*)calculation);

    return true;

} // handleStopThread


/* returns true if blocks were overwritten, initializes overwritten
 * blocks and ranges by contrasting shadow pages with current memory
 * contents
 * 1. reads page in from memory
 * 2. constructs overwritten region list
 * 3. constructs overwrittn basic block list
 * 4. determines if the last of the blocks has an abrupt end, in which 
 *    case it marks it as overwritten
 */
bool process::getOverwrittenBlocks
( std::map<Address, unsigned char *>& overwrittenPages,//input
  std::list<pair<Address,Address> >& overwrittenRanges,//output
  std::list<bblInstance *> &writtenBBIs)//output
{
    const unsigned MEM_PAGE_SIZE = getMemoryPageSize();
    unsigned char * memVersion = (unsigned char *) ::malloc(MEM_PAGE_SIZE);
    Address regionStart;
    bool foundStart = false;
    map<Address, unsigned char*>::iterator pIter = overwrittenPages.begin();
    set<mapped_object*> owObjs;
    for (; pIter != overwrittenPages.end(); pIter++) {
        Address curPageAddr = (*pIter).first / MEM_PAGE_SIZE * MEM_PAGE_SIZE;
        unsigned char *curShadow = (*pIter).second;
    
        // 0. check to make sure curShadow is non-null, if it is null, 
        //    that means it hasn't been written to
        if ( ! curShadow ) {
            continue;
        }

        mapped_object* obj = findObject(curPageAddr);
        if (owObjs.end() != owObjs.find(obj)) {
            obj->setCodeBytesUpdated(false);
        }

        // 1. Read the modified page in from memory
        Address readAddr = curPageAddr;
        if (isMemoryEmulated()) {
            bool valid = false;
            boost::tie(valid,readAddr) = getMemEm()->translate(curPageAddr);
            assert(valid);
        }
        readTextSpace((void*)readAddr, MEM_PAGE_SIZE, memVersion);

        // 2. build overwritten region list by comparing shadow, memory
        for (unsigned mIdx = 0; mIdx < MEM_PAGE_SIZE; mIdx++) {
            if ( ! foundStart && curShadow[mIdx] != memVersion[mIdx] ) {
                foundStart = true;
                regionStart = curPageAddr+mIdx;
            } else if (foundStart && curShadow[mIdx] == memVersion[mIdx]) {
                foundStart = false;
                overwrittenRanges.push_back(
                    pair<Address,Address>(regionStart,curPageAddr+mIdx));
            }
        }
        if (foundStart) {
            foundStart = false;
            overwrittenRanges.push_back(
                pair<Address,Address>(regionStart,curPageAddr+MEM_PAGE_SIZE));
        }
    }

    // 3. Determine which basic blocks have been overwritten
    list<pair<Address,Address> >::const_iterator rIter = overwrittenRanges.begin();
    std::list<bblInstance*> curBBIs;
    while (rIter != overwrittenRanges.end()) {
        mapped_object *curObject = findObject((*rIter).first);

        curObject->findBBIsByRange((*rIter).first,(*rIter).second,curBBIs);
        if (curBBIs.size()) {
            mal_printf("overwrote %d blocks in range %lx %lx \n",
                       curBBIs.size(),(*rIter).first,(*rIter).second);
            writtenBBIs.splice(writtenBBIs.end(), curBBIs);
        }

        curBBIs.clear();
        rIter++;
    }

    free(memVersion);
    if (writtenBBIs.size()) {
        return true;
    } else {
        return false;
    }
}

// distribute the work to mapped_objects
void process::updateCodeBytes
    ( const map<Dyninst::Address,unsigned char*>& owPages, //input
      const list<pair<Address,Address> >&owRanges ) //input
{
    std::map<mapped_object *,list<pair<Address,Address> >*> objRanges;
    list<pair<Address,Address> >::const_iterator rIter = owRanges.begin();
    for (; rIter != owRanges.end(); rIter++) {
        mapped_object *obj = findObject((*rIter).first);
        if (objRanges.find(obj) == objRanges.end()) {
            objRanges[obj] = new list<pair<Address,Address> >();
        }
        objRanges[obj]->push_back(pair<Address,Address>(rIter->first, rIter->second));
    }

    std::map<mapped_object *,list<pair<Address,Address> > *>::iterator oIter = 
        objRanges.begin();
    for (; oIter != objRanges.end(); oIter++) 
    {
        oIter->first->updateCodeBytes( *(oIter->second) );
        delete (oIter->second);
    }
    assert(objRanges.size() <= 1); //o/w analysis code may not be prepared for other cases
}


static void otherFuncBlocks(int_function *func, 
                            const set<bblInstance*> &blks, 
                            set<bblInstance*> &otherBlks)
{
    const set<int_basicBlock*,int_basicBlock::compare> &allBlocks = 
        func->blocks();
    for (set<int_basicBlock*,int_basicBlock::compare>::const_iterator bit =
         allBlocks.begin();
         bit != allBlocks.end(); 
         bit++) 
    {
        if (blks.end() == blks.find((*bit)->origInstance())) {
            otherBlks.insert((*bit)->origInstance());
        }
    }
}


/* Summary
 * Given a list of overwritten blocks, find blocks that are unreachable,
 * functions that have been overwritten at their entry points and can go away,
 * and new function entry for functions that are being overwritten while still
 * executing
 *
 * variables
 * f:  the overwritten function
 * ow: the set of overwritten blocks
 * ex: the set of blocks that are executing on the call stack
 * 
 * primitives
 * R(b,s): yields set of reachable blocks for collection of blocks b, starting
 *         at seed blocks s.
 * B(f):   the blocks pertaining to function f
 * EP(f):  the entry point of function f
 * 
 * calculations
 * Elim(f): the set of blocks to eliminate from function f.
 *          Elim(f) = B(f) - R( B(f)-ow , EP(f) )
 * New(f):  new function entry candidates for f's surviving blocks.
 *          If EB(f) not in ow(f), empty set
 *          Else, all blocks e such that ( e in ex AND e in Elim(f) )
 *          Eliminate New(f) elements that have ancestors in New(f)
 * Del(f):  Blocks that can be deleted altogether
 *          F - R( B(f) - ow , New(f) U (EP(f) \ ow(f)) U (ex(f) intersect Elim(f)) )
 * DeadF:   the set of functions that have no executing blocks 
 *          and were overwritten at their entry points
 *          EP(f) in ow(f) AND ex(f) is empty
 */
bool process::getDeadCode
( const std::list<bblInstance*> &owBlocks, // input
  std::set<bblInstance*> &delBlocks, //output: Del(for all f)
  std::map<int_function*,set<bblInstance*> > &elimMap, //output: elimF
  std::list<int_function*> &deadFuncs, //output: DeadF
  std::map<int_function*,bblInstance*> &newFuncEntries) //output: newF
{
    // do a stackwalk to see which functions are currently executing
    pdvector<pdvector<Frame> >  stacks;
    pdvector<Address> pcs;
    if (!walkStacks(stacks)) {
        inst_printf("%s[%d]:  walkStacks failed\n", FILE__, __LINE__);
        return false;
    }
    for (unsigned i = 0; i < stacks.size(); ++i) {
        pdvector<Frame> &stack = stacks[i];
        for (unsigned int j = 0; j < stack.size(); ++j) {
            Address origPC = 0;
            vector<int_function*> dontcare1;
            baseTrampInstance *dontcare2 = NULL;
            getAddrInfo(stack[j].getPC(), origPC, dontcare1, dontcare2);
            pcs.push_back( origPC );
        }
    }

    // group blocks by function
    std::map<int_function*,set<bblInstance*> > deadMap;
    std::set<int_function*> deadEntryFuncs;
    std::set<Address> owBlockAddrs;
    for (list<bblInstance*>::const_iterator bIter=owBlocks.begin();
         bIter != owBlocks.end(); 
         bIter++) 
    {
        deadMap[(*bIter)->func()].insert(*bIter);
        owBlockAddrs.insert((*bIter)->firstInsnAddr());
        if ((*bIter)->block()->llb() == (*bIter)->func()->ifunc()->entry()) {
            deadEntryFuncs.insert((*bIter)->func());
        }
    }

    // for each modified function, calculate ex, ElimF, NewF, DelF
    for (map<int_function*,set<bblInstance*> >::iterator fit = deadMap.begin();
         fit != deadMap.end(); 
         fit++) 
    {

        // calculate ex(f)
        set<bblInstance*> execBlocks;
        for (unsigned pidx=0; pidx < pcs.size(); pidx++) {
            bblInstance *exB = fit->first->findBlockInstanceByAddr(pcs[pidx]);
            if (exB && owBlockAddrs.end() == owBlockAddrs.find(
                                                    exB->firstInsnAddr())) 
            {
                execBlocks.insert(exB);
            }
        }

        // calculate DeadF: EP(f) in ow and EP(f) not in ex
        if ( 0 == execBlocks.size() ) {
            set<bblInstance*>::iterator eb = fit->second.find(
                fit->first->entryBlock()->origInstance());
            if (eb != fit->second.end()) {
                deadFuncs.push_back(fit->first);
                continue;// treated specially, don't need elimF, NewF or DelF
            }
        } 

        // calculate elimF
        set<bblInstance*> keepF;
        list<bblInstance*> seedBs;
        seedBs.push_back(fit->first->entryBlock()->origInstance());
        fit->first->getReachableBlocks(fit->second, seedBs, keepF);
        otherFuncBlocks(fit->first, keepF, elimMap[fit->first]);

        // calculate NewF
        if (deadEntryFuncs.end() != deadEntryFuncs.find(fit->first)) {
            for (set<bblInstance*>::iterator bit = execBlocks.begin();
                 bit != execBlocks.end();
                 bit++) 
            {
                if (elimMap[fit->first].end() != 
                    elimMap[fit->first].find(*bit)) 
                {
                    newFuncEntries[fit->first] = *bit;
                    break; // just need one candidate
                }
            }
        }

        // calculate Del(f)
        seedBs.clear();
        if (deadEntryFuncs.end() == deadEntryFuncs.find(fit->first)) {
            seedBs.push_back(fit->first->entryBlock()->origInstance());
        }
        else if (newFuncEntries.end() != newFuncEntries.find(fit->first)) {
            seedBs.push_back(newFuncEntries[fit->first]);
        }
        for (set<bblInstance*>::iterator xit = execBlocks.begin();
             xit != execBlocks.end();
             xit++) 
        {
            if (elimMap[fit->first].end() != elimMap[fit->first].find(*xit)) {
                seedBs.push_back(*xit);
            }
        }
        keepF.clear();
        fit->first->getReachableBlocks(fit->second, seedBs, keepF);
        otherFuncBlocks(fit->first, keepF, delBlocks);
        
    }

    return true;
}


// will flush addresses of all addresses in the specified range, if the
// range is null, flush all addresses from the cache.  Also flush 
// rt-lib heap addrs that correspond to the range
void process::flushAddressCache_RT(codeRange *flushRange)
{
    if (flushRange) {
        mal_printf("Flushing address cache of range [%lx %lx]\n",
                   flushRange->get_address(), 
                   flushRange->get_address()+flushRange->get_size());
    } else {
        mal_printf("Flushing address cache of rt_lib heap addrs only \n");
    }

    // Find the runtime cache's address if it hasn't been set yet
    if (0 == RT_address_cache_addr) {
        std::string arg_str ("DYNINST_target_cache");
        pdvector<int_variable *> vars;
        if ( ! findVarsByAll(arg_str, vars) ) {
            fprintf(stderr, "%s[%d]:  cannot find var %s\n", 
                    FILE__, __LINE__, arg_str.c_str());
            assert(0);
        }
        if (vars.size() != 1) {
            fprintf(stderr, "%s[%d]:  ERROR:  %d vars matching %s, not 1\n", 
                    FILE__, __LINE__, (int)vars.size(), arg_str.c_str());
            assert(0);
        }
        RT_address_cache_addr = vars[0]->getAddress();
    }

    // Clear all cache entries that match the runtime library
    // Read in the contents of the cache
    Address* cacheCopy = (Address*)malloc(TARGET_CACHE_WIDTH*sizeof(Address));
    if ( ! readDataSpace( (void*)RT_address_cache_addr, 
                          sizeof(Address)*TARGET_CACHE_WIDTH,(void*)cacheCopy,
                          false ) ) 
    {
        assert(0);
    }

    assert(dyninstRT_heaps.size());
    bool flushedHeaps = false;

    while ( true ) // iterate twice, once to flush the heaps, 
    {              // and once to flush the flushRange
        Address flushStart=0;
        Address flushEnd=0;
        if (!flushedHeaps) {
            // figure out the range of addresses we'll want to flush from

            flushStart = dyninstRT_heaps[0]->addr;
            flushEnd = flushStart + dyninstRT_heaps[0]->length;
            for (unsigned idx=1; idx < dyninstRT_heaps.size(); idx++) {
                Address curAddr = dyninstRT_heaps[idx]->addr;
                if (flushStart > curAddr) {
                    flushStart = curAddr;
                }
                curAddr += dyninstRT_heaps[idx]->length;
                if (flushEnd < curAddr) {
                    flushEnd = curAddr;
                }
            }
        } else {
            flushStart = flushRange->get_address();
            flushEnd = flushStart + flushRange->get_size();
        }
        //zero out entries that lie in the runtime heaps
        for(int idx=0; idx < TARGET_CACHE_WIDTH; idx++) {
            //printf("cacheCopy[%d]=%lx\n",idx,cacheCopy[idx]);
            if (flushStart <= cacheCopy[idx] &&
                flushEnd   >  cacheCopy[idx]) {
                cacheCopy[idx] = 0;
            }
        }
        if ( flushedHeaps || !flushRange ) {
            break;
        }
        flushedHeaps = true;
    }

    // write the modified cache back into the RT_library
    if ( ! writeDataSpace( (void*)RT_address_cache_addr,
                           sizeof(Address)*TARGET_CACHE_WIDTH,
                           (void*)cacheCopy ) ) {
        assert(0);
    }
    free(cacheCopy);
}

/* Given an address that's on the call stack, find the function that's 
 * actively executing that address.  This makes most sense for finding the 
 * address that's triggered a context switch back to Dyninst, either 
 * through instrumentation or a signal
 */
int_function *process::findActiveFuncByAddr(Address addr)
{
    int_function *activeFunc = findOrigByAddr(addr)->is_function();
    bblInstance  *activeBBI  = findOrigByAddr(addr)->is_basicBlockInstance();

    if (!activeFunc) { // addr is a relocated address, use relocation map
        Address origAddr = addr;
        baseTrampInstance *bti = NULL;
        bool success = getRelocInfo(addr, origAddr, activeFunc, bti);
        if (success) {
            return activeFunc;
        }
        return NULL;
    }

    assert(activeBBI);
    if ( ! activeBBI->block()->llb()->isShared() ) {
        return activeFunc;
    }

    // unrelocated shared function address, do a stack walk to figure 
    // out which of the shared functions is on the call stack
    bool foundFrame = false;
    activeFunc = NULL; 
    pdvector<pdvector<Frame> >  stacks;
    if ( false == walkStacks(stacks) ) {
        fprintf(stderr,"ERROR: %s[%d], walkStacks failed\n", 
                FILE__, __LINE__);
        assert(0);
    }
    for (unsigned int i = 0; !foundFrame && i < stacks.size(); ++i) {
        pdvector<Frame> &stack = stacks[i];
        for (unsigned int j = 0; !foundFrame && j < stack.size(); ++j) {
            int_function *frameFunc = NULL;
            Frame *curFrame = &stack[j];
            Address framePC = curFrame->getPC();
            int_basicBlock *frameBlock = findBasicBlockByAddr(framePC);
            if (!frameBlock) {
                // if we're at a relocated address, we can translate 
                // back to the right function, if translation fails 
                // frameFunc will still be NULL
                Address origAddr = framePC;
                baseTrampInstance *bti = NULL;
                getRelocInfo(framePC, origAddr, frameFunc, bti);
            } else if (activeBBI->firstInsnAddr() <= framePC && 
                       framePC <= activeBBI->lastInsnAddr() && 
                       j < stack.size()-1) {
                // find the function by looking at the previous stack 
                // frame's call target
                Address callerPC = stack[j+1].getPC();
                bblInstance *callerBBI = findOrigByAddr(callerPC-1)->
                    is_basicBlockInstance();
                if (callerBBI) {
                    instPoint *callPt = callerBBI->func()->findInstPByAddr
                        (callerBBI->block()->origInstance()->endAddr());
                    if (callPt && callPt->callTarget()) {
                        mapped_object *obj = callPt->func()->obj();
                        frameFunc = findFuncByInternalFunc(
                            obj->parse_img()->findFuncByEntry(
                                callPt->callTarget() 
                                - obj->codeBase()));
                    }
                }
            }
            if (frameFunc) {
                foundFrame = true;
                activeFunc = frameFunc;
            }
        }
    }
    if (!foundFrame) {
        vector<ParseAPI::Function*> funcs;
        activeBBI->block()->llb()->getFuncs(funcs);
        activeFunc = findFuncByInternalFunc(static_cast<image_func*>(funcs[0]));
        fprintf(stderr,"ERROR: Stackwalk couldn't figure out which of the %d "
                "functions corresponding to addr %lx is on the call-stack, "
                "choosing func w/ entry %lx at random %s[%d]\n", funcs.size(),
                addr, activeFunc->getAddress(), FILE__,__LINE__);
    }
                
    return activeFunc;
}

bool process::patchPostCallArea(instPoint *callPt)
{
    // 1) Find all the post-call patch areas that correspond to this 
    //    call point
    // 2) Generate and install the branches that will be inserted into 
    //    these patch areas

    // 1...
    AddrPairSet patchAreas;
    if ( ! generateRequiredPatches(callPt, patchAreas) ) {
        return false;
    }

    // 2...
    generatePatchBranches(patchAreas);
    return true;
}

bool process::generateRequiredPatches(instPoint *callPt, 
                                      AddrPairSet &patchAreas)
{
    // We need to figure out where this patch should branch to.
    // To do that, we're going to:
    // 1) Forward map the entry of the ft block to
    //    its most recent relocated version (if that exists)
    // 2) For each padding area, create a (padAddr,target) pair

    // 1)
    bblInstance *callbbi = callPt->block()->origInstance();
    assert(callPt->addr() <= callbbi->lastInsnAddr());
    bblInstance *ftbbi = callbbi->getFallthroughBBL();
    Relocation::CodeTracker::RelocatedElements reloc;
    if (!relocatedCode_.back().origToReloc(ftbbi->firstInsnAddr(),
					   ftbbi->func(),
					   reloc)) 
    {
        assert(0);
        return false;
    }
    Address to = (reloc.instrumentation ? reloc.instrumentation : reloc.instruction);

    // 2) 
    if (forwardDefensiveMap_.end() != forwardDefensiveMap_.find(callPt)) {
        set<DefensivePad>::iterator d_iter = forwardDefensiveMap_[callPt].begin();
        for (; d_iter != forwardDefensiveMap_[callPt].end(); ++d_iter) 
        {
          Address jumpAddr = d_iter->first;
          patchAreas.insert(std::make_pair(jumpAddr, to));
          mal_printf("patching post-call pad for %lx[%lx] with %lx %s[%d]\n",
                     callbbi->lastInsnAddr(), jumpAddr, to, FILE__,__LINE__);
        }
    }
    if (!patchAreas.size()) {
        mal_printf("no relocs to patch for call at %lx\n", callPt->addr());
    }
    return true;
}

void process::generatePatchBranches(AddrPairSet &branchesNeeded) {
  for (AddrPairSet::iterator iter = branchesNeeded.begin();
       iter != branchesNeeded.end(); ++iter) 
  {
    Address from = iter->first;
    Address to = iter->second;

    codeGen gen(64);
    insnCodeGen::generateBranch(gen, from, to);

    // Safety check: make sure we didn't overrun the patch area
    Address lb, ub;
    instPoint *tmp;
    if (!reverseDefensiveMap_.find(from, lb, ub, tmp)) {
      // Huh? This worked before!
      assert(0);
    }
    assert((from + gen.used()) <= ub);
    if (!writeTextSpace((void *)from, 
			gen.used(),
			gen.start_ptr())) {
      assert(0);
    }
  }
}

bool process::isExploratoryModeOn() 
{ 
    return BPatch_exploratoryMode == analysisMode_ ||
           BPatch_defensiveMode   == analysisMode_;
}

#if defined(cap_syscall_trap)
bool process::checkTrappedSyscallsInternal(Address syscall)
{
    for (unsigned i = 0; i < syscallTraps_.size(); i++) {
        if (syscall == syscallTraps_[i]->syscall_id)
            return true;
    }
    
    return false;
}
#endif

#if 0
#if defined(ox_aix) 
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
#endif

// A copy of the BPatch-level instrumentation installer. 

void process::installInstrRequests(const pdvector<instMapping*> &requests) 
{
    if (requests.size() == 0) {
        return;
    }

    // Instrumentation is now generated on a per-function basis, while
    // the requests are per-inst, not per-function. So 
    // accumulate functions, then generate afterwards. 

    vector<int_function *> instrumentedFuncs;

    for (unsigned lcv=0; lcv < requests.size(); lcv++) {

        instMapping *req = requests[lcv];
        pdvector<miniTramp *> minis;
        
        if(!multithread_capable() && req->is_MTonly())
            continue;
        
        pdvector<int_function *> matchingFuncs;
        
        if (!findFuncsByAll(req->func, matchingFuncs, req->lib)) {
            inst_printf("%s[%d]: failed to find any functions matching %s (lib %s), returning failure from installInstrRequests\n", FILE__, __LINE__, req->func.c_str(), req->lib.c_str());
            return;
        }
        else {
            inst_printf("%s[%d]: found %d functions matching %s (lib %s), instrumenting...\n",
                        FILE__, __LINE__, matchingFuncs.size(), req->func.c_str(), req->lib.c_str());
        }

        for (unsigned funcIter = 0; funcIter < matchingFuncs.size(); funcIter++) {
            int_function *func = matchingFuncs[funcIter];
            if (!func) {
                inst_printf("%s[%d]: null int_func detected\n",
                    FILE__,__LINE__);
                continue;  // probably should have a flag telling us whether errors
            }
            
            // should be silently handled or not
            AstNodePtr ast;
            if ((req->where & FUNC_ARG) && req->args.size()>0) {
                ast = AstNode::funcCallNode(req->inst, 
                                            req->args,
                                            this);
            }
            else {
                pdvector<AstNodePtr> def_args;
                def_args.push_back(AstNode::operandNode(AstNode::Constant,
                                                        (void *)0));
                ast = AstNode::funcCallNode(req->inst,
                                            def_args);
            }
            // We mask to strip off the FUNC_ARG bit...
            switch ( ( req->where & 0x7) ) {
            case FUNC_EXIT:
                {
                    const pdvector<instPoint*> func_rets = func->funcExits();
                    for (unsigned j=0; j < func_rets.size(); j++) {
                        miniTramp *mt = func_rets[j]->addInst(ast,
                                                              req->when,
                                                              req->order,
                                                              (!req->useTrampGuard),
                                                              false);
                        if (mt) 
                            minis.push_back(mt);
                        else {
                           fprintf(stderr, "%s[%d]:  failed to addInst here\n", FILE__, __LINE__);
                        }
                    }
                }
                break;
            case FUNC_ENTRY:
                {
                    const pdvector<instPoint *> func_entries = func->funcEntries();
                    for (unsigned k=0; k < func_entries.size(); k++) {
                        miniTramp *mt = func_entries[k]->addInst(ast,
                                                                 req->when,
                                                                 req->order,
                                                                 (!req->useTrampGuard),
                                                                 false);
                        if (mt) 
                            minis.push_back(mt);
                        else {
                           fprintf(stderr, "%s[%d]:  failed to addInst here\n", FILE__, __LINE__);
                        }
                    }
                }
                break;
            case FUNC_CALL:
                {
                    pdvector<instPoint*> func_calls = func->funcCalls();
                    for (unsigned l=0; l < func_calls.size(); l++) {
                        miniTramp *mt = func_calls[l]->addInst(ast,
                                                               req->when,
                                                               req->order,
                                                               (!req->useTrampGuard),
                                                               false);
                        if (mt) 
                            minis.push_back(mt);
                        else {
                           fprintf(stderr, "%s[%d]:  failed to addInst here\n", FILE__, __LINE__);
                        }
                    }
                }
                break;
            default:
                fprintf(stderr, "Unknown where: %d\n",
                        req->where);
            } // switch
	    minis.clear();
        } // matchingFuncs        
        
    } // requests
    relocate();
    return;
}


#if !defined(os_vxworks)
bool process::extractBootstrapStruct(DYNINST_bootstrapStruct *bs_record)
{
  const std::string vrbleName = "DYNINST_bootstrap_info";

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
#endif

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

bool process::dumpCore(const std::string fileOut) {
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

#if defined (os_linux)
  //  linux pids (returned from waitpid) may match lwpids instead of real pids
  for (unsigned u=0; u<size; u++) {
    if (processVec[u] && processVec[u]->lookupLWP(pid))
      return processVec[u];
  }
#endif
  return NULL;
}

std::string process::getBootstrapStateAsString() const 
{
   // useful for debugging
   switch(bootstrapState) {
     case unstarted_bs:
        return "unstarted";
     case begun_bs:
        return "begun";
     case attached_bs:
       return "attached";
     case libcLoaded_bs:
        return "libcLoaded";
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

const char *processStateAsString(processState state) {
    switch (state) {
    case neonatal:    return "neonatal";   break;
    case running:     return "running";    break;
    case stopped:     return "stopped";    break;
    case detached:    return "detached";   break;
    case exited:      return "exited";     break;
    case deleted:     return "deleted";    break;
    case unknown_ps:  return "unknown_ps"; break;
    default:          /*assert(0);*/ break;
    };
    return "<INVALID>";
}

std::string process::getStatusAsString() const 
{
    return std::string(processStateAsString(status_));
}

bool process::uninstallMutations() {
  //assert(0);
  return false;
}

bool process::reinstallMutations() {
  //assert(0);
  return false;
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

#if defined(cap_garbage_collection)

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
void process::gcInstrumentation(pdvector<pdvector<Frame> > &)
{
  return;
#if 0
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

          if (deletedInst->is_multitramp()) {
              mal_printf("garbage collecting multi %p at %lx[%lx %lx] %s[%d]\n",
                      deletedInst, ((multiTramp*)deletedInst)->instAddr(), 
                      deletedInst->get_address(), 
                      deletedInst->get_address() + deletedInst->get_size(), 
                      FILE__,__LINE__);
          } else {
              mal_printf("garbage collecting object %p at [%lx %lx] %s[%d]\n",
                      deletedInst, deletedInst->get_address(), 
                      deletedInst->get_address() + deletedInst->get_size(), 
                      FILE__,__LINE__);
          }

          pendingGCInstrumentation[deletedIter] = 
              pendingGCInstrumentation.back();
          // Lop off the last one
          pendingGCInstrumentation.pop_back();
          // Back up iterator to cover the fresh one
          deletedIter--;
          delete deletedInst;
      }
  }
#endif
}

#endif

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

  return foundLWP;
}

dyn_lwp *process::lookupLWP(unsigned lwp_id) 
{
    if (status_ == deleted) {
        return NULL;
    }

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
dyn_lwp *process::createFictionalLWP(unsigned lwp_id) 
{
   dyn_lwp *lwp = new dyn_lwp(lwp_id, this);
   if (theRpcMgr) // We may not have a manager yet (fork case)
       theRpcMgr->addLWP(lwp);
   return lwp;
}

dyn_lwp *process::createRealLWP(unsigned lwp_id, int /*lwp_index*/) 
{
   dyn_lwp *lwp = new dyn_lwp(lwp_id, this);
   real_lwps[lwp_id] = lwp;
   if (theRpcMgr) // We may not have a manager (fork case)
       theRpcMgr->addLWP(lwp);
   return lwp;
}

void process::deleteLWP(dyn_lwp *lwp_to_delete) 
{
   // remove lwp from internal structures
   if(real_lwps.size() > 0 && lwp_to_delete!=NULL) {
      theRpcMgr->deleteLWP(lwp_to_delete);
      lwp_to_delete->get_lwp_id();
      real_lwps.undef(lwp_to_delete->get_lwp_id());
   }
   if (lwp_to_delete == representativeLWP)
      representativeLWP = NULL;

   // we keep around the actual dyn_lwp until the process exits.
   // avoids random segfaults when threads exit early
   lwps_to_delete.push_back(lwp_to_delete);
}

#if !defined(os_windows)
void process::deleteThread_(dyn_thread * /*thr*/) {
    //Architecture specific deleteThread_ only needed on windows
}
#endif

void process::deleteThread(dynthread_t tid)
{
  if (status() == exited) return;

    processState newst = running;
    pdvector<dyn_thread *>::iterator iter = threads.end();
    while(iter != threads.begin()) {
        dyn_thread *thr = *(--iter);
        dyn_lwp *lwp = thr->get_lwp();

        //Find the deleted thread
        if (thr->get_tid() != tid) 
            {
                //Update the process state, since deleting a thread may change it.
                if (lwp->status() == stopped)
                    newst = stopped;
                continue;
            } 
        threads.erase(iter);  
        if (thr->get_tid() == 0) {
           fprintf(stderr, "%s[%d]:  trying to remove index mapping for tid 0\n", FILE__, __LINE__);
        }
        removeThreadIndexMapping(thr->get_tid(), thr->get_index());

        deleteThread_(thr);
        //Delete the thread
        if (thr->get_index() != -1)
            getRpcMgr()->deleteThread(thr);
        delete thr;
        
        //Delete the lwp below the thread
        if (!execing() || lwp != representativeLWP)
        deleteLWP(lwp);
        
        break;
    }
    
    /*
      if (threads.size() && (status_ == running || status_ == stopped))
        status_ = newst;
    */
}

bool process::readThreadStruct(Address baseAddr, dyninst_thread_t &struc) {
    // If we match the mutatee, this is a straightforward write. If not, we need
    // to fiddle ourselves into a 32-bit structure.
    // Double-check: read it out of the process....
    if (getAddressWidth() == sizeof(dyntid_t)) {
        if (!readDataSpace((void *)baseAddr,
                           sizeof(dyninst_thread_t),
                           (void *)&struc,
                           false)) {
            fprintf(stderr, "Warning: failed to read data space\n");
            return false;
        }
    }
    else {
        assert(getAddressWidth() == sizeof(int));
        assert(sizeof(dyntid_t) == sizeof(void *));

        // Structure copy for comparison:
        /*
          typedef struct {
          int thread_state;
          int next_free;
          int lwp;
          dyntid_t tid;
          } dyninst_thread_t;
        */
        // Aaand the structure better be as big as we think it is - 4*4=16
        // But of course we can't assert it. 

        // We want all the bits. We can read the entire thing, then drop
        // back the pointer at the end.
        if (!readDataSpace((void *)baseAddr,
                           3*sizeof(int),
                           (void *)&struc,
                           false)) {
            fprintf(stderr, "Warning: failed to read data space\n");
            return false;
        }
        // We got the first three; slurp the fourth.
        unsigned int temp = 0;
        if (!readDataSpace((void *)(baseAddr + (3*sizeof(int))),
                           sizeof(int),
                           (void *)&temp,
                           false)) {
            fprintf(stderr, "Warning: failed to read data space\n");
            return false;
        }
        struc.tid = (void *)(long)temp;
    }
    return true;
}

bool process::removeThreadIndexMapping(dynthread_t tid, unsigned index)
{
    //fprintf(stderr, "%s[%d]:  welcome to removeThreadIndexMapping for %lu\n", FILE__, __LINE__, (unsigned long) tid);
    if (runtime_lib.size())
        return false;

    // Don't worry 'bout it if we're cleaning up and exiting anyway.
    if (exiting_) {
       //fprintf(stderr, "%s[%d]:  ignoring remove... we are exiting\n", FILE__, __LINE__);
       return true;
    }

	if ((unsigned)-1 == index) {
		// The thread wasn't created mutatee side.  Don't need to do anything
		return false;
	}
    signal_printf("%s[%d]: past wait loop, deleting thread....\n", FILE__, __LINE__);

    bool res = false;
    dyn_lwp *lwpToUse=NULL;
    bool continueLWP = false;
    Address thread_struct_addr = 0;
    Address addrToWrite = 0;


    signal_printf("%s[%d]:  removing thread index %d for tid %lu: status is %s\n", 
                  FILE__, __LINE__, 
                  index, tid, 
                  getStatusAsString().c_str());

    lwpToUse = stop_an_lwp(&continueLWP);
    if (!lwpToUse) {
      fprintf(stderr, "Error: no stopped LWP to use in memory write\n");
        goto done;
    }

    signal_printf("%s[%d]: got lwp %d for removeThread write\n",
                  FILE__, __LINE__, lwpToUse->get_lwp_id());
    
    //  Find variable "DYNINST_thread_structs" in the runtime library
    //  this is the array that holds all thread structures
    // We cache this address...
    if (thread_structs_base == 0) {
        const int_variable *thread_structs_var = NULL;

        set<mapped_object *>::iterator runtime_lib_it;
        for(runtime_lib_it = runtime_lib.begin(); 
            runtime_lib_it != runtime_lib.end(); ++runtime_lib_it)
        {
            thread_structs_var = (*runtime_lib_it)->getVariable(DYNINST_thread_structs_name);
            if( thread_structs_var ) break;
        }

        if (!thread_structs_var) {
            goto done;
        }
        
        // Now get the address that pointer points to....
        if (getAddressWidth() == sizeof(thread_structs_base)) {
            if (!readDataSpace((void *)thread_structs_var->getAddress(),
                               getAddressWidth(),
                               (void *)&thread_structs_base,
                               true)) 
            {
                goto done;
            }
        }
        else {
            // We must be 64-bit, they're 32.
            assert(getAddressWidth() == 4);
            assert(sizeof(thread_structs_base) == 8);
            unsigned int temp = 0;
            if (!readDataSpace((void *)thread_structs_var->getAddress(),
                               getAddressWidth(),
                               (void *)&temp,
                               true)) {
	      goto done;
            }
            thread_structs_base = temp;
        }
    }

    if (thread_structs_base == 0) {
      fprintf(stderr, "Error: thread structs at 0?\n");
        goto done;
    }    

    // DO NOT stop the entire process. Our process stop/continue handling
    // is insufficient for this, and you will get accidental continues of lwps that
    // were stopped. 

    // Okay, we have the base addr. Now get the addr of the "real" structure
    if (getAddressWidth() == sizeof(dyntid_t)) {
        thread_struct_addr = thread_structs_base + (index*sizeof(dyninst_thread_t));
    }
    else {
        // Assert AMD64/32
        assert(getAddressWidth() == 4);
        assert(sizeof(dyntid_t) == 8);
        thread_struct_addr = thread_structs_base + (index * (4*sizeof(int)));
    }
    // Mmm array math

    dyninst_thread_t doublecheck;
    
    if (!readThreadStruct(thread_struct_addr, doublecheck)) {
      fprintf(stderr, "%s[%d]: Error: failed to read thread structure\n", FILE__, __LINE__);
        goto done;
    }

    if (doublecheck.tid != (dyntid_t) tid) {
      fprintf(stderr, "%s[%d]:  ERROR:  mismatch between tids %lu != %lu\n", 
              FILE__, __LINE__, (unsigned long) doublecheck.tid, tid);
      goto done;
    }

    if (doublecheck.thread_state != THREAD_COMPLETE) {
        // On platforms where we need to implement thread exit...
        //fprintf(stderr, "%s[%d]:  SKIPPING CLEANUP for thread %lu, thread state is %d, not THREAD_COMPLETE\n", FILE__, __LINE__, tid, doublecheck.thread_state);
       //goto done;
    }

    doublecheck.thread_state = LWP_EXITED;

    // We only want to write the thread state...
    addrToWrite = thread_struct_addr + P_offsetof(dyninst_thread_t, thread_state);

    if (!writeDataSpace((void *)addrToWrite,
                        sizeof(doublecheck.thread_state),
                        (void *)&(doublecheck.thread_state))) {
        fprintf(stderr, "ERROR: resetting thread state failed!\n");
        goto done;
    }
    res = true;

 done:
    // We do this again, as a reschedule during a read may have caused us to
    // realize the process exited. Fun, eh?
    if (exiting_) {
      return true;
    }

    if (continueLWP && lwpToUse) {
        sh->continueProcessAsync(-1, lwpToUse);
    }


    if (!res) 
       fprintf(stderr, "%s[%d]:  ERROR resetting thread state\n", FILE__, __LINE__);


    return res;
}

//used to pass multiple values with the doneRegistering callback
typedef struct done_reg_bundle_t {
   pdvector<int> *lwps;
   pdvector<int> *indexes;
   unsigned *num_completed;
   int this_lwp;
} done_reg_bundle_t;

static int doneRegistering(process *, unsigned, void *data, void *result) 
{
   done_reg_bundle_t *pairs = (done_reg_bundle_t *) data;
   
   long int index = (long int) result;
   int lwp_id = pairs->this_lwp;

   startup_printf("[%s:%u] - RPC for LWP %d completed\n", FILE__, __LINE__, lwp_id);
   pairs->lwps->push_back(lwp_id);
   pairs->indexes->push_back((int)index);
   (*pairs->num_completed)++;
   free(pairs);
   return 0;
}

bool process::recognize_threads(process *parent) 
{
  pdvector<unsigned> lwp_ids;
  unsigned i;
  int result;

  suppress_bpatch_callbacks_ = true;

  startup_printf("%s[%d]: Recognizing threads in process\n",
                 FILE__, __LINE__);

  if (multithread_capable()) {
      result = determineLWPs(lwp_ids);
      if (!result) {
          startup_printf("Error. Recognize_threads couldn't determine LWPs\n");
          suppress_bpatch_callbacks_ = false;
          return false;
      }
  }

  if (!multithread_capable() || 
      ((lwp_ids.size() == 1) && parent)) {
      startup_printf("%s[%d]: creating default thread\n",
                     FILE__, __LINE__);
     // Easy case
     if (parent) {
        assert(parent->threads.size() > 0);
        dyn_thread *base_thread = parent->lastForkingThread;
        parent->lastForkingThread = NULL;
        if (!base_thread)
           base_thread = parent->threads[0];
        if(process::IndependentLwpControl() && (lwp_ids.size() == 1))
           new dyn_thread(base_thread, this, getLWP(lwp_ids[0]));
        else
           new dyn_thread(base_thread, this);
        //The constructor automatically adds the thread to the process list,
        // thus it's safe to not keep a pointer to the new thread
     }
     suppress_bpatch_callbacks_ =false;
     return true;
  }

  if (dyn_debug_startup) {
    startup_printf("[%s:%u] - determineLWPs found %d lwps\n", FILE__, 
		       __LINE__, lwp_ids.size());
    for (unsigned i=0; i<lwp_ids.size(); i++)
      startup_printf("\tLWP #%d = %d\n", i, lwp_ids[i]);
  }

  // Is there someone out there already?
  if (getRpcMgr()->existsActiveIRPC()) 
  {
      fprintf(stderr, "%s[%d]: Odd case: active iRPC  before MT recognition!\n", 
			  FILE__, __LINE__);
  }

  //If !parent, then we're not a forked process and we can just assign the threads to
  //LWPs arbitrarly.  This is most used in the attach/create cases. 
  if (!parent)
  {
     unsigned num_completed = 0;
     //Parallel arrays for simplicity
     pdvector<int> ret_indexes;
     pdvector<int> ret_lwps;

#if defined(os_windows)
     //We continue Windows threads when we find them (so we can keep recieving
     // events on them).  Stop them all now.
     pause();
#endif
     if (status() == exited) 
         return false;

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
     unsigned expected = 0;

     for (i = 0; i < lwp_ids.size(); i++)
     {
        unsigned lwp_id = lwp_ids[i];
        dyn_lwp *lwp = getLWP(lwp_id);

	// See comment below about Solaris threads.
	// TODO: see if there is some mechanism of identifying the helper
	// threads...
#if defined(os_aix) || defined(os_solaris)
	if (lwp->executingSystemCall()) {
            startup_printf("%s[%d]: LWP %d in a system call, skipping in recognize_threads\n",
                           FILE__, __LINE__, lwp_id);
            continue;
        }
#endif

        // So Solaris has these extra threads....
        // #3 appears to be a scheduler, though I couldn't find documentation
        // on what it does. For now, we're hard-skipping 3.
#if (os_solaris==8)
        if (lwp_id == 3)
            continue;
#endif

        if (lwp == NULL) {
            // GRIPE!
            fprintf(stderr, "WARNING: skipping lwp %d; couldn't get handle\n",
                    lwp_id);
            continue;
        }
            
        if (lwp->is_asLWP()) continue;
        
        const pdvector<int_function *> *thread_funcs = NULL;
        
        set<mapped_object *>::iterator runtime_lib_it;
        for(runtime_lib_it = runtime_lib.begin(); 
            runtime_lib_it != runtime_lib.end(); ++runtime_lib_it) 
        {
            thread_funcs = (*runtime_lib_it)->findFuncVectorByMangled("DYNINSTthreadIndex");
            if( thread_funcs ) break;
        }
        assert(thread_funcs && thread_funcs->size() == 1);
        int_function *thread_func = (*thread_funcs)[0];

        pdvector<AstNodePtr> ast_args;
        AstNodePtr ast = AstNode::funcCallNode(thread_func, ast_args);

        done_reg_bundle_t *bundle = (done_reg_bundle_t*) 
           malloc(sizeof(done_reg_bundle_t));
        bundle->indexes = &ret_indexes;
        bundle->lwps = &ret_lwps;
        bundle->this_lwp = lwp_id;
        bundle->num_completed = &num_completed;

        startup_printf("%s[%d]: RECOGNITION RPC posting on LWP %d\n", FILE__, __LINE__, lwp_id);

        getRpcMgr()->postRPCtoDo(ast, true, 
                                 doneRegistering, 
                                 bundle, 
                                 false, // Don't run when done
                                 false, 
                                 NULL, lwp);
        expected++;
     }

     startup_printf("%s[%d]:  waiting for rpc completion\n", FILE__, __LINE__);

     // We hit a problem here. On Solaris, some LWPs are "fake" and permanently
     // in a system call. We can't run iRPCs on those threads at all, and if we
     // wait for them to complete we'll just spin. 
     // Better yet, we don't do independent LWP control - so if we're doing
     // process create and wait for them _at all_ we'll run the process past
     // its starting point. 
     // Instead, we skip anything in a syscall.
     bool rpcNeedsContinue = false;     
     getRpcMgr()->launchRPCs(rpcNeedsContinue, false);
     startup_printf("%s[%d]: launchRPCs complete for process attach, completed %d expected %d\n",
                    FILE__, __LINE__, num_completed, expected);
     if (rpcNeedsContinue)
         continueProc();         

     while (getRpcMgr()->existsActiveIRPC()) {
         if(hasExited()) {
             fprintf(stderr, "%s[%d]:  unexpected process exit\n", FILE__, __LINE__);
             suppress_bpatch_callbacks_ =false;
             return false;
         }
         
         sh->waitForEvent(evtRPCSignal, this, NULL /*lwp*/, statusRPCDone, 
			  false); /* Do _not_ execute callbacks; we want to finish this
	                             before we handle any thread stuff */
         startup_printf("%s[%d]: got RPC event...\n",
                        FILE__, __LINE__);
     }

     // Really really really don't call this; apparently it hoses things but good - bernat, 4MAY06
     // getMailbox()->executeCallbacks(FILE__, __LINE__);

     // Don't assert these... the RPCs can fail (well DYNINSTthreadIndex can fail).
     // assert(ret_lwps.size() == expected);
     // assert(ret_indexes.size() == expected);

     if (hasExited()) {
         return false;
     }

     assert(status() == stopped);
     
     BPatch_process *bproc = BPatch::bpatch->getProcessByPid(getPid());
     assert(bproc);

     startup_printf("[%s:%u] - All LWPs completed.  Waiting for threads\n", FILE__, __LINE__);
     for (i = 0; i < ret_lwps.size(); ++i) {         
         BPatch_thread *bpthrd = NULL;
         // Wait for the thread to show up...
         int timeout = 0;
         startup_printf("[%s:%u] - Waiting for thread for LWP %d\n", FILE__, 
                        __LINE__, ret_lwps[i]);
         while ( (bpthrd = bproc->getThreadByIndex(ret_indexes[i])) == NULL) {
             startup_printf("%s[%d]: waiting for bp thread at index %d\n", FILE__, __LINE__, ret_indexes[i]);
            // getMailbox()->executeCallbacks(FILE__, __LINE__);
            sh->waitForEvent(evtThreadCreate, NULL, NULL, NULL_STATUS_INITIALIZER, false);
            timeout++;
            if (timeout > 1000) break;
         }
     }
     startup_printf("[%s:%u] - All threads found.  Returning.\n", FILE__, __LINE__);

     assert(status() == stopped);
     suppress_bpatch_callbacks_ =false;
     return true;
  } // end non-fork case

  // Fork case
  
  // We have LWPs with objects. The parent has a vector of threads.
  // Hook them up.

  threads.clear();

  // Two cases: one thread made it (pthreads) or multiple threads got copied
  // over. The multiple case is tricky... here we handle the common single case.

  for (unsigned thr_iter = 0; thr_iter < parent->threads.size(); thr_iter++) {
     unsigned matching_lwp = 0;
     dyn_thread *par_thread = parent->threads[thr_iter];
     forkexec_printf("Updating thread %d (tid %d)\n",
                     thr_iter, par_thread->get_tid());
     
     for (unsigned lwp_iter = 0; lwp_iter < lwp_ids.size(); lwp_iter++) {
         forkexec_printf("%s[%d]: checking lwp %d, %d of %d\n", 
                         lwp_ids[lwp_iter],
                         lwp_iter+1,
                         lwp_ids.size());
         
         if (lwp_ids[lwp_iter] == 0) {
             continue;
         }
         dyn_lwp *lwp = getLWP(lwp_ids[lwp_iter]);
         
         // The functionality is not strictly cap_syscall_trap specific,
         // but is not implemented on any other platforms so I'm IFDEFing
         // it -- bernat, jun07
#if defined(cap_syscall_trap)
         if (par_thread->get_lwp()->executingSystemCall()) {
             forkexec_printf("%s[%d]: parent lwp executing system call\n",
                             FILE__, __LINE__);
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
         else 
#endif
             {
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
        forkexec_printf("Creating new thread %d (tid %d) on lwp %d, parent was on %d\n",
                        thr_iter, new_thr->get_tid(), new_thr->get_lwp()->get_lwp_id(),
                        par_thread->get_lwp()->get_lwp_id());
     }
     else {
        forkexec_printf("Failed to find match for thread %d (tid %d), assuming deleted\n",
                        thr_iter, par_thread->get_tid());
     }
  }
  suppress_bpatch_callbacks_ =false;
  return true;
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

static int mapIndexToTid_cb(process *, unsigned, void *data, void *result)
{
   dynthread_t *tid = (dynthread_t *) data;
   *tid = (dynthread_t) result;
   return RPC_LEAVE_AS_IS;
}

//Turn a thread index into a tid via an iRPC
dynthread_t process::mapIndexToTid(int index)
{
   dynthread_t tid = (dynthread_t) -1;
   pdvector<AstNodePtr> ast_args;
   ast_args.push_back(AstNode::operandNode(AstNode::Constant, (void *)(long)index));
   AstNodePtr call_get_tid = AstNode::funcCallNode("DYNINST_getThreadFromIndex", ast_args, this);
   
   getRpcMgr()->postRPCtoDo(call_get_tid, true, mapIndexToTid_cb, &tid,
                            false, // Don't run when done
                            false, NULL, NULL);

     inferiorrpc_printf("%s[%d]:  waiting for rpc completion\n", FILE__, __LINE__);
   while (tid == -1)
   {
       bool rpcNeedsContinue = false;
       getRpcMgr()->launchRPCs(rpcNeedsContinue,
                               false);
       assert(rpcNeedsContinue);
       continueProc();

      getMailbox()->executeCallbacks(FILE__, __LINE__);
      if(hasExited()) return (dynthread_t) -1;
      sh->waitForEvent(evtRPCSignal, this, NULL /*lwp*/, statusRPCDone);
      getMailbox()->executeCallbacks(FILE__, __LINE__);
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

#if defined(arch_x86_64)
//MATT TODO: Temporarily commiting, but should remove
void print_regs(dyn_lwp *lwp);
#endif

Address process::stepi(bool verbose, int lwp) {
   /**
    * Safety checking and warning messages
    **/
   if (status() == detached || status() == exited)
   {
      if (verbose) {
         fprintf(stderr, "[stepi @ %s:%u] - Error. Process %d no longer " 
                 "exists.\n",  FILE__, __LINE__, getPid());
      }
      return (Address) -1;
   }
   if (status() == running)
   {
      if (verbose) {
         fprintf(stderr, "[stepi @ %s:%u] - Warning. Process %d was running.\n",
                 FILE__, __LINE__, getPid());
      }
      bool result = pause();
      if (!result)
      {
         if (verbose) {
            fprintf(stderr, "[stepi @ %s:%u] - Error. Couldn't stop %d.\n", 
                    FILE__, __LINE__, getPid());
         }
         return (Address) -1;
      }
   }
   
   /**
    * Step the process forward one instruction.  If we're being verbose
    * then get a code range for the next instruction and print some information.
    **/
   dyn_lwp *lwp_to_step;
   if (lwp == -1) {
      lwp_to_step = getRepresentativeLWP();
      if (!lwp_to_step) {
         lwp_to_step = getInitialLwp();
      }
   }
   else {
      lwp_to_step = lookupLWP(lwp);
      if (!lwp_to_step) {
         if (verbose)
            fprintf(stderr, "%s[%u]: - Couldn't find lwp %d\n",
                    FILE__, __LINE__, lwp);
         return (Address) -1;
      }
   }
      
   Address nexti = lwp_to_step->step_next_insn();
   
   if ((nexti == (Address) -1) || lwp_to_step->status() != stopped)
   {
      if (verbose) {
         fprintf(stderr, "%s[%u]: - Warning. Couldn't step %d.\n",
                 FILE__, __LINE__, getPid());
      }
   }

   /**
    * Print the results if they're wanted.
    **/
   if (!verbose)
      return nexti;
   
   codeRange *range = findOrigByAddr(nexti);

   fprintf(stderr, "0x%lx ", nexti);
   if (range)
      range->print_range();
   else
      fprintf(stderr, "\n");

#if defined(arch_x86_64)
   //MATT TODO: Temporarily commiting, but should remove
   if (getAddressWidth() == 8) {
      print_regs(lwp_to_step);
   }
#endif
   return nexti;
}

void process::disass(Address start, Address end) 
{
   disass(start, end, false);
}

void process::disass(Address start, Address end, bool leave_files) 
{
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

/**
 * debugSuicide is a kind of alternate debugging continueProc.  It runs the process 
 * until terminated in single step mode, printing each instruction as it executes.
 **/
void process::debugSuicide() {
   pdvector<Frame> activeFrames;
   getAllActiveFrames(activeFrames);
 
   last_single_step = 0;
   for (unsigned i=0; i < activeFrames.size(); i++) {
     Address addr = activeFrames[i].getPC();
     codeRange *range = findOrigByAddr(addr);
     fprintf(stderr, "Frame %u @ 0x%lx ", i, addr);
     if (range)
        range->print_range();
     else
        fprintf(stderr, "\n");
   }

    while (!hasExited()) {
        stepi();
    }
}

dyn_lwp *process::getInitialLwp() const {
  if (!getInitialThread())
    return NULL;
  return getInitialThread()->get_lwp();
}

int process::getPid() const { return sh ? sh->getPid() : -1;}

bool process::shouldSaveFPState() {
   return BPatch::bpatch->isSaveFPROn();
}

const char *process::getInterpreterName() {
   if (interpreter_name_)
      return interpreter_name_;
   if (!mapped_objects.size())
      return NULL;
   interpreter_name_ = getAOut()->parse_img()->linkedFile->getInterpreterName();
   return interpreter_name_;
}

void process::setInterpreterName(const char *name) {
   interpreter_name_ = name;
}

Address process::getInterpreterBase() {
#if defined(os_linux)
   if (!interpreter_base_)
      readAuxvInfo();
#endif
   return interpreter_base_;
}

void process::setInterpreterBase(Address newbase) {
   interpreter_base_ = newbase;
}

#if !defined(os_linux)
// Implementation is only really needed on Linux
Address process::setAOutLoadAddress(fileDescriptor &desc) {
   return desc.loadAddr();
}
#endif

bool process::mappedObjIsDeleted(mapped_object *mobj) {
   for (unsigned i=0; i<deleted_objects.size(); i++)
      if (mobj == deleted_objects[i])
         return true;
   return false;
}

#if !defined(os_linux)
bool process::detachForDebugger(const EventRecord &)
{
   return false;
}
#endif

bool process::needsPIC()
{
   return 0;
}
