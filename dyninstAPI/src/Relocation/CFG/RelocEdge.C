#include "RelocBlock.h"
#include "RelocTarget.h"
#include "RelocEdge.h"

using namespace Dyninst;
using namespace Relocation;

RelocEdge::~RelocEdge() { 
   if (src) delete src;
   if (trg) delete trg;
}

RelocEdge *RelocEdges::find(ParseAPI::EdgeTypeEnum e) {
   // Returns the first one
   for (iterator iter = begin(); iter != end(); ++iter) {
      if ((*iter)->type == e) return *iter;
   }
   return NULL;
}

void RelocEdges::erase(RelocEdge *e) {
   for (iterator iter = begin(); iter != end(); ++iter) {
      if ((*iter) == e) {
         edges.erase(iter);
         return;
      }
   }
}

bool RelocEdges::contains(ParseAPI::EdgeTypeEnum e) {
   return (find(e) != NULL);
}

