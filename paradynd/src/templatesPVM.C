/*
 * Copyright (c) 1996 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
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
 * Generate all the templates in one file.
 *
 */

/* 
 * $Log: templatesPVM.C,v $
 * Revision 1.5  2003/01/02 19:52:21  schendel
 * updates so dyninstAPI and the dyninstAPI tests can be built with the
 * Solaris native compiler  - - - - - - - - - - - - - - - - - - - - - - - -
 * fixed warnings when compiling with Solaris native compiler;
 *
 * Revision 1.4  2000/07/27 15:24:47  hollings
 * Missed Commit of changes in include file location.
 *
 * Revision 1.3  2000/05/31 18:33:25  schendel
 * updates the daemon, util library, igen, dyninstAPI, and dyninstAPI test suite
 * to use the -fno-impilicit-templates flag instead of the -fexplicit-templates
 * flag, which is depracated.
 *
 * Revision 1.2  1996/08/16 21:20:12  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.1  1995/12/15 22:27:07  mjrg
 * Merged paradynd and paradyndPVM
 * Get module name for functions from symbol table in solaris
 * Fixed code generation for multiple instrumentation statements
 * Changed syntax of MDL resource lists
 *
 * Revision 1.2  1994/09/22  02:59:17  markc
 * Added stronger compiler warnings
 * Removed compiler warnings
 *
 * Revision 1.1  1994/06/29  03:00:37  hollings
 * New file for PVM specific template classes.
 *
 *
 */
#pragma implementation  "List.h"

#include "common/h/List.h"

typedef struct task;

typedef List<task *> v1;
template class List<task *>;
