#
# Copyright (c) 1996-2004 Barton P. Miller
# 
# We provide the Paradyn Parallel Performance Tools (below
# described as "Paradyn") on an AS IS basis, and do not warrant its
# validity or performance.  We reserve the right to update, modify,
# or discontinue this software at any time.  We shall have no
# obligation to supply such updates or modifications or any other
# form of support to you.
# 
# This license is for research uses.  For such uses, there is no
# charge. We define "research use" to mean you may freely use it
# inside your organization for whatever purposes you see fit. But you
# may not re-distribute Paradyn or parts of Paradyn, in any form
# source or binary (including derivatives), electronic or otherwise,
# to any other organization or entity without our permission.
# 
# (for other uses, please contact us at paradyn@cs.wisc.edu)
# 
# All warranties, including without limitation, any warranty of
# merchantability or fitness for a particular purpose, are hereby
# excluded.
# 
# By your use of Paradyn, you understand and agree that we (or any
# other person or entity with proprietary rights in Paradyn) are
# under no obligation to provide either maintenance services,
# update services, notices of latent defects, or correction of
# defects for Paradyn.
# 
# Even if advised of the possibility of such damages, under no
# circumstances shall we (or any other person or entity with
# proprietary rights in the software licensed hereunder) be liable
# to you or any third party for direct, indirect, or consequential
# damages of any character regardless of type of action, including,
# without limitation, loss of profits, loss of use, loss of good
# will, or computer failure or malfunction.  You agree to indemnify
# us (and any other person or entity with proprietary rights in the
# software licensed hereunder) for any and all liability it may
# incur to third parties resulting from your use of Paradyn.
#

# $Id: status.tcl,v 1.10 2004/03/23 19:12:16 eli Exp $
#
# status line configuration variables and associated commands.
# C++ status lines can be used after proper initialization.
# check `UIthread/UImain.C' for details.
#

set status_parent_g      .parent.status                 ;# generic status lines
set status_parent_p      .parent.procstatus.canvas.f    ;# process status lines
set procstatus_parent    .parent.procstatus

set status_title_fg       black
set status_mesg_fg_normal blue
set status_mesg_fg_urgent red

set extch         "+"   ;# process name truncation indicator character
set indent_chars   3    ;# indentation of process status lines to fit scrollbar
set min_processes  3    ;# minimum reasonable displayed scrollable lines
set max_processes  4    ;# default maximum displayed lines (adjusts on resize!)
set num_processes  0    ;# current number of process status lines

# 
# ProcCanvasResize %h
#
# should be bound to canvas Configure events such that it provides the new
# height whenever the canvas (window) resized, allowing a calculation of
# the appropriate number of process status lines to display (or scroll)
# and tidy-up the window height so that only entire lines are shown.
# The limit min_processes ensures that the canvas (scrollbar) doesn't get too
# small, whereas max_processes controls when to show the scrollbar
# (and this variable adjusts according to explicit window resize actions).
# Recalculating things explicitly, the height value provided is not used.
#

proc ProcCanvasResize {height} {
    global num_processes min_processes max_processes
    global procstatus_canvas procstatus_container

    set curr_height [ expr [ winfo height $procstatus_canvas ] - 2 ]
    set canv_height [ winfo reqheight $procstatus_container ]
    set bbox [ grid bbox $procstatus_container 0 0 ]
    set incr [ lindex $bbox 3 ]
    if { $incr == 0 } { return }
    set curr_lines [ expr $canv_height / $incr ]
    set drawable_lines [ expr round (double ($curr_height) / $incr) ]
    set draw_height [ expr $incr * $drawable_lines ]

#   puts stderr "Heights: Canvas=$canv_height, Window=$curr_height, Drawable=$draw_height"
#   puts stderr "#procs=$num_processes, #drawn=$curr_lines, #drawable=$drawable_lines"

    if { $drawable_lines > $max_processes } {
        # set max_processes to number drawable in explicitly enlarged window
#       puts stderr "Max_processes: $max_processes->$drawable_lines (+)"
        set max_processes $drawable_lines
    }

    if { $drawable_lines < $min_processes } {
        # fix-up resizes which would be too small (to scroll effectively)
#       puts stderr "Drawable_lines: $drawable_lines->$min_processes"
        set drawable_lines $min_processes
    } else {
        if { ($drawable_lines < $max_processes) && 
             ($curr_lines > $drawable_lines) } {
            # set max_processes to number drawable in explicitly shrunk window
#           puts stderr "Max_processes: $max_processes->$drawable_lines (-)"
            set max_processes $drawable_lines
        }
    }

    set disp_lines $num_processes       ;# initialize to reasonable default

    if { $num_processes > $min_processes } {
        set disp_lines $drawable_lines
#       puts stderr "Modify display to current drawable_lines=$disp_lines"
    }

    if { $disp_lines > $num_processes } {
        set disp_lines $num_processes
#       puts stderr "Truncate display to current num_processes=$disp_lines"
    }

    set disp_height [ expr $incr * $disp_lines ]

    if { $disp_height != $curr_height } {
#       puts stderr "ProcCanvasResize from ${curr_height}($curr_lines) to ${disp_height}($disp_lines)"
        $procstatus_canvas config -height $disp_height
        wm geometry . {}        ;# no idea why this is necessary!
    }

}

