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

// $Id: init-pvm.C,v 1.13 2002/05/10 18:37:26 schendel Exp $

#include "paradynd/src/internalMetrics.h"
#include "dyninstAPI/src/inst.h"
#include "paradynd/src/init.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/util.h"
#include "dyninstAPI/src/os.h"

// NOTE - the tagArg integer number starting with 0.  
static AstNode tagArg(Param, (void *) 1);
static AstNode cmdArg(Param, (void *) 4);

bool initOS() {

  char *doPiggy;

  initialRequests += new instMapping("pvm_recv", "DYNINSTrecordTag",
				 FUNC_ENTRY|FUNC_ARG, &tagArg);

#if defined(hppa1_1_hp_hpux)
  initialRequests += new instMapping("main", "DYNINSTalarmExpire_hpux", FUNC_EXIT);
  initialRequests += new instMapping(EXIT_NAME, "DYNINSTalarmExpire_hpux", FUNC_ENTRY);  
#else 
  initialRequests += new instMapping("main", "DYNINSTalarmExpire", FUNC_EXIT);
  initialRequests += new instMapping(EXIT_NAME, "DYNINSTalarmExpire", FUNC_ENTRY);    
#endif

  initialRequests += new instMapping("main", "DYNINSTinit", FUNC_ENTRY);
  initialRequests += new instMapping(EXIT_NAME, "DYNINSTprintCost", FUNC_ENTRY);
#ifdef notdef
  // there is no good reason to stop a process at the end.  It was
  //   originally here for debugging of early versions of dyninst. - jkh 7/5/95
  //   Since the pause is done with signals, it doesn't work on machines 
  //     that we detach the ptrace when not in use.
  initialRequests += new instMapping(EXIT_NAME, "DYNINSTbreakPoint", FUNC_ENTRY);
#endif
  initialRequests += new instMapping("DYNINSTsampleValues", "DYNINSTreportNewTags",
				 FUNC_ENTRY);


  // kludge to get Critical Path to work.
  // XXX - should be tunable constant.
  doPiggy = getenv("DYNINSTdoPiggy");
  if (doPiggy) {
      initialRequests += new instMapping("main", "DYNINSTpvmPiggyInit", FUNC_ENTRY);
      AstNode *tidArg = new AstNode(Param, (void *) 0);
      initialRequests+= new instMapping("pvm_send", "DYNINSTpvmPiggySend",
                           FUNC_ENTRY|FUNC_ARG, tidArg);
      initialRequests += new instMapping("pvm_recv", "DYNINSTpvmPiggyRecv", FUNC_EXIT);
      tidArg = new AstNode(Param, (void *) 0);
      initialRequests += new instMapping("pvm_mcast", "DYNINSTpvmPiggyMcast",
                           FUNC_ENTRY|FUNC_ARG, tidArg);
  }
  return true;
};


