/*
 * Copyright (c) 1996-1999 Barton P. Miller
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
 * Implements virtual function called during an igen error.
 *
 * $Log: comm.C,v $
 * Revision 1.16  2002/10/15 17:11:34  schendel
 * create Paradyn specific pd_process and pd_thread classes  - - - - - - - -
 * fixed include problem
 *
 * Revision 1.15  2000/07/28 17:22:10  pcroth
 * Updated #includes to reflect util library split
 *
 * Revision 1.14  1999/03/03 18:01:08  pcroth
 * Updated to support changes in igen output,
 * Updated to support automatic startup from Paradyn front end
 *
 * Revision 1.13  1998/04/22 02:37:24  buck
 * Moved showerror.h from paradynd directory to dyninstAPI directory.
 *
 * Revision 1.12  1997/11/01 05:02:07  tamches
 * changed an #include<> to #include""
 *
 * Revision 1.11  1997/03/18 19:45:54  buck
 * first commit of dyninst library.  Also includes:
 * 	moving templates from paradynd to dyninstAPI
 * 	converting showError into a function (in showerror.C)
 * 	many ifdefs for BPATCH_LIBRARY in dyinstAPI/src.
 *
 * Revision 1.10  1997/02/21 20:15:35  naim
 * Moving files from paradynd to dyninstAPI + eliminating references to
 * dataReqNode from the ast class. This is the first pre-dyninstAPI commit! - naim
 *
 * Revision 1.9  1996/08/16 21:18:19  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.8  1995/11/22 00:02:13  mjrg
 * Updates for paradyndPVM on solaris
 * Fixed problem with wrong daemon getting connection to paradyn
 * Removed -f and -t arguments to paradyn
 * Added cleanUpAndExit to clean up and exit from pvm before we exit paradynd
 * Fixed bug in my previous commit
 *
 * Revision 1.7  1995/09/26  20:17:41  naim
 * Adding error messages using showErrorCallback function for paradynd
 *
 * Revision 1.6  1995/02/16  08:53:00  markc
 * Corrected error in comments -- I put a "star slash" in the comment.
 *
 * Revision 1.5  1995/02/16  08:32:54  markc
 * Changed igen interfaces to use strings/vectors rather than char igen-arrays
 * Changed igen interfaces to use bool, not Boolean.
 * Cleaned up symbol table parsing - favor properly labeled symbol table objects
 * Updated binary search for modules
 * Moved machine dependnent ptrace code to architecture specific files.
 * Moved machine dependent code out of class process.
 * Removed almost all compiler warnings.
 * Use "posix" like library to remove compiler warnings
 *
 * Revision 1.4  1994/11/02  11:01:57  markc
 * Replace printf's with logLine calls.
 *
 * Revision 1.3  1994/09/22  01:46:42  markc
 * Made system includes extern "C"
 * Access igen members via methods
 *
 */

#include "common/h/headers.h"
#include "paradynd/src/comm.h"
#include "dyninstAPI/src/util.h"
#include "dyninstAPI/src/showerror.h"
#include "paradynd/src/main.h"
#include "paradynd/src/resource.h"

extern resource *machineResource;

void dump_profile(pdRPC *pdr) {
  delete pdr;
  cleanUpAndExit(-1);
  return;
}

// handle_error is a virtual function in the igen generated code
// defining it allows for custom error handling routines to be implemented
// the error types are.defines in igen generated code, but should be
// relatively stable
//
// THESE are elaborated in the igen documentation (coming soon)
// 
// igen_no_err --> no error
// igen_decode_err --> an error occurred while unmarshalling data
// igen_encode_err --> an error occurred while marshalling data
// igen_send_err   --> an error occurred while sending a message
// igen_read_err --> an error occurred while receiving a messaged
// igen_call_err --> attempt to do a sync call when in an async call handler
// igen_request_err --> received an unknown message tag, or a response
//                      message tag that was unexpected
//
void pdRPC::handle_error()
{
	string resPartName;
	if (machineResource != NULL)
	{
		resPartName = machineResource->part_name();
	}

  switch (get_err_state())
    {
    case igen_encode_err:
    case igen_decode_err:
      sprintf(errorLine, "Could not (un)marshall parameters, dumping core, pid=%ld\n",
	      (long) P_getpid());
      logLine(errorLine);
      showErrorCallback(73,(const char *) errorLine, resPartName);
      P_abort();
      break;

    case igen_proto_err:
      sprintf(errorLine, "Internal error: protocol verification failed, pid=%ld\n", (long) P_getpid());
      logLine(errorLine);
      showErrorCallback(74, (const char *) errorLine, resPartName);
      P_abort();
      break;

    case igen_call_err:
      sprintf(errorLine, "Internal error: cannot do sync call here, pid=%ld\n", (long) P_getpid());
      logLine(errorLine);
      showErrorCallback(75, (const char *) errorLine, resPartName);
      P_abort();
      break;

    case igen_request_err:
      sprintf(errorLine, "Internal error: unknown message tag pid=%ld\n", (long) P_getpid());
      logLine(errorLine);
      showErrorCallback(76, (const char *) errorLine, resPartName);
      P_abort();
      break;

    case igen_no_err:
      sprintf(errorLine, "Internal error: handle error called for err_state = igen_no_err\n");
      logLine(errorLine);
      showErrorCallback(77, (const char *) errorLine, resPartName);
      // fall thru
    case igen_send_err:
    case igen_read_err:
      // if paradyn quits either of these errors can occur, so don't dump core
    default:
      sprintf(errorLine, "Error: err_state = %d\n", get_err_state());
      logLine(errorLine);
      dump_profile(this);
      cleanUpAndExit(-1);
    }
}
