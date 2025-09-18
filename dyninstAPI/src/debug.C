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

// $Id: debug.C,v 1.6 2008/02/23 02:09:05 jaw Exp $

#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <string>
#include "BPatch.h"
#include "dyninstAPI/src/debug.h"
#include "common/src/dthread.h"
#include "compiler_annotations.h"
#include "os.h"

unsigned long getExecThreadID() {
#if defined(os_windows)
    return (unsigned long) _threadid;
#else
    return (unsigned long) pthread_self();
#endif
}


int trap_printf(const char *format, ...) DYNINST_PRINTF_ANNOTATION(1, 2);
int signal_printf_int(const char *format, ...) DYNINST_PRINTF_ANNOTATION(1, 2);
int inferiorrpc_printf_int(const char *format, ...) DYNINST_PRINTF_ANNOTATION(1, 2);
int startup_printf_int(const char *format, ...) DYNINST_PRINTF_ANNOTATION(1, 2);
int bpatch_printf(const char *format, ...) DYNINST_PRINTF_ANNOTATION(1, 2);

// Make a lock.

Mutex<> *debugPrintLock = NULL;

void BPatch_reportError(int errLevel, int num, const char *str) {
    BPatch::reportError((BPatchErrorLevel) errLevel, num, str);
}

void showInfoCallback(std::string msg)
{
    BPatch::reportError(BPatchWarning, 0, msg.c_str());
}

char errorLine[1024];

void showErrorCallback(int num, std::string msg)
{
    BPatch::reportError(BPatchSerious, num, msg.c_str());
}

void logLine(const char *line)
{
    BPatch::reportError(BPatchWarning, 0, line);
}

// The unused parameter is used by Paradyn's version of this function.
void statusLine(const char *line, bool /* unused */ )
{
    BPatch::reportError(BPatchInfo, 0, line);
}

//  bpfatal, bpsevere, bpwarn, and bpinfo are intended as drop-in
//  replacements for printf.
#define ERR_BUF_SIZE 2048 // egad -- 1024 is not large enough

int bpfatal(const char *format, ...)
{
  if (NULL == format) return -1;

  char errbuf[ERR_BUF_SIZE];

  va_list va;
  va_start(va, format);
#if defined (i386_unknown_nt4_0)
  _vsnprintf(errbuf, ERR_BUF_SIZE,format, va);
#else
  vsnprintf(errbuf, ERR_BUF_SIZE,format, va);
#endif
  va_end(va);

  BPatch::reportError(BPatchFatal, 0, errbuf);

  return 0;
}


int bpfatal_lf(const char *__file__, unsigned int __line__, const char *format, ...)
{
  fprintf(stderr, "%s[%d]\n", __FILE__, __LINE__);
  if (NULL == format) return -1;

  char errbuf[ERR_BUF_SIZE];

  fprintf(stderr, "%s[%d]\n", __FILE__, __LINE__);
  int header_len = sprintf(errbuf, "[%lu]%s[%u]: ", getExecThreadID(), __file__, __line__);

  fprintf(stderr, "%s[%d]\n", __FILE__, __LINE__);
  va_list va;
  va_start(va, format);
  VSNPRINTF(errbuf + header_len, ERR_BUF_SIZE - header_len, format, va);
  va_end(va);

  fprintf(stderr, "%s[%d]\n", __FILE__, __LINE__);
  BPatch::reportError(BPatchFatal, 0, errbuf);

  fprintf(stderr, "%s[%d]\n", __FILE__, __LINE__);
  return 0;
}

int bperr(const char *format, ...)
{
  if (NULL == format) return -1;

  char errbuf[ERR_BUF_SIZE];

  va_list va;
  va_start(va, format);
#if defined (i386_unknown_nt4_0)
  int errbuflen = _vsnprintf(errbuf, ERR_BUF_SIZE, format, va);
#else
  int errbuflen = vsnprintf(errbuf, ERR_BUF_SIZE, format, va);
#endif
  va_end(va);

  char syserr[128];
  if (errno) {
    int syserrlen = snprintf(syserr, 128," [%d: %s]", errno, strerror(errno));
    /* reset errno so that future calls to this function don't report same error */
    errno = 0;

    if ((errbuflen + syserrlen) < ERR_BUF_SIZE)
      strcat(errbuf, syserr);
    else {
      BPatch::reportError(BPatchSerious, 0, errbuf);
      BPatch::reportError(BPatchSerious, 0, syserr);
      return 0;
    }
  }
  BPatch::reportError(BPatchSerious, 0, errbuf);

  return 0;
}

