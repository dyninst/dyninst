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

extern "C" {
#ifdef PARADYND_PVM
int pvmputenv (const char *);
int pvmendtask();
#endif
}

#include "util/h/headers.h"
#include "rtinst/h/rtinst.h"
#include "rtinst/h/trace.h"
#include "symtab.h"
#include "process.h"
#include "util.h"
#include "inst.h"
#include "instP.h"
#include "dyninstP.h"
#include "os.h"
#include "showerror.h"
#include "costmetrics.h"
#include "perfStream.h"
#include "dynamiclinking.h"
#include "paradynd/src/mdld.h"
#include "util/h/debugOstream.h"

#ifdef ATTACH_DETACH_DEBUG
debug_ostream attach_cerr(cerr, true);
#else
debug_ostream attach_cerr(cerr, false);
#endif

#ifdef INFERIOR_RPC_DEBUG
debug_ostream inferiorrpc_cerr(cerr, true);
#else
debug_ostream inferiorrpc_cerr(cerr, false);
#endif

#ifdef SHM_SAMPLING_DEBUG
debug_ostream shmsample_cerr(cerr, true);
#else
debug_ostream shmsample_cerr(cerr, false);
#endif

#ifdef FORK_EXEC_DEBUG
debug_ostream forkexec_cerr(cerr, true);
#else
debug_ostream forkexec_cerr(cerr, false);
#endif

#ifdef METRIC_DEBUG
debug_ostream metric_cerr(cerr, true);
#else
debug_ostream metric_cerr(cerr, false);
#endif

#ifdef SIGNAL_DEBUG
debug_ostream signal_cerr(cerr, true);
#else
debug_ostream signal_cerr(cerr, false);
#endif


#define FREE_WATERMARK (hp->totalFreeMemAvailable/2)
#define SIZE_WATERMARK 100
static const timeStamp MAX_WAITING_TIME=10.0;
static const timeStamp MAX_DELETING_TIME=2.0;

unsigned activeProcesses; // number of active processes
vector<process*> processVec;
string process::programName;
vector<string> process::arg_list;

process *findProcess(int pid) { // make a public static member fn of class process
  unsigned size=processVec.size();
  for (unsigned u=0; u<size; u++)
    if (processVec[u] && processVec[u]->getPid() == pid)
      return processVec[u];
  return NULL;
}

#ifdef SHM_SAMPLING
static unsigned numIntCounters=10000; // rather arbitrary; can we do better?
static unsigned numWallTimers =10000; // rather arbitrary; can we do better?
static unsigned numProcTimers =10000; // rather arbitrary; can we do better?
#endif

bool waitingPeriodIsOver()
{
static timeStamp previous=0;
timeStamp current;
bool waiting=false;

  if (!previous) {
    previous=getCurrentTime(false);
    waiting=true;
  }
  else {
    current=getCurrentTime(false);
    if ( (current-previous) > MAX_WAITING_TIME ) {
      previous=getCurrentTime(false);
      waiting=true;
    }
  }
  return(waiting);
}

Frame::Frame(process *proc) {

    frame_ = pc_ = 0;
    proc->getActiveFrame(&frame_, &pc_);

    uppermostFrame = true;
}

Frame Frame::getPreviousStackFrameInfo(process *proc) const {
   if (frame_ == 0){
      // no prev frame exists; must return frame with 0 pc value otherwise
      // will never break out of loop in walkStack()
      Frame fake_frame(0,0,false);
      return fake_frame; 
   }

   int fp = frame_;
   int rtn = pc_;

   Frame result(0, 0, false);
   if (proc->readDataFromFrame(frame_, &fp, &rtn, uppermostFrame)) {
      result.frame_ = fp;
      result.pc_    = rtn;
   }

   return result;
}

vector<Address> process::walkStack(bool noPause)
{
  vector<Address> pcs;
  bool needToCont = noPause ? false : (status() == running);

  if (!noPause && !pause()) {
     // pause failed...give up
     cerr << "walkStack: pause failed" << endl;
     return pcs;
  }

  Address sig_addr = 0;
  u_int sig_size = 0;
  if(signal_handler){
      const image *sig_image = (signal_handler->file())->exec();
      if(getBaseAddress(sig_image, sig_addr)){
          sig_addr += signal_handler->getAddress(this);
      } else {
          sig_addr = signal_handler->getAddress(this);
      }
      sig_size = signal_handler->size();
      // printf("signal_handler = %s size = %d addr = 0x%x\n",
      //     (signal_handler->prettyName()).string_of(),sig_size,sig_addr);
  }

  if (pause()) {
    Frame currentFrame(this);
    while (!currentFrame.isLastFrame()) {
      Address next_pc = currentFrame.getPC();
      // printf("currentFrame pc = %d\n",next_pc);
      pcs += next_pc;
      // is this pc in the signal_handler function?
      if(signal_handler && (next_pc >= sig_addr)
	  && (next_pc < (sig_addr+sig_size))){
	  // check to see if a leaf function was executing when the signal
	  // handler was called.  If so, then an extra frame should be added
	  // for the leaf function...the call to getPreviousStackFrameInfo
	  // will get the function that called the leaf function
	  Address leaf_pc = 0;
	  if(this->needToAddALeafFrame(currentFrame,leaf_pc)){
              pcs += leaf_pc;
	  }
      }
      currentFrame = currentFrame.getPreviousStackFrameInfo(this); 
    }
    pcs += currentFrame.getPC();
  }
  if (!noPause && needToCont) {
     if (!continueProc()){
        cerr << "walkStack: continueProc failed" << endl;
     }
  }  
  return(pcs);
}

bool isFreeOK(process *proc, const disabledItem &disItem, vector<Address> &pcs) {
  const unsigned disItemPointer = disItem.getPointer();
  const inferiorHeapType disItemHeap = disItem.getHeapType();

#if defined(hppa1_1_hp_hpux)
  if (proc->freeNotOK) return(false);
#endif

  heapItem *ptr=NULL;
  if (!proc->heaps[disItemHeap].heapActive.find(disItemPointer, ptr)) {
    sprintf(errorLine,"Warning: attempt to free not defined heap entry %x (pid=%d, heapActive.size()=%d)\n", disItemPointer, proc->getPid(), proc->heaps[disItemHeap].heapActive.size());
    logLine(errorLine);
    //showErrorCallback(67, (const char *)errorLine);
    return(false);
  }
  assert(ptr);

#ifdef FREEDEBUG1
  sprintf(errorLine, "isFreeOK  called on 0x%x\n", ptr->addr);
  logLine(errorLine);
#endif

  const vector<unsigVecType> &disItemPoints = disItem.getPointsToCheck();
  const unsigned disItemNumPoints = disItemPoints.size();

  for (unsigned int j=0;j<disItemNumPoints;j++) {
    for (unsigned int k=0;k<disItemPoints[j].size();k++) {
      unsigned pointer = disItemPoints[j][k];
#ifdef FREEDEBUG_ON
      if (disItemHeap == dataHeap)
        sprintf(errorLine, "checking DATA pointer 0x%x\n", pointer);
      else
        sprintf(errorLine, "checking TEXT pointer 0x%x\n", pointer);
      logLine(errorLine);
#endif

      const dictionary_hash<unsigned, heapItem*> &heapActivePart =
	      proc->splitHeaps ? proc->heaps[textHeap].heapActive :
                                 proc->heaps[dataHeap].heapActive;

      heapItem *np=NULL;
      if (!heapActivePart.find(pointer, np)) { // fills in "np" if found
#ifdef FREEDEBUG1
	    sprintf(errorLine, "something freed addr 0x%x from us\n", pointer);
	    logLine(errorLine);
#endif
        
        // 
        // This point was deleted already and we don't need it anymore in
        // pointsToCheck
        // 
        const int size=disItemPoints[j].size();
        disItemPoints[j][k] = disItemPoints[j][size-1];
        disItemPoints[j].resize(size-1);

	// need to make sure we check the next item too 
	k--;
      } else {
        // Condition 1
        assert(np);
        if ( (ptr->addr >= np->addr) && 
             (ptr->addr <= (np->addr + np->length)) )
        {

#ifdef FREEDEBUG_ON
          sprintf(errorLine,"*** TEST *** IN isFreeOK: (1) we found 0x%x in our inst. range!\n",ptr->addr);
          logLine(errorLine);
#endif

          return(false);     
        }

        for (unsigned int l=0;l<pcs.size();l++) {
          // Condition 2
          if ((pcs[l] >= ptr->addr) && 
              (pcs[l] <= (ptr->addr + ptr->length))) 
          {

#ifdef FREEDEBUG_ON
    sprintf(errorLine,"*** TEST *** IN isFreeOK: (2) we found 0x%x in our inst. range!\n", ptr->addr);
    logLine(errorLine);
#endif

            return(false);
          }
          // Condition 3
          if ( ((pcs[l] >= np->addr) && (pcs[l] <= (np->addr + np->length))) )
          {

#ifdef FREEDEBUG_ON
    sprintf(errorLine,"*** TEST *** IN isFreeOK: (3) we found PC in our inst. range!\n");
    logLine(errorLine);
#endif

            return(false);     
          }
        }
      }
    }
  }
  return(true);
}

//
// This procedure will try to compact the framented memory available in 
// heapFree. This is an emergency procedure that will be called if we
// are running out of memory to insert instrumentation - naim
//
void inferiorFreeCompact(inferiorHeap *hp)
{
  unsigned size;
  heapItem *np;
  size = hp->heapFree.size();
  unsigned j,i=0;

#ifdef FREEDEBUG
  logLine("***** Trying to compact freed memory...\n");
#endif

  while (i < size) {
    np = hp->heapFree[i];
#ifdef FREEDEBUG1
    sprintf(errorLine,"***** Checking address=%d\n",ALIGN_TO_WORDSIZE(np->addr+np->length));
    logLine(errorLine);
#endif
    for (j=0; j < size; j++) {
      if (i != j) {
        if ( (np->addr+np->length)==(hp->heapFree[j])->addr )
        {
          np->length += (hp->heapFree[j])->length;
          hp->heapFree[j] = hp->heapFree[size-1];
          hp->heapFree.resize(size-1);
          size = hp->heapFree.size();
#ifdef FREEDEBUG1
          sprintf(errorLine,"***** Compacting free memory (%d bytes, i=%d, j=%d, heapFree.size=%d)\n",np->length,i,j,size);
          logLine(errorLine);
#endif
          break;
        }
      }
    }
    if (j == size) i++;
  }

#ifdef FREEDEBUG
  for (i=0;i<hp->disabledList.size();i++) {
    for (j=i+1;j<hp->disabledList.size();j++) {
      if ( (hp->disabledList[i]).getPointer() == 
           (hp->disabledList[j]).getPointer() ) {
        sprintf(errorLine,"***** ERROR: address 0x%x appears more than once\n",(hp->disabledList[j]).getPointer());
        logLine(errorLine);
      }
    }
  }
#endif

#ifdef FREEDEBUG
  logLine("***** Compact memory procedure END...\n");
#endif
}

void inferiorFreeDefered(process *proc, inferiorHeap *hp, bool runOutOfMem)
{
  unsigned int i=0;
  vector<Address> pcs;
  vector<disabledItem> *disList;
  timeStamp initTime, maxDelTime;

  pcs = proc->walkStack();

  // this is a while loop since we don't update i if an item is deleted.
  disList = &hp->disabledList;
  if (runOutOfMem) {
    maxDelTime = MAX_DELETING_TIME*2.0;
    sprintf(errorLine,"Emergency attempt to free memory (pid=%d). Please, wait...\n",proc->getPid());
    logLine(errorLine);
#ifdef FREEDEBUG1
    sprintf(errorLine,"***** disList.size() = %d\n",disList->size());
    logLine(errorLine);
#endif
  }
  else
    maxDelTime = MAX_DELETING_TIME;
  initTime=getCurrentTime(false);
  while ( (i < disList->size()) && 
          ((getCurrentTime(false)-initTime) < maxDelTime) )
  {
    disabledItem &item = (*disList)[i];
    if (isFreeOK(proc,item,pcs)) {
      heapItem *np=NULL;
      unsigned pointer = item.getPointer();
      if (!hp->heapActive.find(pointer,np)) {
        showErrorCallback(96,"");
        return;
      }
      assert(np);

      if (np->status != HEAPallocated) {
        sprintf(errorLine,"Attempt to free already freed heap entry %x\n", pointer);
        logLine(errorLine);
        showErrorCallback(67, (const char *)errorLine); 
        return;
      }
      np->status = HEAPfree;      

      // remove from active list.
      hp->heapActive.undef(pointer);

#ifdef FREEDEBUG_ON
   sprintf(errorLine,"inferiorFreeDefered: deleting 0x%x from heap\n",pointer);
   logLine(errorLine);
#endif

      hp->heapFree += np;
      hp->freed += np->length;

      // updating disabledList
      hp->disabledList[i]=hp->disabledList[hp->disabledList.size()-1];
      hp->disabledList.resize(hp->disabledList.size()-1);
      hp->disabledListTotalMem -= np->length;
      hp->totalFreeMemAvailable += np->length;
    } else {
      i++;
    }
  }
}

