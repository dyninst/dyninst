#include "util/h/headers.h"
#include "os.h"
#include "process.h"
#include "stats.h"
#include "util/h/Types.h"
#include <fcntl.h>
#include "showerror.h"
#include "main.h"
#include "symtab.h"

class ptraceKludge {
public:
  static bool haltProcess(process *p);
  static bool deliverPtrace(process *p, int req, char *addr,
			    int data, char *addr2);
  static void continueProcess(process *p, const bool halted);
};

bool ptraceKludge::haltProcess(process *p) {
  bool wasStopped = (p->status() == stopped);
  if (p->status() != neonatal && !wasStopped) {
    if (!p->loopUntilStopped()) {
      cerr << "error in loopUntilStopped\n";
      assert(0);
    }
  }
  return wasStopped;
}


// Search the unwind table to find the entry correspoding to the pc 
// return the pointer to that entry
struct unwind_table_entry *
findUnwindEntry(image* symbols,int pc)
{
    int middleEntry;
    int firstEntry = 0;
    int lastEntry = (symbols->unwind).size() - 1 ;

    while (firstEntry <= lastEntry)
    {
	middleEntry = (firstEntry + lastEntry) / 2;
	if (pc >= symbols->unwind[middleEntry].region_start
	    && pc <= symbols->unwind[middleEntry].region_end)
	{
	    return &(symbols->unwind[middleEntry]);
	}
	
	if (pc < symbols->unwind[middleEntry].region_start)
            lastEntry = middleEntry - 1;
	else
            firstEntry = middleEntry + 1;
    }

    return NULL;
}              

// To see if this frame is valid to determine if this frame is the 
// innermost frame. Return true if it is, otherwise return false. 
bool 
frameChainValid(process *proc, int pc)
{
    pdFunction *funcStart = (proc->symbols)->findOneFunction("_start");
    
    if (funcStart) {
	if (pc >= funcStart->addr() && pc <= (funcStart->addr() + funcStart->size())) {   
	    return false;
	}
    }
    
    funcStart = (proc->symbols)->findOneFunction("$START$");
    
    if (funcStart) {
	if (pc >= funcStart->addr() && pc <= (funcStart->addr() + funcStart->size())) {   
	    return false;
	}
    }

    return true;
}


bool process::getActiveFrame(int *fp, int *pc)
{
    struct unwind_table_entry *u;
    char buf[4];
    bool err = true;
    int reg_sp;
    
    freeNotOK = false;
    
    // Get the value of PC from the reg no.33 which is ss_pcoq_head in
    // the structure save_state. </usr/include/machine/save_state.h>   
    if (ptraceKludge::deliverPtrace(this,PT_RUREGS,(char *)132,0, buf)) {
	*pc = *(int *)&buf[0];
	buf[3] &= ~0x3;
#ifdef DEBUG_STACK
	sprintf(errorLine, "getAF: PC = %x, ", *pc); 
	logLine(errorLine);
#endif
    }
    else err = false;
    
    // We get the value of frame pointer by finding out the frame size
    // of current frame. Then move the SP down to the head of next
    // frame.
    u = findUnwindEntry (symbols, *pc);
    if (!u) {
#ifdef DEBUG_STACK
	sprintf(errorLine, "Not getting any frame, check that!\n");
	logLine(errorLine);
#endif	
	freeNotOK = true;
	err = false;
    }
    
    if (ptraceKludge::deliverPtrace(this,PT_RUREGS,(char *)120,0, buf)) {
	reg_sp = *(int *)&buf[0];
	buf[3] &= ~0x3;
#ifdef DEBUG_STACK
	sprintf(errorLine, "SP  = %x, ", reg_sp);
	logLine(errorLine);
#endif
    }
    
    if (u -> HP_UX_interrupt_marker) {
#ifdef DEBUG_STACK
	sprintf(errorLine, "Interrupt frame, exit.\n");
	logLine(errorLine);
#endif
	freeNotOK = true;
	err = false;
    } else {
	*fp = reg_sp - ((u->Total_frame_size)<<3);
#ifdef DEBUG_STACK
	sprintf(errorLine, "FP = %x.\n", *fp);
	logLine(errorLine);
#endif
    }
    
    return err;

}


