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

// $Id: test1.C,v 1.52 2000/03/22 19:08:48 tikir Exp 
//
// libdyninst validation suite test #1
//    Author: Jeff Hollingsworth (1/7/97)
//        derived from a previous test by Bryan Buck
//

//  This program tests the basic features of the dyninst API.  
//	The mutatee that goes with this file is test5.mutatee.C
//	
//  Naming conventions:
//      All functions, variables, etc are name funcXX_YY, exprXX_YY, etc.
//          XX is the test number
//          YY is the instance withing the test
//	    func1_2 is the second function used in test case #1.
//

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#ifdef i386_unknown_nt4_0
#include <windows.h>
#include <winbase.h>
#else
#include <unistd.h>
#endif

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"
#include "BPatch_flowGraph.h"
#include "test_util.h"

// #include <vector.h>


int debugPrint = 0; // internal "mutator" tracing
int errorPrint = 0; // external "dyninst" tracing (via errorFunc)

bool forceRelocation = false;  // Force relocation upon instrumentation

bool runAllTests = true;
const unsigned int MAX_TEST = 12;
bool runTest[MAX_TEST+1];
bool passedTest[MAX_TEST+1];

BPatch *bpatch;

static const char *mutateeNameRoot = "test5.mutatee";

// control debug printf statements
void dprintf(const char *fmt, ...) {
   va_list args;
   va_start(args, fmt);

   if(debugPrint)
      vfprintf(stderr, fmt, args);

   va_end(args);

   fflush(stderr);
}

/**************************************************************************
 * Error callback
 **************************************************************************/

void errorFunc(BPatchErrorLevel level, int num, const char **params)
{
    if (num == 0) {
        // conditional reporting of warnings and informational messages
        if (errorPrint) {
            if (level == BPatchInfo)
              { if (errorPrint > 1) printf("%s\n", params[0]); }
            else
                printf("%s", params[0]);
        }
    } else {
        // reporting of actual errors
        char line[256];
        const char *msg = bpatch->getEnglishErrorString(num);
        bpatch->formatErrorString(line, sizeof(line), msg, params);
        
	printf("Error #%d (level %d): %s\n", num, level, line);
    
	// We consider some errors fatal.
	if (num == 101) {
	   exit(-1);
	}
    }
}

/**************************************************************************
 * Utility functions
 **************************************************************************/

// check that the cost of a snippet is sane.  Due to differences between
//   platforms, it is impossible to check this exactly in a machine independent
//   manner.
void checkCost(BPatch_snippet snippet)
{
    float cost;
    BPatch_snippet copy;

    // test copy constructor too.
    copy = snippet;

    cost = snippet.getCost();
    dprintf("Snippet cost=%g\n", cost);
    if (cost < 0.0) {
	printf("*Error*: negative snippet cost\n");
    } else if (cost == 0.0) {
	printf("*Warning*: zero snippet cost\n");
    } else if (cost > 0.01) {
	printf("*Error*: snippet cost of %f, exceeds max expected of 0.1",
	    cost);
    }
}


/**************************************************************************
 * Tests
 **************************************************************************/

//  
// Start Test Case #1 - (C++ argument pass)
//       
void mutatorTest1(BPatch_thread *appThread, BPatch_image *appImage)
{
#if defined(sparc_sun_solaris2_4) || defined(i386_unknown_linux2_0) || defined( ia64_unknown_linux2_4 )
  BPatch_Vector<BPatch_function *> bpfv;
  char *fn = "arg_test::call_cpp";
  if (NULL == appImage->findFunction(fn, bpfv) || !bpfv.size()
      || NULL == bpfv[0]){
    fprintf(stderr, "**Failed** test #33 (control flow graphs)\n");
    fprintf(stderr, "    Unable to find function %s\n", fn);
    exit(1);
  }
  BPatch_function *f1 = bpfv[0];  
  BPatch_Vector<BPatch_point *> *point1_1 = f1->findPoint(BPatch_subroutine);
       
   assert(point1_1);

   // check the paramter passing modes
   BPatch_variableExpr *arg0 = appImage->findVariable(*(*point1_1)[0],
       "reference");
   BPatch_variableExpr *arg1 = appImage->findVariable(*(*point1_1)[0],
       "arg1");
   BPatch_variableExpr *arg2 = appImage->findVariable(*(*point1_1)[0],
       "arg2");
   BPatch_variableExpr *arg3 = appImage->findVariable(*(*point1_1)[0],
       "arg3");
   BPatch_variableExpr *arg4 = appImage->findVariable(*(*point1_1)[0],
       "m");

   if (!arg0 || !arg1 || !arg2 || !arg3 || !arg4) {
      fprintf(stderr, "**Failed** test #1 (argument passing)\n");
      if ( !arg0 )
         fprintf(stderr, "  can't find local variable arg0\n");
      if ( !arg1 )
         fprintf(stderr, "  can't find local variable arg1\n");
      if ( !arg2 )
         fprintf(stderr, "  can't find local variable arg2\n");
      if ( !arg3 )
         fprintf(stderr, "  can't find local variable arg3\n");
      if ( !arg4 )
         fprintf(stderr, "  can't find local variable arg4\n");
      return;
   }

   BPatch_type *type1_0 = const_cast<BPatch_type *> (arg0->getType());
   BPatch_type *type1_1 = const_cast<BPatch_type *> (arg1->getType());
   BPatch_type *type1_2 = const_cast<BPatch_type *> (arg2->getType());
   BPatch_type *type1_3 = const_cast<BPatch_type *> (arg4->getType());
   assert(type1_0 && type1_1 && type1_2 && type1_3);

   if (!type1_1->isCompatible(type1_3)) {
       fprintf(stderr, "**Failed** test #1 (C++ argument pass)\n");
       fprintf(stderr,"    type1_1 reported as incompatibile with type1_3\n");
       return;
   }

   if (!type1_2->isCompatible(type1_0)) {
        fprintf(stderr, "**Failed** test #1 (C++ argument pass)\n");
        fprintf(stderr,"    type1_2 reported as incompatibile with type1_0\n")
;
        return;
   }

   BPatch_arithExpr expr1_1(BPatch_assign, *arg3, BPatch_constExpr(1));
   checkCost(expr1_1);
   appThread->insertSnippet(expr1_1, *point1_1);

   // pass a paramter to a class member function
   bpfv.clear();
   char *fn2 = "arg_test::func_cpp";
   if (NULL == appImage->findFunction(fn2, bpfv) || !bpfv.size()
       || NULL == bpfv[0]){
     fprintf(stderr, "**Failed** test #1 (C++ argument pass)\n");
     fprintf(stderr, "    Unable to find function %s\n", fn2);
     exit(1);
   }
   BPatch_function *f2 = bpfv[0];  
   BPatch_Vector<BPatch_point *> *point1_2 = f2->findPoint(BPatch_subroutine);

   if (!point1_2 || (point1_2->size() < 1)) {
     fprintf(stderr, "**Failed** test #1 (C++ argument pass)\n");
      fprintf(stderr, "Unable to find point arg_test::func_cpp - exit.\n");
      exit(-1);
   }

   bpfv.clear();
   char *fn3 = "arg_test::arg_pass";
   if (NULL == appImage->findFunction(fn3, bpfv) || !bpfv.size()
       || NULL == bpfv[0]) {
     fprintf(stderr, "**Failed** test #1 (C++ argument pass)\n");
     fprintf(stderr, "    Unable to find function %s\n", fn3);
     exit(1);
   }
   BPatch_function *call1_func = bpfv[0];  

   BPatch_variableExpr *this1 = appImage->findVariable("test1");
   if (this1 == NULL) {
      fprintf(stderr, "**Failed** test #1 (C++ argument pass)\n");
      fprintf(stderr, "Unable to find variable \"test1\"\n");
      exit(1);
   }

   BPatch_Vector<BPatch_snippet *> call1_args;
   BPatch_constExpr expr1_2((const void *)this1->getBaseAddr());
   call1_args.push_back(&expr1_2);
   BPatch_constExpr expr1_3(1);
   call1_args.push_back(&expr1_3);
   BPatch_funcCallExpr call1Expr(*call1_func, call1_args);

   checkCost(call1Expr);
   appThread->insertSnippet(call1Expr, *point1_2);
#endif
}

