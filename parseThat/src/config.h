#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <iostream>
#include <set>
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
    CHILD_EXITED
};

struct Config {
    RunMode runMode;
    TransactionMode transMode;

    // Holds the next target to open.  Generally set by getNextTarget().
    char target[ PATH_MAX ];
    char **argv;
    unsigned argc;

    // For BATCH_FILE mode.
    const char *config_file;

    // Output file string and descriptor.
    const char *output_file;
    FILE *outfd;

    // History record directory.
    bool record_enabled;
    char record_dir[ PATH_MAX ];
    record_t curr_rec;

    // For IPC debugging.  See if we should run without monitor.
    bool no_fork;

    // Descend into children when processing directories.
    bool recursive;

    // Print instrumemnation summary on mutatee exit.
    bool summary;

    // Parse/Instrument shared libraries as well as program modules.
    bool include_libs;

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
