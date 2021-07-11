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
#include <queue>
#include <iostream>
#include <string>
#include <cctype>
//#include <boost/regex.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/stat.h>           // Needed for stat() call in parseArgs().
#include <sys/wait.h>
using namespace std;

#include "ipc.h"
#include "utils.h"
#include "config.h"
#include "dyninstCore.h"
#include "record.h"
#include "reglist.h"

void userError();
void parseArgs(int argc, char **argv);
void usage(const char *);
bool writeHunt();
bool runHunt();
bool runParseThat(int &bannerLen);

int main(int argc, char **argv)
{
    int retval = 0;
    parseArgs(argc, argv);

    if (config.write_crashes && !config.no_fork) {
        getNextTarget();

		printf(" write_crashes hunt_crashes \n");
        return writeHunt();
    }

    if (config.hunt_crashes && !config.no_fork) {
        getNextTarget();
        return runHunt();
    }

    int bannerLen = 0;
    while (getNextTarget()) {
        retval = runParseThat(bannerLen);
    }
    dlog(INFO, "Analysis complete.\n");
    cleanupFinal();

    return retval;
}

bool runParseThat(int &bannerLen)
{
    int pipefd[2];

    if (config.no_fork) {
        if (config.memcpu) track_usage();
        int result = launch_mutator();
        if (config.memcpu) report_usage();

        return result;
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
            if (!record_create(&config.curr_rec, config.target,
                               config.argc, config.argv))
                dlog(WARN, "Error creating history record for %s\n",
                     config.target);

            else if (!record_search(&config.curr_rec))
                record_update(&config.curr_rec);
        }

        // Convert raw socket to stream based FILE *.
        errno = 0;
        FILE *infd = fdopen(pipefd[0], "r");
        if (infd == NULL) {
            dlog(ERR, "Error on fdopen() in child: %s\n", strerror(errno));
            dlog(ERR, "*** Child exiting.\n");
            exit(-2);
        }

        // Only parent should have signal handlers modified.
        setSigHandlers();

        launch_monitor(infd);

        // Reset signal handers so next child won't be affected.
        resetSigHandlers();

        fprintf(config.outfd, "[ Done processing %s ] %n",
                config.target, &bannerLen);
        while (++bannerLen < 80) fprintf(config.outfd, "-");
        fprintf(config.outfd, "\n");

        // Clean up any known processes we created.
        cleanupProcess();

        fclose(infd);

    } else if (config.pid == 0) {
        /**************************************************************
         * Child Case
         */

        // Register to catch SIGINT
        setSigHandlers();

        // Start new process group.  Makes forced process shutdown easier.
        setpgid(0, 0);

        close(pipefd[0]); // Close (historically) read side of pipe.

        // Leave stdout open for mutatee, but if an output file was specified
        // by user, don't keep multiple descriptors open for it.
        if (config.outfd != stdout) fclose(config.outfd);

        // Convert raw socket to stream based FILE *.
        errno = 0;
        config.outfd = fdopen(pipefd[1], "w");
        if (config.outfd == NULL) {
            fprintf(stderr, "Error on fdopen() in mutator: %s\n",
                    strerror(errno));
            fprintf(stderr, "*** Mutator exiting.\n");
            exit(-2);
        }

        if (config.memcpu) track_usage();
        int result = launch_mutator();
        if (config.memcpu) report_usage();
        exit(result);

    } else {
        /* Fork Error Case */
        dlog(ERR, "Error on fork(): %s\n", strerror(errno));
        exit(-2);
    }

    if (config.abnormal_exit) {
        return -1;
    }
    else return 0;
}

