# mets.tcl
# this file contains procedures used in prompting user for metric/resource
# choices for a visualization.  When the resource hierarchy display is 
# complete, this will be changed and will only be used to get the metric
# choices directly.

# $Log: mets.tcl,v $
# Revision 1.12  1995/10/06 19:52:25  naim
# Minor change: DONE key is disabled if there are no metrics selected - naim
#
# Revision 1.11  1995/09/26  20:47:49  naim
# Minor comment about error message displaying
#
# Revision 1.10  1994/11/07  05:41:27  karavan
# Added clear button
#
# Revision 1.9  1994/11/07  00:32:06  karavan
# eliminated default clearing of the where axis.
#
# Revision 1.8  1994/11/01  05:46:20  karavan
# changed resource selection to allow multiple focus selection on a
# single display.
#
# Revision 1.7  1994/10/25  17:55:11  karavan
# Implemented Resource Display Objects, which manage display of multiple
# resource Abstractions.
#
# Revision 1.6  1994/10/09  01:15:28  karavan
# Implemented new UIM/visithread interface with metrespair data structure
# and selection of resources directly on the where axis.
#
# Revision 1.5  1994/09/13  05:06:36  karavan
# changes to accommodate multiple simultaneous menus.
#
# Revision 1.4  1994/07/21  01:53:33  rbi
# YANS -- Yet another new style
#
# Revision 1.3  1994/07/15  04:16:25  hollings
# Made the resource entry a useful size.
#
# Revision 1.2  1994/05/07  23:25:43  karavan
# added msg if no metrics defined
#
# Revision 1.1  1994/05/07  18:11:43  karavan
# initial version
#

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

# acceptMetChoices
# parse metric selections from checkbuttons to a list of indices into 
# the current Metrics array

proc acceptMetChoices {} {
    global tclSelectedMetrics metCount metmenuCB

    set tclSelectedMetrics ""
    for {set i 0} {$i < $metCount} {incr i} {
	if {[expr $metmenuCB($i)] > 0} {
	    lappend tclSelectedMetrics $i
	} 	
    }
#    puts "metric choices: $tclSelectedMetrics"
    
    return [llength $tclSelectedMetrics]  
}

# sendAllSelections
# processes and returns to VISIthread metric and resource selections
# returns 1 if no metrics selected, 2 if no focus selected, 0 if OK

proc sendAllSelections {visiToken rdoToken} {
    global tclSelectedMetrics
#    puts "sendAllSels vTok pTok: $visiToken $rdoToken"
    if {[acceptMetChoices] == 0} {
	return 1
    }
    if [uimpd processVisiSelection $rdoToken $tclSelectedMetrics] {
	uimpd sendVisiSelections $visiToken 0
	return 0
    } else {
	return 2
    }
}

# endSelection
# handles cancel and done buttons for resource/metric selection
# note: must still call sendVisiSelections on cancel so reply msg is sent to 
#       requesting VISI thread
proc endSelection {visiToken rdoToken cancelflag win} {
    global metList metCount
    if $cancelflag {
	uimpd sendVisiSelections $visiToken 1
	set x 0
    } else {
	set x [sendAllSelections $visiToken $rdoToken]
    }
    if {$x == 1} {
	$win.top.msg configure -text \
		"Please select one or more metrics or press cancel"
		#
		# I think we should use showError here. It does not
		# make sense to have several equivalent routines to
		# display error messages
		#
    } elseif {$x == 2} {
	$win.top.msg configure -text \
		"Please select a focus on the where axis or press cancel"
    } else {
	unset metList
	unset metCount
	set tclSelectionState 0
	destroy $win
    }
} 

proc clearMetSelects {} {
    global metCount metmenuCB metSelected
    for {set i 0} {$i < $metCount} {incr i} {
	set metmenuCB([expr $i]) 0
    }
    set metSelected 0
}

#
# metSelectedProc
# Counts how many metrics have been selected
# This value, stored in metSelected, is used ir
# order to enable and disable the DONE button
# (i.e. DONE button is enabled iff metSelected>0)
#

proc metSelectedProc {i w} {
    global metSelected metmenuCB
    if {[expr $metmenuCB($i)]>0} {
      incr metSelected
    } else {
      incr metSelected -1
    }
    if {$metSelected>0} {
      $w.bot.b2 configure -state normal 
    } else {
      $w.bot.b2 configure -state disabled
    }
}

# getMetsAndRes 
# called from  UIM::chooseMetricsandResources
# Metric/Resource selection starts here!

proc getMetsAndRes {metsAndResID rdo} {
    global metCount metList metMenuCtr tclSelectionState 
    global metSelected metmenuCB

    set tclSelectionState 1
    incr metMenuCtr
    set w .metmenunew$metMenuCtr
    set ret 0
    set msg3 "No Metrics Currently Defined"
    # toplevel window
    toplevel $w  -bd 0
    wm title $w "Paradyn Metrics Menu"
    wm iconname $w "PD Metrics Menu"
    wm geometry $w +425+100
    focus $w

    mkFrame $w.top {top fill expand} -relief raised -border 1
    mkMessage $w.top.msg "" {top expand padx 20 pady 20} \
	    -aspect 1000 -text "Select Metrics and Focus(es) below" \
	    -font -Adobe-times-bold-r-normal--*-120*
    
    if {$metCount == 0} {
	mkMessage $w.top.nometsmsg $msg3 {top expand} \
		-font -Adobe-times-medium-r-normal--*-120*
    } else {
	frame $w.top.1 
	frame $w.top.2
	frame $w.top.3
	if { [expr $metCount % 3] > 0} {
	    set colSize [expr (($metCount/3) + 1)]
	} else {
	    set colSize [expr ($metCount/3)]
	}
	set colNum 1
	set cCnt 1
	set metSelected 0
	for {set i 0} {$i < $metCount} {incr i} {
	    checkbutton $w.top.$colNum.cb$i  -width 20 -anchor w -padx 2 \
		    -variable metmenuCB([expr $i]) \
		    -text [lindex $metList [expr $i]] \
		    -command "metSelectedProc $i $w"

            if {[expr $metmenuCB($i)]>0} {
	      incr metSelected
	    }

	    pack $w.top.$colNum.cb$i -side top
	    if { $cCnt >= $colSize} { 
		incr colNum
		set cCnt 1
	    } else {
		incr cCnt
	    }
	}
	pack $w.top.1 $w.top.2 $w.top.3 -side left -fill both -expand 1
    }

    # buttons
    mkFrame $w.bot {bottom fill expand} -relief raised -borderwidth 4

    button $w.bot.b0 -text "CLEAR" -width 12 \
	    -command "clearMetSelects"

    button $w.bot.b1 -text "CANCEL" -width 12 \
	    -command "endSelection $metsAndResID $rdo 1 $w"

    button $w.bot.b2 -text "DONE" -width 12 \
	    -command "endSelection $metsAndResID $rdo 0 $w"

    if {$metSelected==0} {
      $w.bot.b2 configure -state disabled
    } 

    pack $w.bot.b0 $w.bot.b1 $w.bot.b2 -side left -padx 20 -expand 1 \
	    -pady 4

}

