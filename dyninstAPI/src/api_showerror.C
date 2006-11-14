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

// $Id: api_showerror.C,v 1.31 2006/11/14 20:36:57 bernat Exp $

#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include "common/h/Pair.h"
#include "common/h/Vector.h"
#include "util.h"
#include "BPatch.h"
#include "dyninstAPI/src/showerror.h"
#include "EventHandler.h"

// Make a lock.

eventLock *debugPrintLock = NULL;

void showInfoCallback(pdstring msg)
{
    BPatch::reportError(BPatchWarning, 0, msg.c_str());
}

char errorLine[1024];

void showErrorCallback(int num, pdstring msg)
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
  int header_len = sprintf(errbuf, "[%s]%s[%d]: ", getThreadStr(getExecThreadID()), __file__, __line__);

  fprintf(stderr, "%s[%d]\n", __FILE__, __LINE__);
  va_list va;
  va_start(va, format);
  VSNPRINTF(errbuf + header_len, ERR_BUF_SIZE,format, va);
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
#if defined (i386_unknown_nt4_0)
    int syserrlen = _snprintf(syserr, 128," [%d: %s]", errno, strerror(errno));
#else
    int syserrlen = snprintf(syserr, 128," [%d: %s]", errno, strerror(errno));
#endif
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

int dyn_debug_signal = 0;
int dyn_debug_infrpc = 0;
int dyn_debug_startup = 0;
int dyn_debug_parsing = 0;
int dyn_debug_forkexec = 0;
int dyn_debug_proccontrol = 0;
int dyn_debug_stackwalk = 0;
int dyn_debug_dbi = 0;
int dyn_debug_inst = 0;
int dyn_debug_reloc = 0;
int dyn_debug_dyn_unw = 0;
int dyn_debug_dyn_dbi = 0;
int dyn_debug_mutex = 0;
int dyn_debug_mailbox = 0;
int dyn_debug_async = 0;
int dyn_debug_dwarf = 0;
int dyn_debug_thread = 0;
int dyn_debug_rtlib = 0;
int dyn_debug_catchup = 0;
int dyn_debug_bpatch = 0;
int dyn_debug_regalloc = 0;
int dyn_debug_ast = 0;

