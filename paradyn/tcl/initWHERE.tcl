#initWHERE.tcl
# all tk setup for WHERE axis display, including initializing 
# some default styles for nodes and edges

# $Log: initWHERE.tcl,v $
# Revision 1.13  1994/11/03 06:17:52  karavan
# Status display lines and where axis display pasted into the main window, and
# the look cleaned up some.
#
# Revision 1.12  1994/11/02  21:23:36  tamches
# changed centering of buttons at bottom from 30/70 center-based to
# 45/55 east/west based
#
# Revision 1.11  1994/11/02  21:19:32  tamches
# switch and clear buttons are now oriented toward the center of the window,
# not toward the left.  There is still a problem that shrinking the window
# vertically can hide these buttons -- that was present even before (problem
# seems to be not setting a min window size yet specifying a min canvas size)
#
# Revision 1.10  1994/11/02  19:46:03  tamches
# Fixed a bug in the where axis: now, making the window taller won't put
# unnecessary space above & below the "CLEAR" button area.
#
# Revision 1.9  1994/11/01  05:46:19  karavan
# changed resource selection to allow multiple focus selection on a
# single display.
#
# Revision 1.8  1994/10/25  17:55:10  karavan
# Implemented Resource Display Objects, which manage display of multiple
# resource Abstractions.
#
# Revision 1.7  1994/10/09  01:15:24  karavan
# Implemented new UIM/visithread interface with metrespair data structure
# and selection of resources directly on the where axis.
#
# Revision 1.6  1994/08/30  16:27:34  karavan
# added silent node trimming.
#
# Revision 1.5  1994/08/01  20:26:31  karavan
# changes to accommodate new dag design.
#
# Revision 1.4  1994/06/29  21:47:37  hollings
# killed old background colors and switched to motif like greys.
# cleaned up option specification to use options data base.
#
# Revision 1.3  1994/06/12  22:35:29  karavan
# changed default font for WHERE axis nodes
#
# Revision 1.2  1994/05/26  21:23:11  karavan
# changed parent window name.
#
# Revision 1.1  1994/05/23  01:56:23  karavan
# initial version.
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

proc hideCurrentSelection {whoCan dagID} {
    global currentSelection$dagID
    set nodeID [lindex [$whoCan gettags current] 0]
    if [string match {[nt]*} $nodeID] { 
	set newID [string range $nodeID 1 end]
	set currentSelection$dagID $newID
	uimpd hideSubgraph $dagID currentSelection$dagID
	return $newID
    }
}

# get nodeID for mouse click location and highlight the node
# clicking on a highlighted node unhighlights it
proc addWhereSelection {whoCan dagID} {
    set nodeID [lindex [$whoCan gettags current] 0]
    if [string match {[nt]*} $nodeID] { 
        set newID [string range $nodeID 1 end]
        uimpd highlightNode $newID $dagID
        return $newID
    }
}


proc addDefaultWhereSettings {cname token} {

    $cname bind all <1> "addWhereSelection $cname $token"
    $cname bind all <3> "hideCurrentSelection $cname $token"
    $cname bind all <Shift-3> "uimpd showAllNodes $token"

    uimpd addNStyle $token 1 "#d04b8b3edcab" black  \
			    "-*-Times-Bold-R-Normal--*-100*" black r 1.0

    uimpd addEStyle $token 1 0 #c99e5f54dcab b 2.0

}

# map a particular dag display into the parent resource display window
# this is done by packing/unpacking to save time; of course with lots 
# of dags this may get to be too much of a memory hog.

# need to redo prompt stuff now that this is split from initRDO
#	              set selectionPrompt \"$mprompt\"" \

proc mapRDOdag {rdoID dagID wwindow abs} {
    global currentSelection$dagID tclSelectionState
    set currentSelection$dagID -1    
#    puts "mapRDOdag $wwindow.dag.dag$abs"
    $wwindow.sbutts.1 configure  \
	    -command "uimpd clearResourceSelection dag $dagID" \
	    -state normal
    $wwindow.sbutts.2 configure \
	    -command "uimpd switchRDOdag $rdoID $abs" \
	    -state normal
    $wwindow.title configure -text "Paradyn $abs Where Axis"
    if $tclSelectionState {
	uimpd clearResourceSelection dag $dagID
    }
    pack $wwindow.dag.dag$abs -side top -fill both -expand 1
}

proc initRDOdag {wwindow abs} {
#    puts "initRDOdag $wwindow.dag.dag$abs"
    frame $wwindow.dag.dag$abs
}

proc unmapRDOdag {wwindow abs} {
#    puts "unmap $wwindow.dag.dag$abs"
    pack forget $wwindow.dag.dag$abs
}

proc initRDO {rdoID wwindow wtitle toplvlflag} {
    set mprompt "Select Visualization Metric(s) and press ACCEPT"
    set dtitle " "
#    puts "initRDO:  $rdoID"

if {$toplvlflag == 1} {
    toplevel $wwindow
    #allow interactive sizing
    wm minsize $wwindow 200 200      
    wm title $wwindow "$wtitle"
}
    frame $wwindow.dag -class Dag -geometry 200x100
    label $wwindow.title -text "  " -fg black \
	-font *-New*Century*Schoolbook-Bold-R-*-14-* \
	-relief raised

    # selection buttons enabled only at time of metric/resource selection
    frame $wwindow.sbutts
    mkButtonBar $wwindow.sbutts {} retval \
	    {{CLEAR ""} {SWITCH ""}}
    $wwindow.sbutts.1 configure -state disabled
    $wwindow.sbutts.2 configure -state disabled

    pack $wwindow.title -side top -fill x -expand 0 
    pack $wwindow.dag -side top -fill both -expand 1
    pack $wwindow.sbutts -side top -fill x -expand 0
}
