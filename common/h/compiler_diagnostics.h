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
//      VLA
//              warning about C VLAs (variable length arrays) in C++
//      VLA_EXTENSION
//              clang warning about C VLAs in C++ if VLA is suppressed
//      VLA_ALL`
//              both of the above
//      VLA_GCC_PRAGMA_BUG
//              gcc <9, 11.0, and 11.1 workaround macro
//      LOGICAL_OP
//              warning about duplicate subexpressions in a logical expression
//              Is a false positive due compiler checks after macro/constant
//              propagation (eg. (x == a && x == b) if a and b are distinct
//              constants with the same physical value. Only gcc 6-8.
//      DUPLICATED_BRANCHES
//              similar to LOGICAL_OP except the expressions are the
//              conditionals of a chain of if/then/else's. Only gcc 7-8.
//      UNUSED_VARIABLE
//              clang <10 warns about variables defined solely for RIAA (locks)
//      MAYBE_UNINITIALIZED
//              gcc 12 warns that boost::optional::value_or may use an
//              unitialized value when value_or checks if it is initialized.
//
// Macros to silence unused variable warnings
//
//      DYNINST_SUPPRESS_UNUSED_VARIABLE(var)
//              indicate that variable var OK to be unused
//
// Define DYNINST_DIAGNOSTIC_NO_SUPPRESSIONS to prevents suppressions.


// Define compiler specific suppression codes, an undefined value represents no
// suppression required.  Suppression code  macro names have the form
//
//      DYNINST_SUPPRESS_CODE_<code>
//
#if defined(__GNUC__) && !defined(__clang__)
 #define DYNINST_SUPPRESS_CODE_FLEX_ARRAY                  "-Wpedantic"
 #define DYNINST_SUPPRESS_CODE_VLA                         "-Wvla"
 #if __GNUC__ < 9
  #define DYNINST_SUPPRESS_CODE_LOGICAL_OP                 "-Wlogical-op"
 #endif
 #if __GNUC__ >= 7 && __GNUC__ < 9
  #define DYNINST_SUPPRESS_CODE_DUPLICATED_BRANCHES        "-Wduplicated-branches"
 #endif
 #if __GNUC__ == 12
  #define DYNINST_SUPPRESS_CODE_MAYBE_UNINITIALIZED        "-Wmaybe-uninitialized"
 #endif
#elif defined(__clang__)
 #define DYNINST_SUPPRESS_CODE_FLEX_ARRAY                  "-Wpedantic"
 #define DYNINST_SUPPRESS_CODE_VLA                         "-Wvla"
 #define DYNINST_SUPPRESS_CODE_VLA_EXTENSION               "-Wvla-extension"
 #if __clang_major__ < 10
  #define DYNINST_SUPPRESS_CODE_UNUSED_VARIABLE            "-Wunused-variable"
 #endif
#elif defined(_MSC_VER)
 #define DYNINST_SUPPRESS_CODE_FLEX_ARRAY                  4200
#endif

// Define DYNISNT_DIAGNOSTIC_BEGIN/END macros, expands to nothing if code undefined
#ifdef DYNINST_SUPPRESS_CODE_FLEX_ARRAY
 #define DYNINST_DIAGNOSTIC_BEGIN_SUPPRESS_FLEX_ARRAY      DYNINST_DIAGNOSTIC_PUSH_SUPPRESS_CODE(FLEX_ARRAY)
 #define DYNINST_DIAGNOSTIC_END_SUPPRESS_FLEX_ARRAY        DYNINST_DIAGNOSTIC_POP
#else
 #define DYNINST_DIAGNOSTIC_BEGIN_SUPPRESS_FLEX_ARRAY
 #define DYNINST_DIAGNOSTIC_END_SUPPRESS_FLEX_ARRAY
#endif

#ifdef DYNINST_SUPPRESS_CODE_VLA
 #define DYNINST_DIAGNOSTIC_BEGIN_SUPPRESS_VLA             DYNINST_DIAGNOSTIC_PUSH_SUPPRESS_CODE(VLA)
 #define DYNINST_DIAGNOSTIC_END_SUPPRESS_VLA               DYNINST_DIAGNOSTIC_POP
#else
 #define DYNINST_DIAGNOSTIC_BEGIN_SUPPRESS_VLA
 #define DYNINST_DIAGNOSTIC_END_SUPPRESS_VLA
#endif

