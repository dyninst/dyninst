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
 * $Log: init-aix.C,v $
 * Revision 1.8  1996/12/16 23:10:27  mjrg
 * bug fixes to fork/exec on all platforms, partial fix to fork on AIX
 *
 * Revision 1.7  1996/10/31 08:42:41  tamches
 * removed main-->DYNINSTinit() instrumentation in initOS()
 *
 * Revision 1.6  1996/08/16 21:18:41  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.5  1996/05/08 23:54:42  mjrg
 * added support for handling fork and exec by an application
 * use /proc instead of ptrace on solaris
 * removed warnings
 *
 * Revision 1.4  1996/03/20 17:02:43  mjrg
 * Added multiple arguments to calls.
 * Instrument pvm_send instead of pvm_recv to get tags.
 *
 * Revision 1.3  1996/03/01 22:31:55  mjrg
 * Replaced calls at the exit point by a call to DYNINSTexit
 *
 * Revision 1.2  1995/12/15 22:26:46  mjrg
 * Merged paradynd and paradyndPVM
 * Get module name for functions from symbol table in solaris
 * Fixed code generation for multiple instrumentation statements
 * Changed syntax of MDL resource lists
 *
 * Revision 1.1  1995/08/24  15:03:50  hollings
 * AIX/SP-2 port (including option for split instruction/data heaps)
 * Tracing of rexec (correctly spawns a paradynd if needed)
 * Added rtinst function to read getrusage stats (can now be used in metrics)
 * Critical Path
 * Improved Error reporting in MDL sematic checks
 * Fixed MDL Function call statement
 * Fixed bugs in TK usage (strings passed where UID expected)
 *
 */

#include "metric.h"
#include "internalMetrics.h"
#include "inst.h"
#include "init.h"
#include "metricDef.h"
#include "ast.h"
#include "util.h"
#include "os.h"

// NOTE - the tagArg integer number starting with 0.  
static AstNode tagArg(AstNode::Param, (void *) 1);
static AstNode argvArg(AstNode::Param, (void *) 1);
static AstNode cmdArg(AstNode::Param, (void *) 4);
static AstNode tidArg(AstNode::Param, (void *) 0);

bool initOS() {
//  initialRequests += new instMapping("main", "DYNINSTinit", FUNC_ENTRY);
// (obsoleted by installBootstrapInst() --ari)


  initialRequests += new instMapping("main", "DYNINSTexit", FUNC_EXIT);

  // we need to instrument __fork instead of fork. fork makes a tail call
  // to __fork, and we can't get the correct return value from fork.
  initialRequests += new instMapping("__fork", "DYNINSTfork", FUNC_EXIT|FUNC_ARG, &tidArg);

  initialRequests += new instMapping("execve", "DYNINSTexec", FUNC_ENTRY|FUNC_ARG, &tidArg);
  initialRequests += new instMapping("execve", "DYNINSTexecFailed", FUNC_EXIT);

  initialRequests += new instMapping(EXIT_NAME, "DYNINSTexit", FUNC_ENTRY);

  initialRequests += new instMapping("DYNINSTsampleValues", "DYNINSTreportNewTags", FUNC_ENTRY);
  initialRequests += new instMapping("rexec", "DYNINSTrexec",
			      FUNC_ENTRY|FUNC_ARG, &cmdArg);
  initialRequests += new instMapping("execvp", "DYNINSTexecvp", 
				FUNC_ENTRY|FUNC_ARG, &argvArg);


#ifdef PARADYND_PVM
  char *doPiggy;

  initialRequests += new instMapping("pvm_send", "DYNINSTrecordTag",
				 FUNC_ENTRY|FUNC_ARG, &tagArg);

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
#endif

  return true;
};
