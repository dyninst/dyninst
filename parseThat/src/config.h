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

    // Output file string and descriptor.
    const char *output_file;
    FILE *outfd;

    // History record directory.
    bool record_enabled;
    char record_dir[ PATH_MAX ];
    record_t curr_rec;

    bool no_fork;         // For IPC debugging.  See if we should run without monitor.
    bool recursive;       // Descend into children when processing directories.
    bool summary;         // Print instrumemnation summary on mutatee exit.
    bool include_libs;    // Parse/Instrument shared libraries as well as program modules.
    bool use_attach;      // Attach to running process instead of forking new one.
    int  attach_pid;
    bool use_merge_tramp; // Use merge tramp for instrumentation.
    bool use_save_world;  // Use save-the-world functionality.
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

    // Dyninst test level.
    ParseLevel parse_level;
    InstLevel inst_level;

    // Time limit for child execution.
    int time_limit;

    // Reflects current state of execution.
    RunState state;

    // Mutators register their dBPatch_thread object here for
    // efficient process clean-up in the face of signals.
    dynHandle *dynlib;

    // Rules for which modules/functions to process.
    reglist mod_rules;
    reglist func_rules;
};

extern Config config;

void configInit();
bool getNextTarget();

#endif
