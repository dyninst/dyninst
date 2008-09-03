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

/* $Id: test0.mutatee.C,v 1.1 2008/09/03 06:08:50 jaw Exp $ */

#include <stdio.h>
#include <assert.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>

#include "../../common/h/serialize.h"

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

#if 0
#ifndef FILE__
#define FILE__ strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__
#endif
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

bool runcmd(const char *file, std::vector<char *> &args)
{
   int pid = fork();

   if (pid == 0) 
   {
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
               FILE__, __LINE__, strerror(errno));
         return false;
      }

      if (!WIFEXITED(status)) 
      {
         fprintf(stderr, "%s[%d]:  process exited abnormally \n", 
               FILE__, __LINE__ );

         if (WIFSIGNALED(status)) 
         {
            int signo = WTERMSIG(status);
            fprintf(stderr, "%s[%d]:  process got signal %d \n", 
                  FILE__, __LINE__, signo );
         }

         if (WIFSTOPPED(status)) 
         {
            int signo = WSTOPSIG(status);
            fprintf(stderr, "%s[%d]:  weird, process is stopped with signal %d \n", 
                  FILE__, __LINE__, signo );
         }

         return false;
      }

      int exit_status = WEXITSTATUS(status);

      if (exit_status != 0) 
      {
         fprintf(stderr, "%s[%d]:  process returned code %d, not zero\n", 
               FILE__, __LINE__ , exit_status);
         return false;
      }

      return true;
   }
}



//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
///////////
///////////  Test 1 --
///////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

class ZeroOne : public Serializable {

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

      bool operator==(const ZeroOne &cmp) 
      {
         if (zero_one_int != cmp.zero_one_int) return false;
         if (zero_one_long != cmp.zero_one_long) return false;
#if 0
         if (zero_one_short != cmp.zero_one_short) return false;
#endif
         if (zero_one_char != cmp.zero_one_char) return false;

         return true;
      }

      bool operator!=(const ZeroOne &cmp) 
      {
         return (! (*this == cmp));
      }

      void printcmp(const ZeroOne &cmp) 
      {
         fprintf(stderr, "%s[%d]: %d %s %d\n", __FILE__, __LINE__,
               zero_one_int, 
               zero_one_int == cmp.zero_one_int ? "==" : "!=", 
               cmp.zero_one_int);
         fprintf(stderr, "%s[%d]: %l %s %l\n", __FILE__, __LINE__,
               zero_one_long, 
               zero_one_long == cmp.zero_one_long ? "==" : "!=", 
               cmp.zero_one_long);
#if 0
         fprintf(stderr, "%s[%d]: %hu %s %hu\n", __FILE__, __LINE__, 
               zero_one_short, 
               zero_one_short == cmp.zero_one_short ? "==" : "!=", 
               cmp.zero_one_short);
#endif
         fprintf(stderr, "%s[%d]: %c %s %c\n", __FILE__, __LINE__,
               zero_one_char, 
               zero_one_char == cmp.zero_one_char ? "==" : "!=", 
               cmp.zero_one_char);
      }

      void serialize(SerializerBase *sb, const char *) 
      {
         try 
         {
            gtranslate(sb, zero_one_int, NULL);
            gtranslate(sb, zero_one_unsigned_int, NULL);
            gtranslate(sb, zero_one_long, NULL);
            gtranslate(sb, zero_one_unsigned_long, NULL);
#if 0
            gtranslate(sb, zero_one_short, NULL);
#endif
            gtranslate(sb, zero_one_char, NULL);
         } 
         SER_CATCH("ZeroOne");
      }
};

bool setup_control(ZeroOne &zero1)
{
   zero1.zero_one_int = -1237843;
   zero1.zero_one_unsigned_int = 1237843;
   zero1.zero_one_long = -12378434L;
   zero1.zero_one_unsigned_long = 12378434L;
   zero1.zero_one_char = 'a';
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
   if (!cachefile) 
   {
      fprintf(stderr, "%s[%d]:  NULL param\n", FILE__, __LINE__);
      abort();
   }

   bool res = deserialize_verify<ZeroOne>(cachefile);

   if (!res) 
   {
      fprintf(stderr, "%s[%d]:  test1b failed\n", FILE__, __LINE__);
   }

   return res;
}

