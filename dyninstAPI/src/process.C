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

/*
 * process.C - Code to control a process.
 *
 * $Log: process.C,v $
 * Revision 1.64  1996/10/31 09:31:56  tamches
 * major change: the shm-sampling commit
 * inferiorRPC
 * redesigned constructors
 * removed some warnings
 * some member vrbles are now private
 *
 * Revision 1.63  1996/10/03 22:12:06  mjrg
 * Removed multiple stop/continues when inserting instrumentation
 * Fixed bug on process termination
 * Removed machine dependent code from metric.C and process.C
 *
 * Revision 1.62  1996/09/26 18:59:08  newhall
 * added support for instrumenting dynamic executables on sparc-solaris
 * platform
 *
 * Revision 1.61  1996/09/05 16:36:20  lzheng
 * Move the architecture dependent definations to the architecture dependent files
 *
 * Revision 1.60  1996/08/20 19:18:37  lzheng
 * Implementation of moving multiple instructions sequence and
 * splitting the instrumentation into two phases
 *
 * Revision 1.59  1996/08/16 21:19:39  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.58  1996/07/09 04:11:58  lzheng
 * Implentented the stack walking on HPUX machine
 *
 * Revision 1.57  1996/06/01 00:01:03  tamches
 * heapActive now uses addrHash16 instead of addrHash
 * extensively commented how paradynd captures & processs stdio of the
 * program being run.
 *
 * Revision 1.56  1996/05/15 18:32:55  naim
 * Fixing bug in inferiorMalloc and adding some debugging information - naim
 *
 * Revision 1.55  1996/05/11  23:15:17  tamches
 * inferiorHeap now uses addrHash instead of uiHash; performs better.
 *
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

#define FREE_WATERMARK (hp->totalFreeMemAvailable/2)
#define SIZE_WATERMARK 100
static const timeStamp MAX_WAITING_TIME=10.0;
static const timeStamp MAX_DELETING_TIME=2.0;

unsigned activeProcesses; // number of active processes
vector<process*> processVec;
string process::programName;
vector<string> process::arg_list;

#ifdef SHM_SAMPLING
static unsigned numIntCounters=10000; // rather arbitrary; can we do better?
static unsigned numWallTimers =10000; // rather arbitrary; can we do better?
static unsigned numProcTimers =10000; // rather arbitrary; can we do better?

static bool shmSegExists(key_t theKey, int theSize) {
   // returns true iff a shm seg w/ given key already exists
   // A crude way to find this out is to try and create a shm seg
   // with exclusive privileges; if it already exists then we'll get
   // a specific error return value.

   //cerr << "welcome to shmSegExists with key=" << theKey << endl;

   int shmid = P_shmget(theKey, theSize, IPC_CREAT | IPC_EXCL | 0666);
   if (shmid == -1) {
      if (errno == EEXIST)
	 return true;
      else {
         perror("shmSegExists: unknown shmget errcode");
	 return true; // just to be safe
      }
   }

   // we successfully created the shm seg, so it didn't exist before.
   // But now we need to clean up!
   if (-1 == P_shmctl(shmid, IPC_RMID, NULL)) {
      perror("shmSegExists: unexpected error during shmctl");
      return true; // just to be safe
   }

   return false;
}

static bool garbageCollect1ShmSeg(key_t theKey, int size) {
   if (!shmSegExists(theKey, size))
      return true; // no need to garbage collect; successful

   // attach to existing segment
   int shmid = P_shmget(theKey, size, 0666);
   if (shmid == -1) {
      cerr << "garbageCollect1ShmSeg: key was " << theKey << ", size was " << size << endl;
      perror("garbageCollect1ShmSeg shmget");
      return false;
   }

   void *shmAddr = P_shmat(shmid, NULL, SHM_RDONLY);
   if (shmAddr == (void *)-1) {
      perror("garbageCollect1ShmSeg shmat");
      return false;
   }

   // attach successful.  First word should be our 'cookie'
   unsigned word0 = *(unsigned *)shmAddr;
   if (word0 != 0xabcdefab) {
      cout << "paradynd note: it appears that shm seg with key " << theKey << " is in use by another application" << endl;
      (void)P_shmdt(shmAddr);
      return false; // don't touch
   }

   // cookie matches.  Next 2 words are as follows: applic pid, paradynd pid.
   // If neither process exists, then garbage collect; else, forget it.
   const int applicPid = *((int *)shmAddr + 1);
   const int paradyndPid = *((int *)shmAddr + 1);
   if (P_kill(applicPid, 0) == -1 && errno == ESRCH)
      if (P_kill(paradyndPid, 0) == -1 && errno == ESRCH) {
         // neither process exists.  Garbage collect!

	 //cout << "paradynd trying to garbage collect shm seg with key " << theKey << endl;
	 if (P_shmdt(shmAddr) == 0 && P_shmctl(shmid, IPC_RMID, NULL) == 0) {
            //cout << "paradynd successfully garbage collected" << endl;
	    return true;
	 }
	 //cout << "paradynd could not garbage collect" << endl;
	 return false;
      }

   (void)P_shmdt(shmAddr);
   (void)P_shmctl(shmid, IPC_RMID, NULL);

   return false; // could not garbage collect
}

static bool garbageCollectShmSegs(key_t theKey,
				  int size0, int size1, int size2) {
   // if shm seg corresponding to "theKey" exists then try to garbage
   // collect it.  Return true if freed, false otherwise; false (to be
   // safe) in cases of doubt.

   bool result = true;

   if (!garbageCollect1ShmSeg(theKey, size0))
      result = false;

   if (!garbageCollect1ShmSeg(theKey+1, size1))
      result = false;

   if (!garbageCollect1ShmSeg(theKey+2, size2))
      result = false;

   return result;
}

// shared mem segs have 12 bytes of meta-data at the beginning:
// cookie (unsigned), applic pid (int), paradynd pid (int).
// We make it 16 bytes to make sure we're very, very aligned.
static int intCounterShmSegNumBytes(unsigned num) {
   return sizeof(unsigned) + 2 * sizeof(int) + sizeof(unsigned) + sizeof(intCounter) * num;
}
static int wallTimerShmSegNumBytes(unsigned num) {
   return sizeof(unsigned) + 2 * sizeof(int) + sizeof(unsigned) + sizeof(tTimer) * num;
}
static int procTimerShmSegNumBytes(unsigned num) {
   return sizeof(unsigned) + 2 * sizeof(int) + sizeof(unsigned) + sizeof(tTimer) * num;
}

static key_t pickShmSegKey() {
   // assuming we're creating a new process, let's find an available
   // key to use in the shmget() call.  While we're at it, let's garbage
   // collect any unused segments we find along the way...

   key_t shmSegKey = 7000;
   int size0 = intCounterShmSegNumBytes(::numIntCounters);
   int size1 = wallTimerShmSegNumBytes(::numWallTimers);
   int size2 = procTimerShmSegNumBytes(::numProcTimers);

   while (true) {
      if (!shmSegExists(shmSegKey, size0) && !shmSegExists(shmSegKey+1, size1) &&
	  !shmSegExists(shmSegKey+2, size2))
	 // we found 3 consecutive unused keys, so we're done.
	 return shmSegKey;

      // at least one of the 3 segs we tried above already exists.
      // let's try to garbage collect it.
      if (garbageCollectShmSegs(shmSegKey, size0, size1, size2)) {
	 // We're on a roll, so what the hell, let's garbage collect more.
	 key_t tempKey = shmSegKey;
	 do {
	    tempKey += 10;
	    //cerr << "what the hell with tempKey=" << tempKey << endl;

	    if (!shmSegExists(tempKey, size0) && !shmSegExists(tempKey+1, size1) &&
		!shmSegExists(tempKey+2, size2)) {
	       //cerr << "3 segs in a row don't exist starting at key " << tempKey << "; ceasing extra gc'ing" << endl;
	       break;
	    }
	 } while (garbageCollectShmSegs(tempKey, size0, size1, size2));

	 continue; // we'll soon be successful
      }

      // not successful; try new keys
      shmSegKey += 10;
   }
}
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
   if (!proc->getActiveFrame(&frame_, &pc_)) {
      // failure
      frame_ = pc_ = 0;
   }
}

Frame Frame::getPreviousStackFrameInfo(process *proc) const {
   if (frame_ == 0)
      return *this; // no prev frame exists; not much we can do here

   int theFrame, thePc;
   if (!proc->readDataFromFrame(this->frame_, &theFrame, &thePc))
      ; // what to do here?

   return Frame(theFrame, thePc);
}

vector<Address> process::walkStack(bool noPause)
{
  bool needToCont = noPause ? false : (status() == running);

  vector<Address> pcs; // initially an empty array

  if (!noPause && !pause()) {
     // pause failed...give up
     cerr << "walkStack: pause failed" << endl;
     return pcs;
  }

  Frame currentFrame(this);
  while (!noPause && !currentFrame.isLastFrame()) {
      if (noPause) break;
      pcs += currentFrame.getPC();
      currentFrame = currentFrame.getPreviousStackFrameInfo(this); 
  }
  pcs += currentFrame.getPC();

  if (!noPause && needToCont) {
     if (!continueProc())
        cerr << "walkStack: continueProc failed" << endl;
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
  sprintf(errorLine, "IS ok called on 0x%x\n", ptr->addr);
  logLine(errorLine);
#endif

  const vector<unsigVecType> &disItemPoints = disItem.getPointsToCheck();
  const unsigned disItemNumPoints = disItemPoints.size();

  for (unsigned int j=0;j<disItemNumPoints;j++) {
    for (unsigned int k=0;k<disItemPoints[j].size();k++) {
      unsigned pointer = disItemPoints[j][k];
#ifdef FREEDEBUG1
      sprintf(errorLine, "checking 0x%x\n", pointer);
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
        assert(np);
        if ( (ptr->addr >= np->addr) && 
             (ptr->addr <= (np->addr + np->length)) )
        {

#ifdef FREEDEBUG1
          sprintf(errorLine,"*** TEST *** (pid=%d) IN isFreeOK: we found 0x%x in our inst. range!\n",proc->getPid(),ptr->addr);
          logLine(errorLine);
#endif

          return(false);     
        }

        for (unsigned int l=0;l<pcs.size();l++) {
          if ((pcs[l] >= ptr->addr) && 
              (pcs[l] <= (ptr->addr + ptr->length))) 
          {

#ifdef FREEDEBUG1
    sprintf(errorLine,"      IN isFreeOK: we found 0x%x in our inst. range!\n",ptr->addr);
    logLine(errorLine);
#endif

            return(false);
          }
          if ( ((pcs[l] >= np->addr) && (pcs[l] <= (np->addr + np->length))) )
          {

#ifdef FREEDEBUG1
    sprintf(errorLine,"*** TEST *** (pid=%d) IN isFreeOK: (2) we found PC in our inst. range!\n",proc->getPid());
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

#ifdef FREEDEBUG1
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
    hp->heapFree += np;
}

// create a new inferior heap that is a copy of src. This is used when a process
// we are tracing forks.
inferiorHeap::inferiorHeap(const inferiorHeap &src):
    heapActive(addrHash16)
{
    for (unsigned u = 0; u < src.heapFree.size(); u++) {
      heapFree += new heapItem(src.heapFree[u]);
    }

    vector<heapItem *> items = src.heapActive.values();
    for (unsigned u = 0; u < items.size(); u++) {
      heapActive[items[u]->addr] = new heapItem(items[u]);
    }
    
    for (unsigned u = 0; u < src.disabledList.size(); u++) {
      disabledList += src.disabledList[u];
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

#if !defined(sparc_tmc_cmost7_3)
      inferiorFreeDefered(proc, hp, true);
      inferiorFreeCompact(hp);
      secondChance=true;
#endif
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

#ifndef sparc_tmc_cmost7_3
    //
    // Note: This code will be executed on every platform except for the CM-5.
    // We are not going to delete instrumentation on the CM-5 for the time
    // being - naim 03/26/96
    //
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
#endif
}

process::process(int iPid, image *iImage
#ifdef SHM_SAMPLING
		 , key_t theShmKey,
		 unsigned iicNumElems,
		 unsigned iwtNumElems,
		 unsigned iptNumElems
#endif
) :
                     baseMap(ipHash), 
                     instInstanceMapping(instInstanceHash),
		     pid(iPid), // needed in fastInferiorHeap ctors below
#ifdef SHM_SAMPLING
		     inferiorIntCounters(this, theShmKey, iicNumElems),
		     inferiorWallTimers (this, theShmKey+1, iwtNumElems),
		     inferiorProcessTimers(this, theShmKey+2, iptNumElems),
#endif
                     firstRecordTime(0)
{
    hasBootstrapped = false;

    // this is the 'normal' ctor, when a proc is started fresh as opposed
    // to via a fork().
    symbols = iImage;

    traceLink = -1; ioLink = -1;

    status_ = neonatal;
    thread = 0;

    struct utsname un;
    P_uname(&un);
    string buffer = string(pid) + string("_") + string(un.nodename);
    rid = resource::newResource(processResource, // parent
				(void*)this, // handle
				nullString, // abstraction
				iImage->name(),
				0.0, // creation time
				buffer, // unique name (?)
				MDL_T_STRING // mdl type (?)
				);

    parent = NULL;
    bufStart = 0;
    bufEnd = 0;
    reachedFirstBreak = false; // process won't be paused until this is set
    splitHeaps = false;
    inExec = false;

    cumObsCost = 0;
    lastObsCostLow = 0;

    proc_fd = -1;

    trampTableItems = 0;
    memset(trampTable, 0, sizeof(trampTable));
    currentPC_ = 0;
    hasNewPC = false;
    
    inhandlestart = false;
    dynamiclinking = false;
    dyn = new dynamic_linking;
    shared_objects = 0;
    all_functions = 0;
    all_modules = 0;
    some_modules = 0;
    some_functions = 0;
    waiting_for_resources = false;

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

}

// This is the "fork" constructor:

process::process(const process &parentProc, int iPid
#ifdef SHM_SAMPLING
		 ,key_t theShmKey,
		 void *applShmSegIntCounterPtr,
		 void *applShmSegWallTimerPtr,
		 void *applShmSegProcTimerPtr
#endif
		 ) :
                     baseMap(ipHash), 
                     instInstanceMapping(instInstanceHash),
#ifdef SHM_SAMPLING
		     inferiorIntCounters(parentProc.inferiorIntCounters,
					 this, theShmKey, applShmSegIntCounterPtr),
		     inferiorWallTimers(parentProc.inferiorWallTimers,
					this, theShmKey+1, applShmSegWallTimerPtr),
		     inferiorProcessTimers(parentProc.inferiorProcessTimers,
					   this, theShmKey+2, applShmSegProcTimerPtr),
#endif
                     firstRecordTime(0)
{
    hasBootstrapped = true;

    // This is the "fork" ctor

    symbols = parentProc.symbols;

    /* initialize the traceLink to zero. The child process will get a 
       connection later.  (But when does ioLink get set???) */
    traceLink = -1;
    ioLink = -1;

    status_ = neonatal;
    pid = iPid; thread = 0;

    struct utsname un;
    P_uname(&un);
    string buffer = string(pid) + string("_") + string(un.nodename);
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

    reachedFirstBreak = false; // process won't be paused until this is set

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

    inhandlestart = false;
    dynamiclinking = false;
    dyn = new dynamic_linking;
    shared_objects = 0;
    all_functions = 0;
    all_modules = 0;
    some_modules = 0;
    some_functions = 0;
    waiting_for_resources = false;