#
# ProcCanvasScroll offset size
#
# called whenever the canvas view changes by scrolling or resizing, enables
# a decision on whether to actually display the scrollbar or its placeholder.
# Since the scrollbar/placeholder and canvas need to be re-packed on a switch
# (to ensure that they remain correctly left-packed and the canvas (text) is 
# right-truncated) and this can be expensive (for large/complex canvases) the
# scrollbarVisible state variable is also needed.
#

proc ProcCanvasScroll {offset size} {
    global num_processes min_processes max_processes
    global procstatus_canvas procstatus_scrollbar procstatus_placeholder
    global scrollbarVisible 
    # puts stderr "ProcCanvasScroll ($offset,$size) #p=$num_processes/$max_processes"

    if { ($num_processes > $max_processes) &&
         ($offset != 0.0 || $size != 1.0) } {
        if { $scrollbarVisible } {
            # puts stderr "showing scrollbar (skipped)"
        } else {
            # puts stderr "showing scrollbar..."
            pack forget $procstatus_canvas              ;# temporarily
            pack forget $procstatus_placeholder         ;# hide it
            pack $procstatus_scrollbar -side left -fill y
            pack $procstatus_canvas -side left -fill both -expand true
            set scrollbarVisible true
        }
        $procstatus_scrollbar set $offset $size
    } else {
        if { $scrollbarVisible } {
            # puts stderr "hiding scrollbar..."
            pack forget $procstatus_canvas              ;# temporarily
            pack forget $procstatus_scrollbar           ;# hide it
            pack $procstatus_placeholder -side left -fill y
            pack $procstatus_canvas -side left -fill both -expand true
            set scrollbarVisible false
        } else {
            # puts stderr "hiding scrollbar (skipped)"
        }
    }
}

#
# ProcCanvasCreate frame_widget
#
# creates a new canvas with (vertical) scrollbar for the process status lines,
# with the scrollbar replaced with a placeholder (of equivalent size) when
# less than a reasonable number of lines are present.  Note that the size of
# the scrollbar (and placeholder) have been chosen to match the status text
# font, such that they occupy $indent_chars (of the process status line title)
# and ensure that the message text is reasonably aligned.
#

proc ProcCanvasCreate { procstatus } {
    global procstatus_canvas procstatus_container
    global procstatus_scrollbar procstatus_placeholder scrollbarVisible
    global indent_chars
    set procstatus_canvas $procstatus.canvas
    set procstatus_container $procstatus_canvas.f
    set procstatus_scrollbar $procstatus.scrollbar
    set procstatus_placeholder $procstatus.placeholder
    set scrollbarVisible false

    #puts stderr "ProcCanvasCreate..."
    set bogofont [option get $procstatus font Font]
    set ch_width [font measure $bogofont "M"]
    set indwidth [expr $ch_width * $indent_chars - 2]
    set barwidth [expr $indwidth - 6]

    frame $procstatus_placeholder -width $indwidth -background DimGray
    scrollbar $procstatus_scrollbar -orient vertical -width $barwidth \
        -command [ list $procstatus_canvas yview ]
    canvas $procstatus_canvas -height 0 -yscrollcommand "ProcCanvasScroll"
    pack $procstatus_placeholder -side left -fill y
    pack $procstatus_scrollbar -side left -fill y
    pack forget $procstatus_scrollbar           ;# hide the scrollbar for now
    pack $procstatus_canvas -side left -fill both -expand true
    pack $procstatus -fill both -expand true

    frame $procstatus_container
    $procstatus_canvas create window 0 0 -anchor nw \
        -window $procstatus_container

    # ensure that window "resize" events are caught and handled appropriately
    bind $procstatus_canvas <Configure> { ProcCanvasResize %h }
}

#
# status_create type id title
#
# create a status line object, referenced by `type' and integer id `id' and
# having title `title'.  `type' is "g" for generic status lines and "p" for
# process status lines which are to appear in a separate scrollable canvas.
# It is assumed that the title is formatted to an appropriate constant width.
# A `: ' is appended to all title names before the status message is printed.
# Process titles are truncated by $indent_chars to allow for the scrollbar.
#

