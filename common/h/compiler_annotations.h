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

#ifndef COMPILER_ANNOTATIONS_H
#define COMPILER_ANNOTATIONS_H


/***********************************************************************
 *
 * INTERNAL HELPER MACROS
 *
 * Determine if compiler supports __has_cpp_attrribute and __has_c_attribute,
 * and check if the attribute can be used without warning giving the language
 * version the attribute was introduced (clang and gcc <=6 report true but warn
 * if attribute is used).  The DYNSINT_STD_FOR_VER_HAS_X_ATTRIBUTES parameter
 * is the minumum version of the language that the attribute was introduced.
 */
#if defined(__cplusplus) && defined(__has_cpp_attribute)
    #define DYNINST_HAS_HAS_CPP_ATTRIBUTE               1
    #define DYNINST_STD_FOR_VER_HAS_CPP_ATTRIBUTE(minV) (__GNUC__ > 6 || __cplusplus >= (minV))
#else
    #define DYNINST_HAS_HAS_CPP_ATTRIBUTE               0
    #define DYNINST_STD_FOR_VER_HAS_CPP_ATTRIBUTE(minV) 0
#endif
#if defined(__STDC_VERSION__) && defined(__has_c_attribute)
    #define DYNINST_HAS_HAS_C_ATTRIBUTE                 1
    #define DYNINST_STD_FOR_VER_HAS_C_ATTRIBUTE(minV)   (__GNUC__ > 6 || __STDC_VERSION__ >= (minV))
#else
    #define DYNINST_HAS_HAS_C_ATTRIBUTE                 0
    #define DYNINST_STD_FOR_VER_HAS_C_ATTRIBUTE(minV)   0
#endif


/***********************************************************************
 *
 * DYNINST_FALLTHROUGH
 *
 * Eliminates "this statement may fall through" warnings when used as the last
 * statment of a switch case that falls through to the next case.  The macro
 * is used as shown in indicate that case 2 should fall through to case 1.
 *
 *    switch (x)  {
 *        case 2:
 *            foo();
 *            DYNINST_FALLTHROUGH;
 *        case 1:
 *            foo();
 *            break;
 *        default:
 *            error();
 *            break;
 *    }
 */

#if DYNINST_HAS_HAS_CPP_ATTRIBUTE
    #if __has_cpp_attribute(fallthrough) && DYNINST_STD_FOR_VER_HAS_CPP_ATTRIBUTE(201703)
        #define DYNINST_FALLTHROUGH [[fallthrough]]
    #elif __has_cpp_attribute(gcc::fallthrough)
        #define DYNINST_FALLTHROUGH [[gcc::fallthrough]]
    #elif __has_cpp_attribute(clang::fallthrough)
        #define DYNINST_FALLTHROUGH [[clang::fallthrough]]
    #endif
#elif DYNINST_HAS_HAS_C_ATTRIBUTE
    #if __has_c_attribute(fallthrough) && DYNINST_STD_FOR_VER_HAS_C_ATTRIBUTE(202311L)
        #define DYNINST_FALLTHROUGH [[fallthrough]]
    #elif __STDC_VERSION__ >= 202311L
	// scoped attribute names not valid in C until C23 (:: is not a token)
	#if __has_c_attribute(gcc::fallthrough)
	    #define DYNINST_FALLTHROUGH [[gcc::fallthrough]]
	#elif __has_c_attribute(clang::fallthrough)
	    #define DYNINST_FALLTHROUGH [[clang::fallthrough]]
	#endif
    #endif
#endif

#if !defined(DYNINST_FALLTHROUGH) && defined(__has_attribute)
    #if __has_attribute(fallthrough)
        #define DYNINST_FALLTHROUGH __attribute__((fallthrough))
    #elif __cplusplus || __STDC_VERSION__ >= 202311L
	// scoped attribute names not valid in C until C23 (:: is not a token)
	#if __has_attribute(gcc::fallthrough)
	    #define DYNINST_FALLTHROUGH __attribute__((gcc::fallthrough))
	#elif __has_attribute(clang::fallthrough)
	    #define DYNINST_FALLTHROUGH __attribute__((clang::fallthrough))
	#endif
    #endif
#endif

#if !defined(DYNINST_FALLTHROUGH)
    #define DYNINST_FALLTHROUGH do {} while(0)
#endif



/***********************************************************************
 *
 * DYNINST_DEPRECATED(msg)
 *
 * Adds an annotation to a function, method, variable or type that it is
 * deprecated, and will produce a warning if it used.  The parameter msg
 * must be a quoted string.
 *
 * The annotation should be placed before the definition or declaration.
 * For example:
 *
 *   DYNINST_DEPRECRATED("Use NewFoo") int Foo();
 */

