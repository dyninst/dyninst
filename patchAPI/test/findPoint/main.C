/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */
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

  mapped_object* obj = addrSpaces[0]->getAOut();
  DynAddrSpacePtr as = DynAddrSpace::create(obj);
  PatchMgrPtr mgr = PatchMgr::create(as);

  mapped_object* lib_obj = addrSpaces[1]->getAOut();
  as->loadLibrary(lib_obj);

  // Find Points
  PatchFunction* foo3 = lib_obj->getFunc(foo3_func);
  const vector<PatchBlock*>& blks = foo3->getCallBlocks();
  for (int i = 0; i < blks.size(); i++) {
    vector<Point*> func_points;
    mgr->findPoints(blks[i], Point::PreInsn|Point::PostInsn, inserter(func_points, func_points.begin()));
    cerr << std::hex << blks[i]->start() << "--" << func_points.size() << " points found\n";
  }
  /*
  vector<Point*> pts;
  mgr->findPoints(foo3, Point::FuncExit, inserter(pts, pts.begin()));
  cerr << pts.size() << " exit points found\n";
  const vector<PatchBlock*>& blks2 = foo3->getExitBlocks();
  cerr << blks2.size() << " exit blocks\n";

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
  */
}
