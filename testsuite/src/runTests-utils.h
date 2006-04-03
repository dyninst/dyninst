#ifndef RUNTESTS_UTILS_H
#define RUNTESTS_UTILS_H

/* Windows Specific Includes and Macros */
#if defined(i386_unknown_nt4_0)
// For getpid
#include <process.h>
#define popen _popen
#define pclose _pclose

#define S_ISREG(x) ((x & S_IFMT) == S_IFREG)
#define S_ISDIR(x) ((x & S_IFMT) == S_IFDIR)
#else
/* Unix Specific includes */
#include <unistd.h>
#include <wait.h>

#endif

#if defined(i386_unknown_nt4_0)
#define WEXITSTATUS(x) x
#endif

#include <string>
#include <vector>

using namespace std;

bool isRegFile(const string& filename);

bool isDir(const string& filename); 

void getInput(const char *filename, string& output);

void generateTestString(bool resume, bool useLog, string& logfile,
      int testLimit, vector<char *>& child_argv, string& shellString);

char *setResumeEnv();

char *setLibPath();

void setupVars(bool useLog, string& logfile);

#endif /* RUNTESTS_UTILS_H */
