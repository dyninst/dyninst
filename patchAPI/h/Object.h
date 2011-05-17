/* Plugin Interface */

#ifndef PATCHAPI_H_DYNINST_OBJECT_H_
#define PATCHAPI_H_DYNINST_OBJECT_H_

#include "common.h"

namespace Dyninst {
namespace PatchAPI {

class PatchFunction;

/* Object represents a binary object, which could be either a library or
   executable. It is also a code relocated unit. */
class Object : public dyn_detail::boost::enable_shared_from_this<Object> {
  public:
    friend class AddrSpace;

    virtual ~Object();
    static void destroy(ObjectPtr obj);

    typedef std::vector<PatchFunction *> funclist;
    typedef std::map<ParseAPI::Function *, PatchFunction *> FuncMap;

    // Getters and setter
    void setAs(AddrSpacePtr as) { addr_space_ = as; }
    Address codeBase() { return codeBase_; }
    ParseAPI::CodeObject* co() const { return co_; }
    ParseAPI::CodeSource* cs() const { return cs_; }
    AddrSpacePtr addrSpace() const { return addr_space_; }
    FuncMap& funcMap() { return funcMap_; }
    virtual PatchFunction *getFunction(ParseAPI::Function *);
    virtual void setFunction(PatchFunction* f);

    // Called by Instrumenter
    virtual bool instPreprocess(InstanceSet* /*insertion_set*/,
                                 InstanceSet* /*deletion_set*/,
                                 FuncRepMap*  /*func_rep*/,
                                 CallRepMap*  /*call_rep*/,
                                 CallRemoval* /*call_remove*/) = 0;
    virtual bool instProcess() = 0;

    // Called by Linker
    virtual bool linkerPreprocess() = 0;
    virtual bool linkerProcess() = 0;

  protected:
    ParseAPI::CodeObject* co_;
    ParseAPI::CodeSource* cs_;
    Address codeBase_;
    AddrSpacePtr addr_space_;
    FuncMap funcMap_;

    Object(ParseAPI::CodeObject* o, Address a);
    Object(ObjectPtr par, ParseAPI::CodeObject* o, Address a);
};

}
}

#endif  // PATCHAPI_H_DYNINST_MODULE_H_
