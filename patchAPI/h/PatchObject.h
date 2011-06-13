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
    PATCHAPI_EXPORT static PatchObject* create(ParseAPI::CodeObject* co, Address base,
                                               CFGMakerPtr cm = CFGMakerPtr(new CFGMaker));
    PATCHAPI_EXPORT static PatchObject* clone(PatchObject* par_obj, Address base);
    PATCHAPI_EXPORT virtual ~PatchObject();

    typedef std::vector<PatchFunction *> funclist;
    typedef std::map<const ParseAPI::Function*, PatchFunction*> FuncMap;
    typedef std::map<const ParseAPI::Block*, PatchBlock*> BlockMap;
    typedef std::map<const ParseAPI::Edge*, PatchEdge*> EdgeMap;

    // Getters and setter
    Address codeBase() { return codeBase_; }
    ParseAPI::CodeObject* co() const { return co_; }
    //ParseAPI::CodeSource* cs() const { return cs_; }
    AddrSpacePtr addrSpace() const { return addr_space_; }
    void setAddrSpace(AddrSpacePtr as) { addr_space_ = as; }

    // Function
    PATCHAPI_EXPORT PatchFunction *getFunc(ParseAPI::Function *);
    PATCHAPI_EXPORT void addFunc(PatchFunction*);
    PATCHAPI_EXPORT void removeFunc(PatchFunction*);

    // Block
    PATCHAPI_EXPORT PatchBlock *getBlock(ParseAPI::Block*);
    PATCHAPI_EXPORT void addBlock(PatchBlock*);
    PATCHAPI_EXPORT void removeBlock(PatchBlock*);

    // Edge
    PATCHAPI_EXPORT PatchEdge *getEdge(ParseAPI::Edge*, PatchBlock*, PatchBlock*);
    PATCHAPI_EXPORT void addEdge(PatchEdge*);
    PATCHAPI_EXPORT void removeEdge(PatchEdge*);

  protected:
    ParseAPI::CodeObject* co_;
    //ParseAPI::CodeSource* cs_;
    Address codeBase_;
    AddrSpacePtr addr_space_;
    FuncMap funcs_;
    BlockMap blocks_;
    EdgeMap edges_;
    CFGMakerPtr cfg_maker_;

    PATCHAPI_EXPORT PatchObject(ParseAPI::CodeObject* o, Address a, CFGMakerPtr cm);
    PATCHAPI_EXPORT PatchObject(const PatchObject* par_obj, Address a);
    PATCHAPI_EXPORT void copyCFG(PatchObject* par_obj);
};

}
}

#endif  // PATCHAPI_H_DYNINST_MODULE_H_