//
// Start Test Case #2 - (overload function)
// 
void mutatorTest2(BPatch_thread *appThread, BPatch_image *appImage)
{

#if defined(sparc_sun_solaris2_4) || defined(i386_unknown_linux2_0) || defined( ia64_unknown_linux2_4 )

  BPatch_Vector<BPatch_function *> bpfv;
  char *fn = "overload_func_test::func_cpp";
  if (NULL == appImage->findFunction(fn, bpfv) || !bpfv.size()
      || NULL == bpfv[0]){
    fprintf(stderr, "**Failed** test #2 (overloaded functions)\n");
    fprintf(stderr, "    Unable to find function %s\n", fn);
    exit(1);
  }
  BPatch_function *f1 = bpfv[0];  
  BPatch_Vector<BPatch_point *> *point2_1 = f1->findPoint(BPatch_subroutine);

  if (!point2_1 || (point2_1->size() < 2)) {
    fprintf(stderr, "Unable to find point overload_func_test::func_cpp - calls. \n");
    exit(-1);
  }

  BPatch_Vector<BPatch_point *> *point2_3 = f1->findPoint(BPatch_exit);
  if (!point2_3 || point2_3->size() <1) {
    fprintf(stderr, "Unable to find point overload_func_test::func_cpp - exit.\n");
    exit(-1);
  }

    for (int n=0; n<point2_1->size(); n++) {
       BPatch_function *func;

       if ((func = (*point2_1)[n]->getCalledFunction()) == NULL) continue;

       char fn[256];
       if (func->getName(fn, 256) == NULL) {
            fprintf(stderr, "**Failed** test #2 (overloaded functions)\n");
            fprintf(stderr, "    Can't get name of called function in overload_func_test::func_cpp\n");
            return;
       }
       if (strcmp(fn, "overload_func_test::call_cpp")) {
           fprintf(stderr, "**Failed** test #2 (overloaded functions)\n");
           fprintf(stderr, "    The called function was named \"%s\""
                           " not \"overload_func_test::call_cpp\"\n", fn);
           return;
       }
       BPatch_Vector<BPatch_point *> *point2_2 = func->findPoint(BPatch_entry);
       BPatch_Vector<BPatch_localVar *> *param = func->getParams();
       assert(point2_2 && param);

       switch (n) {
          case 0 : {

	      if ( (param->size() == 1) ||
	           ((param->size() == 2) && (!strcmp((*param)[0]->getName(), "this"))) ) 
		 //First param might be "this"!
		 break;
	      else {
                 fprintf(stderr, "**Failed** test #2 (overloaded functions)\n");
                 fprintf(stderr, "    The overloaded function has wrong number of parameters\n");
                 return;
              }
          }
          case 1 : {
              if ( (param->size() == 1) ||
                   ((param->size() == 2) && (!strcmp((*param)[0]->getName(), "this"))) )
                 //First param might be "this"!
                 break;
              else {
                 fprintf(stderr, "**Failed** test #2 (overloaded functions)\n"); 
                 fprintf(stderr, "    The overloaded function has wrong number of parameters\n");
                 return;
              }
          }
          case 2 : {
              if ( (param->size() == 2) ||
                   ((param->size() == 3) && (!strcmp((*param)[0]->getName(), "this"))) )
                 //First param might be "this"!
                 break;
              else {
                 fprintf(stderr, "**Failed** test #2 (overloaded functions)\n"); 
                 fprintf(stderr, "    The overloaded function has wrong number of parameters\n");
                 return;
              }
          }
          default : {
              fprintf(stderr, "**Failed** test #2 (overloaded functions)\n");
              fprintf(stderr, "    Incorrect number of subroutine calls from overload_func_test::func_cpp\n");
              return;
          }
       };
    }

    bpfv.clear();  
    char *fn2 = "cpp_test_util::call_cpp";
    if (NULL == appImage->findFunction(fn2, bpfv) || !bpfv.size()
	|| NULL == bpfv[0]){
      fprintf(stderr, "**Failed** test #2 (overloaded functions)\n");
      fprintf(stderr, "    Unable to find function %s\n", fn2);
      exit(1);
    }
    BPatch_function *call2_func = bpfv[0];  

    BPatch_variableExpr *this2 = appImage->findVariable("test2");
    if (this2 == NULL) {
       fprintf(stderr, "**Failed** test #2 (overloaded functions)\n");
       fprintf(stderr, "Unable to find variable \"test2\"\n");
       exit(1);
    }

    BPatch_Vector<BPatch_snippet *> call2_args;
    BPatch_constExpr expr2_0((void *)this2->getBaseAddr());
    call2_args.push_back(&expr2_0);
    BPatch_constExpr expr2_1(2);
    call2_args.push_back(&expr2_1);
    BPatch_funcCallExpr call2Expr(*call2_func, call2_args);

    checkCost(call2Expr);
    appThread->insertSnippet(call2Expr, *point2_3);
#endif
}

