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
#ifndef __PARSE_THAT_H__
#define __PARSE_THAT_H__

#include <string>
#include <vector>
#include "test_lib.h"
//#include "dyninst_comp.h"
#include "util.h"
//  A (hopefully simple) c++ class interface for issuing parseThat commands
//  inside the testsuite

class ParseThat {
	public:

	typedef enum {
		T_None,
		T_Func,
		T_Mod,
		T_Proc
	} InstTransaction;

	typedef enum {
		PL_Module = 0,
		PL_Func = 1,
		PL_CFG = 2
	} ParseLevel;

	typedef enum {
		IL_None = 0,
		IL_FuncEntry = 1,
		IL_FuncExit = 2,
		IL_BasicBlock = 3,
		IL_MemRead = 4,
		IL_MemWrite = 5 
	} InstLevel;

	TESTLIB_DLL_EXPORT ParseThat();
	TESTLIB_DLL_EXPORT ~ParseThat();

	TESTLIB_DLL_EXPORT void pt_output_redirect(std::string fname) {pt_out_name = fname;}
	TESTLIB_DLL_EXPORT void cmd_stdout_redirect(std::string fname) {cmd_stdout_name = fname;}
	TESTLIB_DLL_EXPORT void cmd_stderr_redirect(std::string fname) {cmd_stderr_name = fname;}
	TESTLIB_DLL_EXPORT void func_only(std::string fname) {limit_func = fname;}
	TESTLIB_DLL_EXPORT void module_only(std::string mname) {limit_mod = mname;}
	TESTLIB_DLL_EXPORT void set_transactions(InstTransaction t) {trans = t;}
	TESTLIB_DLL_EXPORT void skip_functions_matching(std::string regex) {skip_funcs = regex;}
	TESTLIB_DLL_EXPORT void skip_modules_matching(std::string regex) {skip_mods = regex;}
	TESTLIB_DLL_EXPORT void suppress_ipc_messages() {suppress_ipc  = true;}
	TESTLIB_DLL_EXPORT void set_verbosity(int v) {verbosity = v;}
	TESTLIB_DLL_EXPORT void dont_fork() {nofork = true;}
	TESTLIB_DLL_EXPORT void measure_usage() {measureUsage = true;}
	TESTLIB_DLL_EXPORT void set_timeout(unsigned secs) {timeout_secs  = secs;}
	TESTLIB_DLL_EXPORT void set_tracing() {do_trace = true;}
	TESTLIB_DLL_EXPORT void set_tracesize(unsigned ntraces) {tracelength  = ntraces;}
	TESTLIB_DLL_EXPORT void print_summary() {print_summary_ = true;}
	TESTLIB_DLL_EXPORT void recursive() {do_recursive = true;}
	TESTLIB_DLL_EXPORT void setParseLevel(ParseLevel pl) {parse_level = pl;}
	TESTLIB_DLL_EXPORT void use_rewriter(std::string dest) {rewrite_filename = dest;}
	TESTLIB_DLL_EXPORT void use_merge() {merge_tramps = true;}
	TESTLIB_DLL_EXPORT void inst_level(InstLevel il) {inst_level_ = il;}
	TESTLIB_DLL_EXPORT void include_libs() {include_libs_ = true;}

	TESTLIB_DLL_EXPORT test_results_t operator()(std::string exec_path, 
			std::vector<std::string> &args);
	TESTLIB_DLL_EXPORT test_results_t operator()(int pid);

	TESTLIB_DLL_EXPORT static test_results_t sys_execute(std::string cmd, 
			std::vector<std::string> &args, 
			std::string stdout_redirect = emptyString, 
			std::string stderr_redirect = emptyString);

	static std::string emptyString;
	private:
	std::string pt_path;
	std::string cmd_stdout_name;
	std::string cmd_stderr_name;
	std::string pt_out_name;
	std::string rewrite_filename;
	InstTransaction trans;
	std::string skip_mods; // regex string
	std::string skip_funcs; // regex string
	std::string limit_mod;  // only use this module
	std::string limit_func; // only use this func

	bool suppress_ipc; //
	bool nofork;
	bool measureUsage;
	unsigned verbosity; // presently can be 0-7
	unsigned int timeout_secs;
	bool do_trace;
	unsigned tracelength;
	bool print_summary_;
	ParseLevel parse_level;
	bool do_recursive;
	bool merge_tramps;
	InstLevel inst_level_;
	bool include_libs_;

	bool setup_args(std::vector<std::string> &);
        void setup_env(std::string plat);
	test_results_t pt_execute(std::vector<std::string> &);

};
#endif