int bpwarn(const char *format, ...)
{
  if (NULL == format) return -1;

  char errbuf[ERR_BUF_SIZE];

  va_list va;
  va_start(va, format);
#if defined (i386_unknown_nt4_0)
  _vsnprintf(errbuf, ERR_BUF_SIZE,format, va);
#else
  vsnprintf(errbuf, ERR_BUF_SIZE,format, va);
#endif
  va_end(va);

  BPatch::reportError(BPatchWarning, 0, errbuf);

  return 0;
}

int bpinfo(const char *format, ...)
{
  if (NULL == format) return -1;

  char errbuf[ERR_BUF_SIZE];

  va_list va;
  va_start(va, format);
#if defined (i386_unknown_nt4_0)
  _vsnprintf(errbuf, ERR_BUF_SIZE, format, va);
#else
  vsnprintf(errbuf, ERR_BUF_SIZE, format, va);
#endif
  va_end(va);

  BPatch::reportError(BPatchInfo, 0, errbuf);

  return 0;
}


// Internal debugging

int dyn_debug_malware = 0;
int dyn_debug_trap = 0;
int dyn_debug_signal = 0;
int dyn_debug_infrpc = 0;
int dyn_debug_startup = 0;
int dyn_debug_parsing = 0;
int dyn_debug_proccontrol = 0;
int dyn_debug_stackwalk = 0;
int dyn_debug_inst = 0;
int dyn_debug_reloc = 0;
int dyn_debug_springboard = 0;
int dyn_debug_sensitivity = 0;
int dyn_debug_dyn_unw = 0;
int dyn_debug_mutex = 0;
int dyn_debug_dwarf = 0;
int dyn_debug_rtlib = 0;
int dyn_debug_catchup = 0;
int dyn_debug_bpatch = 0;
int dyn_debug_regalloc = 0;
int dyn_debug_ast = 0;
int dyn_debug_write = 0;
int dyn_debug_infmalloc = 0;
int dyn_debug_crash = 0;
int dyn_debug_stackmods = 0;
char *dyn_debug_crash_debugger = NULL;
int dyn_debug_disassemble = 0;

static char *dyn_debug_write_filename = NULL;
static FILE *dyn_debug_write_file = NULL;

bool check_env_value(const char* env_var)
{
	char* val = getenv(env_var);
	if(!val) return false;
	return (bool)(atoi(val));
}

