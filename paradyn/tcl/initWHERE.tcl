#initWHERE.tcl
# all tk setup for WHERE axis display, including initializing 
# some default styles for nodes and edges

# $Log: initWHERE.tcl,v $
# Revision 1.4  1994/06/29 21:47:37  hollings
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

proc initWHERE {wwindow} {

    global WHEREname
    set WHEREname $wwindow

    dag $WHEREname.d01 
    frame $WHEREname.buttons
    label $WHEREname.title -text "Where Axis" -fg black \
	-font "-Adobe-times-bold-r-normal--*-120*" \
	-relief raised

    pack $WHEREname.title \
	-side top -fill both
    pack $WHEREname.d01 -side top -expand 1 -fill both



    # style 1: WHERE axis node 
    $WHEREname.d01 addNstyle 1 -bg #c99e5f54dcab \
		-font "-Adobe-times-bold-r-normal--*-80*" \
		-text black  \
		-stipple "" -width 1

    $WHEREname.d01 addEstyle 1 -arrow none -fill #c99e5f54dcab -width 2
}

