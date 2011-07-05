/*
 * #Name: patch1_1
 * #Desc: Mutator Side - Insert Snippets
 * #Dep:   Find Points      Snippets inserted
 *         ---------------------------------------------------------------
 *         - FuncEntry      1
 *         - BlockEntry     2, 3 (X2)
 *         - FuncDuring     4, 5, 6
 *         - PreCall        7, 8, 9
 *         - PostCall       10, 11
 *         - FuncExit       12, 13
 * #Notes:
 *         verify these things
 *         - FuncEntry point and BlockEntry point are different
 *         - CallBefore and CallAfter points are different
 *         - Each point's address is as expected
 *         - The # of snippets inserted at each point
 *         - The content of snippets at each point
 */


#include "test_lib.h"
#include "patchapi_comp.h"
using namespace Dyninst;

using Dyninst::PatchAPI::PatchFunction;
using Dyninst::PatchAPI::Point;
using Dyninst::PatchAPI::Snippet;
using Dyninst::PatchAPI::Patcher;
using Dyninst::PatchAPI::PushBackCommand;
using Dyninst::PatchAPI::InstancePtr;

class patch1_1_Mutator : public PatchApiMutator {
  virtual test_results_t executeTest();
};

extern "C" DLLEXPORT TestMutator* patch1_1_factory() {
  return new patch1_1_Mutator();
}

/* Dummy Snippets */
struct DummySnippet {
  DummySnippet(int n) : name(n) {}
  int name;
};

#define SnippetDef(NAME)  \
  DummySnippet s ## NAME ( NAME );  \
  Snippet<DummySnippet*>::Ptr snip ## NAME = Snippet<DummySnippet*>::create(&s ## NAME); \
  patcher.add(PushBackCommand::create(p, snip ## NAME));


bool checkSnippet(Point* pt, int expected_snip_num, int vals[]) {
  if (pt->size() != expected_snip_num) {
    logerror(" There should be %d snippet at %s point, but in fact %d\n",
             expected_snip_num, Dyninst::PatchAPI::type_str(pt->type()), pt->size());
    return false;
  }

  int counter = 0;
  for (Point::instance_iter iter = pt->begin(); iter != pt->end(); iter++) {
    DummySnippet* s = Snippet<DummySnippet*>::get((*iter)->snippet())->rep();
    if (s->name != vals[counter]) {
      logerror(" Wrong snippet value! Should be %d, but in fact %d\n", vals[counter], s->name);
      return false;
    }
    ++counter;
  }
  return true;
}

test_results_t patch1_1_Mutator::executeTest() {
   if (!mgr_->consistency()) {
      logerror("PatchMgr inconsistent at test entry!\n");
      return FAILED;
   }

  PatchFunction* func = findFunction("patch1_1_func");
  if (func == NULL) {
    logerror("**Failed patch1_1 (snippet insertion)\n");
    logerror("  Cannot find function %s \n", func->name().c_str());
    return FAILED;
  }
  logerror("- function %s found!\n", func->name().c_str());

  /* Step 1: find points */
  vector<Point*> pts;
  Point::Type type = Point::FuncEntry | Point::BlockEntry | Point::FuncDuring |
                     Point::PreCall | Point::PostCall | Point::FuncExit;
  mgr_->findPoints(PatchAPI::Scope(func), type, back_inserter(pts));
  // We should have all these point types
  typedef std::map<Point::Type, bool> TypeFound;
  TypeFound type_found;
  type_found[Point::FuncEntry] = false;
  type_found[Point::BlockEntry] = false;
  type_found[Point::FuncDuring] = false;
  type_found[Point::PreCall] = false;
  type_found[Point::PostCall] = false;
  type_found[Point::FuncExit] = false;

  /* Step 2: insert snippets */
  Point* func_entry = NULL;
  vector<Point*> block_entries;
  Point* func_exit = NULL;
  Point* func_during = NULL;
  Point* pre_call = NULL;
  Point* post_call = NULL;

  Patcher patcher(mgr_);
  for (vector<Point*>::iterator i = pts.begin(); i != pts.end(); i++) {
    Point* p = *i;
    switch (p->type()) {
      case Point::FuncEntry:
      {
        func_entry = p;
        SnippetDef(1);
        break;
      }
      case Point::BlockEntry:
      {
        block_entries.push_back(p);
        SnippetDef(2);
        SnippetDef(3);
        break;
      }
      case Point::FuncDuring:
      {
        func_during = p;
        SnippetDef(4);
        SnippetDef(5);
        SnippetDef(6);
        break;
      }
      case Point::PreCall:
      {
        pre_call = p;
        SnippetDef(7);
        SnippetDef(8);
        SnippetDef(9);
        break;
      }
      case Point::PostCall:
      {
        post_call = p;
        SnippetDef(10);
        SnippetDef(11);
        break;
      }
      case Point::FuncExit:
      {
        func_exit = p;
        SnippetDef(12);
        SnippetDef(13);
        break;
      }
      default:
         logerror("Wrong point type: %s!\n", PatchAPI::type_str(p->type()));
        return FAILED;
    }
  }
  patcher.commit();

  /* Step 3: verify! */

  // Step 3.1: checking whether findPoints is correct
  for (vector<Point*>::iterator i = pts.begin(); i != pts.end(); i++) {
    Point* p = *i;
    type_found[p->type()] = true;
    logerror("- Checking %s Point Type ...\n", Dyninst::PatchAPI::type_str(p->type()));
  }
  bool failed = false;
  for (TypeFound::iterator i = type_found.begin(); i != type_found.end(); i++) {
    if (!i->second) {
      failed = true;
      logerror(" %s is not found!\n", type_str(i->first));
    }
  }
  if (failed) return FAILED;

  // Step 3.3: checking snippets
  int entry_vals[1] = {1};
  if (!checkSnippet(func_entry, 1, entry_vals)) return FAILED;

  int exit_vals[2] = {12, 13};
  if (!checkSnippet(func_exit, 2, exit_vals)) return FAILED;

  int during_vals[3] = {4, 5, 6};
  if (!checkSnippet(func_during, 3, during_vals)) return FAILED;

  int precall_vals[3] = {7, 8, 9};
  if (!checkSnippet(pre_call, 3, precall_vals)) return FAILED;

  int postcall_vals[2] = {10, 11};
  if (!checkSnippet(post_call, 2, postcall_vals)) return FAILED;

  int blk_entry_vals[2] = {2, 3};
  if (!checkSnippet(block_entries[0], 2, blk_entry_vals)) return FAILED;

   if (!mgr_->consistency()) {
      logerror("PatchMgr inconsistent at test exit!\n");
      return FAILED;
   }

  return PASSED;
}
