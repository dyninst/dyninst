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
  // initialRequests += new instMapping("main", "DYNINSTalarmExpire", FUNC_EXIT);
  // initialRequests += new instMapping("main", "DYNINSTexit", FUNC_EXIT);
  initialRequests += new instMapping(EXIT_NAME, "DYNINSTexit", FUNC_ENTRY);
  initialRequests += new instMapping("___exit", "DYNINSTexit", FUNC_ENTRY);

#ifdef notdef
  initialRequests += new instMapping("fork", "DYNINSTfork", FUNC_EXIT|FUNC_FULL_ARGS);
#endif

  // initialRequests += new instMapping(EXIT_NAME, "DYNINSTalarmExpire", FUNC_ENTRY);
  // initialRequests += new instMapping(EXIT_NAME, "DYNINSTprintCost", FUNC_ENTRY);
  // initialRequests += new instMapping(EXIT_NAME, "DYNINSTbreakPoint", FUNC_ENTRY);
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