#if DYNINST_HAS_HAS_CPP_ATTRIBUTE && DYNINST_STD_FOR_VER_HAS_CPP_ATTRIBUTE(201402L)
    #if __has_cpp_attribute(deprecated)
        #define DYNINST_DEPRECATED(msg) [[deprecated(msg)]]
    #endif
#elif DYNINST_HAS_HAS_C_ATTRIBUTE && DYNINST_STD_FOR_VER_HAS_C_ATTRIBUTE(202311L)
    #if __has_c_attribute(deprecated)
        #define DYNINST_DEPRECATED(msg) [[deprecated(msg)]]
    #endif
#endif
#if !defined(DYNINST_DEPRECATED) && defined(__has_attribute)
    #if __has_attribute(deprecated)
        #define DYNINST_DEPRECATED(msg) __attribute__((deprecated(msg)))
    #endif
#endif

#if !defined(DYNINST_DEPRECATED)
    #define DYNINST_DEPRECATED(msg)
#endif



/***********************************************************************
 *
 * DYNINST_PRINTF_ANNOTATION(fmtIndex, argIndex)
 * DYNINST_SCANF_ANNOTATION(fmtIndex, argIndex)
 * DYNINST_FORMAT_ANNOTATION(fmtType, fmtIndex, argIndex)
 *
 * Annotates a function as taking a printf for scanf format string and vararg
 * list of values allowing the compiler to validate the format string and the
 * supplied arguments.  fmtIndex is the 1-based index of the format string, and
 * argIndex is the 1-based index that begins the variable argument list used by
 * printf or scanf.
 *
 * The annotation should be placed after the declaration of the function.  For
 * example:
 *
 *      myprintf(int level, char *fmt, ...) DYNINST_PRINTF_ANNOTATION(2, 3);
 */

#if defined(__has_attribute)
    #if __has_attribute(format)
        #define DYNINST_FORMAT_ANNOTATION(fmtType, fmtIndex, argIndex)  __attribute__((format(fmtType, fmtIndex, argIndex)))
    #endif
#endif

#if !defined(DYNINST_FORMAT_ANNOTATION)
    #define DYNINST_FORMAT_ANNOTATION(fmtType, fmtIndex, argIndex)
#endif

#define DYNINST_PRINTF_ANNOTATION(fmtIndex, argIndex) DYNINST_FORMAT_ANNOTATION(printf, fmtIndex, argIndex)
#define DYNINST_SCANF_ANNOTATION(fmtIndex, argIndex) DYNINST_FORMAT_ANNOTATION(scanf, fmtIndex, argIndex)



/***********************************************************************
 *
 * DYNINST_MALLOC_ANNOTATION
 * DYNINST_MALLOC_DEALLOC_ANNOTATION(freeFunction)
 * DYNINST_MALLOC_DEALLOC_POS_ANNOTATION(freeFunction, pos)
 *
 * Annotates a function as returning a unique pointer to suitable memory
 * to hold an object.  The second form names the matching deallocator, and
 * the third form indicates the position of the pointer in the argument list.
 *
 * The annotation should be placed after the declaration of the function.  For
 * example:
 *
 *      Obj* myalloc(int x)  DYNINST_MALLOC_ANNOTATION;
 *      void mydealloc2(Obj* o);
 *      Obj* myalloc2(int x) DYNINST_MALLOC_DEALLOC_ANNOTATION(mydealloc2);
 *      void mydealloc3(int y, Obj* o);
 *      Obj* myalloc3(int x) DYNINST_MALLOC_DEALLOC_POS_ANNOTATION(mydealloc3, 2);
 */

#if defined(__has_attribute)
    #if __has_attribute(malloc)
        #define DYNINST_MALLOC_ANNOTATION  __attribute__((malloc))
	#if __GNUC__ >= 11
	    // malloc attribute with 1 and 2 params is gcc 11 and later only
	    #define DYNINST_MALLOC_DEALLOC_ANNOTATION(f)  __attribute__((malloc, malloc(f)))
	    #define DYNINST_MALLOC_DEALLOC_POS_ANNOTATION(f,p)  __attribute__((malloc, malloc(f,p)))
	#endif
    #endif
#endif

#if !defined(DYNINST_MALLOC_ANNOTATION)
    #define DYNINST_MALLOC_ANNOTATION
#endif
#if !defined(DYNINST_MALLOC_DEALLOC_ANNOTATION)
    #define DYNINST_MALLOC_DEALLOC_ANNOTATION(f) DYNINST_MALLOC_ANNOTATION
#endif
#if !defined(DYNINST_MALLOC_DEALLOC_POS_ANNOTATION)
    #define DYNINST_MALLOC_DEALLOC_POS_ANNOTATION(f,p) DYNINST_MALLOC_ANNOTATION
#endif

#endif /* COMPILER_ANNOTATIONS_H */
