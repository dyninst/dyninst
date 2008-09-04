/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
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

// $Id: test0_0.C,v 1.2 2008/09/04 01:30:29 jaw Exp $
/*
 *
 * #Name: test0_0
 * #Desc: Null test
 * #Arch: all
 * #Dep: 
 */

#include "test_lib.h"
#include "test0.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <vector>

#if 0
bool runcmd(const char *file, std::vector<char *> &args)
{
   int pid = fork();
   if (pid == 0) {
      // child -- exec command
      char **new_argv = new char *[args.size()+ 2];
      assert( new_argv );
      // the first arg is always the filename
      new_argv[0] = strdup(file);

      for (unsigned int i = 0; i < args.size(); ++i) 
      {
         new_argv[i+1] = strdup(args[i]);
      }
      new_argv[args.size()+1] = NULL;

      extern char **environ;
      int res = execve(file, new_argv, environ);
      fprintf(stderr, "%s[%d]:  exec returned code %d: %d:%s\n", 
            __FILE__, __LINE__ , res, errno,strerror(errno));
      abort();
   }
   else 
   {
      // parent, wait for child;
      int status;
      int res = waitpid(pid, &status, 0);
      
      if (pid != res) 
      {
         fprintf(stderr, "%s[%d]:  waitpid: %s\n", 
               __FILE__, __LINE__, strerror(errno));
         return false;
      }
      
      if (!WIFEXITED(status)) 
      {

         fprintf(stderr, "%s[%d]:  process exited abnormally \n", 
               __FILE__, __LINE__ );

         if (WIFSIGNALED(status)) 
         {
            int signo = WTERMSIG(status);
            fprintf(stderr, "%s[%d]:  process got signal %d \n", 
                  __FILE__, __LINE__, signo );
         }

         if (WIFSTOPPED(status)) 
         {
            int signo = WSTOPSIG(status);
            fprintf(stderr, "%s[%d]:  weird, process is stopped with signal %d \n", 
                  __FILE__, __LINE__, signo );
         }

         return false;
      }

      int exit_status = WEXITSTATUS(status);

      if (0 != exit_status) 
      {
         fprintf(stderr, "%s[%d]:  process returned code %d, not zero\n", 
               __FILE__, __LINE__ , exit_status);
         return false;
      }

      return true;
   }
}
#endif

bool subTest(int num, const char *exef)
{
   char numstr[10];
   sprintf(numstr, "%d", num);

   std::vector<char *> args;
   args.push_back((char *)"-run");
   args.push_back(numstr);

   RunCommand runcmd;
   bool res = runcmd(exef, args);

   if (!res) 
   {
      fprintf(stderr, "%s[%d]:  \"%s\" failed\n", __FILE__, __LINE__, exef);
      return false;
   }

   return true;
}

static int mutatorTest()
{

   for (unsigned int i = 1; i <= PART1_TESTS; ++i) 
   {
#if defined (cap_serialization)
      if (!subTest(i, "test0.mutatee_g++")) 
      {
         fprintf(stderr, "%s[%d]:  subtest %d failed\n", FILE__, __LINE__, i);
         return -1;
      }
#else
      fprintf(stderr, "Skipped test 0,  subtest %d, no serilization available\n", i);
      return 0;
#endif
   }

#if defined (cap_serialization)
   //  actually this one doesn't need to be under this flag...  
   //  just here because its still being fleshed out

   fprintf(stderr, "%s[%d]:\n\nNext Phase\n\n\n", FILE__, __LINE__);

   if (!subTest(0, "test00.mutatee_g++")) 
   {
      fprintf(stderr, "%s[%d]:  subtest 0 failed\n", FILE__, __LINE__);
      return -1;
   }
#endif

#if 0
   for (unsigned int i = 1; i <= PART2_TESTS; ++i) 
   {
#if defined (cap_serialization)
      if (!subTest(i, "test00.mutatee_g++")) 
      {
         fprintf(stderr, "%s[%d]:  subtest %d failed\n", FILE__, __LINE__, i);
         return -1;
      }
#else
      fprintf(stderr, "Skipped test 0,  subtest %d, no serilization available\n", i);
      return 0;
#endif
   }
#endif

   return 0;
}

// External Interface
extern "C" TEST_DLL_EXPORT int test0_0_mutatorMAIN(ParameterDict &param)
{
   fprintf(stderr, "%s[%d]:  welcome to test0_0_mutatorMAIN()\n", __FILE__, __LINE__);
   //  Here we should have SELFSTART set, meaning we don't really interface dyninst.
  // dprintf("Entered test1_1 mutatorMAIN()\n");

    bool useAttach = param["useAttach"]->getInt();

    if (useAttach) 
    {
       fprintf(stderr, "%s[%d]:  makes no sense to attach to this test\n", 
             __FILE__, __LINE__);
       return -1;
    }

    // Get log file pointers
    FILE *outlog = (FILE *)(param["outlog"]->getPtr());
    FILE *errlog = (FILE *)(param["errlog"]->getPtr());
    setOutputLog(outlog);
    setErrorLog(errlog);

    // Run mutator code
    return mutatorTest();
}
