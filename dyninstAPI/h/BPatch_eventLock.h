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

#ifndef _BPatch_eventLock_h_
#define _BPatch_eventLock_h_

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <BPatch_dll.h>

#if !defined(os_windows)
#include <pthread.h> // Trying native windows threads for now
#else
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#endif

#if defined(os_irix) || defined (os_windows)
#define CONST_EXPORT
#else
#define CONST_EXPORT const
#endif

#if defined (os_windows)
#define CONST_ARG
#else
#define CONST_ARG const
#endif

/*
 * Class BPatch_eventLock
 *
 *
 * This class contains a (single) mutex lock controlling access to all
 * underlying BPatch (and lower level) data.
 * This object is a virtual class that should be a base class for all user
 * api's to dyninst
 */

#if defined(os_windows)
#define MUTEX_TYPE CRITICAL_SECTION
extern MUTEX_TYPE global_mutex;
#else
#define MUTEX_TYPE pthread_mutex_t
extern MUTEX_TYPE global_mutex;
#endif

extern bool mutex_created;
#if defined(os_linux) && defined (arch_x86)
#define PTHREAD_MUTEX_TYPE PTHREAD_MUTEX_RECURSIVE_NP
#define STRERROR_BUFSIZE 512
#define ERROR_BUFFER char buf[STRERROR_BUFSIZE]
#define STRERROR(x,y) strerror_r(x,y,STRERROR_BUFSIZE)
#else
#define ERROR_BUFFER
#define PTHREAD_MUTEX_TYPE PTHREAD_MUTEX_RECURSIVE
#define STRERROR_BUFSIZE 0
#define STRERROR(x,y) strerror(x)
#endif


//  BPatch_eventLock
//  
//  This class forms a base class for most DyninstAPI classes and is currently
//  responsible for managing a global mutex variable which controls access to 
//  _all_ internal dyninst/paradyn data structures.
//
//  It works in conjunction with the locking macros to isolate all mutex operations,
//  but can be used seperately as needed (should not be necessary, think twice).
//
//  Eventually may become more specialized if we move to a finer-grain of data locking.

#define __LOCK _Lock(__FILE__, __LINE__)
#define __UNLOCK _Unlock(__FILE__, __LINE__)

class BPATCH_DLL_EXPORT BPatch_eventLock {

protected:

  BPatch_eventLock(); 
  virtual ~BPatch_eventLock();

public:
  unsigned long threadID() const;

  int _Lock(const char *__file__, unsigned int __line__) const; 
  int _Trylock(const char *__file__, unsigned int __line__) const; 
  int _Unlock(const char *__file__, unsigned int __line__) const; 

};

#define LOCK_FUNCTION(x)   return lock_function_nv (this, &DYNINST_CLASS_NAME::x, \
                                                    __FILE__, __LINE__)
#define LOCK_FUNCTION_V(x)        lock_function_v  (this, &DYNINST_CLASS_NAME::x, \
                                                    __FILE__, __LINE__)

//  API_EXPORT is a multithread-lock wrapper.
//  Argument summary:
//
//  z:  The suffix appended onto the wrapped function, usually "Int", no quotes.
//      The wrapped function and the wrapping function must have distinct names
//      or else they will not be distinguishible (type-wise) overloads.
//  w:  The list of arguments passed to the wrapped function.  Really just the 
//      argument list without type information.
//  t:  Return type of the function
//  x:  The name of the function as exported to the API user.  The "internal",
//      ie unlocked function prototype is constructed by appending arg <z> to this.
//  y:  The parameter list for the function exported to the API user.

