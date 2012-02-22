/*
 * #Name: patch2_1
 * #Desc: Mutator Side - Remove Snippets at Function Entry
 * #Dep:
 * #Notes:
 *     batch               operation               result
 *     -----               ----------              ------
 *     patcher1            push_back 1             1
 *     patcher1            push_back 2             1,2
 *     patcher1            push_back 3             1,2,3
 *     patcher1            push_back 4             1,2,3,4
 *     patcher2            remove 2                1,3,4
 *     patcher2            remove 4                1,3
 *     patcher2            remove 1                3
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
using Dyninst::PatchAPI::Command;
using Dyninst::PatchAPI::RemoveSnippetCommand;
using Dyninst::PatchAPI::InstancePtr;

/* Dummy Snippets */
struct DummySnippet {
  DummySnippet(int n) : name(n) {}
  int name;
};
#define SnippetDef(NAME)  \
  DummySnippet s ## NAME ( NAME );  \
  Snippet<DummySnippet*>::Ptr snip ## NAME = Snippet<DummySnippet*>::create(&s ## NAME);

class patch2_1_Mutator : public PatchApiMutator {
  virtual test_results_t executeTest();
};

extern "C" DLLEXPORT TestMutator* patch2_1_factory() {
  return new patch2_1_Mutator();
}

test_results_t patch2_1_Mutator::executeTest() {
   if (!mgr_->consistency()) {
      logerror("PatchMgr inconsistent at test entry!\n");
      return FAILED;
   }

  PatchFunction* func = findFunction("patch2_1_func");
  if (func == NULL) {
    logerror("**Failed patch2_1 (delete snippet)\n");
    return FAILED;
  }
  logerror("- function %s found!\n", func->name().c_str());

  /* Step 1: find Points */
  vector<Point*> pts;
  Point::Type type = Point::FuncEntry;
  mgr_->findPoints(PatchAPI::Location::Function(func), type, back_inserter(pts));
  if (1 != pts.size()) {
    logerror("**Failed patch2_1 (snippet removal)\n");
    logerror("  cannot find correct point at function entry\n");
    return FAILED;
  }

  /* Step 2: insert snippets */
  Patcher patcher1(mgr_);
  SnippetDef(1);
  PushBackCommand* c1 = PushBackCommand::create(pts[0], snip1);
  patcher1.add(c1);

  SnippetDef(2);
  PushBackCommand* c2 = PushBackCommand::create(pts[0], snip2);
  patcher1.add(c2);

  SnippetDef(3);
  PushBackCommand* c3 = PushBackCommand::create(pts[0], snip3);
  patcher1.add(c3);

  SnippetDef(4);
  PushBackCommand* c4 = PushBackCommand::create(pts[0], snip4);
  patcher1.add(c4);
  patcher1.commit();

  Patcher patcher2(mgr_);
  patcher2.add(RemoveSnippetCommand::create(c2->instance()));
  patcher2.add(RemoveSnippetCommand::create(c4->instance()));
  patcher2.commit();

  Patcher patcher3(mgr_);
  patcher3.add(RemoveSnippetCommand::create(c1->instance()));
  patcher3.commit();

  /* Step 3: verify the insertion order */
  int expected_vals[] = {3};
  int counter = 0;
  for (Point::instance_iter i = pts[0]->begin(); i != pts[0]->end(); i++) {
    DummySnippet* s = Snippet<DummySnippet*>::get((*i)->snippet())->rep();

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