//
// Start Test Case #3 - (overload operator)
//      
void mutatorTest3(BPatch_thread *appThread, BPatch_image *appImage)
{

  BPatch_Vector<BPatch_function *> bpfv;
  char *fn = "overload_op_test::func_cpp";
  if (NULL == appImage->findFunction(fn, bpfv) || !bpfv.size()
      || NULL == bpfv[0]){
    fprintf(stderr, "**Failed** test #3 (overloaded operation)\n");
    fprintf(stderr, "    Unable to find function %s\n", fn);
    exit(1);
  }
  BPatch_function *f1 = bpfv[0];  
  BPatch_Vector<BPatch_point *> *point3_1 = f1->findPoint(BPatch_subroutine);

  assert(point3_1);

  int index = 0;
  BPatch_function *func;
  while (index < point3_1->size()) {
     if ((func = (*point3_1)[index]->getCalledFunction()) == NULL) {
        fprintf(stderr, "**Failed** test #3 (overload operation)\n");
        fprintf(stderr, "    Can't find the overload operator\n");
        exit(1);
     }
     char fn[256];
     if (!strcmp("overload_op_test::operator++", func->getName(fn, 256)))
        break;
     index ++;
  }

  BPatch_Vector<BPatch_point *> *point3_2 = func->findPoint(BPatch_exit);
  assert(point3_2);
  
  bpfv.clear();
  char *fn2 = "overload_op_test_call_cpp";
  if (NULL == appImage->findFunction(fn2, bpfv) || !bpfv.size()
      || NULL == bpfv[0]){
    fprintf(stderr, "**Failed** test #3 (overloaded operation)\n");
    fprintf(stderr, "    Unable to find function %s\n", fn2);
    exit(1);
  }
  BPatch_function *call3_1 = bpfv[0];  
  
  BPatch_Vector<BPatch_snippet *> opArgs;
  opArgs.push_back(new BPatch_retExpr());
  BPatch_funcCallExpr call3_1Expr(*call3_1, opArgs);
  
  checkCost(call3_1Expr);
  appThread->insertSnippet(call3_1Expr, *point3_2);
  //  int tag = 1;
  //  while (tag != 0) {}
}

//  
// Start Test Case #4 - (static member)
// 
void mutatorTest4(BPatch_thread *appThread, BPatch_image *appImage)
{
#if defined(sparc_sun_solaris2_4) || defined(i386_unknown_linux2_0) || defined( ia64_unknown_linux2_4 )
  BPatch_Vector<BPatch_function *> bpfv;
  char *fn = "static_test::func_cpp";
  if (NULL == appImage->findFunction(fn, bpfv) || !bpfv.size()
      || NULL == bpfv[0]){
    fprintf(stderr, "**Failed** test #4 (static member)\n");
    fprintf(stderr, "    Unable to find function %s\n", fn);
    exit(1);
  }
  BPatch_function *f1 = bpfv[0];  
  BPatch_Vector<BPatch_point *> *point4_1 = f1->findPoint(BPatch_subroutine);
  BPatch_Vector<BPatch_point *> *point4_3 = f1->findPoint(BPatch_exit);
  assert(point4_3);
  assert(point4_1);
  
  int index = 0;
  BPatch_function *func;
  int bound = point4_1->size();
  BPatch_Vector<BPatch_variableExpr *> vect4_1;
  
  while ((index < bound) && (vect4_1.size() < 2)) {
    if ((func = (*point4_1)[index]->getCalledFunction()) == NULL) {
      fprintf(stderr, "**Failed** test #4 (static member)\n");
      fprintf(stderr, "    Can't find the invoked function\n");
      exit(1);
    }

    char fn[256];
    if (!strcmp("static_test::call_cpp", func->getName(fn, 256))) {
      BPatch_Vector<BPatch_point *> *point4_2 = func->findPoint(BPatch_exit);
      assert(point4_2);
      
      // use getComponent to access this "count". However, getComponent is
      // causing core dump at this point
      BPatch_variableExpr *var4_1 = appImage->findVariable(*(*point4_2)[0],
							   "count");
      if (!var4_1) {
	fprintf(stderr, "**Failed** test #4 (static member)\n");
	fprintf(stderr, "  Can't find static variable count\n");
	return;
      }
      vect4_1.push_back(var4_1);
    }
    index ++;
  }
  
  if (2 != vect4_1.size()) {
    fprintf(stderr, "**Failed** test #4 (static member)\n");
    fprintf(stderr, "  Incorrect size of an vector\n");
    exit(1);
  }
  if (vect4_1[0]->getBaseAddr() != vect4_1[1]->getBaseAddr()) {
    fprintf(stderr, "**Failed** test #4 (static member)\n");
    fprintf(stderr, "  Static member does not have a same address\n");
    exit(1);
  };
  
  bpfv.clear();
  char *fn2 = "static_test_call_cpp";
  if (NULL == appImage->findFunction(fn2, bpfv) || !bpfv.size()
      || NULL == bpfv[0]){
    fprintf(stderr, "**Failed** test #4 (static member)\n");
    fprintf(stderr, "    Unable to find function %s\n", fn2);
    exit(1);
  }
  BPatch_function *call4_func = bpfv[0];  

  BPatch_Vector<BPatch_snippet *> call4_args;
  BPatch_constExpr expr4_0(4);
  call4_args.push_back(&expr4_0);
  BPatch_funcCallExpr call4Expr(*call4_func, call4_args);
  
  checkCost(call4Expr);
  appThread->insertSnippet(call4Expr, *point4_3);
  
#endif
}


//  
// Start Test Case #5 - (namespace)
// 
void mutatorTest5(BPatch_thread *appThread, BPatch_image *appImage)
{
#if defined(sparc_sun_solaris2_4) || defined(i386_unknown_linux2_0) || defined( ia64_unknown_linux2_4 )
  BPatch_Vector<BPatch_function *> bpfv;
  char *fn = "namespace_test::func_cpp";
  if (NULL == appImage->findFunction(fn, bpfv) || !bpfv.size()
      || NULL == bpfv[0]){
    fprintf(stderr, "**Failed** test #5 (namespace)\n");
    fprintf(stderr, "    Unable to find function %s\n", fn);
    exit(1);
  }
  BPatch_function *f1 = bpfv[0];  
  BPatch_Vector<BPatch_point *> *point5_1 = f1->findPoint(BPatch_exit);

   assert(point5_1);
   BPatch_variableExpr *var1 = appImage->findVariable(*(*point5_1)[0],
                                                      "local_fn_var");
   BPatch_variableExpr *var2 = appImage->findVariable(*(*point5_1)[0],
                                                      "local_file_var");
   BPatch_variableExpr *var3 = appImage->findVariable(*(*point5_1)[0],
                                                      "CPP_DEFLT_ARG");
   
   if (!var1 || !var2 || !var3) {
      fprintf(stderr, "**Failed** test #5 (namespace)\n");
      if (!var1)
         fprintf(stderr, "  can't find local variable local_fn_var\n");
      if (!var2)
         fprintf(stderr, "  can't find local variable file local_file_var\n");
      if (!var3)
         fprintf(stderr, "  can't find global variable CPP_DEFLT_ARG\n");
      return;
    }

   bpfv.clear();
   char *fn2 = "main";
   if (NULL == appImage->findFunction(fn2, bpfv) || !bpfv.size()
	 || NULL == bpfv[0]){
    fprintf(stderr, "**Failed** test #5 (namespace)\n");
    fprintf(stderr, "    Unable to find function %s\n", fn2);
    exit(1);
  }
  BPatch_function *f2 = bpfv[0];  
  BPatch_Vector<BPatch_point *> *point5_2 = f2->findPoint(BPatch_allLocations);

   if (!point5_2 || (point5_2->size() < 1)) {
      fprintf(stderr, "Unable to find point in main.\n");
      exit(-1);
   }
   BPatch_variableExpr *expr5_1=appImage->findVariable(*(*point5_2)[0], "test5");
   if (!expr5_1) {
      fprintf(stderr, "**Failed** test #5 (namespace)\n");
      fprintf(stderr, "    Unable to locate test5 in main\n");
   }
   
   BPatch_Vector<BPatch_variableExpr *> *fields = expr5_1->getComponents();
   if (!fields || fields->size() == 0 ) {
      fprintf(stderr, "**Failed** test #5 (namespace)\n");
      fprintf(stderr, "  struct lacked correct number of elements\n");
      exit(-1);
   }
   
   int index = 0;
   while ( index < fields->size() ) {
      if (!strcmp("class_variable", (*fields)[index]->getName()) ) {

	BPatch_Vector<BPatch_function *> bpfv2;
	char *fn3 = "cpp_test_util::call_cpp";
	if (NULL == appImage->findFunction(fn3, bpfv2) || !bpfv2.size()
	    || NULL == bpfv2[0]){
	  fprintf(stderr, "**Failed** test #5 (namespace)\n");
	  fprintf(stderr, "    Unable to find function %s\n", fn3);
	  exit(1);
	}
	BPatch_function *call5_func = bpfv2[0];  
            
         BPatch_variableExpr *this5 = appImage->findVariable("test5");
         if (this5 == NULL) {
            fprintf(stderr, "**Failed** test #5 (namespace)\n");
            fprintf(stderr, "Unable to find variable \"test5\"\n");
            exit(1);
         }
         
         BPatch_Vector<BPatch_snippet *> call5_args;
         
         BPatch_constExpr expr5_0((void *)this5->getBaseAddr());
         call5_args.push_back(&expr5_0);
         BPatch_constExpr expr5_1(5);
         call5_args.push_back(&expr5_1);
         BPatch_funcCallExpr call5Expr(*call5_func, call5_args);
         checkCost(call5Expr);
         appThread->insertSnippet(call5Expr, *point5_1);
         return;
      }
      index ++;
   }
   fprintf(stderr, "**Failed** test #5 (namespace)\n");
   fprintf(stderr, "    Can't find class member variables\n");
#endif
}


