
/*
 * Igen derived class.  Used to provide virtual functions to replace the
 * defaults.
 * 
 * $Log: comm.h,v $
 * Revision 1.6  1996/05/31 23:54:05  tamches
 * added alterSendSocketBufferSize
 *
 * Revision 1.5  1995/02/16 08:53:02  markc
 * Corrected error in comments -- I put a "star slash" in the comment.
 *
 * Revision 1.4  1995/02/16  08:32:55  markc
 * Changed igen interfaces to use strings/vectors rather than char igen-arrays
 * Changed igen interfaces to use bool, not Boolean.
 * Cleaned up symbol table parsing - favor properly labeled symbol table objects
 * Updated binary search for modules
 * Moved machine dependnent ptrace code to architecture specific files.
 * Moved machine dependent code out of class process.
 * Removed almost all compiler warnings.
 * Use "posix" like library to remove compiler warnings
 *
 * Revision 1.3  1995/01/26  18:11:52  jcargill
 * Updated igen-generated includes to new naming convention
 *
 * Revision 1.2  1994/08/17  18:03:41  markc
 * Changed variable names to remove compiler warnings.
 *
 * Revision 1.1  1994/06/02  23:26:55  markc
 * Files to implement error handling for igen generated class.
 *
 *
 */

#ifndef _COMM_H_IGEN_
#define _COMM_H_IGEN_

#include "dyninstRPC.xdr.SRVR.h"

class pdRPC : public dynRPC
{
public:
  pdRPC(int family, int port, int type, const string host, xdr_rd_func rf,
	xdr_wr_func wf, bool nblock=false)
    : dynRPC(family, port, type, host, rf, wf, nblock) {

     alterSendSocketBufferSize();

  }


  pdRPC(int fdes, xdr_rd_func r, xdr_wr_func w, bool nblock=false)
    : dynRPC(fdes, r, w, nblock) {

     alterSendSocketBufferSize();
  }

  /* pdRPC(char *m, char *l, char *p, xdrIOFunc r, xdrIOFunc w,
     char **args=0, int nblock=0) :
     dynRPC(m, l, p, r, w, wellKnownPort, args, nblock) {;} */

  void handle_error();

   void alterSendSocketBufferSize() {
     // Now let's alter the socket buffer size to avoid write-write deadlock
     // between paradyn and paradynd
#if defined(sparc_sun_sunos4_1_3) || defined(hppa1_1_hp_hpux)
     int num_bytes = 32768;
     int size = sizeof(num_bytes);

     if (setsockopt(this->get_fd(), SOL_SOCKET, SO_SNDBUF,
                    &num_bytes, size) < 0) {
        cerr << "paradynd warning: could not set socket write buffer size" << endl;
     }
#endif     
  }

};

#endif
