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

// $Id: api_showerror.C,v 1.12 2004/04/02 21:56:47 jaw Exp $

#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
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

void statusLine(char const *line)
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
  vsnprintf(errbuf, ERR_BUF_SIZE,format, va);
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
  int errbuflen = vsnprintf(errbuf, ERR_BUF_SIZE, format, va);
  va_end(va);

  char syserr[128];
  if (errno) {
    int syserrlen = snprintf(syserr, 128," [%d: %s]", errno, strerror(errno));
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
  vsnprintf(errbuf, ERR_BUF_SIZE,format, va);
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
  vsnprintf(errbuf, ERR_BUF_SIZE, format, va);
  va_end(va);

  BPatch::reportError(BPatchInfo, 0, errbuf);

  return 0;
}

