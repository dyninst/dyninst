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

/************************************************************************
 * lprintf.c: printf-like error functions.
************************************************************************/


/************************************************************************
 * header files.
************************************************************************/

#include "common/src/headers.h"
#include "common/src/lprintf.h"
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>

/************************************************************************
 * void log_msg(const char* msg)
 * void log_printf(void (*pfunc)(const char *), const char* fmt, ...)
 * void log_perror(void (*pfunc)(const char *), const char* msg)
 *
 * error printing functions.
************************************************************************/

static char log_buffer[8192];

void log_msg(const char* msg) {
    fprintf(stderr, "%s", msg);
}

void
log_printf(void (*pfunc)(const char *), const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vsprintf(log_buffer, fmt, ap);
    va_end(ap);
    pfunc(log_buffer);
}

void
log_perror(void (*pfunc)(const char *), const char* msg) {
    sprintf(log_buffer, "%s: %s\n", msg, strerror(errno));
    pfunc(log_buffer);
}
