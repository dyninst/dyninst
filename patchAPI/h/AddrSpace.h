
/* Plugin Interface */

#ifndef PATCHAPI_H_ADDRSPACE_H_
#define PATCHAPI_H_ADDRSPACE_H_

#include "common.h"

namespace Dyninst {
namespace PatchAPI {

/* Interface specification for the interation between a PatchMgr and
   the address space */

class AddrSpace {
    friend class PatchMgr;
    friend class PatchFunction;

  public:
    PATCHAPI_EXPORT static AddrSpace* create(PatchObject* obj);
    PATCHAPI_EXPORT virtual ~AddrSpace();

    // Write data in mutatee's address space
    PATCHAPI_EXPORT virtual bool write(PatchObject* /*obj*/, Address /*to*/,
                                       Address /*from*/, size_t /*size*/);

    // Memory allocation / reallocation / deallocation in mutatee's addressSpace
    PATCHAPI_EXPORT virtual Address malloc(PatchObject* /*obj*/, size_t /*size*/,
                                           Address /*near*/);

    PATCHAPI_EXPORT virtual bool realloc(PatchObject* /*obj*/, Address /*orig*/,
                                         size_t /*size*/);

    PATCHAPI_EXPORT virtual bool free(PatchObject* /*obj*/, Address /*orig*/);

    // Load a binary oject into the address space
    PATCHAPI_EXPORT virtual bool loadObject(PatchObject* obj);

    // Getters
    typedef std::map<const ParseAPI::CodeObject*, PatchObject*> ObjMap;
    ObjMap& objMap() { return obj_map_; }
    PatchObject* findObject(const ParseAPI::CodeObject*) const;
    template <class Iter> void objs(Iter iter); // EXPORTED
    PATCHAPI_EXPORT PatchObject* getFirstObject() { return first_object_; }
    PATCHAPI_EXPORT PatchMgrPtr mgr() { return mgr_; }

    std::string format() const;

    bool consistency(const PatchMgr *mgr) const;

  protected:
    ObjMap obj_map_;
    PatchObject* first_object_;
    PatchMgrPtr mgr_;

    PATCHAPI_EXPORT bool init(PatchObject*);
    AddrSpace() {}
    explicit AddrSpace(AddrSpace*) {}
};

template <class Iter>
   void AddrSpace::objs(Iter iter) {
   for (ObjMap::iterator tmp = obj_map_.begin(); tmp != obj_map_.end(); ++tmp) {
      *iter = tmp->second;
      ++iter;
   }
}



}
}

#endif /* PATCHAPI_H_ADDRSPACE_H_ */
