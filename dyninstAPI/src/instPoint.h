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

class int_basicBlock;
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

  // Single instruction we're instrumenting (if at all)
#if defined(cap_instruction_api)
  static void setArch(Dyninst::Architecture a, bool) 
  {
    arch = a;
  }
  static Dyninst::Architecture arch;
  
    Dyninst::InstructionAPI::Instruction::Ptr insn() {
      Dyninst::InstructionAPI::InstructionDecoder dec((const unsigned char*)NULL, 0, arch);
      return dec.decode(insn_);
    }
#else
    static void setArch(Dyninst::Architecture, bool) {}
    const instruction &insn()  { 
        // XXX super-evil cast due to incoherant "codeBuf_t" typedefs
        if(dec_insn_.size() == 0)
          dec_insn_.setInstruction((NS_sparc::codeBuf_t*)insn_,0);
        return dec_insn_; 
    }
#endif
  instPointBase(
    unsigned char * insn_buf,
    size_t insn_len,
    instPointType_t type) :
    ipType_(type)
    { id_ = id_ctr++;
      insn_ = (unsigned char*)(malloc(insn_len));
      memcpy(insn_, insn_buf, insn_len);
    }
  // We need to have a manually-set-everything method
  instPointBase(
          unsigned char * insn_buf,
          size_t insn_len,
          instPointType_t type,
                unsigned int id) :
      id_(id),
      ipType_(type)
      {
        insn_ = (unsigned char*)(malloc(insn_len));
        memcpy(insn_, insn_buf, insn_len);
      }
  // convenience for creations from instPoint
  instPointBase(
#if defined(cap_instruction_api)
    Dyninst::InstructionAPI::Instruction::Ptr insn,
#else
    instruction insn,
#endif
    instPointType_t type) :
    ipType_(type)
    {
        id_ = id_ctr++;
#if defined(cap_instruction_api)
        insn_ = (unsigned char*)malloc(insn->size());
        memcpy(insn_,insn->ptr(),insn->size());
#else
        insn_ = (unsigned char*)malloc(insn.size());
        memcpy(insn_,insn.ptr(),insn.size());
#endif
    } 
  instPointBase(
#if defined(cap_instruction_api)
    Dyninst::InstructionAPI::Instruction::Ptr insn,
#else
    instruction insn,
#endif
    instPointType_t type,
    unsigned int id) :
    id_(id),
    ipType_(type)
    {
        id_ = id_ctr++;
#if defined(cap_instruction_api)
        insn_ = (unsigned char*)malloc(insn->size());
        memcpy(insn_,insn->ptr(),insn->size());
#else
        insn_ = (unsigned char*)malloc(insn.size());
        memcpy(insn_,insn.ptr(),insn.size());
#endif
    } 

  int id() const { return id_; }
    virtual ~instPointBase() {
        if(insn_)
            free(insn_);
    }
 protected:
  unsigned int id_;
  instPointType_t ipType_;
  unsigned char* insn_;
#if !defined(cap_instruction_api)
  instruction dec_insn_;
#endif
};

class image_instPoint : public instPointBase {
 private:
    image_instPoint();
 public:
    // Entry/exit
    image_instPoint(Address offset,
                    unsigned char * insn_buf,
                    size_t insn_len,
                    image * img,
                    instPointType_t type, 
                    bool isUnresolved=false);
    // Call site or otherPoint that has a target
    image_instPoint(Address offset,
                    unsigned char * insn_buf,
                    size_t insn_len,
                    image * img,
                    Address callTarget,
                    bool isDynamic,
                    bool isAbsolute,
                    instPointType_t type, 
                    bool isUnresolved=false);

  Address offset_;
  Address offset() const { return offset_; }

  // For call-site points:
private:
  image * image_;
  mutable image_func *callee_; // cache
  std::string callee_name_;
  Address callTarget_;
  bool targetIsAbsolute_;
  bool isUnres_;
public:
  Address callTarget() const { return callTarget_; }
  bool targetIsAbsolute() const { return targetIsAbsolute_; }
  bool isDynamic_; 
  bool isDynamic() const { return isDynamic_; }
  bool isUnresolved() const { return isUnres_; }
  void setResolved() { isUnres_ = false; }
  image_func *getCallee() const;
  void setCallee(image_func *f) { callee_ = f; }
  std::string getCalleeName() const { return callee_name_; }
  void setCalleeName(std::string s) { callee_name_ = s; }

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

// The actual instPoint is a little more interesting. It wraps the
// abstract view of an instrumentation point. In reality, it may map
// to multiple addresses (if the function it targets was relocated),
// and so may use multiple multiTramps. So we have a 1:many
// mapping. This is taken care of by keeping a vector of iPTarget
// structures.

class instPointInstance {

