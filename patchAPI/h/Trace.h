/* Utility */

#ifndef PATCHAPI_H_DYNINST_WIDGET_TRACE_H_
#define PATCHAPI_H_DYNINST_WIDGET_TRACE_H_

#include "common.h"
#include "Widget.h"
#include "PatchCFG.h"

namespace Dyninst {
namespace PatchAPI {

/* By default, a trace contains only one instruction for code
   relocation. Users can extend Trace to support block or function relocation.
   -- May 16, 2011. wenbin */
class Trace {
  public:

    // Needs to track the definition in BufferMgr.h
    typedef int Label;
    static int TraceID;
    typedef std::list<Widget::Ptr> WidgetList;
    typedef dyn_detail::boost::shared_ptr<Trace> Ptr;

    // Creation via a single Widget
    static Ptr create(Widget::Ptr atom, Address a, PatchFunction *f);

    bool generate(BufferMgr &buffer);
    Address origAddr() const { return origAddr_; }
    void setAddr(Address addr) { curAddr_ = addr; }
    int id() const { return id_; }

    // Non-const for use by transformer classes
    WidgetList &elements() { return elements_; }
    std::string format() const;
    PatchBlock *bbl() const { return bbl_; }

    // Unlike basic blocks, _all_ traces must be
    // created in the context of a function so we can correctly
    // report which function we're from.
    PatchFunction *func() const { return func_; }

    bool extractTrackers(Tracker &);
    Label getLabel() {
      assert(label_ != -1);
      return label_;
    }

  protected:
    explicit Trace(PatchBlock *bbl)
      : curAddr_(0),
      origAddr_(bbl->start()),
      bbl_(bbl),
      id_(TraceID++),
      label_(-1),
      func_(bbl->function()), obj_(bbl->object()) {}

    Trace(Widget::Ptr a, Address origAddr, PatchFunction *f)
      : curAddr_(0),
      origAddr_(origAddr),
      bbl_(NULL),
      id_(TraceID++),
      label_(-1),
      func_(f), obj_(f->object()) {
      elements_.push_back(a);
    }


    typedef std::pair<InstructionAPI::Instruction::Ptr, Address> InsnInstance;
    typedef std::vector<InsnInstance> InsnVec;

    // Analyze the block ender and create a logical control flow
    // construct matching it.
    bool createTraceEnd();
    WidgetList elements_;
    Address curAddr_;
    Address origAddr_;
    PatchBlock *bbl_;
    typedef std::list<Patch *> Patches;
    Patches patches_;
    int id_;
    Label label_;
    PatchFunction *func_;
    ObjectPtr obj_;
};

};
};
#endif  // PATCHAPI_H_DYNINST_WIDGET_TRACE_H_
