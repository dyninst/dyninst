#include <iostream>
#include <string>
#include <cctype>
#include <regex.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/stat.h>		// Needed for stat() call in parseArgs().
using namespace std;

#include "ipc.h"
#include "config.h"
#include "dyninstCore.h"
#include "record.h"
#include "reglist.h"

void parseArgs(int argc, char **argv);
void usage(const char *);

int main(int argc, char **argv)
{
    int bannerLen;
    parseArgs(argc, argv);

    while (getNextTarget()) {
	int pipefd[2];

	if (config.no_fork) {
	    return launch_mutator();
	}

	if (pipe(pipefd) != 0) {
	    dlog(ERR, "Error on pipe(): %s\n", strerror(errno));
	    exit(-2);
	}

	config.grandchildren.clear();
	config.state = NORMAL;

	dlog(INFO, "[ Processing %s ] %n", config.target, &bannerLen);
	while (++bannerLen < 80) dlog(INFO, "=");
	dlog(INFO, "\n");
	fflush(config.outfd);

	config.pid = fork();
	if (config.pid > 0) {
	    /**************************************************************
	     * Parent Case
	     */
	    close(pipefd[1]); // Close (historically) write side of pipe.

	    // Register target in history records.
	    if (config.record_enabled) {
		if (!record_create(&config.curr_rec, config.target, config.argc, config.argv))
		    dlog(WARN, "Error creating history record.  No history will be recorded for %s\n", config.target);

		else if (!record_search(&config.curr_rec))
		    record_update(&config.curr_rec);
	    }

	    int i = 0;
	    errno = 0;
	    FILE *infd = NULL;

	    // Convert raw socket to stream based FILE *.
	    while (infd == NULL) {
		infd = fdopen(pipefd[0], "r");
		if (infd == NULL && errno != EINTR) {
		    dlog(ERR, "Error on fdopen() in child: %s\n", strerror(errno));
		    dlog(ERR, "*** Child exiting.\n");
		    exit(-2);
		}

		if (++i > 10) {
		    dlog(ERR, "fdopen() has been interrupted 10 times.\n");
		    dlog(ERR, "*** Parent exiting.\n");
		    exit(-2);
		}
		errno = 0;
	    }

	    // Only parent should have signal handlers modified.
	    setSigHandlers();

	    launch_monitor(infd);

	    // Reset signal handers so next child won't be affected.
	    resetSigHandlers();

	    // Clean up any known processes we created.
	    cleanupProcesses();

	    fclose(infd);

	    fprintf(config.outfd, "[ Done processing %s ] %n", config.target, &bannerLen);
	    while (++bannerLen < 80) fprintf(config.outfd, "-");
	    fprintf(config.outfd, "\n");

	} else if (config.pid == 0) {
	    /**************************************************************
	     * Child Case
	     */

	    // Register to catch SIGINT
	    setSigHandlers();

	    // Start new process group.  Makes forced process shutdown easier.
	    setpgid(0, 0);

	    close(pipefd[0]); // Close (historically) read side of pipe.

	    int i = 0;
	    errno = 0;

	    // Leave stdout open for mutatee, but if an output file was specified by user,
	    // don't keep multiple descriptors open for it.
	    if (config.outfd != stdout) fclose(config.outfd);
	    config.outfd = NULL;

	    // Convert raw socket to stream based FILE *.
	    while (config.outfd == NULL) {
		config.outfd = fdopen(pipefd[1], "w");
		if (config.outfd == NULL && errno != EINTR) {
		    fprintf(stderr, "Error on fdopen() in mutator: %s\n", strerror(errno));
		    fprintf(stderr, "*** Mutator exiting.\n");
		    exit(-2);
		}

		if (++i > 10) {
		    fprintf(stderr, "fdopen() has been interrupted 10 times.\n");
		    fprintf(stderr, "*** Mutator exiting.\n");
		    exit(-2);
		}
		errno = 0;
	    }
	    return( launch_mutator() );

	} else {
	    /* Fork Error Case */
	    dlog(ERR, "Error on fork(): %s\n", strerror(errno));
	    exit(-2);
	}
    }
    dlog(INFO, "Analysis complete.\n");

    return 0;
}

