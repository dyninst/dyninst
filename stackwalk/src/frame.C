/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
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

#include "stackwalk/h/frame.h"
#include "stackwalk/h/walker.h"
#include "stackwalk/h/swk_errors.h"
#include "stackwalk/h/symlookup.h"
#include "stackwalk/h/procstate.h"

#include "stackwalk/src/symtab-swk.h"

#include <assert.h>
#include <string>

using namespace std;
using namespace Dyninst;
using namespace Dyninst::Stackwalker;

Frame::Frame() :
  ra(0x0),
  fp(0x0),
  sp(0x0),
  sym_value(NULL),
  name_val_set(nv_unset),
  top_frame(false),
  bottom_frame(false),
  frame_complete(false),
  non_call_frame(false),
  prev_frame(NULL),
  stepper(NULL),
  next_stepper(NULL),
  walker(NULL),
  originating_thread(NULL_THR_ID)
{
  ra_loc.location = loc_unknown;
  ra_loc.val.addr = 0x0;
  fp_loc.location = loc_unknown;
  fp_loc.val.addr = 0x0;
  sp_loc.location = loc_unknown;
  sp_loc.val.addr = 0x0;
  
  sw_printf("[%s:%d] - Created null frame at %p\n", FILE__, __LINE__, (void*)this);
}

Frame::Frame(Walker *parent_walker) :
  ra(0x0),
  fp(0x0),
  sp(0x0),
  sym_value(NULL),
  name_val_set(nv_unset),
  top_frame(false),
  bottom_frame(false),
  frame_complete(false),
  non_call_frame(false),
  prev_frame(NULL),
  stepper(NULL),
  next_stepper(NULL),
  walker(parent_walker),
  originating_thread(NULL_THR_ID)
{
  assert(walker);
  ra_loc.location = loc_unknown;
  ra_loc.val.addr = 0x0;
  fp_loc.location = loc_unknown;
  fp_loc.val.addr = 0x0;
  sp_loc.location = loc_unknown;
  sp_loc.val.addr = 0x0;
  
  sw_printf("[%s:%d] - Created frame at %p\n", FILE__, __LINE__, (void*)this);
}

Frame *Frame::newFrame(Dyninst::MachRegisterVal pc, Dyninst::MachRegisterVal sp, Dyninst::MachRegisterVal fp, Walker *walker) {
  sw_printf("[%s:%d] - Manually creating frame with %lx, %lx, %lx, %p\n",
	    FILE__, __LINE__, pc, sp, fp, (void*)walker);
  if (!walker) {
    sw_printf("[%s:%d] - Trying to create Frame with NULL Walker\n",
	      FILE__, __LINE__);
    setLastError(err_badparam, "Walker parameter cannot be NULL when creating frame");
  }
  
  Frame *newframe = new Frame(walker);
  
  newframe->setRA(pc);
  newframe->setSP(sp);
  newframe->setFP(fp);
  
  return newframe;
}

bool Frame::operator==(const Frame &F) const
{
  return ((ra == F.ra) &&
          (fp == F.fp) &&
          (sp == F.sp) &&
          (ra_loc == F.ra_loc) &&
          (fp_loc == F.fp_loc) &&
          (sp_loc == F.sp_loc) &&
          (sym_name == F.sym_name) &&
          (frame_complete == F.frame_complete) &&
          (stepper == F.stepper) &&
          (walker == F.walker) &&
          (originating_thread == F.originating_thread));
}

void Frame::setStepper(FrameStepper *newstep) {
  sw_printf("[%s:%d] - Setting frame %p's stepper to %p\n", 
	    FILE__, __LINE__, (void*)this, (void*)newstep);
  stepper = newstep;
}

void Frame::markTopFrame() {
  sw_printf("[%s:%d] - Marking frame %p as top\n",
	    FILE__, __LINE__, (void*)this);
  top_frame = true;
}

void Frame::markBottomFrame() {
  sw_printf("[%s:%d] - Marking frame %p as bottom\n", 
	    FILE__, __LINE__, (void*)this);
  bottom_frame = true;
}

