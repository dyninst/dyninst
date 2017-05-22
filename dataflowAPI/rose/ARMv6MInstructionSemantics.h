#ifndef ROSE_ARMV6MINSTRUCTIONSEMANTICS_H
#define ROSE_ARMV6MINSTRUCTIONSEMANTICS_H

//#include "rose.h"
#include "semanticsModule.h"
#include <cassert>
#include <cstdio>
#include <iostream>
#include "integerOps.h"

#include "SgAsmExpression.h"
#include "SgAsmARMv6MInstruction.h"
#include "conversions.h"


template <typename Policy, template <size_t> class WordType>
struct ARMv6MInstructionSemantics {
  Policy& policy;

  ARMv6MInstructionSemantics(Policy& policy): policy(policy) {}


  void translate(SgAsmARMv6MInstruction* insn) {
  }

  void processInstruction(SgAsmARMv6MInstruction* insn) {
    ROSE_ASSERT (insn);
    policy.startInstruction(insn);
    translate(insn);
    policy.finishInstruction(insn);
  }


};

#endif
