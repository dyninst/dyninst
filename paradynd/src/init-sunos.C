
/*
 * $Log: init-sunos.C,v $
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

  initialRequests += new instMapping("cmmd_debug", "DYNINSTnodeCreate", FUNC_ENTRY);
  initialRequests += new instMapping("CMRT_init", "DYNINSTnodeCreate", FUNC_ENTRY);
  initialRequests += new instMapping("cmmd_debug", "DYNINSTparallelInit", FUNC_EXIT);
  initialRequests += new instMapping("CMRT_init", "DYNINSTparallelInit", FUNC_ENTRY);
  initialRequests += new instMapping("cmmd_debug", "DYNINSTbreakPoint", FUNC_EXIT);
  initialRequests += new instMapping("CMRT_init", "DYNINSTbreakPoint", FUNC_ENTRY);
  initialRequests += new instMapping("main", "DYNINSTalarmExpire", FUNC_EXIT);

#ifdef notdef
  initialRequests += new instMapping("fork", "DYNINSTfork", FUNC_EXIT|FUNC_FULL_ARGS);
#endif

  initialRequests += new instMapping(EXIT_NAME, "DYNINSTalarmExpire", FUNC_ENTRY);
  initialRequests += new instMapping(EXIT_NAME, "DYNINSTprintCost", FUNC_ENTRY);
  initialRequests += new instMapping(EXIT_NAME, "DYNINSTbreakPoint", FUNC_ENTRY);
  initialRequests += new instMapping("main", "DYNINSTinit", FUNC_ENTRY);
  initialRequests += new instMapping("DYNINSTsampleValues", "DYNINSTreportNewTags",
				 FUNC_ENTRY);
  initialRequests += new instMapping("CMMD_send", "DYNINSTrecordTag", FUNC_ENTRY|FUNC_ARG,
				 &tagArg);
  initialRequests += new instMapping("CMMD_receive", "DYNINSTrecordTag",
				 FUNC_ENTRY|FUNC_ARG, &tagArg);
  initialRequests += new instMapping("CMMD_receive_block", "DYNINSTrecordTag",
				 FUNC_ENTRY|FUNC_ARG, &tagArg);
  initialRequests += new instMapping("CMMD_send_block", "DYNINSTrecordTag",
				 FUNC_ENTRY|FUNC_ARG, &tagArg);
  initialRequests += new instMapping("CMMD_send_async", "DYNINSTrecordTag",
				 FUNC_ENTRY|FUNC_ARG, &tagArg);
  initialRequests += new instMapping("CMMD_receive_async", "DYNINSTrecordTag",
				 FUNC_ENTRY|FUNC_ARG, &tagArg);
  initialRequests += new instMapping("rexec", "DYNINSTrexec",
				 FUNC_ENTRY|FUNC_ARG, &cmdArg);


#ifdef PARADYND_PVM
  char *doPiggy;

  initialRequests += new instMapping("pvm_recv", "DYNINSTrecordTag",
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
