
#ifndef METDEF_H
#define METDEF_H

/* 
 * $Log: metricDef.h,v $
 * Revision 1.2  1994/11/09 18:40:18  rbi
 * the "Don't Blame Me" commit
 *
 * Revision 1.1  1994/11/01  16:58:06  markc
 * Prototypes
 *
 */

#include "ast.h"
#include "metric.h"

// os independent predicate functions
// the following functions return a predicate (counter) that can
// be used to evaluate a condition
extern AstNode *defaultModulePredicate(metricDefinitionNode *mn,
				       char *constraint,
				       AstNode *pred);

extern AstNode *defaultProcessPredicate(metricDefinitionNode *mn,
					char *process,
					AstNode *pred);

extern AstNode *defaultMSGTagPredicate(metricDefinitionNode *mn, 
				       char *tag,
				       AstNode *trigger);

extern AstNode *defaultProcessPredicate(metricDefinitionNode *mn,
					char *constraint,
					AstNode *trigger);

// Note - the following functions always return NULL since they
// are predicates that are used to replace a base metric, they
// do not return a predicate that can be used to evaluate a condition
extern AstNode *perModuleWallTime(metricDefinitionNode *mn, 
				  char *constraint, 
				  AstNode *trigger);

extern AstNode *perModuleCPUTime(metricDefinitionNode *mn, 
				 char *constraint, 
				 AstNode *trigger);

extern AstNode *perModuleCalls(metricDefinitionNode *mn, 
			       char *constraint, 
			       AstNode *trigger);

// os independent metric functions
extern void createCPUTime(metricDefinitionNode *mn, AstNode *pred);
extern void createProcCalls(metricDefinitionNode *mn, AstNode *pred);
extern void createObservedCost(metricDefinitionNode *mn, AstNode *pred);
extern void createCPUTime(metricDefinitionNode *mn, AstNode *pred);
extern void createExecTime(metricDefinitionNode *mn, AstNode *pred);
extern void createSyncOps(metricDefinitionNode *mn, AstNode *trigger);
extern void createActiveProcesses(metricDefinitionNode *mn, AstNode *trigger);
extern void createMsgs(metricDefinitionNode *mn, AstNode *trigger);
extern void dummyCreate(metricDefinitionNode *mn, AstNode *trigger);

// os dependent metric functions
extern void createSyncWait(metricDefinitionNode *mn, AstNode *trigger);
extern void createMsgBytesSent(metricDefinitionNode *mn, AstNode *tr);
extern void createMsgBytesRecv(metricDefinitionNode *mn, AstNode *tr);
extern void createMsgBytesTotal(metricDefinitionNode *mn, AstNode *tr);

extern void createIOBytesTotal(metricDefinitionNode *mn, AstNode *tr);
extern void createIOBytesRead(metricDefinitionNode *mn, AstNode *tr);
extern void createIOBytesWrite(metricDefinitionNode *mn, AstNode *tr);
extern void createIOOps(metricDefinitionNode *mn, AstNode *tr);
extern void createIOWait(metricDefinitionNode *mn, AstNode *tr);
extern void createIOReadWait(metricDefinitionNode *mn, AstNode *tr);
extern void createIOWriteWait(metricDefinitionNode *mn, AstNode *tr);

extern void instAllFunctions(metricDefinitionNode *nm,
			     int tag,		/* bit mask to use */
			     AstNode *enterAst,
			     AstNode *leaveAst);

#endif
