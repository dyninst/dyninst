
/*
 * Implements virtual function called during an igen error.
 *
 * $Log: comm.C,v $
 * Revision 1.7  1995/09/26 20:17:41  naim
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
 * Revision 1.2  1994/06/22  01:43:12  markc
 * Removed warnings.  Changed bcopy in inst-sparc.C to memcpy.  Changed process.C
 * reference to proc->status to use proc->heap->status.
 *
 * Revision 1.1  1994/06/02  23:26:56  markc
 * Files to implement error handling for igen generated class.
 *
 *
 */

#include <util/h/headers.h>
#include "comm.h"
#include "util.h"
#include "showerror.h"

void dump_profile(pdRPC *pdr) {
  delete pdr;
  P_exit(-1);
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
  switch (get_err_state())
    {
    case igen_encode_err:
    case igen_decode_err:
      sprintf(errorLine, "Could not (un)marshall parameters, dumping core, pid=%ld\n",
	      (long) P_getpid());
      logLine(errorLine);
      showErrorCallback(73,(const char *) errorLine);
      P_abort();
      break;

    case igen_proto_err:
      sprintf(errorLine, "Internal error: protocol verification failed, pid=%ld\n", (long) P_getpid());
      logLine(errorLine);
      showErrorCallback(74, (const char *) errorLine);
      P_abort();
      break;

    case igen_call_err:
      sprintf(errorLine, "Internal error: cannot do sync call here, pid=%ld\n", (long) P_getpid());
      logLine(errorLine);
      showErrorCallback(75, (const char *) errorLine);
      P_abort();
      break;

    case igen_request_err:
      sprintf(errorLine, "Internal error: unknown message tag pid=%ld\n", (long) P_getpid());
      logLine(errorLine);
      showErrorCallback(76, (const char *) errorLine);
      P_abort();
      break;

    case igen_no_err:
      sprintf(errorLine, "Internal error: handle error called for err_state = igen_no_err\n");
      logLine(errorLine);
      showErrorCallback(77, (const char *) errorLine);
      // fall thru
    case igen_send_err:
    case igen_read_err:
      // if paradyn quits either of these errors can occur, so don't dump core
    default:
      sprintf(errorLine, "Error: err_state = %d\n", get_err_state());
      logLine(errorLine);
      dump_profile(this);
      P_exit(-1);
    }
}