void parseArgs(int argc, char **argv)
{
    bool needShift;
    char *arg;

    if (argc < 2) {
	usage(argv[0]);
	exit(-1);
    }

    configInit();

    int i = 0;
    while (*argv[++i] == '-') {
	const char *ptr = argv[i];
	while (*(++ptr)) {
	    switch (*ptr) {
	    case 'c':
		if (++i < argc) {
		    config.config_file = argv[i];
		    config.runMode = BATCH_FILE;

		} else {
		    fprintf(stderr, "-c flag requires an argument.\n");
		    usage(argv[0]);
		    exit(-1);
		}
		break;

	    case 'h':
		config.record_enabled = 1;
		break;

	    case 'i':
		if (isdigit(*(ptr + 1))) {
		    config.inst_level = (InstLevel)strtol(++ptr, &arg, 0);
                    ptr = arg - 1;

		} else if (++i < argc)
		    config.inst_level = (InstLevel)atoi(argv[i]);

		else {
		    fprintf(stderr, "-i flag requires an argument.\n");
		    usage(argv[0]);
		    exit(-1);
		}

		if (config.inst_level == 0 && errno == EINVAL) {
		    fprintf(stderr, "Invalid argument to -i flag: '%s'\n", argv[i]);
		    usage(argv[0]);
		    exit(-1);
		}

		if (config.inst_level < 0 || config.inst_level >= INST_MAX) {
		    fprintf(stderr, "Invalid argument to -i flag.  Valid range is 0 through %d\n", INST_MAX - 1);
		    usage(argv[0]);
		    exit(-1);
		}
		break;

	    case 'I':
		config.include_libs = true;
		break;

	    case 'o':
		if (++i < argc) {
		    config.output_file = argv[i];
		    config.outfd = NULL;

		} else {
		    fprintf(stderr, "-o flag requires an argument.\n");
		    usage(argv[0]);
		    exit(-1);
		}
		break;

	    case 'p':
		if (isdigit(*(ptr + 1))) {
		    config.parse_level = (ParseLevel)strtol(++ptr, &arg, 0);
                    ptr = arg - 1;

		} else if (++i < argc)
		    config.parse_level = (ParseLevel)atoi(argv[i]);

		else {
		    fprintf(stderr, "-p flag requires an argument.\n");
		    usage(argv[0]);
		    exit(-1);
		}

		if (config.parse_level == 0 && errno == EINVAL) {
		    fprintf(stderr, "Invalid argument to -p flag: '%s'\n", argv[i]);
		    usage(argv[0]);
		    exit(-1);
		}

		if (config.parse_level < 0 || config.parse_level >= PARSE_MAX) {
		    fprintf(stderr, "Invalid argument to -p flag.  Valid range is 0 through %d\n", PARSE_MAX - 1);
		    usage(argv[0]);
		    exit(-1);
		}
		break;

	    case 'r':
		config.recursive = true;
		break;

	    case 's':
		config.summary = true;
		break;

	    case 'S':
		config.no_fork = true;
		break;

	    case 't':
		errno = 0;
		if (isdigit(*(ptr + 1))) {
		    config.time_limit = strtol(++ptr, &arg, 0);
                    ptr = arg - 1;

		} else if (++i < argc)
		    config.time_limit = atoi(argv[i]);

		else {
		    fprintf(stderr, "-t flag requires an argument.\n");
		    usage(argv[0]);
		    exit(-1);
		}

		if (config.time_limit == 0 && errno == EINVAL) {
		    fprintf(stderr, "Invalid argument to -t flag: '%s'\n", argv[i]);
		    usage(argv[0]);
		    exit(-1);
		}
		break;

	    case 'v':
		if (isdigit(*(ptr + 1))) {
		    config.verbose = strtol(++ptr, &arg, 0);
		    ptr = arg - 1;

		} else if (i+1 < argc && isdigit(*argv[i+1]))
		    config.verbose = atoi(argv[++i]);

		else
		    ++config.verbose;

		if (config.verbose < 0 || config.verbose >= VERBOSE_MAX) {
		    fprintf(stderr, "Invalid -v flag.  Valid range is 0 through %d\n", VERBOSE_MAX - 1);
		}

		break;

	    case 'q':
		--config.verbose;

		if (config.verbose < 0 || config.verbose >= VERBOSE_MAX) {
		    fprintf(stderr, "Invalid -v flag.  Valid range is 0 through %d\n", VERBOSE_MAX - 1);
		}
		break;

	    case '-':
		needShift = false;
		arg = strchr(ptr, '=');
		if (arg) *(arg++) = '\0';
		else if (i+1 < argc) {
		    arg = argv[i+1];
		    needShift = true;
		}

		if (strcmp(ptr, "-include-libs") == 0) {
		    config.include_libs = true;

		} else if (strcmp(ptr, "-use-transactions") == 0) {
		    if (!arg) {
			fprintf(stderr, "No argument supplied to --use-transactions.  Using per-function as default.\n");
			config.transMode = TRANS_FUNCTION;
		    }

		    for (unsigned j = 0; arg[j]; ++j) arg[j] = tolower(arg[j]);
		    if (strcmp(arg, "func") == 0) config.transMode = TRANS_FUNCTION;
		    else if (strcmp(arg, "mod") == 0)  config.transMode = TRANS_MODULE;
		    else if (strcmp(arg, "proc") == 0) config.transMode = TRANS_PROCESS;
		    else {
			fprintf(stderr, "Invalid argument supplied to --use-transactions.  Valid arguments are func, mod, or proc.\n");
			exit(-1);
		    }

		} else if (strcmp(ptr, "-skip-mod") == 0) {
		    if (!arg) {
			fprintf(stderr, "--skip-mod requires a regular expression argument.\n");
			usage(argv[0]);
			exit(-1);
		    }
		    if (!config.mod_rules.insert(arg, RULE_SKIP))
                        exit(-1);
		    if (needShift) ++i;

		} else if (strcmp(ptr, "-skip-func") == 0) {
		    if (!arg) {
			fprintf(stderr, "--skip-func requires a regular expression argument.\n");
			usage(argv[0]);
			exit(-1);
		    }
		    if (!config.func_rules.insert(arg, RULE_SKIP))
			exit(-1);
		    if (needShift) ++i;

		} else if (strcmp(ptr, "-only-mod") == 0) {
		    if (!arg) {
			fprintf(stderr, "--only-mod requires a regular expression argument.\n");
			usage(argv[0]);
			exit(-1);
		    }
		    if (!config.mod_rules.insert(arg, RULE_ONLY))
			exit(-1);
		    if (needShift) ++i;

		} else if (strcmp(ptr, "-only-func") == 0) {
		    if (!arg) {
			fprintf(stderr, "--only-func requires a regular expression argument.\n");
			usage(argv[0]);
			exit(-1);
		    }
		    if (!config.func_rules.insert(arg, RULE_ONLY))
			exit(-1);
		    if (needShift) ++i;

		} else if (strcmp(ptr, "-summary") == 0) {
		    config.summary = true;

		} else {
		    fprintf(stderr, "Unknown parameter: %s\n", ptr);
		    usage(argv[0]);
		    exit(-1);
		}

		ptr += strlen(ptr) - 1;
		break;

	    default:
		fprintf(stderr, "Unknown parameter: -%c\n", *ptr);
		usage(argv[0]);
		exit(-1);
	    }
	}
    }

    // Prepare child arguments
    if (i < argc) {
	strncpy(config.target, argv[i], sizeof(config.target));
	config.argv = argv + i - 1;
	config.argc = argc - i;

	// Shift argv array down one to make room for NULL
	while (i < argc) {
	    argv[i - 1] = argv[i];
	    ++i;
	}
	argv[argc - 1] = NULL;

	if (strrchr(config.target, '/'))
	    config.argv[0] = strrchr(config.target, '/') + 1;

    } else {
	config.argc = 0;
	argv[argc - 1] = NULL;
	config.argv = argv + argc - 1;
    }



    // Argument integrity checks
    if (config.target[0] == '\0') {
	fprintf(stderr, "No directory or program specified.\n");
	usage(argv[0]);
	exit(-1);
    }

    // Determine run mode, if necessary.
    if (config.runMode != BATCH_FILE) {
	i = 0;
	errno = EINTR;
	struct stat file_stat;
	while (errno == EINTR) {
	    errno = 0;
	    if (stat(config.target, &file_stat) != 0 && errno != EINTR) {
		fprintf(stderr, "Could not stat %s: %s\n", config.target, strerror(errno));
		usage(argv[0]);
		exit(-1);
	    }
	    if (++i > 10) {
		fprintf(stderr, "stat() for %s has been interrupted 10 times.  Try again later.\n", config.target);
		exit(-1);
	    }
	}

	if ((file_stat.st_mode & S_IFMT) == S_IFDIR) {
	    // User specified a directory.
	    config.runMode = BATCH_DIRECTORY;

	} else if ((file_stat.st_mode & S_IFMT) == S_IFREG) {
	    // User specified a file.
	    config.runMode = SINGLE_BINARY;

	} else {
	    fprintf(stderr, "%s is not a directory or regular file. 0x%x\n", config.target, (file_stat.st_mode & S_IFMT));
	    usage(argv[0]);
	    exit(-1);
	}
    }

    // Open output file descriptor.
    if (config.output_file) {
	i = 0;
	errno = 0;
	config.outfd = NULL;

	while (config.outfd == NULL) {
	    config.outfd = fopen(config.output_file, "w");
	    if (config.outfd == NULL && errno != EINTR) {
		fprintf(stderr, "Could not open %s for writing: %s\n", config.output_file, strerror(errno));
		usage(argv[0]);
		exit(-1);
	    }
	    if (++i > 10) {
		fprintf(stderr, "fopen() for %s has been interrupted 10 times.  Try again later.\n", config.output_file);
		exit(-1);
	    }
	    errno = 0;
	}
    }

    if (config.runMode == BATCH_FILE && config.target[0] != '\0') {
	fprintf(stderr, "Warning: Batch file specified.  Ignoring command line argument %s.\n", config.target);
    }

    // Create history record directory, if needed.
    if (config.record_enabled) {
	i = 0;
	errno = EINTR;
	struct stat dir_stat;
	while (errno == EINTR) {
	    errno = 0;
	    stat(config.record_dir, &dir_stat);
	    if (errno && errno == ENOENT) {
		mkdir(config.record_dir, S_IRUSR | S_IWUSR | S_IXUSR);
		break;

	    } else if (errno && errno != EINTR) {
		fprintf(stderr, "Could not stat %s: %s\n", config.record_dir, strerror(errno));
		usage(argv[0]);
		exit(-1);

	    } else {
		if (!S_ISDIR(dir_stat.st_mode)) {
		    fprintf(stderr, "%s exists, but is not a directory.  History record disabled.\n", config.record_dir);
		    config.record_enabled = 0;
		    break;
		}
	    }

	    if (++i > 10) {
		fprintf(stderr, "stat() for %s has been interrupted 10 times.  Try again later.\n", config.record_dir);
		exit(-1);
	    }
	}
    }

    // Enforce parse/instrumentation relationships
    if (config.inst_level >= INST_BASIC_BLOCK && config.parse_level < PARSE_CFG)
	config.parse_level = PARSE_CFG;

    if (config.inst_level >= INST_FUNC_ENTRY && config.parse_level < PARSE_FUNC)
	config.parse_level = PARSE_FUNC;

    // Skip .so files unless --include-libs flag is specified.
    if (!config.include_libs) {
	config.mod_rules.insert("\\.so", RULE_SKIP, false);
	config.mod_rules.insert("\\.a$", RULE_SKIP, false);
    }

    if (config.no_fork && config.runMode != SINGLE_BINARY) {
	fprintf(stderr, "Single (No-Fork) only compatible with a single binary mutatee.\n");
	exit(-1);
    }
}

