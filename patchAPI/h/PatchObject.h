/* Plugin Interface */

#ifndef PATCHAPI_H_DYNINST_OBJECT_H_
#define PATCHAPI_H_DYNINST_OBJECT_H_

#include "common.h"
#include "CFGMaker.h"

namespace Dyninst {
namespace PatchAPI {

class PatchFunction;

/* PatchObject represents a binary object, which could be either a library or
   executable. It is also an instrumentation  unit. */
class PatchObject {
  friend class AddrSpace;

  public:
    static PatchObject* create(ParseAPI::CodeObject* co, Address base,
                               CFGMakerPtr cm = CFGMakerPtr(new CFGMaker)) {
      return (new PatchObject(co, base, cm));
    }
    virtual ~PatchObject();

    typedef std::vector<PatchFunction *> funclist;
    typedef std::map<const ParseAPI::Function*, PatchFunction*> FuncMap;
    typedef std::map<const ParseAPI::Block*, PatchBlock*> BlockMap;

    // Getters and setter
    Address codeBase() { return codeBase_; }
    ParseAPI::CodeObject* co() const { return co_; }
    ParseAPI::CodeSource* cs() const { return cs_; }
    AddrSpacePtr addrSpace() const { return addr_space_; }
    void setAddrSpace(AddrSpacePtr as) { addr_space_ = as; }

    // Function
    PatchFunction *getFunc(ParseAPI::Function *);
    void addFunc(PatchFunction*);
    void removeFunc(PatchFunction*);

    // Block
    PatchBlock *getBlock(ParseAPI::Block*);
    void addBlock(PatchBlock*);
    void removeBlock(PatchBlock*);

    // Called by Instrumenter
    virtual bool instrument(InstanceSet* /*insertion_set*/,
                         InstanceSet* /*deletion_set*/,
                         FuncRepMap*  /*func_rep*/,
                         CallRepMap*  /*call_rep*/,
                         CallRemoval* /*call_remove*/)  { return false;}

  protected:
    ParseAPI::CodeObject* co_;
    ParseAPI::CodeSource* cs_;
    Address codeBase_;
    AddrSpacePtr addr_space_;
    FuncMap funcs_;
    BlockMap blocks_;
    CFGMakerPtr cfg_maker_;

    PatchObject(ParseAPI::CodeObject* o, Address a, CFGMakerPtr cm);
    PatchObject(const PatchObject* par_obj, Address a);
};

}
}

#endif  // PATCHAPI_H_DYNINST_MODULE_H_
