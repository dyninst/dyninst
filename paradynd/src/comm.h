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
 * Igen derived class.  Used to provide virtual functions to replace the
 * defaults.
 * 
 * $Log: comm.h,v $
 * Revision 1.8  1996/10/31 08:37:14  tamches
 * removed a warning
 *
 * Revision 1.7  1996/08/16 21:18:20  tamches
 * updated copyright for release 1.1
 *
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

     if (P_setsockopt(this->get_fd(), SOL_SOCKET, SO_SNDBUF,
                      &num_bytes, size) < 0) {
        cerr << "paradynd warning: could not set socket write buffer size" << endl;
     }
#endif     
  }

};

#endif
