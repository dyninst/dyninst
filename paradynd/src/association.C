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
 * association.C - Manage mapping information (associations)
 *
 * $Log: association.C,v $
 * Revision 1.11  1997/10/10 00:41:24  tamches
 * removed a warning
 *
 * Revision 1.10  1997/02/21 20:15:34  naim
 * Moving files from paradynd to dyninstAPI + eliminating references to
 * dataReqNode from the ast class. This is the first pre-dyninstAPI commit! - naim
 *
 * Revision 1.9  1996/08/16 21:18:11  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.8  1995/09/26 20:17:38  naim
 * Adding error messages using showErrorCallback function for paradynd
 *
 * Revision 1.7  1995/08/24  15:03:42  hollings
 * AIX/SP-2 port (including option for split instruction/data heaps)
 * Tracing of rexec (correctly spawns a paradynd if needed)
 * Added rtinst function to read getrusage stats (can now be used in metrics)
 * Critical Path
 * Improved Error reporting in MDL sematic checks
 * Fixed MDL Function call statement
 * Fixed bugs in TK usage (strings passed where UID expected)
 *
 * Revision 1.6  1995/02/16  08:52:45  markc
 * Corrected error in comments -- I put a "star slash" in the comment.
 *
 * Revision 1.5  1995/02/16  08:32:46  markc
 * Changed igen interfaces to use strings/vectors rather than charigen-arrays
 * Changed igen interfaces to use bool, not Boolean.
 * Cleaned up symbol table parsing - favor properly labeled symbol table objects
 * Updated binary search for modules
 * Moved machine dependnent ptrace code to architecture specific files.
 * Moved machine dependent code out of class process.
 * Removed almost all compiler warnings.
 * Use "posix" like library to remove compiler warnings
 *
 * Revision 1.4  1994/11/02  10:59:43  markc
 * Replaced string-handles
 *
 */

#include "util/h/headers.h"
#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/dyninstP.h"
#include "dyninstAPI/src/util.h"
#include "paradynd/src/main.h"

#include <stdio.h>
#include <assert.h>

// TODO -- clean me up
void newAssoc(process *, char *abstraction, char *type, char *key, 
	      char *value)
{

  /* 
   *  Call abstraction-specific translations here
   *  For example, an abstraction may want to 
   *  translate memory addresses to names using the symbol 
   *  table.
   */

  tp->mappingInfoCallback(0, abstraction, type, key, value);
}