Dyninst::MachRegisterVal Frame::getRA() const {
  return ra;
}

Dyninst::MachRegisterVal Frame::getSP() const {
  return sp;
}

Dyninst::MachRegisterVal Frame::getFP() const {
  return fp;
}

location_t Frame::getRALocation() const {
  return ra_loc;
}

location_t Frame::getSPLocation() const {
  return sp_loc;
}

location_t Frame::getFPLocation() const {
  return fp_loc;
}

void Frame::setRA(Dyninst::MachRegisterVal newval) {
  sw_printf("[%s:%d] - Setting ra of frame %p to %lx\n",
	    FILE__, __LINE__, (void*)this, newval);
  ra = newval;
  frame_complete = true;
}

void Frame::setFP(Dyninst::MachRegisterVal newval) {
  sw_printf("[%s:%d] - Setting fp of frame %p to %lx\n",
			  FILE__, __LINE__, (void*)this, newval);
  fp = newval;
}

void Frame::setSP(Dyninst::MachRegisterVal newval) {
  sw_printf("[%s:%d] - Setting sp of frame %p to %lx\n",
	    FILE__, __LINE__, (void*)this, newval);
  sp = newval;
}

static void debug_print_location(const char *s, Frame *f, location_t val) {
  if (val.location == loc_address)
    sw_printf("[%s:%d] - Setting frame %p %s location to address %lx\n",
              FILE__, __LINE__, (void*)f, s, val.val.addr);
  else if (val.location == loc_register)
    sw_printf("[%s:%d] - Setting frame %p %s location to register %s\n",
              FILE__, __LINE__, (void*)f, s, val.val.reg.name().c_str());
  else if (val.location == loc_unknown)
     sw_printf("[%s:%d] - Setting frame %p %s location to unknown\n",
               FILE__, __LINE__, (void*)f, s);
}

void Frame::setRALocation(location_t newval) {
  if (dyn_debug_stackwalk) {
    debug_print_location("RA", this, newval);
  }
  ra_loc = newval;
}

void Frame::setSPLocation(location_t newval) {
  if (dyn_debug_stackwalk) {
    debug_print_location("SP", this, newval);
  }
  sp_loc = newval;
}

void Frame::setFPLocation(location_t newval) {
  if (dyn_debug_stackwalk) {
    debug_print_location("FP", this, newval);
  }
  fp_loc = newval;
}

void Frame::setNameValue() const {
  if (name_val_set == nv_set || name_val_set == nv_err)
    return;
  
  if (!walker) {
    setLastError(err_nosymlookup, "No Walker object was associated with this frame");
    sw_printf("[%s:%d] - Error, No walker found.\n", FILE__, __LINE__);
    name_val_set = nv_err;
    return;
  }
  
  SymbolLookup *lookup = walker->getSymbolLookup();
  if (!lookup) {
    setLastError(err_nosymlookup, "No SymbolLookup object was associated with the Walker");
    sw_printf("[%s:%d] - Error, No symbol lookup found.\n", FILE__, __LINE__);
    name_val_set = nv_err;
    return;
  }
  // Here we lookup return address minus 1 to handle the following special case:
  //
  // Suppose A calls B and B is a non-returnning function, and the call to B in A
  // is the last instruction of A. Then the compiler may generate code where 
  // another function C is immediately after A. In such case, the return address
  // will be the entry address of C. And if we look up function by the return
  // address, we will get C rather than A. 
  bool result = lookup->lookupAtAddr(getRA() - 1, sym_name, sym_value);
  if (!result) {
    sw_printf("[%s:%d] - Error, returned by lookupAtAddr().\n", FILE__, __LINE__);
    name_val_set = nv_err;
  }
  
  sw_printf("[%s:%d] - Successfully looked up symbol for frame %p\n",
	    FILE__, __LINE__, (const void*)this);
  
  name_val_set = nv_set;
}

