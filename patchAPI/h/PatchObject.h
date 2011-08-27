/* Plugin Interface */

#ifndef PATCHAPI_H_DYNINST_OBJECT_H_
#define PATCHAPI_H_DYNINST_OBJECT_H_

#include "common.h"
#include "CFGMaker.h"

namespace Dyninst {
namespace PatchAPI {

class PatchFunction;
class PatchCallback;
class PatchParseCallback;

/* PatchObject represents a binary object, which could be either a library or
   executable. It is also an instrumentation  unit. */
class PatchObject {
  friend class AddrSpace;
  friend class PatchParseCallback;

  public:
    PATCHAPI_EXPORT static PatchObject* create(ParseAPI::CodeObject* co, Address base,
                                               CFGMaker* cm = NULL,
                                               PatchCallback *cb = NULL);

    PATCHAPI_EXPORT static PatchObject* clone(PatchObject* par_obj, Address base,
                                               CFGMaker* cm = NULL,
                                               PatchCallback *cb = NULL);

    PATCHAPI_EXPORT virtual ~PatchObject();

    typedef std::vector<PatchFunction *> funclist;
    typedef std::map<const ParseAPI::Function*, PatchFunction*> FuncMap;
    typedef std::map<const ParseAPI::Block*, PatchBlock*> BlockMap;
    typedef std::map<const ParseAPI::Edge*, PatchEdge*> EdgeMap;

    std::string format() const;

    // Getters and setter
    Address codeBase() { return codeBase_; }
    ParseAPI::CodeObject* co() const { return co_; }
    //ParseAPI::CodeSource* cs() const { return cs_; }
    AddrSpace* addrSpace() const { return addr_space_; }
    void setAddrSpace(AddrSpace* as) { addr_space_ = as; }

    // Function
    PATCHAPI_EXPORT PatchFunction *getFunc(ParseAPI::Function *, bool create = true);
    PATCHAPI_EXPORT void addFunc(PatchFunction*);
    PATCHAPI_EXPORT void removeFunc(PatchFunction*);
    PATCHAPI_EXPORT void removeFunc(ParseAPI::Function *);
    template <class Iter> 
	void funcs(Iter iter); 
    // Block
    PATCHAPI_EXPORT PatchBlock *getBlock(ParseAPI::Block*, bool create = true);
    PATCHAPI_EXPORT void addBlock(PatchBlock*);
    PATCHAPI_EXPORT void removeBlock(PatchBlock*);
    PATCHAPI_EXPORT void removeBlock(ParseAPI::Block*);
    template <class Iter>
     void blocks(Iter iter); 

    // Edge
    PATCHAPI_EXPORT PatchEdge *getEdge(ParseAPI::Edge*, PatchBlock* = NULL, PatchBlock* = NULL, bool create = true);
    PATCHAPI_EXPORT void addEdge(PatchEdge*);
    PATCHAPI_EXPORT void removeEdge(PatchEdge*);
    PATCHAPI_EXPORT void removeEdge(ParseAPI::Edge*);
    template <class Iter>
      void edges(Iter iter); 

    PATCHAPI_EXPORT PatchCallback *cb() const { return cb_; }

    PATCHAPI_EXPORT bool consistency(const AddrSpace *as) const;

  protected:
    ParseAPI::CodeObject* co_;
    Address codeBase_;
    AddrSpace* addr_space_;
    FuncMap funcs_;
    BlockMap blocks_;
    EdgeMap edges_;
    CFGMaker* cfg_maker_;

    PATCHAPI_EXPORT PatchObject(ParseAPI::CodeObject* o, Address a, CFGMaker* cm, PatchCallback *cb = NULL);
    PATCHAPI_EXPORT PatchObject(const PatchObject* par_obj, Address a, CFGMaker* cm, PatchCallback *cb = NULL);
    PATCHAPI_EXPORT void copyCFG(PatchObject* par_obj);
    PATCHAPI_EXPORT bool splitBlock(PatchBlock *first, ParseAPI::Block *second);

    PatchCallback *cb_;
    PatchParseCallback *pcb_;
};

template <class Iter>
   void PatchObject::funcs(Iter iter) {
   for (FuncMap::iterator tmp = funcs_.begin(); tmp != funcs_.end(); ++tmp) {
      *iter = tmp->second;
      ++iter;
   }
}

template <class Iter>
   void PatchObject::blocks(Iter iter) {
   for (BlockMap::iterator tmp = blocks_.begin(); tmp != blocks_.end(); ++tmp) {
      *iter = tmp->second;
      ++iter;
   }
}

template <class Iter>
   void PatchObject::edges(Iter iter) {
   for (EdgeMap::iterator tmp = edges_.begin(); tmp != edges_.end(); ++tmp) {
      *iter = tmp->second;
      ++iter;
   }
}



}
}

#endif  // PATCHAPI_H_DYNINST_MODULE_H_
