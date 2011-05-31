/* Plugin Implementation */

#include "DynObject.h"
#include "PatchMgr.h"
#include "process.h"

using Dyninst::ParseAPI::CodeObject;
using Dyninst::ParseAPI::CodeRegion;
using Dyninst::PatchAPI::DynObject;
using Dyninst::PatchAPI::DynObjectPtr;
using Dyninst::PatchAPI::InstanceSet;
using Dyninst::PatchAPI::InstancePtr;
using Dyninst::PatchAPI::DynCFGMakerPtr;
using Dyninst::PatchAPI::DynCFGMaker;

DynObject::DynObject(ParseAPI::CodeObject* co, AddressSpace* as, Address base)
  : PatchObject(co, base, DynCFGMakerPtr(new DynCFGMaker)), as_(as) {
}

DynObject::DynObject(const DynObject* par_obj, process* child, Address base)
  : PatchObject(par_obj, base), as_(child) {
}

DynObject::~DynObject() {
}

