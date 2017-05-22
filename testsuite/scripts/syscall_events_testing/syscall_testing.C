/* Includes */
#include <getopt.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/* Dyninst Includes */
#include "PCProcess.h"
#include "Event.h"
#include "EventType.h"
#include "PCErrors.h"

/* Local Includes */
#include "utils.h"

using namespace std;
using namespace Dyninst;

void usage(char* arg0)
{
   cerr << "Usage: " << arg0 << endl
      << "\t -b <binary>, OR" << endl
      << "\t [-a <args>]" << endl
      << "\t -p <PID>]" << endl
      << "\t [-o <stdout file>]" << endl
      << "\t [-e <stderr file>]" << endl;
}

std::string BIN_FILE;
std::string ARGS;
int CUR_PID = -1;
std::string STDOUT_FILE;
std::string STDERR_FILE;

static int counter = 1;

/*
 * Parse input options
 */
void parse_options(int argc, char** argv)
{
    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'}
    };

    int ch;
    int option_index = 0;

    while ((ch = getopt_long(argc, 
                    argv,
                    "a:b:e:o:p:h",
                    long_options,
                    &option_index)) != -1)
    {
        switch(ch) {
            case 'a':
                ARGS = std::string(optarg);
                break;
            case 'b':
                BIN_FILE = std::string(optarg);
                break;
            case 'e':
                STDERR_FILE = std::string(optarg);
                break;
            case 'h':
                usage(argv[0]);
                exit(0);
            case 'o':
                STDOUT_FILE = std::string(optarg);
                break;
            case 'p':
                CUR_PID = atoi(optarg);
                break;
            default:
                printf("Illegal option %c\n", ch);
        }
    }

    if ((BIN_FILE == "") && (CUR_PID == -1)) {
        usage(argv[0]);
        exit(1);
    }

    if ((STDOUT_FILE != "" ) && (STDOUT_FILE.compare(STDERR_FILE) == 0)) {
        mydebug("STDOUT_FILE and STDERR_FILE cannot be the same (weird behavior)\n");
        exit(1);
    }
}

void print_options()
{
    myprint("syscall_testing called with:\n");
    if (BIN_FILE != "") { myprint("\t BIN_FILE = %s\n", BIN_FILE.c_str()); }
    if (ARGS != "") { myprint("\t ARGS = %s\n", ARGS.c_str()); }
    if (CUR_PID != -1) { myprint("\t CUR_PID = %d\n", CUR_PID); }
    if (STDOUT_FILE != "") { myprint("\t STDOUT_FILE = %s\n", STDOUT_FILE.c_str()); }
    if (STDERR_FILE != "") { myprint("\t STDERR_FILE = %s\n", STDERR_FILE.c_str()); }
}


ProcControlAPI::Process::cb_ret_t on_event(ProcControlAPI::Event::const_ptr event)
{
    ProcControlAPI::EventType type = event->getEventType();
    ProcControlAPI::Process::const_ptr proc = event->getProcess();
    std::string name = type.name();
    
    ProcControlAPI::Process::cb_ret_t ret = ProcControlAPI::Process::cbProcContinue; // default

    switch(type.code()) {
        case ProcControlAPI::EventType::PreSyscall:
            {
                ProcControlAPI::EventPreSyscall::const_ptr syscallEvent = event->getEventPreSyscall();
                Address addr = syscallEvent->getAddress();
                MachSyscall syscall = syscallEvent->getSyscall();
                myprint("[#%d: %s,%lx,", counter, syscall.name(), addr);
                counter++;
                break;
            }
        case ProcControlAPI::EventType::PostSyscall:
            {
                ProcControlAPI::EventPostSyscall::const_ptr syscallEvent = event->getEventPostSyscall();
                long retValue = syscallEvent->getReturnValue();
                myprint("%ld]\n", retValue);
                break;
            }
        default:
            mydebug("No handler defined for this event type!\n");
            break;
    }

    return ret;
}

