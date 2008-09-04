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

/* Test application (Mutatee) */

/* $Id: test00.mutatee.c,v  */

#include <stdio.h>
#include <assert.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>

#include "../../common/h/serialize.h"
#include "../../symtabAPI/h/Symbol.h"
#include "../../symtabAPI/h/LineInformation.h"
#include "../../symtabAPI/h/symutil.h"
#include "../../symtabAPI/h/AddrLookup.h"
#include "../../symtabAPI/h/Symtab.h"
#include "../../symtabAPI/h/Type.h"
//  TODO:  move this file
#include "../../symtabAPI/src/Collections.h"

using namespace Dyninst;
using namespace Dyninst::SymtabAPI;
#include "mutatee_util.h"
#include "test0.h"

#if defined(i386_unknown_nt4_0) && !defined(__GNUC__)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <process.h>
#define getpid _getpid
#else
#include <unistd.h>
#endif

#ifdef __cplusplus
int mutateeCplusplus = 1;
#else
#error
#endif

#ifndef FILE__
#define FILE__ strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__
#endif

#ifndef COMPILER
#define COMPILER ""
#endif
const char *Builder_id=COMPILER; /* defined on compile line */

/* control debug printf statements */
#define dprintf	if (debugPrint) printf
int debugPrint = 0;
bool verbose = false;
const char *prog_name;

#define MAX_TEST 5


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

      for (unsigned int i = 0; i < args.size(); ++i) {
         new_argv[i+1] = strdup(args[i]);
      }
      new_argv[args.size()+1] = NULL;

      extern char **environ;
      int res = execve(file, new_argv, environ);
      fprintf(stderr, "%s[%d]:  exec returned code %d: %d:%s\n", 
            __FILE__, __LINE__ , res, errno,strerror(errno));
      abort();
   }
   else {
      // parent, wait for child;
      int status;
      int res = waitpid(pid, &status, 0);
      if (pid != res) {
         fprintf(stderr, "%s[%d]:  waitpid: %s\n", __FILE__, __LINE__, strerror(errno));
         return false;
      }
      if (!WIFEXITED(status)) {
         fprintf(stderr, "%s[%d]:  process exited abnormally \n", __FILE__, __LINE__ );
         if (WIFSIGNALED(status)) {
            int signo = WTERMSIG(status);
            fprintf(stderr, "%s[%d]:  process got signal %d \n", __FILE__, __LINE__, signo );
         }
         if (WIFSTOPPED(status)) {
            int signo = WSTOPSIG(status);
            fprintf(stderr, "%s[%d]:  weird, process is stopped with signal %d \n", __FILE__, __LINE__, signo );
         }
         return false;
      }
      int exit_status = WEXITSTATUS(status);
      if (exit_status != 0) {
         fprintf(stderr, "%s[%d]:  process returned code %d, not zero\n", __FILE__, __LINE__ , exit_status);
         return false;
      }
      return true;
   }
}
#endif


class ZeroOne;
class ZeroTwo;
class ZeroThree;
class ZeroFour;
class ZeroFive;
class ZeroSix;

