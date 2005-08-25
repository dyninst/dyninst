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

// $Id: instPoint.h,v 1.22 2005/08/25 22:45:42 bernat Exp $
// Defines class instPoint

#ifndef _INST_POINT_H_
#define _INST_POINT_H_

#include <stdlib.h>
#include "common/h/Types.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/arch.h" // instruction
#include "dyninstAPI/src/codeRange.h"

class image_func;
class int_function;
class instPoint;
class process;
class image;
class AstNode;
class int_basicBlock;
#if defined(__XLC__) || defined(__xlC__)
class BPatch_point;
#endif
class Frame;
class instPointInstance;

// Types of points
typedef enum {
  noneType,
  functionEntry,
  functionExit,
  callSite,
  otherPoint
} instPointType_t;

// We now maintain two data structures, the instPoint and the
// multiTramp. These were obtained by splitting the old instPoint
// class into its logical component parts. An instPoint is the
// user-visible layer, and describes the instrumentation of a single
// logical (e.g. function entry or edge) or physical (e.g. single
// instruction) location in the program. The multipoint describes the
// implementation (which instructions are overwritten) and provides a
// mapping from instPoints to base tramps.

class multiTramp;
class instPoint;
class instruction;

// As above: common class for both parse-time and run-time instPoints

class instPointBase {
 private:
    instPointBase();

 public:
  static unsigned int id_ctr;  
  instPointType_t getPointType() const { return ipType_; }

  // Single instruction we're instrumenting (if at all)
  const instruction &insn() const { return insn_; }
  instPointBase(instruction insn,
                instPointType_t type) :
    ipType_(type),
    insn_(insn)
      { id_ = id_ctr++; }
  // We need to have a manually-set-everything method
  instPointBase(instruction insn,
                instPointType_t type,
                unsigned int id) :
      id_(id),
      ipType_(type),
    insn_(insn)
      {}

  int id() const { return id_; }
    
 protected:
  unsigned int id_;
  instPointType_t ipType_;
  instruction insn_;
};

class image_instPoint : public instPointBase {
 private:
    image_instPoint();
 public:
    // Entry/exit
    image_instPoint(Address offset,
                  instruction insn,
		  image_func *func,
		  instPointType_t type);
    // Call site
    image_instPoint(Address offset,
                    instruction insn,
                    image_func *func,
                    Address callTarget_,
                    bool isDynamicCall);
  

  Address offset_;
  Address offset() const { return offset_; }

  image_func *func() const { return func_; }
  image_func *func_;
  // For call-site points:

  image_func *callee_;
  Address callTarget_;
  Address callTarget() const { return callTarget_; }
  bool isDynamicCall_; 
  bool isDynamicCall() const { return isDynamicCall_; }

  image_func *getCallee() const { return callee_; }
  void setCallee(image_func *f) { callee_ = f; }
  static int compare(image_instPoint *&ip1,
                     image_instPoint *&ip2) {
      if (ip1->offset() < ip2->offset())
          return -1;
      if (ip2->offset() < ip1->offset())
          return 1;
      assert(ip1 == ip2);
      return 0;
  };
};

// The actual instPoint is a little more interesting. It wraps the
// abstract view of an instrumentation point. In reality, it may map
// to multiple addresses (if the function it targets was relocated),
// and so may use multiple multiTramps. So we have a 1:many
// mapping. This is taken care of by keeping a vector of iPTarget
// structures.

class instPointInstance {
    friend class instPoint;
    friend class multiTramp;

    instPointInstance() { assert(0); }
 private:
    instPointInstance(Address a, bblInstance *b, instPoint *ip) :
        addr_(a), block_(b), multiID_(0), point(ip), disabled(false) {
    }

    // No end of trouble if this is a subclass..
    Address addr_; // Address of this particular version
    bblInstance *block_; // And the block instance we're attached to.

    unsigned multiID_;

    instPoint *point; // Backchain pointer

    bool generateInst();
    bool installInst();
    bool linkInst();

    bool disabled;

 public:
    // If we re-generate code we toss the old
    // multiTramp and make a new one; we then need
    // to update everyone with a handle to the multi.

    process *proc() const;
    int_function *func() const;
    
    void updateMulti(unsigned multi);

    multiTramp *multi() const;
    unsigned multiID() const { return multiID_; } // We need this to handle backtracing
    Address addr() const { return addr_; }
    bblInstance *block() const { return block_; }

    void updateVersion();
};

class instPoint : public instPointBase {
    friend class instPointInstance;
    friend class baseTramp;
    friend class multiTramp;
 private:
    // Generic instPoint...
    instPoint(process *proc,
              instruction insn,
              Address addr,
              int_basicBlock *block);

    instPoint(process *proc,
              image_instPoint *img_p,
              Address addr,
              int_basicBlock *block);

    // Fork instPoint
    instPoint(instPoint *parP,
              int_basicBlock *child);

    // A lot of arbitrary/parse creation work can be shared
    static bool commonIPCreation(instPoint *newIP);

    // On windows, an internally-defined class cannot access private
    // members of the "parent" class. This works on all other
    // platforms. Windows is b0rken...

 public:
    bool updateInstances();
    pdvector<instPointInstance *> instances;

    // Adding instances is expensive; if the function
    // hasn't changed, we can avoid doing so.
    int funcVersion;

  // Make a new instPoint at an arbitrary location
  static instPoint *createArbitraryInstPoint(Address addr,
					     process *proc);

