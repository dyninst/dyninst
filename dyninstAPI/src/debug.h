/*
 * Copyright (c) 1996-2009 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef SHOWERROR_H
#define SHOWERROR_H

#include <string>
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


extern void showErrorCallback(int num, std::string msg);

extern void showInfoCallback(std::string msg);

void BPatch_reportError(int errLevel, int num, const char *str);

extern int dyn_debug_malware;
extern int dyn_debug_signal;
extern int dyn_debug_infrpc;
extern int dyn_debug_startup;
extern int dyn_debug_parsing;
extern int dyn_debug_forkexec;
extern int dyn_debug_proccontrol;
extern int dyn_debug_stackwalk;
extern int dyn_debug_inst;
extern int dyn_debug_reloc;
extern int dyn_debug_springboard;
extern int dyn_debug_sensitivity;
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
extern int dyn_debug_infmalloc;
extern int dyn_stats_instru;
extern int dyn_stats_ptrace;
extern int dyn_stats_parse;
extern int dyn_debug_crash;

extern char *dyn_debug_crash_debugger;

#include "common/h/stats.h"

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
#define relocation_cerr   if (dyn_debug_reloc) cerr
#define springboard_cerr  if (dyn_debug_springboard) cerr
#define malware_cerr      if (dyn_debug_malware) cerr
#define sensitivity_cerr  if (dyn_debug_sensitivity) cerr
#define dyn_unw_cerr      if (dyn_debug_dyn_unw) cerr
#define thread_cerr       if (dyn_debug_thread) cerr
#define liveness_cerr     if (dyn_debug_liveness) cerr
#define infmalloc_cerr    if (dyn_debug_infmalloc) cerr
#define crash_cerr        if (dyn_debug_crash) cerr

// C prototypes for internal debugging functions
extern int mal_printf(const char *format, ...);
extern int signal_printf_int(const char *format, ...);
extern int inferiorrpc_printf_int(const char *format, ...);
extern int startup_printf_int(const char *format, ...);
extern int parsing_printf_int(const char *format, ...);
extern int forkexec_printf_int(const char *format, ...);
extern int proccontrol_printf_int(const char *format, ...);
extern int stackwalk_printf_int(const char *format, ...);
extern int inst_printf_int(const char *format, ...);
extern int reloc_printf_int(const char *format, ...);
extern int dyn_unw_printf_int(const char *format, ...);
extern int dbi_printf_int(const char *format, ...);
extern int mutex_printf_int(const char *format, ...);
extern int mailbox_printf_int(const char *format, ...);
extern int async_printf_int(const char *format, ...);
extern int dwarf_printf_int(const char *format, ...);
extern int thread_printf_int(const char *format, ...);
extern int catchup_printf_int(const char *format, ...);
extern int regalloc_printf_int(const char *format, ...);
extern int ast_printf_int(const char *format, ...);
extern int write_printf_int(const char *format, ...);
extern int liveness_printf_int(const char *format, ...);
extern int infmalloc_printf_int(const char *format, ...);
extern int crash_printf_int(const char *format, ...);

#if defined(__GNUC__)

#define signal_printf(format, args...) do { if (dyn_debug_signal) signal_printf_int(format, ## args); } while(0)
#define inferiorrpc_printf(format, args...) do {if (dyn_debug_infrpc) inferiorrpc_printf_int(format, ## args); } while(0)
#define startup_printf(format, args...) do { if (dyn_debug_startup) startup_printf_int(format, ## args); } while(0)
#define parsing_printf(format, args...) do {if (dyn_debug_parsing) parsing_printf_int(format, ## args); } while(0)
#define forkexec_printf(format, args...) do {if (dyn_debug_forkexec) forkexec_printf_int(format, ## args); } while(0)
#define proccontrol_printf(format, args...) do {if (dyn_debug_proccontrol) proccontrol_printf_int(format, ## args); } while(0)
#define stackwalk_printf(format, args...) do {if (dyn_debug_stackwalk) stackwalk_printf_int(format, ## args); } while(0)
#define inst_printf(format, args...) do {if (dyn_debug_inst) inst_printf_int(format, ## args); } while(0)
#define reloc_printf(format, args...) do {if (dyn_debug_reloc) reloc_printf_int(format, ## args); } while(0)
#define dyn_unw_printf(format, args...) do {if (dyn_debug_dyn_unw) dyn_unw_printf_int(format, ## args); } while(0)
#define dbi_printf(format, args...) do {if (dyn_debug_dyn_dbi) dbi_printf_int(format, ## args); } while(0)
#define mutex_printf(format, args...) do {if (dyn_debug_mutex) mutex_printf_int(format, ## args); } while(0)
#define mailbox_printf(format, args...) do {if (dyn_debug_mailbox) mailbox_printf_int(format, ## args); } while(0)
#define async_printf(format, args...) do {if (dyn_debug_async) async_printf_int(format, ## args); } while(0)
#define dwarf_printf(format, args...) do {if (dyn_debug_dwarf) dwarf_printf_int(format, ## args); } while(0)
#define thread_printf(format, args...) do {if (dyn_debug_thread) thread_printf_int(format, ## args); } while(0)
#define catchup_printf(format, args...) do {if (dyn_debug_catchup) catchup_printf_int(format, ## args); } while(0)
#define regalloc_printf(format, args...) do {if (dyn_debug_regalloc) regalloc_printf_int(format, ## args); } while(0)
#define ast_printf(format, args...) do {if (dyn_debug_ast) ast_printf_int(format, ## args); } while(0)
#define write_printf(format, args...) do {if (dyn_debug_write) write_printf_int(format, ## args); } while(0)
#define liveness_printf(format, args...) do {if (dyn_debug_liveness) liveness_printf_int(format, ## args); } while(0)
#define infmalloc_printf(format, args...) do {if (dyn_debug_infmalloc) infmalloc_printf_int(format, ## args); } while(0)
#define crash_printf(format, args...) do {if (dyn_debug_crash) crash_printf_int(format, ## args); } while(0)

#else
// Non-GCC doesn't have the ## macro
#define signal_printf signal_printf_int
#define inferiorrpc_printf inferiorrpc_printf_int
#define startup_printf startup_printf_int
#define parsing_printf parsing_printf_int
#define forkexec_printf forkexec_printf_int
#define proccontrol_printf proccontrol_printf_int
#define stackwalk_printf stackwalk_printf_int
#define inst_printf inst_printf_int
#define reloc_printf reloc_printf_int
#define dyn_unw_printf dyn_unw_printf_int
#define dbi_printf dbi_printf_int
#define mutex_printf mutex_printf_int
#define mailbox_printf mailbox_printf_int
#define async_printf async_printf_int
#define dwarf_printf dwarf_printf_int
#define thread_printf thread_printf_int
#define catchup_printf catchup_printf_int
#define regalloc_printf regalloc_printf_int
#define ast_printf ast_printf_int
#define write_printf write_printf_int
#define liveness_printf liveness_printf_int
#define infmalloc_printf infmalloc_printf_int
#define crash_printf crash_printf_int


#endif


// And initialization
extern bool init_debug();

extern bool init_stats();
extern bool print_stats();

#endif /* SHOWERROR_H */
