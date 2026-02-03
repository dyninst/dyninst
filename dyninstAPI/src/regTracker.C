#include "regTracker.h"
#include "codegen.h"
#include "debug.h"
#include "ast.h"
#include "registerSpace.h"

#include <cassert>

void regTracker_t::addKeptRegister(codeGen &gen, AstNode *n,
                                   Dyninst::Register reg) {
  assert(n);
  if (tracker.find(n) != tracker.end()) {
    assert(tracker[n].keptRegister == reg);
    return;
  }
  commonExpressionTracker t;
  t.keptRegister = reg;
  t.keptLevel = condLevel;
  tracker[n] = t;
  gen.rs()->markKeptRegister(reg);
}

void regTracker_t::removeKeptRegister(codeGen &gen, AstNode *n) {
  auto iter = tracker.find(n);
  if (iter == tracker.end())
    return;

  gen.rs()->unKeepRegister(iter->second.keptRegister);
  tracker.erase(iter);
}

Dyninst::Register regTracker_t::hasKeptRegister(AstNode *n) {
  auto iter = tracker.find(n);
  if (iter == tracker.end())
    return Dyninst::Null_Register;
  else
    return iter->second.keptRegister;
}

// Find if the given register is "owned" by an AST node,
// and if so nuke it.

bool regTracker_t::stealKeptRegister(Dyninst::Register r) {
  ast_printf("STEALING kept register %u for someone else\n", r.getId());
  for (auto iter = tracker.begin(); iter != tracker.end(); ++iter) {
    if (iter->second.keptRegister == r) {
      tracker.erase(iter);
      return true;
    }
  }
  fprintf(stderr, "Odd - couldn't find kept register %u\n", r.getId());
  return true;
}

void regTracker_t::reset() {
  condLevel = 0;
  tracker.clear();
}

void regTracker_t::increaseConditionalLevel() {
  condLevel++;
  ast_printf("Entering conditional branch, level now %d\n", condLevel);
}

void regTracker_t::decreaseAndClean(codeGen &) {

  assert(condLevel > 0);

  ast_printf("Exiting from conditional branch, level currently %d\n",
             condLevel);

  std::vector<AstNode *> delete_list;

  for (auto iter = tracker.begin(); iter != tracker.end();) {
    if (iter->second.keptLevel == condLevel) {
      iter = tracker.erase(iter);
    } else {
      ++iter;
    }
  }

  condLevel--;
}

void regTracker_t::debugPrint() {
  if (!dyn_debug_ast)
    return;

  fprintf(stderr, "==== Begin debug dump of register tracker ====\n");

  fprintf(stderr, "Condition level: %d\n", condLevel);

  for (auto iter = tracker.begin(); iter != tracker.end(); ++iter) {
    fprintf(stderr, "AstNode %p: register %u, condition level %d\n",
            (void *)iter->first, iter->second.keptRegister.getId(),
            iter->second.keptLevel);
  }
  fprintf(stderr, "==== End debug dump of register tracker ====\n");
}