  static instPoint *createParsePoint(int_function *func,
                                     image_instPoint *img_p);

  static instPoint *createForkedPoint(instPoint *p, int_basicBlock *child);

  ~instPoint();

  // Get the correct multitramp for a given function (or address)
  multiTramp *getMulti(int_function *func);
  multiTramp *getMulti(Address addr);

  // Get the instPointInstance at this addr
  instPointInstance *getInstInstance(Address addr);

  // Get the appropriate first (or last) miniTramp for the given callWhen
  miniTramp *getFirstMini(callWhen when);
  miniTramp *getLastMini(callWhen when);

  // For call-site inst points: TODO: what about relocation? This
  // keeps pointing at the old version? Or should we get rid of the
  // instPoint dodge to get the callee and store the information in
  // the int_function directly?
  int_function *callee_;
  bool isDynamicCall_;
  bool isDynamicCall() const { return isDynamicCall_; }

  // Get the base tramp (conglomerate) corresponding to this instPoint
  // (conglomerate)
  // May go out and make a new baseTramp... or find the corresponding
  // one in a neighboring instPoint.
  baseTramp *getBaseTramp(callWhen when);

  // Perform retroactive changes to the process, such as changing a
  // return address or moving a PC value.
  bool instrSideEffect(Frame &frame);

  // Called after an object is mapped to finalize inter-module calls.

  // findCallee: finds the function called by the instruction corresponding
  // to the instPoint "instr". If the function call has been bound to an
  // address, then the callee function is returned in "target" and the
  // instPoint "callee" data member is set to pt to callee's int_function.
  // If the function has not yet been bound, then "target" is set to the
  // int_function associated with the name of the target function (this is
  // obtained by the PLT and relocation entries in the image), and the instPoint
  // callee is not set.  If the callee function cannot be found, (e.g. function
  // pointers, or other indirect calls), it returns false.
  // Returns false on error (ex. process doesn't contain this instPoint).
  
  // TODO: fix original comment; callee_ is set, but returned directly.
  int_function *findCallee();

  // Is this the instPoint referred to?
  bool match(Address addr) const;

  process *proc() const { return proc_; }
  
  int_basicBlock *block() const;
  int_function *func() const;

  Address addr() const { return addr_; }

  // We use a three-phase instrumentation structure:

  // 1) Insert multiple pieces of instrumentation at multiple
  // 1) locations;

  // 2) Generate code for everything inserted in step 1 as well as all
  // pre-existing instrumentation that has been modified by step 1;

  // 3) Insert jumps to the code generated in step 2, and relocate
  // functions if necessary.

  // This function does it all:
  // Doesn't handle deferring though
  miniTramp *instrument(AstNode *ast,
                        callWhen when,
                        callOrder order,
                        bool trampRecursive,
                        bool noCost);

  // Step 1:
  miniTramp *addInst(AstNode *&ast,
                     callWhen when,
                     callOrder order,
                     bool trampRecursive,
                     bool noCost);

  // Step 2:
  bool generateInst();
  // And 3:
  // We split this so that we can relocate between generation and installing
  bool installInst();


  // Determine whether instrumentation will go in smoothly
  // At this point, "given the stacks, can we insert a jump
  // or not"?
  bool checkInst(pdvector<Address> &checkPCs);

  // Step 4:
  // TODO: if we're out-of-lining miniTramps or something, this should
  // be the call that causes linkage of the OOL MT to occur, just for
  // completeness of the model.
  bool linkInst();

  // Catchup: 1) does a PC correspond to the area covered by an
  // instPoint (aka multiTramp). 2) is the PC "before" or "after"
  // the provided new instrumentation?
  // Mmm catchup result: not in instrumentation, pre, or post.
  typedef enum { noMatch_c, notMissed_c, missed_c } catchup_result_t;

  // Explicitly does not require a frame; while that would be nice, 
  // we're eventually going to be called by Dyninst, and the BPatch_frame
  // class doesn't have a Frame member (although it's arguable that it should).
  catchup_result_t catchupRequired(Address pc,
                                   miniTramp *mt);

  bool createMiniTramp(AstNode *&ast,
		       bool noCost,
		       int &trampCost,
		       Address &trampSize,
		       Address &retOffset);

  void updateCost(miniTramp *mt);

  int getPointCost();

  class iterator {
      instPoint *point;
      unsigned index;
      iterator();
  public:
      iterator(instPoint *point) :
          point(point), index(0) {
          point->updateInstances();
      }
      instPointInstance *operator*() {
          if (index < point->instances.size())
              return point->instances[index];
          else
              return NULL;
      }
      instPointInstance *operator++(int) {
          if (index < point->instances.size()) {
              instPointInstance *inst = point->instances[index];
              index++;
              return inst;
          }
          return NULL;
      }
  };

  baseTramp *preBaseTramp() const { return preBaseTramp_; }
  baseTramp *postBaseTramp() const { return postBaseTramp_; }
  baseTramp *targetBaseTramp() const { return targetBaseTramp_; }

  
 private:
  baseTramp *preBaseTramp_;
  baseTramp *postBaseTramp_;
  baseTramp *targetBaseTramp_;

  process *proc_;

  image_instPoint *img_p_;

  // The block we're attached to.
  int_basicBlock *block_;
  Address addr_;

 public:
  // Register optimization
  int *liveRegisters;
  int *liveFPRegisters;
  int *liveSPRegisters;
  // AIX only.
};

typedef instPoint::iterator instPointIter;

#endif /* _INST_POINT_H_ */
