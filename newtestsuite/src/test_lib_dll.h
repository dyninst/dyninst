#ifndef TEST_LIB_DLL_H
#define TEST_LIB_DLL_H

#if defined(_MSC_VER)

// we get numerous spurious warnings about having some template classes
// needing to have a dll-interface if instances of these classes are
// to be used by classes whose public interfaces are exported from a DLL.
// Specifing the template classes with a DLL export interface doesn't 
// satisfy the compiler.  Until the compiler handles instantiated
// templates exported from DLLs better, we disable the warning when building
// or using the dyninstAPI DLL.
#pragma warning(disable:4251)
#pragma warning(disable:4275)
#pragma warning(disable:4786)

#ifdef TESTLIB_DLL_BUILD
// we are building the Testsuite DLL
#define TESTLIB_DLL_EXPORT __declspec(dllexport)
#define TEST_DLL_EXPORT __declspec(dllexport)
#else
#define TESTLIB_DLL_EXPORT __declspec(dllimport)
#define TEST_DLL_EXPORT __declspec(dllimport)

#endif /* TESTLIB_DLL_BUILD */
#else

// we are not building for a Windows target 
#define TESTLIB_DLL_EXPORT
#define TEST_DLL_EXPORT
#endif /* _MSC_VER */

#endif /* TEST_LIB_DLL_H */