bool process::readDataFromFrame(int currentFP, int *fp, int *rtn)
{

    struct unwind_table_entry *u; 
    bool readOK=true;
    char buf[20];

    if (readDataSpace((caddr_t) (currentFP-20),
		      sizeof(int)*6, buf, true)) {
	// this is the previous PC
	*rtn = *(int *)&buf[0];
	//  use the infomatino in the unwind table instead.  
	//  *fp = *(int *)&buf[20];
	
	u = findUnwindEntry (symbols, *rtn);
	if (!u) {
#ifdef DEBUG_STACK
	    sprintf(errorLine, "Not getting any frame, check that!\n");
	    logLine(errorLine);
#endif
	    freeNotOK = true;
	    return false;
	}
	
	if (u -> HP_UX_interrupt_marker) {
#ifdef DEBUG_STACK
	    sprintf(errorLine, "Interrupt frame, exit.\n");
	    logLine(errorLine);
#endif
	    freeNotOK = true;
	    return false;
	}
	
	*fp = currentFP - ((u->Total_frame_size)<<3);

#ifdef DEBUG_STACK 
	sprintf(errorLine, "\t return PC = %x, frame pointer = %x.\n", *rtn, *fp);
	logLine(errorLine);
#endif
	// if we are in the outermost frame ,we should stop walking
	// stack by making fp=0.
	if (!frameChainValid(this, *rtn))  {
	    readOK=false;
	    *fp = 0;
	    *rtn = 0;
	}
    }
    else {
	readOK=false;
    }
    
    return(readOK);
}



bool ptraceKludge::deliverPtrace(process *p, int req, char *addr,
                                 int data, char *addr2) {
  bool halted;
  bool ret;
  int  valu;

  if (req != PT_DETACH) halted = haltProcess(p);
  if (( valu = P_ptrace(req, p->getPid(), (int) addr, data, (int)addr2)) == (-1))
    ret = false;
  else
    ret = true;

  if (req == PT_RUREGS) *(int *)&addr2[0] = valu;

  if (req != PT_DETACH) continueProcess(p, halted);
  return ret;
}

