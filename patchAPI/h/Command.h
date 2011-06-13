/* Plugin / Public Interface */

#ifndef PATCHAPI_COMMAND_H_
#define PATCHAPI_COMMAND_H_

#include "common.h"

namespace Dyninst {
namespace PatchAPI {

/* Interface to support transactional semantics, by implementing an
   instrumentation request (public interface) or an internal step of
   instrumentation (plugin interface) */

class Command {
  public:
    Command() {}
    virtual ~Command() {}

    PATCHAPI_EXPORT virtual bool commit();

    PATCHAPI_EXPORT virtual bool run() = 0;
    PATCHAPI_EXPORT virtual bool undo() = 0;
};

class BatchCommand : public Command {
  public:
    PATCHAPI_EXPORT BatchCommandPtr create();
    BatchCommand() {}
    virtual ~BatchCommand() {}

    PATCHAPI_EXPORT virtual bool run();
    PATCHAPI_EXPORT virtual bool undo();

    /* Add/Remove Commands to to_do_ list. */
    PATCHAPI_EXPORT void add(CommandPtr);
    PATCHAPI_EXPORT void remove(CommandList::iterator);

  protected:
    CommandList to_do_;
    CommandList done_;

};

class Patcher : public BatchCommand {
  public:
    typedef dyn_detail::boost::shared_ptr<Patcher> Ptr;
    PATCHAPI_EXPORT PatcherPtr create(Dyninst::PatchAPI::PatchMgrPtr mgr) {
      return Ptr(new Patcher(mgr));
    }
    Patcher(Dyninst::PatchAPI::PatchMgrPtr mgr) : mgr_(mgr) {}
    virtual ~Patcher() {}

    PATCHAPI_EXPORT virtual bool run();
  private:
    Dyninst::PatchAPI::PatchMgrPtr mgr_;
};

class PushFrontCommand : public Command {
  public:
    typedef dyn_detail::boost::shared_ptr<PushFrontCommand> Ptr;
    static Ptr create(Dyninst::PatchAPI::Point* pt,
                      Dyninst::PatchAPI::SnippetPtr snip) {
      return Ptr(new PushFrontCommand(pt, snip));
    }
    PushFrontCommand(Dyninst::PatchAPI::Point* pt,
                     Dyninst::PatchAPI::SnippetPtr snip) : pt_(pt), snip_(snip) {}
    virtual ~PushFrontCommand() {}

    PATCHAPI_EXPORT virtual bool run();
    PATCHAPI_EXPORT virtual bool undo();
 private:
   Dyninst::PatchAPI::Point* pt_;
   Dyninst::PatchAPI::SnippetPtr snip_;
   Dyninst::PatchAPI::InstancePtr instance_;
};

class PushBackCommand : public Command {
  public:
    typedef dyn_detail::boost::shared_ptr<PushBackCommand> Ptr;
    static Ptr create(Dyninst::PatchAPI::Point* pt,
                      Dyninst::PatchAPI::SnippetPtr snip) {
      return Ptr(new PushBackCommand(pt, snip));
    }
    PushBackCommand(Dyninst::PatchAPI::Point* pt,
                    Dyninst::PatchAPI::SnippetPtr snip)
                    : pt_(pt), snip_(snip) {}
    virtual ~PushBackCommand() {}

    PATCHAPI_EXPORT virtual bool run();
    PATCHAPI_EXPORT virtual bool undo();
  private:
    Dyninst::PatchAPI::Point* pt_;
    Dyninst::PatchAPI::SnippetPtr snip_;
    Dyninst::PatchAPI::InstancePtr instance_;
};

class RemoveSnippetCommand : public Command {
  public:
    typedef dyn_detail::boost::shared_ptr<RemoveSnippetCommand> Ptr;
    static Ptr create(Dyninst::PatchAPI::InstancePtr instance) {
      return Ptr(new RemoveSnippetCommand(instance));
    }
    RemoveSnippetCommand(Dyninst::PatchAPI::InstancePtr instance)
      : instance_(instance) {}
    virtual ~RemoveSnippetCommand() {}

    PATCHAPI_EXPORT virtual bool run();
    PATCHAPI_EXPORT virtual bool undo();
  private:
    Dyninst::PatchAPI::InstancePtr instance_;
};

class RemoveCallCommand : public Command {
  public:
    typedef dyn_detail::boost::shared_ptr<RemoveCallCommand> Ptr;
    static Ptr create(Dyninst::PatchAPI::PatchMgrPtr mgr,
                      Dyninst::PatchAPI::PatchBlock* call_block,
                      Dyninst::PatchAPI::PatchFunction* context = NULL) {
      return Ptr(new RemoveCallCommand(mgr, call_block, context));
    }
    RemoveCallCommand(Dyninst::PatchAPI::PatchMgrPtr mgr,
                      Dyninst::PatchAPI::PatchBlock* call_block,
                      Dyninst::PatchAPI::PatchFunction* context)
      : mgr_(mgr), call_block_(call_block), context_(context) {}
    virtual ~RemoveCallCommand() {}

    PATCHAPI_EXPORT virtual bool run();
    PATCHAPI_EXPORT virtual bool undo();
  private:
    Dyninst::PatchAPI::PatchMgrPtr mgr_;
    Dyninst::PatchAPI::PatchBlock* call_block_;
    Dyninst::PatchAPI::PatchFunction* context_;
};

class ReplaceCallCommand : public Command {
  public:
    typedef dyn_detail::boost::shared_ptr<ReplaceCallCommand> Ptr;
    static Ptr create(Dyninst::PatchAPI::PatchBlock* call_block,
                      Dyninst::PatchAPI::PatchFunction* new_callee,
                      Dyninst::PatchAPI::PatchFunction* context) {
      return Ptr(new ReplaceCallCommand(call_block, new_callee, context));
    }
    ReplaceCallCommand(Dyninst::PatchAPI::PatchBlock* call_block,
                       Dyninst::PatchAPI::PatchFunction* new_callee,
                       Dyninst::PatchAPI::PatchFunction* context)
      : call_block_(call_block), new_callee_(new_callee), context_(context) {}
    virtual ~ReplaceCallCommand() {}

    PATCHAPI_EXPORT virtual bool run();
    PATCHAPI_EXPORT virtual bool undo();
  private:
    Dyninst::PatchAPI::PatchMgrPtr mgr_;
    Dyninst::PatchAPI::PatchBlock* call_block_;
    Dyninst::PatchAPI::PatchFunction* new_callee_;
    Dyninst::PatchAPI::PatchFunction* context_;
};

class ReplaceFuncCommand : public Command {
  public:
    typedef dyn_detail::boost::shared_ptr<ReplaceFuncCommand> Ptr;
    static Ptr create(Dyninst::PatchAPI::PatchFunction* old_func,
                      Dyninst::PatchAPI::PatchFunction* new_func) {
      return Ptr(new ReplaceFuncCommand(old_func, new_func));
    }
    ReplaceFuncCommand(Dyninst::PatchAPI::PatchFunction* old_func,
                       Dyninst::PatchAPI::PatchFunction* new_func)
      : old_func_(old_func), new_func_(new_func)  {}
    virtual ~ReplaceFuncCommand() {}

    PATCHAPI_EXPORT virtual bool run();
    PATCHAPI_EXPORT virtual bool undo();
  private:
    Dyninst::PatchAPI::PatchMgrPtr mgr_;
    Dyninst::PatchAPI::PatchFunction* old_func_;
    Dyninst::PatchAPI::PatchFunction* new_func_;
};

}
}

#endif /* PATCHAPI_COMMAND_H_ */
