/*
 * Copyright (c) 1996-2009 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/* $Id: inferiorRPC.h,v 1.9 2004/03/23 01:12:03 eli Exp $
 */

#ifndef _INFERIOR_RPC_H_
#define _INFERIOR_RPC_H_

#include "common/h/Types.h"

class AstNode;
class dyn_thread;
class process;
class dyn_lwp;
struct dyn_saved_regs;

// RPC state enumerated type
typedef enum { irpcNotValid, irpcNotRunning, irpcRunning, irpcWaitingForTrap,
               irpcNotReadyForIRPC } irpcState_t;

// RPC launch return type
typedef enum { irpcNoIRPC, irpcStarted, irpcAgain, irpcTrapSet, 
               irpcError } irpcLaunchState_t;

// inferior RPC callback function type
typedef void(*inferiorRPCcallbackFunc)(process *p, unsigned rpcid, void *data,
                                       void *result);


class rpcLwp;

struct inferiorRPCtoDo {  
   AstNode *action;
   bool noCost; // if true, cost model isn't updated by generated code.
   inferiorRPCcallbackFunc callbackFunc;
   void *userData;
   bool lowmem; /* set to true when the inferior is low on memory */
   rpcLwp *rpclwp;
   unsigned seq_num;
   irpcState_t irpcState_;
  // if DYNINSTinit, we launch it as regular RPC
  // otherwise launch it as MT RPC
};


struct inferiorRPCinProgress {
  // This structure keeps track of a launched inferiorRPC which we're
  // waiting to complete.  Don't confuse with 'inferiorRPCtoDo', 
  // which is more of a wait queue of RPCs to start launching.
  // Also note: It's only safe for 1 (one) RPC to be in progress at a time.
  // If you _really_ want to launch multiple RPCs at the same time, it's
  // actually easy to do...just do one inferiorRPC with a sequenceNode AST!
  // (Admittedly, that confuses the semantics of callback functions.  So the
  // official line remains: only 1 inferior RPC per process can be ongoing.)
  inferiorRPCcallbackFunc callbackFunc;
  void *userData;
  
  struct dyn_saved_regs *savedRegs; // crucial!
  
  Address origPC;
  
  bool wasRunning; // were we running when we launched the inferiorRPC?
  
  Address firstInstrAddr; // start location of temp tramp
  
  Address stopForResultAddr;
  // location of TRAP or ILL which marks point where paradynd should grab
  // the result register.  Undefined if no callback fn.
  Address justAfter_stopForResultAddr; // undefined if no callback fn.
  Register resultRegister; // undefined if no callback fn.
  
  void *resultValue; // undefined until we stop-for-result, at which time we
  // fill this in.  callback fn (which takes this value)
  // isn't invoked until breakAddr (the final break)
  
  Address breakAddr; // location of TRAP or ILL insn which 
  // marks the end of the inferiorRPC
  
  bool isSafeRPC ;   // was run as a "run later" funclet
  rpcLwp *rpclwp;      // Target the RPC to a specific kernel thread?
  unsigned seq_num;
};



#endif
