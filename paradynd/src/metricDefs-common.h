/*
 * Copyright (c) 1993, 1994 Barton P. Miller, Jeff Hollingsworth,
 *     Bruce Irvin, Jon Cargille, Krishna Kunchithapadam, Karen
 *     Karavanic, Tia Newhall, Mark Callaghan.  All rights reserved.
 * 
 * This software is furnished under the condition that it may not be
 * provided or otherwise made available to, or used by, any other
 * person, except as provided for by the terms of applicable license
 * agreements.  No title to or ownership of the software is hereby
 * transferred.  The name of the principals may not be used in any
 * advertising or publicity related to this software without specific,
 * written prior authorization.  Any use of this software must include
 * the above copyright notice.
 *
 */

/*
 * $Log: metricDefs-common.h,v $
 * Revision 1.2  1994/07/05 03:26:14  hollings
 * observed cost model
 *
 * Revision 1.1  1994/06/29  02:52:42  hollings
 * Added metricDefs-common.{C,h}
 * Added module level performance data
 * cleanedup types of inferrior addresses instrumentation defintions
 * added firewalls for large branch displacements due to text+data over 2meg.
 * assorted bug fixes.
 *
 *
 */
AstNode *defaultModulePredicate(metricDefinitionNode *mn, char *constraint,
    AstNode *pred);

AstNode *defaultProcessPredicate(metricDefinitionNode *mn, char *process,
    AstNode *pred);

void createProcCalls(metricDefinitionNode *mn, AstNode *pred);

void instAllFunctions(metricDefinitionNode *nm,
		      int tag,		/* bit mask to use */
		      AstNode *enterAst,
		      AstNode *leaveAst);

dataReqNode *createCPUTime(metricDefinitionNode *mn, AstNode *pred);
dataReqNode *createObservedCost(metricDefinitionNode *mn, AstNode *pred);

void createExecTime(metricDefinitionNode *mn, AstNode *pred);

void createSyncOps(metricDefinitionNode *mn, AstNode *trigger);

void createActiveProcesses(metricDefinitionNode *mn, AstNode *trigger);

void createMsgs(metricDefinitionNode *mn, AstNode *trigger);

void dummyCreate(metricDefinitionNode *mn, AstNode *trigger);

void createSyncWait(metricDefinitionNode *mn, AstNode *trigger);

void perModuleWallTime(metricDefinitionNode *mn, 
			  char *constraint, 
			  AstNode *pred);

void perModuleCPUTime(metricDefinitionNode *mn, 
		      char *constraint, 
		      AstNode *trigger);

void perModuleCalls(metricDefinitionNode *mn, 
		    char *constraint, 
		    AstNode *trigger);

extern resourcePredicate observedCostPredicates[];
