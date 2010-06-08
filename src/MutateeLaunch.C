/*
 * Copyright (c) 1996-2009 Barton P. Miller
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

#include "MutateeLaunch.h"

static pid_t fork_mutatee();
static int startNewProcessForAttach(const char *pathname, const char *argv[],
                                    FILE *outlog, FILE *errlog, bool attach);

bool getMutateeCmdLine(RunGroup *grp, std::string &cmd, std::vector<std::string> &args)
{
   
}


static const char** parseArgs(RunGroup *group,
                              char *logfilename, char *humanlogname,
                              bool verboseFormat, bool printLabels,
                              int debugPrint, char *pidfilename, char *mutatee_resumelog, int unique)
{
   std::vector<std::string> mutateeArgs;
   getOutput()->getMutateeArgs(mutateeArgs); // mutateeArgs is an output parameter
   const char **child_argv = new const char *[16 + (4 * group->tests.size()) +
                                              mutateeArgs.size()];
   assert(child_argv);
   int n = 0;
   child_argv[n++] = group->mutatee;
   if (logfilename != NULL) {
      child_argv[n++] = const_cast<char *>("-log");
      child_argv[n++] = logfilename;
   }
   if (humanlogname != NULL) {
      child_argv[n++] = const_cast<char *>("-humanlog");
      child_argv[n++] = humanlogname;
 }
   if (false == verboseFormat) {
      child_argv[n++] = const_cast<char *>("-q");
      // TODO I'll also want to pass a parameter specifying a file to write
      // postponed messages to
   }
   if (mutatee_resumelog) {
      child_argv[n++] = "-resumelog";
      child_argv[n++] = mutatee_resumelog;
   }
   if (unique) {
      static char buffer[32];
      snprintf(buffer, 32, "%d", unique);
      child_argv[n++] = "-unique";
      child_argv[n++] = buffer;
   }
   if (debugPrint != 0) {
      child_argv[n++] = const_cast<char *>("-verbose");
   }
   if (pidfilename != NULL) {
      child_argv[n++] = const_cast<char *>("-pidfile");
      child_argv[n++] = pidfilename;
   }
   for (int i = 0; i < group->tests.size(); i++) {
      if (shouldRunTest(group, group->tests[i])) {
         child_argv[n++] = const_cast<char*>("-run");
         child_argv[n++] = group->tests[i]->name;
      }
   }
   if (printLabels) {
      for (int i = 0; i < group->tests.size(); i++) {
         child_argv[n++] = const_cast<char *>("-label");
         child_argv[n++] = group->tests[i]->label;
      }
      child_argv[n++] = const_cast<char *>("-print-labels");
   }
   for (int i = 0; i < mutateeArgs.size(); i++) {
      child_argv[n++] = strdup(mutateeArgs[i].c_str());
   }
   child_argv[n] = NULL;   

   return child_argv;
}


#if !defined(os_windows_test)
#if !defined(os_linux_test)
static pid_t fork_mutatee() {
  return fork();
}
#else
static pid_t fork_mutatee() {
   /**
    * Perform a granchild fork.  This code forks off a child, that child
    * then forks off a granchild.  The original child then exits, making
    * the granchild's new parent init.  Both the original process and the
    * granchild then exit from this function.
    *
    * This works around a linux kernel bug in kernel version 2.6.9 to
    * 2.6.11, see https://www.dyninst.org/emails/2006/5678.html for
    * details.
    **/
   int status, result;
   pid_t gchild_pid, child_pid;
   int filedes[2];

   pipe(filedes);

   child_pid = fork();
   if (child_pid < 0) { // This is an error
      close(filedes[0]);
      close(filedes[1]);
      return child_pid;
   }

   if (child_pid) {
      //Read the grandchild pid from the child.
      do {
         result = read(filedes[0], &gchild_pid, sizeof(pid_t));
      } while (result == -1 && errno == EINTR);
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
#if defined(os_windows_test)
static int startNewProcessForAttach(const char *pathname, const char *argv[],
                                    FILE *outlog, FILE *errlog, bool attach) 
{
   // TODO Fix Windows code to work with log file
	LPCTSTR pipeName = "\\\\.\\pipe\\mutatee_signal_pipe";
	HANDLE mutatee_signal_pipe = CreateNamedPipe(pipeName,
		PIPE_ACCESS_INBOUND,
		PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
		1, // num instances
		1024, // read buffer
		1024, // write buffer
		5000, //timeout
		NULL); // security descriptor
	if(mutatee_signal_pipe == INVALID_HANDLE_VALUE)
	{
      fprintf(stderr, "*ERROR*: Unable to create pipe.\n");
      return -1;
	}
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
      LPTSTR lastErrorMsg;
      DWORD lastError = GetLastError();
      FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | 
                    FORMAT_MESSAGE_FROM_SYSTEM |
                    FORMAT_MESSAGE_IGNORE_INSERTS,
                    NULL,
                    lastError,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                    (LPTSTR)&lastErrorMsg,
                    0, NULL );
      fprintf(stderr, "CreateProcess failed: (%d) %s\n",
              lastError, lastErrorMsg);
      LocalFree(lastErrorMsg);
      
      return -1;
   }

	// Keep synchronization pattern the same as on Unix...
	BOOL conn_ok = ConnectNamedPipe(mutatee_signal_pipe, NULL) ?
		TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
	if(!conn_ok)
	{
		CloseHandle(mutatee_signal_pipe);
		return -1;
	}
	char mutatee_ready = 0x0;
	DWORD bytes_read;
	BOOL read_ok = ReadFile(mutatee_signal_pipe,
                           &mutatee_ready, // buffer
                           1,	// size
                           &bytes_read,
                           NULL); // not overlapped I/O
	if (!read_ok || (bytes_read != 1))
	{
		fprintf(stderr, "Couldn't read from mutatee pipe\n");
		DisconnectNamedPipe(mutatee_signal_pipe);
		CloseHandle(mutatee_signal_pipe);
		return -1;
	}
	if(mutatee_ready != 'T')
	{
		fprintf(stderr, "Got unexpected message from mutatee, aborting\n");
		DisconnectNamedPipe(mutatee_signal_pipe);
		CloseHandle(mutatee_signal_pipe);
		return -1;
	}
	DisconnectNamedPipe(mutatee_signal_pipe);
	CloseHandle(mutatee_signal_pipe);
   return pi.dwProcessId;
}

