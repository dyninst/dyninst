/*
 * Copyright (c) 1996 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

#include "dyninstAPI/src/symtab.h"
#include "util/h/headers.h"
#include "dyninstAPI/src/os.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/stats.h"
#include "util/h/Types.h"
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/termios.h>
#include <unistd.h>
#include "dyninstAPI/src/showerror.h"
#include "util/h/pathName.h" // concat_pathname_components()
#include "util/h/debugOstream.h"
#include "util/h/solarisKludges.h"

#if defined (sparc_sun_solaris2_4)
#include "dyninstAPI/src/inst-sparc.h"
#else
#include "dyninstAPI/src/inst-x86.h"
#endif

#include "instPoint.h"

#include <sys/procfs.h>
//#include <poll.h>
#include <limits.h>
#include <link.h>
#include <dlfcn.h>

#define DLOPEN_MODE (RTLD_NOW | RTLD_GLOBAL)

extern "C" {
extern int ioctl(int, int, ...);
extern long sysconf(int);
};

// The following were defined in process.C
extern debug_ostream attach_cerr;
extern debug_ostream inferiorrpc_cerr;
extern debug_ostream shmsample_cerr;
extern debug_ostream forkexec_cerr;
extern debug_ostream metric_cerr;
extern debug_ostream signal_cerr;

/*
   Define the indices of some registers to be used with pr_reg.
   These values are different on sparc and x86 platforms.
   RETVAL_REG: the registers that holds the return value of calls ($o0 on sparc,
               %eax on x86).
   PC_REG: program counter
   FP_REG: frame pointer (%i7 on sparc, %ebp on x86) 
*/
#ifdef sparc_sun_solaris2_4
#define RETVAL_REG (R_O0)
#define PC_REG (R_PC)
#define FP_REG (R_O6)
#endif
#ifdef i386_unknown_solaris2_5
#define RETVAL_REG (EAX)
#define PC_REG (EIP)
#define FP_REG (EBP)
#endif


extern bool isValidAddress(process *proc, Address where);
extern void generateBreakPoint(instruction &insn);

/*
   osTraceMe is called after we fork a child process to set
   a breakpoint on the exit of the exec system call.
   When /proc is used, this breakpoint **will not** cause a SIGTRAP to 
   be sent to the process. The parent should use PIOCWSTOP to wait for 
   the child.
*/
void OS::osTraceMe(void) {

  return;
}


// already setup on this FD.
// disconnect from controlling terminal 
void OS::osDisconnect(void) {

}

bool process::continueWithForwardSignal(int) {
   return true;
}

bool process::dumpImage() {return false;}


/* 
   execResult: return the result of an exec system call - true if succesful.
   The traced processes will stop on exit of an exec system call, just before
   returning to user code. At this point the return value (errno) is already
   written to a register, and we need to check if the return value is zero.
 */
static inline bool execResult(prstatus_t stat) {
  return 0;
}

/*
   wait for inferior processes to terminate or stop.
*/
int process::waitProcs(int *status) {
   return 0;
}


static char *extract_string_ptr(int procfd, char **ptr) {
   // we want to return *ptr.
   return 0;

}

string extract_string(int procfd, const char *inferiorptr) {
   // assuming inferiorptr points to a null-terminated string in the inferior
   // process, extract it and return it.
   return 0;
}

bool get_ps_stuff(int proc_fd, string &argv0, string &pathenv, string &cwdenv) {
   // Use ps info to obtain argv[0], PATH, and curr working directory of the
   // inferior process designated by proc_fd.  Writes to argv0, pathenv, cwdenv.
   return true;
}

/*
   Open the /proc file correspoding to process pid, 
   set the signals to be caught to be only SIGSTOP and SIGTRAP,
   and set the kill-on-last-close and inherit-on-fork flags.
*/
extern string pd_flavor ;
bool process::attach() {
  return true;
}

#if defined(USES_LIBDYNINSTRT_SO)
bool process::trapAtEntryPointOfMain()
{

  return(false);
}