void ptraceKludge::continueProcess(process *p, const bool wasStopped) {
  if ((p->status() != neonatal) && (!wasStopped))
/* Choose either one of the following methods to continue a process.
 * The choice must be consistent with that in process::continueProc_ and OS::osStop.
 */
#ifndef PTRACE_ATTACH_DETACH
    if (P_ptrace(PT_CONTIN, p->pid, 1, SIGCONT, 0) == -1) {
#else
    if (P_ptrace(PT_DETACH, p->pid, 1, SIGCONT, 0) == -1) {
#endif
      cerr << "error in continueProcess\n";
      assert(0);
    }
}

// already setup on this FD.
// disconnect from controlling terminal 
void OS::osDisconnect(void) {
  //logLine("OS::osDisconnect not available");
}

bool OS::osAttach(pid_t process_id) {
  return (P_ptrace(PT_ATTACH, process_id, 0, 0, 0) != -1);
}

bool OS::osStop(pid_t pid) { 
/* Choose either one of the following methods for stopping a process, but not both. 
 * The choice must be consistent with that in process::continueProc_ 
 * and ptraceKludge::continueProcess
 */
#ifndef PTRACE_ATTACH_DETACH
	return (P_kill(pid, SIGSTOP) != -1); 
#else
	return (osAttach(pid));
#endif
}

// TODO dump core
bool OS::osDumpCore(pid_t pid, const string dumpTo) {
  logLine("dumpcore not available yet");
  showErrorCallback(47, "");
  return false;
}

bool OS::osForwardSignal (pid_t pid, int cont_status) {
  return (P_ptrace(PT_CONTIN, pid, 1, cont_status, 0) != -1);
}

void OS::osTraceMe(void) { P_ptrace(PT_SETTRC, 0, 0, 0, 0); }


// wait for a process to terminate or stop
int process::waitProcs(int *status) {
  return waitpid(0, status, WNOHANG);
}

// attach to an inferior process.
bool process::attach() {
  // we only need to attach to a process that is not our direct children.
  if (parent != 0) {
    return OS::osAttach(pid);
  }
  return true;
}


// TODO is this safe here ?
bool process::continueProc_() {
  int ret;

  if (!checkStatus()) 
    return false;
  ptraceOps++; ptraceOtherOps++;
/* choose either one of the following ptrace calls, but not both. 
 * The choice must be consistent with that in OS::osStop and
ptraceKludge::continueProcess.
 */
#ifndef PTRACE_ATTACH_DETACH
  ret = P_ptrace(PT_CONTIN, pid, 1, 0, 0);
#else
  ret = P_ptrace(PT_DETACH, pid, 1, SIGCONT, 0);
#endif
  return ret != -1;
}

// TODO ??
bool process::pause_() {
  if (!checkStatus()) 
    return false;
  ptraceOps++; ptraceOtherOps++;
  bool wasStopped = (status() == stopped);
  if (status() != neonatal && !wasStopped)
    return (loopUntilStopped());
  else
    return true;
}

bool process::detach_() {
  if (!checkStatus())
    return false;
  ptraceOps++; ptraceOtherOps++;
  return (ptraceKludge::deliverPtrace(this, PT_DETACH, (char*)1, SIGCONT, NULL));
}

// temporarily unimplemented, PTRACE_DUMPCORE is specific to sunos4.1
bool process::dumpCore_(const string coreFile) {
  if (!checkStatus()) 
    return false;
  ptraceOps++; ptraceOtherOps++;

  assert(0);
  errno = 0;
  // int ret = P_ptrace(request, pid, coreFile, 0, (char*) NULL);
  int ret = 0;
  assert(errno == 0);
  return ret;
}

bool process::writeTextWord_(caddr_t inTraced, int data) {
  if (!checkStatus()) 
    return false;
  ptraceBytes += sizeof(int); ptraceOps++;
  return (ptraceKludge::deliverPtrace(this, PT_WIUSER, inTraced, data, NULL));
}

bool process::writeTextSpace_(caddr_t inTraced, int amount, caddr_t inSelf) {
  if (!checkStatus()) 
    return false;
  ptraceBytes += amount; ptraceOps++;
  return (ptraceKludge::deliverPtrace(this, PT_WRTEXT, inTraced, amount, inSelf));
  // the amount is number of bytes, so we need to change it to number
  // of words first!!
  // assert((amount % sizeof(int))==0);     // Same changes in write..
  // amount = amount / sizeof(int);       // read.. procedures;
  // for (unsigned i = 0; i < amount; ++i) {
  //  int data; memcpy(&data, inSelf, sizeof data);
  //  if (!ptraceKludge::deliverPtrace(this, PT_WIUSER, inTraced, data, 0)) {
  //    return false;
  //  }
  //  inTraced += sizeof data;
  //  inSelf += sizeof data;
  //}
  //return true;
}

bool process::writeDataSpace_(caddr_t inTraced, int amount, caddr_t inSelf) {
  if (!checkStatus())
    return false;
  ptraceOps++; ptraceBytes += amount;
  return (ptraceKludge::deliverPtrace(this, PT_WRDATA, inTraced, amount, inSelf));

  //assert((amount % sizeof(int))==0);
  //amount = amount / sizeof(int);   
  //for (unsigned i = 0; i < amount; ++i) {
  //  int data; memcpy(&data, inSelf, sizeof data);
  //  if (!ptraceKludge::deliverPtrace(this, PT_WDUSER, inTraced, data, 0)) {
  //    return false;
  //  }
  //  inTraced += sizeof data;
  //  inSelf += sizeof data;
  //}
  //return true;
}

bool process::readDataSpace_(caddr_t inTraced, int amount, caddr_t inSelf) {
  if (!checkStatus())
    return false;
  ptraceOps++; ptraceBytes += amount;
  return (ptraceKludge::deliverPtrace(this, PT_RDDATA, inTraced, amount, inSelf));


  //int* self_ptr = (int *) ((void *) inSelf);
  //assert((amount % sizeof(int))==0);
  //amount = amount / sizeof(int);   
  //for (unsigned i = 0; i < amount; ++i) {
  //  int data = ptraceKludge::deliverPtrace(this, PT_RDUSER, inTraced, 0, 0);
  //  memcpy(self_ptr, &data, sizeof data);
  //  inTraced += sizeof data;
  //  self_ptr++;
  //}
  //return true;
}

bool process::loopUntilStopped() {
  /* make sure the process is stopped in the eyes of ptrace */
  OS::osStop(pid);
  bool isStopped = false;
  int waitStatus;
  while (!isStopped) {
    int ret = P_waitpid(pid, &waitStatus, WUNTRACED);
    if ((ret == -1 && errno == ECHILD) || (WIFEXITED(waitStatus))) {
      // the child is gone.
      //status_ = exited;
      handleProcessExit(this, WEXITSTATUS(waitStatus));
      return(false);
    }
    if (!WIFSTOPPED(waitStatus) && !WIFSIGNALED(waitStatus)) {
      printf("problem stopping process\n");
      assert(0);
    }
    int sig = WSTOPSIG(waitStatus);
    /* printf("signal is %d\n", sig); fflush(stdout); */
    if (sig == SIGSTOP) {
      isStopped = true;
    } else {
	if (sig > 0) {
	    if (P_ptrace(PT_CONTIN, pid, 1, WSTOPSIG(waitStatus), 0) == -1) {
		cerr << "Ptrace error\n";
		assert(0);
	    }
	}
      OS::osStop(pid);
    }
  }

  return true;
}


bool OS::osDumpImage(const string &, pid_t, Address)
{
  logLine("OS::osDumpImage not yet implemented");
  return false;
}


//
// dummy versions of OS statistics.
//
float OS::compute_rusage_cpu() { return(0.0); }
float OS::compute_rusage_sys() { return(0.0); }
float OS::compute_rusage_min() { return(0.0); }
float OS::compute_rusage_maj() { return(0.0); }
float OS::compute_rusage_swap() { return(0.0); }
float OS::compute_rusage_io_in() { return(0.0); }
float OS::compute_rusage_io_out() { return(0.0); }
float OS::compute_rusage_msg_send() { return(0.0); }
float OS::compute_rusage_sigs() { return(0.0); }
float OS::compute_rusage_vol_cs() { return(0.0); }
float OS::compute_rusage_inv_cs() { return(0.0); }
float OS::compute_rusage_msg_recv() { return(0.0); }

int getNumberOfCPUs()
{
  return(1);
}

