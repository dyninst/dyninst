/*
 *  Copyright 1993 Jeff Hollingsworth.  All rights reserved.
 *
 */

#ifndef lint
static char Copyright[] = "@(#) Copyright (c) 1993 Jeff Hollingsowrth\
    All rights reserved.";

static char rcsid[] = "@(#) /p/paradyn/CVSROOT/core/paradynd/src/process.C,v 1.26 1995/05/18 10:41:09 markc Exp";
#endif

/*
 * process.C - Code to control a process.
 *
 * $Log: process.C,v $
 * Revision 1.37  1996/03/14 14:23:42  naim
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
 * Revision 1.28  1995/09/18  22:41:36  mjrg
 * added directory command.
 *
 * Revision 1.27  1995/08/24  15:04:29  hollings
 * AIX/SP-2 port (including option for split instruction/data heaps)
 * Tracing of rexec (correctly spawns a paradynd if needed)
 * Added rtinst function to read getrusage stats (can now be used in metrics)
 * Critical Path
 * Improved Error reporting in MDL sematic checks
 * Fixed MDL Function call statement
 * Fixed bugs in TK usage (strings passed where UID expected)
 *
 * Revision 1.26  1995/05/18  10:41:09  markc
 * Changed process dict to process map
 *
 * Revision 1.25  1995/02/26  22:48:50  markc
 * vector.size() returns an unsigned.  If the vector is to be traversed in reverse,
 * the bounds check cannot be > 0 since unsigned(0) - 1 is not negative.
 *
 * Revision 1.24  1995/02/16  08:54:00  markc
 * Corrected error in comments -- I put a "star slash" in the comment.
 *
 * Revision 1.23  1995/02/16  08:34:34  markc
 * Changed igen interfaces to use strings/vectors rather than char igen-arrays
 * Changed igen interfaces to use bool, not Boolean.
 * Cleaned up symbol table parsing - favor properly labeled symbol table objects
 * Updated binary search for modules
 * Moved machine dependnent ptrace code to architecture specific files.
 * Moved machine dependent code out of class process.
 * Removed almost all compiler warnings.
 * Use "posix" like library to remove compiler warnings
 *
 * Revision 1.22  1994/11/11  10:44:12  markc
 * Remove non-emergency prints
 * Changed others to use statusLine
 *
 * Revision 1.21  1994/11/09  18:40:33  rbi
 * the "Don't Blame Me" commit
 *
 * Revision 1.20  1994/11/02  11:15:17  markc
 * Started to make process into a class.
 *
 * Revision 1.19  1994/10/13  07:24:56  krisna
 * solaris porting and updates
 *
 * Revision 1.18  1994/09/22  02:23:17  markc
 * changed *allocs to new
 *
 * Revision 1.17  1994/08/17  18:17:43  markc
 * Changed execv to execvp.
 *
 * Revision 1.16  1994/07/26  20:01:41  hollings
 * fixed heap allocation to use hash tables.
 *
 * Revision 1.15  1994/07/20  23:23:39  hollings
 * added insn generated metric.
 *
 * Revision 1.14  1994/07/14  23:29:03  hollings
 * Corrected file mask on io redirection.
 *
 * Revision 1.13  1994/06/29  02:52:47  hollings
 * Added metricDefs-common.{C,h}
 * Added module level performance data
 * cleanedup types of inferrior addresses instrumentation defintions
 * added firewalls for large branch displacements due to text+data over 2meg.
 * assorted bug fixes.
 *
 * Revision 1.12  1994/06/27  21:28:18  rbi
 * Abstraction-specific resources and mapping info
 *
 * Revision 1.11  1994/06/27  18:57:07  hollings
 * removed printfs.  Now use logLine so it works in the remote case.
 * added internalMetric class.
 * added extra paramter to metric info for aggregation.
 *
 * Revision 1.10  1994/06/22  03:46:32  markc
 * Removed compiler warnings.
 *
 * Revision 1.9  1994/06/22  01:43:18  markc
 * Removed warnings.  Changed bcopy in inst-sparc.C to memcpy.  Changed process.C
 * reference to proc->status to use proc->heap->status.
 *
 * Revision 1.8  1994/05/31  17:59:05  markc
 * Closed iopipe fd that had been dup'd if the fd was greater than 2.
 *
 * Revision 1.7  1994/05/18  00:52:31  hollings
 * added ability to gather IO from application processes and forward it to
 * the paradyn proces.
 *
 * Revision 1.6  1994/05/16  22:31:53  hollings
 * added way to request unique resource name.
 *
 * Revision 1.5  1994/03/31  02:00:35  markc
 * Changed to fork for paradyndPVM since client calls pvmendtask which writes
 * to the address space.
 *
 * Revision 1.4  1994/03/22  21:03:15  hollings
 * Made it possible to add new processes (& paradynd's) via addExecutable.
 *
 * Revision 1.3  1994/03/20  01:53:11  markc
 * Added a buffer to each process structure to allow for multiple writers on the
 * traceStream.  Replaced old inst-pvm.C.  Changed addProcess to return type
 * int.
 *
 * Revision 1.2  1994/02/05  23:09:56  hollings
 * Added extern for sys_errlist[] (g++ version 2.5.7).
 *
 * Revision 1.1  1994/01/27  20:31:38  hollings
 * Iinital version of paradynd speaking dynRPC igend protocol.
 *
 * Revision 1.8  1993/10/04  21:38:41  hollings
 * round inferrior mallocs to cache line size.
 *
 * Revision 1.7  1993/08/23  23:15:25  hollings
 * added code to third parameter to findInternalAddress calls.
 *
 * Revision 1.6  1993/08/11  01:47:09  hollings
 * added copyInferrior heap for UNIX fork.
 *
 * Revision 1.5  1993/07/13  18:29:38  hollings
 * new include file syntax.
 *
 * Revision 1.4  1993/06/28  23:13:18  hollings
 * fixed process stopping.
 *
 * Revision 1.3  1993/06/22  19:00:01  hollings
 * global inst state.
 *
 * Revision 1.2  1993/06/08  20:14:34  hollings
 * state prior to bc net ptrace replacement.
 *
 * Revision 1.1  1993/03/19  22:45:45  hollings
 * Initial revision
 *
 *
 */