proc status_create {type id title} {

#   puts stderr "status_create ($type, $id, $title)"

    global procstatus_parent
    global indent_chars extch
    global num_processes min_processes max_processes
    if { $type == "p" && $num_processes == 0 } { 
        # create separate canvas for process status list
        ProcCanvasCreate $procstatus_parent
    }

    global status_parent_g
    global status_parent_p              ;# NB: process canvas must exist!

    set parent [set status_parent_$type]
    set widget $parent.status_$id

    set tag    status_tag_$type$id
    set mark   status_mark_$type$id

    text $widget -relief raised -padx 4

    set titlelen [ string length $title ]

    if { $type == "p" } {
        incr num_processes
        $widget config -width 200       ;# wide width to be truncated when drawn
        # uncompromisingly strip last three characters from title string 
        # corresponding to the width of the additional process canvas scrollbar
        set trunc [ expr $titlelen-$indent_chars-1 ]
        set endch [ string index $title $trunc ]
        if { $endch != " " } { set endch $extch }       ;# indicate truncation
        set title [ string range $title 0 [ expr $trunc-1] ]
        set title [ append title $endch ]
    }

    $widget insert end "$title: "
    set tmark [expr [string length $title] + 1]

    $widget tag  add $tag  1.0 1.$tmark
    $widget mark set $mark     1.$tmark

    global status_title_fg
    global status_mesg_fg_normal

    wm geometry . {}

    $widget tag configure $tag       \
        -foreground $status_title_fg
    $widget configure                      \
        -foreground $status_mesg_fg_normal \
        -height     1                      \
        -wrap       none                   \
        -state      disabled \
	-highlightthickness 0 \
	-borderwidth 1
       # the default borderwidth is a much larger number

    if { $type == "p" } {
        global procstatus_canvas        ;# $parent = $procstatus_container
        grid $widget -in $parent -sticky we
        # reconfigure the canvas to take account of the new addition
        tkwait visibility $widget
        set bbox [ grid bbox $parent 0 0 ]
        set incr [ lindex $bbox 3 ]
        set reqwidth [ winfo reqwidth $parent ]
        set reqheight [ winfo reqheight $parent ]
        $procstatus_canvas config -scrollregion "0 0 $reqwidth $reqheight"
        $procstatus_canvas config -yscrollincrement $incr
        # expand the process canvas to show our new addition,
        # ... truncating at our desired maximum size
        set new_proc $num_processes
        if { $new_proc > $max_processes } { set new_proc $max_processes }
        set height [ expr $incr * $new_proc ]
        $procstatus_canvas config -height $height
        if { $num_processes > $max_processes } {
            # currently scrolling (or soon will be) therefore let's
            # scroll down the canvas to show the newly added line
            $procstatus_canvas yview moveto 1.0
        }
    } else {
        pack $widget -in $parent -side top -fill x
    }

    update
    #
    # Need to use  "update" commands sparingly because they
    # seem to produce a problem (or make it worse as Ari mentioned) when  
    # the user "grab" the main window for long enough, making some widgets
    # "invisibles" (e.g. status line). We also had to add the command
    # wm geometry . {} because the previous solution does not always 
    # work. It just reduces the interval of time when the user could
    # grab the main window and affect the display of the widgets (i.e.
    # it makes the height so small that we cannot see the widget on the
    # screen. That is why we set this value again). Any better solution
    # will be welcome! - naim
    #
}


#
# status_message type id message
#
# make `message' the new text in status line `type'.`id'.
# the status line must already exist.
#

proc status_message {type id message} {
    global status_parent_g
    global status_parent_p

    set parent [set status_parent_$type]
    set widget $parent.status_$id
    set tag    status_tag_$type$id
    set mark   status_mark_$type$id

    $widget configure -state normal
    $widget delete [list $mark +1 chars] end
    $widget insert end $message
    $widget configure -state disabled

    # The paradyn UI freezes when starting paradynd and at other times.
    # At such times, it is advantageous for us to ensure that every
    # status line is updated before the freeze.  When paradyn stops freezing,
    # we can remove this line, which slows things down quite a bit.
    #update
}


#
# status_state type id urgent
#
# set the state of status line `type'.`id' based on the boolean flag `urgent'
#

proc status_state {type id urgent} {
    global status_parent_g
    global status_parent_p

    set parent [set status_parent_$type]
    set widget $parent.status_$id

    global status_mesg_fg_normal
    global status_mesg_fg_urgent

    if $urgent {
        $widget configure -foreground $status_mesg_fg_urgent
    } {
        $widget configure -foreground $status_mesg_fg_normal
    }
    
    # See the argument in the above routine...
    update
}


#
# status_destroy type id
#
# destroy status line `type'.`id' and release associated resources
#

proc status_destroy {type id} {
    global status_parent_g
    global status_parent_p

    global num_processes

    set parent [set status_parent_$type]
    set widget $parent.status_$id

    destroy $widget
    incr num_processes -1
    if { $num_processes == 0 } { 
        puts stderr "Unpacking procframe: #p=$num_processes"
        pack forget .parent.procstatus
    }

    #update
}
