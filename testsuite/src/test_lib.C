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
// $Id: test_lib.C,v 1.6 2008/10/30 19:16:54 legendre Exp $
// Utility functions for use by the dyninst API test programs.
//

#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <errno.h>
#include <assert.h>
#include <stdlib.h>
#include <limits.h>

#include <string>
#include <vector>
using namespace std;

#if !defined(i386_unknown_nt4_0_test)
#include <fnmatch.h>
#endif

#if defined(i386_unknown_nt4_0_test) || defined(mips_unknown_ce2_11_test) //ccw 10 apr 2001 
#ifndef mips_unknown_ce2_11_test //ccw 10 apr 2001
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#else
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif
#include <stdarg.h>

// Blind inclusion from test9.C
#if defined(i386_unknown_linux2_0_test) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */
#include <sys/types.h>
#include <sys/wait.h>
#endif

#if defined(i386_unknown_linux2_0_test) \
 || defined(x86_64_unknown_linux2_4)
#include <unistd.h>
#endif
// end inclusion from test9.C

#if defined(i386_unknown_nt4_0_test)
#define snprintf _snprintf
#endif

#include "test_lib.h"
#include "ResumeLog.h"
#define BINEDIT_DIRNAME "" 


/* Control Debug printf statements */
int debugPrint = 0;

// output logging
FILE *outlog = NULL;
FILE *errlog = NULL;
const char *outlogname = "-";
const char *errlogname = "-";

static char *binedit_dir = BINEDIT_BASENAME;

char *get_binedit_dir()
{
	return binedit_dir;
}

void set_binedit_dir(char *d)
{
	binedit_dir = d;
}

static char *resumelog_name = "resumelog";
char *get_resumelog_name() {
	return resumelog_name;
}

void set_resumelog_name(char *s) {
	resumelog_name = s;
}


LocErr::LocErr(const char *__file__, const int __line__, const std::string msg) :
	msg__(msg),
	file__(std::string(__file__)),
	line__(__line__)
{}

LocErr::~LocErr() THROW
{}

std::string LocErr::file() const
{
	return file__;
}

std::string LocErr::msg() const
{
	return msg__;
}
const char * LocErr::what() const
{
	return msg__.c_str();
}
int LocErr::line() const
{
	return line__;
}

void LocErr::print(FILE * /*stream*/) const
{
	logerror( "Error thrown from %s[%d]:\n\t\"%s\"\n",
			file__.c_str(), line__, what());
}

std::vector<std::string> Tempfile::all_open_files;

Tempfile::Tempfile()
{
#if defined (os_windows_test)
	fname = new char[1024];
	assert(fname);
	const char *dyninst_root = getenv("DYNINST_ROOT");
	char tmp_dir[1024];
	struct stat statbuf;

	if (!dyninst_root)
	{
      dyninst_root = "../..";
	}

	snprintf(tmp_dir, 1024, "%s\temp", dyninst_root);

	if (0 != stat(tmp_dir, &statbuf))
	{
		if (ENOENT == errno)
		{
			//  doesn't exist, make it
			if (0 != _mkdir(tmp_dir))
			{
				fprintf(stderr, "%s[%d]:  mkdir(%s): %s\n", __FILE__, __LINE__, tmp_dir, strerror(errno));
				abort();
			}
		}
		else
		{
			fprintf(stderr, "%s[%d]:  FIXME:  unexpected stat result: %s\n",
					__FILE__, __LINE__, strerror(errno));
			abort();
		}
	}

	if (0 != GetTempFileName(tmp_dir, "tempfile", 0, fname))
	{
		fprintf(stderr, "%s[%d]:  failed to create temp file name\n", __FILE__, __LINE__);
		assert(0);
	}

	fd = CreateFile(fname,
			GENERIC_READ | GENERIC_WRITE, // open r-w 
			0,                    // do not share 
			NULL,                 // default security 
			CREATE_ALWAYS,        // overwrite existing
			FILE_ATTRIBUTE_NORMAL,// normal file 
			NULL);                // no template 

	if (fd == INVALID_HANDLE_VALUE)
	{
		fprintf(stderr, "%s[%d]:  failed to create temp file\n", __FILE__, __LINE__);
		assert(0);
	}
#else
	fname = strdup("/tmp/tmpfileXXXXXX");
	fd = mkstemp(fname);

	if (-1 == fd)
	{
		fprintf(stderr, "%s[%d]:  failed to make temp file\n", __FILE__, __LINE__);
		abort();
	}
#endif
	all_open_files.push_back(std::string(fname));
}

