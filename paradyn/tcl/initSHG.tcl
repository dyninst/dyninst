#initSHG.tcl
# all tk setup for search history graph display, including initializing 
# some default styles for dag nodes and edges

# $Log: initSHG.tcl,v $
# Revision 1.11  1994/11/07 05:41:50  karavan
# set minimum size for window to retain all pieces.
#
# Revision 1.10  1994/11/05  01:51:57  karavan
# small improvements to min window sizes, resizing effects, button names,
# and change pack command in mkLogo to new version.
#
# Revision 1.9  1994/11/01  05:47:44  karavan
# eliminated tcl global SHGname; combined initbindings and styles into single
# procedure since always called together; changed fonts.
#
# Revision 1.8  1994/09/20  21:29:33  jcargill
# added procedure addDefaultShgBindings (really karavan, but she's not here...)
#
# Revision 1.7  1994/09/05  20:09:18  karavan
# small visual fixes
#
# Revision 1.6  1994/08/01  20:26:30  karavan
# changes to accommodate new dag design.
#
# Revision 1.5  1994/06/29  21:47:35  hollings
# killed old background colors and switched to motif like greys.
# cleaned up option specification to use options data base.
#
# Revision 1.4  1994/06/12  22:33:59  karavan
# added status display to Perf Consultant screen
#
# Revision 1.3  1994/05/07  23:25:21  karavan
# added short explanation feature
#
# Revision 1.2  1994/05/06  06:41:04  karavan
# changed buttons to AUTO SEARCH/REFINE/PAUSE-RUN
#
# Revision 1.1  1994/05/03  06:36:00  karavan
# Initial version.
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

proc SHGpause {butt} {
    global PCsearchState
    if {$PCsearchState == 1} {
	set PCsearchState 0
	$butt configure -text "RESUME"
	paradyn search pause
    } else {
	set PCsearchState 1
	$butt configure -text "PAUSE"
	paradyn search true -1
    }
}

proc shgFullName {whoCan dagID} {
    set nodeID [updateCurrentSelection $whoCan $dagID]
    uimpd shgShortExplain $nodeID  
}

proc shgUpdateStatusLine {w newItem} {
    if {[winfo exists $w] == 1} {
	$w.status.txt insert end $newItem
	$w.status.txt yview -pickplace end
    } else {
	puts "shgUpdateStatusLine::NO SUCH WINDOW - $w !!"
    }
}

proc addDefaultShgBindingsAndStyles {cname dagID} {
    $cname bind all <2> "shgFullName $cname $dagID"
    $cname bind all <1> "updateCurrentSelection $cname $dagID"
    $cname bind all <3> "hideCurrentSelection $cname $dagID"

  # style 1: not tested
    uimpd addNStyle $dagID 1 "#e9fbb57aa3c9" DarkSlateGrey  \
	    "-*-Times-Bold-R-Normal--*-100*" black r 1.0

  # style 2: not active
    uimpd addNStyle $dagID 2 "#cc85d5c2777d" DarkSlateGrey  \
	    "-*-Times-Bold-R-Normal--*-80*" black r 1.0

  # style 3: active and true
    uimpd addNStyle $dagID 3 "#acbff48ff6c8" SlateGrey  \
	            "-*-Times-Bold-R-Normal--*-100*" black r 1.0

  # style 4: active and false
    uimpd addNStyle $dagID 4 "#ffffbba5bba5" DarkSlateGrey  \
		"-*-Times-Bold-R-Normal--*-80*" black r 1.0

  # edge 1: where axis refinement
    uimpd addEStyle $dagID 1 0 "#c99e5f54dcab" b 2.0

  # edge 2: why axis refinement
    uimpd addEStyle $dagID 2 0 "#622261fcab01" b 2.0

  # edge 3: when axis refinement
    uimpd addEStyle $dagID 3 0 black b 2.0
}

