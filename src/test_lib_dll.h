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
// posix/ISO warning
#pragma warning(disable:4996)
// shared pointer warning
#pragma warning(disable:4396)

#ifdef COMPLIB_DLL_BUILD
#define COMPLIB_DLL_EXPORT __declspec(dllexport)
#else
#define COMPLIB_DLL_EXPORT __declspec(dllimport)
#endif

#ifdef TESTLIB_DLL_BUILD
// we are building the Testsuite DLL
#define TESTLIB_DLL_EXPORT __declspec(dllexport)
#define TEST_DLL_EXPORT __declspec(dllexport)
#else
#define TESTLIB_DLL_EXPORT __declspec(dllimport)
#define TEST_DLL_EXPORT __declspec(dllimport)


#endif /* TESTLIB_DLL_BUILD */
// Individual mutators should never be importing/imported
#if !defined(DLLEXPORT)
#define DLLEXPORT __declspec(dllexport)
#endif

#else

// we are not building for a Windows target 
#define TESTLIB_DLL_EXPORT
#define TEST_DLL_EXPORT
#define COMPLIB_DLL_EXPORT
#define DLLEXPORT
#endif /* _MSC_VER */

#endif /* TEST_LIB_DLL_H */
