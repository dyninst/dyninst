
/*
 * $Log: init-sunos.C,v $
 * Revision 1.9  1996/08/12 16:27:16  mjrg
 * Code cleanup: removed cm5 kludges and some unused code
 *
 * Revision 1.8  1996/05/08 23:54:45  mjrg
 * added support for handling fork and exec by an application
 * use /proc instead of ptrace on solaris
 * removed warnings
 *
 * Revision 1.7  1996/03/20 17:02:44  mjrg
 * Added multiple arguments to calls.
 * Instrument pvm_send instead of pvm_recv to get tags.
 *
 * Revision 1.6  1996/03/01 22:31:59  mjrg
 * Replaced calls at the exit point by a call to DYNINSTexit
 *
 * Revision 1.5  1995/12/15 22:26:48  mjrg
 * Merged paradynd and paradyndPVM
 * Get module name for functions from symbol table in solaris
 * Fixed code generation for multiple instrumentation statements
 * Changed syntax of MDL resource lists
 *
 * Revision 1.4  1995/08/24  15:03:54  hollings
 * AIX/SP-2 port (including option for split instruction/data heaps)
 * Tracing of rexec (correctly spawns a paradynd if needed)
 * Added rtinst function to read getrusage stats (can now be used in metrics)
 * Critical Path
 * Improved Error reporting in MDL sematic checks
 * Fixed MDL Function call statement
 * Fixed bugs in TK usage (strings passed where UID expected)
 *
 * Revision 1.3  1995/05/18  10:33:50  markc
 * Removed resource predicate definitions
 * Removed metric defintions
 *
 * Revision 1.2  1994/11/10  21:17:38  jcargill
 * Removed active_process from the list of all metrics; it's now an internal
 *
 * Revision 1.1  1994/11/01  16:55:56  markc
 * Environment specific initialization. (pvm, cm5, sun sequential)
 *
 */

#include "metric.h"
#include "internalMetrics.h"
#include "inst.h"
#include "init.h"
#include "ast.h"
#include "util.h"
#include "os.h"

// NOTE - the tagArg integer number starting with 0.  
static AstNode tagArg(Param, (void *) 1);
static AstNode cmdArg(Param, (void *) 4);
static AstNode tidArg(Param, (void *) 0);

bool initOS() {

  initialRequests += new instMapping("main", "DYNINSTinit", FUNC_ENTRY);
  initialRequests += new instMapping("main", "DYNINSTexit", FUNC_EXIT);

  initialRequests += new instMapping(EXIT_NAME, "DYNINSTexit", FUNC_ENTRY);

  initialRequests += new instMapping("fork", "DYNINSTfork", 
				     FUNC_EXIT|FUNC_ARG, &tidArg);
  initialRequests += new instMapping("execve", "DYNINSTexec",
				     FUNC_ENTRY|FUNC_ARG, &tidArg);
  initialRequests += new instMapping("execve", "DYNINSTexecFailed", FUNC_EXIT);

  initialRequests += new instMapping("DYNINSTsampleValues", "DYNINSTreportNewTags",
				 FUNC_ENTRY);
  initialRequests += new instMapping("rexec", "DYNINSTrexec",
				 FUNC_ENTRY|FUNC_ARG, &cmdArg);


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
