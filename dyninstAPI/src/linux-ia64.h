#ifndef LINUX_IA64_HDR
#define LINUX_IA64_HDR

#include "arch-ia64.h"
#include "inst-ia64.h"

IA64_bundle generateTrapBundle();

/* Removes a #ifdef in linux.C to include this here. */
#include <asm/ptrace_offsets.h>

struct dyn_saved_regs { int placeholder; };

#endif
