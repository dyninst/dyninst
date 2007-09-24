#if defined(sparc_sun_solaris2_4) \
 || defined(alpha_dec_osf4_0) \
 || defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */ \
 || defined(i386_unknown_solaris2_5) \
 || defined(ia64_unknown_linux2_4) \
 || defined(os_aix)
#include <dlfcn.h> /* For replaceFunction test */
#endif

#include "mutatee_util.h"

/* Externally accessed function prototypes.  These must have globally unique
 * names.  I suggest following the pattern <testname>_<function>
 */

int test1_22_call1(int x);
int test1_22_call2(int x);
int test1_22_call3(int x);
int test1_22_call7(int x);

/* Global variables accessed by the mutator.  These must have globally unique
 * names.
 */

/* Internally used function prototypes.  These should be declared with the
 * keyword static so they don't interfere with other mutatees in the group.
 */

/* Global variables used internally by the mutatee.  These should be declared
 * with the keyword static so they don't interfere with other mutatees in the
 * group.
 */

static volatile int _unused; /* move decl here to dump compiler warning - jkh */

/* These are copied in libtestA.c and libtestB.c */
#define MAGIC22_1   2200100
#define MAGIC22_2   2200200
#define MAGIC22_3   2200300
#define MAGIC22_4   2200400
#define MAGIC22_5A  2200510
#define MAGIC22_5B  2200520
#define MAGIC22_6   2200600
#define MAGIC22_7   2200700

#if defined(x86_64_unknown_linux2_4) && (__WORDSIZE == 32)
static const char *libNameA = "libtestA_m32.so";
#elif defined(os_windows)
static const char *libNameA = "libtestA.dll";
#else
static const char *libNameA = "libtestA.so";
#endif

/* Function definitions follow */

/*
 * Test #22 - replace function
 *
 * These are defined in libtestA.so
 */
extern void call22_5A(int);
extern void call22_6(int);

typedef int (*call_type)(int);

/* A couple of dlopen-related functions. Moved here from mutatee_util
   so as not to introduce the libdl dependency on all mutatees */
#if defined(os_windows)
void printSysError(unsigned errNo) {
    char buf[1000];
    int result = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, errNo, 
		  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		  buf, 1000, NULL);
    if (!result) {
        fprintf(stderr, "Couldn't print error message\n");
    }
    fprintf(stderr, "%s\n", buf);
}

void *loadDynamicLibrary(char *name) {
  void *result = (void *) LoadLibrary(name);
  if (!result) {
      fprintf(stderr, "[%s:%u] - The mutatee could not load %s\n", __FILE__, __LINE__);
      printSysError(GetLastError());
  }
  return result;
}

void *getFuncFromDLL(void *libhandle, char *func_name) {
    void *result;
    if (!libhandle || !func_name) {
        fprintf(stderr, "[%s:%u] - Test error - getFuncFromDLL passed NULL "
                "parameter\n", __FILE__, __LINE__);
        return NULL;            
    }
    result = GetProcAddress((HMODULE) libhandle, func_name);
    if (!result) {
        fprintf(stderr, "[%s:%u] - Couldn't load symbol %s\n", __FILE__, __LINE__, func_name);
        printSysError(GetLastError());
    }
    return result;
}
#else
void *loadDynamicLibrary(char *name) {
    void *result;
#if defined(os_solaris)
    int dlopenMode = RTLD_NOW | RTLD_GLOBAL;
#else
    int dlopenMode = RTLD_NOW;
#endif
    result = dlopen(name, dlopenMode);
    if (!result) {
        perror("The mutatee couldn't load a dynamic library");
    }
    return result;
}

void *getFuncFromDLL(void *libhandle, char *func_name) {
    void *result = dlsym(libhandle, func_name);
    if (!result) {
        perror("The mutatee couldn't find a function");
    }
    return result;
}
#endif

int test1_22_mutatee()
{
#if defined(os_solaris) \
 || defined(alpha_dec_osf4_0) \
 || defined(os_linux) \
 || defined(os_windows)
    int retval = 0;
    /* libtestA.so should already be loaded (by the mutator), but we
       need to use the dl interface to get pointers to the functions
       it defines. */
/*     int (*call22_5)(int); */
/*     int (*call22_6)(int); */
    call_type call22_5;
    call_type call22_6; /* Why shadowing the top-level declaration? */

    void *handleA;
    char dlopenName[128];
    int result;
    _unused = sprintf(dlopenName, "./%s", libNameA);

    handleA = loadDynamicLibrary(dlopenName);
    if (! handleA) {
	 logerror("**Failed test #22 (replaceFunction)\n");
	 logerror("  Mutatee couldn't get handle for %s\n", libNameA);
	 retval = -1; /* Test failed */
    }
    /* call22_5 = (int(*)(int)) getFuncFromDLL(handleA, "call22_5"); */
    call22_5 = (call_type) getFuncFromDLL(handleA, "call22_5");
    if (! call22_5) {
	 logerror("**Failed test #22 (replaceFunction)\n");
	 logerror("  Mutatee couldn't get handle for call22_5 in %s\n", libNameA);
	 retval = -1; /* Test failed */
    }
    /* call22_6 = (int(*)(int)) getFuncFromDLL(handleA, "call22_6"); */
    call22_6 = (call_type) getFuncFromDLL(handleA, "call22_6");
    if (! call22_6) {
	 logerror("**Failed test #22 (replaceFunction)\n");
	 logerror("  Mutatee couldn't get handle for call22_6 in %s\n", libNameA);
	 retval = -1; /* Test failed */
    }

    /* Call functions that have been replaced by the mutator.  The
       side effects of these calls (replaced, not replaced, or
       otherwise) are independent of each other. */
    result = test1_22_call1(10);  /* replaced by test1_22_call2 */
    if (result != 10 + MAGIC22_2) {
	 logerror("**Failed test #22 (replace function) (a.out -> a.out)\n");
	 retval = -1; /* Test failed */
    }
    result = test1_22_call3(20);  /* replaced by call22_4 */
    if (result != 20 + MAGIC22_4) {
	 logerror("**Failed test #22 (replace function) (a.out -> shlib)\n");
	 retval = -1; /* Test failed */
    }
    if (call22_5 != NULL) {
      result = call22_5(30);  /* replaced by call22_5 (in libtestB) */
      if (result != 30 + MAGIC22_5B) {
	logerror("**Failed test #22 (replace function) (shlib -> shlib)\n");
	retval = -1; /* Test failed */
      }
    }
    if (call22_6 != NULL) {
      result = call22_6(40);  /* replaced by test1_22_call7 */
      if (result != 40 + MAGIC22_7) {
	logerror("**Failed test #22 (replace function) (shlib -> a.out)\n");
	retval = -1; /* Test failed */
      }
    }
    if (0 == retval) {
      logerror("Passed test #22 (replace function)\n");
      test_passes(testname);
    }
#else
    logerror("Skipped test #22 (replace function)\n");
    logerror("\t- not implemented on this platform\n");
    test_passes(testname);
#endif
    return retval;
}

int test1_22_call1(int x) {
    return x + MAGIC22_1;
}

int test1_22_call2(int x) {
    return x + MAGIC22_2;
}

int test1_22_call3(int x) {
    return x + MAGIC22_3;
}

int test1_22_call7(int x) {
    return x + MAGIC22_7;
}
