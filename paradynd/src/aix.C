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

/* 
 * aix.C,v
 * Revision 1.10  1996/05/12  05:12:57  tamches
 * (really Jeff)
 * Added ability to process aix 4.1-linked files.
 *
 * Revision 1.9  1996/05/08 23:54:33  mjrg
 * added support for handling fork and exec by an application
 * use /proc instead of ptrace on solaris
 * removed warnings
 *
 * Revision 1.8  1996/04/06 21:25:24  hollings
 * Fixed inst free to work on AIX (really any platform with split I/D heaps).
 * Removed the Line class.
 * Removed a debugging printf for multiple function returns.
 *
 * Revision 1.7  1996/03/12  20:48:16  mjrg
 * Improved handling of process termination
 * New version of aggregateSample to support adding and removing components
 * dynamically
 * Added error messages
 *
 * Revision 1.6  1996/02/13 16:23:22  hollings
 * Move Object class constructors to this file.
 *
 * Revision 1.5  1996/02/12  16:46:06  naim
 * Updating the way we compute number_of_cpus. On solaris we will return the
 * number of cpus; on sunos, hp, aix 1 and on the CM-5 the number of processes,
 * which should be equal to the number of cpus - naim
 *
 */


#include "util/h/headers.h"
#include "os.h"
#include "process.h"
#include "symtab.h"
#include "stats.h"
#include "util/h/Types.h"
#include "util/h/Object.h"

#include <sys/ioctl.h>
#include <fcntl.h>
#include <assert.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <xcoff.h>
#include <scnhdr.h>
#include <sys/time.h>
#include <sys/reg.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <sys/ptrace.h>

#include "showerror.h"

extern "C" {
extern int ioctl(int, int, ...);
};


unsigned AIX_TEXT_OFFSET_HACK;
unsigned AIX_DATA_OFFSET_HACK;

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

// what is the top most frame.
static int firstFrame;

//
// return the current frame pointer (fp) and program counter (pc). 
//    returns true if we are able to read the regsiters.
//
bool process::getActiveFrame(int *fp, int *pc)
{
    int sp;
    bool ret;
    int dummy;

    errno = 0;
    sp = ptrace(PT_READ_GPR, pid, (int *) STKP, 0, 0); // aix 4.1 likes int *
    if (errno != 0) return false;

    errno = 0;
    *pc = ptrace(PT_READ_GPR, pid, (int *) IAR, 0, 0); // aix 4.1 likes int *
    if (errno != 0) return false;

    // now we need to read the first frame from memory.
    //   The first frame pointer in in the memory location pointed to be sp
    //   However, there is no pc stored there, its the place to store a pc
    //      if the current function makes a call.
    ret = readDataFromFrame(sp, fp, &dummy);
    firstFrame = *fp;
    return(ret);
}

//
// given the pointer to a frame (currentFP), return 
//     (1) the saved frame pointer (fp)
//            NULL -> that currentFP is the bottom (last) frame.	
//     (2) the return address of the function for that frame (rtn).
//     (3) return true if we are able to read the frame.
//
bool process::readDataFromFrame(int currentFP, int *fp, int *rtn)
{
    //
    // define the linkage area of an activation record.
    //    This information is based on the data obtained from the
    //    info system (search for link area). - jkh 4/5/96
    //
    struct {
        unsigned oldFp;
        unsigned savedCR;
        unsigned savedLR;
	unsigned compilerInfo;
	unsigned binderInfo;
	unsigned savedTOC;
    } linkArea;

    if (readDataSpace((caddr_t) currentFP, 
		      sizeof(linkArea), 
		      (caddr_t) &linkArea,
		      false)) {
        *fp = linkArea.oldFp;
        *rtn = linkArea.savedLR;
        if (currentFP == firstFrame) {
            // use the value stored in the link register instead.
            errno = 0;
            *rtn = ptrace(PT_READ_GPR, pid, (int *)LR, 0, 0); // aix 4.1 likes int *
            if (errno != 0) return false;
        }
        return true;
    } else {
        return false;
    }
}

bool ptraceKludge::deliverPtrace(process *p, int req, char *addr,
				 int data, char *addr2) {
  bool halted;
  bool ret;
  
  
  if (req != PT_DETACH) halted = haltProcess(p);
  if (ptrace(req, p->getPid(), (int *)addr, data, (int *)addr2) == -1) // aix 4.1 likes int *
    ret = false;
  else
    ret = true;
  if (req != PT_DETACH) continueProcess(p, halted);
  return ret;
}

