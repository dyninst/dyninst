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

// $Id: miniTramp.h,v 1.6 2005/09/14 21:21:53 bernat Exp $

#ifndef MINI_TRAMP_H
#define MINI_TRAMP_H

#include "common/h/Types.h"
#include "dyninstAPI/src/codeRange.h"
#include "dyninstAPI/src/inst.h" // callOrder and callWhen
#include "dyninstAPI/src/ast.h" // assignAST
#include "dyninstAPI/src/instPoint.h" // generatedCodeObject
#include "dyninstAPI/src/multiTramp.h" // generatedCodeObject

// This is a serious problem: our code generation has no way to check
// the size of the buffer it's emitting to.
// 1 megabyte buffer
#define MAX_MINITRAMP_SIZE (0x100000)

// Callback func for deletion of a minitramp
class miniTramp;
typedef void (*miniTrampFreeCallback)(void *, miniTramp *);

// The new miniTramp class -- description of a particular minitramp.
// mini tramps are kind of annoying... there can be multiple copies of
// a single minitramp depending on whether a function has been cloned
// or not. So we need a single miniTramp structure that can handle
// multiple instantiations of actual code.

class miniTrampInstance : public generatedCodeObject {
    friend class miniTramp;
 public:

  miniTrampInstance(miniTramp *mini,
                    baseTrampInstance *bti) :
      generatedCodeObject(),
      baseTI(bti),
      mini(mini),
      trampBase(0),
      deleted(false) {
  }

  // FORK!
  miniTrampInstance(const miniTrampInstance *parMini,
                    baseTrampInstance *cBTI,
                    miniTramp *cMT,
                    process *child);
  
  // Need to have a destructor, so we can track when each instance goes away. 
  ~miniTrampInstance();

  baseTrampInstance *baseTI;
  miniTramp *mini;
  Address trampBase;
  bool deleted;

  Address get_address_cr() const { return trampBase; }
  unsigned get_size_cr() const; // needs miniTramp and is so defined
                                // in .C file
  void *getPtrToInstruction(Address addr) const;

  Address uninstrumentedAddr() const;
  
  unsigned maxSizeRequired();

  bool generateCode(codeGen &gen,
                    Address baseInMutatee);
  bool installCode();
  
  void invalidateCode();

  bool linkCode();

  void removeCode(generatedCodeObject *subObject);

  generatedCodeObject *replaceCode(generatedCodeObject *newParent);

  bool safeToFree(codeRange *range);

  void freeCode();

  unsigned cost();

  process *proc();

};

class miniTramp {
    friend class miniTrampInstance;
  friend class instPoint;
    // Global numbering of minitramps
  static int _id;

  miniTramp();

  public:

  miniTramp(callWhen when_,
            AstNode *ast,
	    baseTramp *base,
	    instPoint *inst,
            bool noCost);
  
  // Fork constructor
  miniTramp(const miniTramp *parMini, baseTramp *childT, process *proc);
  
  ~miniTramp();

  // Catchup...

  // Returns true if newMT is "after" (later in the chain) than the
  // curMT (and we're out-of-lining), _OR_ if the miniTramps
  // are generated in-line (in which case we miss it
  // completely). TODO: in-line needs to update this.
  
  static bool catchupRequired(miniTramp *curMT, miniTramp *newMT);

  // Generate the code necessary
  // We use a single global image of a minitramp.
  bool generateMT();
  codeGen miniTrampCode_;
  
  // The delete operator, just without delete. Uninstruments minis.
  bool uninstrument();

  unsigned get_size_cr() const {
    assert(returnOffset);
    return returnOffset;
  }

  // Register a callback for when the mini is finally deleted...
  void registerCallback(miniTrampFreeCallback cb, void *data) {
    callback = cb;
    callbackData = data;
  };


  // Returns true if all's well.
  bool checkMTStatus();

  miniTrampInstance *getMTInstanceByBTI(baseTrampInstance *instance);

  int ID;                    // used to match up miniTramps in forked procs
  Address returnOffset;      // Offset from base to the return addr
  unsigned size_;

  // Base tramp we're installed at
  baseTramp *baseT;

  // And the instPoint we belong to.
  instPoint *instP;

  // instPs can go away... keep a local process pointer to let us
  // use it in the future.

  process *proc_;

  process *proc() const { return proc_; }

  void deleteMTI(miniTrampInstance *);

  // This is nice for back-tracking.
  callWhen when; /* Pre or post */

  int cost;		     // cost in cycles of this inst req.
  bool noCost_;

  pdvector<miniTrampInstance *> instances;
  pdvector<miniTrampInstance *> deletedMTIs;

  // Make sure all jumps are consistent...
  bool correctMTJumps();

  // Previous and next minitramps
  miniTramp *prev;
  miniTramp *next;

  // Material to check when deleting this miniTramp
  miniTrampFreeCallback callback; /* Callback to be made when
					instance is deleted */
  void *callbackData;                /* Associated data */

  bool deleteInProgress; // Don't double-delete

  AstNode *ast_; // For regenerating miniTs
};


#endif
