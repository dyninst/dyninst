#include "baseTramp-x86.h"
#include "emitter.h"
#include "codegen.h"

bool baseTramp_x86::generateSaves(codeGen &gen, registerSpace *) {
  return gen.codeEmitter()->emitBTSaves(this, gen);
}

bool baseTramp_x86::generateRestores(codeGen &gen, registerSpace *) {
  return gen.codeEmitter()->emitBTRestores(this, gen);
}
