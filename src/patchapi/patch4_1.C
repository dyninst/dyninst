/*
   #Name: patch4_1
   #Desc: transactional semantics
   #Dep:
     1. Run a batch of operations successfully.
       - Build a batch:
          level1: DummyCommand1->DummyCommand2->DummyCommand3->DummyCommand4->DummyInstrumenter
          level2: DummyInstrumenter
          level3: (from DummyInstrumenter) DummyCommand5->DummyCommand6->DummyCommand7
          level4: (from DummyCommand5) DummyCommand8->DummyCommand9
                  (from DummyCommand7) DummyCommand10->DummyCommand11
         in tree shape:
               Patcher
             / |  |  |  \
            1  2  3  4  inst
                       / |  \
                      5  6   7
                     / \    /  \
                    8   9   10  11
      - check the numeric id of each command

     2. An operation fails, abort this batch.
      - Build a batch as 1, except for making DummyCommand8/10's run return false.
      - check that DummyCommand 10 never executes and all operations are undo before 8.
 */

#include "test_lib.h"
#include "patchapi_comp.h"

using Dyninst::PatchAPI::Command;
using Dyninst::PatchAPI::Patcher;
using Dyninst::PatchAPI::PatchMgr;
using Dyninst::PatchAPI::PatchMgrPtr;
using Dyninst::PatchAPI::AddrSpacePtr;
using Dyninst::PatchAPI::BatchCommand;

typedef std::list<int> CmdList;

/* Implementation of 11 DummyCommands */

