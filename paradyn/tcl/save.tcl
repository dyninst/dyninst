#
# Copyright (c) 1993, 1994 Barton P. Miller, Jeff Hollingsworth,
#     Bruce Irvin, Jon Cargille, Krishna Kunchithapadam, Karen
#     Karavanic, Tia Newhall, Mark Callaghan.  All rights reserved.
# 
#  This software is furnished under the condition that it may not be
#  provided or otherwise made available to, or used by, any other
#  person, except as provided for by the terms of applicable license
#  agreements.  No title to or ownership of the software is hereby
#  transferred.  The name of the principals may not be used in any
#  advertising or publicity related to this software without specific,
#  written prior authorization.  Any use of this software must include
#  the above copyright notice.
#

# this file contains the routines for the "SAVE" button 

proc pdSave {} {
    global saveGlobalData savePhaseData saveResources saveDirectory \
	    saveMessage saveWindow

    # clear previous invalid entry message, if any
    set saveMessage "Enter name of Directory for Data/Resource Files"

    # make sure directory entry is valid
    if { (! [file isdirectory $saveDirectory]) || \
	    (![file writable $saveDirectory]) } {
	set saveMessage "Error: invalid directory name"
	puts "\a"
	return
    }
    if {$saveGlobalData == 1}  {
	if {$savePhaseData == 0} {
	    paradyn save data global $saveDirectory
	} else {
	    paradyn save data all $saveDirectory
	}
    } else {
	if {$savePhaseData == 1} {
	    paradyn save data phase $saveDirectory
	}
    }
    if {$saveResources == 1} {
	    paradyn save resources all $saveDirectory
    }
    destroy $saveWindow
}

proc drawSaveMenu {} {
    global saveGlobalData savePhaseData saveResources saveDirectory \
	    saveMessage saveWindow
    set saveWindow .pdsw
    toplevel $saveWindow
    wm title $saveWindow "Paradyn Save"
    wm iconname $saveWindow "Paradyn Save"
    
    set di $saveWindow.directory
    set wh $saveWindow.what
    set ex $saveWindow.exit
    frame $di 
    frame $wh -relief raised -borderwidth 2
    frame $ex -relief raised -borderwidth 2
    pack $wh $di $ex -side top -fill both -padx 2 -pady 2

    # save what?
    label $wh.la -text "Save What?"
    pack $wh.la -side top -padx 10 -pady 5 
    checkbutton $wh.gd -text "Global Data" -variable saveGlobalData 
    $wh.gd select
    checkbutton $wh.pd -text "Phase Data" -variable savePhaseData

    checkbutton $wh.re -text "Where Axes" -variable saveResources
    $wh.re select
    pack $wh.gd $wh.pd $wh.re -side top -padx 10 -pady 5 -anchor w

    #directory?

    # make the list frame and set up resizing 
    frame $di.top 
    frame $di.bot
    entry $di.top.ent -width 50 -textvariable saveDirectory -relief sunken
    bind $di.top.ent <Return> "pdSave"
    label $di.top.la -text "Directory:"
    pack $di.top.la $di.top.ent -side left -padx 5
    set saveMessage "Enter name of Directory for Data/Resource Files"
    label $di.bot.ms -textvariable saveMessage
    pack $di.bot.ms -side top
    pack $di.top $di.bot -side top -fill both

    #exit
    mkButtonBar $ex {} retval {{SAVE "pdSave"} \
	    {CLEAR "set saveDirectory \"\""} {CANCEL "destroy .pdsw"} }
}

