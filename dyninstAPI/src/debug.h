/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
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
#include "common/src/Pair.h"

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
extern int dyn_debug_trap; // or rather, debug with traps
extern int dyn_debug_signal;
extern int dyn_debug_infrpc;
extern int dyn_debug_startup;
extern int dyn_debug_parsing;
extern int dyn_debug_proccontrol;
extern int dyn_debug_stackwalk;
extern int dyn_debug_inst;
extern int dyn_debug_reloc;
extern int dyn_debug_springboard;
extern int dyn_debug_sensitivity;
extern int dyn_debug_dyn_unw;
extern int dyn_debug_mutex;
extern int dyn_debug_catchup;
extern int dyn_debug_regalloc;
extern int dyn_debug_ast;
extern int dyn_debug_write;
extern int dyn_debug_infmalloc;
extern int dyn_stats_instru;
extern int dyn_stats_ptrace;
extern int dyn_stats_parse;
extern int dyn_debug_crash;
extern int dyn_debug_rtlib;
extern int dyn_debug_disassemble;
extern int dyn_debug_stackmods;

extern char *dyn_debug_crash_debugger;

#include "common/src/stats.h"

extern StatContainer stats_instru;
extern StatContainer stats_ptrace;
extern StatContainer stats_parse;
extern StatContainer stats_codegen;

extern const std::string INST_GENERATE_TIMER;
extern const std::string INST_INSTALL_TIMER;
extern const std::string INST_LINK_TIMER;
extern const std::string INST_REMOVE_TIMER;
extern const std::string INST_GENERATE_COUNTER;
extern const std::string INST_INSTALL_COUNTER;
extern const std::string INST_LINK_COUNTER;
extern const std::string INST_REMOVE_COUNTER;

extern const std::string PTRACE_WRITE_TIMER;
extern const std::string PTRACE_WRITE_COUNTER;
extern const std::string PTRACE_WRITE_AMOUNT;
extern const std::string PTRACE_READ_TIMER;
extern const std::string PTRACE_READ_COUNTER;
extern const std::string PTRACE_READ_AMOUNT;

extern const std::string PARSE_SYMTAB_TIMER;
extern const std::string PARSE_ANALYZE_TIMER;

extern const std::string CODEGEN_AST_TIMER;
extern const std::string CODEGEN_AST_COUNTER;
extern const std::string CODEGEN_REGISTER_TIMER;
extern const std::string CODEGEN_LIVENESS_TIMER;

// C++ prototypes
#define signal_cerr       if (dyn_debug_signal) cerr
#define startup_cerr      if (dyn_debug_startup) cerr
#define parsing_cerr      if (dyn_debug_parsing) cerr
#define proccontrol_cerr  if (dyn_debug_proccontrol) cerr
#define stackwalk_cerr    if (dyn_debug_stackwalk) cerr
#define relocation_cerr   if (dyn_debug_reloc) cerr
#define springboard_cerr  if (dyn_debug_springboard) cerr
#define malware_cerr      if (dyn_debug_malware) cerr
#define trap_cerr         if (dyn_debug_trap) cerr
#define sensitivity_cerr  if (dyn_debug_sensitivity) cerr
#define dyn_unw_cerr      if (dyn_debug_dyn_unw) cerr
#define thread_cerr       if (dyn_debug_thread) cerr
#define infmalloc_cerr    if (dyn_debug_infmalloc) cerr
#define crash_cerr        if (dyn_debug_crash) cerr
#define stackmods_cerr        if (dyn_debug_stackmods) cerr
#define ast_cerr          if (dyn_debug_ast) cerr

// C prototypes for internal debugging functions
extern int mal_printf(const char *format, ...);
extern int startup_printf_int(const char *format, ...);
extern int parsing_printf_int(const char *format, ...);
extern int proccontrol_printf_int(const char *format, ...);
extern int stackwalk_printf_int(const char *format, ...);
extern int inst_printf_int(const char *format, ...);
extern int reloc_printf_int(const char *format, ...);
extern int dyn_unw_printf_int(const char *format, ...);
extern int mutex_printf_int(const char *format, ...);
extern int thread_printf_int(const char *format, ...);
extern int catchup_printf_int(const char *format, ...);
extern int regalloc_printf_int(const char *format, ...);
extern int ast_printf_int(const char *format, ...);
extern int write_printf_int(const char *format, ...);
extern int infmalloc_printf_int(const char *format, ...);
extern int crash_printf_int(const char *format, ...);
extern int stackmods_printf_int(const char *format, ...);

#if defined(__GNUC__)

#define startup_printf(format, args...) do { if (dyn_debug_startup) startup_printf_int(format, ## args); } while(0)
#define parsing_printf(format, args...) do {if (dyn_debug_parsing) parsing_printf_int(format, ## args); } while(0)
#define proccontrol_printf(format, args...) do {if (dyn_debug_proccontrol) proccontrol_printf_int(format, ## args); } while(0)
#define stackwalk_printf(format, args...) do {if (dyn_debug_stackwalk) stackwalk_printf_int(format, ## args); } while(0)
#define inst_printf(format, args...) do {if (dyn_debug_inst) inst_printf_int(format, ## args); } while(0)
#define reloc_printf(format, args...) do {if (dyn_debug_reloc) reloc_printf_int(format, ## args); } while(0)
#define dyn_unw_printf(format, args...) do {if (dyn_debug_dyn_unw) dyn_unw_printf_int(format, ## args); } while(0)
#define mutex_printf(format, args...) do {if (dyn_debug_mutex) mutex_printf_int(format, ## args); } while(0)
#define thread_printf(format, args...) do {if (dyn_debug_thread) thread_printf_int(format, ## args); } while(0)
#define catchup_printf(format, args...) do {if (dyn_debug_catchup) catchup_printf_int(format, ## args); } while(0)
#define regalloc_printf(format, args...) do {if (dyn_debug_regalloc) regalloc_printf_int(format, ## args); } while(0)
#define ast_printf(format, args...) do {if (dyn_debug_ast) ast_printf_int(format, ## args); } while(0)
#define write_printf(format, args...) do {if (dyn_debug_write) write_printf_int(format, ## args); } while(0)
#define infmalloc_printf(format, args...) do {if (dyn_debug_infmalloc) infmalloc_printf_int(format, ## args); } while(0)
#define crash_printf(format, args...) do {if (dyn_debug_crash) crash_printf_int(format, ## args); } while(0)
#define stackmods_printf(format, args...) do {if (dyn_debug_stackmods) stackmods_printf_int(format, ## args); } while(0)

#else
// Non-GCC doesn't have the ## macro
#define startup_printf startup_printf_int
#define parsing_printf parsing_printf_int
#define proccontrol_printf proccontrol_printf_int
#define stackwalk_printf stackwalk_printf_int
#define inst_printf inst_printf_int
#define reloc_printf reloc_printf_int
#define dyn_unw_printf dyn_unw_printf_int
#define mutex_printf mutex_printf_int
#define thread_printf thread_printf_int
#define catchup_printf catchup_printf_int
#define regalloc_printf regalloc_printf_int
#define ast_printf ast_printf_int
#define write_printf write_printf_int
#define infmalloc_printf infmalloc_printf_int
#define crash_printf crash_printf_int
#define stackmods_printf stackmods_printf_int


#endif


// And initialization
extern bool init_debug();

extern bool init_stats();
extern bool print_stats();

#endif /* SHOWERROR_H */
