/*
 * Copyright (c) 1996-2000 Barton P. Miller
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

/* $Id: init-osf.C,v 1.3 2000/12/15 21:38:43 pcroth Exp $ */

#include "paradynd/src/metric.h"
#include "paradynd/src/internalMetrics.h"
#include "dyninstAPI/src/inst.h"
#include "paradynd/src/init.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/util.h"
#include "dyninstAPI/src/os.h"
#include "common/h/Time.h"
#include "common/h/timing.h"
#include "paradynd/src/timeMgr.h"


// NOTE - the tagArg integer number starting with 0.  
bool initOS() {

//  initialRequests += new instMapping("main", "DYNINSTinit", FUNC_ENTRY);
// (obsoleted by installBootstrapInst() --ari)


  AstNode *retVal;

  initialRequests += new instMapping("main", "DYNINSTexit", FUNC_EXIT);

  initialRequests += new instMapping(EXIT_NAME, "DYNINSTexit", FUNC_ENTRY);

  if (process::pdFlavor == "mpi") {
	  instMPI();
	  retVal = new AstNode(AstNode::ReturnVal, (void *) 0);
	  initialRequests += new instMapping("fork", "DYNINSTmpi_fork", 
					     FUNC_EXIT|FUNC_ARG, retVal);
	  retVal = new AstNode(AstNode::ReturnVal, (void *) 0);
	  initialRequests += new instMapping("_fork", "DYNINSTmpi_fork", 
					     FUNC_EXIT|FUNC_ARG, retVal);
  } else { /* Fork and exec */
	  retVal = new AstNode(AstNode::ReturnVal, (void *) 0);
	  initialRequests += new instMapping("fork", "DYNINSTfork", 
					     FUNC_EXIT|FUNC_ARG, retVal);

	  //libthread _fork
	  retVal = new AstNode(AstNode::ReturnVal, (void *) 0);
	  initialRequests += new instMapping("_fork", "DYNINSTfork", 
					     FUNC_EXIT|FUNC_ARG, retVal);


	  //initialRequests += new instMapping("execve", "DYNINSTexec",
	  //			               FUNC_ENTRY|FUNC_ARG, tidArg);
	  //initialRequests += new instMapping("execve", "DYNINSTexecFailed", 
	  //                                   FUNC_EXIT);
	  AstNode *tidArg = new AstNode(AstNode::Param, (void *) 0);
	  initialRequests += new instMapping("_execve", "DYNINSTexec",
					     FUNC_ENTRY|FUNC_ARG, tidArg);
	  initialRequests += new instMapping("_execve", "DYNINSTexecFailed", 
					     FUNC_EXIT);
  }
  
#ifndef SHM_SAMPLING
  initialRequests += new instMapping("DYNINSTsampleValues", "DYNINSTreportNewTags",
				 FUNC_ENTRY);
#endif

        AstNode *cmdArg = new AstNode(AstNode::Param, (void *) 4);
  	initialRequests += new instMapping("rexec", "DYNINSTrexec",
				 FUNC_ENTRY|FUNC_ARG, cmdArg);
//   initialRequests += new instMapping("PROCEDURE_LINKAGE_TABLE","DYNINSTdynlinker",FUNC_ENTRY);


#ifdef PARADYND_PVM
  char *doPiggy;

  AstNode *tagArg = new AstNode(AstNode::Param, (void *) 1);
  initialRequests += new instMapping("pvm_send", "DYNINSTrecordTag",
				 FUNC_ENTRY|FUNC_ARG, tagArg);

  // kludge to get Critical Path to work.
  // XXX - should be tunable constant.
  doPiggy = getenv("DYNINSTdoPiggy");
  if (doPiggy) {
      initialRequests += new instMapping("main", "DYNINSTpvmPiggyInit", FUNC_ENTRY);
      AstNode *tidArg = new AstNode(AstNode::Param, (void *) 0);
      initialRequests+= new instMapping("pvm_send", "DYNINSTpvmPiggySend",
                           FUNC_ENTRY|FUNC_ARG, tidArg);
      initialRequests += new instMapping("pvm_recv", "DYNINSTpvmPiggyRecv", FUNC_EXIT);
      tidArg = new AstNode(AstNode::Param, (void *) 0);
      initialRequests += new instMapping("pvm_mcast", "DYNINSTpvmPiggyMcast",
                           FUNC_ENTRY|FUNC_ARG, tidArg);
  }
#endif

  return true;
};

void initWallTimeMgrPlt() {
  getWallTimeMgr().installLevel(wallTimeMgr_t::LEVEL_TWO, yesFunc,
				timeUnit::us(), timeBase::b1970(), 
				&getRawTime1970, "DYNINSTgetWalltime_sw");
}


