#ifndef DYNINST_DYNINSTAPI_AST_HELPERS_H
#define DYNINST_DYNINSTAPI_AST_HELPERS_H

#include "codegen.h"
#include "registerSpace.h"

#include <cstdint>
#include <cstdio>

#if defined(DYNINST_CODEGEN_ARCH_POWER)
#include "emit-power.h"
#include "inst-power.h"
#elif defined(DYNINST_CODEGEN_ARCH_I386) || defined(DYNINST_CODEGEN_ARCH_X86_64)
#include "emit-x86.h"
#include "inst-x86.h"
#elif defined(DYNINST_CODEGEN_ARCH_AARCH64)
#include "emit-aarch64.h"
#include "inst-aarch64.h"
#elif defined(DYNINST_CODEGEN_ARCH_AMDGPU_GFX908)
#include "emit-amdgpu.h"
#else
#error "Unknown architecture in ast.h"
#endif

#define RETURN_KEPT_REG(r)                                                                         \
  do {                                                                                             \
    if(previousComputationValid(r, gen)) {                                                         \
      decUseCount(gen);                                                                            \
      gen.rs()->incRefCount(r);                                                                    \
      return true;                                                                                 \
    }                                                                                              \
  } while(0)

#define ERROR_RETURN                                                                               \
  do {                                                                                             \
    fprintf(stderr, "[%s:%d] ERROR: failure to generate operand\n", __FILE__, __LINE__);           \
    return false;                                                                                  \
  } while(0)

#define REGISTER_CHECK(r)                                                                          \
  do {                                                                                             \
    if((r) == Dyninst::Null_Register) {                                                            \
      fprintf(stderr, "[%s: %d] ERROR: returned register invalid\n", __FILE__, __LINE__);          \
      return false;                                                                                \
    }                                                                                              \
  } while(0)

extern bool doNotOverflow(int64_t value);

#endif
