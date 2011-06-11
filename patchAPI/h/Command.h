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

    PATCHAPI_EXPORT void add(CommandPtr);
    PATCHAPI_EXPORT void remove(CommandList::iterator);

  protected:
    CommandList to_do_;
    CommandList done_;

};

class Patcher : public BatchCommand {
  public:
    PATCHAPI_EXPORT PatcherPtr create();
    Patcher() {}
    virtual ~Patcher() {}

    PATCHAPI_EXPORT virtual bool run();
};

class PushFrontCommand : public Command {
  public:
    PushFrontCommand() {}
    virtual ~PushFrontCommand() {}

    PATCHAPI_EXPORT virtual bool run();
    PATCHAPI_EXPORT virtual bool undo();
};

class PushBackCommand : public Command {
  public:
    PushBackCommand() {}
    virtual ~PushBackCommand() {}

    PATCHAPI_EXPORT virtual bool run();
    PATCHAPI_EXPORT virtual bool undo();
};

class RemoveSnippetCommand : public Command {
  public:
    RemoveSnippetCommand() {}
    virtual ~RemoveSnippetCommand() {}

    PATCHAPI_EXPORT virtual bool run();
    PATCHAPI_EXPORT virtual bool undo();
};

class RemoveCallCommand : public Command {
  public:
    RemoveCallCommand() {}
    virtual ~RemoveCallCommand() {}

    PATCHAPI_EXPORT virtual bool run();
    PATCHAPI_EXPORT virtual bool undo();
};

class ReplaceCallCommand : public Command {
  public:
    ReplaceCallCommand() {}
    virtual ~ReplaceCallCommand() {}

    PATCHAPI_EXPORT virtual bool run();
    PATCHAPI_EXPORT virtual bool undo();
};

class ReplaceFuncCommand : public Command {
  public:
    ReplaceFuncCommand() {}
    virtual ~ReplaceFuncCommand() {}

    PATCHAPI_EXPORT virtual bool run();
    PATCHAPI_EXPORT virtual bool undo();
};

}
}

#endif /* PATCHAPI_COMMAND_H_ */
