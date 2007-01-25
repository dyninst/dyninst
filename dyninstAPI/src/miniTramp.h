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

// $Id: miniTramp.h,v 1.15 2007/01/25 22:23:58 bernat Exp $

#ifndef MINI_TRAMP_H
#define MINI_TRAMP_H

#include "common/h/Types.h"
#include "dyninstAPI/src/codeRange.h"
#include "dyninstAPI/src/inst.h" // callOrder and callWhen
#include "dyninstAPI/src/instPoint.h" // generatedCodeObject
#include "dyninstAPI/src/multiTramp.h" // generatedCodeObject

// This is a serious problem: our code generation has no way to check
// the size of the buffer it's emitting to.
// 1 megabyte buffer
#define MAX_MINITRAMP_SIZE (0x100000)

// Callback func for deletion of a minitramp
class miniTramp;
class AstNode;
class AstMiniTrampNode;
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

  // Inline replacing of code
  miniTrampInstance(const miniTrampInstance *origMTI,
                    baseTrampInstance *newParent);
  
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

  bool hasChanged();

  bool generateCode(codeGen &gen,
                    Address baseInMutatee,
                    UNW_INFO_TYPE * * unwindInformation);
  bool installCode();
  
  void invalidateCode();

  bool linkCode();

  void removeCode(generatedCodeObject *subObject);

  generatedCodeObject *replaceCode(generatedCodeObject *newParent);

  bool safeToFree(codeRange *range);

  void freeCode();

  unsigned cost();

  process *proc() const;

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
            bool noCost);
  
  // Fork constructor
  miniTramp(const miniTramp *parMini, baseTramp *childT, process *proc);
  
  ~miniTramp();

  // Catchup...

  // Returns true if newMT is "after" (later in the chain) than the
  // curMT (and we're out-of-lining), _OR_ if the miniTramps
  // are generated in-line (in which case we miss it
  // completely). TODO: in-line needs to update this.
  
  static bool catchupRequired(miniTramp *curMT, miniTramp *newMT, bool active);

  // Generate the code necessary
  // We use a single global image of a minitramp.
  bool generateMT(registerSpace *rs);
  codeGen miniTrampCode_;
  
  // The delete operator, just without delete. Uninstruments minis.
  bool uninstrument();

  unsigned get_size_cr() const {
    assert(returnOffset);
    return returnOffset;
  }

  // Returns true if we were put in via a trap. Actually... returns if
  // all miniTrampInstances have multiTramps that were trapped to.
  // Since we can relocate (and not use a trap in one instance, but 
  // do in another).
  bool instrumentedViaTrap() const;

  // Register a callback for when the mini is finally deleted...
  void registerCallback(miniTrampFreeCallback cb, void *data) {
    callback = cb;
    callbackData = data;
  };


  // Returns true if all's well.
  bool checkMTStatus();

  miniTrampInstance *getMTInstanceByBTI(baseTrampInstance *instance,
                                        bool create_if_not_found = true);

  int ID;                    // used to match up miniTramps in forked procs
  Address returnOffset;      // Offset from base to the return addr
  unsigned size_;

  // Base tramp we're installed at
  baseTramp *baseT;

  // Sucks it out of the baseTramp
  instPoint *instP() const;
  int_function *func() const;

  // instPs can go away... keep a local process pointer to let us
  // use it in the future.

  process *proc_;

  process *proc() const { return proc_; }

  void deleteMTI(miniTrampInstance *);

  // This is nice for back-tracking.
  callWhen when; /* Pre or post */

  int cost;		     // cost in cycles of this inst req.
  bool noCost_;

  /**
   * When cleaning up minitramps and minitramp instances we could be doing a 
   * bottom-up cleanup (delete an instance, which deletes a minitramp) during gc
   * or top-down cleanup (delete a minitramp, which deletes an instance) when 
   * instrumentation is removing.  If topDownDelete_ is set to true, we're removing
   * instrumentation and the minitramp instance should not delete it's minitramp.
   **/
  bool topDownDelete_; 

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

  AstMiniTrampNode *ast_; // For regenerating miniTs
};


#endif