//  
// Start Test Case #6 - (exception)
// 
void mutatorTest6(BPatch_thread *appThread, BPatch_image *appImage)
{
#if defined(sparc_sun_solaris2_4) || defined(i386_unknown_linux2_0) || defined( ia64_unknown_linux2_4 )
  BPatch_Vector<BPatch_function *> bpfv;
  char *fn2 = "exception_test::func_cpp";
  if (NULL == appImage->findFunction(fn2, bpfv) || !bpfv.size()
      || NULL == bpfv[0]){
    fprintf(stderr, "**Failed** test #6 (exception)\n");
    fprintf(stderr, "    Unable to find function %s\n", fn2);
    exit(1);
  }
  BPatch_function *f1 = bpfv[0];  
  BPatch_Vector<BPatch_point *> *point6_1 = f1->findPoint(BPatch_subroutine);
  assert(point6_1);
   
   int index = 0;
   BPatch_function *func;
   int bound = point6_1->size();
   
   BPatch_variableExpr *testno = appImage->findVariable(*(*point6_1)[0],
                                                        "testno");
   if (!testno) {
      fprintf(stderr, "**Failed** test #6 (exception)\n");
      fprintf(stderr, "    Can't find the variable in try branch of exception statement\n");
      exit(1);
   }

   while (index < bound) {
     if ((func = (*point6_1)[index]->getCalledFunction()) == NULL) {
        fprintf(stderr, "**Failed** test #6 (exception)\n");
        fprintf(stderr, "    Can't find the invoked function\n");
        exit(1);
     }
     char fn[256];
     if (!strcmp("sample_exception::response", func->getName(fn, 256))) {
         BPatch_Vector<BPatch_point *> *point6_2 = func->findPoint(BPatch_exit);
         assert(point6_2);

	 bpfv.clear();
	 char *fn3 = "exception_test_call_cpp";
	 if (NULL == appImage->findFunction(fn3, bpfv) || !bpfv.size()
	     || NULL == bpfv[0]){
	   fprintf(stderr, "**Failed** test #6 (exception)\n");
	   fprintf(stderr, "    Unable to find function %s\n", fn3);
	   exit(1);
	 }
	 BPatch_function *call6_func = bpfv[0];
  
	 BPatch_Vector<BPatch_snippet *> call6_args;
         BPatch_constExpr expr6_0(6);
         call6_args.push_back(&expr6_0);
         BPatch_funcCallExpr call6Expr(*call6_func, call6_args);

         checkCost(call6Expr);
         appThread->insertSnippet(call6Expr, *point6_2);
         return;
     }
     index++;
   }

#endif
}

//
// Start Test Case #7 - (template)
//
void mutatorTest7(BPatch_thread *appThread, BPatch_image *appImage)
{
#if defined(sparc_sun_solaris2_4) || defined(i386_unknown_linux2_0) || defined( ia64_unknown_linux2_4 )
  BPatch_Vector<BPatch_function *> bpfv;
  char *fn2 = "template_test::func_cpp";
  if (NULL == appImage->findFunction(fn2, bpfv) || !bpfv.size()
      || NULL == bpfv[0]){
    fprintf(stderr, "**Failed** test #7 (template)\n");
    fprintf(stderr, "    Unable to find function %s\n", fn2);
    abort();
  }
  BPatch_function *f1 = bpfv[0];  
  BPatch_Vector<BPatch_point *> *point7_1 = f1->findPoint(BPatch_subroutine);
  assert(point7_1);

   int index = 0;
   int flag = 0;
   BPatch_function *func;
   int bound = point7_1->size();
   BPatch_variableExpr *content7_1;
   BPatch_variableExpr *content7_2;

   while (index < bound) {
     if ((func = (*point7_1)[index]->getCalledFunction()) == NULL) {
        fprintf(stderr, "**Failed** test #7 (template)\n");
        fprintf(stderr, "    Can't find the invoked function\n");
	abort();
     }

     char fn[256];
     if (!strcmp("sample_template<int>::content", func->getName(fn, 256))) {
         BPatch_Vector<BPatch_point *> *point7_2 = func->findPoint(BPatch_entry);
         assert(point7_2);

         content7_1 = appImage->findVariable(*(*point7_2)[0], "ret");
         if (!content7_1) {
            fprintf(stderr, "**Failed** test #7 (template)\n");
            fprintf(stderr, "  Can't find local variable ret\n");
	    abort();
         }
         flag++;
     } else if (!strcmp("sample_template<char>::content", func->getName(fn, 256))) {

            BPatch_Vector<BPatch_point *> *point7_3 = func->findPoint(BPatch_entry);
            assert(point7_3);

            content7_2 = appImage->findVariable(*(*point7_3)[0], "ret");
            if (!content7_2) {
               fprintf(stderr, "**Failed** test #7 (template)\n");
               fprintf(stderr, "  Can't find local variable ret\n");
	       abort();
            }
            flag++;
     }
     index ++;
  }

  if (flag != 2) {
     fprintf(stderr, "**Failed** test #7 (template)\n");
     abort();
  }

   BPatch_type *type7_0 = appImage->findType("int");
   BPatch_type *type7_1 = const_cast<BPatch_type *> (content7_1->getType());
   BPatch_type *type7_2 = appImage->findType("char");
   BPatch_type *type7_3 = const_cast<BPatch_type *> (content7_2->getType());

   if (!type7_0->isCompatible(type7_1)) {
      fprintf(stderr, "**Failed** test #7 (template)\n");
      fprintf(stderr,"    type7_0 reported as incompatibile with type7_1\n");
      abort();
   }

   if (!type7_2->isCompatible(type7_3)) {
      fprintf(stderr, "**Failed** test #7 (template)\n");
      fprintf(stderr,"    type7_2 reported as incompatibile with type7_3\n");
      abort();
   }
   
   bpfv.clear();
   char *fn3 = "template_test_call_cpp";
   if (NULL == appImage->findFunction(fn3, bpfv) || !bpfv.size()
       || NULL == bpfv[0]){
     fprintf(stderr, "**Failed** test #7 (template)\n");
     fprintf(stderr, "    Unable to find function %s\n", fn3);
     abort();
   }
   if (bpfv.size() > 1) {
     fprintf(stderr, "WARNING:  found %d functions matching '%s'\n", bpfv.size(), fn3);
   }
   BPatch_function *call7_func = bpfv[0];  

   BPatch_Vector<BPatch_point *> *point7_4 = f1->findPoint(BPatch_exit);
   assert(point7_4);

   BPatch_Vector<BPatch_snippet *> call7_args;
   BPatch_constExpr expr7_0(7);
   call7_args.push_back(&expr7_0);
   BPatch_funcCallExpr call7Expr(*call7_func, call7_args);

   checkCost(call7Expr);
   appThread->insertSnippet(call7Expr, *point7_4);
#endif
}