void addCallbacks(ProcControlAPI::Process::ptr proc)
{
//    ProcControlAPI::Process::registerEventCallback(ProcControlAPI::EventType(ProcControlAPI::EventType::Pre,
//                ProcControlAPI::EventType::Syscall),
//            on_event);
//    
//    ProcControlAPI::Process::registerEventCallback(ProcControlAPI::EventType(ProcControlAPI::EventType::Post,
//                ProcControlAPI::EventType::Syscall),
//            on_event);
    ProcControlAPI::Process::registerEventCallback(ProcControlAPI::EventType::PreSyscall, on_event);
    ProcControlAPI::Process::registerEventCallback(ProcControlAPI::EventType::PostSyscall, on_event);
}

void setupProcess(ProcControlAPI::Process::ptr proc)
{
    addCallbacks(proc);

    /* Set threads to use syscall mode */
    ProcControlAPI::ThreadPool & threads = proc->threads();
    ProcControlAPI::ThreadPool::iterator tIter;

    for (tIter = threads.begin(); tIter != threads.end(); ++tIter) {     
        ProcControlAPI::Thread::ptr thread = *tIter;
        thread->setSyscallMode(true);
    }
}

ProcControlAPI::Process::ptr initProcess(int pid, const char* bin_file, const char* args,
        const char * stdout_file, const char * stderr_file)
{
    ProcControlAPI::Process::ptr proc;

    if (pid != -1) {
        /* Attach to running process */
//        myprint("Attaching to process %d\n", pid);
        proc = ProcControlAPI::Process::attachProcess(pid);
        if (!proc) { mydebug("attachProcess(%d) failed: %s\n", pid, ProcControlAPI::getLastErrorMsg()); exit(1); }
    } else {
        /* Create a new process */
        
        /* Put together any arguments */
        std::vector<std::string> local_argv;
        local_argv.push_back(bin_file);
        if (args) {
            char * local_args = strdup(args);
            char * tmp;
            const char * delim = " ";
            tmp = strtok(local_args, delim);
            while (tmp != NULL) {
                string tmpStr(tmp);
                local_argv.push_back(tmpStr);
                tmp = strtok(NULL, delim);
            }
        }
        
        /* Put together any necessary fd redirection */
        std::map<int,int> fds;
        if (stdout_file != NULL) {
            int stdout_fd = open(stdout_file, O_RDWR | O_CREAT);
            if (stdout_fd == -1) { myperror("open(stdout_file=%s)", stdout_file); exit(1); }
            fds.insert(make_pair(stdout_fd, STDOUT_FILENO));
//            myprint("Redirecting stdout to %s\n", stdout_file);
        }

        if (stderr_file != NULL) {
            int stderr_fd = open(stderr_file, O_RDWR | O_CREAT);
            if (stderr_fd == -1) { myperror("open(stderr_file=%s)", stderr_file); exit(1); }
            fds.insert(make_pair(stderr_fd, STDERR_FILENO));
            //            myprint("Redirecting stderr to %s\n", stderr_file);
        }

//        myprint("Creating process from binary %s\n", bin_file);
        std::vector<std::string> envp;
        proc = ProcControlAPI::Process::createProcess(bin_file, local_argv, envp, fds);
        if (!proc) { mydebug("createProcess(%d) failed: %s\n", pid, ProcControlAPI::getLastErrorMsg()); exit(1); }
//        myprint("New PID is %d\n", proc->getPid());
    }

    return proc;
}


int main(int argc, char** argv)
{
    parse_options(argc, argv);

//    print_options();

    /* Create/Attach to Process */
    ProcControlAPI::Process::ptr proc = initProcess(CUR_PID, BIN_FILE.c_str(), ARGS.c_str(), 
            (STDOUT_FILE != "" ? STDOUT_FILE.c_str() : NULL), (STDERR_FILE != "" ? STDERR_FILE.c_str() : NULL));
   
    /* Set up callbacks, etc. */ 
    setupProcess(proc);

    /* Continue execution */
//    myprint("Setup complete; process %d continuing\n", proc->getPid());
    if (!proc->continueProc()) {mydebug("continueProc failed: %s\n", ProcControlAPI::getLastErrorMsg()); exit(1); }

    while (!proc->isTerminated()) {
        // Spin for a while
        ProcControlAPI::Process::handleEvents(true);
    }

//    myprint("Process terminated.\n");
    myprint("?]\n"); // We don't have a return value for the exit() syscall; strace prints "?"
}
