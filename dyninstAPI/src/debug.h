/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

#ifndef SHOWERROR_H
#define SHOWERROR_H

#include "common/h/String.h"
#include "common/h/Pair.h"

#define BPFATAL(x) bpfatal_lf(__FILE__, __LINE__, x)
extern void logLine(const char *line);
extern void statusLine(const char *line, bool force = false);
extern char errorLine[];
extern int bpfatal(const char *format, ...);
extern int bperr(const char *format, ...);
extern int bpwarn(const char *format, ...);
extern int bpinfo(const char *format, ...);

extern int bpfatal_lf(const char *__file__, unsigned int __line__, const char *format, ...);


extern void showErrorCallback(int num, pdstring msg);

extern void showInfoCallback(pdstring msg);

void BPatch_reportError(int errLevel, int num, const char *str);

extern int dyn_debug_signal;
extern int dyn_debug_infrpc;
extern int dyn_debug_startup;
extern int dyn_debug_parsing;
extern int dyn_debug_forkexec;
extern int dyn_debug_proccontrol;
extern int dyn_debug_stackwalk;
extern int dyn_debug_inst;
extern int dyn_debug_reloc;
extern int dyn_debug_dyn_unw;
extern int dyn_debug_dyn_dbi;
extern int dyn_debug_mutex;
extern int dyn_debug_mailbox;
extern int dyn_debug_async;
extern int dyn_debug_dwarf;
extern int dyn_debug_thread;
extern int dyn_debug_catchup;
extern int dyn_debug_regalloc;
extern int dyn_debug_ast;
extern int dyn_debug_write;
extern int dyn_debug_liveness;
extern int dyn_stats_instru;
extern int dyn_stats_ptrace;
extern int dyn_stats_parse;

#include "stats.h"

extern StatContainer stats_instru;
extern StatContainer stats_ptrace;
extern StatContainer stats_parse;
extern StatContainer stats_codegen;

#define INST_GENERATE_TIMER "instGenerateTimer"
#define INST_INSTALL_TIMER "instInstallTimer"
#define INST_LINK_TIMER "instLinkTimer"
#define INST_REMOVE_TIMER "instRemoveTimer"
#define INST_GENERATE_COUNTER "instGenerateCounter"
#define INST_INSTALL_COUNTER "instInstallCounter"
#define INST_LINK_COUNTER "instLinkCounter"
#define INST_REMOVE_COUNTER "instRemoveCounter"

#define PTRACE_WRITE_TIMER "ptraceWriteTimer"
#define PTRACE_WRITE_COUNTER "ptraceWriteCounter"
#define PTRACE_WRITE_AMOUNT "ptraceWriteAmountCounter"
#define PTRACE_READ_TIMER "ptraceReadTimer"
#define PTRACE_READ_COUNTER "ptraceReadCounter"
#define PTRACE_READ_AMOUNT "ptraceReadAmountCounter"

#define PARSE_SYMTAB_TIMER "parseSymtabTimer"
#define PARSE_ANALYZE_TIMER "parseAnalyzeTimer"

#define CODEGEN_AST_TIMER "codegenAstTimer"
#define CODEGEN_AST_COUNTER "codegenAstCounter"
#define CODEGEN_REGISTER_TIMER "codegenRegisterTimer"
#define CODEGEN_LIVENESS_TIMER "codegenLivenessTimer"

// C++ prototypes
#define signal_cerr       if (dyn_debug_signal) cerr
#define inferiorrpc_cerr  if (dyn_debug_infrpc) cerr
#define startup_cerr      if (dyn_debug_startup) cerr
#define parsing_cerr      if (dyn_debug_parsing) cerr
#define forkexec_cerr     if (dyn_debug_forkexec) cerr
#define proccontrol_cerr  if (dyn_debug_proccontrol) cerr
#define stackwalk_cerr    if (dyn_debug_stackwalk) cerr
#define relocation_cerr   if (dyn_debug_relocation) cerr
#define dyn_unw_cerr      if (dyn_debug_dyn_unw) cerr
#define thread_cerr      if (dyn_debug_thread) cerr
#define liveness_cerr    if (dyn_debug_liveness) cerr

// C prototypes
extern int signal_printf(const char *format, ...);
extern int inferiorrpc_printf(const char *format, ...);
extern int startup_printf(const char *format, ...);
extern int parsing_printf(const char *format, ...);
extern int forkexec_printf(const char *format, ...);
extern int proccontrol_printf(const char *format, ...);
extern int stackwalk_printf(const char *format, ...);
extern int inst_printf(const char *format, ...);
extern int reloc_printf(const char *format, ...);
extern int dyn_unw_printf(const char *format, ...);
extern int dbi_printf(const char *format, ...);
extern int mutex_printf(const char *format, ...);
extern int mailbox_printf(const char *format, ...);
extern int async_printf(const char *format, ...);
extern int dwarf_printf(const char *format, ...);
extern int thread_printf(const char *format, ...);
extern int catchup_printf(const char *format, ...);
extern int regalloc_printf(const char *format, ...);
extern int ast_printf(const char *format, ...);
extern int write_printf(const char *format, ...);
extern int liveness_printf(const char *format, ...);

// And initialization
extern bool init_debug();

extern bool init_stats();
extern bool print_stats();

#endif /* SHOWERROR_H */