void ptraceKludge::continueProcess(process *p, const bool wasStopped) {
  if ((p->status() != neonatal) && (!wasStopped)) {

/* Choose either one of the following methods to continue a process.
 * The choice must be consistent with that in process::continueProc_ and 
 * OS::osStop.
 */

#if !defined(PTRACE_ATTACH_DETACH)
    if (ptrace(PT_CONTINUE, p->pid, (int *) 1, SIGCONT, NULL) == -1) {
#else
    if (ptrace(PT_DETACH, p->pid, (int *) 1, SIGCONT, NULL) == -1) { 
    // aix 4.1 likes int *
#endif
      cerr << "error in continueProcess\n";
      assert(0);
    }
  }
}

// already setup on this FD.
// disconnect from controlling terminal 
void OS::osDisconnect(void) {
  int ttyfd = open ("/dev/tty", O_RDONLY);
  ioctl (ttyfd, TIOCNOTTY, NULL); 
  close (ttyfd);
}

bool OS::osAttach(pid_t process_id) {
  int ret;

  ret = ptrace(PT_ATTACH, process_id, (int *)0, 0, 0);
  if (ret == -1) {
      ret = ptrace(PT_REATT, process_id, (int *)0, 0, 0);
  }
  return (ret != -1);
}

bool OS::osStop(pid_t pid) { 
/* Choose either one of the following methods for stopping a process, 
 * but not both. 
 * The choice must be consistent with that in process::continueProc_ 
 * and ptraceKludge::continueProcess
 */
#if !defined(PTRACE_ATTACH_DETACH)
	return (P_kill(pid, SIGSTOP) != -1); 
#else
	// attach generates a SIG TRAP which we catch
	if (!osAttach(pid)) {
          assert(kill(pid, SIGSTOP) != -1);
        }
        return(true);
#endif
}

// TODO dump core
bool OS::osDumpCore(pid_t pid, const string dumpTo) {
  // return (!ptrace(PT_DUMPCORE, pid, dumpTo, 0, 0));
  logLine("dumpcore not available yet");
  showErrorCallback(47, "");
  return false;
}

bool OS::osForwardSignal (pid_t pid, int stat) {
#if defined(PTRACE_ATTACH_DETACH)
  if (stat != 0) {
      ptrace(PT_DETACH, pid, (int*)1, stat, 0); // aix 4.1 likes int *
      return (true);
  } else {
      return (ptrace(PT_CONTINUE, pid, (int*)1, 0, 0) != -1); // aix 4.1 likes int *
  }
#else
  return (ptrace(PT_CONTINUE, pid, (int*)1, stat, NULL) != -1);
#endif
}

void OS::osTraceMe(void) { ptrace(PT_TRACE_ME, 0, 0, 0, 0); }


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

/* Choose either one of the following methods to continue a process.
 * The choice must be consistent with that in process::continueProc_ and 
 * OS::osStop.
 */

#if !defined(PTRACE_ATTACH_DETACH)
  // switch these to not detach after every call.
  ret = ptrace(PT_CONTINUE, pid, (int *)1, 0, NULL);
#else
  ret = ptrace(PT_DETACH, pid, (int *)1, SIGCONT, NULL);
#endif

  return (ret != -1);
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
  if (checkStatus()) {
      ptraceOps++; ptraceOtherOps++;
      if (!ptraceKludge::deliverPtrace(this,PT_DETACH,(char*)1,SIGSTOP, NULL)) {
	  sprintf(errorLine, "Unable to detach %d\n", getPid());
	  logLine(errorLine);
	  showErrorCallback(40, (const char *) errorLine);
      }
  }
  // always return true since we report the error condition.
  return (true);
}

// temporarily unimplemented, PT_DUMPCORE is specific to sunos4.1
bool process::dumpCore_(const string coreFile) {
  if (!checkStatus()) 
    return false;
  ptraceOps++; ptraceOtherOps++;

  assert(OS::osDumpImage(symbols->file(), pid, symbols->codeOffset()));
  errno = 0;
  (void) ptrace(PT_CONTINUE, pid, (int*)1, SIGBUS, NULL);
  assert(errno == 0);
  return true;
}

bool process::writeTextWord_(caddr_t inTraced, int data) {
  if (!checkStatus()) 
    return false;
  ptraceBytes += sizeof(int); ptraceOps++;
  return (ptraceKludge::deliverPtrace(this, PT_WRITE_I, inTraced, data, NULL));
}

