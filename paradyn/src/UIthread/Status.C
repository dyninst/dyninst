/************************************************************************
 * Status.c: static data structures for status lines.
************************************************************************/





/************************************************************************
 * header files.
************************************************************************/

#include "Status.h"





/************************************************************************
 * static data structures.
************************************************************************/

unsigned    status_line::n_lines_  = 0;
Tcl_Interp* status_line::interp_   = 0;
const char* status_line::init_cmd_ = "\n\
#\n\
# status line configuration variables.  set them here and recompile\n\
#\n\
\n\
set status_parent         .parent.status\n\
set status_title_fg       black\n\
set status_title_font     8x13bold\n\
set status_mesg_font      8x13\n\
set status_mesg_fg_normal blue\n\
set status_mesg_fg_urgent red\n\
\n\
\n\
#\n\
# status_create id title\n\
#\n\
# create a status line object, to be named by the integer id `id' and\n\
# having title `title'.  it is assumed that the title is formatted\n\
# to an appropriate constant width.  a `: ' is appended to all title\n\
# names before the status message is printed.\n\
#\n\
\n\
proc status_create {id title} {\n\
    global status_parent\n\
    set widget $status_parent.status_$id\n\
    set tag    status_tag_$id\n\
    set mark   status_mark_$id\n\
\n\
    text $widget\n\
\n\
    $widget insert end $title\n\
    $widget insert end \": \"\n\
\n\
    set tmark [expr [string length $title] + 1]\n\
\n\
    $widget tag  add $tag  1.0 1.$tmark\n\
    $widget mark set $mark     1.$tmark\n\
\n\
    global status_title_fg\n\
    global status_title_font\n\
    global status_mesg_font\n\
    global status_mesg_fg_normal\n\
\n\
    $widget tag configure $tag       \\\n\
        -foreground $status_title_fg \\\n\
        -font       $status_title_font
    $widget configure                      \\\n\
        -foreground $status_mesg_fg_normal \\\n\
        -font       $status_mesg_font      \\\n\
        -height     1                      \\\n\
        -wrap       none                   \\\n\
        -state      disabled\n\
    pack $widget -in $status_parent -side top -fill x\n\
    update\n\
}\n\
\n\
\n\
#\n\
# status_message id message\n\
#\n\
# make `message' the new message in status line with id `id'.\n\
# the status line must already exist.\n\
#\n\
\n\
proc status_message {id message} {\n\
    global status_parent\n\
    set widget $status_parent.status_$id\n\
    set tag    status_tag_$id\n\
    set mark   status_mark_$id\n\
\n\
    $widget configure -state normal\n\
    $widget delete [list $mark +1 chars] end\n\
    $widget insert end $message\n\
    $widget configure -state disabled\n\
    update\n\
}\n\
\n\
\n\
#\n\
# status_state id urgent\n\
#\n\
# set the state of status line `id' based on the boolean flag `urgent'\n\
#\n\
\n\
proc status_state {id urgent} {\n\
    global status_parent\n\
    set widget $status_parent.status_$id\n\
    set tag    status_tag_$id\n\
    set mark   status_mark_$id\n\
\n\
    global status_mesg_fg_normal\n\
    global status_mesg_fg_urgent\n\
\n\
    if $urgent {\n\
        $widget configure -foreground $status_mesg_fg_urgent\n\
    } {\n\
        $widget configure -foreground $status_mesg_fg_normal\n\
    }\n\
    update\n\
}\n\
\n\
\n\
#\n\
# status_destroy id\n\
#\n\
# destroy status line with id `id'.  all associated resources\n\
# are released\n\
#\n\
\n\
proc status_destroy {id} {\n\
    global status_parent\n\
    set widget $status_parent.status_$id\n\
    set tag    status_tag_$id\n\
    set mark   status_mark_$id\n\
\n\
    destroy $widget\n\
    update\n\
}\n\
";
