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

/* 
 * $Log: metricDef.h,v $
 * Revision 1.10  2004/09/21 05:33:45  jaw
 * Changes mdl to use BPatch_snippets instead of AstNodes.  These account for
 * many, but not all AstNodes in paradynd.  Remaining groups of AstNodes
 * include:  OS specific init calls (ie anything that uses the class
 * instMapping -- these will have to be dealt with as a group), Anything
 * related to PAPI, MPI...  probably more.
 *
 * Some more (smallish) modifications of the paradynd init including a bugfix
 * (for a bug introduced by my last commit) for Windows involving the DllMain
 * function.  Also variables in libparadynRT that are set by paradynd during
 * the init stages are now dealt with as BPatch_variableExpr's.
 *
 * Revision 1.9  2004/03/23 01:12:35  eli
 * Updated copyright string
 *
 * Revision 1.8  2002/05/10 18:37:39  schendel
 * add ability in daemon to adjust existing metric-focuses for thread creation
 *   - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * removed extraneous header file, metric.h;
 * changed name of class from metricDefinitionNode to metricFocusNode;
 *
 * Revision 1.7  1997/02/21 20:16:00  naim
 * Moving files from paradynd to dyninstAPI + eliminating references to
 * dataReqNode from the ast class. This is the first pre-dyninstAPI commit! - naim
 *
 * Revision 1.6  1996/08/16 21:19:25  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.5  1995/02/16 08:53:45  markc
 * Corrected error in comments -- I put a "star slash" in the comment.
 *
 * Revision 1.4  1995/02/16  08:33:55  markc
 * Changed igen interfaces to use strings/vectors rather than char igen-arrays
 * Changed igen interfaces to use bool, not Boolean.
 * Cleaned up symbol table parsing - favor properly labeled symbol table objects
 * Updated binary search for modules
 * Moved machine dependnent ptrace code to architecture specific files.
 * Moved machine dependent code out of class process.
 * Removed almost all compiler warnings.
 * Use "posix" like library to remove compiler warnings
 *
 * Revision 1.3  1994/11/10  18:58:10  jcargill
 * The "Don't Blame Me Either" commit
 *
 * Revision 1.2  1994/11/09  18:40:18  rbi
 * the "Don't Blame Me" commit
 *
 * Revision 1.1  1994/11/01  16:58:06  markc
 * Prototypes
 *
 */


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
