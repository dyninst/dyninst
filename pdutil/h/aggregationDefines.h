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

/*
 * $Log: aggregationDefines.h,v $
 * Revision 1.2  2004/03/23 01:12:40  eli
 * Updated copyright string
 *
 * Revision 1.1  2001/08/23 14:44:29  schendel
 * aggregation defines which can be included without importing full
 *   aggregation header file
 *
 * Revision 1.4  1996/08/16 21:30:23  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.3  1994/07/02 01:47:17  markc
 * Rename aggregation operator defines.
 *
 * Revision 1.2  1994/06/15  14:56:56  markc
 * Added log.
 *
 */

#ifndef AGGREGATION_DEFINES_H
#define AGGREGATION_DEFINES_H

// how are samples combined
// used in IGEN in an integer type
#define AGG_SUM 0
#define AGG_MIN 1
#define AGG_MAX 2

typedef enum { aggSum=AGG_SUM, aggMax=AGG_MAX, aggMin=AGG_MIN } aggregateOp;

#endif