bool test0()
{
   //  some basic symtabAPI tests
   //  here we just work with test1.mutatee_

   SerializerBin::globalDisable();

   bool err = false;
   Symtab *test1_mutatee = NULL;
   std::string mutatee_name("test1.mutatee_gcc");

   err = Symtab::openFile(test1_mutatee, mutatee_name);

   if (err) 
   {
      fprintf(stderr, "%s[%d]:  failed to create symtab for %s\n", 
            FILE__, __LINE__, mutatee_name.c_str());
      return false;
   }
   
   if (NULL == test1_mutatee) 
   {
      fprintf(stderr, "%s[%d]:  failed to create symtab for %s\n", 
            FILE__, __LINE__, mutatee_name.c_str());
      return false;
   }

   if (!test1_mutatee->isExec()) 
   {
      fprintf(stderr, "%s[%d]:  symtab for %s is not an executable\n", 
            FILE__, __LINE__, mutatee_name.c_str());
      return false;
   }

   fprintf(stderr, "%s[%d]:  passed test0 so far\n", FILE__, __LINE__);

   SerializerBin::globalEnable();

   return true;
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
///////////
///////////  Test 1 --
///////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

class ZeroOne : public Serializable{
   public:
      int zero_one_int;
      unsigned int zero_one_unsigned_int;
      long zero_one_long;
      unsigned long zero_one_unsigned_long;
#if 0
      short zero_one_short;
#endif
      //unsigned short zero_two_unsigned_short;
      char zero_one_char;
      //unsigned char zero_two_unsigned_char;

      bool operator==(const ZeroOne &cmp) {
         if (zero_one_int != cmp.zero_one_int) return false;
         if (zero_one_long != cmp.zero_one_long) return false;
#if 0
         if (zero_one_short != cmp.zero_one_short) return false;
#endif
         if (zero_one_char != cmp.zero_one_char) return false;
         return true;
      }
      bool operator!=(const ZeroOne &cmp) {
         return (! (*this == cmp));
      }
      void printcmp(const ZeroOne &cmp) 
      {
         fprintf(stderr, "%s[%d]: %d %s %d\n", __FILE__, __LINE__,
               zero_one_int, 
               (zero_one_int == cmp.zero_one_int) ? "==" : "!=", 
               cmp.zero_one_int);

         fprintf(stderr, "%s[%d]: %ld %s %ld\n", __FILE__, __LINE__,
               zero_one_long, 
               (zero_one_long == cmp.zero_one_long) ? "==" : "!=", 
               cmp.zero_one_long);

#if 0
         fprintf(stderr, "%s[%d]: %hu %s %hu\n", __FILE__, __LINE__, 
               zero_one_short, 
               zero_one_short == cmp.zero_one_short ? "==" : "!=", 
               cmp.zero_one_short);
#endif
         fprintf(stderr, "%s[%d]: %c %s %c\n", __FILE__, __LINE__,
               zero_one_char, 
               (zero_one_char == cmp.zero_one_char) ? "==" : "!=", 
               cmp.zero_one_char);
      }

      void serialize(SerializerBase *sb, const char *) 
      {
         try {
            gtranslate(sb, zero_one_int, NULL);
            gtranslate(sb, zero_one_unsigned_int, NULL);
            gtranslate(sb, zero_one_long, NULL);
            gtranslate(sb, zero_one_unsigned_long, NULL);
#if 0
            gtranslate(sb, zero_one_short, NULL);
#endif
            gtranslate(sb, zero_one_char, NULL);
         } SER_CATCH("ZeroOne");
      }
};

bool setup_control(ZeroOne &zero1)
{
   zero1.zero_one_int = -1237843;
   zero1.zero_one_unsigned_int = 1237843;
   zero1.zero_one_long = -12378434L;
   zero1.zero_one_unsigned_long = 12378434L;
#if 0
   zero1.zero_one_short = -12;
   //zero1.zero_two_unsigned_char= 12;
   zero1.zero_one_short = -1 * 'a';
#endif
   //zero1.zero_two_unsigned_short = 'a';
   return true;
}

bool test1b(const char *cachefile)
{
   if (!cachefile) {
      fprintf(stderr, "%s[%d]:  NULL param\n", FILE__, __LINE__);
      abort();
   }

   bool res = deserialize_verify<ZeroOne>(cachefile);
   if (!res) {
      fprintf(stderr, "%s[%d]:  test1b failed\n", FILE__, __LINE__);
   }
   return res;
}


bool test1()
{
   bool res = serialize_test<ZeroOne>(1, prog_name);
   if (!res) {
      fprintf(stderr, "%s[%d]:  test1 failed\n", FILE__, __LINE__);
   }

   return res;
}


/* 
 * Test #2 
 */

class ZeroTwo : public Serializable {
   public:
   std::string zero_two_string1;
   std::string zero_two_string2;
   std::string zero_two_string3;
   std::string zero_two_string4;
   std::string zero_two_string5;
   int zero_two_int1;
   int zero_two_int2;
   int zero_two_int3;
   int zero_two_int4;
   int zero_two_int5;
   float zero_two_float1;
   float zero_two_float2;
   float zero_two_float3;
   float zero_two_float4;
   float zero_two_float5;

   public:
      bool operator==(const ZeroTwo &cmp) {
         if (zero_two_int1 != cmp.zero_two_int1) return false;
         if (zero_two_int2 != cmp.zero_two_int2) return false;
         if (zero_two_int3 != cmp.zero_two_int3) return false;
         if (zero_two_int4 != cmp.zero_two_int4) return false;
         if (zero_two_int5 != cmp.zero_two_int5) return false;
         if (zero_two_float1 != cmp.zero_two_float1) return false;
         if (zero_two_float2 != cmp.zero_two_float2) return false;
         if (zero_two_float3 != cmp.zero_two_float3) return false;
         if (zero_two_float4 != cmp.zero_two_float4) return false;
         if (zero_two_float5 != cmp.zero_two_float5) return false;
         if (zero_two_string1 != cmp.zero_two_string1) return false;
         if (zero_two_string2 != cmp.zero_two_string2) return false;
         if (zero_two_string3 != cmp.zero_two_string3) return false;
         if (zero_two_string4 != cmp.zero_two_string4) return false;
         if (zero_two_string5 != cmp.zero_two_string5) return false;
         return true;
      }
      bool operator!=(const ZeroTwo &cmp) {
         return (! (*this == cmp));
      }
      void printcmp(const ZeroTwo &cmp) {
         if (zero_two_int1 != cmp.zero_two_int1) 
            fprintf(stderr, "%s[%d]: fail here\n", FILE__, __LINE__);
         if (zero_two_int2 != cmp.zero_two_int2) 
            fprintf(stderr, "%s[%d]: fail here\n", FILE__, __LINE__);
         if (zero_two_int3 != cmp.zero_two_int3) 
            fprintf(stderr, "%s[%d]: fail here\n", FILE__, __LINE__);
         if (zero_two_int4 != cmp.zero_two_int4)
            fprintf(stderr, "%s[%d]: fail here\n", FILE__, __LINE__);
         if (zero_two_int5 != cmp.zero_two_int5) 
            fprintf(stderr, "%s[%d]: fail here\n", FILE__, __LINE__);
         if (zero_two_float1 != cmp.zero_two_float1) 
            fprintf(stderr, "%s[%d]: fail here\n", FILE__, __LINE__);
         if (zero_two_float2 != cmp.zero_two_float2)
            fprintf(stderr, "%s[%d]: fail here\n", FILE__, __LINE__);
         if (zero_two_float3 != cmp.zero_two_float3) 
            fprintf(stderr, "%s[%d]: fail here\n", FILE__, __LINE__);
         if (zero_two_float4 != cmp.zero_two_float4) 
            fprintf(stderr, "%s[%d]: fail here\n", FILE__, __LINE__);
         if (zero_two_float5 != cmp.zero_two_float5) 
            fprintf(stderr, "%s[%d]: fail here\n", FILE__, __LINE__);
         if (zero_two_string1 != cmp.zero_two_string1) 
            fprintf(stderr, "%s[%d]: fail here\n", FILE__, __LINE__);
         if (zero_two_string2 != cmp.zero_two_string2) 
            fprintf(stderr, "%s[%d]: fail here\n", FILE__, __LINE__);
         if (zero_two_string3 != cmp.zero_two_string3) 
            fprintf(stderr, "%s[%d]: fail here\n", FILE__, __LINE__);
         if (zero_two_string4 != cmp.zero_two_string4) 
            fprintf(stderr, "%s[%d]: fail here\n", FILE__, __LINE__);
         if (zero_two_string5 != cmp.zero_two_string5) 
            fprintf(stderr, "%s[%d]: fail here\n", FILE__, __LINE__);
      }

      void serialize(SerializerBase *sb, const char *)
      {
         try {
            gtranslate(sb, zero_two_int1);
            gtranslate(sb, zero_two_int2);
            gtranslate(sb, zero_two_int3);
            gtranslate(sb, zero_two_int4);
            gtranslate(sb, zero_two_int5);
            gtranslate(sb, zero_two_float1);
            gtranslate(sb, zero_two_float2);
            gtranslate(sb, zero_two_float3);
            gtranslate(sb, zero_two_float4);
            gtranslate(sb, zero_two_float5);
            gtranslate(sb, zero_two_string1);
            gtranslate(sb, zero_two_string2);
            gtranslate(sb, zero_two_string3);
            gtranslate(sb, zero_two_string4);
            gtranslate(sb, zero_two_string5);
         } SER_CATCH("ZeroTwo");
      }
};

bool setup_control(ZeroTwo &control)
{
   control.zero_two_int1 = 0xdeadbeef;
   control.zero_two_int2 = 0xbeeffeed;
   control.zero_two_int3 = 0xdeadfeed;
   control.zero_two_int4 = 0xfeedbeef;
   control.zero_two_int5 = 0xdadabaca;
   control.zero_two_float1 = 0.1;
   control.zero_two_float2 = 1e23;
   control.zero_two_float3 = 1.38742e5;
   control.zero_two_float4 = 5.5;
   control.zero_two_float5 = 6.6;
   control.zero_two_string1 = std::string("hello world");
   control.zero_two_string2 = std::string("This is not a test");
   control.zero_two_string3 = std::string("This is a test");
   control.zero_two_string4 = std::string("The world was beginning to flower into wounds");
   control.zero_two_string5 = std::string("The aggressive stylization of this mass-produced cockpit, the exaggerated mouldings of the instrument binnacles emphasized my growing sense of a new junction between my own body and the automobile, closer than my feelings for Renata's broad hips and strong legs stowed out of sight beneath her red plastic raincoat");
   return true;
}

bool test2()
{
   bool res = serialize_test<ZeroTwo>(2, prog_name);
   if (!res) {
      fprintf(stderr, "%s[%d]:  test2 failed\n", FILE__, __LINE__);
   }

   return res;
}

bool test2b(const char *cachefile)
{
   if (!cachefile) {
      fprintf(stderr, "%s[%d]:  NULL param\n", FILE__, __LINE__);
      abort();
   }

   bool res = deserialize_verify<ZeroTwo>(cachefile);
   if (!res) {
      fprintf(stderr, "%s[%d]:  test2b failed\n", FILE__, __LINE__);
   }
   return res;
}

/*
 * Test #3 
 */

class ZeroThree : public Serializable {
   public:
      std::vector<int> zero_three_ints;
      bool operator==(const ZeroThree &cmp) {
         if (zero_three_ints.size() != cmp.zero_three_ints.size()) {
            return false;
         }
         for (unsigned int i = 0; i < cmp.zero_three_ints.size(); ++i) {
            if ((zero_three_ints[i] != cmp.zero_three_ints[i])) 
                  return false;
         }
         return true;
      }

      bool operator!=(const ZeroThree &cmp) {
         return (! (*this == cmp));
      }
      void printcmp(const ZeroThree &cmp) {
         if (zero_three_ints.size() != cmp.zero_three_ints.size()) {
            fprintf(stderr, "%s[%d]:  vectors have sizes %d and %d\n", 
                  FILE__, __LINE__, cmp.zero_three_ints.size(), zero_three_ints.size());
         }
         for (unsigned int i = 0; i < cmp.zero_three_ints.size(); ++i) {
            if ((zero_three_ints[i] != cmp.zero_three_ints[i])) 
               fprintf(stderr, "elem %d: %d -- %d\n", i, zero_three_ints[i], 
                     cmp.zero_three_ints[i]);
         }
      }

      void serialize(SerializerBase *sb, const char *)
      {
         try {
            gtranslate(sb, zero_three_ints);
         } SER_CATCH("ZeroThree");
      }
};

bool setup_control(ZeroThree &param)
{
   for (unsigned int i = 0; i < 50; ++i) {
      param.zero_three_ints.push_back(i*100 + 2077);
   }
   return true;
}

bool test3()
{
   bool res  = serialize_test<ZeroThree>(3, prog_name);
   if (!res) {
      fprintf(stderr, "%s[%d]:  test3 failed\n", FILE__, __LINE__);
   }

   return res;
}

bool test3b(const char *cachefile)
{
   if (!cachefile) {
      fprintf(stderr, "%s[%d]:  NULL param\n", FILE__, __LINE__);
      abort();
   }

   bool res = deserialize_verify<ZeroThree>(cachefile);
   if (!res) {
      fprintf(stderr, "%s[%d]:  test3b failed\n", FILE__, __LINE__);
   }
   return res;
}

/*
 * Test #4 
 */
class ZeroFour : public Serializable {
   public:
      std::vector<std::vector<int> > zero_four_ints;
      bool operator==(const ZeroFour &cmp) {
         if (zero_four_ints.size() != cmp.zero_four_ints.size()) {
            return false;
         }
         for (unsigned int i = 0; i < cmp.zero_four_ints.size(); ++i) {
            if ((zero_four_ints[i] != cmp.zero_four_ints[i])) 
                  return false;
         }
         return true;
      }

      bool operator!=(const ZeroFour &cmp) {
         return (! (*this == cmp));
      }
      void printcmp(const ZeroFour &cmp) {
         if (zero_four_ints.size() != cmp.zero_four_ints.size()) {
            fprintf(stderr, "%s[%d]:  vectors have sizes %d and %d\n", 
                  FILE__, __LINE__, cmp.zero_four_ints.size(), zero_four_ints.size());
         }
         for (unsigned int i = 0; i < cmp.zero_four_ints.size(); ++i) {
            const std::vector<int> &v1 = cmp.zero_four_ints[i];
            const std::vector<int> &v2 = zero_four_ints[i];

            for (unsigned int j = 0; j < v1.size(); ++j) {

               if ((v1[j] != v2[j])) 
                  fprintf(stderr, "elem %d: %d -- %d\n", j, v1[j], v2[j]);
            }
         }
      }
      void serialize(SerializerBase *sb, const char *)
      {
         try {
            gtranslate(sb, zero_four_ints);
         } SER_CATCH("ZeroFour");
      }

};

bool setup_control(ZeroFour &param)
{

      //std::vector<std::vector<int> > zero_four_ints;
   param.zero_four_ints.resize(50);
   for (unsigned int i = 0; i < 50; ++i) {
      std::vector <int> &v  = param.zero_four_ints[i];
      v.resize(50);
      for (unsigned j = 0; j < 50; ++j) {
         v[j] = i*j + 2077;
      }
   }
   return true;
}

bool test4()
{
   bool res = serialize_test<ZeroFour>(4, prog_name);
   if (!res) {
      fprintf(stderr, "%s[%d]:  test4 failed\n", FILE__, __LINE__);
   }

   return res;
}

bool test4b(const char *cachefile)
{
   if (!cachefile) {
      fprintf(stderr, "%s[%d]:  NULL param\n", FILE__, __LINE__);
      abort();
   }

   bool res = deserialize_verify<ZeroFour>(cachefile);
   if (!res) {
      fprintf(stderr, "%s[%d]:  test4b failed\n", FILE__, __LINE__);
   }
   return res;
}

/* 
 * Test #5 
 */
class ZeroFive : public Serializable {
   public:
      std::vector<int *> zero_five_ints;
      bool operator==(const ZeroFive &cmp) {
         if (zero_five_ints.size() != cmp.zero_five_ints.size()) {
            return false;
         }
         for (unsigned int i = 0; i < cmp.zero_five_ints.size(); ++i) {
            assert(zero_five_ints[i]);
            assert(cmp.zero_five_ints[i]);
            if ((*zero_five_ints[i] != *cmp.zero_five_ints[i])) 
                  return false;
         }
         return true;
      }

      bool operator!=(const ZeroFive &cmp) {
         return (! (*this == cmp));
      }
      void printcmp(const ZeroFive &cmp) {
         if (zero_five_ints.size() != cmp.zero_five_ints.size()) {
            fprintf(stderr, "%s[%d]:  vectors have sizes %d and %d\n", 
                  FILE__, __LINE__, cmp.zero_five_ints.size(), zero_five_ints.size());
         }
         for (unsigned int i = 0; i < cmp.zero_five_ints.size(); ++i) {
            assert(zero_five_ints[i]);
            if ((*zero_five_ints[i] != *cmp.zero_five_ints[i])) 
               fprintf(stderr, "elem %d: %d -- %d\n", i, *zero_five_ints[i], 
                     *cmp.zero_five_ints[i]);
         }
      }

      void serialize(SerializerBase *sb, const char *)
      {
         try {
            gtranslate(sb,zero_five_ints);
         } SER_CATCH("ZeroFive");
      }

};

bool setup_control(ZeroFive &param)
{
   for (unsigned int i = 0; i < 50; ++i) {
      int *newint = new int[1];
      assert(newint);
      *newint = i*100 + 2077;
      param.zero_five_ints.push_back(newint);
   }
   return true;
}

bool test5()
{
   bool res = serialize_test<ZeroFive>(5, prog_name);
   if (!res) {
      fprintf(stderr, "%s[%d]:  test5 failed\n", FILE__, __LINE__);
   }

   return res;
}

bool test5b(const char *cachefile)
{
   if (!cachefile) {
      fprintf(stderr, "%s[%d]:  NULL param\n", FILE__, __LINE__);
      abort();
   }

   bool res = deserialize_verify<ZeroFive>(cachefile);
   if (!res) {
      fprintf(stderr, "%s[%d]:  test5b failed\n", FILE__, __LINE__);
   }
   return res;
}

/* 
 * Test #6 
 */
class ZeroSix : public Serializable {
   public:
      hash_map<char, std::string> dictionary6;
      bool operator==(ZeroSix &cmp) {
         if (dictionary6.size() != cmp.dictionary6.size()) {
            return false;
         }
         hash_map<char, std::string>::iterator iter1 = dictionary6.begin();
         hash_map<char, std::string>::iterator iter2 = cmp.dictionary6.begin();
         while ((iter1 != dictionary6.end()) && (iter2 != cmp.dictionary6.end())) {
            char key1 = iter1->first;
            char key2 = iter2->first;
            std::string &val1 = iter1->second;
            std::string &val2 = iter2->second;
            if (key1 != key2) {
               fprintf(stderr, "%s[%d]: keys differ: %c != %c\n", 
                     FILE__, __LINE__, key1, key2);
               return false;
            }
            if (val1 != val2) {
               fprintf(stderr, "%s[%d]: vals differ: '%s' != '%s'\n", 
                     FILE__, __LINE__, val1.c_str(), val2.c_str());
               return false;
            }
            iter1++;
            iter2++;
         }

         return true;
      }

      bool operator!=(ZeroSix &cmp) {
         return (! (*this == cmp));
      }
      void printcmp(ZeroSix &cmp) {
         if (dictionary6.size() != cmp.dictionary6.size()) {
            fprintf(stderr, "%s[%d]:  hashes have sizes %d and %d\n", 
                  FILE__, __LINE__, cmp.dictionary6.size(), dictionary6.size());
         }
         hash_map<char, std::string>::iterator iter1 = dictionary6.begin();
         hash_map<char, std::string>::iterator iter2 = cmp.dictionary6.begin();
         while (iter1 != dictionary6.end() && iter2 != cmp.dictionary6.end()) {
            char key1 = iter1->first;
            char key2 = iter2->first;
            std::string val1 = iter1->second;
            std::string val2 = iter2->second;
            fprintf(stderr, "%s[%d]:  key1:  %c, val1: '%s'\n", FILE__, __LINE__, key1, val1.c_str());
            fprintf(stderr, "%s[%d]:  key2:  %c, val2: '%s'\n", FILE__, __LINE__, key2, val2.c_str());
            iter1++;
            iter2++;
         }
      }

      void serialize(SerializerBase *sb, const char *) 
      {
         try {
            fprintf(stderr, "%s[%d]:  before gtranslate:  hash size = %d\n", 
                  FILE__, __LINE__, dictionary6.size());
            gtranslate(sb,dictionary6);
         } SER_CATCH("ZeroSix");
      }

};

bool setup_control(ZeroSix &param)
{
   assert(param.dictionary6.size() == 0);
   param.dictionary6['a'] = "aardvark";
   param.dictionary6['b'] = "bollocks";
   param.dictionary6['c'] = "cro-magnon";
   param.dictionary6['d'] = "deadbeat";
   param.dictionary6['e'] = "elephant";
   param.dictionary6['f'] = "farce";
   param.dictionary6['g'] = "guardian";
   param.dictionary6['h'] = "heresy";
   param.dictionary6['i'] = "ipsedixit";
   param.dictionary6['j'] = "jovian";
   param.dictionary6['k'] = "killface";
   param.dictionary6['l'] = "lavatory";
   param.dictionary6['m'] = "masonic";
   param.dictionary6['n'] = "newbie";
   param.dictionary6['o'] = "oddity";
   param.dictionary6['p'] = "paternal";
   param.dictionary6['q'] = "quack";
   param.dictionary6['r'] = "restaurant";
   param.dictionary6['s'] = "salami";
   param.dictionary6['t'] = "twonky";
   param.dictionary6['u'] = "usurper";
   param.dictionary6['v'] = "velocity";
   param.dictionary6['w'] = "wherabouts";
   param.dictionary6['x'] = "xray-spex";
   param.dictionary6['y'] = "yosemite";
   param.dictionary6['z'] = "zeus";
   return true;
}

bool test6()
{
   bool res = serialize_test<ZeroSix>(6, prog_name);
   if (!res) {
      fprintf(stderr, "%s[%d]:  test6 failed\n", FILE__, __LINE__);
   }

   return res;
}

bool test6b(const char *cachefile)
{
   if (!cachefile) {
      fprintf(stderr, "%s[%d]:  NULL param\n", FILE__, __LINE__);
      abort();
   }

   bool res = deserialize_verify<ZeroSix>(cachefile);
   if (!res) {
      fprintf(stderr, "%s[%d]:  test6b failed\n", FILE__, __LINE__);
   }
   return res;
}

int main(int iargc, char *argv[])
{                                       /* despite different conventions */
   unsigned argc=(unsigned)iargc;      /* make argc consistently unsigned */
   unsigned int i;
   unsigned long testNum=0;
   char *modifier = NULL;
   char *aux_file_name = NULL;
   char *logfilename = NULL;
   prog_name = argv[0];

   fprintf(stderr, "%s[%d]:  welcome to \"%s", __FILE__, __LINE__, argv[0]);
   for (unsigned int ii=1; ii < argc; ++ii) {
      fprintf(stderr, " %s", argv[ii]);
   }
   fprintf(stderr, "\"\n");

   for (i=1; i < argc; i++) {
      if (!strcmp(argv[i], "-verbose")) {
         debugPrint = TRUE;
      } else if (!strcmp(argv[i], "-log")) {
         if ((i + 1) >= argc) {
            fprintf(stderr, "Missing log file name\n");
            exit(-1);
         }
         i += 1;
         logfilename = argv[i];
      } else if (!strcmp(argv[i], "-run")) {
         char *str_end_ptr = NULL;
         testNum = strtoul(argv[i+1], &str_end_ptr,10);
         if (*str_end_ptr != '\0') {
            //  catch subtests that are exec'd automatically (eg, 1b, 1c)
            modifier = str_end_ptr;
            aux_file_name = argv[i+2];
            fprintf(stderr, "%s[%d]:  have test modifier %s, testNum = %ld\n", 
                  FILE__, __LINE__, modifier, testNum);
            i += 1;
         }
          else {
            /* end of test list */
            break;
         }
         i++;
      } else {
         fprintf(stderr, "Usage: %s [-verbose] -run <num>\n", argv[0]);
         fprintf(stderr, "\n\nyou asked for:\n\t\"");
         for (unsigned int i = 0; i < argc; ++i) {
            fprintf(stderr, "%s ", argv[i]);
         }
         fprintf(stderr, "\"\n");
         exit(-1);
      }
   }

   if ((logfilename != NULL) && (strcmp(logfilename, "-") != 0)) {
      outlog = fopen(logfilename, "a");
      if (NULL == outlog) {
         fprintf(stderr, "Error opening log file %s\n", logfilename);
         exit(-1);
      }
      errlog = outlog;
   } else {
      outlog = stdout;
      errlog = stderr;
   }

   if ((argc==1) || debugPrint)
      logstatus("Mutatee %s [C++]:\"%s\"\n", argv[0], Builder_id);
   if (argc==1) exit(0);

   bool test_ok = false;
   switch (testNum) {
      case 0:
         if (!modifier)
            test_ok = test0();
         else {
            fprintf(stderr, "%s[%d]:  ERROR:  modifier makes no sense here\n", FILE__, __LINE__);
         }
      case 1:
         if (!modifier)
            test_ok = test1();
         else {
            switch (*modifier) {
               case 'b':  
                  if (!aux_file_name) {
                     fprintf(stderr, "%s[%d]:  no aux file name\n", FILE__, __LINE__);
                     abort();
                  }

                  fprintf(stderr, "%s[%d]:  process %d about to run test1b\n", 
                        FILE__, __LINE__, getpid());
                  test_ok = test1b(aux_file_name); 
                  fprintf(stderr, "%s[%d]:  process %d ran test1b\n", 
                        FILE__, __LINE__, getpid());
                  break;
               default:
                  fprintf(stderr, "%s[%d]:  bad modifier! %s\n", 
                        FILE__, __LINE__, modifier);
            };
         }
         break;

      case 2:
         if (!modifier)
            test_ok = test2();
         else {
            switch (*modifier) {
               case 'b':  test_ok = test2b(aux_file_name); break;
               default:
                  fprintf(stderr, "%s[%d]:  bad modifier! %s\n", 
                        FILE__, __LINE__, modifier);
            };
         }
         break;

      case 3:
         if (!modifier)
            test_ok = test3();
         else {
            switch (*modifier) {
               case 'b':  test_ok = test3b(aux_file_name); break;
               default:
                  fprintf(stderr, "%s[%d]:  bad modifier! %s\n", 
                        FILE__, __LINE__, modifier);
            };
         }
         break;

      case 4:
         if (!modifier)
            test_ok = test4();
         else {
            switch (*modifier) {
               case 'b':  test_ok = test4b(aux_file_name); break;
               default:
                  fprintf(stderr, "%s[%d]:  bad modifier! %s\n", 
                        FILE__, __LINE__, modifier);
            };
         }
         break;

      case 5:
         if (!modifier)
            test_ok = test5();
         else {
            switch (*modifier) {
               case 'b':  test_ok = test5b(aux_file_name); break;
               default:
                  fprintf(stderr, "%s[%d]:  bad modifier! %s\n", 
                        FILE__, __LINE__, modifier);
            };
         }
         break;

      case 6:
         if (!modifier)
            test_ok = test6();
         else {
            switch (*modifier) {
               case 'b':  test_ok = test6b(aux_file_name); break;
               default:
                  fprintf(stderr, "%s[%d]:  bad modifier! %s\n", 
                        FILE__, __LINE__, modifier);
            };
         }
         break;
      default:
         fprintf(stderr, "%s[%d]: invalid test number %ld in mutatee\n", 
               FILE__, __LINE__, testNum);
         break;
   }

   dprintf("Mutatee %s terminating.\n", argv[0]);

   if ((outlog != NULL) && (outlog != stdout)) {
      fclose(outlog);
   }

   if (test_ok) {
      return 0; /* true == no error, so return zero */
   }

   return -1;
}
