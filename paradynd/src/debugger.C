
/*
 * $Log: debugger.C,v $
 * Revision 1.3  1994/06/22 03:46:30  markc
 * Removed compiler warnings.
 *
 */

//
// support for debugger style commands and interface.
//
#include <stdio.h>
#include <sgtty.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <a.out.h>
#include <sys/stat.h>

#include "symtab.h"
#include "process.h"
#include "rtinst/h/rtinst.h"
#include "inst.h"
#include "instP.h"
#include "dyninst.h"
#include "dyninstP.h"

process *defaultProcess;

// <sts/ptrace.h> should really define this. 
extern "C" {
int ptrace(enum ptracereq request, 
		      int pid, 
		      int *addr, 
		      int data, 
		      char *addr2);
}

void changeDefaultProcess(int pid)
{
    process *nProc;

    nProc = findProcess(pid);
    if (!nProc) {
	printf("unable to find process %d\n", pid);
    } else {
	defaultProcess = nProc;
    }
}

void changeDefaultThread(int tid)
{
    int basePid;
    process *nProc;

    if (processList.count()) {
	basePid = (*processList)->pid;
    } else {
	fprintf(stderr, "no process defined to take thread of\n");
	return;
    }

    nProc = findProcess(tid*MAXPID+basePid);
    if (!nProc) {
	printf("unable to find thread %d\n", tid);
    } else {
	defaultProcess = nProc;
    }
}

process *getDefaultProcess()
{
    if (!defaultProcess) changeDefaultThread(0);
    return(defaultProcess);
}

void dumpProcessImage(process *proc, Boolean stopped)
{
    int i;
    int rd;
    int ifd;
    int ofd;
    int total;
    int length;
    struct exec exec;
    char buffer[4096];
    char outFile[256];
    extern int errno;
    struct stat statBuf;

    if (proc->pid > MAXPID) {
	printf("only able to dump for UNIX processes now\n");
	return;
    }

    ifd = open(proc->symbols->file, O_RDONLY, 0);
    if (ifd < 0) {
	perror("open");
	exit(-1);
    }

    rd = read(ifd, (char *) &exec, sizeof(struct exec));
    if (rd != sizeof(struct exec)) {
	perror("read");
	exit(-1);
    }

    rd = fstat(ifd, &statBuf);
    if (rd != 0) {
	perror("fstat");
	exit(-1);
    }
    length = statBuf.st_size;
    sprintf(outFile, "%s.real", proc->symbols->file);
    printf("saving program to %s\n", outFile);

    ofd = open(outFile, O_WRONLY|O_CREAT, 0777);
    if (ofd < 0) {
	perror("open");
	exit(-1);
    }

    /* now copy the rest */

    lseek(ofd, 0, SEEK_SET);
    write(ofd, (char *) &exec, sizeof(exec));

    if (!stopped) {
	// make sure it is stopped.
	kill(proc->pid, SIGSTOP);
	waitpid(proc->pid, NULL, WUNTRACED);
    }

    lseek(ofd, N_TXTOFF(exec), SEEK_SET);
    for (i=0; i < exec.a_text; i+= 4096) {
	errno = 0;
	ptrace(PTRACE_READTEXT, proc->pid, (int*) proc->symbols->textOffset+i, 
	    4096, buffer);
	if (errno) {
	     perror("ptrace");
	     abort();
	}
	write(ofd, buffer, 4096);
    }
    ptrace(PTRACE_CONT, proc->pid, (int *) 1, SIGCONT, 0);

    rd = lseek(ofd, N_DATOFF(exec), SEEK_SET);
    if (rd != N_DATOFF(exec)) {
        perror("lseek");
        exit(-1);
    }

    rd = lseek(ifd, N_DATOFF(exec), SEEK_SET);
    if (rd != N_DATOFF(exec)) {
        perror("lseek");
        exit(-1);
    }

    total = N_DATOFF(exec);
    for (i=N_DATOFF(exec); i < length; i += 4096) {
        rd = read(ifd, buffer, 4096);
        write(ofd, buffer, rd);
        total += rd;
    }
    if (total != length) {
        printf("tried to write %d bytes, only %d written\n", length, total);
    }

    close(ofd);
    close(ifd);
}
