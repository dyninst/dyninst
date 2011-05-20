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
    static AddrSpacePtr create(PatchObject* obj);
    virtual ~AddrSpace();

    // Write data in mutatee's address space
    virtual bool write(PatchObject* /* obj */,
                       Address   /* to */,
                       Address   /*from*/,
                       size_t    /* size */) {
      return false;
    }

    // Memory allocation / reallocation / deallocation in mutatee's address space
    virtual Address malloc(PatchObject* /* obj */,
                           size_t   /* size  */,
                           Address  /* near */) {
      return false;
    }
    virtual bool realloc(PatchObject* /* obj */,
                         Address   /* orig */,
                         size_t    /* size */) {
      return false;
    }
    virtual bool free(PatchObject* /* obj */,
                      Address   /* orig */) {
      return false;
    }

    // Load a binary oject into the address space
    virtual bool loadObject(PatchObject* obj);

    // Getters
    typedef std::map<ParseAPI::CodeObject*, PatchObject*> CoObjMap;
    CoObjMap& getCoobjMap() { return coobj_map_; }
    PatchObject* getFirstObject() { return first_object_; }
    PatchMgrPtr mgr() { return mgr_; }

  protected:
    CoObjMap coobj_map_;
    PatchObject* first_object_;
    PatchMgrPtr mgr_;

    bool init(PatchObject*);
    AddrSpace() {}
    explicit AddrSpace(AddrSpacePtr) {}
};

}
}

#endif /* PATCHAPI_H_ADDRSPACE_H_ */
