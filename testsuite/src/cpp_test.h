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

// C++ mutatee tests header file
// $Id: cpp_test.h,v 1.1 2005/09/29 20:37:47 bpellin Exp $

#ifndef CPP_TEST
#define CPP_TEST

const int CPP_DEFLT_ARG  = 1024;


class cpp_test
{
   public :

     cpp_test() {};
     virtual void func_cpp() = 0;
     virtual int func2_cpp(){ return 0;};

};


class cpp_test_util : public cpp_test
{
   public :

      cpp_test_util(int arg = 0):cpp_test(),CPP_TEST_UTIL_VAR(arg) {};

   protected :

     int CPP_TEST_UTIL_VAR;  
     void call_cpp(int test);

};

class arg_test : public cpp_test_util
{
   public :

     arg_test():cpp_test_util() {};
     void func_cpp();
     
   private :

     void dummy();
     void arg_pass(int test);
     void call_cpp(const int test, int & arg2, int arg3 = CPP_DEFLT_ARG);


};

class overload_func_test : public cpp_test_util
{
   public :

     overload_func_test():cpp_test_util() {};
     void func_cpp();

   private :

     void call_cpp(int arg1);
     void call_cpp(char * arg1);
     void call_cpp(int arg1, float arg2);

};

class overload_op_test : public cpp_test_util
{
    public :

      overload_op_test():cpp_test_util(3){};
      void func_cpp();
      int operator++(); 
};

class static_test : public cpp_test_util
{
   public :

      static_test():cpp_test_util(){};
      void func_cpp();
      static int call_cpp() { return (count++); }

   private :

      static int count;

};

class namespace_test : public cpp_test_util
{
   public :

      namespace_test():cpp_test_util() {};
      void func_cpp();

   private :

      int class_variable;
};

class exception_test : public cpp_test_util
{
    public :

      exception_test():cpp_test_util() {};
      void func_cpp();
       
    private :

      void call_cpp();
};

class sample_exception
{
    public :

      sample_exception() {};
      void response();

};

template <class T> class sample_template
{
    public :

      sample_template(T& input):item(input) {};
      T content();

    private :

      T  item;
};

class template_test : public cpp_test_util
{
    public :

      template_test():cpp_test_util() {};
      void func_cpp();

};


class decl_test : public cpp_test_util
{
   public :

      decl_test():cpp_test_util(){};
      void func_cpp(); 

   private :

      void call_cpp(int test);

};


// It should also contain members, e.g.
// call_cpp() and CPP_TEST_UTIL_VAR
class derivation_test : public cpp_test_util
{
   public :

      derivation_test():cpp_test_util(){};
      void func_cpp();
};

class stdlib_test1 : public cpp_test_util
{
   public :

      stdlib_test1():cpp_test_util(10){};
      void func_cpp();

};

class stdlib_test2 : public cpp_test_util
{
   public :

      stdlib_test2():cpp_test_util(11){};
      void func_cpp();

   private :

      void call_cpp();
};

class func_test : public cpp_test_util
{
   public :
      func_test():cpp_test_util(12) {};
      void func_cpp();
      int func2_cpp() const;

   private :
      inline void call_cpp(int test) {
        int tmp = test;
        cpp_test_util::call_cpp(tmp);
      }
};

#endif