bool test1()
{
   bool res = serialize_test<ZeroOne>(1);

   if (!res) 
   {
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

      bool operator==(const ZeroTwo &cmp) 
      {
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

      bool operator!=(const ZeroTwo &cmp) 
      {
         return (! (*this == cmp));
      }

      void printcmp(const ZeroTwo &cmp) 
      {
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
         try 
         {
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
         } 
         SER_CATCH("ZeroTwo");
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
   bool res = serialize_test<ZeroTwo>(2);

   if (!res) 
   {
      fprintf(stderr, "%s[%d]:  test2 failed\n", FILE__, __LINE__);
   }

   return res;
}

bool test2b(const char *cachefile)
{
   if (!cachefile) 
   {
      fprintf(stderr, "%s[%d]:  NULL param\n", FILE__, __LINE__);
      abort();
   }

   bool res = deserialize_verify<ZeroTwo>(cachefile);

   if (!res) 
   {
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

      bool operator==(const ZeroThree &cmp) 
      {
         if (zero_three_ints.size() != cmp.zero_three_ints.size()) 
         {
            return false;
         }

         for (unsigned int i = 0; i < cmp.zero_three_ints.size(); ++i) 
         {
            if ((zero_three_ints[i] != cmp.zero_three_ints[i])) 
                  return false;
         }
         return true;
      }

      bool operator!=(const ZeroThree &cmp) 
      {
         return (! (*this == cmp));
      }

      void printcmp(const ZeroThree &cmp) 
      {
         if (zero_three_ints.size() != cmp.zero_three_ints.size()) 
         {
            fprintf(stderr, "%s[%d]:  vectors have sizes %d and %d\n", 
                  FILE__, __LINE__, cmp.zero_three_ints.size(), zero_three_ints.size());
         }

         for (unsigned int i = 0; i < cmp.zero_three_ints.size(); ++i) 
         {
            if ((zero_three_ints[i] != cmp.zero_three_ints[i])) 
               fprintf(stderr, "elem %d: %d -- %d\n", i, zero_three_ints[i], 
                     cmp.zero_three_ints[i]);
         }
      }

      void serialize(SerializerBase *sb, const char *)
      {
         try 
         {
            gtranslate(sb, zero_three_ints);
         } 
         SER_CATCH("ZeroThree");
      }
};

bool setup_control(ZeroThree &param)
{
   for (unsigned int i = 0; i < 50; ++i) 
   {
      param.zero_three_ints.push_back(i*100 + 2077);
   }

   return true;
}

bool test3()
{
   bool res  = serialize_test<ZeroThree>(3);

   if (!res) 
   {
      fprintf(stderr, "%s[%d]:  test3 failed\n", FILE__, __LINE__);
   }

   return res;
}

bool test3b(const char *cachefile)
{
   if (!cachefile) 
   {
      fprintf(stderr, "%s[%d]:  NULL param\n", FILE__, __LINE__);
      abort();
   }

   bool res = deserialize_verify<ZeroThree>(cachefile);

   if (!res) 
   {
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

      bool operator==(const ZeroFour &cmp) 
      {
         if (zero_four_ints.size() != cmp.zero_four_ints.size()) 
         {
            return false;
         }

         for (unsigned int i = 0; i < cmp.zero_four_ints.size(); ++i) 
         {
            if ((zero_four_ints[i] != cmp.zero_four_ints[i])) 
                  return false;
         }

         return true;
      }

      bool operator!=(const ZeroFour &cmp) 
      {
         return (! (*this == cmp));
      }

      void printcmp(const ZeroFour &cmp) 
      {
         if (zero_four_ints.size() != cmp.zero_four_ints.size()) 
         {
            fprintf(stderr, "%s[%d]:  vectors have sizes %d and %d\n", 
                  FILE__, __LINE__, cmp.zero_four_ints.size(), zero_four_ints.size());
         }

         for (unsigned int i = 0; i < cmp.zero_four_ints.size(); ++i) 
         {
            const std::vector<int> &v1 = cmp.zero_four_ints[i];
            const std::vector<int> &v2 = zero_four_ints[i];

            for (unsigned int j = 0; j < v1.size(); ++j) 
            {
               if ((v1[j] != v2[j])) 
                  fprintf(stderr, "elem %d: %d -- %d\n", j, v1[j], v2[j]);
            }
         }
      }

      void serialize(SerializerBase *sb, const char *)
      {
         try 
         {
            gtranslate(sb, zero_four_ints);
         } 
         SER_CATCH("ZeroFour");
      }

};

bool setup_control(ZeroFour &param)
{
   param.zero_four_ints.resize(50);

   for (unsigned int i = 0; i < 50; ++i) 
   {
      std::vector <int> &v  = param.zero_four_ints[i];
      v.resize(50);

      for (unsigned j = 0; j < 50; ++j) 
      {
         v[j] = i*j + 2077;
      }
   }

   return true;
}

bool test4()
{
   bool res = serialize_test<ZeroFour>(4);

   if (!res) 
   {
      fprintf(stderr, "%s[%d]:  test4 failed\n", FILE__, __LINE__);
   }

   return res;
}

bool test4b(const char *cachefile)
{
   if (!cachefile) 
   {
      fprintf(stderr, "%s[%d]:  NULL param\n", FILE__, __LINE__);
      abort();
   }

   bool res = deserialize_verify<ZeroFour>(cachefile);

   if (!res) 
   {
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

      bool operator==(const ZeroFive &cmp) 
      {
         if (zero_five_ints.size() != cmp.zero_five_ints.size()) 
         {
            return false;
         }

         for (unsigned int i = 0; i < cmp.zero_five_ints.size(); ++i) 
         {
            assert(zero_five_ints[i]);
            assert(cmp.zero_five_ints[i]);

            if ((*zero_five_ints[i] != *cmp.zero_five_ints[i])) 
                  return false;
         }

         return true;
      }

      bool operator!=(const ZeroFive &cmp) 
      {
         return (! (*this == cmp));
      }

      void printcmp(const ZeroFive &cmp) 
      {
         if (zero_five_ints.size() != cmp.zero_five_ints.size()) 
         {
            fprintf(stderr, "%s[%d]:  vectors have sizes %d and %d\n", 
                  FILE__, __LINE__, cmp.zero_five_ints.size(), zero_five_ints.size());
         }

         for (unsigned int i = 0; i < cmp.zero_five_ints.size(); ++i) 
         {
            assert(zero_five_ints[i]);
            if ((*zero_five_ints[i] != *cmp.zero_five_ints[i])) 
               fprintf(stderr, "elem %d: %d -- %d\n", i, *zero_five_ints[i], 
                     *cmp.zero_five_ints[i]);
         }
      }

      void serialize(SerializerBase *sb, const char *)
      {
         try 
         {
            gtranslate(sb,zero_five_ints);
         } 
         SER_CATCH("ZeroFive");
      }

};

bool setup_control(ZeroFive &param)
{
   for (unsigned int i = 0; i < 50; ++i) 
   {
      int *newint = new int[1];
      assert(newint);
      *newint = i*100 + 2077;
      param.zero_five_ints.push_back(newint);
   }

   return true;
}

bool test5()
{
   bool res = serialize_test<ZeroFive>(5);

   if (!res) 
   {
      fprintf(stderr, "%s[%d]:  test5 failed\n", FILE__, __LINE__);
   }

   return res;
}

bool test5b(const char *cachefile)
{
   if (!cachefile) 
   {
      fprintf(stderr, "%s[%d]:  NULL param\n", FILE__, __LINE__);
      abort();
   }

   bool res = deserialize_verify<ZeroFive>(cachefile);

   if (!res) 
   {
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

      bool operator==(ZeroSix &cmp) 
      {
         if (dictionary6.size() != cmp.dictionary6.size()) 
         {
            return false;
         }

         hash_map<char, std::string>::iterator iter1 = dictionary6.begin();
         hash_map<char, std::string>::iterator iter2 = cmp.dictionary6.begin();

         while ((iter1 != dictionary6.end()) && (iter2 != cmp.dictionary6.end())) 
         {
            char key1 = iter1->first;
            char key2 = iter2->first;
            std::string &val1 = iter1->second;
            std::string &val2 = iter2->second;

            if (key1 != key2) 
            {
               fprintf(stderr, "%s[%d]: keys differ: %c != %c\n", 
                     FILE__, __LINE__, key1, key2);
               return false;
            }

            if (val1 != val2) 
            {
               fprintf(stderr, "%s[%d]: vals differ: '%s' != '%s'\n", 
                     FILE__, __LINE__, val1.c_str(), val2.c_str());
               return false;
            }

            iter1++;
            iter2++;
         }

         return true;
      }

      bool operator!=(ZeroSix &cmp) 
      {
         return (! (*this == cmp));
      }

      void printcmp(ZeroSix &cmp) 
      {
         if (dictionary6.size() != cmp.dictionary6.size()) 
         {
            fprintf(stderr, "%s[%d]:  hashes have sizes %d and %d\n", 
                  FILE__, __LINE__, cmp.dictionary6.size(), dictionary6.size());
         }

         hash_map<char, std::string>::iterator iter1 = dictionary6.begin();
         hash_map<char, std::string>::iterator iter2 = cmp.dictionary6.begin();

         while (iter1 != dictionary6.end() && iter2 != cmp.dictionary6.end()) 
         {
            char key1 = iter1->first;
            char key2 = iter2->first;
            std::string val1 = iter1->second;
            std::string val2 = iter2->second;
            fprintf(stderr, "%s[%d]:  key1:  %c, val1: '%s'\n", 
                  FILE__, __LINE__, key1, val1.c_str());
            fprintf(stderr, "%s[%d]:  key2:  %c, val2: '%s'\n", 
                  FILE__, __LINE__, key2, val2.c_str());
            iter1++;
            iter2++;
         }
      }

      void serialize(SerializerBase *sb, const char *) 
      {
         try 
         {
            fprintf(stderr, "%s[%d]:  before gtranslate:  hash size = %d\n", 
                  FILE__, __LINE__, dictionary6.size());
            gtranslate(sb,dictionary6);
         } 
         SER_CATCH("ZeroSix");
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
   bool res = serialize_test<ZeroSix>(6);

   if (!res) 
   {
      fprintf(stderr, "%s[%d]:  test6 failed\n", FILE__, __LINE__);
   }

   return res;
}

bool test6b(const char *cachefile)
{
   if (!cachefile) 
   {
      fprintf(stderr, "%s[%d]:  NULL param\n", FILE__, __LINE__);
      abort();
   }

   bool res = deserialize_verify<ZeroSix>(cachefile);

   if (!res) 
   {
      fprintf(stderr, "%s[%d]:  test6b failed\n", FILE__, __LINE__);
   }

   return res;
}


class AuxSeven : public Serializable {
   public:
      int a7_int;
      std::string a7_string;

      bool operator==(AuxSeven &cmp) 
      {
         if (a7_int != cmp.a7_int)
            return false;

         if (a7_string != cmp.a7_string)
            return false;

         return true;
      }

      bool operator!=(AuxSeven &cmp) 
      {
         return (! (*this == cmp));
      }

      void printcmp(AuxSeven &cmp) 
      {
         fprintf(stderr, "[%d %s %d][%s %s %s]\n", a7_int, 
               (a7_int == cmp.a7_int) ? "==" : "!=", 
               cmp.a7_int, a7_string.c_str(), 
               (a7_string == cmp.a7_string) ? "==" : "!=",
               cmp.a7_string.c_str());
      }

      void serialize(SerializerBase *sb, const char *) 
      {
         try 
         {
            gtranslate(sb,a7_int);
            gtranslate(sb,a7_string);
         } 
         SER_CATCH("AuxSeven");
      }
};

bool setup_control(AuxSeven &param, int x)
{
   char buf[12];
   sprintf(buf, "%d", x);
   param.a7_int = x;
   param.a7_string = std::string(buf);
   return true;
}

class ZeroSeven : public Serializable {
   public:
      AuxSeven a7_1;
      AuxSeven a7_2;
      AuxSeven a7_3;
      AuxSeven a7_4;
      AuxSeven a7_5;
      AuxSeven a7_6;
      AuxSeven a7_7;
      AuxSeven a7_8;
      AuxSeven a7_9;
      AuxSeven a7_10;

      bool operator==(ZeroSeven &cmp) 
      {
         if (a7_1 != cmp.a7_1) return false;
         if (a7_2 != cmp.a7_2) return false;
         if (a7_3 != cmp.a7_3) return false;
         if (a7_4 != cmp.a7_4) return false;
         if (a7_5 != cmp.a7_5) return false;
         if (a7_6 != cmp.a7_6) return false;
         if (a7_7 != cmp.a7_7) return false;
         if (a7_8 != cmp.a7_8) return false;
         if (a7_9 != cmp.a7_9) return false;
         if (a7_10 != cmp.a7_10) return false;
         return true;
      }

      bool operator!=(ZeroSeven &cmp) 
      {
         return (! (*this == cmp));
      }

      void printcmp(ZeroSeven &cmp) 
      {
         a7_1.printcmp(cmp.a7_1);
         a7_2.printcmp(cmp.a7_2);
         a7_3.printcmp(cmp.a7_3);
         a7_4.printcmp(cmp.a7_4);
         a7_5.printcmp(cmp.a7_5);
         a7_6.printcmp(cmp.a7_6);
         a7_7.printcmp(cmp.a7_7);
         a7_8.printcmp(cmp.a7_8);
         a7_9.printcmp(cmp.a7_9);
         a7_10.printcmp(cmp.a7_10);
      }

      void serialize(SerializerBase *sb, const char *) 
      {
         try 
         {
            gtranslate(sb,a7_1);
            gtranslate(sb,a7_2);
            gtranslate(sb,a7_3);
            gtranslate(sb,a7_4);
            gtranslate(sb,a7_5);
            gtranslate(sb,a7_6);
            gtranslate(sb,a7_7);
            gtranslate(sb,a7_8);
            gtranslate(sb,a7_9);
            gtranslate(sb,a7_10);
         } 
         SER_CATCH("ZeroSeven");
      }
};

bool setup_control(ZeroSeven &param, int mult = 1)
{
   setup_control(param.a7_1,1 *mult);
   setup_control(param.a7_2,11 *mult);
   setup_control(param.a7_3,111 *mult);
   setup_control(param.a7_4,1111 *mult);
   setup_control(param.a7_5,11111 *mult);
   setup_control(param.a7_6,111111 *mult);
   setup_control(param.a7_7,1111111 *mult);
   setup_control(param.a7_8,11111111 *mult);
   setup_control(param.a7_9,111111111 *mult);
   setup_control(param.a7_10,1111111111 *mult);
   return true;
}

bool test7()
{
   bool res = serialize_test<ZeroSeven>(7);

   if (!res) 
   {
      fprintf(stderr, "%s[%d]:  test7 failed\n", FILE__, __LINE__);
   }

   return res;
}

bool test7b(const char *cachefile)
{
   
   if (!cachefile) {
      fprintf(stderr, "%s[%d]:  NULL param\n", FILE__, __LINE__);
      abort();
   }

   bool res = deserialize_verify<ZeroSeven>(cachefile);

   if (!res) 
   {
      fprintf(stderr, "%s[%d]:  test7b failed\n", FILE__, __LINE__);
   }

   return res;
}

class ZeroEight : public Serializable {
   public:
      std::vector<ZeroSeven> v8;

      bool operator==(ZeroEight &cmp) 
      {
         if (v8.size() != cmp.v8.size())
            return false;

         for (unsigned int i = 0; i < v8.size(); ++i) 
         {
            if (v8[i] != cmp.v8[i]) return false;
         }

         return true;
      }

      bool operator!=(ZeroEight &cmp) 
      {
         return (! (*this == cmp));
      }

      void printcmp(ZeroEight &cmp) 
      {
         if (v8.size() != cmp.v8.size()) 
         {
            fprintf(stderr, "%s[%d]:  vector sizes differ %d -- %d\n", 
                  FILE__, __LINE__, v8.size(), cmp.v8.size());

            return;
         }

         for (unsigned int i = 0; i < v8.size(); ++i) 
         {
            v8[i].printcmp(cmp.v8[i]);
         }
      }

      void serialize(SerializerBase *sb, const char *) 
      {
         try 
         {
            gtranslate(sb,v8);
         } SER_CATCH("ZeroEight");
      }
};

bool setup_control(ZeroEight &param)
{
   ZeroSeven z7_1;
   ZeroSeven z7_2;
   ZeroSeven z7_3;
   ZeroSeven z7_4;
   ZeroSeven z7_5;
   ZeroSeven z7_6;
   
   setup_control(z7_1, 1);
   setup_control(z7_2, 2);
   setup_control(z7_3, 3);
   setup_control(z7_4, 4);
   setup_control(z7_5, 5);
   setup_control(z7_6, 6);

   param.v8.push_back(z7_1);
   param.v8.push_back(z7_2);
   param.v8.push_back(z7_3);
   param.v8.push_back(z7_4);
   param.v8.push_back(z7_5);
   param.v8.push_back(z7_6);

   return true;
}

bool test8()
{
   bool res = serialize_test<ZeroEight>(8);
   
   if (!res) 
   {
      fprintf(stderr, "%s[%d]:  test8 failed\n", FILE__, __LINE__);
   }

   return res;
}

bool test8b(const char *cachefile)
{
   if (!cachefile) 
   {
      fprintf(stderr, "%s[%d]:  NULL param\n", FILE__, __LINE__);
      abort();
   }

   bool res = deserialize_verify<ZeroEight>(cachefile);

   if (!res) 
   {
      fprintf(stderr, "%s[%d]:  test8b failed\n", FILE__, __LINE__);
   }

   return res;
}

typedef struct {} zero_nine_anno;

class ZeroNine : public Serializable,
                 public Annotatable<int, zero_nine_anno, true> {
   public:
      int i9;

      bool operator==(ZeroNine &cmp) 
      {
         if (i9 != cmp.i9)
            return false;

         Annotatable<int, zero_nine_anno, true> &z9_anno = *this;
         Annotatable<int, zero_nine_anno, true> &z9_anno_cmp = cmp;

         if (z9_anno.size() != z9_anno_cmp.size()) 
         {
            return false;
         }

         for (unsigned int i = 0; i < z9_anno.size(); ++i) 
         {
            if (z9_anno[i] != z9_anno_cmp[i])
               return false;
         }
      }

      bool operator!=(ZeroNine &cmp) 
      {
         return (! (*this == cmp));
      }

      void printcmp(ZeroNine &cmp) 
      {
         fprintf(stderr, "%s[%d]:  %d ?= %d\n", FILE__, __LINE__, i9, cmp.i9);

         Annotatable<int, zero_nine_anno, true> &z9_anno = *this;
         Annotatable<int, zero_nine_anno, true> &z9_anno_cmp = cmp;

         if (z9_anno.size() != z9_anno_cmp.size()) 
         {
            fprintf(stderr, "%s[%d]:  mismatched annotations count %d vs %d\n", 
                  FILE__, __LINE__, z9_anno.size(), z9_anno_cmp.size());
            return;
         }

         for (unsigned int i = 0; i < z9_anno.size(); ++i) 
         {
            fprintf(stderr, "%s[%d]:  Annotation %d -- %d ?= %d\n", 
                  FILE__, __LINE__, i, z9_anno[i], z9_anno_cmp[i]);
         }
      }

      void serialize(SerializerBase *sb, const char *) 
      {
         try 
         {
            gtranslate(sb,i9);
         } SER_CATCH("ZeroNine");
      }
};

bool setup_control(ZeroNine &param)
{
   param.i9 = 9;
   Annotatable<int, zero_nine_anno, true> &z9_anno = param;
   z9_anno.addAnnotation(55);
   z9_anno.addAnnotation(66);
   z9_anno.addAnnotation(77);
   z9_anno.addAnnotation(88);
   return true;
}

bool test9()
{
#if 0
   bool res = serialize_test<ZeroNine>(9);

   if (!res) 
   {
      fprintf(stderr, "%s[%d]:  test9 failed\n", FILE__, __LINE__);
   }

   return res;
#endif

   fprintf(stderr, "%s[%d]:  skipping test9\n", FILE__, __LINE__);
   return true;
}

bool test9b(const char *cachefile)
{
#if 0
   if (!cachefile) 
   {
      fprintf(stderr, "%s[%d]:  NULL param\n", FILE__, __LINE__);
      abort();
   }

   bool res = deserialize_verify<ZeroNine>(cachefile);

   if (!res) 
   {
      fprintf(stderr, "%s[%d]:  test9b failed\n", FILE__, __LINE__);
   }

   return res;
#endif

   fprintf(stderr, "%s[%d]:  skipping test9b: %s\n", FILE__, __LINE__, cachefile);
   return true;
}

class ZeroTen : public Serializable {
   public:

      bool operator==(ZeroTen &cmp) 
      {
         return true;
      }

      bool operator!=(ZeroTen &cmp) 
      {
         return (! (*this == cmp));
      }

      void printcmp(ZeroTen &cmp) 
      {
      }

      void serialize(SerializerBase *sb, const char *) 
      {
         try 
         {
            //gtranslate(sb,dictionary6);
         } 
         SER_CATCH("ZeroTen");
      }
};

bool setup_control(ZeroTen &param)
{
   return true;
}

bool test10()
{
#if 0
   bool res = serialize_test<ZeroTen>(10);
   if (!res) 
   {
      fprintf(stderr, "%s[%d]:  test10 failed\n", FILE__, __LINE__);
   }

   return res;
#endif

   fprintf(stderr, "%s[%d]:  skipping test10\n", FILE__, __LINE__);
   return true;
}

bool test10b(const char *cachefile)
{
#if 0
   if (!cachefile) 
   {
      fprintf(stderr, "%s[%d]:  NULL param\n", FILE__, __LINE__);
      abort();
   }

   bool res = deserialize_verify<ZeroTen>(cachefile);
   if (!res) 
   {
      fprintf(stderr, "%s[%d]:  test10b failed\n", FILE__, __LINE__);
   }

   return res;
#endif

   fprintf(stderr, "%s[%d]:  skipping test10b\n", FILE__, __LINE__);
   return true;
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
   for (unsigned int ii=1; ii < argc; ++ii) 
   {
      fprintf(stderr, " %s", argv[ii]);
   }
   fprintf(stderr, "\"\n");

   for (i = 1; i < argc; ++i) 
   {
      if (!strcmp(argv[i], "-verbose")) 
      {
         debugPrint = TRUE;
      } 
      else if (!strcmp(argv[i], "-log")) 
      {
         if ((i + 1) >= argc) 
         {
            fprintf(stderr, "Missing log file name\n");
            exit(-1);
         }
         i += 1;
         logfilename = argv[i];
      } 
      else if (!strcmp(argv[i], "-run")) 
      {
         char *str_end_ptr = NULL;
         testNum = strtoul(argv[i+1], &str_end_ptr,10);
         if (*str_end_ptr != '\0') 
         {
            //  catch subtests that are exec'd automatically (eg, 1b, 1c)
            modifier = str_end_ptr;
            aux_file_name = argv[i+2];
            fprintf(stderr, "%s[%d]:  have test modifier %s, testNum = %d\n", 
                  FILE__, __LINE__, modifier, testNum);
            i += 1;
         }
          else 
          {
            /* end of test list */
            break;
         }
         i++;
      } 
      else 
      {
         fprintf(stderr, "Usage: %s [-verbose] -run <num>\n", argv[0]);
         fprintf(stderr, "\n\nyou asked for:\n\t\"");

         for (unsigned int i = 0; i < argc; ++i) 
         {
            fprintf(stderr, "%s ", argv[i]);
         }

         fprintf(stderr, "\"\n");
         exit(-1);
      }
   }

   if ((logfilename != NULL) && (strcmp(logfilename, "-") != 0)) 
   {
      outlog = fopen(logfilename, "a");

      if (NULL == outlog) 
      {
         fprintf(stderr, "Error opening log file %s\n", logfilename);
         exit(-1);
      }
      errlog = outlog;
   }
   else 
   {
      outlog = stdout;
      errlog = stderr;
   }

   if ((argc==1) || debugPrint)
      logstatus("Mutatee %s [C++]:\"%s\"\n", argv[0], Builder_id);

   if (argc==1) exit(0);

   bool test_ok = false;
   switch (testNum) {
      case 1:
         if (!modifier)
            test_ok = test1();
         else 
         {
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
         else 
         {
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
         else 
         {
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
         else 
         {
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
         else 
         {
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
         else 
         {
            switch (*modifier) {
               case 'b':  test_ok = test6b(aux_file_name); break;
               default:
                  fprintf(stderr, "%s[%d]:  bad modifier! %s\n", 
                        FILE__, __LINE__, modifier);
            };
         }
         break;
      case 7:
         if (!modifier)
            test_ok = test7();
         else 
         {
            switch (*modifier) {
               case 'b':  test_ok = test7b(aux_file_name); break;
               default:
                  fprintf(stderr, "%s[%d]:  bad modifier! %s\n", 
                        FILE__, __LINE__, modifier);
            };
         }
         break;
      case 8:
         if (!modifier)
            test_ok = test8();
         else 
         {
            switch (*modifier) {
               case 'b':  test_ok = test8b(aux_file_name); break;
               default:
                  fprintf(stderr, "%s[%d]:  bad modifier! %s\n", 
                        FILE__, __LINE__, modifier);
            };
         }
         break;
      case 9:
         if (!modifier)
            test_ok = test9();
         else 
         {
            switch (*modifier) {
               case 'b':  test_ok = test9b(aux_file_name); break;
               default:
                  fprintf(stderr, "%s[%d]:  bad modifier! %s\n", 
                        FILE__, __LINE__, modifier);
            };
         }
         break;
      case 10:
         if (!modifier)
            test_ok = test10();
         else 
         {
            switch (*modifier) {
               case 'b':  test_ok = test10b(aux_file_name); break;
               default:
                  fprintf(stderr, "%s[%d]:  bad modifier! %s\n", 
                        FILE__, __LINE__, modifier);
            };
         }
         break;
      default:
         fprintf(stderr, "%s[%d]: invalid test number %d in mutatee\n", 
               FILE__, __LINE__, testNum);
         break;
   }

   dprintf("Mutatee %s terminating.\n", argv[0]);

   if ((outlog != NULL) && (outlog != stdout)) 
   {
      fclose(outlog);
   }

   if (test_ok) 
   {
      return 0; /* true == no error, so return zero */
   }

   return -1;
}
