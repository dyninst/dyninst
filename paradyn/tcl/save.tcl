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

# $Id: save.tcl,v 1.7 2004/03/23 19:12:13 eli Exp $
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

