
/*
 * Implements virtual function called during an igen error.
 *
 * $Log: comm.C,v $
 * Revision 1.3  1994/09/22 01:46:42  markc
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

extern "C" {
#include <stdio.h>
#include <unistd.h>
}

#include "comm.h"

// handle_error is a virtual function in the igen generated code
// defining it allows for custom error handling routines to be implemented
// the error types are defined in igen generated code, but should be
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
      fprintf(stderr, "Could not (un)marshall parameters, dumping core, pid=%d\n",
	      getpid());
      abort();
      break;

    case igen_call_err:
      fprintf(stderr, "can't do sync call here, pid=%d\n",
	      getpid());
      abort();
      break;

    case igen_request_err:
      fprintf(stderr, "unknown message tag pid=%d\n",
	      getpid());
      abort();
      break;

    case igen_no_err:
      fprintf(stderr, "Why is handle error called for err_state = igen_no_err\n");
      // fall thru
    case igen_send_err:
    case igen_read_err:
      // if paradyn quits either of these errors can occur, so don't dump core
    default:
      fprintf(stderr, "Error: err_state = %d\n", get_err_state());
      exit(-1);
    }
}