void process::initInferiorHeap(bool initTextHeap)
{
    assert(this->symbols);

    inferiorHeap *hp;
    if (initTextHeap) {
	hp = &this->heaps[textHeap];
    } else {
	hp = &this->heaps[dataHeap];
    }

    heapItem *np = new heapItem;
    if (initTextHeap) {
        bool err;
	np->addr = symbols->findInternalAddress("DYNINSTtext", true,err);
	if (err)
	  abort();
    } else {
        bool err;
	np->addr = symbols->findInternalAddress(INFERIOR_HEAP_BASE, true, err);
	if (err)
	  abort();
    }
    np->length = SYN_INST_BUF_SIZE;
    np->status = HEAPfree;

    // make the heap double-word aligned
    Address base = np->addr & ~0x1f;
    Address diff = np->addr - base;
    if (diff) {
      np->addr = base + 32;
      np->length -= (32 - diff);
    }

    hp->totalFreeMemAvailable = np->length;
    // need to clear everything here, since this function may be called to re-init a heap
    hp->heapActive.clear();
    hp->heapFree.resize(0);
    hp->heapFree += np;
    hp->disabledList.resize(0);
    hp->disabledListTotalMem = 0;
    hp->freed = 0;
}

// create a new inferior heap that is a copy of src. This is used when a process
// we are tracing forks.
inferiorHeap::inferiorHeap(const inferiorHeap &src):
    heapActive(addrHash16)
{
    for (unsigned u1 = 0; u1 < src.heapFree.size(); u1++) {
      heapFree += new heapItem(src.heapFree[u1]);
    }

    vector<heapItem *> items = src.heapActive.values();
    for (unsigned u2 = 0; u2 < items.size(); u2++) {
      heapActive[items[u2]->addr] = new heapItem(items[u2]);
    }
    
    for (unsigned u3 = 0; u3 < src.disabledList.size(); u3++) {
      disabledList += src.disabledList[u3];
    }
    disabledListTotalMem = src.disabledListTotalMem;
    totalFreeMemAvailable = src.totalFreeMemAvailable;
    freed = 0;
}

#ifdef FREEDEBUG
void printHeapFree(process *proc, inferiorHeap *hp, int size)
{
  for (unsigned i=0; i < hp->heapFree.size(); i++) {
    sprintf(errorLine,"***** (pid=%d) i=%d, addr=%d, length=%d, heapFree.size()=%d, size=%d\n",proc->getPid(),i,(hp->heapFree[i])->addr,(hp->heapFree[i])->length,hp->heapFree.size(),size); 
    logLine(errorLine);
  }
}
#endif

//
// This function will return the index corresponding to the next position
// available in heapFree.
//
bool findFreeIndex(inferiorHeap *hp, int size, unsigned *index)
{
  bool foundFree=false;
  unsigned best=0;
  int length=0;
  for (unsigned i=0; i < hp->heapFree.size(); i++) {
    length = (hp->heapFree[i])->length;
    if (length >= size) {
      foundFree = true;
      best=i;
      break;
    }
  }
  if (index) 
    *index = best;
  else
    foundFree=false;
  return(foundFree);
}  

unsigned inferiorMalloc(process *proc, int size, inferiorHeapType type)
{
    inferiorHeap *hp;
    heapItem *np=NULL, *newEntry = NULL;
    
    assert(size > 0);
    /* round to next cache line size */
    /* 32 bytes on a SPARC */
    size = (size + 0x1f) & ~0x1f; 

    if ((type == textHeap) && (proc->splitHeaps)) {
	hp = &proc->heaps[textHeap];
    } else {
	hp = &proc->heaps[dataHeap];
    }

    bool secondChance=false;
    unsigned foundIndex;
    if (!findFreeIndex(hp,size,&foundIndex)) {

#ifdef FREEDEBUG
      sprintf(errorLine,"==> TEST <== In inferiorMalloc: heap overflow, calling inferiorFreeDefered for a second chance...\n");
      logLine(errorLine);
#endif

      inferiorFreeDefered(proc, hp, true);
      inferiorFreeCompact(hp);
      secondChance=true;
    }

    if (secondChance && !findFreeIndex(hp,size,&foundIndex)) {

#ifdef FREEDEBUG
      printHeapFree(proc,hp,size);
#endif

      sprintf(errorLine, "***** Inferior heap overflow: %d bytes freed, %d bytes requested\n", hp->freed, size);
      logLine(errorLine);
      showErrorCallback(66, (const char *) errorLine);
      P__exit(-1);
      //return(0);
    }

    //
    // We must have found a free position in heapFree
    //

    np = hp->heapFree[foundIndex];
    assert(np);
    if (np->length != size) {
	// divide it up.
	newEntry = new heapItem;
	newEntry->length = np->length - size;
	newEntry->addr = np->addr + size;
        hp->totalFreeMemAvailable -= size;
	// overwrite the old entry
	hp->heapFree[foundIndex] = newEntry;

	/* now split curr */
	np->length = size;
    } else {
      unsigned i = hp->heapFree.size();
      // copy the last element over this element
      hp->heapFree[foundIndex] = hp->heapFree[i-1];
      hp->heapFree.resize(i-1);
    }

    // mark it used
    np->status = HEAPallocated;

    // onto in use list
    hp->heapActive[np->addr] = np;

    // make sure its a valid pointer.
    assert(np->addr);

    return(np->addr);
}

void inferiorFree(process *proc, unsigned pointer, inferiorHeapType type,
                  const vector<unsigVecType> &pointsToCheck)
{
    inferiorHeapType which = (type == textHeap && proc->splitHeaps) ? textHeap : dataHeap;
    inferiorHeap *hp = &proc->heaps[which];
    heapItem *np=NULL;

    if (!hp->heapActive.find(pointer, np)) {
      showErrorCallback(96,"");
      return;
    }
    assert(np);

    disabledItem newItem(pointer, which, pointsToCheck);

#ifdef FREEDEBUG
    for (unsigned i=0;i<hp->disabledList.size();i++) {
      if (hp->disabledList[i].getPointer() == pointer) {
        sprintf(errorLine,"***** ERROR, pointer 0x%x already defined in disabledList\n",pointer);
        logLine(errorLine);
      }
    }
#endif

    hp->disabledList += newItem;
    hp->disabledListTotalMem += np->length;

#ifdef FREEDEBUG1
    sprintf(errorLine,"==> TEST <== In inferiorFree: disabledList has %d items, %d bytes, and FREE_WATERMARK is %d\n",hp->disabledList.size(),hp->disabledListTotalMem,FREE_WATERMARK);
    logLine(errorLine);
    for (unsigned i=0; i < hp->disabledList.size(); i++) {
	sprintf(errorLine, "   %x is on list\n", hp->disabledList[i].pointer);
	logLine(errorLine);
    }
#endif

    //
    // If the size of the disabled instrumentation list is greater than
    // half of the size of the heapFree list, then we attempt to free
    // all the defered requests. In my opinion, this seems to be a good
    // time to proceed with the defered delete since this is an expensive
    // procedure and should not be executed often - naim 03/19/96
    //
    if (((hp->disabledListTotalMem > FREE_WATERMARK) ||
         (hp->disabledList.size() > SIZE_WATERMARK)) && waitingPeriodIsOver()) 
    {

#ifdef FREEDEBUG
timeStamp t1,t2;
static timeStamp t3=0.0;
static timeStamp totalTime=0.0;
static timeStamp worst=0.0;
static int counter=0;
t1=getCurrentTime(false);
#endif

      inferiorFreeDefered(proc, hp, false);

#ifdef FREEDEBUG
t2=getCurrentTime(false);
if (!t3) t3=t1;
counter++;
totalTime += t2-t1;
if ((t2-t1) > worst) worst=t2-t1;
if ((float)(t2-t1) > 1.0) {
  sprintf(errorLine,">>>> TEST <<<< (pid=%d) inferiorFreeDefered took %5.2f secs, avg=%5.2f, worst=%5.2f, heapFree=%d, heapActive=%d, disabledList=%d, last call=%5.2f\n", proc->getPid(),(float) (t2-t1), (float) (totalTime/counter), (float)worst, hp->heapFree.size(), hp->heapActive.size(), hp->disabledList.size(), (float)(t1-t3));
  logLine(errorLine);
}
t3=t1;
#endif

    }
}

process::process(int iPid, image *iImage, int iTraceLink, int iIoLink
#ifdef SHM_SAMPLING
		 , key_t theShmKey,
		 const vector<fastInferiorHeapMgr::oneHeapStats> &iShmHeapStats
#endif
) :
             baseMap(ipHash), 
	     pid(iPid) // needed in fastInferiorHeap ctors below
#ifdef SHM_SAMPLING
	     ,inferiorHeapMgr(theShmKey, iShmHeapStats, iPid),
	     inferiorIntCounters((intCounter*)inferiorHeapMgr.getSubHeapInParadynd(0),
				 this, iShmHeapStats[0].maxNumElems),
	     inferiorWallTimers ((tTimer*)inferiorHeapMgr.getSubHeapInParadynd(1),
				 this, iShmHeapStats[1].maxNumElems),
	     inferiorProcessTimers((tTimer*)inferiorHeapMgr.getSubHeapInParadynd(2),
				   this, iShmHeapStats[2].maxNumElems)
#endif
{
    // this is the 'normal' ctor, when a proc is started fresh as opposed
    // to via a fork().  (What about when a process is started via exec()?)

    hasBootstrapped = false;
    reachedFirstBreak = false; // haven't yet seen first trap

    symbols = iImage;

    status_ = neonatal;
    continueAfterNextStop_ = false;

    string buffer = string(pid) + string("_") + getHostName();
    rid = resource::newResource(processResource, // parent
				(void*)this, // handle
				nullString, // abstraction
				iImage->name(), // process name
				0.0, // creation time
				buffer, // unique name (?)
				MDL_T_STRING // mdl type (?)
				);

    parent = NULL;
    bufStart = 0;
    bufEnd = 0;
    inExec = false;

    cumObsCost = 0;
    lastObsCostLow = 0;

    proc_fd = -1;

    trampTableItems = 0;
    memset(trampTable, 0, sizeof(trampTable));
    currentPC_ = 0;
    hasNewPC = false;
    
    dynamiclinking = false;
    dyn = new dynamic_linking;
    shared_objects = 0;
    all_functions = 0;
    all_modules = 0;
    some_modules = 0;
    some_functions = 0;
    waiting_for_resources = false;
    signal_handler = 0;
    execed_ = false;

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

   initInferiorHeap(false);
   splitHeaps = false;

#if defined(rs6000_ibm_aix3_2) || defined(rs6000_ibm_aix4_1)
	// XXXX - move this to a machine dependant place.

	// create a seperate text heap.
	initInferiorHeap(true);
	splitHeaps = true;
#endif

   traceLink = iTraceLink;
   ioLink = iIoLink;

   // attach to the child process (machine-specific implementation)
   attach(); // error check?
}

// the following is the 'attach' ctor:
process::process(int iPid, image *iSymbols
#ifdef SHM_SAMPLING
		 , key_t theShmKey,
		 const vector<fastInferiorHeapMgr::oneHeapStats> &iShmHeapStats
#endif
		 ) :
		 baseMap(ipHash),
		 pid(iPid)
#ifdef SHM_SAMPLING
	     ,inferiorHeapMgr(theShmKey, iShmHeapStats, iPid),
	     inferiorIntCounters((intCounter*)inferiorHeapMgr.getSubHeapInParadynd(0),
				 this, iShmHeapStats[0].maxNumElems),
	     inferiorWallTimers ((tTimer*)inferiorHeapMgr.getSubHeapInParadynd(1),
				 this, iShmHeapStats[1].maxNumElems),
	     inferiorProcessTimers((tTimer*)inferiorHeapMgr.getSubHeapInParadynd(2),
				   this, iShmHeapStats[2].maxNumElems)
