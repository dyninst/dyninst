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
 * $Log: init-sunos.C,v $
 * Revision 1.19  1997/07/14 20:43:23  naim
 * Fixing problem with fork on x86 - naim
 *
 * Revision 1.18  1997/05/07 19:01:55  naim
 * Getting rid of old support for threads and turning it off until the new
 * version is finished. Additionally, new superTable, baseTable and superVector
 * classes for future support of multiple threads. The fastInferiorHeap class has
 * also changed - naim
 *
 * Revision 1.17  1997/03/23 16:53:11  zhichen
 * based on process::pdFlavor, set initial inst accordingly.
 *
 * Revision 1.16  1997/02/21 20:15:47  naim
 * Moving files from paradynd to dyninstAPI + eliminating references to
 * dataReqNode from the ast class. This is the first pre-dyninstAPI commit! - naim
 *
 * Revision 1.15  1997/01/27 19:40:41  naim
 * Part of the base instrumentation for supporting multithreaded applications
 * (vectors of counter/timers) implemented for all current platforms +
 * different bug fixes - naim
 *
 * Revision 1.14  1996/12/11 17:02:48  mjrg
 * fixed problems with handling of fork and exec
 *
 * Revision 1.13  1996/11/14 14:27:00  naim
 * Changing AstNodes back to pointers to improve performance - naim
 *
 * Revision 1.12  1996/10/31 08:44:32  tamches
 * in initOS(), main no longer calls DYNINSTinit
 *
 * Revision 1.11  1996/09/26 18:58:32  newhall
 * added support for instrumenting dynamic executables on sparc-solaris
 * platform
 *
 * Revision 1.10  1996/08/16 21:18:44  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.9  1996/08/12 16:27:16  mjrg
 * Code cleanup: removed cm5 kludges and some unused code
 *
 * Revision 1.8  1996/05/08 23:54:45  mjrg
 * added support for handling fork and exec by an application
 * use /proc instead of ptrace on solaris
 * removed warnings
 *
 * Revision 1.7  1996/03/20 17:02:44  mjrg
 * Added multiple arguments to calls.
 * Instrument pvm_send instead of pvm_recv to get tags.
 *
 * Revision 1.6  1996/03/01 22:31:59  mjrg
 * Replaced calls at the exit point by a call to DYNINSTexit
 *
 * Revision 1.5  1995/12/15 22:26:48  mjrg
 * Merged paradynd and paradyndPVM
 * Get module name for functions from symbol table in solaris
 * Fixed code generation for multiple instrumentation statements
 * Changed syntax of MDL resource lists
 *
 * Revision 1.4  1995/08/24  15:03:54  hollings
 * AIX/SP-2 port (including option for split instruction/data heaps)
 * Tracing of rexec (correctly spawns a paradynd if needed)
 * Added rtinst function to read getrusage stats (can now be used in metrics)
 * Critical Path
 * Improved Error reporting in MDL sematic checks
 * Fixed MDL Function call statement
 * Fixed bugs in TK usage (strings passed where UID expected)
 *
 */

#include "paradynd/src/metric.h"
#include "paradynd/src/internalMetrics.h"
#include "dyninstAPI/src/inst.h"
#include "paradynd/src/init.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/util.h"
#include "dyninstAPI/src/os.h"

// NOTE - the tagArg integer number starting with 0.  
static AstNode *tagArg = new AstNode(AstNode::Param, (void *) 1);
static AstNode *cmdArg = new AstNode(AstNode::Param, (void *) 4);
static AstNode *tidArg = new AstNode(AstNode::Param, (void *) 0);
static AstNode *retVal = new AstNode(AstNode::ReturnVal, (void *) 0);
#if defined(SHM_SAMPLING) && defined(MT_THREAD)
static AstNode *THRidArg = new AstNode(AstNode::Param, (void *) 5);
#endif

bool initOS() {

//  initialRequests += new instMapping("main", "DYNINSTinit", FUNC_ENTRY);
// (obsoleted by installBootstrapInst() --ari)


  initialRequests += new instMapping("main", "DYNINSTexit", FUNC_EXIT);

  initialRequests += new instMapping(EXIT_NAME, "DYNINSTexit", FUNC_ENTRY);
  if(process::pdFlavor != string("cow"))
  {
#if defined(i386_unknown_solaris2_5)
  	initialRequests += new instMapping("_fork", "DYNINSTfork", 
				     FUNC_EXIT|FUNC_ARG, retVal);
#else
  	initialRequests += new instMapping("fork", "DYNINSTfork", 
				     FUNC_EXIT|FUNC_ARG, retVal);

      	initialRequests += new instMapping("_libc_fork", "DYNINSTfork", 
				     FUNC_EXIT|FUNC_ARG, retVal);
#endif
  }
#if defined(SHM_SAMPLING) && defined(MT_THREAD)
  initialRequests += new instMapping("MY_thr_create", "DYNINSTthreadCreate", 
                                     FUNC_EXIT|FUNC_ARG, THRidArg);
#endif

  if(process::pdFlavor != string("cow"))
  {
  	initialRequests += new instMapping("execve", "DYNINSTexec",
				     FUNC_ENTRY|FUNC_ARG, tidArg);
  	initialRequests += new instMapping("execve", "DYNINSTexecFailed", FUNC_EXIT);
  	initialRequests += new instMapping("_execve", "DYNINSTexec",
				     FUNC_ENTRY|FUNC_ARG, tidArg);
  	initialRequests += new instMapping("_execve", "DYNINSTexecFailed", FUNC_EXIT);
  }

#ifndef SHM_SAMPLING
  initialRequests += new instMapping("DYNINSTsampleValues", "DYNINSTreportNewTags",
				 FUNC_ENTRY);
#endif

  if(process::pdFlavor != string("cow"))
  {
  	initialRequests += new instMapping("rexec", "DYNINSTrexec",
				 FUNC_ENTRY|FUNC_ARG, cmdArg);
  }
//   initialRequests += new instMapping("PROCEDURE_LINKAGE_TABLE","DYNINSTdynlinker",FUNC_ENTRY);


#ifdef PARADYND_PVM
  char *doPiggy;

  initialRequests += new instMapping("pvm_send", "DYNINSTrecordTag",
				 FUNC_ENTRY|FUNC_ARG, tagArg);

  // kludge to get Critical Path to work.
  // XXX - should be tunable constant.
  doPiggy = getenv("DYNINSTdoPiggy");
  if (doPiggy) {
      initialRequests += new instMapping("main", "DYNINSTpvmPiggyInit", FUNC_ENTRY);
      initialRequests+= new instMapping("pvm_send", "DYNINSTpvmPiggySend",
                           FUNC_ENTRY|FUNC_ARG, tidArg);
      initialRequests += new instMapping("pvm_recv", "DYNINSTpvmPiggyRecv", FUNC_EXIT);
      initialRequests += new instMapping("pvm_mcast", "DYNINSTpvmPiggyMcast",
                           FUNC_ENTRY|FUNC_ARG, tidArg);
  }
#endif

  return true;
};
