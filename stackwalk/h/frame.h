/*
 * Copyright (c) 1996-2011 Barton P. Miller
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

#ifndef FRAME_H_
#define FRAME_H_

#include "basetypes.h"
#include "Annotatable.h"
#include <string>
#include <set>

class StackCallback;

namespace Dyninst {
namespace Stackwalker {

class Walker;
class FrameStepper;

class Frame : public AnnotatableDense {
  friend class Walker;
  friend class CallTree;
  friend class ::StackCallback;
protected:
  Dyninst::MachRegisterVal ra;
  Dyninst::MachRegisterVal fp;
  Dyninst::MachRegisterVal sp;
	
  location_t ra_loc;
  location_t fp_loc;
  location_t sp_loc;
  
  mutable std::string sym_name;
  mutable void *sym_value;
  mutable enum { nv_unset, nv_set, nv_err } name_val_set;
  
  bool top_frame;
  bool bottom_frame;
  bool frame_complete;
  
  const Frame *prev_frame;
  FrameStepper *stepper;
  FrameStepper *next_stepper;
  Walker *walker;
  THR_ID originating_thread;
  
  void setStepper(FrameStepper *newstep);
  void setWalker(Walker *newwalk);
  void markTopFrame();
  void markBottomFrame();
  
  void setNameValue() const;
  
 public:
  Frame();
  Frame(Walker *walker);
  static Frame *newFrame(Dyninst::MachRegisterVal ra, Dyninst::MachRegisterVal sp, Dyninst::MachRegisterVal fp, Walker *walker);

  bool operator==(const Frame &F) const;

  Dyninst::MachRegisterVal getRA() const;
  Dyninst::MachRegisterVal getSP() const;
  Dyninst::MachRegisterVal getFP() const;
  
  void setRA(Dyninst::MachRegisterVal);
  void setSP(Dyninst::MachRegisterVal);
  void setFP(Dyninst::MachRegisterVal);
  void setThread(THR_ID);
  
  location_t getRALocation() const;
  location_t getSPLocation() const;
  location_t getFPLocation() const;
  
  void setRALocation(location_t newval);
  void setSPLocation(location_t newval);
  void setFPLocation(location_t newval);
  
  bool getName(std::string &str) const;
  bool getObject(void* &obj) const;
  bool getLibOffset(std::string &lib, Dyninst::Offset &offset, void* &symtab) const;
  
  bool isTopFrame() const;
  bool isBottomFrame() const;
  bool isFrameComplete() const;
  
  const Frame *getPrevFrame() const;
  FrameStepper *getStepper() const;
  FrameStepper *getNextStepper() const;
  Walker *getWalker() const;
  THR_ID getThread() const;

  ~Frame();
};

//Default FrameComparators, if none provided
typedef bool (*frame_cmp_t)(const Frame &a, const Frame &b); //Return true if a < b, by some comparison
bool frame_addr_cmp(const Frame &a, const Frame &b); //Default
bool frame_lib_offset_cmp(const Frame &a, const Frame &b);
bool frame_symname_cmp(const Frame &a, const Frame &b);
bool frame_lineno_cmp(const Frame &a, const Frame &b);

class FrameNode;
struct frame_cmp_wrapper {
   frame_cmp_t f;
   bool operator()(const FrameNode *a, const FrameNode *b);
};
typedef std::set<FrameNode *, frame_cmp_wrapper> frame_set_t;

class FrameNode {
   friend class CallTree;
   friend class WalkerSet;
   friend struct frame_cmp_wrapper;
  private:

   frame_set_t children;
   FrameNode *parent;
   enum {
      FTFrame,
      FTThread,
      FTHead
   } frame_type;
   Frame frame;
   THR_ID thrd;
   Walker *walker;
   bool had_error;

   FrameNode(frame_cmp_wrapper f);
  public:
   ~FrameNode();

   bool isFrame() const { return frame_type == FTFrame; }
   bool isThread() const { return frame_type == FTThread; }
   bool isHead() const { return frame_type == FTHead; }

   const Frame *getFrame() const { return (frame_type == FTFrame) ? &frame : NULL; }
   Frame *getFrame() { return (frame_type == FTFrame) ? &frame : NULL; }
   THR_ID getThread() const { return (frame_type == FTThread) ? thrd : NULL_LWP; }
   bool hadError() const { return (frame_type == FTThread) ? had_error : false; }

   const frame_set_t &getChildren() const { return children; }
   frame_set_t &getChildren() { return children; }

   const FrameNode *getParent() const { return parent; }
   FrameNode *getParent() { return parent; }

   Walker *getWalker() { return walker; }
};

class CallTree {
   friend class WalkerSet;
  public:

   CallTree(frame_cmp_t cmpf = frame_addr_cmp);
   ~CallTree();

   FrameNode *getHead() const { return head; }

   FrameNode *addFrame(const Frame &f, FrameNode *parent);
   FrameNode *addThread(THR_ID thrd, FrameNode *parent, Walker *walker, bool err_stack);

   void addCallStack(const std::vector<Frame> &stk, THR_ID thrd, Walker *walker, bool err_stack);
  private:
   FrameNode *head;
   frame_cmp_wrapper cmp_wrapper;
};

}
}

#endif
