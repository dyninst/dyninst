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

#ifndef _DYNINSTP_H
#define _DYNINSTP_H

/*
 * private structures used by the implementation of the instrumentation 
 *   interface.  modules that use the instrumentation interface should not
 *   include this file.
 *  
 * This file will be empty during the restructuring of the paradyn daemon
 *
 * $Log: dyninstP.h,v $
 * Revision 1.14  1996/10/31 08:39:15  tamches
 * removed an old cm5 call
 *
 * Revision 1.13  1996/08/16 21:18:29  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.12  1996/08/12 16:27:19  mjrg
 * Code cleanup: removed cm5 kludges and some unused code
 *
 * Revision 1.11  1995/10/19  22:36:37  mjrg
 * Added callback function for paradynd's to report change in status of application.
 * Added Exited status for applications.
 * Removed breakpoints from CM5 applications.
 * Added search for executables in a given directory.
 *
 * Revision 1.10  1995/09/18  22:41:32  mjrg
 * added directory command.
 *
 * Revision 1.9  1995/05/18  10:31:57  markc
 * Cleaned up declarations of metric functions
 *
 * Revision 1.8  1995/02/26  22:45:09  markc
 * Changed addProcess interface to use reference to string vectors.
 *
 * Revision 1.7  1995/02/16  08:53:06  markc
 * Corrected error in comments -- I put a "star slash" in the comment.
 *
 * Revision 1.6  1995/02/16  08:33:10  markc
 * Changed igen interfaces to use strings/vectors rather than char igen-arrays
 * Changed igen interfaces to use bool, not Boolean.
 * Cleaned up symbol table parsing - favor properly labeled symbol table objects
 * Updated binary search for modules
 * Moved machine dependnent ptrace code to architecture specific files.
 * Moved machine dependent code out of class process.
 * Removed almost all compiler warnings.
 * Use "posix" like library to remove compiler warnings
 *
 */

#include "util/h/Vector.h"
#include "util/h/String.h"

typedef enum { selfTermination, controlTermination } executableType;
typedef enum { Trace, Sample } dataType;

bool isApplicationPaused();

/*
 * error handler call back.
 *
 */
typedef int (*errorHandler)(int errno, char *message);

/*
 * Define a program to run (this is very tentative!)
 *
 *   argv - arguments to command
 *   envp - environment args, for pvm
 *   dir  - the directory where the program will run
 */
int addProcess(vector<string> &argv, vector<string> &envp, string dir = "");

/*
 * Find out if an application has been.defines yet.
 *
 */
bool applicationDefined();

/*
 * Start an application running (This starts the actual execution).
 *  app - an application context from createPerformanceConext.
 */
bool startApplication();

/*
 *   Stop all processes associted with the application.
 *	app - an application context from createPerformanceConext.
 *
 * Pause an application (I am not sure about this but I think we want it).
 *      - Does this force buffered data to be delivered?
 *	- Does a paused application respond to enable/disable commands?
 */
bool pauseAllProcesses();

/*
 * Continue a paused application.
 *    app - an application context from createPerformanceConext.
 */
bool continueAllProcesses();

/*
 * Disconnect the tool from the process.
 *    pause - leave the process in a stopped state.
 *
 */
bool detachProcess(int pid, bool pause);


// TODO -- is this needed
const string nullString((char*) NULL);

#endif