#ifdef SHM_SAMPLING
#ifdef sparc_sun_sunos4_1_3
   childUareaPtr = NULL;
#endif
#endif

}

#ifdef SHM_SAMPLING
void process::registerInferiorAttachedSegs(intCounter *inferiorIntCounterPtr,
					   tTimer *inferiorWallTimerPtr,
					   tTimer *inferiorProcTimerPtr) {
cout << "welcome to register with intCounterPtr=" << inferiorIntCounterPtr << ", wallTimerPtr=" << inferiorWallTimerPtr << ", procTimerPtr=" << inferiorProcTimerPtr << endl;
cout.flush();

   inferiorIntCounters.setBaseAddrInApplic(inferiorIntCounterPtr);
   inferiorWallTimers.setBaseAddrInApplic(inferiorWallTimerPtr);
   inferiorProcessTimers.setBaseAddrInApplic(inferiorProcTimerPtr);
}

static key_t childShmKeyToUse;

#endif

/*
 * Create a new instance of the named process.  Read the symbols and start
 *   the program
 */
process *createProcess(const string File, vector<string> argv, vector<string> envp, const string dir = "")
{
    int r;
    unsigned i, j, k;
    int tracePipe[2];
    string inputFile, outputFile;
    string file = File;

    // prepend the directory (if any) to the file, unless the filename
    // starts with a /
    if (!file.prefixed_by("/") && dir.length() > 0)
      file = dir + "/" + file;

    // check for I/O redirection in arg list.
    for (i=0; i<argv.size(); i++) {
      if (argv[i] == "<") {
	inputFile = argv[i+1];
	for (j=i+2, k=i; j<argv.size(); j++, k++)
	  argv[k] = argv[j];
	argv.resize(argv.size()-2);
      }
    }
    // TODO -- this assumes no more than 1 of each "<", ">"
    for (i=0; i<argv.size(); i++) {
      if (argv[i] == ">") {
	outputFile = argv[i+1];
	for (j=i+2, k=i; j<argv.size(); j++, k++)
	  argv[k] = argv[j];
	argv.resize(argv.size()-2);
      }
    }

    // Strange, but using socketpair here doesn't seem to work OK on SunOS.
    // Pipe works fine.
    // r = P_socketpair(AF_UNIX, SOCK_STREAM, (int) NULL, tracePipe);
    r = P_pipe(tracePipe);
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

#ifdef SHM_SAMPLING
    // Do this _before_ the fork so both the parent and child get the updated
    // value of ::childShmKeyToUse
    ::childShmKeyToUse = pickShmSegKey();
#endif

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
	extern bool establishBaseAddrs(int pid, int &status);
	int status;

	if (!establishBaseAddrs(pid, status)) {
	    return(NULL);
	}
#endif

// NEW: We bump up batch mode here; the matching bump-down occurs after shared objects
//      are processed (after receiving the SIGSTOP indicating the end of running DYNINSTinit;
//      more specifically, tryToReadAndProcessBootstrapInfo()).
//      Prevents a diabolical w/w deadlock on solaris --ari
tp->resourceBatchMode(true);

	image *img = image::parseImage(file);
	if (!img) {
	    string msg = string("Unable to parse image: ") + file;
	    showErrorCallback(68, msg.string_of());
	    // destroy child process
	    P_kill(pid, 9);

	    return(NULL);
	}

	/* parent */
	statusLine("initializing process data structures");
	// sprintf(name, "%s", (char*)img->name);

//	cerr << "welcome to new process" << endl; cerr.flush();
//	kill(getpid(), SIGSTOP);
//	cerr << "doing new process with key=" << childShmKeyToUse << endl; cerr.flush();

	process *ret = new process(pid, img
#ifdef SHM_SAMPLING
				   , ::childShmKeyToUse,
				   numIntCounters,
				   numWallTimers,
				   numProcTimers
#endif
				   );
	   // change this to a ctor that takes in more args
	assert(ret);
	processVec += ret;
	activeProcesses++;
	if (!costMetric::addProcessToAll(ret))
	   assert(false);

	ret->initInferiorHeap(false);
	ret->splitHeaps = false;

#if defined(rs6000_ibm_aix3_2) || defined(rs6000_ibm_aix4_1)
	// XXXX - move this to a machine dependant place.

	// create a seperate text heap.
	ret->initInferiorHeap(true);
	ret->splitHeaps = true;
#endif

	ret->traceLink = tracePipe[0];
	ret->ioLink = ioPipe[0];
	close(tracePipe[1]);
	close(ioPipe[1]);
           // parent closes write end of io pipe; child closes its read end.
           // pipe output goes to the parent's read fd (ret->ioLink); pipe input
           // comes from the child's write fd.  In short, when the child writes to
           // its stdout/stderr, it gets sent to the pipe which in turn sends it to
           // the parent's ret->ioLink fd for reading.

	statusLine("ready");

	// attach to the child process
	ret->attach();

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

	// The child doesn't care to read anything from the ioPipe; it only writes
        // to it.  Hence we close ioPipe[0], the read end.  Then we call dup2() twice
        // to assign our (the child's) stdout and stderr to the write end of the pipe.
	close(ioPipe[0]);
	dup2(ioPipe[1], 1);
           // assigns fd 1 (stdout) to be a copy of ioPipe[1].  (Since stdout is already
           // in use, dup2 will first close it then reopen it with the characteristics)
	   // of ioPipe[1].
           // In short, stdout gets redirected towards the pipe.  Which brings up the
           // question of who receives such data.  The answer is the read end of the
           // pipe, which is attached to the parent process; we (the child of fork())
           // don't use it.


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
        // execution of main()]  So when do we do it?  At the program's main(), via
        // rtinst's DYNINSTinit (RTposix.c et al.)

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
	  for (int ep=envp.size()-1; ep>=0; ep--)
	    pvmputenv((char *)envp[ep].string_of());
#endif
        // hand off info about how to start a paradynd to the application.
	//   used to catch rexec calls, and poe events.
	//
	char paradynInfo[1024];
	sprintf(paradynInfo, "PARADYN_MASTER_INFO= ");
	for (i=0; i < process::arg_list.size(); i++) {
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

	/* put the traceSocket address in the environment. This will be used
	   by forked processes to get a connection with the daemon
	*/
	extern int traceSocket;
	string paradyndSockInfo = string("PARADYND_TRACE_SOCKET=") + string(traceSocket);
	P_putenv(paradyndSockInfo.string_of());

#ifdef SHM_SAMPLING
        // Although the parent (paradynd) incremented ::inferiorIntCounterKey et al,
        // we (the child) don't see those effects, so we'll get the old values, which
        // is exactly what we want.
	int keyInt = (int)::childShmKeyToUse;
	string paradyndShmSegIntCtr = string("PARADYND_SHMSEG_INTCTRKEY=") + string(keyInt);
	P_putenv(paradyndShmSegIntCtr.string_of());

	string paradyndShmSegIntCtrSize = string("PARADYND_SHMSEG_INTCTRSIZE=") + string(intCounterShmSegNumBytes(::numIntCounters));
	P_putenv(paradyndShmSegIntCtrSize.string_of());

        keyInt++;
	string paradyndShmSegWallTimer = string("PARADYND_SHMSEG_WALLTIMERKEY=") + string(keyInt);
	P_putenv(paradyndShmSegWallTimer.string_of());
	string paradyndShmSegWallTimerSize = string("PARADYND_SHMSEG_WALLTIMERSIZE=") + string(wallTimerShmSegNumBytes(::numWallTimers));
	P_putenv(paradyndShmSegWallTimerSize.string_of());

        keyInt++;
	string paradyndShmSegProcTimer = string("PARADYND_SHMSEG_PROCTIMERKEY=") + string(keyInt);
	P_putenv(paradyndShmSegProcTimer.string_of());
	string paradyndShmSegProcTimerSize = string("PARADYND_SHMSEG_PROCTIMERSIZE=") + string(procTimerShmSegNumBytes(::numProcTimers));
	P_putenv(paradyndShmSegProcTimerSize.string_of());
#endif

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

#ifdef SHM_SAMPLING
void process::doSharedMemSampling(unsigned long long theWallTime) {
   // Process Time:
   const unsigned long long theProcTime = this->getInferiorProcessCPUtime();
   
   inferiorIntCounters.processAll  (theWallTime, theProcTime);
   inferiorWallTimers.processAll   (theWallTime, theProcTime);
   inferiorProcessTimers.processAll(theWallTime, theProcTime);

   // Now do the observed cost.
   // WARNING: we should be using a mutex!
   unsigned *costAddr = this->getObsCostLowAddrInParadyndSpace();
   const unsigned theCost = *costAddr;

   this->processCost(theCost, theWallTime, theProcTime);
}
#endif

extern void removeFromMetricInstances(process *);
extern void disableAllInternalMetrics();

void handleProcessExit(process *proc, int exitStatus) {
  if (proc->status() == exited)
    return;

  proc->Exited();
  if (proc->traceLink >= 0) {
    processTraceStream(proc);
    P_close(proc->traceLink);
    proc->traceLink = -1;
  }
  if (proc->ioLink >= 0) {
    processAppIO(proc);
    P_close(proc->ioLink);
    proc->ioLink = -1;
  }
  removeFromMetricInstances(proc);
  --activeProcesses;
  if (activeProcesses == 0)
    disableAllInternalMetrics();

  proc->detach(false);

#ifdef PARADYND_PVM
  if (pvm_running) {
    PDYN_reportSIGCHLD(proc->getPid(), exitStatus);
  }
#endif

  // Shouldn't we call 'delete' on "proc", remove it from the global
  // list of all processes, etc.???
}


/*
   process::forkProcess: called when a process forks, to initialize a new
   process object for the child.
*/   
process *process::forkProcess(const process *theParent, pid_t childPid
#ifdef SHM_SAMPLING
			      ,key_t theShmSegBaseKey,
			      void *applShmSegIntCounterPtr,
			      void *applShmSegWallTimerPtr,
			      void *applShmSegProcTimerPtr
#endif
			      ) {
    //process *ret = allocateProcess(childPid, theParent->symbols->name());

    process *ret = new process(*theParent, childPid
#ifdef SHM_SAMPLING
			       , theShmSegBaseKey,
			       applShmSegIntCounterPtr,
			       applShmSegWallTimerPtr,
			       applShmSegProcTimerPtr
#endif
			       );

       // change this to a "fork" ctor that takes in more args
    assert(ret);
    processVec += ret;
    activeProcesses++;
    if (!costMetric::addProcessToAll(ret))
       assert(false);

    /* attach to child */
    if (!ret->attach()) {
      showErrorCallback(69, "Error in forkprocess: cannot attach to child process");
      return 0;
    }

    /* all instrumentation on the parent is active on the child */
    /* TODO: what about instrumentation inserted near the fork time??? */
    ret->baseMap = theParent->baseMap;

    /* copy all instrumentation instances of the parent to the child */
    /* this will update instMapping */
    copyInstInstances(theParent, ret, ret->instInstanceMapping);

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

/* process::handleExec: called when a process exec.
   Parse the new image and disable metric instances on the old image.
*/
void process::handleExec() {

    // all instrumentation that was inserted in this process is gone.
    // set exited here so that the disables won't try to write to process
    status_ = exited; 

    removeFromMetricInstances(this);

    image *img = image::parseImage(execFilePath);
    if (!img) {
       string msg = string("Unable to parse image: ") + execFilePath;
       showErrorCallback(68, msg.string_of());
       P_kill(pid, 9);
       return;
    }

    // delete proc->symbols ???
    symbols = img;

    /* update process status */
    reachedFirstBreak = false;
    status_ = stopped;
}

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
    string msg = string("System error: unable to write to process data space:")
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
    string msg = string("System error: unable to write to process data space:")
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
    // first break.
  }

  return true;
}

//
//  handler for a trace record of type TR_START
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
    p->getSharedObjects();

//    // either continue the process here (if the SIGSTOP has already been
//    // caught by the daemon), or wait and continue it when the SIGSTOP
//    // is received
//    bool needToCont = (p->status() == running);
//    if (needToCont){ 
//	// this means that the signal from the child process has not been
//	// caught yet...set flag that will be tested for in handleSigChild
//	// which will continue the child process
//	p->inhandlestart = true;
//	int status;
//	int waiting_pid = process::waitProcs(&status);
//	if (waiting_pid > 0) {
//	    extern int handleSigChild(int,int);
//	    handleSigChild(waiting_pid, status);
//	}
//    }
//    else {
//	p->status_ = stopped;
//	// if there are no outstanding resource responses from Paradyn
//	// then continure the process, otherwise set flag to continue
//	// the process when all outstanding creates have completed

	if(!resource::num_outstanding_creates){
	    //p->continueProc();
	}
	else {
	   p->setWaitingForResources();
	}
//    }

    return true;
}