#endif
{
   // this is the "attach" ctor.  New with release 1.2

   hasBootstrapped = false;
   reachedFirstBreak = true; // the initial trap of program entry was passed long ago...

   symbols = iSymbols;

   status_ = neonatal;
   continueAfterNextStop_ = false;

    string buffer = string(pid) + string("_") + getHostName();
    rid = resource::newResource(processResource, // parent
				(void*)this, // handle
				nullString, // abstraction
				symbols->name(),
				0.0, // creation time
				buffer, // unique name (?)
				MDL_T_STRING // mdl type (?)
				);

    parent = NULL;
    bufStart = 0;
    bufEnd = 0;
    inExec = false;

    cumObsCost = 0;
    lastObsCostLow = 0;

    proc_fd = -1;

    trampTableItems = 0;
    memset(trampTable, 0, sizeof(trampTable));
    currentPC_ = 0;
    hasNewPC = false;
    
    dynamiclinking = false;
    dyn = new dynamic_linking;
    shared_objects = 0;
    all_functions = 0;
    all_modules = 0;
    some_modules = 0;
    some_functions = 0;
    waiting_for_resources = false;
    signal_handler = 0;
    execed_ = false;

   initInferiorHeap(false);
   splitHeaps = false;

#if defined(rs6000_ibm_aix3_2) || defined(rs6000_ibm_aix4_1)
	// XXXX - move this to a machine dependant place.

	// create a seperate text heap.
	initInferiorHeap(true);
	splitHeaps = true;
#endif

   traceLink = -1; // will be set later, when the appl runs DYNINSTinit

   ioLink = -1; // (ARGUABLY) NOT YET IMPLEMENTED...MAYBE WHEN WE ATTACH WE DON'T WANT
                // TO REDIRECT STDIO SO WE CAN LEAVE IT AT -1.

   // Now the actual attach...the moment we've all been waiting for

   attach_cerr << "process attach ctor: about to attach to pid " << getPid() << endl;

   // The following should set up the signals that we want to get forwarded to paradynd.
   // Does it?
   if (!attach()) {
      showErrorCallback(26, ""); // unable-to-attach
      return;
   }

   // Does attach() have the side effect of pausing the program?
   // (on solaris, it seems that it doesn't (!))
   // If not, we should probably explicitly pause it now.
   status_ = running;
      // may not be accurate, but at least it ensures that pause() tries to pause.

   if (!pause())
      assert(false);

   assert(status_ == stopped);

   // Does attach() send a SIGTRAP, a la the initial SIGTRAP sent at the
   // end of exec?  It seems that on some platforms it does; on others
   // it doesn't.  Ick.  On solaris, I don't think it sends a SIGTRAP.

   // note: we don't call getSharedObjects() yet; that happens once DYNINSTinit
   //       finishes (handleStartProcess)
}

// This is the "fork" constructor:
process::process(const process &parentProc, int iPid, int iTrace_fd
#ifdef SHM_SAMPLING
		 ,key_t theShmKey,
		 void *applShmSegPtr,
		 const vector<fastInferiorHeapMgr::oneHeapStats> &iShmHeapStats
#endif
		 ) :
                     baseMap(ipHash) // could change to baseMap(parentProc.baseMap)
#ifdef SHM_SAMPLING
		     ,inferiorHeapMgr(parentProc.inferiorHeapMgr, applShmSegPtr,
				      theShmKey, iShmHeapStats, iPid),
		     inferiorIntCounters(parentProc.inferiorIntCounters,
					 this,
					 inferiorHeapMgr.getSubHeapInParadynd(0),
					 inferiorHeapMgr.getSubHeapInApplic(0)),
		     inferiorWallTimers(parentProc.inferiorWallTimers,
					this,
					inferiorHeapMgr.getSubHeapInParadynd(1),
					inferiorHeapMgr.getSubHeapInApplic(1)),
		     inferiorProcessTimers(parentProc.inferiorProcessTimers,
					   this,
					   inferiorHeapMgr.getSubHeapInParadynd(2),
					   inferiorHeapMgr.getSubHeapInApplic(2))
#endif
{
    // This is the "fork" ctor

    hasBootstrapped = false;
       // The child of fork ("this") has yet to run DYNINSTinit.

    symbols = parentProc.symbols; // shouldn't a reference count also be bumped?

    traceLink = iTrace_fd;

    ioLink = -1; // when does this get set?

    status_ = neonatal; // is neonatal right?
    continueAfterNextStop_ = false;

    pid = iPid; 

    string buffer = string(pid) + string("_") + getHostName();
    rid = resource::newResource(processResource, // parent
				(void*)this, // handle
				nullString, // abstraction
				parentProc.symbols->name(),
				0.0, // creation time
				buffer, // unique name (?)
				MDL_T_STRING // mdl type (?)
				);

    parent = &parentProc;
    bufStart = 0;
    bufEnd = 0;

    reachedFirstBreak = true; // initial TRAP has (long since) been reached

    splitHeaps = parentProc.splitHeaps;

    heaps[0] = inferiorHeap(parentProc.heaps[0]);
    heaps[1] = inferiorHeap(parentProc.heaps[1]);

    inExec = false;

    cumObsCost = 0;
    lastObsCostLow = 0;

    proc_fd = -1;

    trampTableItems = 0;
    memset(trampTable, 0, sizeof(trampTable));
    currentPC_ = 0;
    hasNewPC = false;

    dynamiclinking = parentProc.dynamiclinking;
    dyn = new dynamic_linking;
    *dyn = *parentProc.dyn;

    shared_objects = 0;

    // make copy of parent's shared_objects vector
    if (parentProc.shared_objects) {
      shared_objects = new vector<shared_object*>;
      for (unsigned u1 = 0; u1 < parentProc.shared_objects->size(); u1++){
	*shared_objects += 
		new shared_object(*(*parentProc.shared_objects)[u1]);
      }
    }

    all_functions = 0;
    if (parentProc.all_functions) {
      all_functions = new vector<pdFunction *>;
      for (unsigned u2 = 0; u2 < parentProc.all_functions->size(); u2++)
	*all_functions += (*parentProc.all_functions)[u2];
    }

    all_modules = 0;
    if (parentProc.all_modules) {
      all_modules = new vector<module *>;
      for (unsigned u3 = 0; u3 < parentProc.all_modules->size(); u3++)
	*all_modules += (*parentProc.all_modules)[u3];
    }

    some_modules = 0;
    if (parentProc.some_modules) {
      some_modules = new vector<module *>;
      for (unsigned u4 = 0; u4 < parentProc.some_modules->size(); u4++)
	*some_modules += (*parentProc.some_modules)[u4];
    }
    
    some_functions = 0;
    if (parentProc.some_functions) {
      some_functions = new vector<pdFunction *>;
      for (unsigned u5 = 0; u5 < parentProc.some_functions->size(); u5++)
	*some_functions += (*parentProc.some_functions)[u5];
    }

    waiting_for_resources = false;
    signal_handler = parentProc.signal_handler;
    execed_ = false;

#ifdef SHM_SAMPLING
#ifdef sparc_sun_sunos4_1_3
   childUareaPtr = NULL;
#endif
#endif

   if (!attach()) {     // moved from ::forkProcess
      showErrorCallback(69, "Error in fork: cannot attach to child process");
   }

   status_ = stopped;
      // would neonatal be more appropriate?  Nah, we've reached the first trap
}

#ifdef SHM_SAMPLING
void process::registerInferiorAttachedSegs(void *inferiorAttachedAtPtr) {
   shmsample_cerr << "process pid " << getPid() << ": welcome to register with inferiorAttachedAtPtr=" << inferiorAttachedAtPtr << endl;

   inferiorHeapMgr.registerInferiorAttachedAt(inferiorAttachedAtPtr);
   inferiorIntCounters.setBaseAddrInApplic((intCounter*)
					   inferiorHeapMgr.getSubHeapInApplic(0));
   inferiorWallTimers.setBaseAddrInApplic((tTimer*)
					  inferiorHeapMgr.getSubHeapInApplic(1));
   inferiorProcessTimers.setBaseAddrInApplic((tTimer*)
					     inferiorHeapMgr.getSubHeapInApplic(2));
}
#endif

/*
 * Create a new instance of the named process.  Read the symbols and start
 *   the program
 */