bool init_debug() {
  char *p;
  if ( (p=getenv("DYNINST_DEBUG_SIGNAL"))) {
    fprintf(stderr, "Enabling DyninstAPI signal debug\n");
    dyn_debug_signal = 1;
  }
  if ( (p=getenv("DYNINST_DEBUG_INFRPC"))) {
    fprintf(stderr, "Enabling DyninstAPI inferior RPC debug\n");
    dyn_debug_infrpc = 1;
  }
  if ( (p=getenv("DYNINST_DEBUG_INFERIORRPC"))) {
    fprintf(stderr, "Enabling DyninstAPI inferior RPC debug\n");
    dyn_debug_infrpc = 1;
  }
  if ( (p=getenv("DYNINST_DEBUG_STARTUP"))) {
    fprintf(stderr, "Enabling DyninstAPI startup debug\n");
    dyn_debug_startup = 1;
  }
  if ( (p=getenv("DYNINST_DEBUG_PARSING"))) {
    fprintf(stderr, "Enabling DyninstAPI parsing debug\n");
    dyn_debug_parsing = 1;
  }
  if ( (p=getenv("DYNINST_DEBUG_PARSE"))) {
    fprintf(stderr, "Enabling DyninstAPI parsing debug\n");
    dyn_debug_parsing = 1;
  }
  if ( (p=getenv("DYNINST_DEBUG_FORKEXEC"))) {
    fprintf(stderr, "Enabling DyninstAPI forkexec debug\n");
    dyn_debug_forkexec = 1;
  }
  if ( (p=getenv("DYNINST_DEBUG_PROCCONTROL"))) {
    fprintf(stderr, "Enabling DyninstAPI process control debug\n");
    dyn_debug_proccontrol = 1;
  }
  if ( (p=getenv("DYNINST_DEBUG_STACKWALK"))) {
    fprintf(stderr, "Enabling DyninstAPI stack walking debug\n");
    dyn_debug_stackwalk = 1;
  }
  if ( (p=getenv("DYNINST_DEBUG_INST"))) {
    fprintf(stderr, "Enabling DyninstAPI inst debug\n");
    dyn_debug_inst = 1;
  }
  if ( (p=getenv("DYNINST_DEBUG_RELOC"))) {
    fprintf(stderr, "Enabling DyninstAPI relocation debug\n");
    dyn_debug_reloc = 1;
  }
  if ( (p=getenv("DYNINST_DEBUG_RELOCATION"))) {
    fprintf(stderr, "Enabling DyninstAPI relocation debug\n");
    dyn_debug_reloc = 1;
  }
  if ( (p=getenv("DYNINST_DEBUG_DYN_UNW"))) {
    fprintf(stderr, "Enabling DyninstAPI dynamic unwind debug\n");
    dyn_debug_dyn_unw = 1;
    }
  if ( (p=getenv("DYNINST_DEBUG_DBI"))) {
    fprintf(stderr, "Enabling DyninstAPI debugger interface debug\n");
    dyn_debug_dyn_dbi = 1;
    }
  if ( (p=getenv("DYNINST_DEBUG_MUTEX"))) {
    fprintf(stderr, "Enabling DyninstAPI mutex debug\n");
    dyn_debug_mutex = 1;
    }
  if ( (p=getenv("DYNINST_DEBUG_MAILBOX"))) {
    fprintf(stderr, "Enabling DyninstAPI callbacks debug\n");
    dyn_debug_mailbox= 1;
    }
  if ( (p=getenv("DYNINST_DEBUG_DWARF"))) {
    fprintf(stderr, "Enabling DyninstAPI dwarf debug\n");
    dyn_debug_dwarf= 1;
    }
  if ( (p=getenv("DYNINST_DEBUG_ASYNC"))) {
    fprintf(stderr, "Enabling DyninstAPI async debug\n");
    dyn_debug_async= 1;
    }
  if ( (p=getenv("DYNINST_DEBUG_THREAD"))) {
      fprintf(stderr, "Enabling DyninstAPI thread debug\n");
      dyn_debug_thread = 1;
  }
  if ( (p=getenv("DYNINST_DEBUG_RTLIB"))) {
      fprintf(stderr, "Enabling DyninstAPI RTlib debug\n");
      dyn_debug_rtlib = 1;
  }
  if ( (p=getenv("DYNINST_DEBUG_CATCHUP"))) {
      fprintf(stderr, "Enabling DyninstAPI catchup debug\n");
      dyn_debug_catchup = 1;
  }
  if ( (p=getenv("DYNINST_DEBUG_BPATCH"))) {
      fprintf(stderr, "Enabling DyninstAPI bpatch debug\n");
      dyn_debug_bpatch = 1;
  }
  if ( (p=getenv("DYNINST_DEBUG_REGALLOC"))) {
      fprintf(stderr, "Enabling DyninstAPI register allocation debug\n");
      dyn_debug_regalloc = 1;
  }
  if ( (p=getenv("DYNINST_DEBUG_AST"))) {
      fprintf(stderr, "Enabling DyninstAPI ast debug\n");
      dyn_debug_ast = 1;
  }

  debugPrintLock = new eventLock();

  return true;
}

int signal_printf(const char *format, ...)
{
  if (!dyn_debug_signal) return 0;
  if (NULL == format) return -1;

  debugPrintLock->_Lock(FILE__, __LINE__);

  fprintf(stderr, "[%s]", getThreadStr(getExecThreadID()));
  va_list va;
  va_start(va, format);
  int ret = vfprintf(stderr, format, va);
  va_end(va);

  debugPrintLock->_Unlock(FILE__, __LINE__);

  return ret;
}

int inferiorrpc_printf(const char *format, ...)
{
  if (!dyn_debug_infrpc) return 0;
  if (NULL == format) return -1;

  debugPrintLock->_Lock(FILE__, __LINE__);

  fprintf(stderr, "[%s]", getThreadStr(getExecThreadID()));
  va_list va;
  va_start(va, format);
  int ret = vfprintf(stderr, format, va);
  va_end(va);

  debugPrintLock->_Unlock(FILE__, __LINE__);

  return ret;
}

int startup_printf(const char *format, ...)
{
  if (!dyn_debug_startup) return 0;
  if (NULL == format) return -1;

  debugPrintLock->_Lock(FILE__, __LINE__);

  fprintf(stderr, "[%s]", getThreadStr(getExecThreadID()));
  va_list va;
  va_start(va, format);
  int ret = vfprintf(stderr, format, va);
  va_end(va);

  debugPrintLock->_Unlock(FILE__, __LINE__);

  return ret;
}