#ifdef DYNINST_SUPPRESS_CODE_VLA_EXTENSION
 #define DYNINST_DIAGNOSTIC_BEGIN_SUPPRESS_VLA_EXTENSION   DYNINST_DIAGNOSTIC_PUSH_SUPPRESS_CODE(VLA_EXTENSION)
 #define DYNINST_DIAGNOSTIC_END_SUPPRESS_VLA_EXTENSION     DYNINST_DIAGNOSTIC_POP
#else
 #define DYNINST_DIAGNOSTIC_BEGIN_SUPPRESS_VLA_EXTENSION
 #define DYNINST_DIAGNOSTIC_END_SUPPRESS_VLA_EXTENSION
#endif

#define DYNINST_DIAGNOSTIC_BEGIN_SUPPRESS_VLA_ALL          DYNINST_DIAGNOSTIC_BEGIN_SUPPRESS_VLA DYNINST_DIAGNOSTIC_BEGIN_SUPPRESS_VLA_EXTENSION
#define DYNINST_DIAGNOSTIC_END_SUPPRESS_VLA_ALL            DYNINST_DIAGNOSTIC_END_SUPPRESS_VLA DYNINST_DIAGNOSTIC_END_SUPPRESS_VLA_EXTENSION


// Suppressions to work around compiler specific diagnostic bugs

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

#ifdef DYNINST_SUPPRESS_CODE_UNUSED_VARIABLE
 #define DYNINST_DIAGNOSTIC_BEGIN_SUPPRESS_UNUSED_VARIABLE     DYNINST_DIAGNOSTIC_PUSH_SUPPRESS_CODE(UNUSED_VARIABLE)
 #define DYNINST_DIAGNOSTIC_END_SUPPRESS_UNUSED_VARIABLE       DYNINST_DIAGNOSTIC_POP
#else
 #define DYNINST_DIAGNOSTIC_BEGIN_SUPPRESS_UNUSED_VARIABLE
 #define DYNINST_DIAGNOSTIC_END_SUPPRESS_UNUSED_VARIABLE
#endif

#ifdef DYNINST_SUPPRESS_CODE_MAYBE_UNINITIALIZED
 #define DYNINST_DIAGNOSTIC_BEGIN_SUPPRESS_MAYBE_UNINITIALIZED DYNINST_DIAGNOSTIC_PUSH_SUPPRESS_CODE(MAYBE_UNINITIALIZED)
 #define DYNINST_DIAGNOSTIC_END_SUPPRESS_MAYBE_UNINITIALIZED   DYNINST_DIAGNOSTIC_POP
#else
 #define DYNINST_DIAGNOSTIC_BEGIN_SUPPRESS_MAYBE_UNINITIALIZED
 #define DYNINST_DIAGNOSTIC_END_SUPPRESS_MAYBE_UNINITIALIZED
#endif

// gcc <9, 11.0 and 11.1 (there may be others) have a bug where 'pragma
// diagnostic ignores' do not take affect until the next line, so this is a
// workaround for the suppression and VLA are in the same macro
#if defined(__GNUC__) && !defined(__clang__) && (__GNUC__ < 9 || __GNUC__ == 11 && __GNUC_MINOR__ < 2)
 #define DYNINST_DIAGNOSTIC_BEGIN_SUPPRESS_VLA_GCC_PRAGMA_BUG DYNINST_DIAGNOSTIC_BEGIN_SUPPRESS_VLA
 #define DYNINST_DIAGNOSTIC_END_SUPPRESS_VLA_GCC_PRAGMA_BUG   DYNINST_DIAGNOSTIC_END_SUPPRESS_VLA
#else
 #define DYNINST_DIAGNOSTIC_BEGIN_SUPPRESS_VLA_GCC_PRAGMA_BUG
 #define DYNINST_DIAGNOSTIC_END_SUPPRESS_VLA_GCC_PRAGMA_BUG
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


// use the variable in a void expression to indicate use
#ifndef DYNINST_DIAGNOSTIC_NO_SUPPRESSIONS
 #define DYNINST_SUPPRESS_UNUSED_VARIABLE(var) (void)(var)
#endif

// if not defined, expand to nothing
#ifndef DYNINST_SUPPRESS_UNUSED_VARIABLE
 #define DYNINST_SUPPRESS_UNUSED_VARIABLE(var)
#endif

#endif /* COMPILER_DIAGNOSTICS_H */