process *createProcess(const string File, vector<string> argv, vector<string> envp, const string dir = "")
{
    // prepend the directory (if any) to the file, unless the filename
    // starts with a /
    string file = File;
    if (!file.prefixed_by("/") && dir.length() > 0)
      file = dir + "/" + file;

    // check for I/O redirection in arg list.
    string inputFile;
    for (unsigned i=0; i<argv.size(); i++) {
      if (argv[i] == "<") {
	inputFile = argv[i+1];
	for (unsigned j=i+2, k=i; j<argv.size(); j++, k++)
	  argv[k] = argv[j];
	argv.resize(argv.size()-2);
      }
    }
    // TODO -- this assumes no more than 1 of each "<", ">"
    string outputFile;
    for (unsigned i=0; i<argv.size(); i++) {
      if (argv[i] == ">") {
	outputFile = argv[i+1];
	for (unsigned j=i+2, k=i; j<argv.size(); j++, k++)
	  argv[k] = argv[j];
	argv.resize(argv.size()-2);
      }
    }

    // Strange, but using socketpair here doesn't seem to work OK on SunOS.
    // Pipe works fine.
    // r = P_socketpair(AF_UNIX, SOCK_STREAM, (int) NULL, tracePipe);
    int tracePipe[2];
    int r = P_pipe(tracePipe);
    if (r) {
	// P_perror("socketpair");
        string msg = string("Unable to create trace pipe for program '") + File +
	               string("': ") + string(sys_errlist[errno]);
	showErrorCallback(68, msg);
	return(NULL);
    }

    // ioPipe is used to redirect the child's stdout & stderr to a pipe which is in
    // turn read by the parent via the process->ioLink socket.
    int ioPipe[2];

    // r = P_socketpair(AF_UNIX, SOCK_STREAM, (int) NULL, ioPipe);
    r = P_pipe(ioPipe);
    if (r) {
	// P_perror("socketpair");
        string msg = string("Unable to create IO pipe for program '") + File +
	               string("': ") + string(sys_errlist[errno]);
	showErrorCallback(68, msg);
	return(NULL);
    }

    //
    // WARNING This code assumes that vfork is used, and a failed exec will
    //   corectly change failed in the parent process.
    //
    errno = 0;
#ifdef PARADYND_PVM
// must use fork, since pvmendtask will do some writing in the address space
    int pid = fork();
    // fprintf(stderr, "FORK: pid=%d\n", pid);
#else
    int pid = vfork();
#endif
    if (pid > 0) {
	if (errno) {
	    sprintf(errorLine, "Unable to start %s: %s\n", file.string_of(), sys_errlist[errno]);
	    logLine(errorLine);
	    showErrorCallback(68, (const char *) errorLine);
	    return(NULL);
	}

#if defined(rs6000_ibm_aix3_2) || defined(rs6000_ibm_aix4_1)
	extern bool establishBaseAddrs(int pid, int &status, bool waitForTrap);
	int status;

	if (!establishBaseAddrs(pid, status, true)) {
	    return(NULL);
	}
#endif

// NEW: We bump up batch mode here; the matching bump-down occurs after shared objects
//      are processed (after receiving the SIGSTOP indicating the end of running
//      DYNINSTinit; more specifically, procStopFromDYNINSTinit().
//      Prevents a diabolical w/w deadlock on solaris --ari
tp->resourceBatchMode(true);

	image *img = image::parseImage(file);
	if (!img) {
	    // For better error reporting, two failure return values would be useful
	    // One for simple error like because-file-not-because
	    // Another for serious errors like found-but-parsing-failed (internal error;
	    //    please report to paradyn@cs.wisc.edu)

	    string msg = string("Unable to parse image: ") + file;
	    showErrorCallback(68, msg.string_of());
	    // destroy child process
	    P_kill(pid, 9);

	    return(NULL);
	}

	/* parent */
	statusLine("initializing process data structures");

#ifdef SHM_SAMPLING
	vector<fastInferiorHeapMgr::oneHeapStats> theShmHeapStats(3);
	theShmHeapStats[0].elemNumBytes = sizeof(intCounter);
	theShmHeapStats[0].maxNumElems  = numIntCounters;

	theShmHeapStats[1].elemNumBytes = sizeof(tTimer);
	theShmHeapStats[1].maxNumElems  = numWallTimers;

	theShmHeapStats[2].elemNumBytes = sizeof(tTimer);
	theShmHeapStats[2].maxNumElems  = numProcTimers;
#endif

	process *ret = new process(pid, img,
				   tracePipe[0], // trace link
				   ioPipe[0] // io link
#ifdef SHM_SAMPLING
				   , 7000, // shm seg key to try first
				   theShmHeapStats
#endif
				   );
	   // change this to a ctor that takes in more args

	assert(ret);

	processVec += ret;
	activeProcesses++;

	if (!costMetric::addProcessToAll(ret))
	   assert(false);

        // find the signal handler function
	ret->findSignalHandler(); // should this be in the ctor?

	close(tracePipe[1]);
	   // parent never writes trace records; it only receives them.

	close(ioPipe[1]);
           // parent closes write end of io pipe; child closes its read end.
           // pipe output goes to the parent's read fd (ret->ioLink); pipe input
           // comes from the child's write fd.  In short, when the child writes to
           // its stdout/stderr, it gets sent to the pipe which in turn sends it to
           // the parent's ret->ioLink fd for reading.

#if defined(rs6000_ibm_aix3_2) || defined(rs6000_ibm_aix4_1)
	// XXXX - this is a hack since establishBaseAddrs needed to wait for
	//    the TRAP signal.
	// We really need to move most of the above code (esp parse image)
	//    to the TRAP signal handler.  The problem is that we don't
	//    know the base addresses until we get the load info via ptrace.
	//    In general it is even harder, since dynamic libs can be loaded
	//    at any time.
	extern int handleSigChild(int pid, int status);

	(void) handleSigChild(pid, status);
#endif

	return(ret);
    } else if (pid == 0) {
#ifdef PARADYND_PVM
	if (pvm_running)
	  pvmendtask(); 
#endif   

	// handle stdio.

        // We only write to ioPipe.  Hence we close ioPipe[0], the read end.  Then we
        // call dup2() twice to assign our stdout and stderr to the write end of the
	// pipe.
	close(ioPipe[0]);
	dup2(ioPipe[1], 1);
           // assigns fd 1 (stdout) to be a copy of ioPipe[1].  (Since stdout is already
           // in use, dup2 will first close it then reopen it with the characteristics
	   // of ioPipe[1].)
           // In short, stdout gets redirected towards the write end of the pipe.
           // The read end of the pipe is read by the parent (paradynd), not by us.

	dup2(ioPipe[1], 2); // redirect fd 2 (stderr) to the pipe, like above.

        // We're not using ioPipe[1] anymore; close it.
	if (ioPipe[1] > 2) close (ioPipe[1]);

	// Now that stdout is going to a pipe, it'll (unfortunately) be block buffered
        // instead of the usual line buffered (when it goes to a tty).  In effect the
        // stdio library is being a little too clever for our purposes.  We don't want
        // the "bufferedness" to change.  So we set it back to line-buffered.
        // The command to do this is setlinebuf(stdout) [stdio.h call]  But we don't
        // do it here, since the upcoming execve() would undo our work [execve keeps
        // fd's but resets higher-level stdio information, which is recreated before
        // execution of main()]  So when do we do it?  In rtinst's DYNINSTinit
        // (RTposix.c et al.)

	// setup stderr for rest of exec try.
	FILE *childError = P_fdopen(2, "w");

	P_close(tracePipe[0]);

	if (P_dup2(tracePipe[1], 3) != 3) {
	    fprintf(childError, "dup2 failed\n");
	    fflush(childError);
	    P__exit(-1);
	}

	/* close if higher */
	if (tracePipe[1] > 3) close(tracePipe[1]);

	if ((dir.length() > 0) && (P_chdir(dir.string_of()) < 0)) {
	  sprintf(errorLine, "cannot chdir to '%s': %s\n", dir.string_of(), sys_errlist[errno]);
	  logLine(errorLine);
	  P__exit(-1);
	}

	/* see if I/O needs to be redirected */
	if (inputFile.length()) {
	    int fd = P_open(inputFile.string_of(), O_RDONLY, 0);
	    if (fd < 0) {
		fprintf(childError, "stdin open of %s failed\n", inputFile.string_of());
		fflush(childError);
		P__exit(-1);
	    } else {
		dup2(fd, 0);
		P_close(fd);
	    }
	}

	if (outputFile.length()) {
	    int fd = P_open(outputFile.string_of(), O_WRONLY|O_CREAT, 0444);
	    if (fd < 0) {
		fprintf(childError, "stdout open of %s failed\n", outputFile.string_of());
		fflush(childError);
		P__exit(-1);
	    } else {
		dup2(fd, 1); // redirect fd 1 (stdout) to a copy of descriptor "fd"
		P_close(fd); // not using descriptor fd any more; close it.
	    }
	}

	/* indicate our desire to be traced */
	errno = 0;
	OS::osTraceMe();
	if (errno != 0) {
	  sprintf(errorLine, "ptrace error, exiting, errno=%d\n", errno);
	  logLine(errorLine);
	  logLine(sys_errlist[errno]);
	  showErrorCallback(69, string("Internal error: ") + 
	                        string((const char *) errorLine)); 
	  P__exit(-1);   // double underscores are correct
	}
#ifdef PARADYND_PVM
	if (pvm_running && envp.size())
	  for (int ep=envp.size()-1; ep>=0; ep--) {
	    pvmputenv(envp[ep].string_of());
	  }
#endif
        // hand off info about how to start a paradynd to the application.
	//   used to catch rexec calls, and poe events.
	//
	char paradynInfo[1024];
	sprintf(paradynInfo, "PARADYN_MASTER_INFO= ");
	for (unsigned i=0; i < process::arg_list.size(); i++) {
	    const char *str;

	    str = P_strdup(process::arg_list[i].string_of());
	    if (!strcmp(str, "-l1")) {
		strcat(paradynInfo, "-l0");
	    } else {
		strcat(paradynInfo, str);
	    }
	    strcat(paradynInfo, " ");
	}
	P_putenv(paradynInfo);

	/* put the traceSocketPath in the environment variable PARADYND_TRACE_SOCKET
	   This will be use by forked processes to get a connection with the daemon
	*/
	string paradyndSockInfo = string("PARADYND_TRACE_SOCKET=") + string(traceSocketPath);

	P_putenv(paradyndSockInfo.string_of());

	char **args;
	args = new char*[argv.size()+1];
	for (unsigned ai=0; ai<argv.size(); ai++)
	  args[ai] = P_strdup(argv[ai].string_of());
	args[argv.size()] = NULL;
	P_execvp(file.string_of(), args);

	sprintf(errorLine, "paradynd: execv failed, errno=%d\n", errno);
	logLine(errorLine);

	logLine(sys_errlist[errno]);
	int i=0;
	while (args[i]) {
	  sprintf(errorLine, "argv %d = %s\n", i, args[i]);
	  logLine(errorLine);
	  i++;
	}
	P__exit(-1);
	return(NULL);
    } else {
	sprintf(errorLine, "vfork failed, errno=%d\n", errno);
	logLine(errorLine);
	showErrorCallback(71, (const char *) errorLine);
	return(NULL);
    }
}

bool attachProcess(const string &dir, const string &cmd, int pid) {
   // implementation of dynRPC::attach() (the igen call)
   // This is meant to be "the other way" to start a process (competes w/ createProcess)
   // dir + cmd give us the disk location of the executable, which we use ONLY to
   // read the symbol table.

   attach_cerr << "welcome to attachProcess for pid " << pid << endl;

   // QUESTION: When we attach to a process, do we want to redirect its stdout/stderr
   //           (like we do when we fork off a new process the 'usual' way)?
   //           My first guess would be no.  -ari
   //           But although we may ignore the io, we still need the trace stream.

   // When we attach to a process, we don't fork...so this routine is much simpler
   // than its "competitor", createProcess() (above).

   // TODO: What about AIX establishBaseAddrs???  Do that now?

   string fileName = "";
   if (dir.length() > 0) {
      fileName += dir;
      if (!cmd.prefixed_by("/"))
	 fileName += "/";
   }
   fileName += cmd;

   tp->resourceBatchMode(true);
      // matching bump-down occurs in procStopFromDYNINSTinit().

   image *theImage = image::parseImage(fileName);
   if (theImage == NULL) {
      // two failure return values would be useful here, to differentiate
      // file-not-found vs. catastrophic-parse-error.
      string msg = string("Unable to parse image: ") + fileName;
      showErrorCallback(68, msg.string_of());
      return false; // failure
   }

#ifdef SHM_SAMPLING
   vector<fastInferiorHeapMgr::oneHeapStats> theShmHeapStats(3);
   theShmHeapStats[0].elemNumBytes = sizeof(intCounter);
   theShmHeapStats[0].maxNumElems  = numIntCounters;

   theShmHeapStats[1].elemNumBytes = sizeof(tTimer);
   theShmHeapStats[1].maxNumElems  = numWallTimers;

   theShmHeapStats[2].elemNumBytes = sizeof(tTimer);
   theShmHeapStats[2].maxNumElems  = numProcTimers;
#endif

   // NOTE: the actual attach happens in the process "attach" constructor:
   process *theProc = new process(pid, theImage
#ifdef SHM_SAMPLING
				  ,7000, // shm seg key to try first
				  theShmHeapStats
#endif				  
				  );
   assert(theProc);

   // the attach ctor should have left the process in a paused state.
   assert(theProc->status() == stopped);

   processVec += theProc;
   activeProcesses++;

   if (!costMetric::addProcessToAll(theProc))
      assert(false);

   // find the signal handler function
   theProc->findSignalHandler(); // shouldn't this be in the ctor?

   // Now force DYNINSTinit() to be invoked, via inferiorRPC.
   string buffer = string("PID=") + string(pid) + ", running DYNINSTinit()...";
   statusLine(buffer.string_of());

   attach_cerr << "calling DYNINSTinit with args:" << endl;

   vector<AstNode*> the_args(3);

#ifdef SHM_SAMPLING
   the_args[0] = new AstNode(AstNode::Constant,
			     (void*)(theProc->getShmKeyUsed()));
   attach_cerr << theProc->getShmKeyUsed() << endl;

   const unsigned shmHeapTotalNumBytes = theProc->getShmHeapTotalNumBytes();
   the_args[1] = new AstNode(AstNode::Constant,
			     (void*)shmHeapTotalNumBytes);
   attach_cerr << shmHeapTotalNumBytes << endl;;
#else
   // 2 dummy args when not shm sampling -- just make sure they're not both -1
   the_args[0] = new AstNode(AstNode::Constant, (void*)0);
   the_args[1] = new AstNode(AstNode::Constant, (void*)0);
#endif

   /*
      the third argument to DYNINSTinit is the our own pid. It is used
      by DYNINSTinit to build the socket path to which it connects to.
      This socket is set up in controllerMainLoop (perfStream.C).
   */
   the_args[2] = new AstNode(AstNode::Constant, (void*)getpid());
   attach_cerr << getpid() << endl;

   AstNode *the_ast = new AstNode("DYNINSTinit", the_args);
   for (unsigned i=0;i<the_args.size();i++) removeAst(the_args[i]);

   theProc->postRPCtoDo(the_ast,
			true, // true --> don't try to update cost yet
			NULL, // no callback routine needed
			NULL // user data
			);
      // the rpc will be launched with a call to launchRPCifAppropriate()
      // in the main loop (perfStream.C).
      // DYNINSTinit() ends with a DYNINSTbreakPoint(), so we pick up
      // where we left off in the processing of the forwarded SIGSTOP signal.
      // In other words, there's lots more work to do, but since we can't do it until
      // DYNINSTinit has run, we wait until the SIGSTOP is forwarded.

   return true; // successful
}

#ifdef SHM_SAMPLING
bool process::doMajorShmSample(unsigned long long theWallTime) {
   bool result = true; // will be set to false if any processAll() doesn't complete
                       // successfully.

   if (!inferiorIntCounters.doMajorSample  (theWallTime, 0))
      result = false;

   if (!inferiorWallTimers.doMajorSample   (theWallTime, 0))
      result = false;

   if (!inferiorProcessTimers.doMajorSample(theWallTime, 0))
      // inferiorProcessTimers used to take in a non-dummy process time as the
      // 2d arg, but it looks like that we need to re-read the process time for
      // each proc timer, at the time of sampling the timer's value, to avoid
      // ugly jagged spikes in histogram (i.e. to avoid incorrect sampled values).
      //
      // Come to think of it: the same may have to be done for the wall time too!!!
      result = false;

   const unsigned long long theProcTime = getInferiorProcessCPUtime();

   // Now sample the observed cost.
   unsigned *costAddr = this->getObsCostLowAddrInParadyndSpace();
   const unsigned theCost = *costAddr; // WARNING: shouldn't we be using a mutex?!

   this->processCost(theCost, theWallTime, theProcTime);

   return result;
}

bool process::doMinorShmSample() {
   // Returns true if the minor sample has successfully completed all outstanding
   // samplings.
   bool result = true; // so far...

   if (!inferiorIntCounters.doMinorSample())
      result = false;

   if (!inferiorWallTimers.doMinorSample())
      result = false;

   if (!inferiorProcessTimers.doMinorSample())
      result = false;

   return result;
}
#endif

extern void removeFromMetricInstances(process *);
extern void disableAllInternalMetrics();

