/*
 * Copyright (c) 1996 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
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

// $Id: api_showerror.C,v 1.9 2003/09/05 16:27:37 schendel Exp $

#include <stdio.h>
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
