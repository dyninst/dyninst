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

# $Id: mets.tcl,v 1.22 2004/03/23 19:12:12 eli Exp $
# this file contains procedures used in prompting user for metric/resource
# choices for a visualization.  When the resource hierarchy display is 
# complete, this will be changed and will only be used to get the metric
# choices directly.
# getMetsAndRes (bottom of this file) is the entry-point.
#

proc acceptMetChoices {threadid numMetrics metIndexes2Id} {
    # Called by sendAllSelections, below.  Fills in tclSelectedMetrics($threadid).
    # NOTE a new requirement: tclSelectedMetrics($threadid) MUST contain metric ids,
    #      not metric indexes!
    global tclSelectedMetrics metmenuCB

    set tclSelectedMetrics($threadid) ""
    #puts stderr "welcome to acceptMetChoices; threadid=$threadid; metIndexes2Id=$metIndexes2Id"

    for {set i 0} {$i < $numMetrics} {incr i} {
	if {$metmenuCB($threadid$i) > 0} {
	    set theMetricId [lindex $metIndexes2Id $i]
	    lappend tclSelectedMetrics($threadid) $theMetricId
	} 	
    }
    
    return [llength $tclSelectedMetrics($threadid)]  
}

# sendAllSelections
# processes and returns to VISIthread metric and resource selections
# returns 1 if no metrics selected, 2 if no focus selected, 0 if OK

proc sendAllSelections {visiToken threadid numMetrics metIndexes2Id} {
    # Called by endSelection, below.

    global tclSelectedMetrics

    # Fill in tclSelectedMetrics($threadid):
    #puts stderr "Welcome to sendAllSelections; metIndexes2Id=$metIndexes2Id"
    if {[acceptMetChoices $threadid $numMetrics "$metIndexes2Id"] == 0} {
	return 1
    }

    #puts stderr "sendAllSelections: about to call uimpd processVisiSelection with tclSelectedMetrics for this threadid of:"
    #puts stderr $tclSelectedMetrics($threadid)

    if [uimpd processVisiSelection $tclSelectedMetrics($threadid)] {
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
proc endSelection {visiToken cancelflag threadid numMetrics metIndexes2Id} {
    # If cancelflag is false, this routine calls sendAllSelections, above.
    # If all's well, it then deletes the window.

    #puts stderr "welcome to endSelection; metIndexes2Id=$metIndexes2Id"

    if $cancelflag {
	uimpd sendVisiSelections $visiToken 1
	set x 0
    } else {
	set x [sendAllSelections $visiToken $threadid $numMetrics "$metIndexes2Id"]
    }

    set win .metmenunew$threadid
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
	destroy $win
    }
} 

proc clearMetSelects {threadid numMetrics metIndexes2Id} {
    global metmenuCB metSelected selectedMetricNames

    #puts stderr "welcome to clearMetSelects; metIndexes2Id=$metIndexes2Id"

    for {set i 0} {$i < $numMetrics} {incr i} {
      set metricId [lindex $metIndexes2Id $i]
      set selectedMetricNames($metricId) 0
      set metmenuCB($threadid$i) 0
    }
    set metSelected($threadid) 0

    set w .metmenunew$threadid
    $w.bot.b0 configure -state disabled
}

#
# metSelectedProc
# Counts how many metrics have been selected
# This value, stored in metSelected, is used in
# order to enable and disable the DONE button
# (i.e. DONE button is enabled iff metSelected>0)
#

proc metSelectedProc {i threadid numMetrics metIndexes2Id} {
    # new: metSelected is an array indexed by window-name
    #      (so things don't get messed up when >1 window is open
    #      at a time)
    global metSelected metmenuCB
    global selectedMetricNames
    #puts stderr "welcome to metSelectedProc; metIndexes2Id=$metIndexes2Id"

    set metricId [lindex $metIndexes2Id $i]

    if {$metmenuCB($threadid$i)>0} {
      incr metSelected($threadid)
      set selectedMetricNames($metricId) 1
    } else {
      incr metSelected($threadid) -1
      set selectedMetricNames($metricId) 0
    }

    set w .metmenunew$threadid
    if {$metSelected($threadid)>0} {
      $w.bot.b0 configure -state normal 
    } else {
      $w.bot.b0 configure -state disabled
    }
}

# getMetsAndRes 
# called from  UIM::chooseMetricsandResources
# Metric/Resource selection starts here!

proc getMetsAndRes {metsAndResID requestingThread numMetrics metIndexes2Id} {
    # requestingThread can help us identify specific visis
    #puts stderr "welcome to getMetsAndRes; metIndexes2Id=$metIndexes2Id"

    global metricNamesById
    global metSelected metmenuCB

    set w .metmenunew$requestingThread
    if {[winfo exists $w]} {
       # puts stderr "Window $w already exists --> this visi already has a pending metric selection..."

       # Simulate a cancel here.
       uimpd sendVisiSelections $metsAndResID 1

       return
    }

    # toplevel window
    toplevel $w  -bd 0 -class Paradyn
    wm title $w "Paradyn Metrics Menu"
    wm iconname $w "PD Metrics Menu"
    wm geometry $w +425+100
    focus $w

    mkFrame $w.top {top fill expand} -relief raised -border 1
    mkMessage $w.top.msg "" {top expand padx 20 pady 20} \
	    -aspect 1000 -text "Select Metrics and Focus(es) below"
   
    set metSelected($requestingThread) 0
    if {$numMetrics == 0} {
        set msg3 "No Metrics Currently Defined"
	mkMessage $w.top.nometsmsg $msg3 {top expand}
    } else {
	frame $w.top.1 
	frame $w.top.2
	frame $w.top.3
	if { [expr $numMetrics % 3] > 0} {
	    set colSize [expr (($numMetrics/3) + 1)]
	} else {
	    set colSize [expr ($numMetrics/3)]
	}
	set colNum 1
	set cCnt 1

	for {set i 0} {$i < $numMetrics} {incr i} {
	    set metricId [lindex $metIndexes2Id $i]
            set metricName $metricNamesById($metricId)

            global selectedMetricNames
            if {[array names selectedMetricNames $metricId] == ""} {
               set selectedMetricNames($metricId) 0
	    }
            set initiallySet $selectedMetricNames($metricId)
            set metmenuCB($requestingThread$i) $initiallySet

	    checkbutton $w.top.$colNum.cb$i  -width 20 -anchor w -padx 2 \
		    -variable metmenuCB($requestingThread$i) \
		    -text $metricName \
		    -command "metSelectedProc $i $requestingThread $numMetrics {$metIndexes2Id}"

            if {$metmenuCB($requestingThread$i) > 0} {
	      incr metSelected($requestingThread)
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

    button $w.bot.b0 -text "Accept" -width 12 \
	    -command "endSelection $metsAndResID 0 $requestingThread $numMetrics {$metIndexes2Id}"

    button $w.bot.b1 -text "Clear" -width 12 \
	    -command "clearMetSelects $requestingThread $numMetrics {$metIndexes2Id}"

    button $w.bot.b2 -text "Cancel" -width 12 \
	    -command "endSelection $metsAndResID 1 $requestingThread $numMetrics {$metIndexes2Id}"

    if {$metSelected($requestingThread)==0} {
      $w.bot.b0 configure -state disabled
    } 

    pack $w.bot.b0 $w.bot.b1 $w.bot.b2 -side left -padx 20 -expand 1 \
	    -pady 4
}
