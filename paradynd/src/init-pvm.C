
/*
 * $Log: init-pvm.C,v $
 * Revision 1.3  1994/11/10 18:57:54  jcargill
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
  wallTimePredicates[0].set("/Process", nullPredicate, NULL);
  wallTimePredicates[1].set("/SyncObject/MsgTag", simplePredicate,
			   defaultMSGTagPredicate);
  wallTimePredicates[2].set("/SyncObject", invalidPredicate, NULL);
  wallTimePredicates[3].set("/Procedure", replaceBase, perModuleWallTime);
  wallTimePredicates[4].set("/Machine", nullPredicate, NULL);
  wallTimePredicates[5].set((char*)NULL, nullPredicate, NULL, false);

  procCallsPredicates = new resourcePredicate[5];
  procCallsPredicates[0].set("/Procedure", replaceBase, perModuleCalls);
  procCallsPredicates[1].set("/SyncObject", invalidPredicate, NULL);
  procCallsPredicates[2].set("/Machine", nullPredicate, NULL);
  procCallsPredicates[3].set("/Process", nullPredicate, NULL);
  procCallsPredicates[4].set((char*)NULL, nullPredicate, NULL, false);

  msgPredicates = new resourcePredicate[6];
  msgPredicates[0].set("/Procedure", simplePredicate, defaultModulePredicate);
  msgPredicates[1].set("/SyncObject/MsgTag", simplePredicate,
		       defaultMSGTagPredicate);
  msgPredicates[2].set("/SyncObject", nullPredicate, NULL);
  msgPredicates[3].set("/Machine", nullPredicate, NULL);
  msgPredicates[4].set("/Process", nullPredicate, NULL);
  msgPredicates[5].set((char*)NULL, nullPredicate, NULL, false);

  // TODO mdc - these should be provided for all flavors
  ioPredicates = new resourcePredicate[5];
  ioPredicates[0].set("/Procedure", simplePredicate, defaultModulePredicate);
  ioPredicates[1].set("/SyncObject", nullPredicate, NULL);
  ioPredicates[2].set("/Machine", nullPredicate, NULL);
  ioPredicates[3].set("/Process", nullPredicate, NULL);
  ioPredicates[4].set((char*)NULL, nullPredicate, NULL, false);
  
  defaultPredicates = new resourcePredicate[6];
  defaultPredicates[0].set("/Procedure", simplePredicate, defaultModulePredicate);
  defaultPredicates[1].set("/SyncObject/MsgTag", simplePredicate,		
			   defaultMSGTagPredicate);
  defaultPredicates[2].set("/SyncObject", invalidPredicate, NULL);
  defaultPredicates[3].set("/Machine", nullPredicate, NULL);
  defaultPredicates[4].set("/Process", nullPredicate, NULL);
  defaultPredicates[5].set((char*)NULL, nullPredicate, NULL, false);

  globalOnlyPredicates = new resourcePredicate[6];
  globalOnlyPredicates[0].set("/Procedure", simplePredicate, NULL);
  globalOnlyPredicates[1].set("/SyncObject/MsgTag", simplePredicate, NULL);
  globalOnlyPredicates[2].set("/SyncObject", invalidPredicate, NULL);
  globalOnlyPredicates[3].set("/Machine", nullPredicate, NULL);
  globalOnlyPredicates[4].set("/Process", nullPredicate, NULL);
  globalOnlyPredicates[5].set((char*)NULL, nullPredicate, NULL, false);

  assert (cpuTimePredicates &&
	  wallTimePredicates &&
	  procCallsPredicates &&
	  msgPredicates &&
	  globalOnlyPredicates && 
	  defaultPredicates &&
	  observedCostPredicates
	  );

  DYNINSTallMetrics = new metric[10];
  metricCount = 10;

  DYNINSTallMetrics[0].set("observed_cost", EventCounter,
			   aggMax, "# CPUs", createObservedCost,
			   observedCostPredicates);

  DYNINSTallMetrics[1].set("cpu", EventCounter, aggSum, "# CPUs",
			   createCPUTime, cpuTimePredicates);

  DYNINSTallMetrics[2].set("exec_time", EventCounter, aggSum, "%Time",
			   createExecTime, wallTimePredicates);

  DYNINSTallMetrics[3].set("procedure_calls", EventCounter, aggSum, "Calls/sec",
			   createProcCalls, procCallsPredicates);

  DYNINSTallMetrics[4].set( "msgs", EventCounter, aggSum, "Ops/sec",
			   createMsgs, defaultPredicates);

  DYNINSTallMetrics[5].set( "msg_bytes", EventCounter, aggSum, "Bytes/Sec",
			   createMsgBytesTotal, defaultPredicates);

  DYNINSTallMetrics[6].set("msg_bytes_sent", EventCounter, aggSum, "Bytes/Sec",
			   createMsgBytesSent, defaultPredicates);

  DYNINSTallMetrics[7].set("msg_bytes_recv", EventCounter, aggSum, "Bytes/Sec",
			   createMsgBytesRecv, defaultPredicates);

  DYNINSTallMetrics[8].set("sync_ops", EventCounter, aggSum, "Ops/sec",
			   createSyncOps, msgPredicates);

  DYNINSTallMetrics[9].set("sync_wait", EventCounter, aggSum, "# Waiting",
			    createSyncWait, msgPredicates, false);

/*
  DYNINSTallMetrics[10].set("io_wait", EventCounter, aggSum, "# Waiting",
			    createIOWait, ioPredicates);

  DYNINSTallMetrics[11].set("io_ops", EventCounter, aggSum, "Ops/sec",
			    createIOOps, ioPredicates, false);

  DYNINSTallMetrics[13].set("io_bytes", EventCounter, aggSum, "Bytes/Sec",
			    createIOBytesTotal, ioPredicates, false);
			    */

  initialRequests = new instMapping[8];

  initialRequests[0].set("pvm_send", "DYNINSTrecordTag",
			 FUNC_ENTRY|FUNC_ARG, &tagArg);
  initialRequests[1].set("pvm_recv", "DYNINSTrecordTag",
			 FUNC_ENTRY|FUNC_ARG, &tagArg);
  initialRequests[2].set("main", "DYNINSTalarmExpire", FUNC_EXIT);
  initialRequests[3].set("main", "DYNINSTinit", FUNC_ENTRY);
  initialRequests[4].set(EXIT_NAME, "DYNINSTalarmExpire", FUNC_ENTRY);
  initialRequests[5].set(EXIT_NAME, "DYNINSTbreakPoint", FUNC_ENTRY);
  initialRequests[6].set("DYNINSTsampleValues", "DYNINSTreportNewTags", FUNC_ENTRY);

  // KLUDGE TODO - false means end of array
  initialRequests[7].set(NULL, NULL, 0, NULL, false);

  return true;
};


