
/*
 * $Log: init-sunos.C,v $
 * Revision 1.2  1994/11/10 21:17:38  jcargill
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
#include "metricDef.h"
#include "ast.h"
#include "util.h"
#include "os.h"

// NOTE - the tagArg integer number starting with 0.  
static AstNode tagArg(Param, (void *) 1);

bool initOS() {
  cpuTimePredicates = new resourcePredicate[6];
  cpuTimePredicates[0].set("/Procedure", replaceBase, perModuleCPUTime);
  cpuTimePredicates[1].set("/SyncObject/MsgTag", invalidPredicate, NULL);
  cpuTimePredicates[2].set("/SyncObject", invalidPredicate, NULL);
  cpuTimePredicates[3].set("/Machine", nullPredicate, NULL);
  cpuTimePredicates[4].set("/Process", nullPredicate, NULL);
  cpuTimePredicates[5].set((char*)NULL, nullPredicate, NULL, false);

  wallTimePredicates = new resourcePredicate[6];
  wallTimePredicates[0].set("/Procedure", replaceBase, perModuleWallTime);
  wallTimePredicates[1].set("/SyncObject/MsgTag", simplePredicate,
			    defaultMSGTagPredicate);
  wallTimePredicates[2].set("/SyncObject", invalidPredicate, NULL);
  wallTimePredicates[3].set("/Machine", nullPredicate, NULL);
  wallTimePredicates[4].set("/Process", nullPredicate, NULL);
  wallTimePredicates[5].set((char*)NULL, nullPredicate,   NULL, false);

  procCallsPredicates = new resourcePredicate[5];
  procCallsPredicates[0].set("/Procedure", replaceBase, perModuleCalls);
  procCallsPredicates[1].set("/SyncObject", invalidPredicate, NULL);
  procCallsPredicates[2].set("/Machine", nullPredicate, NULL);
  procCallsPredicates[3].set("/Process", nullPredicate, NULL);
  procCallsPredicates[4].set((char*)NULL, nullPredicate,   NULL, false);

  msgPredicates = new resourcePredicate[6];
  msgPredicates[0].set("/Procedure", simplePredicate, defaultModulePredicate);
  msgPredicates[1].set("/SyncObject/MsgTag", simplePredicate, defaultMSGTagPredicate);
  msgPredicates[2].set("/SyncObject", invalidPredicate, NULL);
  msgPredicates[3].set("/Machine", nullPredicate, NULL);
  msgPredicates[4].set("/Process", nullPredicate, NULL);
  msgPredicates[5].set((char*)NULL, nullPredicate, NULL, false);

  defaultPredicates = new resourcePredicate[6];
  defaultPredicates[0].set("/Procedure", simplePredicate, defaultModulePredicate);
  defaultPredicates[1].set("/SyncObject/MsgTag", simplePredicate,
			   defaultMSGTagPredicate);
  defaultPredicates[2].set("/SyncObject", invalidPredicate, NULL);
  defaultPredicates[3].set("/Machine", nullPredicate, NULL);
  defaultPredicates[4].set("/Process", nullPredicate, NULL);
  defaultPredicates[5].set((char*)NULL, nullPredicate,   NULL, false);

  globalOnlyPredicates = new resourcePredicate[6];
  globalOnlyPredicates[0].set("/Procedure", simplePredicate, NULL);
  globalOnlyPredicates[1].set("/SyncObject/MsgTag", simplePredicate, NULL);
  globalOnlyPredicates[2].set("/SyncObject", invalidPredicate, NULL);
  globalOnlyPredicates[3].set("/Machine", nullPredicate, NULL);
  globalOnlyPredicates[4].set("/Process", nullPredicate, NULL);
  globalOnlyPredicates[5].set((char*)NULL, nullPredicate,  NULL, false);

  DYNINSTallMetrics = new metric[10];
  metricCount = 10;

  DYNINSTallMetrics[0].set("observed_cost", EventCounter, aggMax, "Wasted CPUs",
			   createObservedCost, observedCostPredicates);
  DYNINSTallMetrics[1].set("cpu", EventCounter, aggSum, "# CPUs",
			   createCPUTime, cpuTimePredicates);
  DYNINSTallMetrics[2].set("exec_time", EventCounter, aggSum, "%Time",
			   createExecTime, wallTimePredicates);
  DYNINSTallMetrics[3].set("procedure_calls", EventCounter, aggSum, "Calls/sec",
			   createProcCalls, procCallsPredicates);
  DYNINSTallMetrics[4].set("msgs", EventCounter, aggSum, "Ops/sec",
			   createMsgs, defaultPredicates);
  DYNINSTallMetrics[5].set("msg_bytes", EventCounter, aggSum, "Bytes/Sec",
			   createMsgBytesTotal, defaultPredicates);
  DYNINSTallMetrics[6].set("msg_bytes_sent", EventCounter, aggSum, "Bytes/Sec",
			   createMsgBytesSent, defaultPredicates);
  DYNINSTallMetrics[7].set("msg_bytes_recv", EventCounter, aggSum, "Bytes/Sec",
			   createMsgBytesRecv, defaultPredicates);
  DYNINSTallMetrics[8].set("sync_ops", EventCounter, aggSum, "Ops/sec",
			   createSyncOps, defaultPredicates);
  DYNINSTallMetrics[9].set("sync_wait", EventCounter, aggSum, "# Waiting",
			    createSyncWait, defaultPredicates, false);

  initialRequests = new instMapping[19];

  initialRequests[0].set("cmmd_debug", "DYNINSTnodeCreate", FUNC_ENTRY);
  initialRequests[1].set("CMRT_init", "DYNINSTnodeCreate", FUNC_ENTRY);
  initialRequests[2].set("cmmd_debug", "DYNINSTparallelInit", FUNC_EXIT);
  initialRequests[3].set("CMRT_init", "DYNINSTparallelInit", FUNC_ENTRY);
  initialRequests[4].set("cmmd_debug", "DYNINSTbreakPoint", FUNC_EXIT);
  initialRequests[5].set("CMRT_init", "DYNINSTbreakPoint", FUNC_ENTRY);
  initialRequests[6].set("main", "DYNINSTalarmExpire", FUNC_EXIT);

#ifdef notdef
  initialRequests[?].set("fork", "DYNINSTfork", FUNC_EXIT|FUNC_FULL_ARGS);
#endif

  initialRequests[7].set(EXIT_NAME, "DYNINSTalarmExpire", FUNC_ENTRY);
  initialRequests[8].set(EXIT_NAME, "DYNINSTprintCost", FUNC_ENTRY);
  initialRequests[9].set(EXIT_NAME, "DYNINSTbreakPoint", FUNC_ENTRY);
  initialRequests[10].set("main", "DYNINSTinit", FUNC_ENTRY);
  initialRequests[11].set("DYNINSTsampleValues", "DYNINSTreportNewTags", FUNC_ENTRY);
  initialRequests[12].set("CMMD_send", "DYNINSTrecordTag", FUNC_ENTRY|FUNC_ARG,
			  &tagArg);
  initialRequests[13].set("CMMD_receive", "DYNINSTrecordTag",
			  FUNC_ENTRY|FUNC_ARG, &tagArg);
  initialRequests[14].set("CMMD_receive_block", "DYNINSTrecordTag",
			  FUNC_ENTRY|FUNC_ARG, &tagArg);
  initialRequests[15].set("CMMD_send_block", "DYNINSTrecordTag",
			  FUNC_ENTRY|FUNC_ARG, &tagArg);
  initialRequests[16].set("CMMD_send_async", "DYNINSTrecordTag",
			  FUNC_ENTRY|FUNC_ARG, &tagArg);
  initialRequests[17].set("CMMD_receive_async", "DYNINSTrecordTag",
			  FUNC_ENTRY|FUNC_ARG, &tagArg);

  // KLUDGE TODO
  initialRequests[18].set(NULL, NULL, 0, NULL, false);

  return true;
};
