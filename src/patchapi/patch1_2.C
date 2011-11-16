/*
 * #Name: patch1_2
 * #Desc: Mutator Side - Insert Snippet Order
 * #Dep:
 * #Notes:
 *     batch            operation               result
 *     -----           -----------              ------
 *     patcher1         push_back 1             1
 *     patcher1         push_back 2             1,2
 *     patcher1         push_front 3            3,1,2
 *     patcher1         push_front 4            4,3,1,2
 *     patcher2         none                    4,3,1,2
 *     patcher3         push_back 5             4,3,1,2,5
 *     patcher3         push_front 6            6,4,3,1,2,5
 *     patcher4         push_front 7            7,6,4,3,1,2,5
 */

#include "test_lib.h"
#include "patchapi_comp.h"

using namespace Dyninst;

using Dyninst::PatchAPI::PatchFunction;
using Dyninst::PatchAPI::Patcher;
using Dyninst::PatchAPI::PushFrontCommand;
using Dyninst::PatchAPI::PushBackCommand;
using Dyninst::PatchAPI::Point;
using Dyninst::PatchAPI::Snippet;
using Dyninst::PatchAPI::Buffer;

class patch1_2_Mutator : public PatchApiMutator {
  virtual test_results_t executeTest();
};

extern "C" DLLEXPORT TestMutator* patch1_2_factory() {
  return new patch1_2_Mutator();
}

class DummySnippet : public Dyninst::PatchAPI::Snippet {
public:
  DummySnippet(int n) : name(n) {}
  int name;

   virtual bool generate(Point *, Buffer &) { return false; }
   virtual ~DummySnippet() {};

};

#define SnippetDef(NAME)  \
  Snippet::Ptr snip ## NAME = Snippet::create(new DummySnippet(NAME)); 

test_results_t patch1_2_Mutator::executeTest() {
   if (!mgr_->consistency()) {
      logerror("PatchMgr inconsistent at test entry!\n");
      return FAILED;
   }

  PatchFunction* func = findFunction("patch1_2_func");
  if (func == NULL) {
    logerror("**Failed patch1_2 (snippet insertion order)\n");
    logerror("  Cannot find function %s \n", func->name().c_str());
    return FAILED;
  }
  logerror("- function %s found!\n", func->name().c_str());

  /* Step 1: find Points */
  vector<Point*> pts;
  Point::Type type = Point::FuncEntry;
  mgr_->findPoints(PatchAPI::Location::Function(func), type, back_inserter(pts));
  if (1 != pts.size()) {
    logerror("**Failed patch1_2 (snippet insertion order)\n");
    logerror("  cannot find correct point at function entry\n");
    return FAILED;
  }

  /* Step 2: insert snippets */
  Patcher patcher1(mgr_);
  SnippetDef(1);
  patcher1.add(PushBackCommand::create(pts[0], snip1));
  SnippetDef(2);
  patcher1.add(PushBackCommand::create(pts[0], snip2));
  SnippetDef(3);
  patcher1.add(PushFrontCommand::create(pts[0], snip3));
  SnippetDef(4);
  patcher1.add(PushFrontCommand::create(pts[0], snip4));
  patcher1.commit();

  Patcher patcher2(mgr_);
  patcher2.commit();

  Patcher patcher3(mgr_);
  SnippetDef(5);
  patcher3.add(PushBackCommand::create(pts[0], snip5));
  SnippetDef(6);
  patcher3.add(PushFrontCommand::create(pts[0], snip6));
  patcher3.commit();

  Patcher patcher4(mgr_);
  SnippetDef(7);
  patcher4.add(PushFrontCommand::create(pts[0], snip7));
  patcher4.commit();

  /* Step 3: verify the insertion order */
  int expected_vals[] = {7,6,4,3,1,2,5};
  int counter = 0;
  for (Point::instance_iter i = pts[0]->begin(); i != pts[0]->end(); i++) {
     DummySnippet* s = static_cast<DummySnippet *>((*i)->snippet().get());
    if (s->name != expected_vals[counter]) {
      logerror(" expect %d, but in fact %d", s->name, expected_vals[counter]);
      return FAILED;
    }
    ++counter;
  }

   if (!mgr_->consistency()) {
      logerror("PatchMgr inconsistent at test exit!\n");
      return FAILED;
   }
  return PASSED;
}
