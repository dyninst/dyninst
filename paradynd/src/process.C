/*
 *  Copyright 1993 Jeff Hollingsworth.  All rights reserved.
 *
 */

#ifndef lint
char process_Copyright[] = "@(#) Copyright (c) 1993 Jeff Hollingsowrth\
    All rights reserved.";

char process_rcsid[] = "@(#) /p/paradyn/CVSROOT/core/paradynd/src/process.C,v 1.26 1995/05/18 10:41:09 markc Exp";
#endif

/*
 * process.C - Code to control a process.
 *
 * $Log: process.C,v $
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
 * Revision 1.54  1996/05/10 21:38:43  naim
 * Chaning some parameters for instrumentation deletion - naim
 *
 * Revision 1.53  1996/05/10  13:51:33  naim
 * Changes to improve the instrumentation deletion process - naim
 *
 * Revision 1.52  1996/05/10  06:55:50  tamches
 * isFreeOK now takes in references as its last 2 args
 * calls to disabledItem member fns, now that its data are private
 * call to dictionary's find() instead of defines() & operator[].
 *
 * Revision 1.51  1996/05/08 23:55:03  mjrg
 * added support for handling fork and exec by an application
 * use /proc instead of ptrace on solaris
 * removed warnings
 *
 * Revision 1.50  1996/05/08 19:46:44  naim
 * inferiorFreeDefered does not execute on the CM-5 - naim
 *
 * Revision 1.49  1996/05/08  19:36:19  naim
 * Yet another fix to my previous fix! - naim
 *
 * Revision 1.48  1996/05/08  19:30:51  naim
 * Minor fix - naim
 *
 * Revision 1.47  1996/05/08  18:25:58  naim
 * Eliminating warning messages - naim
 *
 * Revision 1.46  1996/05/08  18:24:32  naim
 * Eliminating some minor warning messages - naim
 *
 * Revision 1.45  1996/05/06  13:48:47  naim
 * Fixing problem with deletion of instrumentation and adding procedure to
 * compact memory when we run out of space to insert more instrumentation - naim
 *
 * Revision 1.44  1996/05/01  19:04:31  naim
 * Adding debugging information during the deletion of instrumentation - naim
 *
 * Revision 1.43  1996/04/24  15:00:34  naim
 * Fixing misspelling error - naim
 *
 * Revision 1.42  1996/04/22  16:10:25  naim
 * Fixing bug: making sure that application was running before calling
 * continueProc in procedure walkStack - naim
 *
 * Revision 1.41  1996/04/18  22:06:07  naim
 * Adding parameters that control and delay (when necessary) the deletion
 * of instrumentation. Also, some minor misspelling fixes - naim
 *
 * Revision 1.40  1996/04/08  21:25:18  lzheng
 * inferiorFreeDefered is not called for HP, since it's not yet
 * implemented for HP.
 *
 * Revision 1.39  1996/04/06 21:25:28  hollings
 * Fixed inst free to work on AIX (really any platform with split I/D heaps).
 * Removed the Line class.
 * Removed a debugging printf for multiple function returns.
 *
 * Revision 1.38  1996/04/03  14:27:52  naim
 * Implementation of deallocation of instrumentation for solaris and sunos - naim
 *
 * Revision 1.37  1996/03/14  14:23:42  naim
 * Minor change - naim
 *
 * Revision 1.36  1996/03/12  20:48:36  mjrg
 * Improved handling of process termination
 * New version of aggregateSample to support adding and removing components
 * dynamically
 * Added error messages
 *
 * Revision 1.35  1996/03/05 18:53:22  mjrg
 * Replaced socketpair with pipe.
 * Removed compiler warning.
 *
 * Revision 1.34  1996/03/01 22:37:19  mjrg
 * Added a type to resources.
 * Added function handleProcessExit to handle exiting processes.
 *
 * Revision 1.33  1996/02/13 06:17:34  newhall
 * changes to how cost metrics are computed. added a new costMetric class.
 *
 * Revision 1.32  1995/12/15  22:26:57  mjrg
 * Merged paradynd and paradyndPVM
 * Get module name for functions from symbol table in solaris
 * Fixed code generation for multiple instrumentation statements
 * Changed syntax of MDL resource lists
 *
 * Revision 1.31  1995/11/28 15:56:56  naim
 * Minor fix. Changing char[number] by string - naim
 *
 * Revision 1.30  1995/10/19  22:36:44  mjrg
 * Added callback function for paradynd's to report change in status of application.
 * Added Exited status for applications.
 * Removed breakpoints from CM5 applications.
 * Added search for executables in a given directory.
 *
 * Revision 1.29  1995/09/26  20:17:51  naim
 * Adding error messages using showErrorCallback function for paradynd
 *
 */

extern "C" {
#ifdef PARADYND_PVM
int pvmputenv (const char *);
int pvmendtask();
#endif
}