//
// Start Test Case #8 - (declaration)
//   
void mutatorTest8(BPatch_thread *appThread, BPatch_image *appImage)
{
#if defined(sparc_sun_solaris2_4) || defined(i386_unknown_linux2_0) || defined( ia64_unknown_linux2_4 )
   // Find the exit point to the procedure "func_cpp"
  BPatch_Vector<BPatch_function *> bpfv;
  char *fn = "decl_test::func_cpp";
  if (NULL == appImage->findFunction(fn, bpfv) || !bpfv.size()
      || NULL == bpfv[0]){
    fprintf(stderr, "**Failed** test #8 (declaration)\n");
    fprintf(stderr, "    Unable to find function %s\n", fn);
    exit(1);
  }
  BPatch_function *f1 = bpfv[0];  
  BPatch_Vector<BPatch_point *> *point8_1 = f1->findPoint(BPatch_exit);
  if (!point8_1 || (point8_1->size() < 1)) {
    fprintf(stderr, "Unable to find point decl_test::func_cpp - exit.\n");
    exit(-1);
  }

  bpfv.clear();
  char *fn2 = "main";
  if (NULL == appImage->findFunction(fn2, bpfv) || !bpfv.size()
      || NULL == bpfv[0]){
    fprintf(stderr, "**Failed** test #8 (declaration)\n");
    fprintf(stderr, "    Unable to find function %s\n", fn2);
    exit(1);
  }
  BPatch_function *f2 = bpfv[0];  
  BPatch_Vector<BPatch_point *> *point8_2 = f2->findPoint(BPatch_allLocations);
  if (!point8_2 || (point8_2->size() < 1)) {
    fprintf(stderr, "Unable to find point in main.\n");
    exit(-1);
  }

  bpfv.clear();
  char *fn3 = "decl_test::call_cpp";
  if (NULL == appImage->findFunction(fn3, bpfv) || !bpfv.size()
      || NULL == bpfv[0]){
    fprintf(stderr, "**Failed** test #8 (declaration)\n");
    fprintf(stderr, "    Unable to find function %s\n", fn3);
    exit(1);
  }
  BPatch_function *call8_func = bpfv[0];  

  BPatch_variableExpr *this8 = appImage->findVariable("test8");
  if (this8 == NULL) {
    fprintf(stderr, "**Failed** test #8 (declaration)\n");
    fprintf(stderr, "Unable to find variable \"test8\"\n");
    exit(1);
  }

  BPatch_Vector<BPatch_snippet *> call8_args;
  BPatch_constExpr expr8_0((void *)this8->getBaseAddr());
  call8_args.push_back(&expr8_0);
  BPatch_constExpr expr8_1(8);
  call8_args.push_back(&expr8_1);
  BPatch_funcCallExpr call8Expr(*call8_func, call8_args);

     // find the variables of different scopes
     BPatch_variableExpr *expr8_2=appImage->findVariable("CPP_DEFLT_ARG");
     BPatch_variableExpr *expr8_3=appImage->findVariable(*(*point8_2)[0], "test8");
     BPatch_variableExpr *expr8_4=appImage->findVariable(*(*point8_1)[0], "CPP_DEFLT_ARG");
     if (!expr8_2 || !expr8_3 || !expr8_4) {
           fprintf(stderr, "**Failed** test #8 (delcaration)\n");
           fprintf(stderr, "    Unable to locate one of variables\n");
           exit(1);
     }

    BPatch_Vector<BPatch_variableExpr *> *fields = expr8_3->getComponents();
    if (!fields || fields->size() == 0 ) {
          fprintf(stderr, "**Failed** test #8 (declaration)\n");
          fprintf(stderr, "  struct lacked correct number of elements\n");
          exit(-1);
     }

    int index = 0;
    while ( index < fields->size() ) {
	char fieldName[100];
	strcpy(fieldName, (*fields)[index]->getName());
       if ( !strcmp("CPP_TEST_UTIL_VAR", (*fields)[index]->getName()) ) {
           dprintf("Inserted snippet2\n");
           checkCost(call8Expr);
           appThread->insertSnippet(call8Expr, *point8_1);
           return;
       }
       index ++;
    }
    fprintf(stderr, "**Failed** test #8 (declaration)\n");
    fprintf(stderr, "    Can't find inherited class member variables\n");
#endif
}