#define DummyCommandDef(NUM)     \
class DummyCommand ## NUM : public Command { \
  public: \
    typedef dyn_detail::boost::shared_ptr<DummyCommand ## NUM> Ptr; \
    DummyCommand ## NUM (CmdList* cl) : cl_(cl), run_ret_(true) {} \
    static Ptr create(CmdList* cl) { return Ptr(new DummyCommand ## NUM (cl)); } \
    virtual bool run() { cl_->push_back(NUM); return run_ret_; } \
    virtual bool undo()  { cl_->pop_back(); return true; } \
    void setRunReturn(bool r) { run_ret_ = r; } \
  private: \
    CmdList* cl_; \
    bool run_ret_; \
};

DummyCommandDef(1) // DummyCommand1
DummyCommandDef(2) // DummyCommand2
DummyCommandDef(3) // DummyCommand3
DummyCommandDef(4) // DummyCommand4
DummyCommandDef(6) // DummyCommand6
DummyCommandDef(8) // DummyCommand8
DummyCommandDef(9) // DummyCommand9
DummyCommandDef(10) // DummyCommand10
DummyCommandDef(11) // DummyCommand11

/* Implementation of DummyCommand5, and 7 */
#define DummyBatchCommandDef(NUM, CHILD1, CHILD2)  \
class DummyCommand ## NUM : public BatchCommand { \
  public: \
    typedef dyn_detail::boost::shared_ptr<DummyCommand ## NUM> Ptr; \
    DummyCommand ## NUM (CmdList* cl) { \
      add(DummyCommand ## CHILD1::create(cl)); \
      add(DummyCommand ## CHILD2::create(cl)); \
    } \
    static Ptr create(CmdList* cl) { return Ptr(new DummyCommand ## NUM(cl)); } \
};

DummyBatchCommandDef(5, 8, 9)
DummyBatchCommandDef(7, 10, 11)

/* Implementation of DummyInstrumenter */
class DummyInstrumenter : public BatchCommand {
  public:
    typedef dyn_detail::boost::shared_ptr<DummyInstrumenter> Ptr;
    DummyInstrumenter(CmdList* cl) {
      add(DummyCommand5::create(cl));
      add(DummyCommand6::create(cl));
      add(DummyCommand7::create(cl));
    }
    static Ptr create(CmdList* cl) { return Ptr(new DummyInstrumenter(cl)); }
};

/* Implementation of BatchCommand5, and 7 that has failed subcommands */
#define FailedBatchCommandDef(NUM, CHILD1, CHILD2)  \
class FailedCommand ## NUM : public BatchCommand { \
  public: \
    typedef dyn_detail::boost::shared_ptr<FailedCommand ## NUM> Ptr; \
    FailedCommand ## NUM (CmdList* cl) { \
      DummyCommand ## CHILD1::Ptr f = DummyCommand ## CHILD1::create(cl); \
      f->setRunReturn(false); \
      add(f);\
      add(DummyCommand ## CHILD2::create(cl)); \
    } \
    static Ptr create(CmdList* cl) { return Ptr(new FailedCommand ## NUM(cl)); } \
};

FailedBatchCommandDef(5, 8, 9)
FailedBatchCommandDef(7, 10, 11)

/* Implementation of FailedInstrumenter */
class FailedInstrumenter : public BatchCommand {
  public:
    typedef dyn_detail::boost::shared_ptr<FailedInstrumenter> Ptr;
    FailedInstrumenter(CmdList* cl) {
      add(FailedCommand5::create(cl));
      add(DummyCommand6::create(cl));
      add(FailedCommand7::create(cl));
    }
    static Ptr create(CmdList* cl) { return Ptr(new FailedInstrumenter(cl)); }
};

class patch4_1_Mutator : public PatchApiMutator {
  virtual test_results_t executeTest();
};

extern "C" DLLEXPORT TestMutator* patch4_1_factory() {
  return new patch4_1_Mutator();
}

test_results_t patch4_1_Mutator::executeTest() {
  // Oracle against this cmd_list!
  CmdList cmd_list;

  // XXX: should be Patcher in the future ...

  /* Test1: run a batch of operations successfully */
  // BatchCommand patcher;
  PatchMgrPtr mgr = makePatchMgr(NULL);
  Patcher patcher(mgr);
  patcher.add(DummyCommand1::create(&cmd_list));
  patcher.add(DummyCommand2::create(&cmd_list));
  patcher.add(DummyCommand3::create(&cmd_list));
  patcher.add(DummyCommand4::create(&cmd_list));
  patcher.add(DummyInstrumenter::create(&cmd_list));
  patcher.commit();

  CmdList oracle;
  oracle.push_back(1);
  oracle.push_back(2);
  oracle.push_back(3);
  oracle.push_back(4);
  oracle.push_back(8);
  oracle.push_back(9);
  oracle.push_back(6);
  oracle.push_back(10);
  oracle.push_back(11);

  if (oracle.size() != 9) {
    logerror("**Failed patch4_1 (transactional semantics)\n");
    logerror("  Expect 9 commands to execute, but in fact %d executed\n", oracle.size());
    return FAILED;
  }
  for (CmdList::iterator i = cmd_list.begin(); i != cmd_list.end(); i++) {
    if (oracle.front() != *i) {
      logerror("**Failed patch4_1 (transactional semantics)\n");
      logerror("  Expect DummyCommand%d to execute, but encountered DummyCommand%d\n", oracle.front(), *i);
      return FAILED;
    }
    oracle.pop_front();
  }

  /* Test2:  */
  cmd_list.clear();
  BatchCommand failed_patcher;
  failed_patcher.add(DummyCommand1::create(&cmd_list));
  failed_patcher.add(DummyCommand2::create(&cmd_list));
  failed_patcher.add(DummyCommand3::create(&cmd_list));
  failed_patcher.add(DummyCommand4::create(&cmd_list));
  failed_patcher.add(FailedInstrumenter::create(&cmd_list));
  failed_patcher.commit();

  if (!cmd_list.empty()) {
    logerror("**Failed patch4_1 (transactional semantics)\n");
    logerror("  Expect cmd_list to be empty, but %d commands not undone: \n", cmd_list.size());
    for (CmdList::iterator i = cmd_list.begin(); i != cmd_list.end(); i++)
      std::cerr << "    DummyCommand" << *i << "\n";
    return FAILED;
  }

  return PASSED;
}