bool process::writeTextSpace_(caddr_t inTraced, int amount, caddr_t inSelf) {
  if (!checkStatus()) 
    return false;
  ptraceBytes += amount; ptraceOps++;
  return (ptraceKludge::deliverPtrace(this, PT_WRITE_BLOCK, inTraced, amount, inSelf));
}

bool process::writeDataSpace_(caddr_t inTraced, int amount, caddr_t inSelf) {
  if (!checkStatus())
    return false;
  ptraceOps++; ptraceBytes += amount;
  return (ptraceKludge::deliverPtrace(this, PT_WRITE_BLOCK, inTraced, amount, inSelf));
}

bool process::readDataSpace_(caddr_t inTraced, int amount, caddr_t inSelf) {
  if (!checkStatus())
    return false;
  ptraceOps++; ptraceBytes += amount;
  return (ptraceKludge::deliverPtrace(this, PT_READ_BLOCK, inTraced, amount, inSelf));
}

bool process::loopUntilStopped() {
  /* make sure the process is stopped in the eyes of ptrace */
  assert(OS::osStop(pid));
  bool isStopped = false;
  int waitStatus;
  while (!isStopped) {
    int ret = waitpid(pid, &waitStatus, WUNTRACED);
    if ((ret == -1) && (errno == EINTR)) continue;
    if ((ret == -1 && errno == ECHILD) || (WIFEXITED(waitStatus))) {
      // the child is gone.
      //status_ = exited;
      handleProcessExit(this, WEXITSTATUS(waitStatus));
      return(false);
    }
    if (!WIFSTOPPED(waitStatus) && !WIFSIGNALED(waitStatus)) {
      printf("problem stopping process\n");
      return false;
    }
    int sig = WSTOPSIG(waitStatus);
    if ((sig == SIGTRAP) || (sig == SIGSTOP) || (sig == SIGINT)) {
      isStopped = true;
    } else {
      if (ptrace(PT_CONTINUE, pid, (int*)1, WSTOPSIG(waitStatus), 0) == -1) {
	logLine("Ptrace error in PT_CONTINUE, loopUntilStopped\n");
        return false;
      }
    }
  }

  return true;
}


//
// Write out the current contents of the text segment to disk.  This is useful
//    for debugging dyninst.
//
bool OS::osDumpImage(const string &imageFileName,  int pid, const Address codeOff)
{
    int i;
    int rd;
    int ifd;
    int ofd;
    int cnt;
    int ret;
    int total;
    int length;
    Address baseAddr;
    extern int errno;
    char buffer[4096];
    char outFile[256];
    struct filehdr hdr;
    struct stat statBuf;
    struct aouthdr aout;
    struct scnhdr *sectHdr;
    bool needsCont = false;
    struct ld_info info[64];

    ifd = open(imageFileName.string_of(), O_RDONLY, 0);
    if (ifd < 0) {
      sprintf(errorLine, "Unable to open %s\n", outFile);
      logLine(errorLine);
      showErrorCallback(41, (const char *) errorLine);
      perror("open");
      return true;
    }

    rd = fstat(ifd, &statBuf);
    if (rd != 0) {
      perror("fstat");
      sprintf(errorLine, "Unable to stat %s\n", outFile);
      logLine(errorLine);
      showErrorCallback(72, (const char *) errorLine);
      return true;
    }
    length = statBuf.st_size;
    sprintf(outFile, "%s.real", imageFileName.string_of());
    sprintf(errorLine, "saving program to %s\n", outFile);
    logLine(errorLine);

    ofd = open(outFile, O_WRONLY|O_CREAT, 0777);
    if (ofd < 0) {
      perror("open");
      exit(-1);
    }

    /* read header and section headers */
    cnt = read(ifd, &hdr, sizeof(struct filehdr));
    if (cnt != sizeof(struct filehdr)) {
	sprintf(errorLine, "Error reading header\n");
	logLine(errorLine);
	showErrorCallback(44, "");
	return false;
    }

    cnt = read(ifd, &aout, sizeof(struct aouthdr));

    sectHdr = (struct scnhdr *) calloc(sizeof(struct scnhdr), hdr.f_nscns);
    cnt = read(ifd, sectHdr, sizeof(struct scnhdr) * hdr.f_nscns);
    if ((unsigned) cnt != sizeof(struct scnhdr)* hdr.f_nscns) {
	sprintf(errorLine, "section headers\n");
	logLine(errorLine);
	return false;
    }

    /* now copy the entire file */
    lseek(ofd, 0, SEEK_SET);
    lseek(ifd, 0, SEEK_SET);
    for (i=0; i < length; i += 4096) {
        rd = read(ifd, buffer, 4096);
        write(ofd, buffer, rd);
        total += rd;
    }

    if (!stopped) {
        // make sure it is stopped.
        osStop(pid);
        waitpid(pid, NULL, WUNTRACED);
	needsCont = true;
    }

    ret = ptrace(PT_LDINFO, pid, (int *) &info, sizeof(info), (int *) &info);
    if (ret != 0) {
	statusLine("Unable to get loader info about process");
	showErrorCallback(43, "Unable to get loader info about process");
	return false;
    }

    baseAddr = info[0].ldinfo_textorg + aout.text_start;
    sprintf(errorLine, "seeking to %d as the offset of the text segment \n",
	aout.text_start);
    logLine(errorLine);
    sprintf(errorLine, " code offset= %d\n", baseAddr);
    logLine(errorLine);

    /* seek to the text segment */
    lseek(ofd, aout.text_start, SEEK_SET);
    for (i=0; i < aout.tsize; i+= 1024) {
        errno = 0;
        length = ((i + 1024) < aout.tsize) ? 1024 : aout.tsize -i;
        ptrace(PT_READ_BLOCK, pid, (int*) (baseAddr + i), length, (int *)buffer);
        if (errno) {
	    perror("ptrace");
	    assert(0);
        }
	write(ofd, buffer, length);
    }

    if (needsCont) {
	ptrace(PT_CONTINUE, pid, (int*) 1, SIGCONT, 0);
    }

    close(ofd);
    close(ifd);

    return true;
}