#include "rtinst/h/rtinst.h"
#include "rtinst/h/trace.h"
#include "symtab.h"
#include "process.h"
#include "util.h"
#include "inst.h"
#include "dyninstP.h"
#include "os.h"
#include "showerror.h"
#include "costmetrics.h"
#include "perfStream.h"

#define FREE_WATERMARK (hp->totalFreeMemAvailable/2)
#define SIZE_WATERMARK 100
static const timeStamp MAX_WAITING_TIME=10.0;
static const timeStamp MAX_DELETING_TIME=2.0;

unsigned activeProcesses; // number of active processes
vector<process*> processVec;
string process::programName;
vector<string> process::arg_list;

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

vector<Address> process::walkStack() 
{
  Frame currentFrame;
  vector<Address> pcs;
  bool needToCont = (status() == running);

  if (pause()) {
    currentFrame.getActiveStackFrameInfo(this);
    while (!currentFrame.isLastFrame()) {
      pcs += currentFrame.getPC();
      currentFrame = currentFrame.getPreviousStackFrameInfo(this); 
    }
    pcs += currentFrame.getPC();
    if (needToCont) continueProc();
  }
  return(pcs);
}  

bool isFreeOK(process *proc, const disabledItem &disItem, vector<Address> &pcs) {
  const unsigned disItemPointer = disItem.getPointer();
  const inferiorHeapType disItemHeap = disItem.getHeapType();

  heapItem *ptr=NULL;
  if (!proc->heaps[disItemHeap].heapActive.find(disItemPointer, ptr)) {
    sprintf(errorLine,"Warning: attempt to free not defined heap entry %x (pid=%d, heapActive.size()=%d)\n", disItemPointer, proc->pid, proc->heaps[disItemHeap].heapActive.size());
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

      const dictionary_hash<unsigned, heapItem*> &heapActivePart = proc->splitHeaps ?
	                                                           proc->heaps[textHeap].heapActive :
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
          sprintf(errorLine,"*** TEST *** (pid=%d) IN isFreeOK: we found 0x%x in our inst. range!\n",proc->pid,ptr->addr);
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
    sprintf(errorLine,"*** TEST *** (pid=%d) IN isFreeOK: (2) we found PC in our inst. range!\n",proc->pid);
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
    sprintf(errorLine,"Emergency attempt to free memory (pid=%d). Please, wait...\n",proc->pid);
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

void initInferiorHeap(process *proc, bool globalHeap, bool initTextHeap)
{
    heapItem *np;
    inferiorHeap *hp;
    bool err;

    assert(proc->symbols);

    if (initTextHeap) {
	hp = &proc->heaps[textHeap];
    } else {
	hp = &proc->heaps[dataHeap];
    }

    np = new heapItem;
    if (initTextHeap) {
	np->addr = 
	  (proc->symbols)->findInternalAddress("DYNINSTtext", true,err);
	if (err)
	  abort();
    } else if (globalHeap) {
	np->addr = 
	  (proc->symbols)->findInternalAddress(GLOBAL_HEAP_BASE, true,err);
	if (err)
	  abort();
    } else {
	np->addr = 
	  (proc->symbols)->findInternalAddress(INFERIOR_HEAP_BASE, true, err);
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
    sprintf(errorLine,"***** (pid=%d) i=%d, addr=%d, length=%d, heapFree.size()=%d, size=%d\n",proc->pid,i,(hp->heapFree[i])->addr,(hp->heapFree[i])->length,hp->heapFree.size(),size); 
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

#if !defined(hppa1_1_hp_hpux) && !defined(sparc_tmc_cmost7_3)
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
                  vector<unsigVecType> &pointsToCheck)
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
#if !defined(hppa1_1_hp_hpux)
      inferiorFreeDefered(proc, hp, false);
#endif
#ifdef FREEDEBUG
t2=getCurrentTime(false);
if (!t3) t3=t1;
counter++;
totalTime += t2-t1;
if ((t2-t1) > worst) worst=t2-t1;
if ((float)(t2-t1) > 1.0) {
  sprintf(errorLine,">>>> TEST <<<< (pid=%d) inferiorFreeDefered took %5.2f secs, avg=%5.2f, worst=%5.2f, heapFree=%d, heapActive=%d, disabledList=%d, last call=%5.2f\n", proc->pid,(float) (t2-t1), (float) (totalTime/counter), (float)worst, hp->heapFree.size(), hp->heapActive.size(), hp->disabledList.size(), (float)(t1-t3));
  logLine(errorLine);
}
t3=t1;
#endif

    }
#endif
}

process *allocateProcess(int pid, const string name)
{
    process *ret;

    ret = new process;
    processVec += ret;
    ++activeProcesses;
    if(!costMetric::addProcessToAll(ret)) assert(0);

    ret->pid = pid;

    string buffer;
    struct utsname un;
    P_uname(&un);
    buffer = string(pid) + string("_") + string(un.nodename);
    ret->rid = resource::newResource(processResource, (void*)ret, nullString, name,
				     0.0, P_strdup(buffer.string_of()), MDL_T_STRING);
    ret->bufEnd = 0;

    // this process won't be paused until this flag is set
    ret->reachedFirstBreak = false;
    return(ret);
}

/*
 * Create a new instance of the named process.  Read the symbols and start
 *   the program
 */
process *createProcess(const string File, vector<string> argv, vector<string> envp, const string dir = "")
{
    int r;
    int fd;
    int pid;
    image *img;
    unsigned i, j, k;
    process *ret=0;
    int tracePipe[2];
    FILE *childError;
    string inputFile, outputFile;
    string file = File;

    if ((!file.prefixed_by("/")) && (dir.length() > 0)) {
      file = dir + "/" + file;
    }

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
    //
    // WARNING This code assumes that vfork is used, and a failed exec will
    //   corectly change failed in the parent process.
    //
    errno = 0;
#ifdef PARADYND_PVM
// must use fork, since pvmendtask will do some writing in the address space
    pid = fork();
    // fprintf(stderr, "FORK: pid=%d\n", pid);
#else
    pid = vfork();
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

	img = image::parseImage(file);
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
	ret = allocateProcess(pid, img->name());
	ret->symbols = img;

	initInferiorHeap(ret, false, false);
	ret->splitHeaps = false;

#if defined(rs6000_ibm_aix3_2) || defined(rs6000_ibm_aix4_1)
	// XXXX - move this to a machine dependant place.

	// create a seperate text heap.
	initInferiorHeap(ret, false, true);
	ret->splitHeaps = true;
#endif

	ret->status_ = neonatal;
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
        // do it here, since the upcoming execve() would undo our work [actually, the man
        // page for execve claims fd's are left alone so it must be some stdio bootstrap code
        // called before main]  So when do we do it?  At the program's main(), via
        // rtinst's DYNINSTinit (RTposix.c et al.)

	// setup stderr for rest of exec try.
	childError = P_fdopen(2, "w");

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
	    fd = P_open(inputFile.string_of(), O_RDONLY, 0);
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
	    fd = P_open(outputFile.string_of(), O_WRONLY|O_CREAT, 0444);
	    if (fd < 0) {
		fprintf(childError, "stdout open of %s failed\n", outputFile.string_of());
		fflush(childError);
		P__exit(-1);
	    } else {
		dup2(fd, 1); // redirect fd 1 (stdout) to a copy of descriptor "fd"
		P_close(fd); // not using descriptor fd any more; close it.
	    }
	}

	/* indicate our desire to be trace */
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
	putenv(paradynInfo);

	/* put the traceSocket address in the environment. This will be used
	   by forked processes to get a connection with the daemon
	*/
	extern int traceSocket;
	string paradyndSockInfo = string("PARADYND_TRACE_SOCKET=") + string(traceSocket);
	putenv((char *)paradyndSockInfo.string_of());

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
	free(ret);
	return(NULL);
    }
}

