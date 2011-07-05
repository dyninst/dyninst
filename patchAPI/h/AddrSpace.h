
/* Plugin Interface */

#ifndef PATCHAPI_H_ADDRSPACE_H_
#define PATCHAPI_H_ADDRSPACE_H_

#include "common.h"

namespace Dyninst {
namespace PatchAPI {

/* Interface specification for the interation between a PatchMgr and
   the address space */

class AddrSpace : public dyn_detail::boost::enable_shared_from_this<AddrSpace>{
    friend class PatchMgr;
    friend class PatchFunction;

  public:
    PATCHAPI_EXPORT static AddrSpacePtr create(PatchObject* obj);
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
    typedef std::set<PatchObject*> ObjSet;
    PATCHAPI_EXPORT ObjSet& objSet() { return obj_set_; }
    PATCHAPI_EXPORT PatchObject* getFirstObject() { return first_object_; }
    PATCHAPI_EXPORT PatchMgrPtr mgr() { return mgr_; }

    std::string format() const;

    bool consistency(const PatchMgr *mgr) const;

  protected:
    ObjSet obj_set_;
    PatchObject* first_object_;
    PatchMgrPtr mgr_;

    PATCHAPI_EXPORT bool init(PatchObject*);
    AddrSpace() {}
    explicit AddrSpace(AddrSpacePtr) {}
};

}
}

#endif /* PATCHAPI_H_ADDRSPACE_H_ */