proc initSHG {SHGname dagID} {

    global PCsearchState shgExplainStr currentSelection$dagID

    set PCsearchState 1
    set shgExplainStr ""
    set csvar \$currentSelection$dagID
    set currentSelection$dagID -1      
    set clrSHGQUITBUTTbg "#fb63e620d36b"
    set clrSHGSTEPBUTTbg "#fb63e620d36b"
    set clrSHGAUTOBUTTbg "#fb63e620d36b"
    set clrSHGPAUSEBUTTbg "#fb63e620d36b"

    toplevel $SHGname -class "Paradyn.Shg" -geometry 600x400 
    option add *Shg*background #fb63e620d36b
    wm minsize $SHGname 650 500
    frame $SHGname.dag -class Dag 
    frame $SHGname.buttons -relief raised -borderwidth 4

    button $SHGname.buttons.b1 -text "REFINE" -width 12 \
	    -command "uimpd refineSHG $csvar"
    button $SHGname.buttons.b2 -text "SEARCH" -width 12 \
	    -command {paradyn search true -1}
    button $SHGname.buttons.b3 -text "PAUSE" -width 12 \
	    -command "SHGpause $SHGname.buttons.b3"   
    button $SHGname.buttons.b4 -text "EXIT PC" -width 12 \
	    -command "uimpd closeDAG $dagID; destroy $SHGname"

    frame $SHGname.topbar
    frame $SHGname.topbar.r
    label $SHGname.topbar.r.title -text "The Performance Consultant" \
	    -fg white \
	    -font *-New*Century*Schoolbook-Bold-R-*-14-* \
	    -relief raised -width 80 \
	    -bg "#4cc6c43dc7ef"
  ## Performance Consultant Menu
    frame $SHGname.topbar.r.menu -class TopMenu -relief raised \
	    -borderwidth 2
    menubutton $SHGname.topbar.r.menu.b1 -text "Search_Display" \
	    -menu $SHGname.topbar.r.menu.b1.m -underline 0
    menubutton $SHGname.topbar.r.menu.b2 -text "Help" -underline 0
    tk_menuBar $SHGname.topbar.r.menu $SHGname.topbar.r.menu.b1 \
	    $SHGname.topbar.r.menu.b2
    tk_bindForTraversal $SHGname.topbar.r.menu
    menu $SHGname.topbar.r.menu.b1.m 
    $SHGname.topbar.r.menu.b1.m add command -label "Show Only Active Nodes" \
	    -underline 5
    $SHGname.topbar.r.menu.b1.m add command -label "Show All Nodes" \
	     -underline 5 \
	     -command "uimpd showAllNodes $dagID"
    $SHGname.topbar.r.menu.b1.m add command -label "Hide Subgraph" \
	    -underline 5 \
	    -command "uimpd hideSubgraph $dagID currentSelection$dagID"
    mkLogo $SHGname.topbar.logo right
    label $SHGname.explain -textvariable shgExplainStr -fg black \
	    -relief raised -width 80

    frame $SHGname.status
    text $SHGname.status.txt -borderwidth 2 -height 6 -relief sunken \
	    -yscrollcommand "$SHGname.status.vs set"
    scrollbar $SHGname.status.vs -relief sunken -command \
	    "$SHGname.status.txt yview"

    pack $SHGname.topbar -side top -fill both -expand 0
    pack $SHGname.topbar.r.title -side top -fill both -expand 1
    pack $SHGname.topbar.r.menu.b2  -side right \
	    -padx 10
    pack $SHGname.topbar.r.menu.b1 -side left -padx 10
    pack $SHGname.topbar.r.menu -side top -fill both -expand 1

    pack $SHGname.topbar.r -side right -fill both -expand 1
    pack $SHGname.status.vs -side right -fill y
    pack $SHGname.status.txt -side left -fill both -expand yes
    pack $SHGname.explain $SHGname.status -side top -fill both
    
    pack $SHGname.dag -side top -expand 1 -fill both
    pack $SHGname.buttons.b1 $SHGname.buttons.b2 $SHGname.buttons.b3 \
	    $SHGname.buttons.b4 -side left -expand 1 -padx 20 -pady 4
    pack $SHGname.buttons -side bottom -expand 0 -fill x
    wm title $SHGname "Perf. Consultant"

}

