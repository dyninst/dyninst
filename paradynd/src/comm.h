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
 * Igen derived class.  Used to provide virtual functions to replace the
 * defaults.
 * 
 * $Id: comm.h,v 1.12 2003/07/15 22:46:42 schendel Exp $
 */

#ifndef _COMM_H_IGEN_
#define _COMM_H_IGEN_

#include "dyninstRPC.xdr.SRVR.h"

class pdRPC : public dynRPC
{
public:
  pdRPC(int family, int port, int type, const pdstring host, xdr_rd_func rf,
	xdr_wr_func wf, int nblock=0)
    : dynRPC(family, port, type, host, rf, wf, nblock) {

     alterSendSocketBufferSize();

  }


  pdRPC(PDSOCKET sock, xdr_rd_func r, xdr_wr_func w, int nblock=0)
    : dynRPC(sock, r, w, nblock) {

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

     if (P_setsockopt(this->get_sock(), SOL_SOCKET, SO_SNDBUF,
                      &num_bytes, size) < 0) {
        cerr << "paradynd warning: could not set socket write buffer size" << endl;
     }
#endif     
  }

  virtual void send_mdl( pdvector<T_dyninstRPC::rawMDL> mdlBufs );
  void send_metrics( pdvector<T_dyninstRPC::mdl_metric*>* mv );
  void send_constraints( pdvector<T_dyninstRPC::mdl_constraint*>* cv );
  void send_stmts( pdvector<T_dyninstRPC::mdl_stmt*>* sv );
  void send_libs( pdvector<pdstring>* libs );
  void send_no_libs( void );
};

#endif
