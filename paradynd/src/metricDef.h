/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

#ifndef METDEF_H
#define METDEF_H

#ifdef NOTDEF // PDSEP
#include "dyninstAPI/src/ast.h"

// os independent predicate functions
// the following functions return a predicate (counter) that can
// be used to evaluate a condition
extern AstNode *defaultModulePredicate(metricFocusNode *mn,
				       char *constraint,
				       AstNode *pred);

extern AstNode *defaultProcessPredicate(metricFocusNode *mn,
					char *process,
					AstNode *pred);

extern AstNode *defaultMSGTagPredicate(metricFocusNode *mn, 
				       char *tag,
				       AstNode *trigger);

extern AstNode *defaultProcessPredicate(metricFocusNode *mn,
					char *constraint,
					AstNode *trigger);

// Note - the following functions always return NULL since they
// are predicates that are used to replace a base metric, they
// do not return a predicate that can be used to evaluate a condition
extern AstNode *perModuleWallTime(metricFocusNode *mn, 
				  char *constraint, 
				  AstNode *trigger);

extern AstNode *perModuleCPUTime(metricFocusNode *mn, 
				 char *constraint, 
				 AstNode *trigger);

extern AstNode *perModuleCalls(metricFocusNode *mn, 
			       char *constraint, 
			       AstNode *trigger);

// os independent metric functions
extern void createCPUTime(metricFocusNode *mn, AstNode *pred);
extern void createProcCalls(metricFocusNode *mn, AstNode *pred);
extern void createObservedCost(metricFocusNode *mn, AstNode *pred);
extern void createCPUTime(metricFocusNode *mn, AstNode *pred);
extern void createExecTime(metricFocusNode *mn, AstNode *pred);
extern void createSyncOps(metricFocusNode *mn, AstNode *trigger);
extern void createMsgs(metricFocusNode *mn, AstNode *trigger);
extern void dummyCreate(metricFocusNode *mn, AstNode *trigger);

// os dependent metric functions
extern void createSyncWait(metricFocusNode *mn, AstNode *trigger);
extern void createMsgBytesSent(metricFocusNode *mn, AstNode *tr);
extern void createMsgBytesRecv(metricFocusNode *mn, AstNode *tr);
extern void createMsgBytesTotal(metricFocusNode *mn, AstNode *tr);

extern void createIOBytesTotal(metricFocusNode *mn, AstNode *tr);
extern void createIOBytesRead(metricFocusNode *mn, AstNode *tr);
extern void createIOBytesWrite(metricFocusNode *mn, AstNode *tr);
extern void createIOOps(metricFocusNode *mn, AstNode *tr);
extern void createIOWait(metricFocusNode *mn, AstNode *tr);
extern void createIOReadWait(metricFocusNode *mn, AstNode *tr);
extern void createIOWriteWait(metricFocusNode *mn, AstNode *tr);

extern void instAllFunctions(metricFocusNode *nm,
			     unsigned tag,		/* bit mask to use */
			     AstNode *enterAst,
			     AstNode *leaveAst);
#endif // NOTDEF PDSEP
#endif