//
// Seek to the desired offset and read the passed length of the file
//   into dest.  If any errors are detected, log a message and return false.
//
bool seekAndRead(int fd, int offset, void **dest, int length, bool allocate)
{
    int cnt;

    if (allocate) {
	*dest = malloc(length);
    }

    if (!*dest) {
	sprintf(errorLine, "Unable to parse executable file\n");
	logLine(errorLine);
	showErrorCallback(42, (const char *) errorLine);
	return false;
    }

    cnt = lseek(fd, offset, SEEK_SET);
    if (cnt != offset) {
        sprintf(errorLine, "Unable to parse executable file\n");
	logLine(errorLine);
	showErrorCallback(42, (const char *) errorLine);
	return false;
    }
    cnt = read(fd, *dest, length);
    if (cnt != length) {
        sprintf(errorLine, "Unable to parse executable file\n");
	logLine(errorLine);
	showErrorCallback(42, (const char *) errorLine);
	return false;
    }
    return true;
}

void Object::load_object()
{
    long i;
    int fd;
    int cnt;
    string name;
    string module;
    unsigned value;
    int poolOffset;
    int poolLength;
    union auxent *aux;
    struct filehdr hdr;
    struct syment *sym;
    struct aouthdr aout;
    union auxent *csect;
    char *stringPool=NULL;
    bool newModule = false;
    Symbol::SymbolType type; 
    int *lengthPtr = &poolLength;
    struct syment *symbols = NULL;
    struct scnhdr *sectHdr = NULL;
    Symbol::SymbolLinkage linkage;

    fd = open(file_.string_of(), O_RDONLY, 0);
    if (fd <0) {
        sprintf(errorLine, "Unable to open executable file %s\n", 
	    file_.string_of());
	statusLine(errorLine);
	showErrorCallback(27,(const char *) errorLine);
	goto cleanup;
    }

    cnt = read(fd, &hdr, sizeof(struct filehdr));
    if (cnt != sizeof(struct filehdr)) {
        sprintf(errorLine, "Error reading executable file %s\n", 
	    file_.string_of());
	statusLine(errorLine);
	showErrorCallback(49,(const char *) errorLine);
	goto cleanup;
    }

    cnt = read(fd, &aout, sizeof(struct aouthdr));
    if (cnt != sizeof(struct aouthdr)) {
        sprintf(errorLine, "Error reading executable file %s\n", 
	    file_.string_of());
	statusLine(errorLine);
	showErrorCallback(49,(const char *) errorLine);
	goto cleanup;
    }

    sectHdr = (struct scnhdr *) malloc(sizeof(struct scnhdr) * hdr.f_nscns);
    assert(sectHdr);
    cnt = read(fd, sectHdr, sizeof(struct scnhdr) * hdr.f_nscns);
    if ((unsigned) cnt != sizeof(struct scnhdr)* hdr.f_nscns) {
        sprintf(errorLine, "Error reading executable file %s\n", 
	    file_.string_of());
	statusLine(errorLine);
	showErrorCallback(49,(const char *) errorLine);
	goto cleanup;
    }

    // fprintf(stderr, "symbol table has %d entries starting at %d\n",
    //    (int) hdr.f_nsyms, (int) hdr.f_symptr);

    if (!seekAndRead(fd, hdr.f_symptr, (void**) &symbols, 
	hdr.f_nsyms * SYMESZ, true)) {
	goto cleanup;
    }

    /*
     * Get the string pool
     */
    poolOffset = hdr.f_symptr + hdr.f_nsyms * SYMESZ;
    /* length is stored in the first 4 bytes of the string pool */
    if (!seekAndRead(fd, poolOffset, (void**) &lengthPtr, sizeof(int), false)) {
	goto cleanup;
    }

    if (!seekAndRead(fd, poolOffset, (void**) &stringPool, poolLength, true)) {
	goto cleanup;
    }

    // identify the code region.
    if ((unsigned) aout.tsize != sectHdr[aout.o_sntext-1].s_size) {
	// consistantcy check failed!!!!
        sprintf(errorLine, 
	    "Executable header file interal error: text segment size %s\n", 
	    file_.string_of());
	statusLine(errorLine);
	showErrorCallback(45,(const char *) errorLine);
	goto cleanup;
    }

    if (!seekAndRead(fd, sectHdr[aout.o_sntext-1].s_scnptr, 
	(void **) &code_ptr_, aout.tsize, true)) {
	goto cleanup;
    }

    //code_off_ =  aout.text_start + AIX_TEXT_OFFSET_HACK; (OLD, pre-4.1)
     code_off_ =  aout.text_start;
     if (aout.text_start < TEXTORG) {
       code_off_ += AIX_TEXT_OFFSET_HACK;
     } else {
       AIX_TEXT_OFFSET_HACK = 0;
     }


    // fprintf(stderr, "reading code starting at %x\n", code_off_);

    code_len_ = aout.tsize;

    // now the init data segment.
    if ((unsigned) aout.dsize != sectHdr[aout.o_sndata-1].s_size) {
	// consistantcy check failed!!!!
        sprintf(errorLine, 
	    "Executable header file interal error: data segment size %s\n", 
	    file_.string_of());
	statusLine(errorLine);
	showErrorCallback(45,(const char *) errorLine);
	goto cleanup;
    }
    if (!seekAndRead(fd, sectHdr[aout.o_sndata-1].s_scnptr, 
	(void **) &data_ptr_, aout.dsize, true)) {
	goto cleanup;
    }

    // data_off_ = sectHdr[aout.o_sndata-1].s_vaddr + AIX_DATA_OFFSET_HACK; (OLD, pre-4.1)
    data_off_ = aout.data_start;
    if (aout.data_start < DATAORG) {
       data_off_ += AIX_DATA_OFFSET_HACK;
    } else {
       AIX_DATA_OFFSET_HACK = 0;
    }

    data_len_ = aout.dsize;

    for (i=0; i < hdr.f_nsyms; i++) {
	/* do the pointer addition by hand since sizeof(struct syment)
         *   seems to be 20 not 18 as it should be */
        sym = (struct syment *) (((unsigned) symbols) + i * SYMESZ);
        if (!(sym->n_sclass & DBXMASK)) {
	    if ((sym->n_sclass == C_HIDEXT) || 
		(sym->n_sclass == C_EXT) ||
		(sym->n_sclass == C_FILE)) {
		if (!sym->n_zeroes) {
		    name = string(&stringPool[sym->n_offset]);
		} else {
		    char tempName[9];
		    memset(tempName, 0, 9);
		    strncpy(tempName, sym->n_name, 8);
		    name = string(tempName);
		}
	    }
	    
	    if ((sym->n_sclass == C_HIDEXT) || (sym->n_sclass == C_EXT)) {
		if (sym->n_sclass == C_HIDEXT) {
		    linkage = Symbol::SL_LOCAL;
	        } else {
		    linkage = Symbol::SL_GLOBAL;
		}

		if (sym->n_scnum == aout.o_sntext) {
		    type = Symbol::PDST_FUNCTION;
		    // XXX - Hack for AIX loader.
		    value = sym->n_value + AIX_TEXT_OFFSET_HACK;
	        } else {
		    // bss or data
		    csect = (union auxent *)
			((char *) sym + sym->n_numaux * SYMESZ);
		    if ((csect->x_csect.x_smclas == XMC_TC) ||
		        (csect->x_csect.x_smclas == XMC_DS)) {
			// table of contents related entry not a real symbol.
			continue;
		    }
		    type = Symbol::PDST_OBJECT;
		    // XXX - Hack for AIX loader.
		    value = sym->n_value + AIX_DATA_OFFSET_HACK;
		}


		if (newModule) {
		    // modules are defined multiple times for xlf Fortran.
		    if (symbols_.defines(module)) {
			Symbol &oldValue = symbols_[module];
			// symbols should be in assending order.
			if (oldValue.addr() > value) {
			    logLine("Symbol table out of order, use -Xlinker -bnoobjreorder");
			    showErrorCallback(48, "");
			    goto cleanup;
			}
		    } else {
			symbols_[module] = Symbol(module, module, 
			    Symbol::PDST_MODULE, linkage, value, false);
		    }
		    newModule = false;
		}

		// XXXX - Hack to make names match assumptions of symtab.C
		if (name.prefixed_by(".")) {
		    char temp[512];
		    sprintf(temp, "_%s", &name.string_of()[1]);
		    name = string(temp);
		} else if (type == Symbol::PDST_FUNCTION) {
		    // text segment without a leady . is a toc item
		    continue;
		}

		//fprintf(stderr, "Found symbol %s in (%s) at %x\n", 
		//   name.string_of(), module.string_of(), value);
		symbols_[name] = Symbol(name, module, type, linkage, 
			    value, false);
	    } else if (sym->n_sclass == C_FILE) {
		if (!strcmp(name.string_of(), ".file")) {
		    int j;
		    /* has aux record with additional information. */
		    for (j=1; j <= sym->n_numaux; j++) {
			aux = (union auxent *) ((char *) sym + j * SYMESZ);
			if (aux->x_file._x.x_ftype == XFT_FN) {
			    // this aux record contains the file name.
			    if (!aux->x_file._x.x_zeroes) {
				name = 
				  string(&stringPool[aux->x_file._x.x_offset]);
			    } else {
				// x_fname is 14 bytes
				char tempName[14];
				memset(tempName, 0, 15);
				strncpy(tempName, aux->x_file.x_fname, 14);
				name = string(tempName);
			    }
			}
		    }
		}
		// fprintf(stderr, "Found module %s\n", name.string_of());
		// mark it to be added, but don't add it until the next symbol
		//    tells us the address.
		newModule = true;
		module = name;
		continue;
	    }
        }
    }

cleanup:
    close(fd);
    if (sectHdr) free(sectHdr);
    if (stringPool) free(stringPool);
    if (symbols) free(symbols);

    return;
}


