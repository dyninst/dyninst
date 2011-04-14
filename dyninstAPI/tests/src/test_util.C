/*
 * Copyright (c) 1996-2011 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
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

//
// $Id: test_util.C,v 1.30 2008/04/11 23:30:40 legendre Exp $
// Utility functions for use by the dyninst API test programs.
//

#include <stdio.h>
#include <signal.h>
#include <sys/types.h>

#if defined(i386_unknown_nt4_0) || defined(mips_unknown_ce2_11) //ccw 10 apr 2001 
#ifndef mips_unknown_ce2_11 //ccw 10 apr 2001
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#else
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>
#endif

#if defined(os_linux)
#include <linux/limits.h>
#endif

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "dyninstAPI_RT/h/dyninstAPI_RT.h" // for DYNINST_BREAKPOINT_SIGNUM

//
// Wait for the mutatee to stop.
//
void waitUntilStopped(BPatch *bpatch, BPatch_thread *appThread, int testnum,
                      const char *testname)
{
    while (!appThread->isStopped() && !appThread->isTerminated())
        bpatch->waitForStatusChange();
    
    if (!appThread->isStopped()) {
        printf("**Failed test #%d (%s)\n", testnum, testname);
        printf("    process did not signal mutator via stop\n");
        printf("thread is not stopped\n");
        if (appThread->isTerminated())
          printf("thread is terminated\n");
        exit(-1);
    }
#if defined(i386_unknown_nt4_0)  || defined(mips_unknown_ce2_11) //ccw 10 apr 2001
    else if (appThread->stopSignal() != EXCEPTION_BREAKPOINT && appThread->stopSignal() != -1) {
	printf("**Failed test #%d (%s)\n", testnum, testname);
	printf("    process stopped on signal %d, not SIGTRAP\n", 
		appThread->stopSignal());
	exit(-1);
    }
#else
#ifdef DETACH_ON_THE_FLY
    /* FIXME: Why add SIGILL here? */
    else if ((appThread->stopSignal() != SIGSTOP) &&
	     (appThread->stopSignal() != SIGHUP) &&
	     (appThread->stopSignal() != DYNINST_BREAKPOINT_SIGNUM) &&
	     (appThread->stopSignal() != SIGILL)) {
#else
    else if ((appThread->stopSignal() != SIGSTOP) &&
	     (appThread->stopSignal() != DYNINST_BREAKPOINT_SIGNUM) &&
#if defined(bug_irix_broken_sigstop)
	     (appThread->stopSignal() != SIGEMT) &&
#endif
	     (appThread->stopSignal() != SIGHUP)) {
#endif /* DETACH_ON_THE_FLY */
	printf("**Failed test #%d (%s)\n", testnum, testname);
	printf("    process stopped on signal %d, not SIGSTOP(%d)\n", 
		appThread->stopSignal(), SIGSTOP);
	exit(-1);
    }
#endif
}


//
// Signal the child that we've attached.  The child contains a function
// "checkIfAttached" which simply returns the value of the global variable
// "isAttached."  We add instrumentation to "checkIfAttached" to set
// "isAttached" to 1.
//
void signalAttached(BPatch_thread* /*appThread*/, BPatch_image *appImage)
{
    BPatch_variableExpr *isAttached = appImage->findVariable("isAttached");
    if (isAttached == NULL) {
	printf("*ERROR*: unable to start tests because variable \"isAttached\""
               " could not be found in the child process\n");
	exit(-1);
    }

    int yes = 1;
    isAttached->writeValue(&yes);
}

#if !defined(os_windows)
#if !defined(os_linux)
pid_t fork_mutatee() {
   return fork();
}
#else
pid_t fork_mutatee() {
   /**
    * Perform a granchild fork.  This code forks off a child, that child
    * then forks off a granchild.  The original child then exits, making
    * the granchild's new parent init.  Both the original process and the
    * granchild then exit from this function.
    *
    * This works around a linux kernel bug in kernel version 2.6.9 to
    * 2.6.11, see https://www.dyninst.org/emails/2006/5676.html for
    * details.
    **/
   int status, result;
   pid_t gchild_pid, child_pid;
   int filedes[2];

   pipe(filedes);

   child_pid = fork();
   if (child_pid < 0) {
      close(filedes[0]);
      close(filedes[1]);
      return child_pid;
   }

   if (child_pid) {
      //Read the grandchild pid from the child.
      result = read(filedes[0], &gchild_pid, sizeof(pid_t));
      if (result == -1) {
         perror("Couldn't read from pipe");
      }

      int options = 0;
      do {
         result = waitpid(child_pid, &status, options);
         if (result != child_pid) {
            perror("Couldn't join child");
            break;
         }
      } while (!WIFEXITED(status));
      close(filedes[0]);
      close(filedes[1]);
      return gchild_pid;
   }
   //Child
   
   gchild_pid = fork();
   if (gchild_pid) {
      //Child pid, send grand child pid to parent then terminate
      result = write(filedes[1], &gchild_pid, sizeof(pid_t));
      if (result == -1) {
         perror("Couldn't write to parent");
      }         
      close(filedes[0]);
      close(filedes[1]);
      exit(0);
   }   

   //Granchild
   close(filedes[0]);
   close(filedes[1]);
   return 0;
}
#endif
#endif

