/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
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
#include "test_lib.h"
#include "test_lib_test9.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern FILE *outlog;
extern FILE *errlog;

#if !defined(os_windows_test)
#include <sys/types.h>
#include <errno.h>
#include <sys/wait.h>
#endif

#if defined(i386_unknown_linux2_0_test) \
 || defined(x86_64_unknown_linux2_4_test)
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif


extern char **environ;

void changePath(char *path){

	char  *newPATH;

	newPATH= new char[strlen("PWD=") + strlen(path) +  1];
	newPATH[0] = '\0';
	strcpy(newPATH, "PWD=");
	strcat(newPATH,path); 

	for(int i=0; environ[i]; i++){

		if( strstr(environ[i], "PWD=") ){
			environ[i] = newPATH;
		}
	}

}

static int preloadMutatedRT(const char *path)
{
    char *rt_lib_name = getenv("DYNINSTAPI_RT_LIB");
    if (rt_lib_name == 0) {
	logerror("preloadMutatedRT: DYNINSTAPI_RT_LIB is undefined\n");
	return (-1);
    }
    char *rt_lib_base = strrchr(rt_lib_name, '/');
    if (rt_lib_base == 0) {
	logerror("preloadMutatedRT: DYNINSTAPI_RT_LIB not a full path\n");
	return (-1);
    }
    const char *var_prefix = "LD_PRELOAD=";
    char *ld_preload = new char[strlen(var_prefix) + strlen(path) +
				strlen(rt_lib_base) + 1];
    strcpy(ld_preload, var_prefix);
    strcat(ld_preload, path);
    strcat(ld_preload, rt_lib_base); // starts with a slash
    if (putenv(ld_preload) < 0) {
	perror("preloadMutatedRT: putenv error");
	return (-1);
    }
    return 0;
}

int runMutatedBinaryLDLIBRARYPATH(char *path, char* fileName, char* testID){

   pid_t pid;
   int status, died;

	char *mutatedBinary;
#if defined(rs6000_ibm_aix4_1_test) \
 || defined(rs6000_ibm_aix5_1)
	char *aixBinary="dyninst_mutatedBinary";
#endif
	char *realFileName;

	realFileName = fileName;
#if defined(rs6000_ibm_aix4_1_test) \
 || defined(rs6000_ibm_aix5_1)
	realFileName = aixBinary;
#endif
	char *currLDPATH, *newLDPATH;

	currLDPATH = getenv("LD_LIBRARY_PATH");
	newLDPATH= new char[strlen("LD_LIBRARY_PATH=") + strlen(currLDPATH) + strlen(":") +strlen(path) + 1];
	newLDPATH[0] = '\0';
	strcpy(newLDPATH, "LD_LIBRARY_PATH=");
	strcat(newLDPATH,path); 
	strcat(newLDPATH,":");
	strcat(newLDPATH, currLDPATH);

	mutatedBinary= new char[strlen(path) + strlen(realFileName) + 10];

	memset(mutatedBinary, '\0', strlen(path) + strlen(realFileName) + 10);

	strcat(mutatedBinary, path);
	strcat(mutatedBinary, realFileName);
	char *command = new char[strlen(mutatedBinary)+ strlen(realFileName) + strlen("-run") + strlen(testID)+10];
	sprintf(command,"%s -run %s", mutatedBinary, testID);

	int retVal =0;
	int outlog_fd = fileno(outlog);
	int errlog_fd = fileno(errlog);
	switch((pid=fork())){
		case -1: 
		logerror("can't fork\n");
                        return 0;
		case 0 : 
			//child
			logerror(" running: %s %s %s\n", mutatedBinary, realFileName, testID);

			// redirect output to log files
			dup2(outlog_fd, 1); // stdout
			dup2(errlog_fd, 2); // stderr

#if defined(rs6000_ibm_aix5_1) \
 || defined(rs6000_ibm_aix4_1_test)
			changePath(path);
#endif
			for(int i=0; environ[i]; i++){

				if( strstr(environ[i], "LD_LIBRARY_PATH=") ){
					environ[i] = newLDPATH;
				}
			}
			if (preloadMutatedRT(path) < 0) {
			    return (-1);
			}
#if  defined(i386_unknown_linux2_0_test) \
 || defined(x86_64_unknown_linux2_4_test)
			struct stat buf;
			retVal = stat("/usr/bin/setarch", &buf);
			if(retVal != -1 ){
				execl("/usr/bin/setarch","setarch","i386",mutatedBinary, "-run", testID,0); 
			}else{
				logerror(" Running without /usr/bin/setarch\n");
				execl(mutatedBinary, realFileName,"-run", testID,0); 
			}
#else

			execl(mutatedBinary, realFileName,"-run", testID,0); 
#endif
			logerror("ERROR!\n");
			perror("execl");
                        return 0;

		default: 
			//parent
			delete [] command;
			delete [] mutatedBinary;
#if defined(rs6000_ibm_aix4_1_test) \
 || defined(i386_unknown_linux2_0_test) \
 || defined(x86_64_unknown_linux2_4_test) /* Blind duplication - Ray */ \
 || defined(rs6000_ibm_aix5_1)
			died= waitpid(pid, &status, 0); 
#endif
   	}

#if defined(rs6000_ibm_aix4_1_test) \
 || defined(i386_unknown_linux2_0_test) \
 || defined(x86_64_unknown_linux2_4_test) /* Blind duplication - Ray */ \
 || defined(rs6000_ibm_aix5_1)
	if(WIFEXITED(status)){
		int exitStatus = WEXITSTATUS(status);

		if(exitStatus == 0){
			return 1;
		}
	}else if(WIFSIGNALED(status)){
		logerror(" terminated with signal: %d \n", WTERMSIG(status));
	}
#endif
	return 0; 
}


void sleep_ms(int ms) 
{
  struct timespec ts,rem;
  if (ms >= 1000) {
    ts.tv_sec = (int) ms / 1000;
  }
  else
    ts.tv_sec = 0;

  ts.tv_nsec = (ms - (ts.tv_sec * 1000)) * 1000 * 1000;
  //fprintf(stderr, "%s[%d]:  sleep_ms (sec = %lu, nsec = %lu)\n",
  //        __FILE__, __LINE__, ts.tv_sec, ts.tv_nsec);

  sleep:

  if (0 != nanosleep(&ts, &rem)) {
    if (errno == EINTR) {
      dprintf("%s[%d]:  sleep interrupted\n", __FILE__, __LINE__);
      ts.tv_sec = rem.tv_sec;
      ts.tv_nsec = rem.tv_nsec;
      goto sleep;
    }
    assert(0);
  }
}

