/* Plugin Interface */

#ifndef PATCHAPI_H_SNIPPETREP_H_
#define PATCHAPI_H_SNIPPETREP_H_

#include "common.h"

namespace Dyninst {
namespace PatchAPI {

/* Representation of snippet
   We don't template PatchAPI::Snippet class, because we don't want
   to template all other classes that utilizes PatchAPI::Snippet.
   So, we add another layer of indirection: SnippetRep.

   Usage:
   SnippetRep<AstNodePtr>* rep = new SnippetRep<AstNodePtr>(ast);
   SnippetPtr snip = Snippet::create(rep); */
template <class T>
class SnippetRep {
  public:
    explicit SnippetRep(T rep) : rep_(rep) {}
    ~SnippetRep() {}
    T rep() { return rep_; }

  protected:
    T rep_;
};

}
}
#endif  // PATCHAPI_H_SNIPPETREP_H_
