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
 * $Log: init-pvm.C,v $
 * Revision 1.10  1997/02/21 20:15:46  naim
 * Moving files from paradynd to dyninstAPI + eliminating references to
 * dataReqNode from the ast class. This is the first pre-dyninstAPI commit! - naim
 *
 * Revision 1.9  1996/08/16 21:18:43  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.8  1996/04/08 21:23:08  lzheng
 * changes to accomodate DYNINSTalarmExpire_hpux
 *
 * Revision 1.7  1995/08/28 01:48:05  hollings
 * Corrected error with instMapping constructor for critical path.
 *
 * Revision 1.6  1995/08/24  15:03:53  hollings
 * AIX/SP-2 port (including option for split instruction/data heaps)
 * Tracing of rexec (correctly spawns a paradynd if needed)
 * Added rtinst function to read getrusage stats (can now be used in metrics)
 * Critical Path
 * Improved Error reporting in MDL sematic checks
 * Fixed MDL Function call statement
 * Fixed bugs in TK usage (strings passed where UID expected)
 *
 * Revision 1.5  1995/05/18  10:33:49  markc
 * Removed resource predicate definitions
 * Removed metric defintions
 *
 * Revision 1.4  1995/03/10  19:33:44  hollings
 * Fixed several aspects realted to the cost model:
 *     track the cost of the base tramp not just mini-tramps
 *     correctly handle inst cost greater than an imm format on sparc
 *     print starts at end of pvm apps.
 *     added option to read a file with more accurate data for predicted cost.
 *
 */

#include "paradynd/src/metric.h"
#include "paradynd/src/internalMetrics.h"
#include "dyninstAPI/src/inst.h"
#include "paradynd/src/init.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/util.h"
#include "dyninstAPI/src/os.h"

// NOTE - the tagArg integer number starting with 0.  
static AstNode tidArg(Param, (void *) 0);
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
      initialRequests+= new instMapping("pvm_send", "DYNINSTpvmPiggySend",
                           FUNC_ENTRY|FUNC_ARG, &tidArg);
      initialRequests += new instMapping("pvm_recv", "DYNINSTpvmPiggyRecv", FUNC_EXIT);
      initialRequests += new instMapping("pvm_mcast", "DYNINSTpvmPiggyMcast",
                           FUNC_ENTRY|FUNC_ARG, &tidArg);
  }
  return true;
};