int parsing_printf(const char *format, ...)
{
  if (!dyn_debug_parsing) return 0;
  if (NULL == format) return -1;

  debugPrintLock->_Lock(FILE__, __LINE__);

  fprintf(stderr, "[%s]", getThreadStr(getExecThreadID()));
  va_list va;
  va_start(va, format);

  int ret = vfprintf(stderr, format, va);

  va_end(va);

  debugPrintLock->_Unlock(FILE__, __LINE__);

  return ret;
}

int forkexec_printf(const char *format, ...)
{
  if (!dyn_debug_forkexec) return 0;
  if (NULL == format) return -1;

  debugPrintLock->_Lock(FILE__, __LINE__);

  fprintf(stderr, "[%s]", getThreadStr(getExecThreadID()));
  va_list va;
  va_start(va, format);
  int ret = vfprintf(stderr, format, va);
  va_end(va);

  debugPrintLock->_Unlock(FILE__, __LINE__);

  return ret;
}

int proccontrol_printf(const char *format, ...)
{
  if (!dyn_debug_proccontrol) return 0;
  if (NULL == format) return -1;

  debugPrintLock->_Lock(FILE__, __LINE__);

  fprintf(stderr, "[%s]", getThreadStr(getExecThreadID()));
  va_list va;
  va_start(va, format);
  int ret = vfprintf(stderr, format, va);
  va_end(va);

  debugPrintLock->_Unlock(FILE__, __LINE__);

  return ret;
}

int stackwalk_printf(const char *format, ...)
{
  if (!dyn_debug_stackwalk) return 0;
  if (NULL == format) return -1;

  debugPrintLock->_Lock(FILE__, __LINE__);

  fprintf(stderr, "[%s]", getThreadStr(getExecThreadID()));
  va_list va;
  va_start(va, format);
  int ret = vfprintf(stderr, format, va);
  va_end(va);

  debugPrintLock->_Unlock(FILE__, __LINE__);

  return ret;
}



int inst_printf(const char *format, ...)
{
  if (!dyn_debug_inst) return 0;
  if (NULL == format) return -1;

  debugPrintLock->_Lock(FILE__, __LINE__);

  fprintf(stderr, "[%s]", getThreadStr(getExecThreadID()));
  va_list va;
  va_start(va, format);
  int ret = vfprintf(stderr, format, va);
  va_end(va);

  debugPrintLock->_Unlock(FILE__, __LINE__);

  return ret;
}

int reloc_printf(const char *format, ...)
{
  if (!dyn_debug_reloc) return 0;
  if (NULL == format) return -1;

  debugPrintLock->_Lock(FILE__, __LINE__);
  
  fprintf(stderr, "[%s]", getThreadStr(getExecThreadID()));
  va_list va;
  va_start(va, format);
  int ret = vfprintf(stderr, format, va);
  va_end(va);

  debugPrintLock->_Unlock(FILE__, __LINE__);

  return ret;
}

int dyn_unw_printf(const char *format, ...)
{
  if (!dyn_debug_dyn_unw ) return 0;
  if (NULL == format) return -1;

  debugPrintLock->_Lock(FILE__, __LINE__);
  
  fprintf(stderr, "[%s]", getThreadStr(getExecThreadID()));
  va_list va;
  va_start(va, format);
  int ret = vfprintf(stderr, format, va);
  va_end(va);

  debugPrintLock->_Unlock(FILE__, __LINE__);

  return ret;
}

int dbi_printf(const char *format, ...)
{
  if (!dyn_debug_dyn_dbi ) return 0;
  if (NULL == format) return -1;

  debugPrintLock->_Lock(FILE__, __LINE__);
  
  fprintf(stderr, "[%s]", getThreadStr(getExecThreadID()));
  va_list va;
  va_start(va, format);
  int ret = vfprintf(stderr, format, va);
  va_end(va);

  debugPrintLock->_Unlock(FILE__, __LINE__);

  return ret;
}