void handleProcessExit(process *proc, int exitStatus) {
  if (proc->status() == exited)
    return;

  proc->Exited();
  if (proc->traceLink >= 0) {
    // We used to call processTraceStream(proc) here to soak up any last
    // messages but since it uses a blocking read, that doesn't seem like such
    // a good idea.
    //processTraceStream(proc);

    P_close(proc->traceLink);
    proc->traceLink = -1;
  }
  if (proc->ioLink >= 0) {
    // We used to call processAppIO(proc) here to soak up any last
    // messages but since it uses a blocking read, that doesn't seem like such
    // a good idea.
    //processAppIO(proc);

    P_close(proc->ioLink);
    proc->ioLink = -1;
  }
  removeFromMetricInstances(proc);
  --activeProcesses;
  if (activeProcesses == 0)
    disableAllInternalMetrics();

  proc->detach(false);
     // after this, the process will continue to run (presumably, just to complete
     // an exit())

#ifdef PARADYND_PVM
  if (pvm_running) {
    PDYN_reportSIGCHLD(proc->getPid(), exitStatus);
  }
#endif

  // Perhaps these lines can be un-commented out in the future, but since
  // cleanUpAndExit() does the same thing, and it always gets called
  // (when paradynd detects that paradyn died), it's not really necessary
  // here.  -ari
//  for (unsigned lcv=0; lcv < processVec.size(); lcv++)
//     if (processVec[lcv] == proc) {
//        delete proc; // destructor removes shm segments...
//	processVec[lcv] = NULL;
//     }
}


/*
   process::forkProcess: called when a process forks, to initialize a new
   process object for the child.

   the variable childHasInstrumentation is true if the child process has the 
   instrumentation of the parent. This is the common case.
   On some platforms (AIX) the child does not have any instrumentation because
   the text segment of the child is not a copy of the parent text segment at
   the time of the fork, but a copy of the original text segment of the parent,
   without any instrumentation.
   (actually, childHasInstr is obsoleted by aix's completeTheFork() routine)
*/
process *process::forkProcess(const process *theParent, pid_t childPid,
		      dictionary_hash<instInstance*,instInstance*> &map, // gets filled in
		      int iTrace_fd
#ifdef SHM_SAMPLING
			      ,key_t theKey,
			      void *applAttachedPtr
#endif
			      ) {
#ifdef SHM_SAMPLING
    vector<fastInferiorHeapMgr::oneHeapStats> theShmHeapStats(3);
    theShmHeapStats[0].elemNumBytes = sizeof(intCounter);
    theShmHeapStats[0].maxNumElems  = numIntCounters;
    
    theShmHeapStats[1].elemNumBytes = sizeof(tTimer);
    theShmHeapStats[1].maxNumElems  = numWallTimers;

    theShmHeapStats[2].elemNumBytes = sizeof(tTimer);
    theShmHeapStats[2].maxNumElems  = numProcTimers;
#endif

    forkexec_cerr << "paradynd welcome to process::forkProcess; parent pid=" << theParent->getPid() << "; calling fork ctor now" << endl;

    // Call the "fork" ctor:
    process *ret = new process(*theParent, childPid, iTrace_fd
#ifdef SHM_SAMPLING
			       , theKey,
			       applAttachedPtr,
			       theShmHeapStats
#endif
			       );
    assert(ret);

    forkexec_cerr << "paradynd fork ctor has completed ok...child pid is " << ret->getPid() << endl;

    processVec += ret;
    activeProcesses++;

    if (!costMetric::addProcessToAll(ret))
       assert(false);

    // We used to do a ret->attach() here...it was moved to the fork ctor, so it's
    // been done already.

    /* all instrumentation on the parent is active on the child */
    /* TODO: what about instrumentation inserted near the fork time??? */
    ret->baseMap = theParent->baseMap; // WHY IS THIS HERE?

    // the following writes to "map", s.t. for each instInstance in the parent
    // process, we have a map to the corresponding one in the child process.
    // that's all this routine does -- it doesn't actually touch
    // any instrumentation (because it doesn't need to -- fork() syscall copied
    // all of the actual instrumentation [but what about AIX and its weird load
    // behavior?])
    copyInstInstances(theParent, ret, map);
         // doesn't copy anything; just writes to "map"

    return ret;
}

#ifdef SHM_SAMPLING
void process::processCost(unsigned obsCostLow,
			  unsigned long long wallTime,
			  unsigned long long processTime) {
   // wallTime and processTime should compare to DYNINSTgetWallTime() and
   // DYNINSTgetCPUtime().

   // check for overflow, add to running total, convert cycles to
   // seconds, and report.
   // Member vrbles of class process: lastObsCostLow and cumObsCost (the latter
   // a 64-bit value).

   // code to handle overflow used to be in rtinst; we borrow it pretty much
   // verbatim. (see rtinst/RTposix.c)
   if (obsCostLow < lastObsCostLow) {
      // we have a wraparound
      cumObsCost += ((unsigned)0xffffffff - lastObsCostLow) + obsCostLow + 1;
   }
   else
      cumObsCost += (obsCostLow - lastObsCostLow);

   lastObsCostLow = obsCostLow;

   extern double cyclesPerSecond; // perfStream.C

   double observedCostSecs = cumObsCost;
   observedCostSecs /= cyclesPerSecond;
//   cerr << "processCost: cyclesPerSecond=" << cyclesPerSecond << "; cum obs cost=" << observedCostSecs << endl;

   // Notice how most of the rest of this is copied from processCost() of metric.C
   // Be sure to keep the two "in sync"!
   timeStamp newSampleTime  = (double)wallTime / 1000000.0; // usec to seconds
   timeStamp newProcessTime = (double)processTime / 1000000.0; // usec to secs

   extern costMetric *totalPredictedCost; // init.C
   extern costMetric *observed_cost; // init.C
   extern costMetric *smooth_obs_cost; // init.C

   const timeStamp lastProcessTime =
                        totalPredictedCost->getLastSampleProcessTime(this);

    // find the portion of uninstrumented time for this interval
    const double unInstTime = ((newProcessTime - lastProcessTime)
                         / (1+currentPredictedCost));
    // update predicted cost
    // note: currentPredictedCost is the same for all processes
    //       this should be changed to be computed on a per process basis
    sampleValue newPredCost = totalPredictedCost->getCumulativeValue(this);
    newPredCost += (float)(currentPredictedCost*unInstTime);

    totalPredictedCost->updateValue(this,newPredCost,
                                    newSampleTime,newProcessTime);
    // update observed cost
    observed_cost->updateValue(this,observedCostSecs,
                               newSampleTime,newProcessTime);

    // update smooth observed cost
    smooth_obs_cost->updateSmoothValue(this,observedCostSecs,
				       newSampleTime,newProcessTime);
}
#endif

/*
 * Copy data from controller process to the named process.
 */
bool process::writeDataSpace(void *inTracedProcess, int size,
			     const void *inSelf) {
  bool needToCont = false;

  if (status_ == exited)
    return false;

  if (status_ == running) {
    needToCont = true;
    if (! pause())
      return false;
  }

  if (status_ != stopped && status_ != neonatal) {
    showErrorCallback(38, "Internal paradynd error in process::writeDataSpace");
    return false;
  }

  bool res = writeDataSpace_(inTracedProcess, size, inSelf);
  if (!res) {
    string msg = string("System error: unable to write to process data space:")
	           + string(sys_errlist[errno]);
    showErrorCallback(38, msg);
    return false;
  }

  if (needToCont)
    return this->continueProc();
  return true;
}

bool process::readDataSpace(const void *inTracedProcess, int size,
			    void *inSelf, bool displayErrMsg) {
  bool needToCont = false;

  if (status_ == exited)
    return false;

  if (status_ == running) {
    needToCont = true;
    if (! pause())
      return false;
  }

  if (status_ != stopped && status_ != neonatal) {
    showErrorCallback(38, "Internal paradynd error in process::readDataSpace");
    return false;
  }

  bool res = readDataSpace_(inTracedProcess, size, inSelf);
  if (!res) {
    if (displayErrMsg) {
      string msg;
      msg=string("System error: unable to read from process data space:")
          + string(sys_errlist[errno]);
      showErrorCallback(38, msg);
    }
    return false;
  }

  if (needToCont)
    return this->continueProc();
  return true;

}

bool process::writeTextWord(caddr_t inTracedProcess, int data) {
  bool needToCont = false;

  if (status_ == exited)
    return false;

  if (status_ == running) {
    needToCont = true;
    if (! pause())
      return false;
  }

  if (status_ != stopped && status_ != neonatal) {
    string msg = string("Internal paradynd error in process::writeTextWord")
               + string((int)status_);
    showErrorCallback(38, msg);
    //showErrorCallback(38, "Internal paradynd error in process::writeTextWord");
    return false;
  }

  bool res = writeTextWord_(inTracedProcess, data);
  if (!res) {
    string msg = string("System error: unable to write to process text word:")
	           + string(sys_errlist[errno]);
    showErrorCallback(38, msg);
    return false;
  }

  if (needToCont)
    return this->continueProc();
  return true;

}

bool process::writeTextSpace(void *inTracedProcess, int amount, const void *inSelf) {
  bool needToCont = false;

  if (status_ == exited)
    return false;

  if (status_ == running) {
    needToCont = true;
    if (! pause())
      return false;
  }

  if (status_ != stopped && status_ != neonatal) {
    string msg = string("Internal paradynd error in process::writeTextSpace")
               + string((int)status_);
    showErrorCallback(38, msg);
    //showErrorCallback(38, "Internal paradynd error in process::writeTextSpace");
    return false;
  }

  bool res = writeTextSpace_(inTracedProcess, amount, inSelf);
  if (!res) {
    string msg = string("System error: unable to write to process text space:")
	           + string(sys_errlist[errno]);
    showErrorCallback(38, msg);
    return false;
  }

  if (needToCont)
    return this->continueProc();
  return true;
}

bool process::pause() {
  if (status_ == stopped || status_ == neonatal)
    return true;

  if (status_ == exited)
    return false;

  if (status_ == running && reachedFirstBreak) {
    bool res = pause_();
    if (!res)
      return false;

    status_ = stopped;
  }
  else {
    // The only remaining combination is: status==running but haven't yet
    // reached first break.  We never want to pause before reaching the
    // first break (trap, actually).  But should we be returning true or false in this
    // case?
  }

  return true;
}

//
//  this routines inserts initial instrumentation into the PLT
//  so that function calls to dynamically linked objects that invoke
//  the dynamic linker are caught 
//
//  TODO: This routine also traverses the link map and creates a list
//        of (shared object file name,shared object base address, processed)
//        structs for each element in the link map.  The daemon also 
//	  instruments the r_brk routine so that mapping and unmapping
//	  of shared objects by the runtime linker is detected
//
bool process::handleStartProcess(process *p){

    if(!p){
        return false;
    }

    // get shared objects, parse them, and define new resources 
    p->getSharedObjects(); // should now work when we attach to a running process

    if(resource::num_outstanding_creates)
       p->setWaitingForResources();

    return true;
}

//  Executes on the exit point of the exec: does any necessary initialization
//  for the run time linker to export dynamic linking information
//  returns true if the executable is dynamic
//
// bool process::findDynamicLinkingInfo(){
//     dynamiclinking = true;
//     return dynamiclinking;
// }

// addASharedObject: This routine is called whenever a new shared object
// has been loaded by the run-time linker
// It processes the image, creates new resources
bool process::addASharedObject(shared_object &new_obj){

    image *img = image::parseImage(new_obj.getName(),new_obj.getBaseAddress());
    if(!img){
        logLine("error parsing image in addASharedObject\n");
    }
    new_obj.addImage(img);

    // if the list of all functions and all modules have already been 
    // created for this process, then the functions and modules from this
    // shared object need to be added to those lists 
    if(all_modules){
        *all_modules += *(new_obj.getModules()); 
    }
    if(all_functions){
        *all_functions += *(new_obj.getAllFunctions()); 
    }

    // if the signal handler function has not yet been found search for it
    if(!signal_handler){
        signal_handler = img->findOneFunction(SIGNAL_HANDLER);
    }

    // clear the include_funcs flag if this shared object should not be
    // included in the some_functions and some_modules lists
    vector<string> lib_constraints;
    if(mdl_get_lib_constraints(lib_constraints)){
        for(u_int i=0; i < lib_constraints.size(); i++){
           if(new_obj.getName() == lib_constraints[i]){
	      new_obj.changeIncludeFuncs(false); 
           }
        }
    }

    if(new_obj.includeFunctions()){
        if(some_modules){
            *some_modules += *(new_obj.getModules()); 
        }
        if(some_functions){
            *some_functions += *(new_obj.getAllFunctions()); 
        }
    }
    return true;
}

