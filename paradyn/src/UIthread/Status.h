/************************************************************************
 * Status.h: tcl/tk text-widget based status lines.
************************************************************************/





#if !defined(_Status_h_)
#define _Status_h_





/************************************************************************
 * header files.
************************************************************************/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "tclclean.h"





/************************************************************************
 * symbolic constants.
************************************************************************/

static const unsigned STATUS_BUFSIZE = 128;
static const unsigned BUF_LENGTH     = 256;
static const unsigned TITLE_LENGTH   = 20;
static const unsigned MESG_LENGTH    = 200;





/************************************************************************
 * class status_line
************************************************************************/

class status_line {
public:
     status_line (const char *);
    ~status_line ();

    enum State {
        NORMAL,
        URGENT
    };

    void            message (const char *);
    status_line& operator<< (const char *);
    void              state (State);

    static void status_init (Tcl_Interp *);

private:
    void             create (const char *);

    unsigned id_;

    static unsigned    n_lines_;
    static Tcl_Interp* interp_;

    status_line            (const status_line &); // explicitly private
    status_line& operator= (const status_line &); // explicitly private
};

inline
status_line::status_line(const char* title)
    : id_(n_lines_++) {
    create(title);
}

inline
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

inline
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

inline
status_line&
status_line::operator<<(const char* msg) {
    message(msg);
    return *this;
}

inline
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

inline
void
status_line::status_init(Tcl_Interp* interp) {
    interp_ = interp;
}

inline
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





#endif /* !defined(_Status_h_) */
