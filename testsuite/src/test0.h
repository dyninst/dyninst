#if !defined(__TEST0_H__)
#define __TEST0_H__

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "Annotatable.h"
#include "Serialization.h"

#ifndef FILE__
#define FILE__ strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__
#endif

#define PART1_TESTS 9
#define PART2_TESTS 1

class Tempfile {

   char *fname;
   int fd;

   public:

   Tempfile() 
   {
      fname = strdup("/tmp/tmpfileXXXXXX");
      fd = mkstemp(fname);

      if (-1 == fd) 
      {
         fprintf(stderr, "%s[%d]:  failed to make temp file\n", __FILE__, __LINE__);
         abort();
      }
   }

   ~Tempfile() 
   {
      if (0 != unlink (fname)) 
      {
         fprintf(stderr, "%s[%d]:  unlink failed: %s\n",
               __FILE__, __LINE__, strerror(errno));
      }
      free (fname);
   }

   const char *getName() 
   {
      return fname;
   }
};

template<class T>
bool deserialize_verify(const char *cachefile, bool (*cmpfunc)(T &, T&) = NULL)
{
   T control;
   T deserialize_class;

   setup_control(control);
   assert(cachefile);
   std::string file(cachefile);

   SerializerBin sb("BinSerializer", file, sd_deserialize, true);
   deserialize_class.serialize(&sb, NULL);

   if (cmpfunc) 
   {
      if ((*cmpfunc)(control,deserialize_class)) 
      {
         fprintf(stderr, "%s[%d]:  deserialize failed\n", __FILE__, __LINE__);
         control.printcmp(deserialize_class);
         return false;
      }
      else 
      {
         fprintf(stderr, "%s[%d]:  deserialize succeeded\n", __FILE__, __LINE__);
      }
   }
   else {
      if (control != deserialize_class) 
      {
         fprintf(stderr, "%s[%d]:  deserialize failed\n", __FILE__, __LINE__);
         control.printcmp(deserialize_class);
         return false;
      }
      else 
      {
         fprintf(stderr, "%s[%d]:  deserialize succeeded\n", __FILE__, __LINE__);
      }
   }

   return true;
}

class RunCommand {
   public:
      RunCommand() {}
      bool operator()(const char *file, std::vector<char *> &args)
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
};

template <class T>
bool serialize_test(unsigned int testNo, 
      const char *prog_name, 
      bool (*cmpfunc)(T &, T&) = NULL)
{
   fprintf(stderr, "%s[%d]: welcome to serialize test %d\n", 
         FILE__, __LINE__, testNo);

   T control;
   T deserialize_result;

   if (!setup_control(control)) 
   {
      fprintf(stderr, "%s[%d]:  failed to setup control structure\n", 
            FILE__, __LINE__);
      return false;
   }

   fprintf(stderr, "%s[%d]: welcome to serialize test %d, before ttb\n",
         FILE__, __LINE__, testNo);

   Tempfile tf;
   std::string file(tf.getName());

   SerializerBin sb_serializer("SerializerBin", file, sd_serialize, true);
   control.serialize(&sb_serializer, NULL);

   fflush(NULL);

   SerializerBin sb_deserializer("DeserializerBin", file, sd_deserialize, true);
   deserialize_result.serialize(&sb_deserializer, NULL);

   if (cmpfunc) 
   {
      if ((*cmpfunc)(deserialize_result,control)) 
      {
         fprintf(stderr, "%s[%d]:  deserialize failed\n", __FILE__, __LINE__);
         control.printcmp(deserialize_result);
         return false;
      }
   }
   else 
   {
      if (deserialize_result != control) 
      {
         fprintf(stderr, "%s[%d]:  deserialize failed\n", __FILE__, __LINE__);
         control.printcmp(deserialize_result);
         return false;
      }
   }

   fprintf(stderr, "%s[%d]:  deserialize succeeded\n", __FILE__, __LINE__);

         //  so far so good....  now launch a new process to do the same deserialization
   //  (verify that no static state is carrying over)

   char modifier[6];
   std::vector<char *> args;
   args.push_back((char *)"-run");
   sprintf(modifier, "%db", testNo);
   args.push_back(modifier);

   char *s = const_cast<char *>(file.c_str());
   char *ss = strdup(s);
   args.push_back(ss);

   RunCommand runcmd;
   if (!runcmd(prog_name, args)) 
   {
      fprintf(stderr, "%s[%d]:  \"%s", __FILE__, __LINE__, prog_name);

      for (unsigned int i = 0; i < args.size(); ++i) 
      {
         fprintf(stderr, " %s", args[i]);
      }

      fprintf(stderr, "\"failed\n");
      return false;
   }

   return true;
}


#endif
