
/*
 * interface between PVM starter/ptrace processes and dyninst controller.
 *   This interface is implemented as a series of PVM messages.
 *
 */

/* message types (i.e. TAGS) */
#define PTRACE_REQUEST	1
#define START_PROCESS	2
#define PROCESS_STARTED	3
#define TRACE_RECORD	4
#define PROCESS_EXITED	5
#define DAEMON_READY	6
#define PRINT_MESSAGE	7
#define PTRACE_RESPONCE	8
#define TASK_GONE	9
#define STARTER_REPLACED	10

#define PARADYN_PVM_VERSION	2
