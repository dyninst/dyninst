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

// $Id: pdwinnt.h,v 1.12 2004/03/23 01:12:07 eli Exp $

#if !defined(PDWINNT_H)
#define PDWINNT_H

#if !defined(i386_unknown_nt4_0) && !(defined mips_unknown_ce2_11) //ccw 20 july 2000 : 29 mar 2001
#error "invalid architecture-os inclusion"
#endif

#ifndef PDWINNT_HDR
#define PDWINNT_HDR
#include "w32CONTEXT.h" //ccw 30 mar 2001

typedef HANDLE handleT;

struct dyn_saved_regs {
    w32CONTEXT cont;
};

#define EXIT_NAME "_exit"
#define SIGNAL_HANDLER 0
#endif

// Number of bytes to save in an overwrite operation
#define BYTES_TO_SAVE 256


#endif /* PDWINNT_H */
