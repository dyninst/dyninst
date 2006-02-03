#include <iostream>
#include <csignal>
#include <cassert>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>

#include "ipc.h"
#include "log.h"
#include "utils.h"
#include "config.h"
#include "dyninstCompat.h"

using namespace std;

const char *msgStr(messageID msgID)
{
    switch (msgID) {
    case ID_INVALID:			return "Invalid message identifier.";
    case ID_INIT_CREATE_BPATCH:		return "Creating new BPatch object.";
    case ID_INIT_REGISTER_EXIT:		return "Requesting notification of mutatee exit.";
    case ID_INIT_REGISTER_FORK:		return "Requestion notification of mutator fork.";
    case ID_INIT_CREATE_PROCESS:	return "Forking mutatee process.";
    case ID_INIT_GET_IMAGE:		return "Retrieving BPatch_image object.";

    case ID_PARSE_MODULE:		return "Parsing image for module information.";
    case ID_PARSE_FUNC:			return "Parsing symbol table and debugging information from module.";
    case ID_PARSE_MODULE_CFG:		return "Analyzing module to generate CFGs.";
    case ID_PARSE_FUNC_CFG:		return "Analyzing function code to generate CFG.";

    case ID_POST_FORK:			return "Mutator has forked.";

    case ID_INST_START_TRANS:		return "Starting instrumentation transaction.";
    case ID_INST_END_TRANS:		return "Ending instrumentation transaction.";

    case ID_INST_MODULE:		return "Attempting to instrument functions in module.";
    case ID_INST_FUNC:			return "Attempting to instrument function.";
    case ID_INST_FUNC_ENTRY:		return "Attempting to instrument function entry.";
    case ID_INST_FUNC_EXIT:		return "Attempting to instrument function exit.";
    case ID_INST_BASIC_BLOCK:		return "Attempting to instrument basic blocks.";
    case ID_INST_MEM_READ:		return "Attempting to instrument memory reads.";
    case ID_INST_MEM_WRITE:		return "Attempting to instrument memory writes.";
	
    case ID_GET_CFG:			return "Retrieving CFG information from function.";
    case ID_INST_GET_BB:		return "Retrieving basic blocks from CFG.";
    case ID_INST_BB_LIST:		return "Attempting to instrument basic blocks of CFG.";
    case ID_INST_GET_BB_POINTS:		return "Retrieving list of instrumentable points from basic block.";

    case ID_ALLOC_COUNTER:		return "Allocating an integer in mutatee.";
    case ID_INST_FIND_INT:		return "Locating BPatch_type representation for int type.";
    case ID_INST_MALLOC_INT:		return "Instructing mutatee to call malloc().";
    case ID_INST_GET_FUNCS:		return "Retrieving list of functions from module.";
    case ID_INST_FIND_POINTS:		return "Retrieving list of instrumentable points from function.";
    case ID_INST_INSERT_CODE:		return "Inserting counter-increment snippet into function entry.";

    case ID_RUN_CHILD:			return "Continuing execution of mutatee.";
    case ID_WAIT_TERMINATION:		return "Waiting for child termination.";
    case ID_WAIT_STATUS_CHANGE:		return "Waiting for status change in child.";
    case ID_EXIT_CODE:			return "Mutatee exited normally.";
    case ID_EXIT_SIGNAL:		return "Mutatee terminated via signal.";

    case ID_SUMMARY_INSERT:		return "Inserting information into summary report.";

    case ID_DATA_STRING:		return "Passing string to monitor.";

    default:				return "Unknown message ID";
    }
    return NULL;
}

message *readMsg(FILE *infd, message *msg)
{
    msg->id_data = encodeID(ID_INVALID, INFO, ID_TEST);

    char *msg_line = fgets_static(infd);
    if (!msg_line) {
	if (errno) dlog(ERR, "    Error in fgets_static() from readMsg(). %d (%s)\n", errno, strerror(errno));
	return NULL;
    }
    chomp(msg_line);

    if (config.curr_rec.enabled)
	fprintf(config.curr_rec.raw_fd, "%s\n", msg_line);

    errno = 0;
    char *str_data;
    unsigned id_data = (unsigned)strtol(msg_line, &str_data, 16);
    if (errno) {
	dlog(WARN, "Invalid message received (Error parsing ID): [%s]\n", msg_line);
	return NULL;
    }

    if (*str_data != ' ') {
	dlog(WARN, "Invalid message received (Invalid character in ID): [%s]\n", msg_line);
	return NULL;
    }

    ++str_data;
    str_data = decodeStr(str_data);
    if (!str_data) {
	dlog(ERR, "    Error in decodeStr() from readMsg()\n");
	return NULL;
    }

    msg->id_data = id_data;
    msg->int_data = -1;
    msg->str_data = strdup(str_data);

    if (!msg->str_data) {
	dlog(ERR, "strdup() could not allocate memory for string.\n");
	return NULL;
    }

    if (*str_data == '\0') return msg;
    for (unsigned i = 0; str_data[i] != '\0'; ++i)
	if (!isdigit(str_data[i])) return msg;

    msg->int_data = strtol(str_data, NULL, 10);
    return msg;
}

