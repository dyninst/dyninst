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

#include <stdarg.h>
#include "../h/thread.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef CURRENT_FILE
#error Do not include thr_debug.h without defining CURRENT_FILE
#else
/* This indirection is necessary so that CURRENT_FILE will be expanded. */
#define __pastetoks(x,y) x ## y
#define pastetoks(x,y) __pastetoks(x,y)
#define thr_debug_msg pastetoks(thr_debug_msg_for_, CURRENT_FILE)
#endif

#ifndef __GNUC__
#define CURRENT_FUNCTION ((const char*)0)
#else
#define CURRENT_FUNCTION __PRETTY_FUNCTION__
#endif

#ifndef DO_DEBUG_LIBPDTHREAD
#define DO_DEBUG_LIBPDTHREAD 0
#endif

static void thr_debug_msg(const char* func_name, const char* format, ...);

#if DO_DEBUG_LIBPDTHREAD == 1

static void thr_debug_msg(const char* func_name, const char* format, ...) {
    va_list ap;
    va_start(ap, format);

    if (do_traces_for->get((char*)func_name, strlen(func_name)) != 0) return; 

    static const char* preamble_format = "[id: (%d,%d,%d,\"%s\"); func: %s] ";
    const char* name = thr_name(NULL);

    name = name ? name : "no name";
    
    unsigned len = 2048 + strlen(preamble_format) + (func_name ? strlen(func_name) : 0) + strlen(format) + strlen(name);
    char* real_format = 
        new char[len];
    unsigned ct = snprintf(real_format, 256,
                    preamble_format, getpid(), thr_self(),
                    Thread::GetSelfId(),
                    name, func_name);
    strncat(real_format, format, len - ct - 1);
    vfprintf(stderr, real_format, ap); 
    vfprintf(stderr, format, ap); 
    delete [] real_format; 
}

#define DEBUGGING_CURRENT_MODULE
#else
#if DO_DEBUG_LIBPDTHREAD == 0
inline static void thr_debug_msg(const char* func_name, const char* format, ...) { }
#else
#error Do not include thr_debug.h without checking to ensure that debugging is either enabled or disabled
#endif
#endif

