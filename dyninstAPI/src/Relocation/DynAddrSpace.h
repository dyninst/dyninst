/* Plugin Implementation */

#ifndef PATCHAPI_H_DYNINST_DYNADDRSPACE_H_
#define PATCHAPI_H_DYNINST_DYNADDRSPACE_H_

#include "DynCommon.h"

class AddressSpace;

namespace Dyninst {
namespace PatchAPI {

class DynAddrSpace : public AddrSpace {
  public:
    static DynAddrSpace* create(DynObject* obj);
    bool loadLibrary(DynObject*);
    bool initAs(DynObject*);
    bool removeAddrSpace(AddressSpace *);

    typedef std::set<AddressSpace*> AsSet;
    AsSet& asSet() { return as_set_; }
    //typedef std::map<ParseAPI::CodeObject*, AddressSpace*> CoAsMap;
    //CoAsMap& coas_map() { return coas_map_; }

    virtual bool write(PatchObject*, Address to, Address from, size_t size);
    virtual Address malloc(PatchObject*, size_t size, Address near);
    virtual bool realloc(PatchObject*, Address orig, size_t size);
    virtual bool free(PatchObject*, Address orig);

    bool isRecursive() { return recursive_; }
    void setRecursive(bool r) { recursive_ = r; }

  protected:
    DynAddrSpace();
    DynAddrSpace(AddrSpace* par);

    //CoAsMap coas_map_;
    AsSet as_set_;
    AddressSpace* first_as_;
    bool recursive_;
};

}
}
#endif  // PATCHAPI_H_DYNINST_DYNADDRSPACE_H_
