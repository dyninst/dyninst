# main tool bar

# $Log: mainMenu.tcl,v $
# Revision 1.13  1994/07/21 17:47:44  rbi
# No more jumpy resizes.
#
# Revision 1.12  1994/07/21  01:53:32  rbi
# YANS -- Yet another new style
#
# Revision 1.11  1994/07/20  18:16:50  rbi
# Cut and Paste for Entries (Yahoo!) and better BW support
#
# Revision 1.10  1994/07/07  18:17:25  karavan
# bug fix:  menu name specification error
#
# Revision 1.9  1994/07/07  05:57:05  karavan
# UIM error service implementation
#
# CVr: ----------------------------------------------------------------------
#
# Revision 1.8  1994/06/29  21:47:38  hollings
# killed old background colors and switched to motif like greys.
# cleaned up option specification to use options data base.
#
# Revision 1.7  1994/06/12  22:32:44  karavan
# implemented button update by status change callback.
#
# Revision 1.6  1994/05/30  19:22:26  hollings
# Removed debugging puts.
#
# Revision 1.5  1994/05/26  20:56:36  karavan
# changed location of bitmap file for logo.
#
# Revision 1.4  1994/05/23  01:55:41  karavan
# its a whole new look for paradyn!
#
# Revision 1.3  1994/05/06  06:42:13  karavan
# buttons now functional: Performance Consultant; RUN/PAUSE; Status
#
# Revision 1.2  1994/05/05  19:51:24  karavan
# added call to uimpd command for visi menu.
#
# Revision 1.1  1994/05/03  06:36:02  karavan
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

## changeApplicState
## changes button status of "run" and "pause" buttons, so that opposite 
##  of current state can always be pressed but current state cannot.

proc changeApplicState {newVal} {
    if {$newVal} {
	.buttons.2 configure -state normal
	.buttons.1 configure -state disabled
    } else {
	.buttons.2 configure -state disabled
	.buttons.1 configure -state normal
    }
}

proc drawToolBar {} {

    global PdBitmapDir

    if {[string match [tk colormodel .] color] == 1} {
      # . seems to be created before the options data base gets loaded.
      . config -bg grey

      option add *background grey
      option add *Scrollbar*background DimGray

      option add *Scrollbar*foreground grey
      option add *activeBackground LightGrey
      option add *activeForeground black
      option add *Scrollbar*activeBackground DimGray
      option add *Entry.relief sunken
    } else {
      option add *Background white
      option add *Foreground black
      option add *Entry.relief groove
    }
    option add *TopMenu*font *-New*Century*Schoolbook-Bold-R-*-14-*

    bind Entry <2> { %W insert insert [selection get] }

#    wm geometry . 750x375
    wm minsize . 517 400

    frame .menub -relief raised -borderwidth 2
    frame .where -height 312
    frame .main -height 0 -width 0
    frame .buttons
    mkButtonBar .buttons {} retval {{RUN "paradyn cont"} \
	    {PAUSE "paradyn pause"} {REPORT "paradyn status"} {SAVE ""} \
	    {EXIT "destroy ."}}
    .buttons.2 configure -state disabled

    frame .menub.left
    label .menub.left.title -text "Paradyn Main Control" \
          -font *-New*Century*Schoolbook-Bold-R-*-18-* \
          -relief raised -background #b3331e1b53c7 -foreground white

    frame .menub.left.men -class TopMenu -borderwidth 2 -relief raised
    menubutton .menub.left.men.b1 -text "Setup" -menu .menub.left.men.b1.m 
    menubutton .menub.left.men.b3 -text "Metrics"
    menubutton .menub.left.men.b2 -text "Options"
    menubutton .menub.left.men.b5 -text "Start Visual" \
	    -menu .menub.left.men.b5.m 
    menubutton .menub.left.men.b6 -text "Help" 
    menu .menub.left.men.b5.m -postcommand \
	    {uimpd drawStartVisiMenu .menub.left.men.b5.m}
    menu .menub.left.men.b1.m 
    .menub.left.men.b1.m add command -label "Application Control" \
	    -command ApplicDefn
    .menub.left.men.b1.m add command -label "Start Perf Consultant" \
	    -command {paradyn shg start}
    .menub.left.men.b1.m add command -label "Options Control" \
	    -state disabled
    menu .menub.left.men.b2.m 
    .menub.left.men.b2.m add command -label "Error History" \
	    -command {showErrorHistory}

    set mb .menub.left.men
    tk_menuBar $mb $mb.b1 $mb.b2 $mb.b3 $mb.b4 $mb.b5 $mb.b6

    wm title . "Paradyn"

    pack .menub -side top -fill x
    pack .where -side top -fill both -expand 1
    pack .buttons -side bottom -fill x 

    pack .menub.left.men.b6 -side right -padx 10
    pack .menub.left.men.b1 .menub.left.men.b3 .menub.left.men.b2 \
	    .menub.left.men.b5 \
	    -side left -padx 10
    pack .menub.left -side left -fill both -expand 1
    mkLogo .menub.logobox right

    pack .menub.left.title .menub.left.men -side top -fill both -expand 1

    # read in error file

    uplevel #0 {source "$PdBitmapDir/errorList.tcl"}
    
    InitApplicDefnScreen 

}


