
/*
 * $Log: init-aix.C,v $
 * Revision 1.1  1995/08/24 15:03:50  hollings
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
static AstNode tagArg(Param, (void *) 1);
static AstNode argvArg(Param, (void *) 1);
static AstNode cmdArg(Param, (void *) 4);

bool initOS() {
  initialRequests += new instMapping("main", "DYNINSTalarmExpire", FUNC_EXIT);

#ifdef notdef
  initialRequests += new instMapping("fork", "DYNINSTfork", FUNC_EXIT|FUNC_FULL_ARGS);
#endif

  initialRequests += new instMapping(EXIT_NAME, "DYNINSTalarmExpire", FUNC_ENTRY);
  initialRequests += new instMapping(EXIT_NAME, "DYNINSTprintCost", FUNC_ENTRY);
  initialRequests += new instMapping("main", "DYNINSTinit", FUNC_ENTRY);
  initialRequests += new instMapping("DYNINSTsampleValues", "DYNINSTreportNewTags", FUNC_ENTRY);
  initialRequests += new instMapping("rexec", "DYNINSTrexec",
			      FUNC_ENTRY|FUNC_ARG, &cmdArg);
  initialRequests += new instMapping("execvp", "DYNINSTexecvp", 
				FUNC_ENTRY|FUNC_ARG, &argvArg);

  return true;
};
