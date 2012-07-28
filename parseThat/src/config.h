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
#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <iostream>
#include <set>
#include <deque>
#include <string>
#include <unistd.h>
#include <limits.h>		// Needed for PATH_MAX.

using namespace std;

#include "record.h"
#include "reglist.h"
#include "dyninstCompat.h"

#define STRING_MAX 1024

#define HISTORY_RECORD_DIR_DEFAULT ".parseThat"
#define HISTORY_RECORD_INDEX_FILE ".history_index"
#define DEFAULT_HUNT_FILE "hunt_parseThat"

enum RunMode {
    SINGLE_BINARY,
    BATCH_DIRECTORY,
    BATCH_FILE
};

enum TransactionMode {
    TRANS_NONE,
    TRANS_FUNCTION,
    TRANS_MODULE,
    TRANS_PROCESS
};

enum ParseLevel {
    PARSE_MODULE,
    PARSE_FUNC,
    PARSE_CFG,
    PARSE_MAX
};

enum InstLevel {
    INST_NONE,
    INST_FUNC_ENTRY,
    INST_FUNC_EXIT,
    INST_BASIC_BLOCK,
    INST_MEMORY_READ,
    INST_MEMORY_WRITE,
    INST_MAX
};

enum InstType {
    USER_FUNC,
    DEFAULT 
};

enum RunState {
    NORMAL,
    SIGCHLD_WAIT,
    TIME_EXPIRED,
    CHILD_EXITED,
    DETACHED
};

struct Config {
    RunMode runMode;
    TransactionMode transMode;

    // Holds the next target to open.  Generally set by getNextTarget().
    char target[ PATH_MAX ];
    char **argv;
    unsigned argc;

    // For BATCH_FILE runMode.
    const char *config_file;

    // For instrumenting with functions loaded from library
    // format library:function_name
    const char *inst_function;
    InstType instType;

    // Output file string and descriptor.
    const char *output_file;
    FILE *outfd;

    // String to hold the list of arguments for executing rewritten binary
    // in runHunt mode
    char *binary_args;

    // History record directory.
    bool record_enabled;
    char record_dir[ PATH_MAX ];
    record_t curr_rec;

    bool no_fork; // For IPC debugging.  See if we should run without monitor.
    bool recursive; // Descend into children when processing directories.
    bool summary; // Print instrumemnation summary on mutatee exit.
    bool memcpu; // Print memory and CPU usage on mutatee exit.
    bool include_libs; // Process shared libraries as well as program modules.
    bool use_attach; // Attach to running process instead of forking new one.
    int  attach_pid;
    bool use_merge_tramp; // Use merge tramp for instrumentation.
  
    bool use_exe; // Use seperate execitable file instead of the rewritten
                  // library. Used for executing rewritten shared libraries
    bool use_process; // Standard in-core instrumentation or binary edit
    bool write_crashes; // Rewrite with given limits
    bool hunt_crashes; // Keep running until crash is found
    bool hunt_crashed; // True if process crashed
    int hunt_low;
    int hunt_high;
    FILE *hunt_file;

    char writeFilePath[PATH_MAX];
    char exeFilePath[PATH_MAX];

    char *saved_mutatee;

    bool trace_inst;      // Trace mutatee as it is running.
    unsigned int trace_count;
    deque< string > trace_history;
    char pipe_filename[ PATH_MAX ];
    int pipefd;

    // Child PID accounting.
    int pid;
    set< int > grandchildren;

    // Debug print level.
    int verbose;
    bool printipc;

    // Dyninst test level.
    ParseLevel parse_level;
    InstLevel inst_level;

    // Time limit for child execution.
    int time_limit;

    // Reflects current state of execution.
    RunState state;

    // True if parseThat/mutatee exited abnormally
    bool abnormal_exit;

    // Mutators register their dBPatch_thread object here for
    // efficient process clean-up in the face of signals.
    dynHandle *dynlib;

    // Rules for which modules/functions to process.
    reglist mod_rules;
    reglist func_rules;

    // Collection of libraries used for symbols when rewriting 
    // a static binary
    deque<string> symbol_libraries;

    BPatch_remoteHost *remoteHost;
};

extern Config config;

void configInit();
bool getNextTarget();

#endif
