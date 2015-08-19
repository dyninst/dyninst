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

// C++ mutatee tests header file
// $Id: cpp_test.h,v 1.1 2008/10/30 19:17:12 legendre Exp $

#ifndef CPP_TEST
#define CPP_TEST

#define CPP_DEFLT_ARG_VAL 1024

extern int CPP_DEFLT_ARG;

#if defined(_MSVC)
  #define STDCALL __stdcall
#else
  #define STDCALL
#endif


class cpp_test
{
   public :

     cpp_test() {};
     virtual ~cpp_test() {}
     virtual void func_cpp() = 0;
     virtual int func2_cpp() const { return 0;};

};


class cpp_test_util : public cpp_test
{
   public :

      cpp_test_util(int arg = 0):cpp_test(),CPP_TEST_UTIL_VAR(arg) {};

     void call_cpp(int test);

   protected :

     int CPP_TEST_UTIL_VAR;  

};

class arg_test : public cpp_test_util
{
   public :
     int value;

     arg_test():cpp_test_util() { value = 5; };
     void func_cpp();
     
   private :

     void dummy();
     void STDCALL arg_pass(int test);
     void call_cpp(const int test, int & arg2, int arg3 = CPP_DEFLT_ARG_VAL);
};

class overload_func_test : public cpp_test_util
{
   public :

     overload_func_test(): cpp_test_util() {};
     void func_cpp();

   private :
     void call_cpp(int arg1);
     void call_cpp(const char * arg1);
     void call_cpp(int arg1, float arg2);
     void pass();
};

class overload_op_test : public cpp_test_util
{
    public :

      overload_op_test():cpp_test_util(3){};
      void func_cpp();
      void STDCALL  call_cpp(int arg);
      int operator++(); 
};

class static_test : public cpp_test_util
{
   public :

      static_test():cpp_test_util(){};
      void func_cpp();
      void pass();
      static int call_cpp() { return (count++); }

   private :

      static int count;

};

class namespace_test : public cpp_test_util
{
   public :

      namespace_test():cpp_test_util() {};
      void func_cpp();
      void pass();

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

      void STDCALL call_cpp(int test);

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


