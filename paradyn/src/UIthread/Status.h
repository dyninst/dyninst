/************************************************************************
 * Status.h: tcl/tk text-widget based status lines.
************************************************************************/


#if !defined(_Status_h_)
#define _Status_h_


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

#endif /* !defined(_Status_h_) */
