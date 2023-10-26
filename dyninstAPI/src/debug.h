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
#include "compiler_annotations.h"

#define BPFATAL(x) bpfatal_lf(__FILE__, __LINE__, x)
extern void logLine(const char *line);
extern void statusLine(const char *line, bool force = false);
extern char errorLine[];
extern int bpfatal(const char *format, ...) DYNINST_PRINTF_ANNOTATION(1, 2);
extern int bperr(const char *format, ...) DYNINST_PRINTF_ANNOTATION(1, 2);
extern int bpwarn(const char *format, ...) DYNINST_PRINTF_ANNOTATION(1, 2);
extern int bpinfo(const char *format, ...) DYNINST_PRINTF_ANNOTATION(1, 2);

extern int bpfatal_lf(const char *__file__, unsigned int __line__, const char *format, ...)
	DYNINST_PRINTF_ANNOTATION(3, 4);


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
#define debug_sys_cerr(debug_sys)  if (dyn_debug_##debug_sys) cerr

#define signal_cerr       debug_sys_cerr(signal)
#define startup_cerr      debug_sys_cerr(startup)
#define parsing_cerr      debug_sys_cerr(parsing)
#define proccontrol_cerr  debug_sys_cerr(proccontrol)
#define stackwalk_cerr    debug_sys_cerr(stackwalk)
#define relocation_cerr   debug_sys_cerr(reloc)
#define springboard_cerr  debug_sys_cerr(springboard)
#define malware_cerr      debug_sys_cerr(malware)
#define trap_cerr         debug_sys_cerr(trap)
#define sensitivity_cerr  debug_sys_cerr(sensitivity)
#define dyn_unw_cerr      debug_sys_cerr(dyn_unw)
#define thread_cerr       debug_sys_cerr(thread)
#define infmalloc_cerr    debug_sys_cerr(infmalloc)
#define crash_cerr        debug_sys_cerr(crash)
#define stackmods_cerr    debug_sys_cerr(stackmods)
#define ast_cerr          debug_sys_cerr(ast)

// C prototypes for internal debugging functions
#define DECLARE_PRINTF_FUNC(f) extern int f(const char *format, ...) DYNINST_PRINTF_ANNOTATION(1, 2)

DECLARE_PRINTF_FUNC(mal_printf);
DECLARE_PRINTF_FUNC(startup_printf_int);
DECLARE_PRINTF_FUNC(parsing_printf_int);
DECLARE_PRINTF_FUNC(proccontrol_printf_int);
DECLARE_PRINTF_FUNC(stackwalk_printf_int);
DECLARE_PRINTF_FUNC(inst_printf_int);
DECLARE_PRINTF_FUNC(reloc_printf_int);
DECLARE_PRINTF_FUNC(dyn_unw_printf_int);
DECLARE_PRINTF_FUNC(mutex_printf_int);
DECLARE_PRINTF_FUNC(thread_printf_int);
DECLARE_PRINTF_FUNC(catchup_printf_int);
DECLARE_PRINTF_FUNC(regalloc_printf_int);
DECLARE_PRINTF_FUNC(ast_printf_int);
DECLARE_PRINTF_FUNC(write_printf_int);
DECLARE_PRINTF_FUNC(infmalloc_printf_int);
DECLARE_PRINTF_FUNC(crash_printf_int);
DECLARE_PRINTF_FUNC(stackmods_printf_int);


#define debug_sys_printf(debug_sys, ...) do { if (dyn_debug_##debug_sys) debug_sys##_printf_int(__VA_ARGS__); } while(0)

#define startup_printf(...)     debug_sys_printf(startup, __VA_ARGS__)
#define parsing_printf(...)     debug_sys_printf(parsing, __VA_ARGS__)
#define proccontrol_printf(...) debug_sys_printf(proccontrol, __VA_ARGS__)
#define stackwalk_printf(...)   debug_sys_printf(stackwalk, __VA_ARGS__)
#define inst_printf(...)        debug_sys_printf(inst, __VA_ARGS__)
#define reloc_printf(...)       debug_sys_printf(reloc, __VA_ARGS__)
#define dyn_unw_printf(...)     debug_sys_printf(dyn_unw, __VA_ARGS__)
#define mutex_printf(...)       debug_sys_printf(mutex, __VA_ARGS__)
#define thread_printf(...)      debug_sys_printf(thread, __VA_ARGS__)
#define catchup_printf(...)     debug_sys_printf(catchup, __VA_ARGS__)
#define regalloc_printf(...)    debug_sys_printf(regalloc, __VA_ARGS__)
#define ast_printf(...)         debug_sys_printf(ast, __VA_ARGS__)
#define write_printf(...)       debug_sys_printf(write, __VA_ARGS__)
#define infmalloc_printf(...)   debug_sys_printf(infmalloc, __VA_ARGS__)
#define crash_printf(...)       debug_sys_printf(crash, __VA_ARGS__)
#define stackmods_printf(...)   debug_sys_printf(stackmods, __VA_ARGS__)


// And initialization
extern bool init_debug();

extern bool init_stats();
extern bool print_stats();

#endif /* SHOWERROR_H */