bool runHunt_binaryEdit()
{
    int result, status;
    int pid = fork();
    if (pid == 0) {
        // child case
        // run new binary
	std::string filename = std::string("./") + ((config.use_exe) ? config.exeFilePath : config.writeFilePath);
	char *exeFile = strdup(filename.c_str());

        int numargs = 0;
        char **arg = (char **) malloc (2 * sizeof(char*));
        arg[0] = exeFile;

        fprintf(stderr, "Executing new binary: \"%s", exeFile);
        if (config.binary_args) {
            char nargs[1024];
            int offset = strcspn (config.binary_args, ":");
            if(offset == 0) {
                dlog(ERR, "\nInvalid format for command line arguments to the binary --args=<numArgs>:<comma-seperated args list>\n");
                exit(-2);
            }
            strncpy (nargs, config.binary_args, offset);
            nargs[offset] = '\0';
            numargs = atoi(nargs);

            arg = (char **) realloc (arg, (numargs+2) * sizeof(char*));

            char *args = strchr(config.binary_args, ':');
            args++;
                
            char *p = strtok(args, ",");
            int i = 1;
            while ( p != NULL && i <= numargs ) {
                fprintf(stderr," %s", p);
                arg[i] = p;
                i++;
                p = strtok(NULL, ",");
            }
            if (i <= numargs) {
                dlog(ERR, "\nWrong number of arguments specified for executing binary. Expected %d but got only %d arguments\n", numargs, i-1);
                exit(-2);
            }
        }
                        
        fprintf(stderr,"\"\n");
        arg[numargs+1] = NULL;

        execvp(exeFile, arg);
        perror ("execvp failed");
        abort();
    } else if(pid < 0) {
        /* Fork Error Case */
        dlog(ERR, "Error on runHunt fork(): %s\n", strerror(errno));
        exit(-2);
    }

    result = waitpid(pid,&status,0);
    if (result == -1) {
        perror ("waitpid failed");
        return true;
    }else if ( WIFEXITED(status) && WEXITSTATUS(status) != 0) {
        printf(" Exiting non-zero \n");
        return true;
    }else if (WIFSIGNALED (status) && WTERMSIG(status) != 1) {
        printf(" Exiting with  signal %d \n", WTERMSIG(status));
        return true;
    }
        
    return false;
}

bool writeHunt()
{
    int bannerLen = 0;
    runParseThat(bannerLen);
    return true;
}

bool runHunt()
{
    int hard_low = -1;
    int hard_high = -1;
    int bannerLen = 0;
    bool crash = false;
    FILE *hunt_file = config.hunt_file;
    std::queue<std::pair<int, int> > postponed_crashes;

    if (config.hunt_low == -1 || config.hunt_high == -1) 
        {
            //Initial run over everything, get ranges.
            runParseThat(bannerLen);

            if (!config.use_process) {
                crash = runHunt_binaryEdit();
            }
        
            if (!config.hunt_crashed && !crash) {
                fprintf(hunt_file, "No crashes detected in initial run\n");
                goto done;
            }
            hard_low = config.hunt_low = 0;
            hard_high = config.hunt_high;

            if (hard_high == -1) {
                fprintf(hunt_file, "Mutator crash prevent range discovery, fix this first\n");
                fflush(hunt_file);
                goto done;
            }
        }
    else if(config.hunt_high == config.hunt_low+1) {
        //Initial run over everything, get ranges.
        runParseThat(bannerLen);

        if (!config.use_process) {
            crash = runHunt_binaryEdit();
        }
        if (!crash) goto done;
        
    } else {
        hard_low = config.hunt_low;
        hard_high = config.hunt_high;
    }
      

    for (;;) {
        int mid = (hard_low + hard_high) / 2;
      
        if (mid == hard_low) {
            fprintf(hunt_file, "** Found Crash point at [%d, %d) **\n", 
                    hard_low, hard_high);
            fflush(hunt_file);
            if (!postponed_crashes.size())
                goto done;
            std::pair<int, int> p = postponed_crashes.front();
            postponed_crashes.pop();
            hard_low = p.first;
            hard_high = p.second;
            continue;
        }

        fprintf(hunt_file, "Crash is in interval [%d, %d)\n", hard_low, hard_high);

        config.hunt_low = hard_low;
        config.hunt_high = mid;
        config.hunt_crashed = false;

        fprintf(hunt_file, "\tRunning interval [%d, %d)...", hard_low, mid);
        fflush(hunt_file);
        runParseThat(bannerLen);
        crash = false;
        if (!config.use_process) {
            crash = runHunt_binaryEdit();
        }

        bool low_half_crash = config.hunt_crashed || crash;
        if (low_half_crash)
            fprintf(hunt_file, "Crashed\n");
        else
            fprintf(hunt_file, "Success\n");

        config.hunt_low = mid;
        config.hunt_high = hard_high;
        fprintf(hunt_file, "\tRunning interval [%d, %d)...", mid, hard_high);
        fflush(hunt_file);
        runParseThat(bannerLen);
        crash = false;
        if (!config.use_process) {
            crash = runHunt_binaryEdit();
        }

        bool high_half_crash = config.hunt_crashed || crash;
        if (high_half_crash)
            fprintf(hunt_file, "Crashed\n");
        else
            fprintf(hunt_file, "Success\n");
      
        if (low_half_crash && !high_half_crash) {
            hard_high = mid;
        }
        else if (!low_half_crash && high_half_crash) {
            hard_low = mid;
        }
        else if (low_half_crash && high_half_crash) {
            std::pair<int, int> p;
            p.first = mid;
            p.second = hard_high;
            postponed_crashes.push(p);

            hard_high = mid;
        }
        else if (!low_half_crash && !high_half_crash) {
            fprintf(hunt_file, "** Found Crash point at [%d, %d) **\n", 
                    hard_low, hard_high);
            fflush(hunt_file);
            if (!postponed_crashes.size())
                goto done;
            std::pair<int, int> p = postponed_crashes.front();
            postponed_crashes.pop();
            hard_low = p.first;
            hard_high = p.second;
            continue;
        }
    }

  done:
    fclose(hunt_file);
    dlog(INFO, "Analysis complete.\n");
    cleanupFinal();
    return 0;
}