bool Frame::getName(std::string &str) const {
  setNameValue();
  if (name_val_set == nv_set) {
    str = sym_name;
    sw_printf("[%s:%d] - Frame::getName (frame %p) returning %s\n",
	      FILE__, __LINE__, (const void*)this, str.c_str());
    return true;
  }
  else {
    sw_printf("[%s:%d] - Frame::getName (frame %p) returning error\n",
	      FILE__, __LINE__, (const void*)this);
    return false;
  }
}

bool Frame::getObject(void* &obj) const {
  setNameValue();
  if (name_val_set == nv_set) {
    obj = sym_value;
    sw_printf("[%s:%d] - Frame::getObject (frame %p) returning %p\n",
	      FILE__, __LINE__, (const void*)this, obj);
    return true;
  }
  else {
    sw_printf("[%s:%d] - Frame::getObject (frame %p) returning error\n",
	      FILE__, __LINE__, (const void*)this);
    return false;
  }
}

bool Frame::isTopFrame() const {
  return top_frame;
}

bool Frame::isBottomFrame() const {
  return bottom_frame;
}

const Frame *Frame::getPrevFrame() const {
  return prev_frame;
}

FrameStepper *Frame::getStepper() const {
  return stepper;
}

FrameStepper *Frame::getNextStepper() const {
  return next_stepper;
}

Walker *Frame::getWalker() const {
  return walker;
}

bool Frame::isFrameComplete() const {
  return frame_complete;
}

Frame::~Frame() {
  sw_printf("[%s:%d] - Destroying frame %p\n", FILE__, __LINE__, (void*)this);
}

bool Frame::getLibOffset(std::string &lib, Dyninst::Offset &offset, void*& symtab) const
{
  LibraryState *libstate = getWalker()->getProcessState()->getLibraryTracker();
  if (!libstate) {
    sw_printf("[%s:%d] - getLibraryAtAddr, had no library tracker\n",
              FILE__, __LINE__);
    setLastError(err_unsupported, "No valid library tracker registered");
    return false;
  }

  LibAddrPair la;
  bool result = libstate->getLibraryAtAddr(getRA(), la);
  if (!result) {
    sw_printf("[%s:%d] - getLibraryAtAddr returned false for %lx\n",
              FILE__, __LINE__, getRA());
    return false;
  }

  lib = la.first;
  offset = getRA() - la.second;

#if defined(WITH_SYMTAB_API)
  symtab = static_cast<void *>(SymtabWrapper::getSymtab(lib));
#else
  symtab = NULL;
#endif

  return true;
}

THR_ID Frame::getThread() const
{
   return originating_thread;
}

void Frame::setThread(THR_ID t)
{
   originating_thread = t;
}

void Frame::setNonCall() 
{
   non_call_frame = true;
}

bool Frame::nonCall() const
{ 
   return non_call_frame;
}

FrameNode::FrameNode(frame_cmp_wrapper f) :
   children(f),
   parent(NULL),
   frame_type(),
   thrd(NULL_THR_ID),
   walker(NULL),
   had_error(false)
{
}

FrameNode::FrameNode(frame_cmp_wrapper f, string s) :
   children(f),
   parent(NULL),
   frame_type(FTString),
   thrd(NULL_THR_ID),
   walker(NULL),
   had_error(false),
   ftstring(s)
{
}

FrameNode::FrameNode(const FrameNode &fn) :
   children(fn.children.key_comp()),
   parent(NULL),
   frame_type(fn.frame_type),
   frame(fn.frame),
   thrd(fn.thrd),
   walker(fn.walker),
   had_error(fn.had_error),
   ftstring(fn.ftstring)
{
}

FrameNode::~FrameNode()
{
}

inline bool frame_cmp_wrapper::operator()(const FrameNode *a, const FrameNode *b) const {
   if (a->frame_type == FrameNode::FTThread && b->frame_type == FrameNode::FTThread) {
      Dyninst::PID a_pid = a->getWalker()->getProcessState()->getProcessId();
      Dyninst::PID b_pid = b->getWalker()->getProcessState()->getProcessId();
      if (a_pid != b_pid)
         return a_pid < b_pid;
      return a->thrd < b->thrd;
   }
   else if (a->frame_type == FrameNode::FTThread)
      return false;
   else if (b->frame_type == FrameNode::FTThread)
      return true;
   else if (a->frame_type == FrameNode::FTString && b->frame_type == FrameNode::FTString)
      return a->frameString() < b->frameString();
   else if (a->frame_type == FrameNode::FTString)
      return false;
   else if (b->frame_type == FrameNode::FTString)
      return true;
   else
      return f(a->frame, b->frame);
}

