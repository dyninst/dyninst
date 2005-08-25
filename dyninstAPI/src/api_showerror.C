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

// $Id: api_showerror.C,v 1.19 2005/08/25 22:45:08 bernat Exp $

#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include "common/h/Pair.h"
#include "common/h/Vector.h"
#include "util.h"
#include "BPatch.h"
#include "dyninstAPI/src/showerror.h"


void showInfoCallback(pdstring msg)
{
    BPatch::reportError(BPatchWarning, 0, msg.c_str());
}

#if defined(BPATCH_LIBRARY)
char errorLine[1024];

void showErrorCallback(int num, pdstring msg)
{
    BPatch::reportError(BPatchSerious, num, msg.c_str());
}

void logLine(char const *line)
{
    BPatch::reportError(BPatchWarning, 0, line);
}

// The unused parameter is used by Paradyn's version of this function.
void statusLine(char const *line, bool /* unused */ )
{
    BPatch::reportError(BPatchInfo, 0, line);
}

#endif
//  bpfatal, bpsevere, bpwarn, and bpinfo are intended as drop-in 
//  replacements for printf.
#define ERR_BUF_SIZE 2048 // egad -- 1024 is not large enough

int bpfatal(const char *format, ...)
{
  if (NULL == format) return -1;

  static char errbuf[ERR_BUF_SIZE];

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

int bperr(const char *format, ...)
{
  if (NULL == format) return -1;

  static char errbuf[ERR_BUF_SIZE];

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

  static char errbuf[ERR_BUF_SIZE];

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

  static char errbuf[ERR_BUF_SIZE];

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
int dyn_debug_inst = 0;
int dyn_debug_reloc = 0;

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
  if ( (p=getenv("DYNINST_DEBUG_STARTUP"))) {
    fprintf(stderr, "Enabling DyninstAPI startup debug\n");
    dyn_debug_startup = 1;
  }
  if ( (p=getenv("DYNINST_DEBUG_PARSING"))) {
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
  return true;
}

int signal_printf(const char *format, ...)
{
  if (!dyn_debug_signal) return 0;
  if (NULL == format) return -1;

  va_list va;
  va_start(va, format);
  int ret = vfprintf(stderr, format, va);
  va_end(va);

  return ret;
}

int inferiorrpc_printf(const char *format, ...)
{
  if (!dyn_debug_infrpc) return 0;
  if (NULL == format) return -1;

  va_list va;
  va_start(va, format);

  int ret = vfprintf(stderr, format, va);

  va_end(va);

  return ret;
}

int startup_printf(const char *format, ...)
{
  if (!dyn_debug_startup) return 0;
  if (NULL == format) return -1;

  va_list va;
  va_start(va, format);

  int ret = vfprintf(stderr, format, va);

  va_end(va);

  return ret;
}

int parsing_printf(const char *format, ...)
{
  if (!dyn_debug_parsing) return 0;
  if (NULL == format) return -1;

  va_list va;
  va_start(va, format);

  int ret = vfprintf(stderr, format, va);

  va_end(va);

  return ret;
}

int forkexec_printf(const char *format, ...)
{
  if (!dyn_debug_forkexec) return 0;
  if (NULL == format) return -1;

  va_list va;
  va_start(va, format);

  int ret = vfprintf(stderr, format, va);

  va_end(va);

  return ret;
}

int proccontrol_printf(const char *format, ...)
{
  if (!dyn_debug_proccontrol) return 0;
  if (NULL == format) return -1;

  va_list va;
  va_start(va, format);
  int ret = vfprintf(stderr, format, va);
  va_end(va);

  return ret;
}

int stackwalk_printf(const char *format, ...)
{
  if (!dyn_debug_stackwalk) return 0;
  if (NULL == format) return -1;

  va_list va;
  va_start(va, format);
  int ret = vfprintf(stderr, format, va);
  va_end(va);

  return ret;
}



int inst_printf(const char *format, ...)
{
  if (!dyn_debug_inst) return 0;
  if (NULL == format) return -1;

  va_list va;
  va_start(va, format);
  int ret = vfprintf(stderr, format, va);
  va_end(va);

  return ret;
}



int reloc_printf(const char *format, ...)
{
  if (!dyn_debug_reloc) return 0;
  if (NULL == format) return -1;
  
  va_list va;
  va_start(va, format);
  int ret = vfprintf(stderr, format, va);
  va_end(va);

  return ret;
}