bool process::trapDueToDyninstLib()
{
  return(false);
}

void process::handleIfDueToDyninstLib() 
{

}

void process::handleTrapAtEntryPointOfMain()
{

}

void process::insertTrapAtEntryPointOfMain()
{

}

bool process::dlopenDYNINSTlib() {
  // we will write the following into a buffer and copy it into the
  // application process's address space
  // [....LIBRARY's NAME...|code for DLOPEN]

  // write to the application at codeOffset. This won't work if we
  // attach to a running process.
  //Address codeBase = this->getImage()->codeOffset();
  // ...let's try "_start" instead
  return true;
}

unsigned process::get_dlopen_addr() const {
    return(0);
}
#endif

bool process::isRunning_() const {
   // determine if a process is running by doing low-level system checks, as
   // opposed to checking the 'status_' member vrble.  May assume that attach()
   // has run, but can't assume anything else.
      return true;
}

bool process::attach_() {assert(false);}
bool process::stop_() {assert(false);}

/* 
   continue a process that is stopped 
*/
bool process::continueProc_() {
  return true;
}

#ifdef BPATCH_LIBRARY
/*
   terminate execution of a process
 */
bool process::terminateProc_()
{
	return true;
}
#endif

/*
   pause a process that is running
*/
bool process::pause_() {
 return 0;
}

/*
   close the file descriptor for the file associated with a process
*/
bool process::detach_() {

  return true;
}

#ifdef BPATCH_LIBRARY
/*
   detach from thr process, continuing its execution if the parameter "cont"
   is true.
 */
bool process::API_detach_(const bool cont)
{

  return true;
}
#endif

bool process::dumpCore_(const string) {
  return false;
}

bool process::writeTextWord_(caddr_t inTraced, int data) {
  return false;
}

bool process::writeTextSpace_(void *inTraced, int amount, const void *inSelf) {
//  cerr << "writeTextSpace pid=" << getPid() << ", @ " << (void *)inTraced << " len=" << amount << endl; cerr.flush();
  return false;
}

#ifdef BPATCH_SET_MUTATIONS_ACTIVE
bool process::readTextSpace_(void *inTraced, int amount, const void *inSelf) {
  return false;
}
#endif

bool process::writeDataSpace_(void *inTraced, int amount, const void *inSelf) {
  return 0;
}

bool process::readDataSpace_(const void *inTraced, int amount, void *inSelf) {
  return 0;
}

bool process::loopUntilStopped() {
  assert(0);
}

#ifdef notdef
// TODO -- only call getrusage once per round
static struct rusage *get_usage_data() {
  return NULL;
}
#endif

float OS::compute_rusage_cpu() {
  return 0;
}

float OS::compute_rusage_sys() {
  return 0;
}

float OS::compute_rusage_min() {
  return 0;
}
float OS::compute_rusage_maj() {
  return 0;
}

float OS::compute_rusage_swap() {
  return 0;
}
float OS::compute_rusage_io_in() {
  return 0;
}
float OS::compute_rusage_io_out() {
  return 0;
}
float OS::compute_rusage_msg_send() {
  return 0;
}
float OS::compute_rusage_msg_recv() {
  return 0;
}
float OS::compute_rusage_sigs() {
  return 0;
}
float OS::compute_rusage_vol_cs() {
  return 0;
}
float OS::compute_rusage_inv_cs() {
  return 0;
}

int getNumberOfCPUs()
{
    return(1);
}  


bool process::getActiveFrame(int *fp, int *pc)
{
  return 0;
}


bool process::readDataFromFrame(int currentFP, int *fp, int *rtn, bool uppermost)
{
 


  return false;
}