bool init_debug() {
  static bool init = false;
  if (init) return true;
  init = true;

  char *p;
  if (check_env_value("DYNINST_DEBUG_MALWARE")) {
    fprintf(stderr, "Enabling DyninstAPI malware debug\n");
    dyn_debug_malware = 1;
  }
  if (check_env_value("DYNINST_DEBUG_TRAP")) {
    fprintf(stderr, "Enabling DyninstAPI debugging using traps\n");
    dyn_debug_trap = 1;
  }
  if (check_env_value("DYNINST_DEBUG_SPRINGBOARD")) {
    fprintf(stderr, "Enabling DyninstAPI springboard debug\n");
    dyn_debug_springboard = 1;
  }
  if (check_env_value("DYNINST_DEBUG_STARTUP")) {
    fprintf(stderr, "Enabling DyninstAPI startup debug\n");
    dyn_debug_startup = 1;
  }
  if ( (check_env_value("DYNINST_DEBUG_PARSING"))) {
     fprintf(stderr, "Enabling DyninstAPI parsing debug\n");
     dyn_debug_parsing = 1;
  }
  if ( (check_env_value("DYNINST_DEBUG_PARSE"))) {
     fprintf(stderr, "Enabling DyninstAPI parsing debug\n");
     dyn_debug_parsing = 1;
  }
  if (    (check_env_value("DYNINST_DEBUG_DYNPC"))
       || (check_env_value("DYNINST_DEBUG_FORKEXEC"))
       || (check_env_value("DYNINST_DEBUG_INFRPC"))
       || (check_env_value("DYNINST_DEBUG_SIGNAL"))
       || (check_env_value("DYNINST_DEBUG_INFERIORRPC"))
       || (check_env_value("DYNINST_DEBUG_THREAD"))
       || (check_env_value("DYNINST_DEBUG_MAILBOX"))
       || (check_env_value("DYNINST_DEBUG_DBI"))
     )
  {
    fprintf(stderr, "Enabling DyninstAPI process control debug\n");
    dyn_debug_proccontrol = 1;
  }

  if ( (check_env_value("DYNINST_DEBUG_STACKWALK"))) {
    fprintf(stderr, "Enabling DyninstAPI stack walking debug\n");
    dyn_debug_stackwalk = 1;
  }
  if ( (check_env_value("DYNINST_DEBUG_INST"))) {
    fprintf(stderr, "Enabling DyninstAPI inst debug\n");
    dyn_debug_inst = 1;
  }
  if ( (check_env_value("DYNINST_DEBUG_RELOC"))) {
    fprintf(stderr, "Enabling DyninstAPI relocation debug\n");
    dyn_debug_reloc = 1;
  }
  if ( (check_env_value("DYNINST_DEBUG_RELOCATION"))) {
    fprintf(stderr, "Enabling DyninstAPI relocation debug\n");
    dyn_debug_reloc = 1;
  }
  if ( (check_env_value("DYNINST_DEBUG_SENSITIVITY"))) {
    fprintf(stderr, "Enabling DyninstAPI sensitivity debug\n");
    dyn_debug_sensitivity = 1;
  }
  if ( (check_env_value("DYNINST_DEBUG_DYN_UNW"))) {
    fprintf(stderr, "Enabling DyninstAPI dynamic unwind debug\n");
    dyn_debug_dyn_unw = 1;
    }
  if ( (check_env_value("DYNINST_DEBUG_MUTEX"))) {
    fprintf(stderr, "Enabling DyninstAPI mutex debug\n");
    dyn_debug_mutex = 1;
    }
  if ( (check_env_value("DYNINST_DEBUG_RTLIB"))) {
      fprintf(stderr, "Enabling DyninstAPI RTlib debug\n");
      dyn_debug_rtlib = 1;
  }
  if ( (check_env_value("DYNINST_DEBUG_CATCHUP"))) {
      fprintf(stderr, "Enabling DyninstAPI catchup debug\n");
      dyn_debug_catchup = 1;
  }
  if ( (check_env_value("DYNINST_DEBUG_BPATCH"))) {
      fprintf(stderr, "Enabling DyninstAPI bpatch debug\n");
      dyn_debug_bpatch = 1;
  }
  if ( (check_env_value("DYNINST_DEBUG_REGALLOC"))) {
      fprintf(stderr, "Enabling DyninstAPI register allocation debug\n");
      dyn_debug_regalloc = 1;
  }
  if ( (check_env_value("DYNINST_DEBUG_AST"))) {
      fprintf(stderr, "Enabling DyninstAPI ast debug\n");
      dyn_debug_ast = 1;
  }
  if ( (p=getenv("DYNINST_DEBUG_WRITE"))) {
    fprintf(stderr, "Enabling DyninstAPI process write debugging\n");
    dyn_debug_write = 1;
    dyn_debug_write_filename = p;
  }
  if ( (check_env_value("DYNINST_DEBUG_INFMALLOC")) ||
       (check_env_value("DYNINST_DEBUG_INFERIORMALLOC"))) {
    fprintf(stderr, "Enabling DyninstAPI inferior malloc debugging\n");
    dyn_debug_infmalloc = 1;
  }
  if ((p=getenv("DYNINST_DEBUG_CRASH"))) {
     fprintf(stderr, "Enable DyninstAPI crash debugging\n");
     dyn_debug_crash = 1;
     dyn_debug_crash_debugger = p;
  }
  if ((p=getenv("DYNINST_DEBUG_STACKMODS"))) {
     fprintf(stderr, "Enable DyninstAPI stackmods debugging\n");
     dyn_debug_stackmods = 1;
  }
  if (check_env_value("DYNINST_DEBUG_DISASS")) {
      fprintf(stderr, "Enabling DyninstAPI instrumentation disassembly debugging\n");
      dyn_debug_disassemble = 1;
  }
  debugPrintLock = new Mutex<>();

  return true;
}