    friend class instPoint;
    friend class multiTramp;
    
 public:
    typedef enum {
        noMultiTramp,
        instOfSharedBlock,
        mTrampTooBig,
        pointPreviouslyModified,
        generateFailed,
        generateSucceeded,
        installFailed,
        installSucceeded,
        linkFailed,
        linkSucceeded
    } result_t;

 private:
    instPointInstance() { assert(0); }
    instPointInstance(Address a, bblInstance *b, instPoint *ip) :
        addr_(a), block_(b), multiID_(0), point(ip), disabled(false) {
    }

    // No end of trouble if this is a subclass..
    Address addr_; // Address of this particular version
    bblInstance *block_; // And the block instance we're attached to.

    unsigned multiID_;

    instPoint *point; // Backchain pointer

    result_t generateInst();
    result_t installInst();
    result_t linkInst();

    bool disabled;

 public:
    // If we re-generate code we toss the old
    // multiTramp and make a new one; we then need
    // to update everyone with a handle to the multi.

    AddressSpace *proc() const;
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
    friend class int_basicBlock;
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
#if defined(cap_instruction_api)
    Dyninst::InstructionAPI::Instruction::Ptr insn,
#else
                                 instruction insn,
#endif
              Address addr,
              int_basicBlock *block);

    instPoint(AddressSpace *proc,
              image_instPoint *img_p,
              Address addr,
              int_basicBlock *block);

    // Fork instPoint
    instPoint(instPoint *parP,
              int_basicBlock *childB,
              process *childP);

    // A lot of arbitrary/parse creation work can be shared
    static bool commonIPCreation(instPoint *newIP);

    // On windows, an internally-defined class cannot access private
    // members of the "parent" class. This works on all other
    // platforms. Windows is b0rken...

 public:
    bool updateInstances();
    // We sometimes need to split the work that updateInstances does.
    // We can have a large number of points per block; if we 
    // call updateInstances, we'll generate the multiTramp an excessive
    // number of times. 
    // This is a workaround; we may want to revisit the entire structure
    // later.
    // Do no code generation; unsafe to call individually
    bool updateInstancesBatch();
    bool updateInstancesFinalize();

    pdvector<instPointInstance *> instances;

    // Adding instances is expensive; if the function
    // hasn't changed, we can avoid doing so.
    int funcVersion;

    bool hasNewInstrumentation() { return hasNewInstrumentation_; }
    bool hasAnyInstrumentation() { return hasAnyInstrumentation_; }

  // Make a new instPoint at an arbitrary location
  static instPoint *createArbitraryInstPoint(Address addr,
                                             AddressSpace *proc,
                                             int_function *func);

  static instPoint *createParsePoint(int_function *func,
                                     image_instPoint *img_p);

  static instPoint *createForkedPoint(instPoint *p, int_basicBlock *child, process *childP);

  static int liveRegSize();

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
  
  int_basicBlock *block() const;
  int_function *func() const;

  Address addr() const { return addr_; }

  Address callTarget() const;

  // the saved target for the point, kept in sync with other
  // points at this address if there's function sharing
  Address getSavedTarget();
  void setSavedTarget(Address st_);
  // returns false if it was already resolved
  bool setResolved();
  void removeMultiTramps();
  // needed for blocks that are split after the initial parse
  void setBlock( int_basicBlock* newBlock );




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
  int_basicBlock *block_;
  Address addr_;

  // We used to decide whether to generate/install/link new instances
  // based on whether the first instance was G/I/L, as appropriate. However,
  // function relocation can remove that first multiTramp, leaving us
  // with no information. Instead, we keep it here.
  bool shouldGenerateNewInstances_;
  bool shouldInstallNewInstances_;
  bool shouldLinkNewInstances_;

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

  Address savedTarget_;

};

typedef instPoint::iterator instPointIter;

#endif /* _INST_POINT_H_ */
