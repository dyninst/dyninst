
/*
 * Igen derived class.  Used to provide virtual functions to replace the
 * defaults.
 * 
 * $Log: comm.h,v $
 * Revision 1.4  1995/02/16 08:32:55  markc
 * Changed igen interfaces to use strings/vectors rather than char*/igen-arrays
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
    : dynRPC(family, port, type, host, rf, wf, nblock) { }
  pdRPC(int fdes, xdr_rd_func r, xdr_wr_func w, bool nblock=false)
    : dynRPC(fdes, r, w, nblock) {}
  /* pdRPC(char *m, char *l, char *p, xdrIOFunc r, xdrIOFunc w,
     char **args=0, int nblock=0) :
     dynRPC(m, l, p, r, w, wellKnownPort, args, nblock) {;} */

  void handle_error();
};

#endif
