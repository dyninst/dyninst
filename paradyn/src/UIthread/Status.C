#include "Status.h"

/************************************************************************
 * static data structures.
************************************************************************/

unsigned    status_line::n_lines_ = 0;
Tcl_Interp* status_line::interp_  = 0;

/************************************************************************
 * implementations of member functions
************************************************************************/

status_line::status_line(const char* title)
    : id_(n_lines_++) {
    create(title);
}

status_line::~status_line() {
    assert(interp_);
    char buf[BUF_LENGTH];
    (void) sprintf(buf, "status_destroy %u", id_);
    int ret = Tcl_VarEval(interp_, buf, 0);
    if (ret != TCL_OK) {
        fprintf(stderr, "status_line::~status_line: command `%s' -> `%s'\n",
            buf, interp_->result);
    }
    // assert(ret == TCL_OK);
}

void
status_line::message(const char* msg) {
    assert(interp_);
    char buf[BUF_LENGTH];
    (void) sprintf(buf, "status_message %u {%-*.*s}",
        id_, (int) MESG_LENGTH, (int) MESG_LENGTH, msg);
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
    (void) sprintf(buf, "status_state %u %u", id_, (st != NORMAL));
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
    (void) sprintf(buf, "status_create %u {%-*.*s}",
        id_, (int) TITLE_LENGTH, (int) TITLE_LENGTH, title);
    int ret = Tcl_VarEval(interp_, buf, 0);
    if (ret != TCL_OK) {
        fprintf(stderr, "status_line::create: command `%s' -> `%s'\n",
            buf, interp_->result);
    }
    // assert(ret == TCL_OK);
}


