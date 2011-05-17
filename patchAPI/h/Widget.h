/* Utility */

#ifndef DYNINST_PATCHAPI_WIDGET_H_
#define DYNINST_PATCHAPI_WIDGET_H_

#include "common.h"
#include "BufferMgr.h"

namespace Dyninst {
namespace PatchAPI {

class Trace;

/* Widget code generation class */
class Widget {
 public:
  typedef dyn_detail::boost::shared_ptr<Widget> Ptr;
  typedef dyn_detail::boost::shared_ptr<Trace> TracePtr;

  Widget() {};

  // A default value to make sure things don't go wonky.
  virtual Address addr() const { return 0; }
  virtual unsigned size() const { return 0; }
  virtual InstructionAPI::Instruction::Ptr insn() const {
    return InstructionAPI::Instruction::Ptr();
  }

  // Make binary from the thing
  // Current address (if we need it)
  // is in the codeGen object.
  virtual bool generate(const Trace *trace,
                        BufferMgr &buffer) = 0;

  virtual std::string format() const = 0;
  virtual ~Widget() {};
};

};
};

#endif  // DYNINST_PATCHAPI_WIDGET_H_

