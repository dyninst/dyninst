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

// $Id: Status.C,v 1.8 2004/03/20 20:44:47 pcroth Exp $

#include "Status.h"
#include <string.h>

/************************************************************************
 * static data structures.
************************************************************************/

unsigned    status_line::n_lines_ = 0;
unsigned    status_line::n_procs_ = 0;
Tcl_Interp* status_line::interp_  = 0;

status_line::status_pair* status_line::registry = status_line::reg_init();

/************************************************************************
 * implementations of member functions
************************************************************************/

status_line::status_line(const char* title, const LineType type)
    : type_(type) {

    my_pair = n_procs_ + n_lines_;
    status_pair* pair = &(registry[my_pair]);
    
    pair->name = title;
    pair->sl = this;
    
    if (type_==PROCESS)
    {
        id_=n_procs_++;
    }
    else 
    {
        id_=n_lines_++;
    }
    create(title);
}

status_line::~status_line() {
    assert(interp_);
    char buf[BUF_LENGTH];
    (void) sprintf(buf, "status_destroy %c %u", type_, id_);
    int ret = Tcl_VarEval(interp_, buf, 0);
    if (ret != TCL_OK) {
        fprintf(stderr, "status_line::~status_line: command `%s' -> `%s'\n",
            buf, interp_->result);
    }

    status_pair* pair = &(registry[my_pair]);
    
    pair->name = NULL;
    pair->sl = NULL;
    // assert(ret == TCL_OK);
}

void
status_line::message(const char* msg) {
    assert(interp_);
    char buf[BUF_LENGTH];
    (void) sprintf(buf, "status_message %c %u {%-*.*s}",
        type_, id_, (int) MESG_LENGTH, (int) MESG_LENGTH, msg);
    int ret = Tcl_VarEval(interp_, buf, 0);
    if (ret != TCL_OK) {
        fprintf(stderr, "status_line::message: command `%s' -> `%s'\n",
            buf, interp_->result);
    }
    // assert(ret == TCL_OK);
}

status_line&
status_line::operator<<(const char* msg) {
    message(msg);
    return *this;
}

void
status_line::state(State st) {
    assert(interp_);
    char buf[BUF_LENGTH];
    (void) sprintf(buf, "status_state %c %u %u", type_, id_, (st != NORMAL));
    int ret = Tcl_VarEval(interp_, buf, 0);
    if (ret != TCL_OK) {
        fprintf(stderr, "status_line::state: command `%s' -> `%s'\n",
            buf, interp_->result);
    }
    // assert(ret == TCL_OK);
}

void
status_line::status_init(Tcl_Interp* interp) {
    interp_ = interp;
}

void
status_line::create(const char* title) {
    assert(interp_);
    char buf[BUF_LENGTH];
    (void) sprintf(buf, "status_create %c %u {%-*.*s}",
        type_, id_, (int) TITLE_LENGTH, (int) TITLE_LENGTH, title);
    int ret = Tcl_VarEval(interp_, buf, 0);
    if (ret != TCL_OK) {
        fprintf(stderr, "status_line::create: command `%s' -> `%s'\n",
            buf, interp_->result);
    }
    // assert(ret == TCL_OK);
}

status_line::status_pair*
status_line::reg_init() {
    status_pair* retval = new status_pair[MAX_STATUS_LINES];
    for(int i = 0; i < MAX_STATUS_LINES; i++) {
        (retval[i]).name = NULL;
        (retval[i]).sl = NULL;
    }
    
    return retval;
}

status_line*
status_line::find(const char* name) {
    for(int i = 0; i <= (n_lines_ + n_procs_); i++)
        if(strcmp(name, (registry[i]).name) == 0) return (registry[i]).sl;
    return NULL;
}

