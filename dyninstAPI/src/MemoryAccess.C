/* $Id: MemoryAccess.C,v 1.4 2002/08/06 23:20:54 gaburici Exp $ */

#include <BPatch_memoryAccess_NP.h>

BPatch_memoryAccess* const BPatch_memoryAccess::none = init_tables();

BPatch_Vector<BPatch_point*>* BPatch_memoryAccess::filterPoints(
		const BPatch_Vector<BPatch_point *> &points,
                unsigned int numMAs)
{
  BPatch_Vector<BPatch_point*> *result = new BPatch_Vector<BPatch_point *>;

  for(unsigned int i = 0; i < points.size(); ++i) {
    const BPatch_memoryAccess *ma = points[i]->getMemoryAccess();
    if(ma)
      if(ma->getNumberOfAccesses() >= numMAs)
        result->push_back(points[i]);
  }
  return result;
}