Object::Object(const string file, void (*err_func)(const char *))
    : AObject(file, err_func) {
    load_object();
}

Object::Object(const Object& obj)
    : AObject(obj) {
    load_object();
}

Object::~Object() { }

Object& Object::operator=(const Object& obj) {
    (void) AObject::operator=(obj);
    return *this;
}

//
// Verify that that program is statically linked, and establish the text 
//   and data segments starting addresses.
//
bool establishBaseAddrs(int pid, int &status)
{
    int ret;
    struct ld_info *ptr;
    struct ld_info info[64];

    // check that the program was loaded at the correct address.

    /* It seems that AIX has some timing problems and
     when the user stack grows, the kernel doesn't update the stack info in time
     and ptrace calls step on user stack. This is the reason why call sleep 
     here, giving the kernel some time to update its internals. */
    usleep (36000);

    // wait for the TRAP point.
    waitpid(pid, &status, WUNTRACED);


    ret = ptrace(PT_LDINFO, pid, (int *) &info, sizeof(info), (int *) &info);
    if (ret != 0) {
	statusLine("Unable to get loader info about process, application aborted");
	showErrorCallback(43, "Unable to get loader info about process, application aborted");
	return false;
    }

    ptr = info;
    if (ptr->ldinfo_next) {
	statusLine("ERROR: program not statically linked");
	logLine("ERROR: program not statically linked");
	showErrorCallback(46, "Program not statically linked");
	return false;
    }

    // now check addr.
    AIX_TEXT_OFFSET_HACK = (unsigned) ptr->ldinfo_textorg + 0x200;
    AIX_DATA_OFFSET_HACK = (unsigned) ptr->ldinfo_dataorg;

    return true;
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

