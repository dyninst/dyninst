# $Id: save.tcl,v 1.6 2004/03/20 20:44:50 pcroth Exp $
# this file contains the routines for the "EXPORT" button 

proc pdSave {} {
    global saveGlobalData savePhaseData saveResources saveDirectory \
	    saveMessage saveWindow saveGlobalSearch savePhaseSearch

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
    if {$saveGlobalSearch == 1} {
	paradyn save shg global $saveDirectory
    }
    if {$savePhaseSearch == 1} {
	paradyn save shg phase $saveDirectory
    }

    destroy $saveWindow
}

proc drawSaveMenu {} {
    global saveGlobalData savePhaseData saveResources saveDirectory \
	    saveMessage saveWindow saveGlobalSearch savePhaseSearch
    set saveWindow .pdsw
    toplevel $saveWindow -class Paradyn
    wm title $saveWindow "Paradyn Export"
    wm iconname $saveWindow "Paradyn Export"
    
    set di $saveWindow.directory
    set wh $saveWindow.what
    set ex $saveWindow.exit
    frame $di 
    frame $wh -relief raised -borderwidth 2
    frame $ex -relief raised -borderwidth 2
    pack $wh $di $ex -side top -fill both -padx 2 -pady 5

    # save what?
    label $wh.la -text "Export Paradyn Data" \
	    -foreground white -anchor c \
	    -relief raised \
	    -background purple
    pack  $wh.la -side top -fill both -expand true

    frame $wh.data 
    checkbutton $wh.data.gd -text "Global Data" -variable saveGlobalData 
    $wh.data.gd select
    checkbutton $wh.data.pd -text "Phase Data" -variable savePhaseData
    pack $wh.data.gd $wh.data.pd -side left -padx 25 -pady 5 -anchor w
    pack $wh.data -side top

    frame $wh.other
    checkbutton $wh.other.re -text "Where Axis" -variable saveResources
    $wh.other.re select
    pack $wh.other.re -side left
    pack $wh.other -side top 

    frame $wh.perf
    label $wh.perf.la -text "Performance Consultant: "
    checkbutton $wh.perf.pc -text "Global Search" \
            -variable saveGlobalSearch
    checkbutton $wh.perf.pcph -text "Phase Search(es)" \
            -variable savePhaseSearch
    pack $wh.perf.la $wh.perf.pc $wh.perf.pcph -side left \
            -padx 25 -pady 5 -anchor w
    pack $wh.perf -side top -fill both

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
    mkButtonBar $ex {} retval {{Export "pdSave"} \
	    {Clear "set saveDirectory \"\""} {Cancel "destroy .pdsw"} }
}