int mal_printf(const char *format, ...)
{
  if (!dyn_debug_malware) return 0;
  if (NULL == format) return -1;

  debugPrintLock->lock();
  va_list va;
  va_start(va, format);
  int ret = vfprintf(stderr, format, va);
  va_end(va);

  debugPrintLock->unlock();

  return ret;
}

int trap_printf(const char *format, ...)
{
  if (!dyn_debug_trap) return 0;
  if (NULL == format) return -1;

  debugPrintLock->lock();

  va_list va;
  va_start(va, format);
  int ret = vfprintf(stderr, format, va);
  va_end(va);

  debugPrintLock->unlock();

  return ret;
}

int signal_printf_int(const char *format, ...)
{
  if (!dyn_debug_signal) return 0;
  if (NULL == format) return -1;

  debugPrintLock->lock();

  fprintf(stderr, "[%lu]", getExecThreadID());
  va_list va;
  va_start(va, format);
  int ret = vfprintf(stderr, format, va);
  va_end(va);

  debugPrintLock->unlock();

  return ret;
}

int inferiorrpc_printf_int(const char *format, ...)
{
  if (!dyn_debug_infrpc) return 0;
  if (NULL == format) return -1;

  debugPrintLock->lock();

  fprintf(stderr, "[%lu]", getExecThreadID());
  va_list va;
  va_start(va, format);
  int ret = vfprintf(stderr, format, va);
  va_end(va);

  debugPrintLock->unlock();

  return ret;
}

int startup_printf_int(const char *format, ...)
{
  if (!dyn_debug_startup) return 0;
  if (NULL == format) return -1;

  debugPrintLock->lock();

  fprintf(stderr, "[%lu]", getExecThreadID());
  va_list va;
  va_start(va, format);
  int ret = vfprintf(stderr, format, va);
  va_end(va);

  debugPrintLock->unlock();

  return ret;
}

int parsing_printf_int(const char *format, ...)
{
  if (!dyn_debug_parsing) return 0;
  if (NULL == format) return -1;

  debugPrintLock->lock();

  //fprintf(stderr, "[%ld]", getExecThreadID());
  va_list va;
  va_start(va, format);

  int ret = vfprintf(stderr, format, va);

  va_end(va);

  debugPrintLock->unlock();

  return ret;
}

int proccontrol_printf_int(const char *format, ...)
{
  if (!dyn_debug_proccontrol) return 0;
  if (NULL == format) return -1;

  debugPrintLock->lock();

  fprintf(stderr, "[%lu]", getExecThreadID());
  va_list va;
  va_start(va, format);
  int ret = vfprintf(stderr, format, va);
  va_end(va);

  debugPrintLock->unlock();

  return ret;
}

int stackwalk_printf_int(const char *format, ...)
{
  if (!dyn_debug_stackwalk) return 0;
  if (NULL == format) return -1;

  debugPrintLock->lock();

  fprintf(stderr, "[%lu]", getExecThreadID());
  va_list va;
  va_start(va, format);
  int ret = vfprintf(stderr, format, va);
  va_end(va);

  debugPrintLock->unlock();

  return ret;
}

int inst_printf_int(const char *format, ...)
{
  if (!dyn_debug_inst) return 0;
  if (NULL == format) return -1;

  debugPrintLock->lock();

  fprintf(stderr, "[%lu]", getExecThreadID());
  va_list va;
  va_start(va, format);
  int ret = vfprintf(stderr, format, va);
  va_end(va);

  debugPrintLock->unlock();

  return ret;
}

