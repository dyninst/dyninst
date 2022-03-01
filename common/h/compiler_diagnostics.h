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

#ifndef COMPILER_DIAGNOSTICS_H
#define COMPILER_DIAGNOSTICS_H

// This file defines macros to suppress compiler diagnostics for a region of
// code.  They are used to suppress diagnostic that are due to 1) non-standard
// code and 2) the compiler produced false positives.  They expand to nothing
// if not applicable with the current compiler.
//
// The macros to begin and end the region take the form:
//
//      DYNINST_DIAGNOSTIC_BEGIN_SUPPRESS_<code>
//      DYNINST_DIAGNOSTIC_END_SUPPRESS_<code>
//
// They should be place on a lines of their own without trailing '()' or ';'.
//
// Currently defined value for <code> are
//
//      FLEX_ARRAY
//              warning about C flexible arrays in C++
//      LOGICAL_OP
//              warning about duplicate subexpressions in a logical expression
//              Is a false positive due compiler checks after macro/constant
//              propagation (eg. (x == a && x == b) if a and b are distinct
//              constants with the same physical value. Only gcc 6-8.
//      DUPLICATED_BRANCHES
//              similar to LOGICAL_OP except the expressions are the
//              conditionals of a chain of if/then/else's. Only gcc 7-8.
//
// Define DYNINST_DIAGNOSTIC_NO_SUPPRESSIONS to prevents suppressions.


// Define compiler specific suppression codes, an undefined value represents no
// suppression required.  Suppression code  macro names have the form
//
//      DYNINST_SUPPRESS_CODE_<code>
//
#if defined(__GNUC__)
 #define DYNINST_SUPPRESS_CODE_FLEX_ARRAY                  "-Wpedantic"
 #if __GNUC__ < 9
  #define DYNINST_SUPPRESS_CODE_LOGICAL_OP                 "-Wlogical-op"
 #endif
 #if __GNUC__ >= 7 && __GNUC__ < 9
  #define DYNINST_SUPPRESS_CODE_DUPLICATED_BRANCHES        "-Wduplicated-branches"
 #endif
#elif defined(_MSC_VER)
  #define DYNINST_SUPPRESS_CODE_FLEX_ARRAY                 4200
#endif

// Define DYNISNT_DIAGNOSTIC_BEGIN/END macros, expands to nothing if code undefined
#ifdef DYNINST_SUPPRESS_CODE_FLEX_ARRAY
 #define DYNINST_DIAGNOSTIC_BEGIN_SUPPRESS_FLEX_ARRAY      DYNINST_DIAGNOSTIC_PUSH_SUPPRESS_CODE(FLEX_ARRAY)
 #define DYNINST_DIAGNOSTIC_END_SUPPRESS_FLEX_ARRAY        DYNINST_DIAGNOSTIC_POP
#else
 #define DYNINST_DIAGNOSTIC_BEGIN_SUPPRESS_FLEX_ARRAY
 #define DYNINST_DIAGNOSTIC_END_SUPPRESS_FLEX_ARRAY
#endif

#ifdef DYNINST_SUPPRESS_CODE_LOGICAL_OP
 #define DYNINST_DIAGNOSTIC_BEGIN_SUPPRESS_LOGICAL_OP      DYNINST_DIAGNOSTIC_PUSH_SUPPRESS_CODE(LOGICAL_OP)
 #define DYNINST_DIAGNOSTIC_END_SUPPRESS_LOGICAL_OP        DYNINST_DIAGNOSTIC_POP
#else
 #define DYNINST_DIAGNOSTIC_BEGIN_SUPPRESS_LOGICAL_OP
 #define DYNINST_DIAGNOSTIC_END_SUPPRESS_LOGICAL_OP
#endif

#ifdef DYNINST_SUPPRESS_CODE_DUPLICATED_BRANCHES
 #define DYNINST_DIAGNOSTIC_BEGIN_SUPPRESS_DUPLICATED_BRANCHES DYNINST_DIAGNOSTIC_PUSH_SUPPRESS_CODE(DUPLICATED_BRANCHES)
 #define DYNINST_DIAGNOSTIC_END_SUPPRESS_DUPLICATED_BRANCHES   DYNINST_DIAGNOSTIC_POP
#else
 #define DYNINST_DIAGNOSTIC_BEGIN_SUPPRESS_DUPLICATED_BRANCHES
 #define DYNINST_DIAGNOSTIC_END_SUPPRESS_DUPLICATED_BRANCHES
#endif


// Create pragma from parameters
#define DYNINST_Pragma(x)                       _Pragma(#x)

// Create compiler specific macros
//
//      DYNINST_DIAGNOSTIC_Pragma(x)    - diagnostic pragma for x which is unquoted
//      DYNINST_DIAGNOSTIC_SUPPRESS(x)  - diagnostic pragma to suppress warning x:
//                                              quoted string (gcc), number (MSVC)

#ifndef DYNINST_DIAGNOSTIC_NO_SUPPRESSIONS
 #if defined(__GNUC__)
  #define DYNINST_DIAGNOSTIC_Pragma(x)          DYNINST_Pragma(GCC diagnostic x)
  #define DYNINST_DIAGNOSTIC_SUPPRESS(x)        DYNINST_DIAGNOSTIC_Pragma(ignored x)
 #elif defined(_MSC_VER)
  #define DYNINST_DIAGNOSTIC_Pragma(x)          DYNINST_Pragma(warning(x))
  #define DYNINST_DIAGNOSTIC_SUPPRESS(x)        DYNINST_DIAGNOSTIC_Pragma(disable:x)
 #endif
#endif

// if not defined, expand to nothing
#ifndef DYNINST_DIAGNOSTIC_Pragma
 #define DYNINST_DIAGNOSTIC_Pragma(x)
#endif
#ifndef DYNINST_DIAGNOSTIC_SUPPRESS
 #define DYNINST_DIAGNOSTIC_SUPPRESS(x)
#endif

// Define macros in terms of compiler specific macros
//
//      DYNINST_DIAGNOSTIC_POP                  - pop stack of pushed diagnostic state
//      DYNINST_DIAGNOSTIC_PUSH                 - push current diagnostic state on stack
//      DYNINST_DIAGNOSTIC_PUSH_SUPPRESS(x)     - push diagnostic state and add suppression x

#define DYNINST_DIAGNOSTIC_POP                  DYNINST_DIAGNOSTIC_Pragma(pop)
#define DYNINST_DIAGNOSTIC_PUSH                 DYNINST_DIAGNOSTIC_Pragma(push)
#define DYNINST_DIAGNOSTIC_PUSH_SUPPRESS(x)     DYNINST_DIAGNOSTIC_PUSH         \
                                                    DYNINST_DIAGNOSTIC_SUPPRESS(x)
#define DYNINST_DIAGNOSTIC_PUSH_SUPPRESS_CODE(x) DYNINST_DIAGNOSTIC_PUSH_SUPPRESS(DYNINST_SUPPRESS_CODE_##x)

#endif /* COMPILER_DIAGNOSTICS_H */
