#if !defined(__TEST0_H__)
#define __TEST0_H__

#ifndef FILE__
#define FILE__ strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__
#endif

#define PART1_TESTS 8 
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

template <class T>
bool serialize_test(unsigned int testNo, bool (*cmpfunc)(T &, T&) = NULL)
{
   fprintf(stderr, "%s[%d]: welcome to serialize test %d\n", FILE__, __LINE__, testNo);

   T control;
   T deserialize_result;

   if (!setup_control(control)) 
   {
      fprintf(stderr, "%s[%d]:  failed to setup control structure\n", FILE__, __LINE__);
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
   args.push_back("-run");
   sprintf(modifier, "%db", testNo);
   args.push_back(modifier);

   char *s = const_cast<char *>(file.c_str());
   char *ss = strdup(s);
   args.push_back(ss);

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
