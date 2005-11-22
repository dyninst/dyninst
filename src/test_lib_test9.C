#include "test_lib.h"
#include "test_lib_test9.h"

#if defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */ \
 || defined(sparc_sun_solaris2_4)
#include <sys/types.h>
#include <sys/wait.h>
#endif

#if defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4)
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

int runMutatedBinary(char *path, char* fileName, char* testID){

   pid_t pid;
   int status, died;
 	char *mutatedBinary;

#if defined(rs6000_ibm_aix4_1) \
 || defined(rs6000_ibm_aix5_1)
	char *aixBinary="dyninst_mutatedBinary";
#endif
	char *realfileName;

	realfileName = fileName;
#if defined(rs6000_ibm_aix4_1) \
 || defined(rs6000_ibm_aix5_1)
	realfileName = aixBinary;
#endif

	mutatedBinary= new char[strlen(path) + strlen(realfileName) + 1];

	memset(mutatedBinary, '\0', strlen(path) + strlen(realfileName) + 1);

	strcat(mutatedBinary, path);
	strcat(mutatedBinary, realfileName);

	switch((pid=fork())){
		case -1: 
		fprintf(stderr,"can't fork\n");
    	    		exit(-1);
		case 0 : 
			//child
			fprintf(stderr," running: %s %s %s\n", mutatedBinary, realfileName, testID);
#if defined(rs6000_ibm_aix5_1) \
 || defined(rs6000_ibm_aix4_1)
			changePath(path);
#endif

			execl(mutatedBinary, realfileName,"-run", testID, 0); 
			fprintf(stderr,"ERROR!\n");
			perror("execl");
			exit(-1);

		default: 
			//parent
			delete [] mutatedBinary;
#if defined(sparc_sun_solaris2_4) \
 || defined(rs6000_ibm_aix4_1) \
 || defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */ \
 || defined(rs6000_ibm_aix5_1)
			died= waitpid(pid, &status, 0); 
#endif
   	}
#if defined(sparc_sun_solaris2_4) \
 || defined(rs6000_ibm_aix4_1) \
 || defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */ \
 || defined(rs6000_ibm_aix5_1)
	if(WIFEXITED(status)){
		int exitStatus = WEXITSTATUS(status);

		if(exitStatus == 0){
			return 1;
		}
	}else if(WIFSIGNALED(status)){
		fprintf(stderr," terminated with signal: %d \n", WTERMSIG(status));
	}
#endif
	return 0; 
	

}

extern char **environ;

void changePath(char *path){

	char  *newPATH;

	newPATH= new char[strlen("PWD=") + strlen(path) +  1];
	newPATH[0] = '\0';
	strcpy(newPATH, "PWD=");
	strcat(newPATH,path); 

	for(int i=0;environ[i]!= '\0';i++){

		if( strstr(environ[i], "PWD=") ){
			environ[i] = newPATH;
		}
	}

}

int runMutatedBinaryLDLIBRARYPATH(char *path, char* fileName, char* testID){

   pid_t pid;
   int status, died;

	char *mutatedBinary;
#if defined(rs6000_ibm_aix4_1) \
 || defined(rs6000_ibm_aix5_1)
	char *aixBinary="dyninst_mutatedBinary";
#endif
	char *realFileName;

	realFileName = fileName;
#if defined(rs6000_ibm_aix4_1) \
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

	mutatedBinary= new char[strlen(path) + strlen(realFileName) + 1];

	memset(mutatedBinary, '\0', strlen(path) + strlen(realFileName) + 1);

	strcat(mutatedBinary, path);
	strcat(mutatedBinary, realFileName);
	char *command = new char[strlen(mutatedBinary)+ strlen(realFileName) + strlen("-run") + strlen(testID)+10];
	sprintf(command,"%s -run %s", mutatedBinary, testID);

	int retVal =0;
	switch((pid=fork())){
		case -1: 
		fprintf(stderr,"can't fork\n");
    	    		exit(-1);
		case 0 : 
			//child
			fprintf(stderr," running: %s %s %s\n", mutatedBinary, realFileName, testID);

#if defined(rs6000_ibm_aix5_1) \
 || defined(rs6000_ibm_aix4_1)
			changePath(path);
#endif
			for(int i=0;environ[i]!= '\0';i++){

				if( strstr(environ[i], "LD_LIBRARY_PATH=") ){
					environ[i] = newLDPATH;
				}
			}
#if  defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4)
			struct stat buf;
			retVal = stat("/usr/bin/setarch", &buf);
			if(retVal != -1 ){
				execl("/usr/bin/setarch","setarch","i386",mutatedBinary, "-run", testID,0); 
			}else{

				execl(mutatedBinary, realFileName,"-run", testID,0); 
			}
#else

			execl(mutatedBinary, realFileName,"-run", testID,0); 
#endif
			fprintf(stderr,"ERROR!\n");
			perror("execl");
			exit(-1);

		default: 
			//parent
			delete [] command;
			delete [] mutatedBinary;
#if defined(sparc_sun_solaris2_4) \
 || defined(rs6000_ibm_aix4_1) \
 || defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */ \
 || defined(rs6000_ibm_aix5_1)
			died= waitpid(pid, &status, 0); 
#endif
   	}

#if defined(sparc_sun_solaris2_4) \
 || defined(rs6000_ibm_aix4_1) \
 || defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */ \
 || defined(rs6000_ibm_aix5_1)
	if(WIFEXITED(status)){
		int exitStatus = WEXITSTATUS(status);

		if(exitStatus == 0){
			return 1;
		}
	}else if(WIFSIGNALED(status)){
		fprintf(stderr," terminated with signal: %d \n", WTERMSIG(status));
	}
#endif
	return 0; 
}


void sleep_ms(int ms) 
{
//#if defined(os_solaris) && (os_solaris < 9)
#ifdef NOTDEF
  if (ms < 1000) {
    usleep(ms * 1000);
  }
  else {
    sleep(ms / 1000);
    usleep((ms % 1000) * 1000);
  }
#else
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
      fprintf(stderr, "%s[%d]:  sleep interrupted\n", __FILE__, __LINE__);
      ts.tv_sec = rem.tv_sec;
      ts.tv_nsec = rem.tv_nsec;
      goto sleep;
    }
    assert(0);
  }
#endif
}