CallTree::CallTree(frame_cmp_t cmpf)
{
   cmp_wrapper.f = cmpf;
   head = new FrameNode(cmp_wrapper);
   head->frame_type = FrameNode::FTHead;
   head->parent = NULL;
}

frame_cmp_t CallTree::getComparator()
{
   return cmp_wrapper.f;
}

frame_cmp_wrapper CallTree::getCompareWrapper()
{
   return cmp_wrapper;
}

static void deleteTree(FrameNode *node) {
   frame_set_t &children = node->getChildren();
   for (frame_set_t::iterator i = children.begin(); i != children.end(); i++)
      deleteTree(*i);
   delete(node);
}

CallTree::~CallTree() 
{
   deleteTree(head);
   head = NULL;
}

FrameNode *CallTree::addFrame(const Frame &f, FrameNode *parent)
{
   FrameNode search_node(cmp_wrapper);
   search_node.frame_type = FrameNode::FTFrame;
   search_node.frame = f;

   pair<frame_set_t::iterator, frame_set_t::iterator> is = parent->children.equal_range(&search_node);
   bool found = (is.first != is.second);
   if (found) {
      //Common case, already have this node in tree, don't create a new one.
      FrameNode *n = *is.first;
      return n;
   }

   //Create and insert a new node at position i
   FrameNode *new_node = new FrameNode(cmp_wrapper);
   new_node->frame_type = FrameNode::FTFrame;
   new_node->frame = f;
   new_node->walker = f.getWalker();
   parent->children.insert(is.first, new_node);

   return new_node;
}

FrameNode *CallTree::addThread(THR_ID thrd, FrameNode *parent, Walker *walker, bool err_stack)
{
   FrameNode *new_node = new FrameNode(cmp_wrapper);
   assert(walker);
   new_node->frame_type = FrameNode::FTThread;
   new_node->thrd = thrd;
   new_node->walker = walker;
   new_node->had_error = err_stack;

   pair<frame_set_t::iterator, bool> i = parent->children.insert(new_node);
   if (!i.second) {
      //Element already existed.
      delete new_node;
      return *(i.first);
   }

   return new_node;
}

void CallTree::addCallStack(const vector<Frame> &stk, THR_ID thrd, Walker *walker, bool err_stack)
{
   FrameNode *cur = head;
   for (vector<Frame>::const_reverse_iterator i = stk.rbegin(); i != stk.rend(); i++) {
      cur = addFrame(*i, cur);
   }
   addThread(thrd, cur, walker, err_stack);
}
 
bool Dyninst::Stackwalker::frame_addr_cmp(const Frame &a, const Frame &b)
{
   return a.getRA() < b.getRA();
}

bool Dyninst::Stackwalker::frame_lib_offset_cmp(const Frame &a, const Frame &b)
{
   string a_lib, b_lib;
   Offset a_off = 0, b_off = 0;
   void *a_ignore, *b_ignore;
   a.getLibOffset(a_lib, a_off, a_ignore);
   b.getLibOffset(b_lib, b_off, b_ignore);
   int str_cmp = a_lib.compare(b_lib);
   if (str_cmp < 0)
      return true;
   else if (str_cmp > 0)
      return false;
   else
      return a_off < b_off;
}

bool Dyninst::Stackwalker::frame_symname_cmp(const Frame &a, const Frame &b)
{
   string a_name, b_name;
   a.getName(a_name);
   b.getName(b_name);
   return a_name < b_name;
}

bool Dyninst::Stackwalker::frame_lineno_cmp(const Frame &, const Frame &)
{
   assert(0 && "frame_lineno_cmp unimplemented");
	return false;
}
