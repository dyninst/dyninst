#initWHERE.tcl
# all tk setup for WHERE axis display, including initializing 
# some default styles for nodes and edges

# $Log: initWHERE.tcl,v $
# Revision 1.5  1994/08/01 20:26:31  karavan
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

proc initWHERE {dagID wwindow wtitle} {
    toplevel $wwindow
    #allow interactive sizing
    wm minsize $wwindow 200 200      

    frame $wwindow.buttons -geometry 200x20
    label $wwindow.title -text $wtitle -fg black \
	-font *-New*Century*Schoolbook-Bold-R-*-14-* \
	-relief raised
    frame $wwindow.dag -class Dag -geometry 200x100

    pack $wwindow.title -side top -fill x -expand 1 
    pack $wwindow.dag -side top -fill both -expand 1

    button $wwindow.buttons.b1 -text "CLOSE" -width 10 -height 1 \
	    -command "uimpd closeDAG $dagID; destroy $wwindow"
    pack $wwindow.buttons -side top -expand 1 -fill both
    pack $wwindow.buttons.b1 -side left  -padx 20 -pady 1

}

