/* Plugin Interface */

#ifndef PATCHAPI_H_GENERATOR_H_
#define PATCHAPI_H_GENERATOR_H_

#include "common.h"
#include "SnippetRep.h"

namespace Dyninst {
namespace PatchAPI {

/* Generate binary code for snippets. */
class SnippetGenerator {
 public:
   SnippetGenerator() {}
   virtual ~SnippetGenerator() {}

   virtual bool process() { return false; }
};

}
}
#endif  // PATCHAPI_H_GENERATOR_H_