//
// Start Test Case #9 - (derivation)
//
void mutatorTest9(BPatch_thread *, BPatch_image *appImage)
{
#if defined(sparc_sun_solaris2_4) || defined(i386_unknown_linux2_0)
   bool found = false;
   
  BPatch_Vector<BPatch_function *> bpfv;
  char *fn = "derivation_test::func_cpp";
  if (NULL == appImage->findFunction(fn, bpfv) || !bpfv.size()
      || NULL == bpfv[0]){
    fprintf(stderr, "**Failed** test #9 (derivation)\n");
    fprintf(stderr, "    Unable to find function %s\n", fn);
    exit(1);
  }
  BPatch_function *f1 = bpfv[0];  
  BPatch_Vector<BPatch_point *> *point9_1 = f1->findPoint(BPatch_exit);
  if (!point9_1 || (point9_1->size() < 1)) {
    fprintf(stderr, "Unable to find point derivation_test::func_cpp - exit.\n");
    exit(-1);
  }
   
  bpfv.clear();
  char *fn2 = "main";
  if (NULL == appImage->findFunction(fn2, bpfv) || !bpfv.size()
      || NULL == bpfv[0]){
    fprintf(stderr, "**Failed** test #9 (derivation)\n");
    fprintf(stderr, "    Unable to find function %s\n", fn2);
    exit(1);
  }
  BPatch_function *f2 = bpfv[0];  
  BPatch_Vector<BPatch_point *> *point9_2 = f2->findPoint(BPatch_allLocations);
  if (!point9_2 || (point9_2->size() < 1)) {
    fprintf(stderr, "Unable to find point in main.\n");
    exit(-1);
  }

   BPatch_variableExpr *expr9_0=appImage->findVariable(*(*point9_2)[0], "test9");
   if (!expr9_0) {
      fprintf(stderr, "**Failed** test #9 (derivation)\n");
      fprintf(stderr, "    Unable to locate one of variables\n");
      exit(1);
   }

   BPatch_Vector<BPatch_variableExpr *> *fields = expr9_0->getComponents();
   if (!fields || fields->size() == 0 ) {
         fprintf(stderr, "**Failed** test #9 (derivation)\n");
         fprintf(stderr, "  struct lacked correct number of elements\n");
         exit(-1);
   }

   int index = 0;
   while ( index < fields->size() ) {
       if ( !strcmp("call_cpp", (*fields)[index]->getName()) ) {
          found = true;
          break;
       }
       index ++;
   }
   
   if ( !found ) {
     fprintf(stderr, "**Failed** test #9 (derivation)\n");
     fprintf(stderr, "    Can't find inherited class member functions\n");
  }
#endif
}

//
// Start Test Case #10 - (find standard C++ library)
//
void mutatorTest10(BPatch_thread *appThread, BPatch_image *appImage)
{
#if defined(sparc_sun_solaris2_4) \
 || defined(i386_unknown_solaris2_5) \
 || defined(i386_unknown_linux2_0) \
 || defined(mips_sgi_irix6_4) \
 || defined(alpha_dec_osf4_0) \
 || defined(ia64_unknown_linux2_4) /* Temporary duplication - TLM. */

   char libStdC[64];
   BPatch_module *modStdC = NULL;
   BPatch_Vector<BPatch_module *> *mods = appImage->getModules();

   strcpy(libStdC, "libstdc++");

   // Lookup the libstdc++.so standard library
   if (!mods || mods->size() == 0) {
     fprintf(stderr, "**Failed test #10 (find standard C++ library)\n");
     fprintf(stderr, "  Mutator couldn't search modules of standard library\n");
     exit(1);
   }
   for (int i = 0; i < mods->size() && !(modStdC); i++) {
       char buf[1024];
       BPatch_module *m = (*mods)[i];
       m->getName(buf, 1024);
       if (!strncmp(libStdC, buf, strlen(libStdC)))
          modStdC = m;
   }
   if (! modStdC ) {
      fprintf(stderr, "**Failed test #10 (find standard C++ library)\n");
      fprintf(stderr, "  Mutator couldn't find shlib in standard library\n");
      fflush(stdout);
      exit(1);
   }

   // find ostream::operator<< function in the standard library
   BPatch_Vector<BPatch_function *> bpmv;
   if (NULL == modStdC->findFunction("ostream::operator<<", bpmv) || !bpmv.size()) {
     fprintf(stderr, "**Failed test #10 (find standard C++ library)\n");
     fprintf(stderr, "  Mutator couldn't find a function in %s\n", libStdC);
     exit(1);
   } 

#endif
}

//
// Start Test Case #11 - (replace function in standard C++ library)
//
void mutatorTest11(BPatch_thread *appThread, BPatch_image *appImage)
{
// There is no corresponding failure (test2) testing because the only
// bad input to replaceFunction is a non-existent BPatch_function.

#if defined(sparc_sun_solaris2_4) \
 || defined(alpha_dec_osf4_0) \
 || defined(i386_unknown_linux2_0) \
 || defined( ia64_unknown_linux2_4 )

   char libStdC[64];
   BPatch_module *modStdC = NULL;
   BPatch_Vector<BPatch_module *> *mods = appImage->getModules();

   strcpy(libStdC, "libstdc++");

   // Lookup the libstdc++.so standard library
   if (!mods || mods->size() == 0) {
     fprintf(stderr, "**Failed test #11 (replace function in standard C++ library)\n");
     fprintf(stderr, "  Mutator couldn't search modules of standard library\n");
     exit(1);
   }
   for (int i = 0; i < mods->size() && !(modStdC); i++) {
       char buf[1024];
       BPatch_module *m = (*mods)[i];
       m->getName(buf, 1024);
       if (!strncmp(libStdC, buf, strlen(libStdC)))
          modStdC = m;
   }
   if (! modStdC ) {
      fprintf(stderr, "**Failed test #11 (replace function in standard C++ library)\n");
      fprintf(stderr, "  Mutator couldn't find shlib in standard library\n");
      fflush(stdout);
      exit(1);
   }

   // Replace a shlib function with a shlib function
   char buf1[64], buf2[64];
   BPatch_Vector<BPatch_function *> bpmv;
   if (NULL == modStdC->findFunction("ostream::operator<<", bpmv) || !bpmv.size()) {
     fprintf(stderr, "**Failed test #10 (find standard C++ library)\n");
     fprintf(stderr, "  Mutator couldn't find a function in %s\n", libStdC);
     exit(1);
   } 
   BPatch_function *func1 = bpmv[0];

   bpmv.clear();
   if (NULL == modStdC->findFunction("istream::operator>>", bpmv) || !bpmv.size()) {
     fprintf(stderr, "**Failed test #10 (find standard C++ library)\n");
     fprintf(stderr, "  Mutator couldn't find a function in %s\n", libStdC);
     exit(1);
   } 
   BPatch_function *func2 = bpmv[0];

   func1->getName(buf1, 64);
   func2->getName(buf2, 64);

   if (! func1 || ! func2) {
       fprintf(stderr, "**Failed test #11 (replace function in standard C++ library)\n");
       fprintf(stderr, "  Mutator couldn't find a function in %s\n", libStdC);
       exit(1);
   }
   if (! appThread->replaceFunction(*func1, *func2)) {
        fprintf(stderr, "**Failed test #11 (replace function in standard C++ library)\n");
        fprintf(stderr, "  Mutator couldn't replaceFunction (shlib -> shlib)\n");
   }

   // Replace a shlib function with an a.out function
  BPatch_Vector<BPatch_function *> bpfv;
  char *fn = "stdlib_test2::call_cpp";
  if (NULL == appImage->findFunction(fn, bpfv) || !bpfv.size()
      || NULL == bpfv[0]){
    fprintf(stderr, "**Failed** test #11 (replace function in standard C++ library)\n");
    fprintf(stderr, "    Unable to find function %s\n", fn);
    exit(1);
  }
  BPatch_function *func3 = bpfv[0];  
  if (! appThread->replaceFunction(*func1, *func3)) {
    fprintf(stderr, "**Failed test #11 (replace function in standard C++ library)\n");
    fprintf(stderr, "  Mutator couldn't replaceFunction (shlib -> a.out)\n");
    exit(1);
  }

   // Replace an a.out function with a shlib function
    if (! appThread->replaceFunction(*func3, *func2) ) {
      fprintf(stderr, "**Failed test #11 (replace function in standard C++ library)\n");
      fprintf(stderr, "  Mutator couldn't replaceFunction (a.out -> shlib)\n");
      exit(1);
    }

#endif
}