// getSharedObjects: This routine is called before main() to get and
// process all shared objects that have been mapped into the process's
// address space
bool process::getSharedObjects() {

    assert(!shared_objects);
    shared_objects = dyn->getSharedObjects(this); 
    if(shared_objects){
	statusLine("parsing shared object files");

        tp->resourceBatchMode(true);
	// for each element in shared_objects list process the 
	// image file to find new instrumentaiton points
	for(u_int i=0; i < shared_objects->size(); i++){
	    string temp2 = string(i);
	    temp2 += string("th shared obj, addr: ");
	    temp2 += string(((*shared_objects)[i])->getBaseAddress());
	    temp2 += string(" name: ");
	    temp2 += string(((*shared_objects)[i])->getName());
	    temp2 += string("\n");
	    // logLine(P_strdup(temp2.string_of()));
	    if(!addASharedObject(*((*shared_objects)[i]))){
	        logLine("Error after call to addASharedObject\n");
	    }
	}

	tp->resourceBatchMode(false);
	return true;
    }
    // else this a.out does not have a .dynamic section
    dynamiclinking = false;
    return false;
}

// findOneFunction: returns the function associated with func  
// this routine checks both the a.out image and any shared object
// images for this resource
pdFunction *process::findOneFunction(resource *func,resource *mod){
    
    if((!func) || (!mod)) { return 0; }
    if(func->type() != MDL_T_PROCEDURE) { return 0; }
    if(mod->type() != MDL_T_MODULE) { return 0; }

    const vector<string> &f_names = func->names();
    const vector<string> &m_names = mod->names();
    string func_name = f_names[f_names.size() -1]; 
    string mod_name = m_names[m_names.size() -1]; 

    // KLUDGE: first search any shared libraries for the module name 
    //  (there is only one module in each shared library, and that 
    //  is the library name)
    if(dynamiclinking && shared_objects){
        for(u_int i=0; i < shared_objects->size(); i++){
            module *next = 0;
	    next = ((*shared_objects)[i])->findModule(mod_name);
	    if(next){
	        return(((*shared_objects)[i])->findOneFunction(func_name));
	    }
        }
    }

    // check a.out for function symbol
    return(symbols->findOneFunction(func_name));
}

// findOneFunction: returns the function associated with func  
// this routine checks both the a.out image and any shared object
// images for this resource
pdFunction *process::findOneFunction(const string &func_name){
    
    // first check a.out for function symbol
    pdFunction *pdf = symbols->findOneFunction(func_name);
    if(pdf) return pdf;

    // search any shared libraries for the file name 
    if(dynamiclinking && shared_objects){
        for(u_int i=0; i < shared_objects->size(); i++){
	    pdf = ((*shared_objects)[i])->findOneFunction(func_name);
	    if(pdf){
	        return(pdf);
	    }
    } }
    return(0);
}

// findFunctionIn: returns the function containing the address "adr"
// this routine checks both the a.out image and any shared object
// images for this resource
pdFunction *process::findFunctionIn(Address adr){

    // first check a.out for function symbol
    pdFunction *pdf = symbols->findFunctionIn(adr,this);
    if(pdf) return pdf;
    // search any shared libraries for the function 
    if(dynamiclinking && shared_objects){
        for(u_int i=0; i < shared_objects->size(); i++){
	    pdf = ((*shared_objects)[i])->findFunctionIn(adr,this);
	    if(pdf){
	        return(pdf);
	    }
    } }

    if(!all_functions) getAllFunctions();


    // if the function was not found, then see if this addr corresponds
    // to  a function that was relocated in the heap
    if(all_functions){
        for(u_int i=0; i < all_functions->size(); i++){
	    Address func_adr = ((*all_functions)[i])->getAddress(this);
            if((adr>=func_adr) && 
		(adr<=(((*all_functions)[i])->size()+func_adr))){
	        return((*all_functions)[i]);
	    }
        }
    }
    return(0);
}
	
	
// findModule: returns the module associated with mod_name 
// this routine checks both the a.out image and any shared object
// images for this resource
module *process::findModule(const string &mod_name){

    // KLUDGE: first search any shared libraries for the module name 
    //  (there is only one module in each shared library, and that 
    //  is the library name)
    if(dynamiclinking && shared_objects){
        for(u_int i=0; i < shared_objects->size(); i++){
            module *next = 0;
	    next = ((*shared_objects)[i])->findModule(mod_name);
	    if(next){
	        return(next);
	    }
    } }

    // check a.out for function symbol
    return(symbols->findModule(mod_name));
}

// getSymbolInfo:  get symbol info of symbol associated with name n
// this routine starts looking a.out for symbol and then in shared objects
bool process::getSymbolInfo(string &name, Symbol &info){
    
    // first check a.out for symbol
    if(symbols->symbol_info(name,info)) return true;

    // next check shared objects
    if(dynamiclinking && shared_objects){
        for(u_int i=0; i < shared_objects->size(); i++){
	    if(((*shared_objects)[i])->getSymbolInfo(name,info)) { 
	        return true; 
    } } } 
    return false;
}


// getAllFunctions: returns a vector of all functions defined in the
// a.out and in the shared objects
// TODO: what to do about duplicate function names?
vector<pdFunction *> *process::getAllFunctions(){

    // if this list has already been created, return it
    if(all_functions) 
	return all_functions;

    // else create the list of all functions
    all_functions = new vector<pdFunction *>;
    *all_functions += symbols->mdlNormal;

    if(dynamiclinking && shared_objects){
        for(u_int i=0; i < shared_objects->size(); i++){
	   vector<pdFunction *> *funcs = 
			((*shared_objects)[i])->getAllFunctions();
	   if(funcs) { 
	       *all_functions += *funcs; 
           }
    } } 
    return all_functions;
}
      
// getAllModules: returns a vector of all modules defined in the
// a.out and in the shared objects
vector<module *> *process::getAllModules(){

    // if the list of all modules has already been created, the return it
    if(all_modules) return all_modules;

    // else create the list of all modules
    all_modules = new vector<module *>;
    *all_modules += symbols->mods;

    if(dynamiclinking && shared_objects){
        for(u_int i=0; i < shared_objects->size(); i++){
	   vector<module *> *mods = ((*shared_objects)[i])->getModules();
	   if(mods) {
	       *all_modules += *mods; 
           }
    } } 
    return all_modules;
}

// getIncludedFunctions: returns a vector of all functions defined in the
// a.out and in the shared objects
// TODO: what to do about duplicate function names?
vector<pdFunction *> *process::getIncludedFunctions(){

    // if this list has already been created, return it
    if(some_functions) 
	return some_functions;

    // else create the list of all functions
    some_functions = new vector<pdFunction *>;
    *some_functions += symbols->mdlNormal;

    if(dynamiclinking && shared_objects){
        for(u_int i=0; i < shared_objects->size(); i++){
	    if(((*shared_objects)[i])->includeFunctions()){
	        vector<pdFunction *> *funcs = 
			((*shared_objects)[i])->getAllFunctions();
	        if(funcs) { 
	            *some_functions += *funcs; 
                }
            } 
    } } 
    return some_functions;
}

// getIncludedModules: returns a vector of all modules defined in the
// a.out and in the shared objects that are included as specified in
// the mdl
vector<module *> *process::getIncludedModules(){

    // if the list of all modules has already been created, the return it
    if(some_modules) return some_modules;

    // else create the list of all modules
    some_modules = new vector<module *>;
    *some_modules += symbols->mods;

    if(dynamiclinking && shared_objects){
        for(u_int i=0; i < shared_objects->size(); i++){
	    if(((*shared_objects)[i])->includeFunctions()){
	       vector<module *> *mods = ((*shared_objects)[i])->getModules();
	       if(mods) {
	           *some_modules += *mods; 
               }
	   }
    } } 
    return some_modules;
}
      
// getBaseAddress: sets baseAddress to the base address of the 
// image corresponding to which.  It returns true  if image is mapped
// in processes address space, otherwise it returns 0
bool process::getBaseAddress(const image *which,u_int &baseAddress){

  if((u_int)(symbols) == (u_int)(which)){
      baseAddress = 0; 
      return true;
  }
  else if (shared_objects) {  
      // find shared object corr. to this image and compute correct address
      for(u_int i=0; i <  shared_objects->size(); i++){ 
	  if(((*shared_objects)[i])->isMapped()){
            if(((*shared_objects)[i])->getImageId() == (u_int)which) { 
	      baseAddress = ((*shared_objects)[i])->getBaseAddress();
	      return true;
	  } }
      }
  }
  return false;
}

// findSignalHandler: if signal_handler is 0, then it checks all images
// associtated with this process for the signal handler function.
// Otherwise, the signal handler function has already been found
void process::findSignalHandler(){

    if(SIGNAL_HANDLER == 0) return;
    if(!signal_handler) { 
        // first check a.out for signal handler function
        signal_handler = symbols->findOneFunction(SIGNAL_HANDLER);

	// search any shared libraries for signal handler function
        if(!signal_handler && dynamiclinking && shared_objects) { 
	    for(u_int i=0;(i < shared_objects->size()) && !signal_handler; i++){
	        signal_handler = 
		      ((*shared_objects)[i])->findOneFunction(SIGNAL_HANDLER);
	} }
    }
}

bool process::continueProc() {
  if (status_ == exited) return false;

  if (status_ != stopped && status_ != neonatal) {
    showErrorCallback(38, "Internal paradynd error in process::continueProc");
#ifdef FORK_EXEC_DEBUG
    assert(false);
#endif
    return false;
  }

  bool res = continueProc_();

  if (!res) {
    showErrorCallback(38, "System error: can't continue process");
    return false;
  }
  status_ = running;
  return true;
}

bool process::detach(const bool paused) {
  if (paused) {
    logLine("detach: pause not implemented\n"); // why not? --ari
  }
  bool res = detach_();
  if (!res) {
    // process may have exited
    return false;
  }
  return true;
}

/* process::handleExec: called when a process successfully exec's.
   Parse the new image, disable metric instances on the old image, create a
   new (and blank) shm segment.  The process hasn't yet bootstrapped, so we
   mustn't try to enable anything...
*/
void process::handleExec() {
    // NOTE: for shm sampling, the shm segment has been removed, so we
    //       mustn't try to disable any dataReqNodes in the standard way...

   // since the exec syscall has run, we're not ready to enable any m/f pairs or
   // sample anything.
   // So we set hasBootstrapped to false until we run DYNINSTinit again.
   hasBootstrapped = false;

    // all instrumentation that was inserted in this process is gone.
    // set exited here so that the disables won't try to write to process
    status_ = exited; 
   
     // can't use removeFromMetricInstances, because in truth we actually want to
     // try and carry over as much stuff as appropriate to the post-exec process.
     // (an inappropriate, un-carry-overable mi would be something specific to the
     // code hierarchy of the pre-exec process, which doesn't exist in the space of
     // the new process).  We can't carry over anything yet, however, since the proc
     // hasn't yet bootstrapped.
//    removeFromMetricInstances(this);

    // Clean up state from old exec: all dynamic linking stuff, all lists 
    // of functions and modules from old executable

    // can't delete dynamic linking stuff here, because parent process
    // could still have pointers
    dynamiclinking = false;
    dyn = 0; // AHEM.  LEAKED MEMORY!
    dyn = new dynamic_linking;
    if(shared_objects){
        for(u_int i=0; i< shared_objects->size(); i++){
            delete (*shared_objects)[i];
        }
        delete shared_objects;
        shared_objects = 0;
    }

    // TODO: when can pdFunction's be deleted???  definitely not here.
    delete some_modules;
    delete some_functions;
    delete all_functions;
    delete all_modules;
    some_modules = 0;
    some_functions = 0;
    all_functions = 0;
    all_modules = 0;
    signal_handler = 0;
    trampTableItems = 0;
    memset(trampTable, 0, sizeof(trampTable));
    baseMap.clear();

#if defined(rs6000_ibm_aix3_2) || defined(rs6000_ibm_aix4_1)
    // must call establishBaseAddrs before parsing the new image,
    // but doesn't need to wait for trap, since we already got the trap.
    bool establishBaseAddrs(int pid, int &status, bool waitForTrap);
    int status;
    establishBaseAddrs(getPid(), status, false);
#endif

    image *img = image::parseImage(execFilePath);
    if (!img) {
       // For better error reporting, two failure return values would be useful
       // One for simple error like because-file-not-found
       // Another for serious errors like found-but-parsing-failed (internal error;
       //    please report to paradyn@cs.wisc.edu)

       string msg = string("Unable to parse image: ") + execFilePath;
       showErrorCallback(68, msg.string_of());
       P_kill(pid, 9);
       return;
    }

    // delete proc->symbols ???  No, the image can be shared by more
    // than one process...images and instPoints can not be deleted...TODO
    // add some sort of reference count to these classes so that they can
    // be deleted
    symbols = img; // AHEM!  LEAKED MEMORY!!!

    // see if new image contains the signal handler function
    this->findSignalHandler();

    // initInferiorHeap can only be called after symbols is set!
    initInferiorHeap(false);
    if (splitHeaps) {
      initInferiorHeap(true);  // create separate text heap
    }

    /* update process status */
    reachedFirstBreak = false;
       // we haven't yet seen initial SIGTRAP for this proc (is this right?)

    status_ = stopped; // was 'exited'

   // TODO: We should remove (code) items from the where axis, if the exec'd process
   // was the only one who had them.

   // the exec'd process has the same fd's as the pre-exec, so we don't need
   // to re-initialize traceLink or ioLink (is this right???)

   // we don't need to re-attach after an exec (is this right???)
 
#ifdef SHM_SAMPLING
   inferiorHeapMgr.handleExec();
      // reuses the shm seg (paradynd's already attached to it); resets applic-attached-
      // at to NULL.  Quite similar to the (non-fork) ctor, really.

   inferiorIntCounters.handleExec();
   inferiorWallTimers.handleExec();
   inferiorProcessTimers.handleExec();
#endif

   inExec = false;
   execed_ = true;
}

