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

// $Id: instPoint.h,v 1.49 2008/09/08 16:44:04 bernat Exp $
// Defines class instPoint

#ifndef _INST_POINT_H_
#define _INST_POINT_H_

#include <stdlib.h>
#include "common/h/Types.h"
#include "dyninstAPI/src/inst.h"
#include "common/h/arch.h" // instruction
#include "dyninstAPI/src/codeRange.h"
#include "common/h/stats.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/bitArray.h"

#include "arch-forward-decl.h" // instruction

class image_func;
class int_function;
class instPoint;
class process;
class image;

class int_block;
class Frame;
class instPointInstance;

// Types of points
typedef enum {
  noneType,
  functionEntry,
  functionExit,
  callSite,
  abruptEnd,
  otherPoint
} instPointType_t;

class instPoint;

namespace Dyninst {
   namespace ParseAPI {
      class Block;
   };
};

#if defined(cap_instruction_api)
#include "Instruction.h"
#include "InstructionDecoder.h"
#endif
// As above: common class for both parse-time and run-time instPoints

class instPointBase {
 private:
    instPointBase() ;

 public:
  static unsigned int id_ctr;  
  instPointType_t getPointType() const { return ipType_; }
  void setPointType(instPointType_t type_) { ipType_ = type_; }

  instPointBase(instPointType_t type)
     : ipType_(type) {
       id_ = id_ctr++;
    } 
  instPointBase(instPointType_t type,
                unsigned int id) :
    id_(id),
    ipType_(type)
    {
    } 

  int id() const { return id_; }
  virtual ~instPointBase() { }
 protected:
  unsigned int id_;
  instPointType_t ipType_;
};

class image_instPoint : public instPointBase {
 private:
    image_instPoint();
 public:
    // Entry/exit
    image_instPoint(Address offset,
                    ParseAPI::Block *,
                    image * img,
                    instPointType_t type);
    // Call site or otherPoint that has a target
    image_instPoint(Address offset,
                    ParseAPI::Block *,
                    image * img,
                    Address callTarget,
                    bool isDynamic,
                    bool isAbsolute,
                    instPointType_t type, 
                    bool isUnresolved=false);

  Address offset_;
  Address offset() const { return offset_; }
  ParseAPI::Block *block() const { return block_; }
  void setBlock(ParseAPI::Block *b) { block_ = b; }
  // For call-site points:

private:
  ParseAPI::Block *block_;
  image * image_;
  mutable image_func *callee_; // cache
  std::string callee_name_;
  Address callTarget_;
  bool targetIsAbsolute_;
  bool isUnres_;
  bool isDynamic_; 
public:
  Address callTarget() const { return callTarget_; }
  bool targetIsAbsolute() const { return targetIsAbsolute_; }
  bool isDynamic() const { return isDynamic_; }
  bool isUnresolved() const { return isUnres_; }
  void setUnresolved(bool isUnres) { isUnres_ = isUnres; }
  image_func *getCallee() const;
  void setCallee(image_func *f) { callee_ = f; }
  std::string getCalleeName() const { return callee_name_; }
  void setCalleeName(std::string s) { callee_name_ = s; }
  // merge any otherP information that was unset in this point, we need 
  // to do this since there can only be one point at a given address and
  // we create entryPoints and exit points without initializing less information
  void mergePoint(image_instPoint *otherP);

#if defined (cap_use_pdvector)
  static int compare(image_instPoint *&ip1,
                     image_instPoint *&ip2) {
      if (ip1->offset() < ip2->offset())
          return -1;
      if (ip2->offset() < ip1->offset())
          return 1;
#if 0
      assert(ip1 == ip2);
#endif
      if (ip1 != ip2) {
         fprintf(stderr, "%s[%d]:  WARNING:  duplicate instPoints?? [%p %p] [%p %p]\n", FILE__, __LINE__, ip1, ip2, (void *)ip1->offset(), (void *)ip2->offset());
      }
      return 0;
  }
#else
  static bool compare(image_instPoint *ip1,
        image_instPoint *ip2) {
#if 0
     if (ip1->offset() == ip2->offset())
        fprintf(stderr, "%s[%d]:  WARNING:  strict weak ordring may not suffice here!\n", FILE__, __LINE__);
#endif
     if (ip1->offset() < ip2->offset())
        return true;
     return false;
#if 0
     if (ip2->offset() < ip1->offset())
        return true;
#if 0
     assert(ip1 == ip2);
#endif
     if (ip1 != ip2) {
        fprintf(stderr, "%s[%d]:  WARNING:  duplicate instPoints?? [%p %p] [%p %p]\n", FILE__, __LINE__, ip1, ip2, (void *)ip1->offset(), (void *)ip2->offset());
     }
     return false;
#endif
  }

#endif
};

class instPoint : public instPointBase {
    friend class instPointInstance;
    friend class baseTramp;
    friend class int_block;
    friend class int_function;
    friend void initRegisters();
    friend class registerSpace; // Liveness
 public:
  typedef enum { 
      tryRelocation,
      generateSucceeded,
      generateFailed,
      installSucceeded,
      installFailed,
      wasntGenerated,
      linkSucceeded,
      linkFailed,
      wasntInstalled} result_t;

 private:
    // Generic instPoint...
    instPoint(AddressSpace *proc,
              Address addr,
              int_block *block);

    instPoint(AddressSpace *proc,
              image_instPoint *img_p,
              Address addr,
              int_block *block);

    // Fork instPoint
    instPoint(instPoint *parP,
              int_block *childB,
              process *childP);

    // A lot of arbitrary/parse creation work can be shared
    static bool commonIPCreation(instPoint *newIP);