#else

static int startNewProcessForAttach(const char *pathname, const char *argv[],
                                    FILE *outlog, FILE *errlog, bool attach)
{
   /* Make a pipe that we will use to signal that the mutatee has started. */
   int fds[2];
   char fdstr[32];
   if (attach) {
      if (pipe(fds) != 0) {
         fprintf(stderr, "*ERROR*: Unable to create pipe.\n");
         return -1;
      }
      /* Create the argv string for the child process. */
      sprintf(fdstr, "%d", fds[1]);
   }

   int i;
   for (i = 0; argv[i] != NULL; i++) ;
   // i now contains the count of elements in argv
   const char **attach_argv = (const char**)malloc(sizeof(char *) * (i + 3));
   // attach_argv is length i + 3 (including space for NULL)

   for (i = 0; argv[i] != NULL; i++)
      attach_argv[i] = argv[i];
   if (attach)
   {
      attach_argv[i++] = const_cast<char*>("-attach");
      attach_argv[i++] = fdstr;
   }
   attach_argv[i++] = NULL;
   
   int pid;
   if (attach)
      pid = fork_mutatee();
   else 
      pid = fork();
   if (pid == 0) {
      // child
      if (attach) 
         close(fds[0]); // We don't need the read side
      if (outlog != NULL) {
         int outlog_fd = fileno(outlog);
         if (dup2(outlog_fd, 1) == -1) {
            fprintf(stderr, "Error duplicating log fd(1)\n");
         }
      }
      if (errlog != NULL) {
         int errlog_fd = fileno(errlog);
         if (dup2(errlog_fd, 2) == -1) {
            fprintf(stderr, "Error duplicating log fd(2)\n");
         }
      }
      char *ld_path = getenv("LD_LIBRARY_PATH");
      char *new_ld_path = NULL;
      if (ld_path) {
         new_ld_path = (char *) malloc(strlen(ld_path) + strlen(binedit_dir) + 4);
         strcpy(new_ld_path, "./");
         strcat(new_ld_path, binedit_dir);
         strcat(new_ld_path, ":");
         strcat(new_ld_path, ld_path);
      }
      else {
         new_ld_path = (char *) malloc(strlen(binedit_dir) + 4);
         strcpy(new_ld_path, "./");
         strcat(new_ld_path, binedit_dir);
         strcat(new_ld_path, ":");
      }
      setenv("LD_LIBRARY_PATH", new_ld_path, 1);

	  //fprintf(stderr, "%s[%d]:  before exec '%s':  LD_LIBRARY_PATH='%s'\n", 
	  //FILE__, __LINE__, pathname, getenv("LD_LIBRARY_PATH"));

      execvp(pathname, (char * const *)attach_argv);
      char *newname = (char *) malloc(strlen(pathname) + 3);
      strcpy(newname, "./");
      strcat(newname, pathname);
      execvp(newname, (char * const *)attach_argv);
      logerror("%s[%d]:  Exec failed!\n", FILE__, __LINE__);
      exit(-1);
   } else if (pid < 0) {
      return -1;
   }

   registerPID(pid);

   // parent
   if (attach) {
      close(fds[1]);  // We don't need the write side
      
      // Wait for the child to write to the pipe
      char ch;
      if (read(fds[0], &ch, sizeof(char)) != sizeof(char)) {
         perror("read");
         fprintf(stderr, "*ERROR*: Error reading from pipe\n");
         return -1;
      }

      if (ch != 'T') {
         fprintf(stderr, "*ERROR*: Child didn't write expected value to pipe.\n");
         return -1;
      }
       
#if defined( os_linux_test )
      /* Random Linux-ism: it's possible to close the pipe before the 
         mutatee returns from the write() system call matching the above
         read().  In this case, rather than ignore the close() because the
         write() is on its way out the kernel, Linux sends a SIGPIPE
         to the mutatee, which causes us no end of trouble.  read()ing
         an EOF from the pipe seems to alleviate this problem, and seems
         more reliable than a sleep(1).  The condition test if we somehow
         got any /extra/ bytes on the pipe. */
      if( read( fds[0], & ch, sizeof( char ) ) != 0 ) {
         fprintf(stderr, "*ERROR*: Shouldn't have read anything here.\n");
         return -1;
      }
#endif /* defined( os_linux_test ) */
      
      close( fds[0] ); // We're done with the pipe
   }
   return pid;
#endif
}
