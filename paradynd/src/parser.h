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

/*
 * parser.h
 *
 * $Log: parser.h,v $
 * Revision 1.5  2004/09/21 05:33:45  jaw
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
 * Revision 1.4  2004/03/23 01:12:35  eli
 * Updated copyright string
 *
 * Revision 1.3  1996/08/16 21:19:29  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.2  1994/09/22 02:19:28  markc
 * Changed names for resource classes
 *
 * Revision 1.1  1994/01/27  20:31:33  hollings
 * Iinital version of paradynd speaking dynRPC igend protocol.
 *
 * Revision 1.2  1993/10/19  15:27:54  hollings
 * AST based mini-tramp code generator.
 *
 * Revision 1.1  1993/03/19  22:51:05  hollings
 * Initial revision
 *
 *
 */

struct parseStack {
    int                 i;
    double              f;
    char                *cp;
    resource		*r;
    resourceListRec	*rl;
    opCode		op;
    BPatch_snippet	*snip;
    dataReqNode		*dp;
};

#define YYSTYPE struct parseStack
