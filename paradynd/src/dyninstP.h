/*
 *  Copyright 1993 Jeff Hollingsworth.  All rights reserved.
 *
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
 * Revision 1.11  1995/10/19 22:36:37  mjrg
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
 * Revision 1.5  1994/11/02  11:04:16  markc
 * Changed casts to remove compiler warnings.
 *
 * Revision 1.4  1994/09/22  01:51:40  markc
 * Added most of dyninst.h, temporary
 *
 * Revision 1.3  1994/08/08  20:13:35  hollings
 * Added suppress instrumentation command.
 *
 * Revision 1.2  1994/02/01  18:46:51  hollings
 * Changes for adding perfConsult thread.
 *
 * Revision 1.1  1994/01/27  20:31:18  hollings
 * Iinital version of paradynd speaking dynRPC igend protocol.
 *
 * Revision 1.6  1993/08/23  23:14:58  hollings
 * removed unused pid field.
 *
 * Revision 1.5  1993/07/13  18:27:00  hollings
 * new include file syntax.
 *
 * Revision 1.4  1993/06/22  19:00:01  hollings
 * global inst state.
 *
 * Revision 1.3  1993/06/08  20:14:34  hollings
 * state prior to bc net ptrace replacement.
 *
 * Revision 1.2  1993/04/27  14:39:21  hollings
 * signal forwarding and args tramp.
 *
 * Revision 1.1  1993/03/19  22:51:05  hollings
 * Initial revision
 *
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
 *   stopAtFirstbrk - if true, the process pauses when it reaches the trap at the start
 *                      of the program. If false, paradynd will start running the process
 *                      automatically.
 */
int addProcess(vector<string> &argv, vector<string> &envp, string dir = "", bool stopAtFirstBrk = false);

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
 * Continue process that is waiting for the CM5 node daemon. Used by CM5 processes only.
 */
void continueProcWaitingForDaemon();

/*
 * Disconnect the tool from the process.
 *    pause - leave the process in a stopped state.
 *
 */
bool detachProcess(int pid, bool pause);


// TODO -- is this needed
const string nullString((char*) NULL);

#endif