void usage(const char *progname)
{
    fprintf(stderr, "Usage: %s [options] <dir | prog> [prog_args]\n\n", progname);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -c <filename>\n");
    fprintf(stderr, "    Specifies batch mode configuration file.\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "  -h\n");
    fprintf(stderr, "    Enables history record logging.  Log files will be placed in:\n");
    fprintf(stderr, "      %s/%s\n", getenv("HOME"), HISTORY_RECORD_DIR_DEFAULT);
    fprintf(stderr, "\n");
    fprintf(stderr, "  -i <int>\n");
    fprintf(stderr, "    Instrumentation level.  Valid parameters range from 0 to %d, where:\n", INST_MAX - 1);
    fprintf(stderr, "      0 = No instrumentation\n");
    fprintf(stderr, "      1 = Function entry instrumentation\n");
    fprintf(stderr, "      2 = Function exit instrumentation\n");
    fprintf(stderr, "      3 = Basic block instrumentation\n");
    fprintf(stderr, "      4 = Memory read instrumentation\n");
    fprintf(stderr, "      5 = Memory write instrumentation\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "  -I, --include-libs\n");
    fprintf(stderr, "    Include shared libraries as targets for parsing and instrumentation.\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "  -o <filename>\n");
    fprintf(stderr, "    Send all output from monitor to specified file.\n");
    fprintf(stderr, "    NOTE: Mutator and mutatee output will not be sent to file.\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "  -p <int>\n");
    fprintf(stderr, "    Parse level.  Valid parameters range from 0 to %d, where:\n", PARSE_MAX - 1);
    fprintf(stderr, "      0 = Parse for module data\n");
    fprintf(stderr, "      1 = Parse for function data\n");
    fprintf(stderr, "      2 = Parse for control flow graph data\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "  -r\n");
    fprintf(stderr, "    Descend into subdirectories when processing directories.\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "  -s, --summary\n");
    fprintf(stderr, "    Print values of counters allocated for instrumentation on mutatee exit.\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "  -S\n");
    fprintf(stderr, "    Single-process mode.  Do not fork before launching mutatee.\n");
    fprintf(stderr, "    Used mainly for internal %s debugging.\n", progname);
    fprintf(stderr, "\n");
    fprintf(stderr, "  -t <seconds>\n");
    fprintf(stderr, "    Time limit before monitor forcibly kills mutator and mutatee.\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "  -v [int]\n");
    fprintf(stderr, "    Increase verbose level.  Each -v encountered will increment level.\n");
    fprintf(stderr, "    You may also provide a parameter from 0 to %d.  DEFAULT = %d\n", VERBOSE_MAX - 1, (int)INFO);
    fprintf(stderr, "\n");
    fprintf(stderr, "  -q\n");
    fprintf(stderr, "    Decrement verbose level.\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "  --use-transactions=<string>\n");
    fprintf(stderr, "    Enable instrumentation transactions.  Valid parameters are:\n");
    fprintf(stderr, "      func = Insert instrumentation once per function.\n");
    fprintf(stderr, "      mod  = Insert instrumentation once per module.\n");
    fprintf(stderr, "      proc = Insert instrumentation once per process.\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "  --only-mod=<regex>\n");
    fprintf(stderr, "  --only-func=<regex>\n");
    fprintf(stderr, "    Only parse/instrument modules or functions that match the\n");
    fprintf(stderr, "    specified basic regular expression.\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "  --skip-mod=<regex>\n");
    fprintf(stderr, "  --skip-func=<regex>\n");
    fprintf(stderr, "    Do not parse/instrument modules or functions that match the\n");
    fprintf(stderr, "    specified basic regular expression.\n");
    fprintf(stderr, "\n");
}