//
// Create a new process and return its process id.  If process creation 
// fails, this function returns -1.
//
int startNewProcessForAttach(const char *pathname, const char *argv[])
{
#if defined(i386_unknown_nt4_0)  || defined(mips_unknown_ce2_11) //ccw 10 apr 2001
    char child_args[1024];
    strcpy(child_args, "");
    if (argv[0] != NULL) {
	strcpy(child_args, pathname);
	for (int i = 1; argv[i] != NULL; i++) {
	    strcat(child_args, " ");
	    strcat(child_args, argv[i]);
	}	    
	strcat(child_args, " -attach");
    }

    STARTUPINFO si;
    memset(&si, 0, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);
    PROCESS_INFORMATION pi;
    if (!CreateProcess(pathname,	// application name
		       child_args,	// command line
		       NULL,		// security attributes
		       NULL,		// thread security attributes
		       FALSE,		// inherit handles
		       0,		// creation flags
		       NULL,		// environment,
		       NULL,		// current directory
		       &si,
		       &pi)) {
	return -1;
    }

    return pi.dwProcessId;
#else
    /* Make a pipe that we will use to signal that the mutatee has started. */
    int fds[2];
    if (pipe(fds) != 0) {
	fprintf(stderr, "*ERROR*: Unable to create pipe.\n");
	exit(-1);
    }

    /* Create the argv string for the child process. */
    char fdstr[32];
    sprintf(fdstr, "%d", fds[1]);

    int i;
    for (i = 0; argv[i] != NULL; i++) ;
    const char **attach_argv = (const char**)malloc(sizeof(char *) * (i + 3));

    for (i = 0; argv[i] != NULL; i++)
	attach_argv[i] = argv[i];
    attach_argv[i++] = const_cast<char*>("-attach");
    attach_argv[i++] = fdstr;
    attach_argv[i++] = NULL;

    int pid = fork_mutatee();
    if (pid == 0) {
	// child
	close(fds[0]); // We don't need the read side
	execvp(pathname, (char * const *)attach_argv);
	exit(-1);
    } else if (pid < 0) {
	return -1;
    }

    // parent
    close(fds[1]);  // We don't need the write side

    // Wait for the child to write to the pipe
    char ch;
    if (read(fds[0], &ch, sizeof(char)) != sizeof(char)) {
	perror("read");
	fprintf(stderr, "*ERROR*: Error reading from pipe\n");
	exit(-1);
    }

    if (ch != 'T') {
	fprintf(stderr, "*ERROR*: Child didn't write expected value to pipe.\n");
	exit(-1);
    }

    close(fds[0]);  // We're done with the pipe

    return pid;
#endif
}

void updateSearchPaths(const char *filename) {
#if !defined(os_windows)
   // First, find the directory we reside in

   char *execpath;
   char pathname[PATH_MAX];
   getcwd(pathname, PATH_MAX);

   if (filename[0] == '/') {
      // If it begins with a slash, it's an absolute path
      execpath = strdup(filename);
      strrchr(execpath,'/')[0] = '\0';
   } else if (strchr(filename,'/')) {
      // If it contains slashes, it's a relative path
      char *filename_copy = strdup(filename);
      
      char *loc = strrchr(filename_copy, '/');
      if (loc)
         *loc = '\0';
#if 0
      *strrchr(filename_copy,'/') = '\0';
#endif
      execpath = (char *) ::malloc(strlen(pathname) + strlen(filename_copy) + 2);
      strcpy(execpath,pathname);
      strcat(execpath,"/");
      strcat(execpath,filename_copy);
      ::free(filename_copy);
   } else {
      // If it's just a name, it was found in PATH
      const char *pathenv = getenv("PATH");
      char *pathenv_copy = strdup(pathenv);
      char *ptrptr;
      char *nextpath = strtok_r(pathenv_copy, ":", &ptrptr);
      while (nextpath) {
         struct stat statbuf;
         
         char *fullpath = new char[strlen(nextpath)+strlen(filename)+2];
         strcpy(fullpath,nextpath);
         strcat(fullpath,"/");
         strcat(fullpath,filename);
         
         if (!stat(fullpath,&statbuf)) {
            execpath = strdup(nextpath);
            delete[] fullpath;
            break;
         }
         delete[] fullpath;
         nextpath = strtok_r(NULL,":", &ptrptr);
      }
      ::free(pathenv_copy);
      
      if (nextpath == NULL) {
          // Not found in PATH - we'll assume its in CWD
          return;
      }
   }

   // Now update PATH and LD_LIBRARY_PATH/LIBPATH

    char *envCopy;

    char *envPath = getenv("PATH");
    envCopy = (char *) ::malloc(((envPath && strlen(envPath)) ? strlen(envPath) + 1 : 0) + strlen(execpath) + 6);
    strcpy(envCopy, "PATH=");
    if (envPath && strlen(envPath)) {
       strcat(envCopy, envPath);
       strcat(envCopy, ":");
    } 
    strcat(envCopy, execpath);
    assert(!putenv(envCopy));
    
    char *envLibPath;
#if defined(os_aix)
    envLibPath = getenv("LIBPATH");
#else
    envLibPath = getenv("LD_LIBRARY_PATH");
#endif
    
    envCopy = (char *) ::malloc(((envLibPath && strlen(envLibPath)) ? strlen(envLibPath) + 1 : 0) + strlen(execpath) + 17);
#if defined(os_aix)
    strcpy(envCopy, "LIBPATH=");
#else
    strcpy(envCopy, "LD_LIBRARY_PATH=");
#endif
    if (envLibPath && strlen(envLibPath)) {
       strcat(envCopy, envLibPath);
       strcat(envCopy, ":");
    }
    strcat(envCopy, execpath);
    assert(!putenv(envCopy));

    ::free(execpath);
#endif
}