Tempfile::~Tempfile()
{
#if defined (os_windows_test)
	if (0 == DeleteFile(fname))
	{
		fprintf(stderr, "%s[%d]:  DeleteFile failed: %s\n",
				__FILE__, __LINE__, strerror(errno));
	}
	delete [] fname;
#else
	logerror( "%s[%d]:  unlinking %s\n", FILE__, __LINE__, fname);
	if (0 != unlink (fname))
	{
		fprintf(stderr, "%s[%d]:  unlink failed: %s\n",
				__FILE__, __LINE__, strerror(errno));
	}
	free (fname);
#endif
}

const char *Tempfile::getName()
{
	return fname;
}

void Tempfile::deleteAll()
{
	for (unsigned int i = (all_open_files.size() - 1); i > 0; --i)
	{
		const char *fn = all_open_files[i].c_str();
		assert(fn);
#if defined (os_windows_test)
		if (0 == DeleteFile(fn))
		{
			fprintf(stderr, "%s[%d]:  DeleteFile failed: %s\n",
					__FILE__, __LINE__, strerror(errno));
		}
#else
		fprintf(stderr, "%s[%d]:  unlinking %s\n", FILE__, __LINE__, fn);
		if (0 != unlink (fn))
		{
			fprintf(stderr, "%s[%d]:  unlink failed: %s\n",
					__FILE__, __LINE__, strerror(errno));
		}
#endif
	}
	all_open_files.clear();
}

TestOutputDriver * output = NULL;

// windows has strange support for sharing variables across
// a dll and a program, so here is a simple utility function to do that, since
// FUNCTION definitions are easily shared.
TestOutputDriver * getOutput() {
	return output;
}

	void setOutput(TestOutputDriver * new_output) {
		if (output != NULL)
			delete output;
		output = new_output;
	}

void setOutputLog(FILE *log_fp) {
	if (log_fp != NULL) {
		outlog = log_fp;
	} else {
		outlog = stdout;
	}
}

FILE *getOutputLog() {
	return outlog;
}

void setErrorLog(FILE *log_fp) {
	if (log_fp != NULL) {
		errlog = log_fp;
	} else {
		errlog = stderr;
	}
}

FILE *getErrorLog() {
	return errlog;
}

void setOutputLogFilename(char *log_fn) {
	if (log_fn != NULL) {
		outlogname = log_fn;
	}
}

void setErrorLogFilename(char *log_fn) {
	if (log_fn != NULL) {
		errlogname = log_fn;
	}
}

const char *getOutputLogFilename() {
	return outlogname;
}

const char *getErrorLogFilename() {
	return errlogname;
}

void logstatus(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	getOutput()->vlog(LOGINFO, fmt, args);
	va_end(args);
}

void logerror(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	getOutput()->vlog(LOGERR, fmt, args);
	va_end(args);
}

void flushOutputLog() {
	if (outlog != NULL) {
		fflush(outlog);
	}
}
void flushErrorLog() {
	if (errlog != NULL) {
		fflush(errlog);
	}
}

