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

#ifndef DYNRPC_H
#define DYNRPC_h

/* 
 * $Log: dynrpc.h,v $
 * Revision 1.3  2001/06/20 20:39:54  schendel
 * Add selector and mutator methods to wrap the global current and internal
 *   metric sampling rate variables.  Use pdutil instead of pdutilOld.  Update
 *   to use new time and sample value types (ie. timeStamp, relTimeStamp,
 *   timeLength, pdSample, pdRate).
 *
 * Revision 1.2  1996/08/16 21:18:35  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.1  1994/11/01 16:58:00  markc
 * Prototypes
 *
 */

class timeLength;

const timeLength &getIMetricSamplingRate();
void setCurrSamplingRate(timeLength tl);
const timeLength &getCurrSamplingRate();

#endif
