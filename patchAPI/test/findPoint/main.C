#include "SnippetRep.h"
#include "PatchMgr.h"
#include "Point.h"
#include "PatchCFG.h"

#include "DynObject.h"
#include "DynAddrSpace.h"

#include "BPatch.h"
#include "BPatch_binaryEdit.h"
#include "BPatch_image.h"
#include "BPatch_function.h"
#include "BPatch_Vector.h"
#include "addressSpace.h"
#include "mapped_object.h"

//parse API
#include <CodeObject.h>
#include <CodeSource.h>

#include <stdlib.h>
#include <iterator>
#include <string>
#include <vector>
#include <iterator>

using namespace Dyninst::PatchAPI;
using namespace Dyninst::ParseAPI;

int main(int argc, const char *argv[]) {

  // Use BPatch_* classes to initialize
  BPatch bpatch;
  BPatch_binaryEdit* app = bpatch.openBinary("mutatee/c");
  BPatch_image* image = app->getImage();


  BPatch_Vector<BPatch_function *> found_funcs;
  app->loadLibrary("mutatee/liblib.so");

  found_funcs.clear();
  image->findFunction("foo3", found_funcs);
  Function* foo3_func = found_funcs[0]->getParseAPIFunc();

  // Here we go, create PatchAPI objects!
  vector<AddressSpace*> addrSpaces;
  app->getAS(addrSpaces);

  CodeObject* co = addrSpaces[0]->getAOut()->parse_img()->codeObject();
  DynObject* obj = new DynObject(co, addrSpaces[0], 0);
  DynAddrSpacePtr as = DynAddrSpace::create(obj);
  PatchMgrPtr mgr = PatchMgr::create(as);

  CodeObject* co_lib = addrSpaces[1]->getAOut()->parse_img()->codeObject();
  DynObject* lib_obj = new DynObject(co_lib, addrSpaces[1], 0);
  as->loadLibrary(lib_obj);

  // Find Points
  PatchFunction* foo3 = lib_obj->getFunc(foo3_func);
  vector<PointPtr> func_points;
  mgr->findPoints(foo3, Point::CallBefore, inserter(func_points, func_points.begin()));
  cerr << func_points[0]->getCallee()->name() << "\n";
  // Insert snippets
  BPatch_variableExpr *intCounter = app->malloc(*image->findType("int"));
  BPatch_arithExpr addOne(BPatch_assign, *intCounter,
                          BPatch_arithExpr(BPatch_plus, *intCounter, BPatch_constExpr(1)));
  BPatch_arithExpr addTwo(BPatch_assign, *intCounter,
                          BPatch_arithExpr(BPatch_plus, *intCounter, BPatch_constExpr(2)));
  BPatch_arithExpr addThree(BPatch_assign, *intCounter,
                            BPatch_arithExpr(BPatch_plus, *intCounter, BPatch_constExpr(3)));
  BPatch_arithExpr addFour(BPatch_assign, *intCounter,
                           BPatch_arithExpr(BPatch_plus, *intCounter, BPatch_constExpr(4)));

  SnippetRep<AstNodePtr> one(addOne.ast_wrapper);
  SnippetRep<AstNodePtr> two(addTwo.ast_wrapper);
  SnippetRep<AstNodePtr> three(addThree.ast_wrapper);
  SnippetRep<AstNodePtr> four(addFour.ast_wrapper);
  SnippetPtr snippet = Snippet::create(&one);
  SnippetPtr snippet1 = Snippet::create(&two);
  SnippetPtr snippet2 = Snippet::create(&three);
  SnippetPtr snippet3 = Snippet::create(&four);

  vector<InstancePtr> errorInstances;

  mgr->batchStart();
  func_points[0]->push_back(snippet);
  mgr->batchFinish(errorInstances);

  PatchObject::destroy(obj);
  PatchObject::destroy(lib_obj);
  delete obj;
  delete lib_obj;
}
