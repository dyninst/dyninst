/*
 * Copyright (c) 1996-2000 Barton P. Miller
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

// $Id: BPatch_init.C,v 1.15 2003/10/22 16:01:09 schendel Exp $

#define BPATCH_FILE


#include "dyninstAPI/src/dyninstP.h" // nullString

#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/process.h"

extern int getNumberOfCPUs();

int numberOfCPUs_;

pdvector<instMapping*> initialRequests;

pdvector<sym_data> syms_to_find;

bool dyninstAPI_init() {

  numberOfCPUs_ = getNumberOfCPUs();

  initDefaultPointFrequencyTable();

  // Protect our signal handler by overriding any which the application 
  // may already have or subsequently install.
#if defined(i386_unknown_linux2_0) || defined(i386_unknown_solaris2_5)
  // dyninst should really use the appropriate initOS function for this
  // but initOS is currently part of paradynd and (probably) won't
  // get moved over into dyninst until the event management does,
  // so here's a copy, just for dyninst.
#if defined( i386_unknown_linux2_0 )
  const char *sigactionF="__sigaction";
#else
  const char *sigactionF="_libc_sigaction";
#endif

  pdvector<AstNode*> argList(3);
  static AstNode  sigArg(AstNode::Param, (void*) 0); argList[0] = &sigArg;
  static AstNode  actArg(AstNode::Param, (void*) 1); argList[1] = &actArg;
  static AstNode oactArg(AstNode::Param, (void*) 2); argList[2] = &oactArg;
      
  initialRequests.push_back(new instMapping(sigactionF, "DYNINSTdeferSigHandler",
                                     FUNC_ENTRY|FUNC_ARG, argList));
      
  initialRequests.push_back(new instMapping(sigactionF, "DYNINSTresetSigHandler",
                                     FUNC_EXIT|FUNC_ARG, argList));
#endif

  return true;
}