void parseArgs(int argc, char **argv)
{
   int tempInt;
    bool needShift;
    char *arg;

    if (argc < 2) {
        fprintf(stderr, "Too few arguments.\n");
        userError();
    }

    configInit();

    int i = 0;
    while (++i < argc && *argv[i] == '-') {
        char *ptr = argv[i];
        while (*(++ptr)) {
            switch (*ptr) {
            case 'a':
                config.include_libs = true;
                break;

            case 'c':
                if (++i < argc) {
                    config.config_file = argv[i];
                    config.runMode = BATCH_FILE;

                } else {
                    fprintf(stderr, "-c flag requires an argument.\n");
                    userError();
                }
                break;

            case 'f':           
                if (++i < argc) {
                    config.inst_function = argv[i];
                    if (strchr (config.inst_function, ':')) {
                        config.instType = USER_FUNC;
                    } else {
                        fprintf(stderr, "-f flag requires an argument of the format library:function_name.\n");
                        userError();
                    }
                } else {
                    fprintf(stderr, "-f flag requires an argument of the format library:function_name.\n");
                    userError();
                }
                break;

            case 'h':
                config.record_enabled = 1;
                break;

            case 'i':
                if (isdigit(*(ptr + 1))) {
                    config.inst_level = (InstLevel)strtol(++ptr, &arg, 0);
                    ptr = arg - 1;

                } else if (++i < argc) {
                    config.inst_level = (InstLevel)atoi(argv[i]);

                } else {
                    fprintf(stderr, "-i flag requires an argument.\n");
                    userError();
                }

                if (config.inst_level == 0 && errno == EINVAL) {
                    fprintf(stderr, "Invalid argument to -i flag: '%s'\n", argv[i]);
                    userError();
                }

                if (config.inst_level < 0 || config.inst_level >= INST_MAX) {
                    fprintf(stderr, "Invalid argument to -i flag.  Valid range is 0 through %d\n", INST_MAX - 1);
                    userError();
                }
                break;

            case 'm':
                config.use_merge_tramp = true;
                break;

            case 'o':
                if (++i < argc) {
                    config.output_file = argv[i];
                    config.outfd = NULL;

                } else {
                    fprintf(stderr, "-o flag requires an argument.\n");
                    userError();
                }
                break;

            case 'p':
                if (isdigit(*(ptr + 1))) {
                    config.parse_level = (ParseLevel)strtol(++ptr, &arg, 0);
                    ptr = arg - 1;

                } else if (++i < argc) {
                    config.parse_level = (ParseLevel)atoi(argv[i]);

                } else {
                    fprintf(stderr, "-p flag requires an argument.\n");
                    userError();
                }

                if (config.parse_level == 0 && errno == EINVAL) {
                    fprintf(stderr, "Invalid argument to -p flag: '%s'\n", argv[i]);
                    userError();
                }

                if (config.parse_level < 0 || config.parse_level >= PARSE_MAX) {
                    fprintf(stderr, "Invalid argument to -p flag.  Valid range is 0 through %d\n", PARSE_MAX - 1);
                    userError();
                }
                break;

            case 'P':
                if (isdigit(*(ptr + 1))) {
                    config.attach_pid = strtol(++ptr, &arg, 0);
                    ptr = arg - 1;

                } else if (++i < argc) {
                    config.attach_pid = atoi(argv[i]);

                } else {
                    fprintf(stderr, "-P flag requires an argument.\n");
                    userError();
                }

                if (config.attach_pid == 0) {
                    fprintf(stderr, "Invalid argument to -P flag.\n");
                    userError();
                }

                config.use_attach = true;
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

                } else if (++i < argc) {
                    config.time_limit = atoi(argv[i]);

                } else {
                    fprintf(stderr, "-t flag requires an argument.\n");
                    userError();
                }

                if (config.time_limit == 0 && errno == EINVAL) {
                    fprintf(stderr, "Invalid argument to -t flag: '%s'\n", argv[i]);
                    userError();
                }
                break;

            case 'T':
               tempInt = 0;
               if (isdigit(*(ptr + 1))) {
                  tempInt = strtol(++ptr, &arg, 0);
                  ptr = arg - 1;
                  
               } else if (i+1 < argc && isdigit(*argv[i+1])) {
                  tempInt = atoi(argv[++i]);
               }
               
               config.trace_count = (tempInt < 0 ? 0 : (unsigned int)tempInt);
               if (config.use_process)
                  config.trace_inst = true;
               break;

            case 'v':
                if (isdigit(*(ptr + 1))) {
                    config.verbose = strtol(++ptr, &arg, 0);
                    ptr = arg - 1;

                } else if (i+1 < argc && isdigit(*argv[i+1])) {
                    config.verbose = atoi(argv[++i]);

                } else {
                    ++config.verbose;
                }

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
            case 'l':
                if( ++i < argc ) {
                    config.symbol_libraries.push_back(string(argv[i]));
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

                if (strcmp(ptr, "-check") == 0) {
			
		} else if (strcmp(ptr, "-all") == 0 ||
                    strcmp(ptr, "-include-libs") == 0) {
                    config.include_libs = true;

                } else if (strcmp(ptr, "-pid") == 0) {
                    if (!arg) {
                        fprintf(stderr, "--pid requires an integer argument.\n");
                        userError();

                    } else if (isdigit(*arg)) {
                        config.attach_pid = strtol(arg, &arg, 0);
                    }

                    if (config.attach_pid == 0) {
                        fprintf(stderr, "Invalid argument to --pid flag.\n");
                        userError();
                    }
                    config.use_attach = true;

                    if (needShift) ++i;
                    break;
                    
                } else if (strcmp(ptr, "-help") == 0) {
                    usage(argv[0]);
                    exit(0);

                } else if (strcmp(ptr, "-merge-tramp") == 0) {
                    config.use_merge_tramp = true;

                } else if (strcmp(ptr, "-memcpu") == 0 ||
                           strcmp(ptr, "-cpumem") == 0) {
                    config.memcpu = true;

                } else if (strcmp(ptr, "-only-func") == 0) {
                    if (!arg) {
                        fprintf(stderr, "--only-func requires a regular expression argument.\n");
                        userError();
                    }
                    if (!config.func_rules.insert(arg, RULE_ONLY))
                        userError();
                    if (needShift) ++i;

                } else if (strcmp(ptr, "-only-mod") == 0) {
                    if (!arg) {
                        fprintf(stderr, "--only-mod requires a regular expression argument.\n");
                        userError();
                    }
                    if (!config.mod_rules.insert(arg, RULE_ONLY))
                        userError();
                    if (needShift) ++i;

                } else if (strcmp(ptr, "-summary") == 0) {
                    config.summary = true;

                } else if (strcmp(ptr, "-suppress-ipc") == 0) {
                    config.printipc = false;

                }  else if (strcmp(ptr, "-binary-edit") == 0) {
                    config.use_process = false;
                    config.trace_inst = false;
                    if (!arg) {
                        fprintf(stderr, "--binary-edit requires a path argument\n");
                        userError();
                    }
                    else
                        {
                            strcpy(config.writeFilePath,arg);
                            printf("Write File Path is %s\n", config.writeFilePath);
                        }
                }  else if (strcmp(ptr, "-exe") == 0) {
                    config.use_exe = true;
                    if (!arg) {
                        fprintf(stderr, "--binary-edit requires a path argument\n");
                        userError();
                    }
                    else
                        {
                            strcpy(config.exeFilePath,arg);
                            printf(" Executable Path is %s\n", config.exeFilePath);
                        }
                } else if (strcmp(ptr, "-args") == 0) {
                    if (!arg) {
                        fprintf(stderr, "-args requires an argument of the format <number_of_arguments:comma-seperated list of arguments>\n");
                        userError();
                    }
                    else
                        {
                            if (!strchr (arg, ':')) {
                                fprintf(stderr, "-args requires an argument of the format <number_of_arguments:comma-seperated list of arguments>\n");
                                userError();
                            }
                            config.binary_args = (char *) malloc (strlen(arg)+1);
                            strcpy(config.binary_args,arg);
                        }
                        
                }  else if (strcmp(ptr, "-skip-func") == 0) {
                    if (!arg) {
                        fprintf(stderr, "--skip-func requires a regular expression argument.\n");
                        userError();
                    }
                    if (!config.func_rules.insert(arg, RULE_SKIP))
                        userError();
                    if (needShift) ++i;

                } else if (strcmp(ptr, "-skip-mod") == 0) {
                    if (!arg) {
                        fprintf(stderr, "--skip-mod requires a regular expression argument.\n");
                        userError();
                    }
                    if (!config.mod_rules.insert(arg, RULE_SKIP))
                        userError();
                    if (needShift) ++i;
                } else if (strcmp(ptr, "-writehunt") == 0) {
                    config.write_crashes = true;
                    config.hunt_crashes = true;
                    config.hunt_file = stderr;
                } else if (strcmp(ptr, "-hunt") == 0) {
                    config.hunt_crashes = true;
                    if (arg) {
                        config.hunt_file = fopen(arg, "w");
                    }
                    else {
                        config.hunt_file = fopen(DEFAULT_HUNT_FILE, "w");
                    }
                    if (!config.hunt_file)
                        config.hunt_file = stderr;
                } else if (strcmp(ptr, "-hunt-low") == 0) {
                    config.hunt_low = atoi(arg);
                } else if (strcmp(ptr, "-hunt-high") == 0) {
                    config.hunt_high = atoi(arg);
                } else if (strcmp(ptr, "-trace") == 0) {
                    tempInt = 0;
                    if (isdigit(*arg)) {
                        tempInt = strtol(arg, &arg, 0);
                        if (needShift) ++i;
                    }

                    config.trace_count = (tempInt < 0 ? 0 : (unsigned int)tempInt);
                    if (config.use_process)
                        config.trace_inst = true;

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
                        userError();
                    }

                } else if (strcmp(ptr, "-wtx-target") == 0) {
                    BPatch_remoteWtxInfo *info;
                    if (arg) {
                        info = (BPatch_remoteWtxInfo *)
                            malloc(sizeof(BPatch_remoteWtxInfo));
                        memset(info, 0, sizeof(BPatch_remoteWtxInfo));

                        if (!strchr(arg, ':')) {
                            info->target = arg;

                        } else {
                            info->target = strchr(arg, ':') + 1;
                            info->host   = arg;
                            *(strchr(arg, ':')) = '\0';
                        }
                        info->tool = strdup("parseThat");

                        config.remoteHost = (BPatch_remoteHost *)
                            malloc(sizeof(BPatch_remoteHost));
                        config.remoteHost->type = BPATCH_REMOTE_DEBUG_WTX;
                        config.remoteHost->info = info;
                    }
                    if (needShift) ++i;

                } else {
                    fprintf(stderr, "Unknown parameter: %s\n", ptr);
                    userError();
                }

                ptr += strlen(ptr) - 1;
                break;

            default:
                fprintf(stderr, "Unknown parameter: -%c\n", *ptr);
                userError();
            }
        }
    }


    // Prepare child arguments
    if (i < argc) {
        strncpy(config.target, argv[i], sizeof(config.target) - 1U);
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
        if (config.use_attach)
            fprintf(stderr, "Attach mode requries an image file in addition to process id.\n");
        else
            fprintf(stderr, "No directory or program specified.\n");
        //userError();
    }

    // Determine run mode, if necessary.
    if (config.runMode != BATCH_FILE && config.target[0] != '\0') {
        errno = 0;
        struct stat file_stat;
        if (stat(config.target, &file_stat) != 0) {
            fprintf(stderr, "Could not stat %s: %s\n", config.target, strerror(errno));
            userError();
        }

        if ((file_stat.st_mode & S_IFMT) == S_IFDIR) {
            // User specified a directory.
            config.runMode = BATCH_DIRECTORY;

        } else if ((file_stat.st_mode & S_IFMT) == S_IFREG) {
            // User specified a file.
            config.runMode = SINGLE_BINARY;

        } else {
            fprintf(stderr, "%s is not a directory or regular file. 0x%x\n", config.target, (file_stat.st_mode & S_IFMT));
            userError();
        }
    }

    // Catch run mode inconsistencies.
    if (config.use_attach && config.runMode != SINGLE_BINARY) {
        fprintf(stderr, "Attach flag cannot be used with batch mode.\n");
        userError();
    }

    // Open output file descriptor.
    if (config.output_file) {
        errno = 0;
        config.outfd = fopen(config.output_file, "w");
        if (config.outfd == NULL) {
            fprintf(stderr, "Could not open %s for writing: %s\n", config.output_file, strerror(errno));
            userError();
        }
    }

    if (config.runMode == BATCH_FILE && config.target[0] != '\0') {
        fprintf(stderr, "Warning: Batch file specified.  Ignoring command line argument %s.\n", config.target);
    }

    // Create history record directory, if needed.
    if (config.record_enabled) {
        if (config.record_dir[0] == '\0') {
            fprintf(stderr, "*\n* Environment variable HOME not defined.  Disabling history records.\n*\n");
            config.record_enabled = false;
          
        } else {
            errno = 0;
            struct stat dir_stat;
            if (stat(config.record_dir, &dir_stat) < 0) {
                if (errno && errno == ENOENT) {
                    errno = 0;
                    mkdir(config.record_dir, S_IRUSR | S_IWUSR | S_IXUSR);
                    if (errno) {
                        fprintf(stderr, "Could not create directory '%s': %s\n", config.record_dir, strerror(errno));
                        userError();
                    }
                } else {
                    fprintf(stderr, "Could not stat %s: %s\n", config.record_dir, strerror(errno));
                    userError();
                }
             
            } else if (!S_ISDIR(dir_stat.st_mode)) {
                fprintf(stderr, "%s exists, but is not a directory.  History record disabled.\n", config.record_dir);
                config.record_enabled = 0;
            }
        }
    }

    // Create named pipe, if needed.
    if (config.trace_inst) {
        if (config.pipe_filename[0] == '\0') {
            fprintf(stderr, "*\n* Environment variable HOME not defined.  Disabling instrumentation tracing.\n*\n");
            config.trace_inst = false;
          
        } else {
            int retval = -1;
            for (int j = 0; retval < 0; ++j) {
                char *tmp_pipename = sprintf_static("%s.%d", config.pipe_filename, j);

                errno = 0;
                retval = mkfifo(tmp_pipename, S_IRUSR | S_IWUSR | S_IXUSR);
                if (errno) {
                    if (errno == EEXIST) continue;
                    fprintf(stderr, "Could not create named pipe '%s': %s\n", tmp_pipename, strerror(errno));
                    userError();

                }
                strncpy(config.pipe_filename, tmp_pipename, sizeof(config.pipe_filename) - 1U);
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
        userError();
    }
}

void usage(const char *pname)
{
    fprintf(stderr, "Usage: %s [options] <dir|prog> [prog_args]\n\n", pname);
    fprintf(stderr, "Options:\n"
"  -a, --all, --include-libs  Include shared libraries for instrumentation\n"
"  --args=<number_of_arguments:comma-separated list of arguments>\n"
"                             While using runHunt, --args is used to specify\n"
"                               a list of command line arguments to run with\n"
"                               the rewritten binary\n"
"  --binary-edit=<filename>   Rewrite binary with instrumentation into\n"
"                               <filename>\n"
"  -c <filename>              Specifies batch mode configuration file\n"
"  -f <library_name:function_name>\n"
"                             Call the specified function at instrumentation\n"
"                               points (Library is loaded if needed)\n"
"  -h                         Enable history logging. Log files will be\n"
"                               stored in /tmp/" HISTORY_RECORD_DIR_DEFAULT "\n"
"  --help                     Print this message\n"
"  -i <num>                   Instrumentation level:\n"
"                               0 = No instrumentation\n"
"                               1 = Function entry instrumentation\n"
"                               2 = Function exit instrumentation\n"
"                               3 = Basic block instrumentation\n"
"                               4 = Memory read instrumentation\n"
"                               5 = Memory write instrumentation\n"
"  -l <library_name>          Force load specified library: can be relative\n"
"                               to standard search path or full path\n"
"  --memcpu, --cpumem         Print memory and CPU usage of mutator on exit\n"
"  -m, --merge-tramp          Merge minitramps into basetramps (more efficent)\n"
"  -o <filename>              Send all monitor output to specified file\n"
"                               (Mutator and mutatee output not included)\n"
"  --only-mod=<regex>, --only-func=<regex>\n"
"                             Only consider modules or functions that match\n"
"                               the specified basic regular expression\n"
"  -p <num>                   Parse level:\n"
"                               0 = Parse for module data\n"
"                               1 = Parse for function data\n"
"                               2 = Parse for control flow graph data\n"
"  -P <int>, --pid=<id>       Attach to specified PID as mutatee\n"
"  -q                         Decrease verboseness\n"
"  -r                         Descend into subdirectories when processing\n"
"                               directories\n"
"  -s, --summary              Print values of counters allocated for\n"
"                               instrumentation on mutatee exit\n"
"  -S                         Single-process mode: Do not fork before\n"
"                               starting mutatee. Internal use only\n"
"  --skip-mod=<regex>, --skip-func=<regex>\n"
"                             Do not consider modules or functions that\n"
"                               match the specified basic regular expression\n"
"  --suppress-ipc             Disable mutator to monitor IPC messages\n"
"  -t <seconds>               Time limit before monitor kills mutator\n"
"  -T [count], --trace[=num]  Report mutatee progress at each function entry\n"
"                               on exit. If count > 0, only last [num] trace\n"
"                               points will be reported\n"
"  --use-transactions=<string>\n"
"                             Enable instrumentation transactions:\n"
"                               func = Per function insertion\n"
"                               mod  = Per module insertion\n"
"                               proc = Per process insertion\n"
"  -v [num]                   Increase verboseness\n"
);
}

void userError()
{
    fprintf(stderr, "Use --help for list of valid parameters.\n");
    exit(-1);
}