//  Executes on the exit point of the exec: does any necessary initialization
//  for the run time linker to export dynamic linking information
//  returns true if the executable is dynamic
//
bool process::findDynamicLinkingInfo(){
    dynamiclinking = dyn->findDynamicLinkingInfo(this);
    return dynamiclinking;
}

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
	statusLine("ready");

	tp->resourceBatchMode(false);
	return true;
    }
    // else this a.out does not have a .dynamic section
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
      
bool process::continueProc() {
  if (status_ == exited) return false;

  if (status_ != stopped && status_ != neonatal) {
    showErrorCallback(38, "Internal paradynd error in process::continueProc");
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
    logLine("detach: pause not implemented\n");
  }
  bool res = detach_();
  if (!res) {
    // process may have exited
    return false;
  }
  return true;
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
    int pc = frame.getPC();

    // Go thru the instWList to find out the ones to be deleted 
    instWaitingList *instW;
    if ((instW = instWList.find((void *)pc)) != NULL) {
        instW->cleanUp(this, pc);
	instWList.remove(instW);

	// we'll be returning true
	if (wasRunning)
	   continueProc();

	return true;
    }
    else {
       return false;
    }
}

void process::postRPCtoDo(const AstNode &action, bool noCost,
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

bool process::launchRPCifAppropriate(bool wasRunning) {
   // asynchronously launches iff RPCsWaitingToStart.size() > 0 AND
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
         // I honestly don't know what the hell to do in this case, but we sure
         // can't expect the process to be able to execute any more code!

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

   void *theSavedRegs = getRegisters(); // machine-specific implementation
      // result is allocated via new[]; we'll delete[] it later.
      // return value of NULL indicates total failure.
      // return value of (void *)-1 indicates that the state of the machine isn't quite
      //    ready for an inferiorRPC, and that we should try again 'later'.  In particular,
      //    we must handle the (void *)-1 case very gracefully (i.e., leave
      //    the vrble 'RPCsWaitingToStart' untouched).

   if (theSavedRegs == (void *)-1) {
      cerr << "launchRPCifAppropriate: deferring" << endl;
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
#if defined(hppa1_1_hp_hpux)
if (wasRunning)
  (void)continueProc();
return false;
#endif

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

   // change the PC and nPC registers to the addr of the temp tramp
   if (!changePC(tempTrampBase, theSavedRegs)) {
      cerr << "launchRPCifAppropriate failed because changePC() failed" << endl;
      if (wasRunning)
	 (void)continueProc();
      return false;
   }

   if (!continueProc()) {
      cerr << "launchRPCifAppropriate: continueProc() failed" << endl;
      return false;
   }

   return true; // success
}

unsigned process::createRPCtempTramp(const AstNode &action,
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
   memset(insnBuffer, 0x00, sizeof(insnBuffer)); // aids debugging

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

   reg resultReg = action.generateCode(this, regSpace,
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
   // If we determine that the PC of a level of the back trace
   // falls within the bounds of [currRunningRPCs[0]'s address range],
   // then we assume success.  Note that we could probably narrow
   // it down to an EXACT address, to increase resistence to spurious
   // signals.
   Frame theFrame(this);
   while (true) {
      if (theFrame.isLastFrame())
	 // well, we've gone as far as we can, with no match.
	 return false;

      // do we have a match?
      const int framePC = theFrame.getPC();
      if (framePC >= currRunningRPCs[0].firstPossibleBreakAddr &&
	  framePC <= currRunningRPCs[0].lastPossibleBreakAddr) {
	 // we've got a match!
	 break;
      }

      // else, backtrace 1 more level
      theFrame = theFrame.getPreviousStackFrameInfo(this);
   }

   // Okay, we have a match.
   inferiorRPCinProgress theStruct = currRunningRPCs.removeByIndex(0);
//   assert(theStruct.trapInstrAddr == currPCreg);

   // step 1) restore registers:
   if (!restoreRegisters(theStruct.savedRegs)) {
      cerr << "handleTrapIfDueToRPC failed because restoreRegisters failed" << endl;
      assert(false);
   }
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

bool process::tryToReadAndProcessBootstrapInfo() {
   // returns true iff we are now processing the bootstrap info.
   // if false is returned, there must be no side effects.

   if (hasBootstrapped)
      return false;

   string vrbleName = "DYNINST_bootstrap_info";
   
   internalSym *sym = findInternalSymbol(vrbleName, true);
   assert(sym);

   Address symAddr = sym->getAddr();

   // Read the structure; if pid 0 then not yet written!
   DYNINST_bootstrapStruct bs_record;
   if (!readDataSpace((const void*)symAddr, sizeof(bs_record), &bs_record, true)) {
      cerr << "tryToReadAndProcessBootstrapInfo failed because readDataSpace failed" << endl;
      return false;
   }

   if (bs_record.pid == 0)
      return false;

   string str=string("PID=") + string(bs_record.pid) + ", receiving bootstrap info...";
   statusLine(str.string_of());

   process *checkProc = findProcess(bs_record.pid);
   assert(checkProc);
   assert(checkProc == this); // just to be sure

#ifdef SHM_SAMPLING
   registerInferiorAttachedSegs(bs_record.intCounterAttachedAt,
				bs_record.wallTimerAttachedAt,
				bs_record.procTimerAttachedAt);
#endif

   str=string("PID=") + string(bs_record.pid) + ", calling handleStartProcess...";
   statusLine(str.string_of());

   if (!handleStartProcess(this))
      logLine("warning: handleStartProcess failed\n");

// NEW: we decrement the batch mode here; the matching bump-up occurs in createProcess()
tp->resourceBatchMode(false);


   str=string("PID=") + string(bs_record.pid) + ", installing default inst...";
   statusLine(str.string_of());

   extern vector<instMapping*> initialRequests; // init.C
   installDefaultInst(this, initialRequests);

   str=string("PID=") + string(bs_record.pid) + ", propagating mi's...";
   statusLine(str.string_of());

   // propagate any metric that is already enabled to the new process.
   vector<metricDefinitionNode *> MIs = allMIs.values();
   for (unsigned j = 0; j < MIs.size(); j++) {
      MIs[j]->propagateMetricInstance(this);
   }

   costMetric::addProcessToAll(this);

   hasBootstrapped = true;

   str=string("PID=") + string(bs_record.pid) + ", executing new-prog callback...";
   statusLine(str.string_of());

   tp->newProgramCallbackFunc(bs_record.pid, this->arg_list, 
			      machineResource->part_name());
      // in paradyn, this will call paradynDaemon::addRunningProgram()

   // The following call must be done before any samples are sent to
   // paradyn; otherwise, prepare for an assert fail.

   const time64 currWallTime = getCurrWallTime();

   if (!firstRecordTime) {
      //cerr << "process.C setting firstRecordTime to " << currWallTime << endl;
      firstRecordTime = currWallTime; // firstRecordTime may soon be obsolete; for now, it's used in metric.C and maybe perfStream.C
   }
   if (!::firstRecordTime) {
      //cerr << "process.C setting ::firstRecordTime to " << currWallTime << endl;
     ::firstRecordTime = currWallTime;
   }

   tp->firstSampleCallback(getPid(), (double)currWallTime / 1000000.0);
	       
   str=string("PID=") + string(bs_record.pid) + ", ready.";
   statusLine(str.string_of());

   return true;
}

void process::continueProcessIfWaiting(){ // called by dynrpc.C ::resourceInfoResponse
   if(waiting_for_resources){
      //continueProc(); (obsolete, hence waiting_for_resources may be obsolete)
   }
   
   waiting_for_resources = false;
}