    // On windows, an internally-defined class cannot access private
    // members of the "parent" class. This works on all other
    // platforms. Windows is b0rken...

 public:
    bool hasNewInstrumentation() { return hasNewInstrumentation_; }
    bool hasAnyInstrumentation() { return hasAnyInstrumentation_; }

  // Make a new instPoint at an arbitrary location
  static instPoint *createArbitraryInstPoint(Address addr,
                                             AddressSpace *proc,
                                             int_function *func);

  static instPoint *createParsePoint(int_function *func,
                                     image_instPoint *img_p);

  static instPoint *createForkedPoint(instPoint *p, int_block *child, process *childP);

  static int liveRegSize();

  ~instPoint();

  // Get the appropriate first (or last) miniTramp for the given callWhen
  miniTramp *getFirstMini(callWhen when);
  miniTramp *getLastMini(callWhen when);

  // For call-site inst points: TODO: what about relocation? This
  // keeps pointing at the old version? Or should we get rid of the
  // instPoint dodge to get the callee and store the information in
  // the int_function directly?
  int_function *callee_;
  bool isDynamic_;
  bool isDynamic() const { return isDynamic_; }
  bool isReturnInstruction();

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
  std::string getCalleeName();

  // Is this the instPoint referred to?
  bool match(Address addr) const;

  AddressSpace *proc() const { return proc_; }
  
  int_block *block() const;
  int_function *func() const;

  Address addr() const { return addr_; }

#if defined(cap_instruction_api)
  InstructionAPI::Instruction::Ptr insn() const;
#endif

  Address callTarget() const;

  // returns the saved targets for the point, returning
  // false if there are none.  
  bool getSavedTargets(std::vector<Address> &targs);
  // returns false if it was already resolved
  bool setResolved();
  // needed for blocks that are split after the initial parse
  void setBlock( int_block* newBlock );




  // We use a three-phase instrumentation structure:

  // 1) Insert multiple pieces of instrumentation at multiple
  // 1) locations;

  // 2) Generate code for everything inserted in step 1 as well as all
  // pre-existing instrumentation that has been modified by step 1;

  // 3) Insert jumps to the code generated in step 2, and relocate
  // functions if necessary.

  // This function does it all:
  // Doesn't handle deferring though
  miniTramp *instrument(AstNodePtr ast,
                        callWhen when,
                        callOrder order,
                        bool trampRecursive,
                        bool noCost);

  // Step 1:
  miniTramp *addInst(AstNodePtr ast,
                     callWhen when,
                     callOrder order,
                     bool trampRecursive,
                     bool noCost);

  // Step 1.5 (alternate)
  // Instead of adding new instrumentation, replace the instruction at the current
  // point with the provided AST.
  bool replaceCode(AstNodePtr ast);

  // Step 2:
  result_t generateInst();
  // And 3:
  // We split this so that we can relocate between generation and installing
  result_t installInst();


  // Determine whether instrumentation will go in smoothly
  // At this point, "given the stacks, can we insert a jump
  // or not"?
  bool checkInst(pdvector<Address> &checkPCs);

  // Step 4:
  // TODO: if we're out-of-lining miniTramps or something, this should
  // be the call that causes linkage of the OOL MT to occur, just for
  // completeness of the model.
  result_t linkInst(bool update_trap_table = true);

  // Catchup: 1) does a PC correspond to the area covered by an
  // instPoint (aka multiTramp). 2) is the PC "before" or "after"
  // the provided new instrumentation?
  // Mmm catchup result: not in instrumentation, pre, or post.
  typedef enum { noMatch_c, notMissed_c, missed_c } catchup_result_t;

  // Explicitly does not require a frame; while that would be nice, 
  // we're eventually going to be called by Dyninst, and the BPatch_frame
  // class doesn't have a Frame member (although it's arguable that it should).
  catchup_result_t catchupRequired(Address pc,
                                   miniTramp *mt,
                                   bool active);

  bool createMiniTramp(AstNode &ast,
		       bool noCost,
		       int &trampCost,
		       Address &trampSize,
		       Address &retOffset);

  bool optimizeBaseTramps(callWhen when);

  void updateCost(miniTramp *mt);

  int getPointCost();


  baseTramp *preBaseTramp() const { return preBaseTramp_; }
  baseTramp *postBaseTramp() const { return postBaseTramp_; }
  baseTramp *targetBaseTramp() const { return targetBaseTramp_; }

  AstNodePtr replacedCode() const { return replacedCode_; }
  
  image_instPoint *imgPt() const { return img_p_; }

 private:
  baseTramp *preBaseTramp_;
  baseTramp *postBaseTramp_;
  baseTramp *targetBaseTramp_;
  AstNodePtr replacedCode_;

  AddressSpace *proc_;

  image_instPoint *img_p_;

  // The block we're attached to.
  int_block *block_;
  Address addr_;

 public:
  // RegisterSpace-only methods; all data in here is
  // specific to a registerSpace slot. 
  // 
  bitArray liveRegisters(callWhen when);

 private:

  // From post liveness we can work out pre liveness. Nifty,
  // huh? Almost as if it was a _backwards flow_ algorithm...
  bitArray postLiveRegisters_;

  // Calculate the liveness values for this
  // particular (inst) point.
  // A note: is this _pre_ or _post_? 
  // I will assume that all liveness values are
  // _input_, and so post-instruction instrumentation
  // will need an additional piece of work.
  void calcLiveness();

  bool hasNewInstrumentation_;
  bool hasAnyInstrumentation_;

};

#endif /* _INST_POINT_H_ */
