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

/*
 * dyninst.h - exported interface to instrumentation.
 *
 * $Log: dyninst.h,v $
 * Revision 1.13  1996/08/16 21:18:28  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.12  1994/11/02 11:03:46  markc
 * Removed stringPool
 *
 * Revision 1.11  1994/09/22  01:50:54  markc
 * reorganized, temporary
 *
 * Revision 1.10  1994/08/08  20:13:34  hollings
 * Added suppress instrumentation command.
 *
 * Revision 1.9  1994/06/27  21:28:08  rbi
 * Abstraction-specific resources and mapping info
 *
 * Revision 1.8  1994/05/18  00:52:26  hollings
 * added ability to gather IO from application processes and forward it to
 * the paradyn proces.
 *
 * Revision 1.7  1994/05/16  22:31:49  hollings
 * added way to request unique resource name.
 *
 * Revision 1.6  1994/04/09  18:34:52  hollings
 * Changed {pause,continue}Application to {pause,continue}AllProceses, and
 * made the RPC interfaces use these.  This makes the computation of pause
 * Time correct.
 *
 */

#ifndef INSTRUMENTATION_H
#define INSTRUMENTATION_H

#include "rtinst/h/trace.h"
#include "util/h/stringDecl.h"

/* time */
typedef double timeStamp;		

#endif