// PID registration for mutatee cleanup
// TODO Check if these make any sense on Windows.  I suspect I'll need to
// change them.
char *pidFilename = NULL;
void setPIDFilename(char *pfn) {
	pidFilename = pfn;
}
char *getPIDFilename() {
	return pidFilename;
}
void registerPID(int pid) {
	if (NULL == pidFilename) {
		return;
	}
	FILE *pidFile = fopen(pidFilename, "a");
	if (NULL == pidFile) {
		//fprintf(stderr, "[%s:%u] - Error registering mutatee PID: unable to open PID file\n", __FILE__, __LINE__);
	} else {
     fprintf(pidFile, "%d\n", pid);
     fclose(pidFile);
  }
}

void cleanPIDFile()
{
	if(!pidFilename)
		return;
   FILE *f = fopen(pidFilename, "r");
   if (!f)
      return;
   for (;;)
   {
      int pid;
      int res = fscanf(f, "%d\n", &pid);
      if (res != 1)
         break;
#if !defined(os_windows_test)
      kill(pid, SIGKILL);
#else
      HANDLE h = OpenProcess(PROCESS_ALL_ACCESS, false, pid);
      if (h == NULL) {
         return;
      }
      bool dummy = TerminateProcess(h,0);
      CloseHandle(h);
#endif
   }
   fclose(f);
   f = fopen(pidFilename, "w");
   fclose(f);
}

void setDebugPrint(int debug) {
   debugPrint = debug;
}

#if !defined(os_windows_test)
#if !defined(os_linux_test)
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

   if( pipe(filedes) == -1 ) {
       perror("pipe");
       return -1;
   }

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

bool inTestList(test_data_t &test, std::vector<char *> &test_list)
{
   for (unsigned int i = 0; i < test_list.size(); i++ )
   {
#if defined(i386_unknown_nt4_0_test)
      if ( strcmp(test_list[i], test.name) == 0 )
#else
      if ( fnmatch(test_list[i], test.name, 0) == 0 )
#endif
      {
         return true;
      }
   }

   return false;
}