int mutex_printf(const char *format, ...)
{
  if (!dyn_debug_mutex ) return 0;
  if (NULL == format) return -1;

  debugPrintLock->_Lock(FILE__, __LINE__);
  
  fprintf(stderr, "[%s]", getThreadStr(getExecThreadID()));
  va_list va;
  va_start(va, format);
  int ret = vfprintf(stderr, format, va);
  va_end(va);

  debugPrintLock->_Unlock(FILE__, __LINE__);

  return ret;
}

int mailbox_printf(const char *format, ...)
{
  if (!dyn_debug_mailbox ) return 0;
  if (NULL == format) return -1;

  debugPrintLock->_Lock(FILE__, __LINE__);
  
  fprintf(stderr, "[%s]", getThreadStr(getExecThreadID()));
  va_list va;
  va_start(va, format);
  int ret = vfprintf(stderr, format, va);
  va_end(va);

  debugPrintLock->_Unlock(FILE__, __LINE__);

  return ret;
}

int async_printf(const char *format, ...)
{
  if (!dyn_debug_async ) return 0;
  if (NULL == format) return -1;

  debugPrintLock->_Lock(FILE__, __LINE__);
  
  fprintf(stderr, "[%s]", getThreadStr(getExecThreadID()));
  va_list va;
  va_start(va, format);
  int ret = vfprintf(stderr, format, va);
  va_end(va);

  debugPrintLock->_Unlock(FILE__, __LINE__);

  return ret;
}

int dwarf_printf(const char *format, ...)
{
  if (!dyn_debug_dwarf ) return 0;
  if (NULL == format) return -1;

  debugPrintLock->_Lock(FILE__, __LINE__);
  
  fprintf(stderr, "(dwarf) [thread %s]: ", getThreadStr(getExecThreadID()));
  va_list va;
  va_start(va, format);
  int ret = vfprintf(stderr, format, va);
  va_end(va);

  debugPrintLock->_Unlock(FILE__, __LINE__);

  return ret;
}


int thread_printf(const char *format, ...)
{
  if (!dyn_debug_thread) return 0;
  if (NULL == format) return -1;

  debugPrintLock->_Lock(FILE__, __LINE__);
  
  fprintf(stderr, "[%s]: ", getThreadStr(getExecThreadID()));
  va_list va;
  va_start(va, format);
  int ret = vfprintf(stderr, format, va);
  va_end(va);

  debugPrintLock->_Unlock(FILE__, __LINE__);

  return ret;
}

int catchup_printf(const char *format, ...)
{
  if (!dyn_debug_catchup) return 0;
  if (NULL == format) return -1;

  debugPrintLock->_Lock(FILE__, __LINE__);
  
  fprintf(stderr, "[%s]: ", getThreadStr(getExecThreadID()));
  va_list va;
  va_start(va, format);
  int ret = vfprintf(stderr, format, va);
  va_end(va);

  debugPrintLock->_Unlock(FILE__, __LINE__);

  return ret;
}

int bpatch_printf(const char *format, ...)
{
  if (!dyn_debug_bpatch) return 0;
  if (NULL == format) return -1;

  debugPrintLock->_Lock(FILE__, __LINE__);
  
  fprintf(stderr, "[%s]: ", getThreadStr(getExecThreadID()));
  va_list va;
  va_start(va, format);
  int ret = vfprintf(stderr, format, va);
  va_end(va);

  debugPrintLock->_Unlock(FILE__, __LINE__);

  return ret;
}

int regalloc_printf(const char *format, ...)
{
  if (!dyn_debug_regalloc) return 0;
  if (NULL == format) return -1;

  debugPrintLock->_Lock(FILE__, __LINE__);
  
  fprintf(stderr, "[%s]: ", getThreadStr(getExecThreadID()));
  va_list va;
  va_start(va, format);
  int ret = vfprintf(stderr, format, va);
  va_end(va);

  debugPrintLock->_Unlock(FILE__, __LINE__);

  return ret;
}

int ast_printf(const char *format, ...)
{
  if (!dyn_debug_ast) return 0;
  if (NULL == format) return -1;

  debugPrintLock->_Lock(FILE__, __LINE__);
  
  fprintf(stderr, "[%s]: ", getThreadStr(getExecThreadID()));
  va_list va;
  va_start(va, format);
  int ret = vfprintf(stderr, format, va);
  va_end(va);

  debugPrintLock->_Unlock(FILE__, __LINE__);

  return ret;
}
