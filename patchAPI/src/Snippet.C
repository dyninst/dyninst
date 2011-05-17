/* Public Interface */

#include "Snippet.h"

using Dyninst::PatchAPI::Snippet;
using Dyninst::PatchAPI::SnippetPtr;

SnippetPtr Snippet::create(void* snippet_rep) {
  SnippetPtr ret = SnippetPtr(new Snippet(snippet_rep));
  if (!ret) return SnippetPtr();
  return ret;
}