//
// Create a new process and return its process id.  If process creation 
// fails, this function returns -1.
//
int startNewProcessForAttach(const char *pathname, const char *argv[],
                             FILE *outlog, FILE *errlog, bool attach) {
#if defined(os_windows_test)
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

   registerPID(pi.dwProcessId);
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
#else
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
      ld_path = new_ld_path;

      vector<string> attach_argv;
      char *platform = getenv("PLATFORM");
      bool bgp_test = strcmp(platform ? platform : "", "ppc32_bgp") == 0;
      if (bgp_test) {
         //attach_argv.push_back("mpirun");
         char *partition = getenv("DYNINST_BGP_PARTITION");
         //if(partition == NULL) partition = "BGB1";
         if (partition) {
            attach_argv.push_back("-nofree");
            attach_argv.push_back("-partition");
            attach_argv.push_back(string(partition));
         }
         attach_argv.push_back("-np");
         attach_argv.push_back("1");
         attach_argv.push_back("-env");
         
         char *cwd_cstr = (char *) malloc(PATH_MAX);
         getcwd(cwd_cstr, PATH_MAX);
         string cwd = cwd_cstr;
         free(cwd_cstr);
         
         char *dyninst_base_cstr = realpath(string(cwd + "/../../../").c_str(), NULL);
         string dyninst_base = dyninst_base_cstr;
         free(dyninst_base_cstr);
         
         char *ld_lib_path = getenv("LD_LIBRARY_PATH");
         string ldpath = "LD_LIBRARY_PATH=" + dyninst_base + "/dyninst/testsuite/" + 
            platform + "/binaries:.:" + dyninst_base + "/" + platform + "/lib";
         if (ld_lib_path)
            ldpath += string(":") + ld_lib_path;
         attach_argv.push_back(ldpath);
         
         attach_argv.push_back("-cwd");
         attach_argv.push_back(cwd + BINEDIT_DIRNAME);
         
         attach_argv.push_back("-exe");
         attach_argv.push_back(pathname);
         attach_argv.push_back("-args");
         
         string args = "\"";
         for (unsigned j=0; argv[j] != NULL; j++) {
            args += argv[j];
            if (argv[j+1])
               args += " ";
         }
         args += "\"";
         attach_argv.push_back(args);
      }
      else {
         for (unsigned int i=0; argv[i] != NULL; i++) {
            attach_argv.push_back(argv[i]);
         }
			if (attach) {
         	attach_argv.push_back(const_cast<char *>("-attach"));
         	attach_argv.push_back(fdstr);
			}
      }
      char **attach_argv_cstr = (char **) malloc((attach_argv.size()+1) * sizeof(char *));
      for (unsigned int i=0; i<attach_argv.size(); i++) {
         attach_argv_cstr[i] = const_cast<char *>(attach_argv[i].c_str());
      }
      attach_argv_cstr[attach_argv.size()] = NULL;

      if(bgp_test) {
         
         dprintf("running mpirun:");
         for (int i = 0 ; i < attach_argv.size(); i++)
            dprintf(" %s", attach_argv_cstr[i]);
         dprintf("\n");
         
         execvp("mpirun", (char * const *)attach_argv_cstr);
         logerror("%s[%d]:  Exec failed!\n", FILE__, __LINE__);
         exit(-1);
      }else{
         execvp(pathname, (char * const *)attach_argv_cstr);
         char *newname = (char *) malloc(strlen(pathname) + 3);
         strcpy(newname, "./");
         strcat(newname, pathname);
         execvp(newname, (char * const *)attach_argv_cstr);
         logerror("%s[%d]:  Exec failed!\n", FILE__, __LINE__);
         exit(-1);
      }

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


// control debug printf statements
void dprintf(const char *fmt, ...) {
   va_list args;
   va_start(args, fmt);

   if(debugPrint)
      vfprintf(stderr, fmt, args);

   va_end(args);

   fflush(stderr);
}

// Build Architecture specific libname
// FIXME Is this used any more?  Is it necessary?
void addLibArchExt(char *dest, unsigned int dest_max_len, int psize, bool isStatic)
{
   int dest_len;

   dest_len = strlen(dest);

   // Patch up alternate ABI filenames
#if defined(rs6000_ibm_aix64_test)
   if(psize == 4) {
     strncat(dest, "_32", dest_max_len - dest_len);
     dest_len += 3;
   }
#endif

#if defined(arch_x86_64_test)
   if (psize == 4) {
      strncat(dest,"_m32", dest_max_len - dest_len);
      dest_len += 4;   
   }
#endif

#if defined(mips_sgi_irix6_4_test)
   strncat(dest,"_n32", dest_max_len - dest_len);
   dest_len += 4;
#endif

#if defined(os_windows_test)
   strncat(dest, ".dll", dest_max_len - dest_len);
   dest_len += 4;
#else
   if( isStatic ) {
       strncat(dest, ".a", dest_max_len - dest_len);
       dest_len += 2;
   }else{
       strncat(dest, ".so", dest_max_len - dest_len);
       dest_len += 3;
   }
#endif
}

#define TOLOWER(c) ((c >= 'A' && c <= 'Z') ? c - 'A' + 'a' : c)
int strcmpcase(char *s1, char *s2) {
    unsigned i;
    unsigned char s1_c, s2_c;
    for (i=0; s1[i] || s2[i]; i++) {
        s1_c = TOLOWER(s1[i]);
        s2_c = TOLOWER(s2[i]);
        if (s1_c < s2_c)
            return -1;
        if (s1_c > s2_c)
            return 1;
    }
    return 0;
}


#if !defined(os_windows_test)
char *searchPath(const char *path, const char *file) {
   assert(path);
   assert(file);

   char *pathcopy = strdup(path);
   char *fullpath;
   char *ptr = NULL; // Purify complained that this was read uninitialized
   char *token = strtok_r(pathcopy, ":", &ptr);

   while (token) {
      fullpath = (char *) ::malloc(strlen(token) + strlen(file) + 2);
      strcpy(fullpath, token);
      strcat(fullpath, "/");
      strcat(fullpath, file);

      struct stat statbuf;
      if (!stat(fullpath, &statbuf))
         break;

      ::free(fullpath);
      token = strtok_r(NULL, ":", &ptr);
   }
   ::free(pathcopy);
   if (token)
      return fullpath;
   return NULL;
}
#else
char *searchPath(const char *path, const char *file) {
  char *fullpath = (char *) ::malloc(strlen(path) + strlen(file) + 2);
  strcpy(fullpath, path);
  strcat(fullpath, "\\");
  strcat(fullpath, file);
  return fullpath;
}
#endif

/**
 * A test should be run if:
 *   1) It isn't disabled
 *   2) It hasn't reported a failure/crash/skip
 *   3) It hasn't reported final results already
 **/
bool shouldRunTest(RunGroup *group, TestInfo *test)
{
   if (group->disabled || test->disabled)
      return false;
   
   if (test->result_reported)
      return false;

   for (unsigned i=0; i<NUM_RUNSTATES; i++)
   {
      if (i == program_teardown_rs)
         continue;
      if (test->results[i] == FAILED ||
          test->results[i] == SKIPPED ||
          test->results[i] == CRASHED)
      {
         reportTestResult(group, test);
         return false;
      }
      assert(test->results[i] == UNKNOWN ||
             test->results[i] == PASSED);
   }
   return true;
}

void reportTestResult(RunGroup *group, TestInfo *test)
{
   if (test->result_reported || test->disabled)
      return;

   test_results_t result = UNKNOWN;
   bool has_unknown = false;
   int failed_state = -1;

   for (unsigned i=0; i<NUM_RUNSTATES; i++)
   {
      if (i == program_teardown_rs)
         continue;
      if (test->results[i] == FAILED ||
          test->results[i] == CRASHED || 
          test->results[i] == SKIPPED) {
         result = test->results[i];
         failed_state = i;
         break;
      }
      else if (test->results[i] == PASSED) {
         result = test->results[i];
      }
      else if (test->results[i] == UNKNOWN) {
         has_unknown = true;
      }
      else {
         assert(0 && "Unknown run state");
      }
   }

   if (result == PASSED && has_unknown)
      return;

   std::map<std::string, std::string> attrs;
   TestOutputDriver::getAttributesMap(test, group, attrs);
   getOutput()->startNewTest(attrs, test, group);
   getOutput()->logResult(result, failed_state);
   getOutput()->finalizeOutput();

   log_testreported(group->index, test->index);
   test->result_reported = true;
}

#if defined(os_linux_test)
#if !defined(cap_gnu_demangler_test)
/**
 * Many linkers don't want to link the static libiberty.a unless
 * we have a reference to it.  It's really needed by libtestdyninst.so
 * and libtestsymtab.so, but we can't link into them because 
 * most systems only provide a static version of this library.
 * 
 * Thus we need libiberty.a linked in with test_driver.  We put a reference
 * to libiberty in libtestSuite.so here, which causes cplus_demangle to 
 * be exported for use by libraries in in test_driver.
 *
 * This is intentionally unreachable code
 **/
extern "C" char *cplus_demangle(char *, int);

void use_liberty()
{
   cplus_demangle("a", 0);
}
#endif
#endif

#if defined (os_windows_test)
//  solaris does not provide setenv, so we provide an ersatz replacement.
// yes it's leaky, but we don't plan on using it too much, so who cares?
int setenv(const char *envname, const char *envval, int)
{
	std::string *alloc_env = new std::string(std::string(envname) 
			+ std::string("=") + std::string(envval));
	return putenv((char *)alloc_env->c_str());

}
#endif
