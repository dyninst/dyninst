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
 * This file contains special versions of some of the functions in process.C
 * that have been modified for the dyninstAPI library.
 */
/*
 * $Log: process2.C,v $
 * Revision 1.1  1997/03/18 19:44:23  buck
 * first commit of dyninst library.  Also includes:
 * 	moving templates from paradynd to dyninstAPI
 * 	converting showError into a function (in showerror.C)
 * 	many ifdefs for BPATCH_LIBRARY in dyinstAPI/src.
 *
 *
 */

extern "C" {
#ifdef PARADYND_PVM
int pvmputenv (const char *);
int pvmendtask();
#endif
}

#include "util/h/headers.h"
#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/util.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/instP.h"
#include "dyninstAPI/src/dyninstP.h"
#include "dyninstAPI/src/os.h"
#include "paradynd/src/showerror.h"
#include "paradynd/src/perfStream.h"
#include "dyninstAPI/src/dynamiclinking.h"

#include "util/h/debugOstream.h"

#ifdef ATTACH_DETACH_DEBUG
extern debug_ostream attach_cerr;
#else
extern debug_ostream attach_cerr;
#endif

extern unsigned activeProcesses; // number of active processes
extern vector<process*> processVec;

extern bool dyninstAPI_forkNewProcess(string file, string dir,
    vector<string> argv, vector<string>envp, string inputFile,
    string outputFile, int &traceLink, int &ioLink, int &pid, int &tid,
    int &procHandle, int &thrHandle);

/*
 * Create a new instance of the named process.  Read the symbols and start
 *   the program
 */
process *dyninstAPI_createProcess(const string File, vector<string> argv,
	vector<string> envp, const string dir = "")
{
    // prepend the directory (if any) to the file, unless the filename
    // starts with a /
    string file = File;
    if (!file.prefixed_by("/") && dir.length() > 0)
      file = dir + "/" + file;

    int traceLink;
    int ioLink;
    int pid;
    int tid;
    int procHandle;
    int thrHandle;

    string inputFile;
    string outputFile;

    ioLink = traceLink = -1;
    if (!dyninstAPI_forkNewProcess(file, dir, argv, envp, inputFile, outputFile,
		   traceLink, ioLink, pid, tid, procHandle, thrHandle)) {
      // forkNewProcess is resposible for displaying error messages
      return NULL;
    }


#if defined(rs6000_ibm_aix3_2) || defined(rs6000_ibm_aix4_1)
	extern bool establishBaseAddrs(int pid, int &status, bool waitForTrap);
	int status;

	if (!establishBaseAddrs(pid, status, true)) {
	    return(NULL);
	}
#endif

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

	process *ret = new process(pid, img, traceLink, ioLink
#ifdef SHM_SAMPLING
				   , 7000, // shm seg key to try first
				   theShmHeapStats
#endif
				   );
	   // change this to a ctor that takes in more args

	assert(ret);

	processVec += ret;
	activeProcesses++;

        // find the signal handler function
	ret->findSignalHandler(); // should this be in the ctor?

        // initializing vector of threads - thread[0] is really the 
        // same process
        ret->threads += new Thread(ret);

        // we use this flag to solve race condition between inferiorRPC and 
        // continueProc message from paradyn - naim
        ret->deferredContinueProc = false;

        ret->numOfActCounters_is=0;
        ret->numOfActProcTimers_is=0;
        ret->numOfActWallTimers_is=0;

    return ret;

}


process *dyninstAPI_attachProcess(const string &progpath, int pid,
	int afterAttach)
{
   // implementation of dynRPC::attach() (the igen call)
   // This is meant to be "the other way" to start a process (competes w/ createProcess)

   // progpath gives the full path name of the executable, which we use ONLY to
   // read the symbol table.

   // We try to make progpath optional, since given pid, we should be able to
   // calculate it with a clever enough search of the process' PATH, examining
   // its argv[0], examining its current directory, etc.  /proc gives us this
   // information on solaris...not sure about other platforms...

   // possible values for afterAttach: 1 --> pause, 2 --> run, 0 --> leave as is

   attach_cerr << "welcome to attachProcess for pid " << pid << endl;

   // QUESTION: When we attach to a process, do we want to redirect its stdout/stderr
   //           (like we do when we fork off a new process the 'usual' way)?
   //           My first guess would be no.  -ari
   //           But although we may ignore the io, we still need the trace stream.

   // When we attach to a process, we don't fork...so this routine is much simpler
   // than its "competitor", createProcess() (above).

   // TODO: What about AIX establishBaseAddrs???  Do that now?

   string fullPathToExecutable = process::tryToFindExecutable(progpath, pid);
   if (!fullPathToExecutable.length())
      return NULL;

   image *theImage = image::parseImage(fullPathToExecutable);
   if (theImage == NULL) {
      // two failure return values would be useful here, to differentiate
      // file-not-found vs. catastrophic-parse-error.
      string msg = string("Unable to parse image: ") + fullPathToExecutable;
      showErrorCallback(68, msg.string_of());
      return NULL; // failure
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
   process *theProc = new process(pid, theImage, afterAttach
#ifdef SHM_SAMPLING
				  ,7000, // shm seg key to try first
				  theShmHeapStats
#endif				  
				  );
   assert(theProc);

   // the attach ctor always leaves us stopped...we may get continued once
   // DYNINSTinit has finished running...
   assert(theProc->status() == stopped);

   processVec += theProc;
   activeProcesses++;

   // find the signal handler function
   theProc->findSignalHandler(); // shouldn't this be in the ctor?

   return theProc; // successful
}