//
// Start Test Case #12 - (C++ member function - virtual, const and inline)
//
void mutatorTest12(BPatch_thread *appThread, BPatch_image *appImage)
{
 
  const char *fn1="cpp_test::func2_cpp";
  const char *fn2="cpp_test::func_cpp";
  const char *fn3="func_test::func_cpp";
  const char *fn4="func_test::call_cpp";

  BPatch_Vector<BPatch_function *> bpfv;
  if (NULL == appImage->findFunction(fn1, bpfv) || !bpfv.size()
      || NULL == bpfv[0]){
    fprintf(stderr, "**Failed** test #12 (C++ member function)\n");
    fprintf(stderr, "    Unable to find function %s\n", fn1);
    exit(1);
  }
  BPatch_function *f1 = bpfv[0];  

  bpfv.clear();
  if (NULL == appImage->findFunction(fn2, bpfv) || !bpfv.size()
      || NULL == bpfv[0]){
    fprintf(stderr, "**Failed** test #12 (C++ member function)\n");
    fprintf(stderr, "    Unable to find function %s\n", fn2);
    exit(1);
  }
  BPatch_function *f2 = bpfv[0];  

  bpfv.clear();
  if (NULL == appImage->findFunction(fn3, bpfv) || !bpfv.size()
      || NULL == bpfv[0]){
    fprintf(stderr, "**Failed** test #12 (C++ member function)\n");
    fprintf(stderr, "    Unable to find function %s\n", fn3);
    exit(1);
  }
  BPatch_function *f3 = bpfv[0];  

  bpfv.clear();
  if (NULL == appImage->findFunction(fn4, bpfv) || !bpfv.size()
      || NULL == bpfv[0]){
    fprintf(stderr, "**Failed** test #12 (C++ member function)\n");
    fprintf(stderr, "    Unable to find function %s\n", fn4);
    exit(1);
  }
  BPatch_function *f4 = bpfv[0]; 
 
  BPatch_Vector<BPatch_point *> *point12_0 = f1->findPoint(BPatch_allLocations);
  BPatch_Vector<BPatch_point *> *point12_1 = f2->findPoint(BPatch_allLocations);
  BPatch_Vector<BPatch_point *> *point12_2 = f3->findPoint(BPatch_allLocations);
  BPatch_Vector<BPatch_point *> *point12_3 = f4->findPoint(BPatch_allLocations);
  
  if ( !point12_0 || (point12_0->size() < 1) ||
       !point12_1 || (point12_1->size() < 1) ||
       !point12_2 || (point12_2->size() < 1) ||
       !point12_3 || (point12_3->size() < 1)  ) {

       if ( !point12_0 || (point12_0->size() < 1) ) {
         fprintf(stderr, "**Failed** test #12 (C++ member functions)\n");
         fprintf(stderr, "     Unable to find point in an virtual function \"cpp_test::func2_cpp.\"\n");
       }
       if ( !point12_1 || (point12_1->size() < 1) ) {
         fprintf(stderr, "**Warning** test #12 (C++ member functions)\n");
         fprintf(stderr, "    Unable to find point in a pure virtual function \"cpp_test::func_cpp.\"\n");
       }
       if ( !point12_2 || (point12_2->size() < 1) ) {
         fprintf(stderr, "**Failed** test #12 (C++ member functions)\n");
         fprintf(stderr, "     Unable to find point in a const function \"func_test::func_cpp.\"\n");
       }
       if ( !point12_3 || (point12_3->size() < 1) ) {
         fprintf(stderr, "**Failed** test #12 (C++ member functions)\n");
         fprintf(stderr, "     Unable to find point in an inline function \"func_test::call_cpp.\"\n");
       }
  }

  for (int n=0; n<point12_2->size(); n++) {
     BPatch_function *func;

      if ((func = (*point12_2)[n]->getCalledFunction()) == NULL) continue;

      char fn[256];
      if (func->getName(fn, 256) == NULL) {
           fprintf(stderr, "**Failed** test #12 (C++ member function)\n");
           fprintf(stderr, "    Can't get name of called function in func_test::func_cpp\n");
           exit(1);
      }

      if (! strcmp(fn, "func_test::call_cpp") ) {
        BPatch_Vector<BPatch_localVar *> *param = func->getParams();
        assert(param);

        if ( param->size() != 0 ) {
          fprintf(stderr, "**Failed** test #12 (C++ member function)\n");
          fprintf(stderr, "    The inline function is not inlined\n");
          exit(1);
        }

        BPatch_variableExpr *var1 = appImage->findVariable(*(*point12_2)[0],
                       "tmp");

        if (var1 == NULL) {
          fprintf(stderr, "**Failed** test #12 (C++ member function)\n");
          fprintf(stderr, "    The inline function is not inlined\n");
          exit(1);
        }
       return;
      }
  }

  fprintf(stderr, "**Failed** test #12 (C++ member function)\n");
  fprintf(stderr, "  Mutator couldn't find inline function in the caller\n");
  exit(1);
}

/*******************************************************************************/
/*******************************************************************************/
/*******************************************************************************/

