#if !defined(sparc_sun_sunos4_1_3) && !defined(sparc_sun_solaris2_4)
#error "invalid architecture-os inclusion"
#endif

#ifndef INST_SPARC_H
#define INST_SPARC_H

/*
 * $Log: inst-sparc.h,v $
 * Revision 1.4  1995/05/30 05:22:21  krisna
 * architecture-os include protection
 *
 * Revision 1.3  1994/11/02  11:07:46  markc
 * Moved defines to arch-sparc.h
 *
 */

/*
 *  Copyright 1993 Jeff Hollingsworth.  All rights reserved.
 *
 */

/*
 * inst-sparc.h - Common definitions to the SPARC specific instrumentation code.
 *
 * $Log: inst-sparc.h,v $
 * Revision 1.4  1995/05/30 05:22:21  krisna
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
