# $Id: save.tcl,v 1.3 1998/03/03 23:09:49 wylie Exp $
# this file contains the routines for the "SAVE" button 

proc pdSave {} {
    global saveGlobalData savePhaseData saveResources saveDirectory \
	    saveMessage saveWindow

    # clear previous invalid entry message, if any
    set saveMessage "Enter name of Directory for Data/Resource Files"

    # make sure directory entry is valid
    if { ! [file exists $saveDirectory]} {
	#try to create the directory
	catch {exec mkdir $saveDirectory}
    }
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
    pack $wh $di $ex -side top -fill both -padx 2 -pady 5

    # save what?
    label $wh.la -text "Save Paradyn Data" \
	    -foreground white -anchor c \
	    -font { Times 14 bold } \
	    -relief raised \
	    -background purple
    pack  $wh.la -side top -fill both -expand true


    checkbutton $wh.gd -text "Global Data" -variable saveGlobalData 
    $wh.gd select
    checkbutton $wh.pd -text "Phase Data" -variable savePhaseData

    checkbutton $wh.re -text "Where Axes" -variable saveResources
    $wh.re select
    pack $wh.gd $wh.pd $wh.re -side left -padx 25 -pady 5 -anchor w

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

