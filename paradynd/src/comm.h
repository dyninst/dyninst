
/*
 * Igen derived class.  Used to provide virtual functions to replace the
 * defaults.
 * 
 * $Log: comm.h,v $
 * Revision 1.2  1994/08/17 18:03:41  markc
 * Changed variable names to remove compiler warnings.
 *
 * Revision 1.1  1994/06/02  23:26:55  markc
 * Files to implement error handling for igen generated class.
 *
 *
 */

#ifndef _COMM_H_IGEN_
#define _COMM_H_IGEN_

#include "dyninstRPC.SRVR.h"

class pdRPC : public dynRPC
{
 public:
   pdRPC(int family, int port, int type, char *host, xdrIOFunc rf,
	 xdrIOFunc wf, int nblock=0) :
	   dynRPC(family, port, type, host, rf, wf, nblock) {;}
   pdRPC(int fdes, xdrIOFunc r, xdrIOFunc w, int nblock=0) :
     dynRPC(fdes, r, w, nblock) {;}
   pdRPC(char *m, char *l, char *p, xdrIOFunc r, xdrIOFunc w,
	 char **args=0, int nblock=0) :
	   dynRPC(m, l, p, r, w, args, nblock) {;}

  void handle_error();
};

#endif