/* 
   process::cleanUpInstrumentation called when paradynd catch
   a SIGTRAP to find out if there's any previous unfinished instrumentation
   requests 
*/
bool process::cleanUpInstrumentation(bool wasRunning) {
    // Try to process an item off of the waiting list 'instWlist'.
    // If something was found & processed, then true will be returned.
    // Additionally, if true is returned, the process will be continued
    // if 'wasRunning' is true.
    // But if false is returned, then there should be no side effects: noone
    // should be continued, nothing removed from 'instWList', no change
    // to this->status_, and so on (this is important to avoid bugs).

    assert(status_ == stopped); // since we're always called after a SIGTRAP
    Frame frame(this);
    Address pc = frame.getPC();

    // Go thru the instWList to find out the ones to be deleted 
    bool done = false;
    u_int i=0;
    bool found = false;
    while(!done){
	//process *p = (instWList[i])->which_proc;
        if(((instWList[i])->pc_ == pc) && ((instWList[i])->which_proc == this)){
	    (instWList[i])->cleanUp(this,pc);
	    u_int last = instWList.size()-1;
	    delete (instWList[i]);
	    instWList[i] = instWList[last];
	    instWList.resize(last);
	    found = true;
	}
	else {
	    i++;
	}
	if(i >= instWList.size()) done = true;
    }
    if(found && wasRunning) continueProc();
    return found;
}

void process::postRPCtoDo(AstNode *action, bool noCost,
			  void (*callbackFunc)(process *, void *),
			  void *userData) {
   // posts an RPC, but does NOT make any effort to launch it.
   inferiorRPCtoDo theStruct;
   theStruct.action = action;
   theStruct.noCost = noCost;
   theStruct.callbackFunc = callbackFunc;
   theStruct.userData = userData;

   RPCsWaitingToStart += theStruct;
}

bool process::existsRPCreadyToLaunch() const {
   if (currRunningRPCs.empty() && !RPCsWaitingToStart.empty())
      return true;
   return false;
}

bool process::existsRPCinProgress() const {
   return (!currRunningRPCs.empty());
}

bool process::launchRPCifAppropriate(bool wasRunning) {
   // asynchronously launches an inferiorRPC iff RPCsWaitingToStart.size() > 0 AND
   // if currRunningRPCs.size()==0 (the latter for safety)

   if (!currRunningRPCs.empty())
      // an RPC is currently executing, so it's not safe to launch a new one.
      return false;

   if (RPCsWaitingToStart.empty())
      // duh, no RPC is waiting to run, so there's nothing to do.
      return false;

   /* ****************************************************** */

   if (status_ == exited)
      return false;

   if (status_ == neonatal)
      // not sure if this should be some kind of error...is the inferior ready
      // to execute inferior RPCs??? For now, we'll allow it.
      ; 

   // Steps to take (on sparc, at least)
   // 1) pause the process and wait for it to stop
   // 2) GETREGS (ptrace call) & store away
   // 3) create temp tramp: save, action, restore, trap, illegal
   //    (the illegal is just ot be sure that the trap never returns)
   // 4) set the PC and nPC regs to addr of temp tramp
   // 5) PTRACE_CONT; go back to main loop (SIGTRAP will eventually be delivered)

   // When SIGTRAP is received,
   // 1) verify that PC is the location of the TRAP instr in the temp tramp
   // 2) free tramp
   // 3) SETREGS to restore all regs, including PC and nPC.
   // 4) continue inferior, if appropriate (THIS IS AN UNSETTLED ISSUE).

   if (!pause()) {
      cerr << "launchRPCifAppropriate failed because pause failed" << endl;
      return false;
   }

   bool syscall = false;

   void *theSavedRegs = getRegisters(syscall); // machine-specific implementation
      // result is allocated via new[]; we'll delete[] it later.
      // return value of NULL indicates total failure.
      // return value of (void *)-1 indicates that the state of the machine isn't quite
      //    ready for an inferiorRPC, and that we should try again 'later'.  In
      //    particular, we must handle the (void *)-1 case very gracefully (i.e., leave
      //    the vrble 'RPCsWaitingToStart' untouched).

   if (theSavedRegs == (void *)-1) {
      // cerr << "launchRPCifAppropriate: deferring" << endl;
      if (wasRunning)
	 (void)continueProc();
      return false;
   }

   if (theSavedRegs == NULL) {
      cerr << "launchRPCifAppropriate failed because getRegisters() failed" << endl;
      if (wasRunning)
	 (void)continueProc();
      return false;
   }

   inferiorRPCtoDo todo = RPCsWaitingToStart.removeOne();
      // note: this line should always be below the test for (void*)-1, thus
      // leaving 'RPCsWaitingToStart' alone in that case.

// Code for the HPUX version of inferiorRPC exists, but crashes, so
// for now, don't do inferiorRPC on HPUX!!!!
/* #if defined(hppa1_1_hp_hpux)
if (wasRunning)
  (void)continueProc();
return false;
#endif */

   inferiorRPCinProgress inProgStruct;
   inProgStruct.callbackFunc = todo.callbackFunc;
   inProgStruct.userData = todo.userData;
   inProgStruct.savedRegs = theSavedRegs;
   inProgStruct.wasRunning = wasRunning;
   unsigned tempTrampBase = createRPCtempTramp(todo.action,
					       todo.noCost,
					       inProgStruct.firstPossibleBreakAddr,
					       inProgStruct.lastPossibleBreakAddr);
      // the last 2 args are written to

   if (tempTrampBase == NULL) {
      cerr << "launchRPCifAppropriate failed because createRPCtempTramp failed" << endl;
      if (wasRunning)
	 (void)continueProc();
      return false;
   }

   assert(tempTrampBase);

   inProgStruct.firstInstrAddr = tempTrampBase;

   assert(currRunningRPCs.empty()); // since it's unsafe to run > 1 at a time
   currRunningRPCs += inProgStruct;

   // cerr << "Changing pc and exec.." << endl;

   // change the PC and nPC registers to the addr of the temp tramp
#if defined(hppa1_1_hp_hpux)
   if (syscall) {
       // within system call, just directly change the PC
       if (!changePC(tempTrampBase, theSavedRegs)) {
	   cerr << "launchRPCifAppropriate failed because changePC() failed" << endl;
	   if (wasRunning)
	       (void)continueProc();
	   return false;
       }
   } else {
       pdFunction *f = symbols->findOneFunctionFromAll("$$dyncall");
       if (!f) {
	   cerr << "$$dyncall was not found, inferior RPC won't work!" << endl;   
	
	   //       if (wasRunning)
	   //	   (void)continueProc();
	   //       return false;
       }
       
       int errno = 0;
       ptrace(PT_WUREGS, getPid(), 22 * 4, tempTrampBase, 0);
       if (errno != 0) {
	   perror("process::changePC");
	   cerr << "reg num was 22." << endl;
	   return false;
       }
       
       int miniAddr = f -> getAddress(this);
       if (!changePC(miniAddr, theSavedRegs)) {
	    cerr << "launchRPCifAppropriate failed because changePC() failed" << endl;
	   if (wasRunning)
	       (void)continueProc();
	   return false;
	}
   }
#else
       if (!changePC(tempTrampBase, theSavedRegs)) {
	   cerr << "launchRPCifAppropriate failed because changePC() failed" << endl;
	   if (wasRunning)
	       (void)continueProc();
	   return false;
       }
#endif

   // cerr << "Finished...." << endl;

   if (!continueProc()) {
      cerr << "launchRPCifAppropriate: continueProc() failed" << endl;
      return false;
   }

   return true; // success
}

unsigned process::createRPCtempTramp(AstNode *action,
				     bool noCost,
				     unsigned &firstPossibBreakAddr,
				     unsigned &lastPossibBreakAddr) {

   // Returns addr of temp tramp, which was allocated in the inferoir heap.
   // You must free it yourself when done.

   // Note how this is, in many ways, a greatly simplified version of
   // addInstFunc().

   // Temp tramp structure: save; code; restore; trap; illegal
   // the illegal is just to make sure that the trap never returns
   // note that we may not need to save and restore anything, since we've
   // already done a GETREGS and we'll restore with a SETREGS, right?

   unsigned char insnBuffer[4096];

   initTramps(); // initializes "regSpace", but only the 1st time it gets called...
   extern registerSpace *regSpace;
   regSpace->resetSpace();

   unsigned count = 0;

   // The following is implemented in an arch-specific source file...
   if (!emitInferiorRPCheader(insnBuffer, count)) {
      // a fancy dialog box is probably called for here...
      cerr << "createRPCtempTramp failed because emitInferiorRPCheader failed." << endl;
      return NULL;
   }

   reg resultReg = action->generateCode(this, regSpace,
				        (char*)insnBuffer,
				        count, noCost);
   // do we need to use the result register?
   regSpace->freeRegister(resultReg);

   // Now, the trailer (restore, TRAP, illegal)
   // (the following is implemented in an arch-specific source file...)

   unsigned firstPossibBreakOffset, lastPossibBreakOffset;
   if (!emitInferiorRPCtrailer(insnBuffer, count,
			       firstPossibBreakOffset, lastPossibBreakOffset)) {
      // last 3 args are modified by the call
      cerr << "createRPCtempTramp failed because emitInferiorRPCtrailer failed." << endl;
      return NULL;
   }

   unsigned tempTrampBase = inferiorMalloc(this, count, textHeap);
   assert(tempTrampBase);

   firstPossibBreakAddr = tempTrampBase + firstPossibBreakOffset;
   lastPossibBreakAddr = tempTrampBase + lastPossibBreakOffset;

   /* Now, write to the tempTramp, in the inferior addr's data space
      (all tramps are allocated in data space) */
   if (!writeDataSpace((void*)tempTrampBase, count, insnBuffer)) {
      // should put up a nice error dialog window
      cerr << "createRPCtempTramp failed because writeDataSpace failed" << endl;
      return NULL;
   }

   extern int trampBytes; // stats.h
   trampBytes += count;

   return tempTrampBase;
}

bool process::handleTrapIfDueToRPC() {
   // get curr PC register (can assume process is stopped), search for it in
   // 'currRunningRPCs'.  If found, restore regs, do callback, delete tramp, and
   // return true.  Returns false if not processed.

   assert(status_ == stopped); // a TRAP should always stop a process (duh)
   
   if (currRunningRPCs.empty())
      return false; // no chance of a match

   assert(currRunningRPCs.size() == 1);
      // it's unsafe to have > 1 RPCs going on at a time within a single process

   // Okay, time to do a stack trace.
   // If we determine that the PC of any level of the back trace
   // falls within the bounds of [currRunningRPCs[0]'s address range],
   // then we assume success.  Note that we could probably narrow
   // it down to an EXACT address, to increase resistence to spurious
   // signals.
   Frame theFrame(this);
   while (true) {
      // do we have a match?
      const int framePC = theFrame.getPC();
      if ((unsigned)framePC >= currRunningRPCs[0].firstPossibleBreakAddr &&
	  (unsigned)framePC <= currRunningRPCs[0].lastPossibleBreakAddr) {
	 // we've got a match!
	 break;
      }

      if (theFrame.isLastFrame())
	 // well, we've gone as far as we can, with no match.
	 return false;

      // else, backtrace 1 more level
      theFrame = theFrame.getPreviousStackFrameInfo(this);
   }

   inferiorRPCinProgress theStruct = currRunningRPCs[0];

   // step 1) restore registers:
   if (!restoreRegisters(theStruct.savedRegs)) {
      cerr << "handleTrapIfDueToRPC failed because restoreRegisters failed" << endl;
      assert(false);
   }
   // Okay, we have a match.
   currRunningRPCs.removeByIndex(0);

//   assert(theStruct.trapInstrAddr == currPCreg);
   delete [] theStruct.savedRegs;


   // step 2) delete temp tramp
   vector< vector<unsigned> > pointsToCheck;
      // blank on purpose; deletion is safe to take place even right now
   inferiorFree(this, theStruct.firstInstrAddr, textHeap, pointsToCheck);

   // step 3) invoke callback, if any
   if (theStruct.callbackFunc) {
      theStruct.callbackFunc(this, theStruct.userData);
   }

   // step 4) continue process, if appropriate
   if (theStruct.wasRunning) {
      if (!continueProc())
	 cerr << "RPC completion: continueProc failed" << endl;
   }

   return true;
}

