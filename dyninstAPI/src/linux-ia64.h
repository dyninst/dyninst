/* -*- Mode: C; indent-tabs-mode: true; tab-width: 4 -*- */

#ifndef LINUX_IA64_HDR
#define LINUX_IA64_HDR

#include "arch-ia64.h"
#include "inst-ia64.h"
#include <linux/ptrace.h>

IA64_bundle generateTrapBundle();

#if defined( MUTATOR_SAVES_REGISTERS )
struct dyn_saved_regs {
    struct pt_regs pt;
    struct switch_stack ss;
};
#else
struct dyn_saved_regs {
	Address pc;
	bool restorePredicateRegistersFromStack;
	bool pcMayHaveRewound;
	};
#endif

/* Removes a #ifdef in linux.C to include this here. */
#include <asm/ptrace_offsets.h>

#endif
