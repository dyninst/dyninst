
/*
 * $Log: init-pvm.C,v $
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
 * Revision 1.3  1994/11/10  18:57:54  jcargill
 * The "Don't Blame Me Either" commit
 *
 * Revision 1.2  1994/11/09  18:40:00  rbi
 * the "Don't Blame Me" commit
 *
 * Revision 1.1  1994/11/01  16:55:54  markc
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
static AstNode tidArg(Param, (void *) 0);
static AstNode tagArg(Param, (void *) 1);
static AstNode cmdArg(Param, (void *) 4);

bool initOS() {

  char *doPiggy;

  initialRequests += new instMapping("pvm_recv", "DYNINSTrecordTag",
				 FUNC_ENTRY|FUNC_ARG, &tagArg);
  initialRequests += new instMapping("main", "DYNINSTalarmExpire", FUNC_EXIT);
  initialRequests += new instMapping("main", "DYNINSTinit", FUNC_ENTRY);
  initialRequests += new instMapping(EXIT_NAME, "DYNINSTalarmExpire", FUNC_ENTRY);
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