#ifdef SHM_SAMPLING
time64 process::getInferiorProcessCPUtime() {
   // returns user+sys time from the u or proc area of the inferior process, which in
   // turn is presumably obtained by mmapping it (sunos) or by using a /proc ioctl
   // to obtain it (solaris).  It must not stop the inferior process in order
   // to obtain the result, nor can it assue that the inferior has been stopped.
   // The result MUST be "in sync" with rtinst's DYNINSTgetCPUtime().

   // We use the PIOCUSAGE /proc ioctl

   // Other /proc ioctls that should work too: PIOCPSINFO
   // and the lower-level PIOCGETPR and PIOCGETU which return copies of the proc
   // and u areas, respectively.
   // PIOCSTATUS does _not_ work because its results are not in sync
   // with DYNINSTgetCPUtime

   time64 result;
   return result;
}
#endif

void *process::getRegisters() {
   // Astonishingly, this routine can be shared between solaris/sparc and
   // solaris/x86.  All hail /proc!!!
   // assumes the process is stopped (/proc requires it)

   return 0;
}

bool process::executingSystemCall() {
   return(false);
}

bool process::changePC(unsigned addr, const void *savedRegs) {
   return true;
}

bool process::changePC(unsigned addr) {
   return true;
}

bool process::restoreRegisters(void *buffer) {
   // The fact that this routine can be shared between solaris/sparc and
   // solaris/x86 is just really, really cool.  /proc rules!

   return true;
}

#ifdef i386_unknown_solaris2_5

bool process::readDataFromFrame(int currentFP, int *fp, int *rtn, bool )
{
  return(readOK);
}

#endif

#ifdef i386_unknown_solaris2_5
// ******** TODO **********
bool process::needToAddALeafFrame(Frame , Address &) {
  return false;
}

#else

// needToAddALeafFrame: returns true if the between the current frame 
// and the next frame there is a leaf function (this occurs when the 
// current frame is the signal handler and the function that was executing
// when the sighandler was called is a leaf function)
bool process::needToAddALeafFrame(Frame current_frame, Address &leaf_pc){


   return false;
}
#endif

string process::tryToFindExecutable(const string &iprogpath, int pid) {
   // returns empty string on failure.
   // Otherwise, returns a full-path-name for the file.  Tries every
   // trick to determine the full-path-name, even though "progpath" may be
   // unspecified (empty string).
   
   // Remember, we can always return the empty string...no need to
   // go nuts writing the world's most complex algorithm.
   return "";
}

bool process::set_breakpoint_for_syscall_completion() {
   /* Can assume: (1) process is paused and (2) in a system call.
      We want to set a TRAP for the syscall exit, and do the
      inferiorRPC at that time.  We'll use /proc PIOCSEXIT.
      Returns true iff breakpoint was successfully set. */
   return true;
}

unsigned process::read_inferiorRPC_result_register(reg) {
      return 0; // assert(false)?
}


// hasBeenBound: returns true if the runtime linker has bound the
// function symbol corresponding to the relocation entry in at the address
// specified by entry and base_addr.  If it has been bound, then the callee 
// function is returned in "target_pdf", else it returns false.
bool process::hasBeenBound(const relocationEntry entry, 
			   pd_Function *&target_pdf, Address base_addr) {

  return false;

}



// findCallee: finds the function called by the instruction corresponding
// to the instPoint "instr". If the function call has been bound to an
// address, then the callee function is returned in "target" and the 
// instPoint "callee" data member is set to pt to callee's function_base.  
// If the function has not yet been bound, then "target" is set to the 
// function_base associated with the name of the target function (this is 
// obtained by the PLT and relocation entries in the image), and the instPoint
// callee is not set.  If the callee function cannot be found, (ex. function
// pointers, or other indirect calls), it returns false.
// Returns false on error (ex. process doesn't contain this instPoint).
//
// The assumption here is that for all processes sharing the image containing
// this instPoint they are going to bind the call target to the same function. 
// For shared objects this is always true, however this may not be true for
// dynamic executables.  Two a.outs can be identical except for how they are
// linked, so a call to fuction foo in one version of the a.out may be bound
// to function foo in libfoo.so.1, and in the other version it may be bound to 
// function foo in libfoo.so.2.  We are currently not handling this case, since
// it is unlikely to happen in practice.
bool process::findCallee(instPoint &instr, function_base *&target){

    return false;  
}











