/* Utility */
#ifndef PATCHAPI_H_DYNINST_WIDGET_TARGET_H_
#define PATCHAPI_H_DYNINST_WIDGET_TARGET_H_

#include "common.h"
#include "Widget.h"
#include "Trace.h"

namespace Dyninst {
namespace PatchAPI {

/* Wraps an object that can serve as a  control flow target. This
   may include existing code objects (a block or function)
   or something that has been relocated. We wrap them with this
   template class (which will then be specialized as appropriate)
   so we don't pollute the base class with extraneous code (I'm
   looking at _you_, get_address_cr....

   Preliminary requirement: T must be persistent during the existence
   of this class so we can use a reference to it.

   predictedAddr takes into account things moving during code generation */
class TargetInt {
  public:
    typedef enum {
      Illegal,
      TraceTarget,
      BlockTarget,
      AddrTarget
    } type_t;

    TargetInt() : necessary_(true) {}
    virtual ~TargetInt() {}
    virtual std::string format() const { return "<INVALID>"; }
    virtual Address origAddr() const = 0;

    // It would be nice to eventually move these into the code generator loop,
    // but for now it's okay to keep them here.
    virtual bool necessary() const { return necessary_; }
    virtual void setNecessary(bool a) { necessary_ = a; }
    virtual type_t type() const { return Illegal; }

    virtual bool matches(Trace::Ptr) const { return false; }
    virtual int label(BufferMgr * /*buf*/) const { return -1; }

  protected:
    bool necessary_;
};

template <typename T>
class Target : public TargetInt {
  public:
    explicit Target(const T t) : t_(t) {}
    ~Target() {}
    const T t() { return t_; }

  private:
    const T &t_;
};

template <>
class Target<Trace::Ptr> : public TargetInt {
  public:
    explicit Target(Trace::Ptr t) : t_(t) {}
    ~Target() {}
    const Trace::Ptr &t() const { return t_; }
    Address origAddr() const { return t_->origAddr(); }
    virtual type_t type() const { return TraceTarget; }
    virtual string format() const {
      stringstream ret;
      ret << "B{" << t_->id() << "/" << (necessary() ? "+" : "-") << "}";
      return ret.str();
    }

    virtual bool matches(Trace::Ptr t) const { return (t_ == t); }
    int label(BufferMgr * /*buf*/) const { return t_->getLabel(); }

  private:
    const Trace::Ptr t_;
};

template <>
class Target<PatchBlock *> : public TargetInt {
  public:

    explicit Target(PatchBlock *t) : t_(t) {}
    ~Target() {}

    PatchBlock *t() const { return t_; }
    virtual type_t type() const { return BlockTarget; }
    Address origAddr() const { return t_->start(); }

    virtual string format() const {
      stringstream ret;
      assert(t_);
      ret << "O{" << std::hex << t_->start() << "/" << (necessary() ? "+" : "-")
          << std::dec << "}";
      return ret.str();
    }
    int label(BufferMgr* buf) const;

  private:
    PatchBlock *t_;
};


template <>
class Target<Address> : public TargetInt {
  public:
    explicit Target(Address t) : t_(t) {}
    ~Target() {}
    const Address &t() const { return t_; }
    virtual type_t type() const { return AddrTarget; }

    Address origAddr() const { return t_; }
    virtual string format() const {
      stringstream ret;
      ret << "A{" << std::hex << t_ << "/" << (necessary() ? "+" : "-")
          <<  std::dec << "}";
      return ret.str();
    }

    int label(BufferMgr* buf) const;

 private:
  const Address t_;
};

};
};

#endif  // PATCHAPI_H_DYNINST_WIDGET_TARGET_H_