extern "C" {
#ifdef PARADYND_PVM
int pvmputenv (char *);
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

unsigned activeProcesses; // number of active processes
vector<process*> processVec;
string process::programName;
vector<string> process::arg_list;

void initInferiorHeap(process *proc, bool globalHeap, bool textHeap)
{
    heapItem *np;
    bool err;

    assert(proc->symbols);

    np = new heapItem;
    if (textHeap) {
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


    if (textHeap) {
	proc->textHeapFree += np;
    } else {
	proc->dataHeapFree += np;
    }
}

void copyInferiorHeap(process *from, process *to)
{
    abort();

#ifdef notdef
    heapItem *curr;
    heapItem *newEntry;

    // not done jkh 7/22/94

    assert(from->symbols);
    assert(to->symbols);

    // to->heapActive = NULL;
    /* copy individual elements */
    for (curr=from->heap; curr; curr=curr->next) {
	newEntry = new heapItem;
	*newEntry = *curr;

	/* setup next pointers */
	newEntry->next = to->heap;
	to->heap = newEntry;
    }
    to->heap->status = HEAPfree;
#endif
}

unsigned inferiorMalloc(process *proc, int size, inferiorHeapType type)
{
    vector<heapItem*> *heapFree;
    dictionary_hash<unsigned, heapItem*> *heapActive;
    heapItem *np=NULL, *newEntry = NULL;
    
    assert(size > 0);
    /* round to next cache line size */
    /* 32 bytes on a SPARC */
    size = (size + 0x1f) & ~0x1f; 

    if ((type == textHeap) && (proc->splitHeaps)) {
	heapFree = &proc->textHeapFree;
	heapActive = &proc->textHeapActive;
    } else {
	heapFree = &proc->dataHeapFree;
	heapActive = &proc->dataHeapActive;
    }

    unsigned foundIndex; bool found=false; 
    for (unsigned i=0; i < heapFree->size(); i++) {
      if (((*heapFree)[i])->length >= size) {
	np = (*heapFree)[i];
	found = true;
	foundIndex = i;
	break;
      }
    }

    if (!found) {
	logLine("Inferior heap overflow\n");
	sprintf(errorLine, "%d bytes freed\n", proc->freed);
	sprintf(errorLine, "%d bytes requested\n", size);
	logLine(errorLine);
	showErrorCallback(66, (const char *) errorLine);
        fflush(stdout);
        P__exit(-1);
	//abort();
    }

    if (np->length != size) {
	// divide it up.
	newEntry = new heapItem;
	newEntry->length = np->length - size;
	newEntry->addr = np->addr + size;
	
	// overwrite the old entry
	(*heapFree)[foundIndex] = newEntry;

	/* now split curr */
	np->length = size;
    } else {
      unsigned i = heapFree->size();
      // copy the last element over this element
      if (foundIndex < (i-1)) {
	(*heapFree)[foundIndex] = (*heapFree)[i-1];
	heapFree->resize(i-1);
      } else if (i == 1) {
	logLine("Inferior heap overflow\n");
	abort();
      }
    }

    // mark it used
    np->status = HEAPallocated;

    // onto in use list
    (*heapActive)[np->addr] = np;
    return(np->addr);
}

void inferiorFree(process *proc, unsigned pointer, inferiorHeapType type)
{
    heapItem *np;
    vector<heapItem*> *heapFree;
    dictionary_hash<unsigned, heapItem*> *heapActive;

    if ((type == textHeap) && (proc->splitHeaps)) {
	heapFree = &proc->textHeapFree;
	heapActive = &proc->textHeapActive;
    } else {
	heapFree = &proc->dataHeapFree;
	heapActive = &proc->dataHeapActive;
    }

    if (!heapActive->defines(pointer))
      abort();
    np = (*heapActive)[pointer];
    proc->freed += np->length;
#ifdef notdef
    /* free is currently disabled because we can't handle the case of an
     *  inst function being deleted while active.  Not freeing the memory means
     *  it stil contains the tramp and will get itself out safely.
     */

    if (np->status != HEAPallocated) {
      char buffer[40];
      logLine("attempt to free already free heap entry %x\n", pointer);
      showErrorCallback(67, string("Internal error: attempt to free already free heap entry ") + string(sprintf(buffer,"%x",pointer))); 
      abort();
    }
    np->status = HEAPfree;

    // remove from active list.
    proc->heapActive.undef(pointer);

    // back onto free list.
    proc->heapFree += pointer;
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
    int ioPipe[2];
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

#if defined(rs6000_ibm_aix3_2)
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

#if defined(rs6000_ibm_aix3_2)
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
	statusLine("ready");

#if defined(rs6000_ibm_aix3_2)
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
	close(ioPipe[0]);
	dup2(ioPipe[1], 1);
	dup2(ioPipe[1], 2);
	if (ioPipe[1] > 2) close (ioPipe[1]);

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
		dup2(fd, 1);
		P_close(fd);
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

	    pvmputenv(envp[ep].string_of());
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

	char **args;
	args = new char*[argv.size()+1];
	for (unsigned ai=0; ai<argv.size(); ai++)
	  args[ai] = P_strdup(argv[ai].string_of());
	args[argv.size()] = NULL;
	P_execvp(file.string_of(), args);

	sprintf(errorLine, "execv failed, exiting, errno=%d\n", errno);
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
#ifdef PARADYND_PVM
  if (pvm_running) {
    PDYN_reportSIGCHLD(proc->getPid(), exitStatus);
  }
#endif
}

