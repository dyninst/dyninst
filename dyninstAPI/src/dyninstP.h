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

/* $Id: dyninstP.h,v 1.20 2003/07/15 22:44:01 schendel Exp $
 *
 * private structures used by the implementation of the instrumentation 
 *   interface.  modules that use the instrumentation interface should not
 *   include this file.
 *  
 * This file will be empty during the restructuring of the paradyn daemon
 */

#ifndef _DYNINSTP_H
#define _DYNINSTP_H

#include "common/h/Vector.h"
#include "common/h/String.h"

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
int addProcess(pdvector<pdstring> &argv, pdvector<pdstring> &envp, pdstring dir = "");

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
const pdstring nullString("");

#endif