void sendMsg(FILE *outfd, messageID msgID, logLevel priority, statusID statID, const char *str_data)
{
    unsigned encoded_id = encodeID(msgID, priority, statID);
    if (!str_data) str_data = "";

    char *encoded_str = encodeStr(str_data);
    if (!encoded_str) {
	dlog(ERR, "    Error in encodeStr() from sendMsg()\n");
	return;
    }

    const char *msgstr;
    switch (statID) {
    case ID_INFO:
    case ID_TEST: msgstr = msgStr(msgID); break;
    case ID_WARN: msgstr = "Warning encountered."; break;
    case ID_FAIL: msgstr = "Error encountered."; break;
    case ID_PASS: msgstr = "Success."; break;
    }
    fprintf(outfd, "%05x %s %s\n", encoded_id, encoded_str, msgstr);
    fflush(outfd);
}

void sendMsg(FILE *outfd, messageID msgID, logLevel priority, statusID statID, int int_data)
{
    char *buf = sprintf_static("%d", int_data);

    if (buf)
	sendMsg(outfd, msgID, priority, statID, buf);
    else
	dlog(ERR, "    Error in sprintf_static() from sendMsg()\n");
}

void sigintHandler(int);
void sigchldHandler(int);
void sigalrmHandler(int);

void setSigHandlers()
{
    struct sigaction action;

    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;

    action.sa_handler = sigintHandler;
    sigaction(SIGINT, &action, NULL);

    if (config.pid != 0) {
	// Monitor-only signal handlers
	action.sa_handler = sigchldHandler;
	sigaction(SIGCHLD, &action, NULL);

	action.sa_handler = sigalrmHandler;
	sigaction(SIGALRM, &action, NULL);
    }
}

void resetSigHandlers()
{
    struct sigaction action;

    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    action.sa_handler = SIG_DFL;

    sigaction(SIGCHLD, &action, NULL);
    sigaction(SIGALRM, &action, NULL);
}

void killProcess(pid_t pid)
{
    int retval = 10;

    if (kill(pid, 0) == -1) return;
    dlog(INFO, "Allowing mutator/mutatee (%d) time to exit gracefully...\n", pid);

    do {
	if (kill(pid, SIGTERM) == -1) return;
	kill(pid, SIGCONT);  // In case children are in a sleep state.
    } while ((retval = sleep(retval)) > 0);

    dlog(INFO, "Times up.  Sending SIGKILL to %d", pid);
    for (int i = 0; i < 10; ++i) {
	dlog(INFO, ".");
	if (kill(pid, SIGKILL) == -1) {
	    dlog(INFO, "\n");
	    return;
	}
	kill(pid, SIGCONT); // In case children are in a sleep state.
	sleep(1);
    }
    dlog(WARN, "\n* Could not terminate process %d.\n",  pid);
}

void cleanupProcesses()
{
    // Clean up any grandchildren we know about.
    set< int >::iterator iter = config.grandchildren.begin();
    while (iter != config.grandchildren.end()) {
	killProcess(-(*iter));
	++iter;
    }

    // Clean up mutator
    killProcess(-config.pid);
}

void sigintHandler(int signal)
{
    assert(signal == SIGINT);

    if (config.pid > 0) {
	dlog(ERR, "\nMonitor received SIGINT.  Requesting mutator to exit...\n");

	// Disable alarm, if any.
	alarm(0);

	kill(-config.pid, SIGINT);
	sleep(10);

	cleanupProcesses();
	exit(-1);

    } else if (config.pid == 0) {
	// Mutator received SIGINT.  Instruct mutatee to terminate.
	if (config.dynlib && !dynIsTerminated(config.dynlib))
	    dynTerminateExecution(config.dynlib);
	exit(-1);
    }
}

void sigchldHandler(int signal)
{
    int pid, status, options = 0;
    assert(signal == SIGCHLD);
    if (config.pid) dlog(DEBUG, "Monitor received SIGCHLD\n");

    do {
	errno = 0;
	pid = waitpid(-1, &status, options);

	dlog(DEBUG, "waitpid returned %x status for pid %d\n", status, pid);
	if (pid > 0) {
	    if (WIFSTOPPED(status)) {
		dlog(WARN, "\n*\n* Abnormal case: Dyninst mutator stopped on signal %d.  Ignorning.\n*\n", WSTOPSIG(status));

	    } else if (WIFSIGNALED(status)) {
		dlog(ERR, "\n*\n* Abnormal case: Dyninst mutator terminated via signal %d.  Monitor exiting.\n", WTERMSIG(status));
#ifdef WCOREDUMP
		if (WCOREDUMP(status)) dlog(ERR, "* Core file generated.\n");
#endif
		dlog(ERR, "*\n");

	    } else if (WIFEXITED(status)) {
		dlog(INFO, "Dyninst mutator exited normally and returned %d.\n", WEXITSTATUS(status));
	    }
	    if (pid == config.pid) config.state = CHILD_EXITED;
	}
	options = WNOHANG;
    } while (pid != 0 && pid != -1);

    if (errno && errno != ECHILD) {
	dlog(ERR, "Error in waitpid(): %s\n", strerror(errno));
	exit(-2);
    }
}

void sigalrmHandler(int signal)
{
    assert(signal == SIGALRM);

    if (config.state == NORMAL)
	dlog(ERR, "\n*\n* Abnormal case: Dyninst mutator exceeded time limit.  Forcing mutator/mutatee termination.\n*\n");
    config.state = TIME_EXPIRED;
    kill(config.pid, SIGTERM);
}
