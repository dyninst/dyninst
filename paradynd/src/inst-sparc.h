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

#if !defined(sparc_sun_sunos4_1_3) && !defined(sparc_sun_solaris2_4) && !defined(sparc_tmc_cmost7_3)
#error "invalid architecture-os inclusion"
#endif

#ifndef INST_SPARC_H
#define INST_SPARC_H

/*
 * $Log: inst-sparc.h,v $
 * Revision 1.6  1996/08/16 21:19:01  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.5  1995/07/11 20:57:30  jcargill
 * Changed sparc-specific ifdefs to include sparc_tmc_cmost7_3
 *
 * Revision 1.4  1995/05/30  05:22:21  krisna
 * architecture-os include protection
 *
 * Revision 1.3  1994/11/02  11:07:46  markc
 * Moved defines to arch-sparc.h
 *
 */

/*
 * inst-sparc.h - Common definitions to the SPARC specific instrumentation code.
 *
 * $Log: inst-sparc.h,v $
 * Revision 1.6  1996/08/16 21:19:01  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.5  1995/07/11 20:57:30  jcargill
 * Changed sparc-specific ifdefs to include sparc_tmc_cmost7_3
 *
 * Revision 1.4  1995/05/30  05:22:21  krisna
 * architecture-os include protection
 *
 * Revision 1.3  1994/11/02  11:07:46  markc
 * Moved defines to arch-sparc.h
 *
 * Revision 1.2  1994/07/26  19:57:28  hollings
 * moved instruction definitions to seperate header file.
 *
 * Revision 1.1  1994/01/27  20:31:23  hollings
 * Iinital version of paradynd speaking dynRPC igend protocol.
 *
 * Revision 1.2  1993/06/22  19:00:01  hollings
 * global inst state.
 *
 * Revision 1.1  1993/03/19  22:51:05  hollings
 * Initial revision
 *
 *
 */


#include "ast.h"
#include "as-sparc.h"

/* "pseudo" instructions that are placed in the tramp code for the inst funcs
 *   to patch up.   This must be invalid instructions (any instruction with
 *   its top 10 bits as 0 is invalid (technically UNIMP).
 *
 */

extern registerSpace *regSpace;

extern trampTemplate baseTemplate;
extern trampTemplate noArgsTemplate;
extern trampTemplate withArgsTemplate;

#endif