#define API_EXPORT(z, w, t, x, y)      private:  t x##z y; \
                                       public:   t x y \
                                       { LOCK_FUNCTION(x##z) w;}

//  API_EXPORT_V is used for functions that return void.
//  This variant would not be necessary were (windows) vc6 not 
//  buggy in its template specialization code.  But here we are...
//  If we move to vc7, this should be re-evaluated.  The (documented)
//  windows compiler bugs only apply to vc6.

#define API_EXPORT_V(z, w, t, x, y)    private:  t x##z y; \
                                       public:   t x y \
                                       { LOCK_FUNCTION_V(x##z) w;}

//  "CTOR" and "DTOR" flavors do not have return type.
//  wrapped function implicitly returns void.

#define API_EXPORT_CTOR(z, w, x, y)    private:  void x##z y; \
                                       public:   x y \
                                       { LOCK_FUNCTION_V(x##z) w;}

//  here t is for "tilde" (ignored, but there for dtor appearance)

#define API_EXPORT_DTOR(z, w, t, x, y) private:  void x##z y; \
                                       public:   virtual ~x y \
                                       { LOCK_FUNCTION_V(x##z) w;}

//  operators present a special case for the preprocessor
//  suffix <z> is appended to the word "operator", eg, operator_lessthan
//  (with <z> = "_lessthan"), forms the wrapped function name

#define API_EXPORT_OPER(z, w, t, x, y) private:  t operator##z y; \
                                       public:   t x y \
                                       { LOCK_FUNCTION(operator##z) w;}



//  Typed template argument paramters (used to specify operator types)
#define OPER_ARGS_0 ()
#define OPER_ARGS_1 (A1 a1)
#define OPER_ARGS_2 (A1 a1, A2 a2)
#define OPER_ARGS_3 (A1 a1, A2 a2, A3 a3)
#define OPER_ARGS_4 (A1 a1, A2 a2, A3 a3, A4 a4)
#define OPER_ARGS_5 (A1 a1, A2 a2, A3 a3, A4 a4, A5 a5)
#define OPER_ARGS_6 (A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6)
#define OPER_ARGS_7 (A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7)
#define OPER_ARGS_8 (A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8)
#define OPER_ARGS_9 (A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9)

//  Untyped template argument parameters (parameters passed to locking operators)
#define ARGS_NOTYPE_0 ()
#define ARGS_NOTYPE_1 (a1)
#define ARGS_NOTYPE_2 (a1,a2)
#define ARGS_NOTYPE_3 (a1,a2,a3)
#define ARGS_NOTYPE_4 (a1,a2,a3,a4)
#define ARGS_NOTYPE_5 (a1,a2,a3,a4,a5)
#define ARGS_NOTYPE_6 (a1,a2,a3,a4,a5,a6)
#define ARGS_NOTYPE_7 (a1,a2,a3,a4,a5,a6,a7)
#define ARGS_NOTYPE_8 (a1,a2,a3,a4,a5,a6,a7,a8)
#define ARGS_NOTYPE_9 (a1,a2,a3,a4,a5,a6,a7,a8,a9)

// Lists of argument types for locking operators (used to specify func ptr types)
#define ARG_TYPES_0 
#define ARG_TYPES_1 A1
#define ARG_TYPES_2 A1,A2
#define ARG_TYPES_3 A1,A2,A3
#define ARG_TYPES_4 A1,A2,A3,A4
#define ARG_TYPES_5 A1,A2,A3,A4,A5
#define ARG_TYPES_6 A1,A2,A3,A4,A5,A6
#define ARG_TYPES_7 A1,A2,A3,A4,A5,A6,A7
#define ARG_TYPES_8 A1,A2,A3,A4,A5,A6,A7,A8
#define ARG_TYPES_9 A1,A2,A3,A4,A5,A6,A7,A8,A9

//  EXPORT_FUNC_OBJ expands to 2 things: (1) a function-object template class (some
//  people like to call them "functors", and (2) an overloaded function that 
//  implicitly instantiates the function object.  
//
//  the function that instantiates the function-object is either called
//  locked_function_v, or locked_function_nv, depending on whether the 
//  function that is encapsulated in the function object returns a value (_nv), or
//  void (_v).  Again, this distinction is only necessary due to bugs in the windows
//  vc6 compilers.  Otherwise we could use template specialization for the creation
//  of function objects, and have uniformly named instantiation functions (and hence
//  a more uniform macro, without the special case of _V).
//
//  By implicitly instantiating the function object and immediately calling its
//  operator() with the right arguments, we can effectivly create an inline,
//  one-line function wrapper to handle all mutex locking.  This keeps other dyninst
//  code free of messy locking considerations, and, hopefully safer from accidentally
//  introduced race-conditions as development inevitably progresses.
//
//  

#define EXPORT_FUNC_OBJ(num_args, arg_types, t_classes, oper, template_spec, voidflag) \
  template  t_classes  \
  class locked_function_##num_args##_args_##voidflag { \
  private: \
  RetType (BaseType::*fptr)(arg_types); \
  BaseType *baseptr; \
  const char *__file__; \
  unsigned int __line__; \
  public: \
  explicit inline\
  locked_function_##num_args##_args_##voidflag(BaseType *bptr, \
                                               RetType (BaseType::*fptr_)(arg_types),\
                                               const char *_f, unsigned int _l) : \
                                               fptr(fptr_), baseptr(bptr), \
                                               __file__(_f), __line__(_l) {} \
  oper \
  }; \
  template t_classes \
  inline locked_function_##num_args##_args_##voidflag template_spec \
  lock_function_##voidflag(BaseType *bptr, RetType (BaseType::*fptr_)(arg_types), \
                           const char *__file__, unsigned int __line__) { \
    return locked_function_##num_args##_args_##voidflag template_spec \
                                 (bptr, fptr_, __file__, __line__); \
  }


//  OPER_NV and OPER_V are where the locking & unlocking actually happen
//  These define the "flavor" of EXPORT_FUNC_OBJ, along with <voidflag>
//  -- there is a 1-1 mapping:  <voidflag> = "v" <--> OPER_V, and
//  <voidflag> = "nv" <--> OPER_NV

#define OPER_NV(oper_args, oper_args_notype) \
  inline RetType CONST_EXPORT operator() oper_args \
  { \
    baseptr->_Lock(__file__, __line__); \
    RetType ret = (baseptr->*fptr) oper_args_notype; \
    baseptr->_Unlock(__file__, __line__); \
    return ret; \
  } 

//  RetType is implicitly void in OPER_V
//  (left as template parameter as sanity check 
//     -- the compiler will complain if its not void for some reason)

#define OPER_V(oper_args, oper_args_notype) \
  inline RetType CONST_EXPORT operator() oper_args \
  { \
    baseptr->_Lock(__file__, __line__); \
    (baseptr->*fptr) oper_args_notype; \
    baseptr->_Unlock(__file__, __line__); \
  } 

#define T_SPEC_0 <BaseType, RetType>
#define T_SPEC_1 <BaseType, RetType, A1>
#define T_SPEC_2 <BaseType, RetType, A1, A2>
#define T_SPEC_3 <BaseType, RetType, A1, A2, A3>
#define T_SPEC_4 <BaseType, RetType, A1, A2, A3, A4>
#define T_SPEC_5 <BaseType, RetType, A1, A2, A3, A4, A5>
#define T_SPEC_6 <BaseType, RetType, A1, A2, A3, A4, A5, A6>
#define T_SPEC_7 <BaseType, RetType, A1, A2, A3, A4, A5, A6, A7>
#define T_SPEC_8 <BaseType, RetType, A1, A2, A3, A4, A5, A6, A7, A8>
#define T_SPEC_9 <BaseType, RetType, A1, A2, A3, A4, A5, A6, A7, A8, A9>

#define T_CLASSES_0 <class BaseType, class RetType>
#define T_CLASSES_1 <class BaseType, class RetType, class A1>
#define T_CLASSES_2 <class BaseType, class RetType, class A1, class A2>
#define T_CLASSES_3 <class BaseType, class RetType, class A1, class A2, class A3>
#define T_CLASSES_4 <class BaseType, class RetType, class A1, class A2, class A3,\
                     class A4>
#define T_CLASSES_5 <class BaseType, class RetType, class A1, class A2, class A3,\
                     class A4, class A5>
#define T_CLASSES_6 <class BaseType, class RetType, class A1, class A2, class A3,\
                     class A4, class A5, class A6>
#define T_CLASSES_7 <class BaseType, class RetType, class A1, class A2, class A3,\
                     class A4, class A5, class A6, class A7>
#define T_CLASSES_8 <class BaseType, class RetType, class A1, class A2, class A3,\
                     class A4, class A5, class A6, class A7, class A8>
#define T_CLASSES_9 <class BaseType, class RetType, class A1, class A2, class A3,\
                     class A4, class A5, class A6, class A7, class A8, class A9>

//  Locking function objects for functions that return values
//  (At time of writing, the largest number of arguments for a dyninstAPI function was 6
//  Providing up to 9)

EXPORT_FUNC_OBJ(0,ARG_TYPES_0, T_CLASSES_0, OPER_NV(OPER_ARGS_0, ARGS_NOTYPE_0),T_SPEC_0,nv);
EXPORT_FUNC_OBJ(1,ARG_TYPES_1, T_CLASSES_1, OPER_NV(OPER_ARGS_1, ARGS_NOTYPE_1),T_SPEC_1,nv);
EXPORT_FUNC_OBJ(2,ARG_TYPES_2, T_CLASSES_2, OPER_NV(OPER_ARGS_2, ARGS_NOTYPE_2),T_SPEC_2,nv);
EXPORT_FUNC_OBJ(3,ARG_TYPES_3, T_CLASSES_3, OPER_NV(OPER_ARGS_3, ARGS_NOTYPE_3),T_SPEC_3,nv);
EXPORT_FUNC_OBJ(4,ARG_TYPES_4, T_CLASSES_4, OPER_NV(OPER_ARGS_4, ARGS_NOTYPE_4),T_SPEC_4,nv);
EXPORT_FUNC_OBJ(5,ARG_TYPES_5, T_CLASSES_5, OPER_NV(OPER_ARGS_5, ARGS_NOTYPE_5),T_SPEC_5,nv);
EXPORT_FUNC_OBJ(6,ARG_TYPES_6, T_CLASSES_6, OPER_NV(OPER_ARGS_6, ARGS_NOTYPE_6),T_SPEC_6,nv);
EXPORT_FUNC_OBJ(7,ARG_TYPES_7, T_CLASSES_7, OPER_NV(OPER_ARGS_7, ARGS_NOTYPE_7),T_SPEC_7,nv);
EXPORT_FUNC_OBJ(8,ARG_TYPES_8, T_CLASSES_8, OPER_NV(OPER_ARGS_8, ARGS_NOTYPE_8),T_SPEC_8,nv);
EXPORT_FUNC_OBJ(9,ARG_TYPES_9, T_CLASSES_9, OPER_NV(OPER_ARGS_9, ARGS_NOTYPE_9),T_SPEC_9,nv);

//  Corresponding locking function objects for functions that return void

EXPORT_FUNC_OBJ(0,ARG_TYPES_0, T_CLASSES_0, OPER_V (OPER_ARGS_0, ARGS_NOTYPE_0),T_SPEC_0, v);
EXPORT_FUNC_OBJ(1,ARG_TYPES_1, T_CLASSES_1, OPER_V (OPER_ARGS_1, ARGS_NOTYPE_1),T_SPEC_1, v);
EXPORT_FUNC_OBJ(2,ARG_TYPES_2, T_CLASSES_2, OPER_V (OPER_ARGS_2, ARGS_NOTYPE_2),T_SPEC_2, v);
EXPORT_FUNC_OBJ(3,ARG_TYPES_3, T_CLASSES_3, OPER_V (OPER_ARGS_3, ARGS_NOTYPE_3),T_SPEC_3, v);
EXPORT_FUNC_OBJ(4,ARG_TYPES_4, T_CLASSES_4, OPER_V (OPER_ARGS_4, ARGS_NOTYPE_4),T_SPEC_4, v);
EXPORT_FUNC_OBJ(5,ARG_TYPES_5, T_CLASSES_5, OPER_V (OPER_ARGS_5, ARGS_NOTYPE_5),T_SPEC_5, v);
EXPORT_FUNC_OBJ(6,ARG_TYPES_6, T_CLASSES_6, OPER_V (OPER_ARGS_6, ARGS_NOTYPE_6),T_SPEC_6, v);
EXPORT_FUNC_OBJ(7,ARG_TYPES_7, T_CLASSES_7, OPER_V (OPER_ARGS_7, ARGS_NOTYPE_7),T_SPEC_7, v);
EXPORT_FUNC_OBJ(8,ARG_TYPES_8, T_CLASSES_8, OPER_V (OPER_ARGS_8, ARGS_NOTYPE_8),T_SPEC_8, v);
EXPORT_FUNC_OBJ(9,ARG_TYPES_9, T_CLASSES_9, OPER_V (OPER_ARGS_9, ARGS_NOTYPE_9),T_SPEC_9, v);


  
#endif

