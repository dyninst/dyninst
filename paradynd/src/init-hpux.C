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

  return true;
};
