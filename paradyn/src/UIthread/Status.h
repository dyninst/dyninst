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

/************************************************************************
 * Status.h: tcl/tk text-widget based status lines.
 * $Id: Status.h,v 1.12 2004/03/23 01:12:29 eli Exp $
************************************************************************/


#if !defined(_Status_h_)
#define _Status_h_


#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "tcl.h"


/************************************************************************
 * symbolic constants.
************************************************************************/

static const unsigned STATUS_BUFSIZE = 128;
static const unsigned BUF_LENGTH     = 256;
static const unsigned TITLE_LENGTH   = 20;
static const unsigned MESG_LENGTH    = 200;

static const unsigned MAX_STATUS_LINES = 65536;

class status_line {
public:
    enum LineType {
        GENERAL='g',
        PROCESS='p'
    };

    enum State {
        NORMAL,
        URGENT
    };

     status_line (const char *, const LineType=GENERAL);
    ~status_line ();

    void            message (const char *);
    status_line& operator<< (const char *);
    void              state (State);

    static void status_init (Tcl_Interp *);

    static status_line* find(const char* name);
    
private:
    class status_pair {
      public:
        const char* name;
        status_line* sl;
    };

    void             create (const char *);

    LineType type_;
    unsigned id_;

    static status_pair* registry;
    static status_pair* reg_init();

    unsigned my_pair;
    
    static unsigned    n_lines_;
    static unsigned    n_procs_;
    static Tcl_Interp* interp_;

    status_line            (const status_line &); // explicitly private
    status_line& operator= (const status_line &); // explicitly private
};

#endif /* !defined(_Status_h_) */