int mutatorMAIN(char *pathname, bool useAttach)
{
    BPatch_thread *appThread;

    // Create an instance of the BPatch library
    bpatch = new BPatch;

    // Force functions to be relocated
    if (forceRelocation) {
      bpatch->setForcedRelocation_NP(true);
    }

    // Register a callback function that prints any error messages
    bpatch->registerErrorCallback(errorFunc);

    // Start the mutatee
    printf("Starting \"%s\"\n", pathname);

    const char *child_argv[MAX_TEST+5];
   
    int n = 0;
    child_argv[n++] = pathname;
    if (debugPrint) child_argv[n++] = const_cast<char*>("-verbose");

    if (runAllTests) {
        child_argv[n++] = const_cast<char*>("-runall"); // signifies all tests
    } else {
        child_argv[n++] = const_cast<char*>("-run");
        for (unsigned int j=1; j <= MAX_TEST; j++) {
            if (runTest[j]) {
        	char str[5];
        	sprintf(str, "%d", j);
        	child_argv[n++] = strdup(str);
            }
        }
    }

    child_argv[n] = NULL;

    if (useAttach) {
	int pid = startNewProcessForAttach(pathname, child_argv);
	if (pid < 0) {
	    printf("*ERROR*: unable to start tests due to error creating mutatee process\n");
	    exit(-1);
        } else {
            dprintf("New mutatee process pid %d started; attaching...\n", pid);
	}
        P_sleep(1); // let the mutatee catch its breath for a moment
	appThread = bpatch->attachProcess(pathname, pid);
    } else {
	appThread = bpatch->createProcess(pathname, child_argv,NULL);
    }

    if (appThread == NULL) {
	fprintf(stderr, "Unable to run test program.\n");
	exit(1);
    }

    // Read the program's image and get an associated image object
    BPatch_image *appImage = appThread->getImage();

    // Signal the child that we've attached
    if (useAttach) {
	signalAttached(appThread, appImage);
    }

    int i;
    BPatch_Vector<BPatch_module *> *m = appImage->getModules();
    for (i=0; i < m->size(); i++) {
        // dprintf("func %s\n", (*m)[i]->name());
    }
    BPatch_Vector<BPatch_function *> *p = appImage->getProcedures();
    for (i=0; i < p->size(); i++) {
        // dprintf("func %s\n", (*p)[i]->name());
    }

    if (runTest[1]) mutatorTest1(appThread, appImage);
    if (runTest[2]) mutatorTest2(appThread, appImage); 
    if (runTest[3]) mutatorTest3(appThread, appImage);
    if (runTest[4]) mutatorTest4(appThread, appImage); 
    if (runTest[5]) mutatorTest5(appThread, appImage); 
    if (runTest[6]) mutatorTest6(appThread, appImage); 
    if (runTest[7]) mutatorTest7(appThread, appImage);
    if (runTest[8]) mutatorTest8(appThread, appImage);
    if (runTest[9]) mutatorTest9(appThread, appImage);

    // Start of code to continue the process.  All mutations made
    // above will be in place before the mutatee begins its tests.

    dprintf("starting program execution.\n");
    appThread->continueExecution();

    while (!appThread->isTerminated())
       bpatch->waitForStatusChange();

    int ret_val;
    if(appThread->terminationStatus() == ExitedNormally) {
       int exitCode = appThread->getExitCode();
       if (exitCode || debugPrint)
          printf("Mutatee exited with exit code 0x%x\n", exitCode);
       ret_val = exitCode;
    } else if(appThread->terminationStatus() == ExitedViaSignal) {
       int signalNum = appThread->getExitSignal();
       if (signalNum || debugPrint)
          printf("Mutatee exited from signal 0x%d\n", signalNum);
       ret_val = signalNum;
    }

    dprintf("Done.\n");
    return ret_val;
}

//
// main - decide our role and call the correct "main"
//
int
main(unsigned int argc, char *argv[])
{
    char mutateeName[128];
    char libRTname[256];

    bool N32ABI = false;
    bool useAttach = false;

    strcpy(mutateeName,mutateeNameRoot);
    libRTname[0]='\0';

    if (!getenv("DYNINSTAPI_RT_LIB")) {
	 fprintf(stderr,"Environment variable DYNINSTAPI_RT_LIB undefined:\n"
#if defined(i386_unknown_nt4_0)
		 "    using standard search strategy for libdyninstAPI_RT.dll\n");
#else
	         "    set it to the full pathname of libdyninstAPI_RT\n");   
         exit(-1);
#endif
    } else
         strcpy((char *)libRTname, (char *)getenv("DYNINSTAPI_RT_LIB"));

    unsigned int i;
    // by default run all tests
    for (i=1; i <= MAX_TEST; i++) {
        runTest[i] = true;
        passedTest[i] = false;
    }

    for (i=1; i < argc; i++) {
        if (strncmp(argv[i], "-v+", 3) == 0)    errorPrint++;
        if (strncmp(argv[i], "-v++", 4) == 0)   errorPrint++;
	if (strncmp(argv[i], "-verbose", 2) == 0) {
	    debugPrint = 1;
	} else if (!strcmp(argv[i], "-V")) {
            fprintf (stdout, "%s\n", V_libdyninstAPI);
            if (libRTname[0]) 
                fprintf (stdout, "DYNINSTAPI_RT_LIB=%s\n", libRTname);
            fflush(stdout);
	} else if (!strcmp(argv[i], "-attach")) {
	    useAttach = true;
	} else if (!strcmp(argv[i], "-skip")) {
	    unsigned int j;
	    runAllTests = false;
            for (j=i+1; j < argc; j++) {
                unsigned int testId;
                if ((testId = atoi(argv[j]))) {
                    if ((testId > 0) && (testId <= MAX_TEST)) {
                        runTest[testId] = false;
                    } else {
                        printf("invalid test %d requested\n", testId);
                        exit(-1);
                    }
                } else {
                    // end of test list
		    break;
                }
            }
            i=j-1;
	} else if (!strcmp(argv[i], "-run")) {
	    unsigned int j;
	    runAllTests = false;
            for (j=0; j <= MAX_TEST; j++) runTest[j] = false;
            for (j=i+1; j < argc; j++) {
                unsigned int testId;
                if ((testId = atoi(argv[j]))) {
                    if ((testId > 0) && (testId <= MAX_TEST)) {
                        runTest[testId] = true;
                    } else {
                        printf("invalid test %d requested\n", testId);
                        exit(-1);
                    }
                } else {
                    // end of test list
		    break;
                }
            }
            i=j-1;
	} else if (!strcmp(argv[i], "-mutatee")) {
	    i++;
            if (*argv[i]=='_')
                strcat(mutateeName,argv[i]);
            else
                strcpy(mutateeName,argv[i]);
#if defined(i386_unknown_nt4_0) || defined(i386_unknown_linux2_0) || defined(sparc_sun_solaris2_4) || defined(ia64_unknown_linux2_4)
	} else if (!strcmp(argv[i], "-relocate")) {
            forceRelocation = true;
#endif
#if defined(mips_sgi_irix6_4)
	} else if (!strcmp(argv[i], "-n32")) {
            N32ABI = true;
#endif
	} else {
	    fprintf(stderr, "Usage: test5 "
		    "[-V] [-verbose] [-attach] "
#if defined(mips_sgi_irix6_4)
		    "[-n32] "
#endif
                    "[-mutatee <test5.mutatee>] "
		    "[-run <test#> <test#> ...] "
		    "[-skip <test#> <test#> ...]\n");
            fprintf(stderr, "%d subtests\n", MAX_TEST);
	    exit(-1);
	}
    }

    if (!runAllTests) {
        printf("Running Tests: ");
	for (unsigned int j=1; j <= MAX_TEST; j++) {
	    if (runTest[j]) printf("%d ", j);
	}
	printf("\n");
    }

    // patch up the default compiler in mutatee name (if necessary)
    if (!strstr(mutateeName, "_"))
#if defined(i386_unknown_nt4_0)
        strcat(mutateeName,"_VC++");
#else
        strcat(mutateeName,"_g++");
#endif
    if (N32ABI || strstr(mutateeName,"_n32")) {
        // patch up file names based on alternate ABI (as necessary)
        if (!strstr(mutateeName, "_n32")) strcat(mutateeName,"_n32");
    }
    // patch up the platform-specific filename extensions
#if defined(i386_unknown_nt4_0)
    if (!strstr(mutateeName, ".exe")) strcat(mutateeName,".exe");
#endif
    
    int exitCode = mutatorMAIN(mutateeName, useAttach);

    return exitCode;
}