int reloc_printf_int(const char *format, ...)
{
  if (!dyn_debug_reloc) return 0;
  if (NULL == format) return -1;

  debugPrintLock->lock();

  fprintf(stderr, "[%lu]", getExecThreadID());
  va_list va;
  va_start(va, format);
  int ret = vfprintf(stderr, format, va);
  va_end(va);

  debugPrintLock->unlock();

  return ret;
}

int dyn_unw_printf_int(const char *format, ...)
{
  if (!dyn_debug_dyn_unw ) return 0;
  if (NULL == format) return -1;

  debugPrintLock->lock();

  fprintf(stderr, "[%lu]", getExecThreadID());
  va_list va;
  va_start(va, format);
  int ret = vfprintf(stderr, format, va);
  va_end(va);

  debugPrintLock->unlock();

  return ret;
}

int mutex_printf_int(const char *format, ...)
{
  if (!dyn_debug_mutex ) return 0;
  if (NULL == format) return -1;

  debugPrintLock->lock();

  fprintf(stderr, "[%lu]", getExecThreadID());
  va_list va;
  va_start(va, format);
  int ret = vfprintf(stderr, format, va);
  va_end(va);

  debugPrintLock->unlock();

  return ret;
}

int catchup_printf_int(const char *format, ...)
{

    if (!dyn_debug_catchup) return 0;
    if (NULL == format) return -1;

    debugPrintLock->lock();

    fprintf(stderr, "[%lu]", getExecThreadID());
    va_list va;
    va_start(va, format);
    int ret = vfprintf(stderr, format, va);
    va_end(va);

    debugPrintLock->unlock();

    return ret;
}

int bpatch_printf(const char *format, ...)
{
  if (!dyn_debug_bpatch) return 0;
  if (NULL == format) return -1;

  debugPrintLock->lock();

  fprintf(stderr, "[%lu]", getExecThreadID());
  va_list va;
  va_start(va, format);
  int ret = vfprintf(stderr, format, va);
  va_end(va);

  debugPrintLock->unlock();

  return ret;
}

int regalloc_printf_int(const char *format, ...)
{
  if (!dyn_debug_regalloc) return 0;
  if (NULL == format) return -1;

  debugPrintLock->lock();

  fprintf(stderr, "[%lu]", getExecThreadID());
  va_list va;
  va_start(va, format);
  int ret = vfprintf(stderr, format, va);
  va_end(va);

  debugPrintLock->unlock();

  return ret;
}

int ast_printf_int(const char *format, ...)
{
  if (!dyn_debug_ast) return 0;
  if (NULL == format) return -1;

  debugPrintLock->lock();

  fprintf(stderr, "[%lu]", getExecThreadID());
  va_list va;
  va_start(va, format);
  int ret = vfprintf(stderr, format, va);
  va_end(va);

  debugPrintLock->unlock();

  return ret;
}

int write_printf_int(const char *format, ...)
{
  if (!dyn_debug_write) return 0;
  if (NULL == format) return -1;

  debugPrintLock->lock();

  if (!dyn_debug_write_file) {
    if (dyn_debug_write_filename && strlen(dyn_debug_write_filename)) {
      dyn_debug_write_file = fopen(dyn_debug_write_filename, "w");
    }
    if (!dyn_debug_write_file) {
      dyn_debug_write_file = stderr;
    }
  }

  va_list va;
  va_start(va, format);
  int ret = vfprintf(dyn_debug_write_file, format, va);
  va_end(va);
  fflush(dyn_debug_write_file);

  debugPrintLock->unlock();

  return ret;
}


int infmalloc_printf_int(const char *format, ...)
{
  if (!dyn_debug_infmalloc) return 0;
  if (NULL == format) return -1;

  debugPrintLock->lock();

  fprintf(stderr, "[%lu]", getExecThreadID());
  va_list va;
  va_start(va, format);
  int ret = vfprintf(stderr, format, va);
  va_end(va);

  debugPrintLock->unlock();

  return ret;
}

int stackmods_printf_int(const char *format, ...)
{
  if (!dyn_debug_stackmods) return 0;
  if (NULL == format) return -1;

  debugPrintLock->lock();

  va_list va;
  va_start(va, format);
  int ret = vfprintf(stderr, format, va);
  va_end(va);

  debugPrintLock->unlock();

  return ret;
}
