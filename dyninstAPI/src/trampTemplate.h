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

// $Id: trampTemplate.h,v 1.1 2003/10/21 17:22:53 bernat Exp $

// trampTemplate class definition

#ifndef TRAMP_TEMPLATE_H
#define TRAMP_TEMPLATE_H

#include "common/h/Types.h"
#include "inst.h" // callWhen

class instPoint;

// Todo: make things private/protected

class trampTemplate {
 public:
    unsigned size;
    void *trampTemp;		/* template of code to execute,
                               if pregenerated */
    Address baseAddr;       /* the base address of this tramp */

    int localPreOffset;     /* Offset to jump to pre-instrumentation */
    int localPreReturnOffset;/* Return from pre instrumentation */

    int localPostOffset;    /* Offset to jump to post-instrumentation */
    int localPostReturnOffset;/* Return from post instrumentation */

    int returnInsOffset;  
    int skipPreInsOffset; /* Branch around the pre save/restore */
    int skipPostInsOffset;/* Branch around the post save/restore */
    int emulateInsOffset; /* Address of emulated instruction block */
    int updateCostOffset; /* Update the global cost counter */

    int savePreInsOffset; /* Save regs before pre instrumentation */
    int restorePreInsOffset;/* Restore from above */
    int savePostInsOffset;/* Save regs before post instrumentation */
    int restorePostInsOffset;/* Restore from above */

#if defined(rs6000_ibm_aix4_1)
    // Or other on-the-fly generated tramp 
    int recursiveGuardPreJumpOffset;/* Minitramp guard jump offset */
    int recursiveGuardPostJumpOffset;
#elif defined(mips_sgi_irix6_4)
    // Needs all four
    int recursiveGuardPreOnOffset;
    int recursiveGuardPostOnOffset;
    int recursiveGuardPreOffOffset;
    int recursiveGuardPostOffOffset;
#endif
    int cost;			/* cost in cycles for this basetramp. */
    Address costAddr;           /* address of cost in this tramp      */
    bool prevInstru;
    bool postInstru;
    int  prevBaseCost;
    int  postBaseCost;

    const instPoint *location; /* Pointer to the owning inst point structure */
    process *proc; /* Process this base tramp is in */
    
    void updateTrampCost(int c);
    
    bool inBasetramp( Address addr );
    bool inSavedRegion( Address addr );
    
    miniTramps_list *pre_minitramps;
    miniTramps_list *post_minitramps;
    
    miniTramps_list *getMiniTrampList(callWhen when) {
        if (when == callPreInsn) return pre_minitramps;
        return post_minitramps;
    }


    trampTemplate() : baseAddr(0),
    prevInstru(false), postInstru(false),
    prevBaseCost(0), postBaseCost(0),
    location(NULL), proc(NULL),
    pre_minitramps(NULL), post_minitramps(NULL) {};

    // Normal constructor
    trampTemplate(const instPoint *l, process *p);
    
    // Fork constructor
    trampTemplate(const trampTemplate *t, process *p);

    // Assignment
    trampTemplate &operator=(const trampTemplate &t);
    
    // destructor
    ~trampTemplate();
};

extern trampTemplate baseTemplate;

#endif