void process::installBootstrapInst() {
   // instrument main to call DYNINSTinit().  Don't use the shm seg for any
   // temp tramp space, since we can't assume that it's been intialized yet.
   // We build an ast saying: "call DYNINSTinit() with args
   // key_base, nbytes, -1, -1".

   vector<AstNode *> the_args(3);

   // 2 dummy args when not shm sampling (just don't use -1, which is reserved
   // for fork)
   key_t theKey      = (key_t)0; // so far
   unsigned numBytes = 0;
   
#ifdef SHM_SAMPLING
   theKey   = getShmKeyUsed();
   numBytes = getShmHeapTotalNumBytes();
#endif

#ifdef SHM_SAMPLING_DEBUG
   cerr << "paradynd inst.C: about to call DYNINSTinit() with key=" << theKey
        << " and #bytes=" << numBytes << endl;
#endif

   the_args[0] = new AstNode(AstNode::Constant, (void*)theKey);
   the_args[1] = new AstNode(AstNode::Constant, (void*)numBytes);
   the_args[2] = new AstNode(AstNode::Constant, (void*)-1);

   AstNode *ast = new AstNode("DYNINSTinit", the_args);
   for (unsigned j=0; j<the_args.size(); j++) {
       removeAst(the_args[j]);
   }

   pdFunction *func = findOneFunction("main");
   assert(func);

   const instPoint *func_entry = func->funcEntry(this);
   addInstFunc(this, func_entry, ast, callPreInsn,
	       orderFirstAtPoint,
	       true // true --> don't try to have tramp code update the cost
	       );
   removeAst(ast);
      // returns an "instInstance", which we ignore (but should we?)
}

void process::installInstrRequests(const vector<instMapping*> &requests) {
   for (unsigned lcv=0; lcv < requests.size(); lcv++) {
      instMapping *req = requests[lcv];
      
      pdFunction *func = findOneFunction(req->func);
      if (!func)
	 continue;  // probably should have a flag telling us whether errors should
	            // be silently handled or not

      AstNode *ast;
      if (req->where & FUNC_ARG)
	 ast = new AstNode(req->inst, req->arg);
      else {
         AstNode *tmp = new AstNode(AstNode::Constant, (void*)0);
	 ast = new AstNode(req->inst, tmp);
	 removeAst(tmp);
      }

      if (req->where & FUNC_EXIT) {
	 const vector<instPoint*> func_rets = func->funcExits(this);
	 for (unsigned j=0; j < func_rets.size(); j++)
	    (void)addInstFunc(this, func_rets[j], ast,
			      callPreInsn, orderLastAtPoint, false);

      }

      if (req->where & FUNC_ENTRY) {
	 const instPoint *func_entry = func->funcEntry(this);
	 (void)addInstFunc(this, func_entry, ast,
			   callPreInsn, orderLastAtPoint, false);

      }

      if (req->where & FUNC_CALL) {
	 const vector<instPoint*> func_calls = func->funcCalls(this);
	 if (func_calls.size() == 0)
	    continue;

	 for (unsigned j=0; j < func_calls.size(); j++)
	    (void)addInstFunc(this, func_calls[j], ast,
			      callPreInsn, orderLastAtPoint, false);

      }

      removeAst(ast);
   }
}

bool process::extractBootstrapStruct(DYNINST_bootstrapStruct *bs_record) {
   const string vrbleName = "DYNINST_bootstrap_info";

   internalSym *sym = findInternalSymbol(vrbleName, true);
   assert(sym);

   Address symAddr = sym->getAddr();

   if (!readDataSpace((const void*)symAddr, sizeof(*bs_record), bs_record, true)) {
      cerr << "extractBootstrapStruct failed because readDataSpace failed" << endl;
      return false;
   }

   return true;
}

bool process::handleStopDueToExecEntry() {
   // returns true iff we are processing a stop due to the entry point of exec
   // The exec hasn't yet occurred.

   assert(status_ == stopped);

   DYNINST_bootstrapStruct bs_record;
   if (!extractBootstrapStruct(&bs_record))
      assert(false);

   if (bs_record.event != 4)
      return false;

   assert(getPid() == bs_record.pid);

   // for now, we just set aside the following information, to be used after the
   // exec actually happens (we'll get a SIGTRAP for that).
   assert(!inExec);
   inExec = true;
   execFilePath = string(bs_record.path);

   // the process was stopped...let's continue it so we can process the exec...
   assert(status_ == stopped);
   if (!continueProc())
      assert(false);

   // should we set status_ to neonatal now?  Nah, probably having the inExec flag
   // set is good enough...

   // shouldn't we be setting reachedFirstBreak to false???

   return true;
}

bool process::procStopFromDYNINSTinit() {
   // returns true iff we are now processing the bootstrap info filled in
   // by the end of DYNINSTinit.  Note that there are 3 cases:
   // 1) the normal case     (detect by bs_record.event==1 && execed_ == false)
   // 2) called after a fork (detect by bs_record.event==2)
   // 3) called after an exec (detect by bs_record.event==1 and execed_ == true)
   // 4) called for an attach (detect by bs_record.event==3)
   // note that bs_record.event == 4 is reserved for "sending" a tr_exec "record".
   //
   // The exec case is tricky: we must loop thru all component mi's of this process
   // and decide now whether or not to carry them over to the new process.

   // if false is returned, there must be no side effects.

   assert(status_ == stopped);

   if (hasBootstrapped)
      return false;

   DYNINST_bootstrapStruct bs_record;
   if (!extractBootstrapStruct(&bs_record))
      assert(false);

   // Read the structure; if event 0 then it's undefined! (not yet written)
   if (bs_record.event == 0)
      return false;

   forkexec_cerr << "procStopFromDYNINSTinit pid " << getPid() << "; got rec" << endl;

   string str=string("PID=") + string(bs_record.pid) + ", receiving bootstrap info...";

   statusLine(str.string_of());

   assert(bs_record.event == 1 || bs_record.event == 2 || bs_record.event==3);
   const bool calledFromFork   = (bs_record.event == 2);
   const bool calledFromExec   = (bs_record.event == 1 && execed_);
   const bool calledFromAttach = (bs_record.event == 3);

   assert(getPid() == bs_record.pid);

#ifdef SHM_SAMPLING
   if (!calledFromFork)
      registerInferiorAttachedSegs(bs_record.appl_attachedAtPtr);
#endif

   if (!calledFromFork)
      getObservedCostAddr();

   // handleStartProcess gets shared objects, so no need to do it again after a fork.
   // (question: do we need to do this after an exec???)
   if (!calledFromFork) {
      str=string("PID=") + string(bs_record.pid) + ", calling handleStartProcess...";
      statusLine(str.string_of());

      if (!handleStartProcess(this))
         logLine("warning: handleStartProcess failed\n");

      // we decrement the batch mode here; it matches the bump-up in createProcess()
      tp->resourceBatchMode(false);

      str=string("PID=") + string(bs_record.pid) + ", installing default inst...";
      statusLine(str.string_of());

      extern vector<instMapping*> initialRequests; // init.C
      installInstrRequests(initialRequests);

      str=string("PID=") + string(bs_record.pid) + ", propagating mi's...";
      statusLine(str.string_of());

      forkexec_cerr << "procStopFromDYNINSTinit pid " << getPid() << "; about to propagate mi's" << endl;

      if (!calledFromExec) {
         // propagate any metric that is already enabled to the new process.
         // For a forked process, this isn't needed because handleFork() has its own
         // special propagation algorithm (it propagates every aggregate mi having the
	 // parent as a component, except for aggregate mi's whose focus is specifically
	 // refined to the parent).
	 vector<metricDefinitionNode *> MIs = allMIs.values();
	 for (unsigned j = 0; j < MIs.size(); j++) {
	    MIs[j]->propagateToNewProcess(this);
	    // change to a process:: method which takes in the metricDefinitionNode
	 }
      }
      else {
         // exec propagates in its own, special way that differs from a new process.
	 // (propagate all mi's that make sense in the new process)
	 metricDefinitionNode::handleExec(this);
      }

      forkexec_cerr << "procStopFromDYNINSTinit pid " << getPid() << "; done propagate mi's" << endl;
   }

   hasBootstrapped = true; // now, shm sampling may safely take place.

   str=string("PID=") + string(bs_record.pid) + ", executing new-prog callback...";
   statusLine(str.string_of());

   time64 currWallTime = calledFromExec ? 0 : getCurrWallTime();
   if (!calledFromExec) {
      // The following must be done before any samples are sent to
      // paradyn; otherwise, prepare for an assert fail.

      if (!::firstRecordTime)
	 ::firstRecordTime = currWallTime;
   }

   assert(status_ == stopped);

   // Beware of race condition when we make an igen call to paradyn -- anytime an igen
   // msg is sent, we can receive and process an igen msg (is this right???), so we
   // could conceivably receive and process a tp->continueProgram() (which paradyn
   // usually sends as soon as it receives a newProgramCallbackFunc()), all occuring
   // before the remainder of this fn is executed.
   tp->newProgramCallbackFunc(bs_record.pid, this->arg_list, 
			      machineResource->part_name(),
			      calledFromExec);
         // in paradyn, this will call paradynDaemon::addRunningProgram().
         // If the state of the application as a whole is 'running' then paradyn will
         // soon issue an igen call that'll continue this process.
         // QUESTION: If the DYNINSTinit() was due to an exec() syscall, then should
         //           this call be made?  Probably not, since the pid doesn't change and
         //           so in a sense no new process was created.

   if (!calledFromExec)
      tp->firstSampleCallback(getPid(), (double)currWallTime / 1000000.0);

   str=string("PID=") + string(bs_record.pid) + ", ready.";
   statusLine(str.string_of());

   assert(status_ == stopped);

   if (calledFromFork) {
      // the parent proc has been waiting patiently at the start of DYNINSTfork
      // (i.e. the fork syscall executed but that's it).  We can continue it now.
      process *parentProcess = findProcess(bs_record.ppid);
      if (parentProcess) {
         if (parentProcess->status() == stopped) {
	    if (!parentProcess->continueProc())
	       assert(false);
	 }
	 else
	    parentProcess->continueAfterNextStop();
      }
   }

   return true;
}

void process::getObservedCostAddr() {

#ifndef SHM_SAMPLING
    bool err;
    costAddr_ = findInternalAddress("DYNINSTobsCostLow", true, err);
    if (err) {
	logLine("Internal error: unable to find addr of DYNINSTobsCostLow\n");
	showErrorCallback(79, "");
	P_abort();
    }
#else
    costAddr_ = (int)getObsCostLowAddrInApplicSpace();
#endif
}

bool process::checkStatus() {
  if (status_ == exited) {
    sprintf(errorLine, "attempt to ptrace exited process %d\n", pid);
    logLine(errorLine);
    return(false);
  } else
    return true;
}

bool process::dumpCore(const string fileOut) {
  bool res = dumpCore_(fileOut);
  if (!res) {
    return false;
  }
  return true;
}


/*
 * The process was stopped by a signal. Update its status and notify Paradyn.
 */
void process::Stopped() {
  if (status_ != stopped) {
    status_ = stopped;
    tp->processStatus(pid, procPaused);

    if (continueAfterNextStop_) {
       continueAfterNextStop_ = false;
       if (!continueProc())
          assert(false);
    }
  }
}

/*
 *  The process has exited. Update its status and notify Paradyn.
 */
void process::Exited() {
  if (status_ != exited) {
    status_ = exited;
    tp->processStatus(pid, procExited);
  }
}

string process::getStatusAsString() const {
   // useful for debugging
   if (status_ == neonatal)
      return "neonatal";
   if (status_ == stopped)
      return "stopped";
   if (status_ == running)
      return "running";
   if (status_ == exited)
      return "exited";

   assert(false);
   return "???";
}
