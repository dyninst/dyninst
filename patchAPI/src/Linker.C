/* Plugin */

#include "Linker.h"

using Dyninst::PatchAPI::LinkerPtr;
using Dyninst::PatchAPI::Linker;

LinkerPtr Linker::create(AddrSpacePtr as) {
  LinkerPtr ret = LinkerPtr(new Linker(as));
  if (!ret) return LinkerPtr();
  return ret;
}

bool Linker::preprocess() {
  for (AddrSpace::CoObjMap::iterator ci = as_->getCoobjMap().begin();
       ci != as_->getCoobjMap().end(); ci++) {
    ObjectPtr obj = (*ci).second;
    obj->linkerPreprocess();
  }
  return true;
}

bool Linker::process() {
  for (AddrSpace::CoObjMap::iterator ci = as_->getCoobjMap().begin();
       ci != as_->getCoobjMap().end(); ci++) {
    ObjectPtr obj = (*ci).second;
    obj->linkerProcess();
  }
  return true;
}