extern void removeFromMetricInstances(process *);
extern void disableAllInternalMetrics();

void handleProcessExit(process *proc, int exitStatus) {
  if (proc->status() == exited)
    return;

  proc->Exited();
  removeFromMetricInstances(proc);
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
  --activeProcesses;
  if (activeProcesses == 0)
    disableAllInternalMetrics();

  proc->detach(false);

#ifdef PARADYND_PVM
  if (pvm_running) {
    PDYN_reportSIGCHLD(proc->getPid(), exitStatus);
  }
#endif
}


/*
   process::forkProcess: called when a process forks, to initialize a new
   process object for the child.
*/   
process *process::forkProcess(process *parent, pid_t childPid) {
    process *ret = allocateProcess(childPid, parent->symbols->name());
    ret->symbols = parent->symbols;

    /* initialize the traceLink to zero. The child process will get a 
       connection later. */
    ret->traceLink = -1;
    ret->ioLink = -1;
    ret->parent = parent;
  
    /* attach to child */
    if (!ret->attach()) {
      showErrorCallback(69, "Error in forkprocess: cannot attach to child process");
      return 0;
    }

    /* initialize the heaps */
    ret->splitHeaps = parent->splitHeaps;
    ret->heaps[0] = inferiorHeap(parent->heaps[0]);
    ret->heaps[1] = inferiorHeap(parent->heaps[1]);

    /* all instrumentation on the parent is active on the child */
    /* TODO: what about instrumentation inserted near the fork time??? */
    ret->baseMap = parent->baseMap;

    /* copy all instrumentation instances of the parent to the child */
    /* this will update instMapping */
    copyInstInstances(parent, ret, ret->instInstanceMapping);

    /* update process status */
    ret->reachedFirstBreak = false;
    ret->status_ = neonatal;

    return ret;
}


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

