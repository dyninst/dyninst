#
# status.tcl
#
# status line configuration variables and associated commands.
# C++ status lines can be used after proper initialization.
# check `UIthread/UImain.C' for details.
#

set status_parent         .parent.status
set status_title_fg       black
set status_title_font     8x13bold
set status_mesg_font      8x13
set status_mesg_fg_normal blue
set status_mesg_fg_urgent red


#
# status_create id title
#
# create a status line object, to be named by the integer id `id' and
# having title `title'.  it is assumed that the title is formatted
# to an appropriate constant width.  a `: ' is appended to all title
# names before the status message is printed.
#

proc status_create {id title} {
    global status_parent
    set widget $status_parent.status_$id
    set tag    status_tag_$id
    set mark   status_mark_$id

    text $widget

    $widget insert end $title
    $widget insert end ": "

    set tmark [expr [string length $title] + 1]

    $widget tag  add $tag  1.0 1.$tmark
    $widget mark set $mark     1.$tmark

    global status_title_fg
    global status_title_font
    global status_mesg_font
    global status_mesg_fg_normal

    $widget tag configure $tag       \
        -foreground $status_title_fg \
        -font       $status_title_font
    $widget configure                      \
        -foreground $status_mesg_fg_normal \
        -font       $status_mesg_font      \
        -height     1                      \
        -wrap       none                   \
        -state      disabled
    pack $widget -in $status_parent -side top -fill x
    update
}


#
# status_message id message
#
# make `message' the new message in status line with id `id'.
# the status line must already exist.
#

proc status_message {id message} {
    global status_parent
    set widget $status_parent.status_$id
    set tag    status_tag_$id
    set mark   status_mark_$id

    $widget configure -state normal
    $widget delete [list $mark +1 chars] end
    $widget insert end $message
    $widget configure -state disabled
    update
}


#
# status_state id urgent
#
# set the state of status line `id' based on the boolean flag `urgent'
#

proc status_state {id urgent} {
    global status_parent
    set widget $status_parent.status_$id
    set tag    status_tag_$id
    set mark   status_mark_$id

    global status_mesg_fg_normal
    global status_mesg_fg_urgent

    if $urgent {
        $widget configure -foreground $status_mesg_fg_urgent
    } {
        $widget configure -foreground $status_mesg_fg_normal
    }
    update
}


#
# status_destroy id
#
# destroy status line with id `id'.  all associated resources
# are released
#

proc status_destroy {id} {
    global status_parent
    set widget $status_parent.status_$id
    set tag    status_tag_$id
    set mark   status_mark_$id

    destroy $widget
    update
}
