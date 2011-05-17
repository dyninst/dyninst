//#include "dyninst/DynRewriterAs.h"
//#include "dyninst/DynInstrumenter.h"
//#include "dyninst/DynLinker.h"
#include "SnippetRep.h"
#include "PatchMgr.h"
#include "Point.h"
#include "PatchCFG.h"

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

  //---------------------------------------- 
  //Use BPatch_* classes to initialize
  //---------------------------------------- 
  BPatch bpatch;
  BPatch_binaryEdit* app = bpatch.openBinary("mutatee/c");
  BPatch_image* image = app->getImage();


  BPatch_Vector<BPatch_function *> found_funcs;

  //  image->findFunction("main", found_funcs);
  //  Function* main_func = found_funcs[0]->getParseAPIFunc();

  app->loadLibrary("mutatee/liblib.so");

  found_funcs.clear();
  image->findFunction("foo3", found_funcs);
  Function* foo3_func = found_funcs[0]->getParseAPIFunc();
  
  //------------------------------------------
  //Here we go, create PatchAPI objects!
  //------------------------------------------
  vector<AddressSpace*> addrSpaces;
  app->getAS(addrSpaces);
  cerr << "start..." << addrSpaces.size() << "\n";

  CodeObject* co = addrSpaces[0]->getAOut()->parse_img()->codeObject();
  AddrSpacePtr as = AddrSpace::create(co, 0);
  InstrumenterPtr instor = Instrumenter::create(as);
  LinkerPtr linker = Linker::create(as);
  PatchMgrPtr mgr = PatchMgr::create(instor, linker);

  CodeObject* co_lib = addrSpaces[1]->getAOut()->parse_img()->codeObject();
  ObjectPtr obj = as->loadObject(co_lib, 0);
  PatchFunction* foo3 = obj->getFunction(foo3_func);
  cerr << foo3->name() << "\n";
  vector<PointPtr> func_points;
  mgr->findPoints(foo3, Point::CallBefore, inserter(func_points, func_points.begin()));
  cerr << func_points[0]->getCallee()->name() << "\n";

  /*
  DynRewriterAsPtr as = DynRewriterAs::create(addrSpaces[0]);
  DynInstrumenterPtr instor = DynInstrumenter::create(as);
  DynLinkerPtr linker = DynLinker::create(as);
  PatchMgrPtr mgr = PatchMgr::create(instor, linker);
  */
  //as->loadLibrary(addrSpaces[1]);

  //------------------------------------------
  //find points
  //------------------------------------------
  /*
  //mgr->findPoints(main_func, Point::CallBefore, inserter(func_points, func_points.begin()));
	for (vector<PointPtr>::iterator i = func_points.begin(); i != func_points.end(); i++) 
	  cout << "\t\t" << type_str((*i)->type()) << ": " << (*i)->getCallee()->name() << "\n";

	//insn
  Address insn = 0x4004cc;
  vector<PointPtr> insn_points;
  mgr->findPoints(&insn, Point::CallBefore, inserter(insn_points, insn_points.begin()));
	for (vector<PointPtr>::iterator i = insn_points.begin(); i != insn_points.end(); i++) 
	  cout << "\t\t" << std::hex << (*i)->address() << "\n";

	//function
  func_points.clear();
  mgr->findPoints(main_func, Point::EdgeDuring|Point::CallBefore, inserter(func_points, func_points.begin()));
	for (vector<PointPtr>::iterator i = func_points.begin(); i != func_points.end(); i++) {
    if ((*i)->type() == Point::CallBefore)
	    cout << "\t\t" << type_str((*i)->type()) << ": " << (*i)->getCallee()->name() << "\n";
    else
	    cout << "\t\t" << type_str((*i)->type()) << "\n"; 
  }
  */
}
