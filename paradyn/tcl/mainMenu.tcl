# main tool bar

# $Log: mainMenu.tcl,v $
# Revision 1.43  1995/11/06 02:56:07  tamches
# removed ugly borderwidth for .parent.menub, .parent.status, and
# .parent.buttons
# removed .parent.where, which no longer exists
#
# Revision 1.42  1995/11/03 21:19:22  naim
# Changing exit option - naim
#
# Revision 1.41  1995/10/17  22:25:00  tamches
# "performance consultant" now calls shgInitialize instead of
# "paradyn shg start global".
# Added a "Where Axis" item to de-iconify the where axis.
#
# Revision 1.40  1995/10/06 19:50:57  naim
# Minor change to "changeApplicState". Now there are 3 states for the RUN and
# PAUSE keys: either RUN or PAUSE is enabled and the other one disabled, and
# both keys disabled. There is also a minor change to drawToolBar. Now, the
# first state for buttons PAUSE and RUN is disabled (when paradyn starts)-naim
#
# Revision 1.39  1995/10/05  04:19:23  karavan
# Added search phase to title bar of Perf Consultant window.
# Changed arguments to agree with new igen interfaces for UI and PC.
#
# Revision 1.38  1995/08/30  05:01:06  tamches
# The main window is no longer manually resizable (because it has
# no reason to)
#
# Revision 1.37  1995/08/01  02:18:59  newhall
# changes to support phase interface
#
# Revision 1.36  1995/07/19 23:03:00  tamches
# Commented out an entry widget binding which inserts the current
# selection when the middle button is pressed.  This feature is
# provided automatically in tk4.0, so the effect was to insert
# things twice.
#
# Revision 1.35  1995/07/18  19:14:58  krisna
# removed "source" command in mainMenu.tcl
# moved code for status lines to status.tcl
#
# Revision 1.34  1995/07/01 23:00:00  rbi
# updated colormodel handling to tk 4.0 and temporarily delayed need
# for rmc technology
#
# Revision 1.33  1994/11/09  22:35:29  karavan
# in which Karen gets all the resize functionality exactly the way we want
# it and concedes defeat on the appearance ofthe initial screen startup...
#
# Revision 1.32  1994/11/09  16:17:20  karavan
# back to original flicker solution; seems like this is the best we can do.
#
# Revision 1.31  1994/11/07  07:30:15  karavan
# changed pack order.
#
# Revision 1.30  1994/11/05  01:51:59  karavan
# small improvements to min window sizes, resizing effects, button names,
# and change pack command in mkLogo to new version.
#
# Revision 1.29  1994/11/04  21:03:49  karavan
# fixed pause button
#
# Revision 1.28  1994/11/04  20:10:02  karavan
# Sent flicker back to the sea where it belongs...
#
# Revision 1.27  1994/11/03  22:15:19  tamches
# change "Start Perf Consultant" to "Performance Consultant"
#
# Revision 1.26  1994/11/03  22:13:37  tamches
# changed "Exit" to "Exit Paradyn"
#
# Revision 1.25  1994/11/03  22:08:03  tamches
# Changed "Start Visual" to "Visualization"
#
# Revision 1.24  1994/11/03  21:04:24  tamches
# "Metrics" (blank) and "Options" (error history, where axis postscripts) menus
# commented out.  "File" menu (only option: Exit) added.
#
# Revision 1.23  1994/11/03  18:06:23  karavan
# Nasty useless obsolete REPORT button put to death
#
# Revision 1.22  1994/11/03  16:10:44  rbi
# New process definition interface.
#
# Revision 1.21  1994/11/03  06:17:56  karavan
# Status display lines and where axis display pasted into the main window, and
# the look cleaned up some.
#
# Revision 1.20  1994/11/03  00:04:27  karavan
# added frame for status line service.
#
# Revision 1.19  1994/11/01  05:49:15  karavan
# updated Where axis choices
#
# Revision 1.18  1994/10/26  22:54:38  tamches
# Added tunable constants menu item
#
# Revision 1.17  1994/10/09  01:15:26  karavan
# Implemented new UIM/visithread interface with metrespair data structure
# and selection of resources directly on the where axis.
#
# Revision 1.16  1994/09/13  05:07:03  karavan
# initialize new global: metMenuCtr
#
# Revision 1.15  1994/08/01  20:26:33  karavan
# changes to accommodate new dag design.
#
# Revision 1.14  1994/07/25  16:18:59  rbi
# Scrollbars are even more comely than before.
#
# Revision 1.13  1994/07/21  17:47:44  rbi
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
## of current state can always be pressed but current state cannot.

proc changeApplicState {newVal} {
    if {$newVal==1} {
	## PAUSE enabled, "run" disabled
	.parent.buttons.2 configure -state normal
	.parent.buttons.1 configure -state disabled
    } elseif {$newVal==0} {
	## "pause" disabled, RUN enabled 
	.parent.buttons.2 configure -state disabled
	.parent.buttons.1 configure -state normal
    } elseif {$newVal==2} {
	## PAUSE and RUN disabled 
	.parent.buttons.2 configure -state disabled
	.parent.buttons.1 configure -state disabled
    }
}

proc drawToolBar {} {

    global PdBitmapDir fmap metMenuCtr tclSelectionState

    # setup fontmap for dumping postscript files
    set fmap(-*-Times-Bold-R-Normal--*-80*)  \
	    {Times-Bold 10}
    set fmap(-*-Times-Medium-R-Normal--*-100*) \
	    {Times-Roman 12}
    set fmap(-*-Times-Bold-R-Normal--*-140*)  \
	    {Times-Bold 16}
    set fmap(-*-Times-Medium-R-Normal--*-140*) \
	    {Times-Roman 16}
    set fmap(-*-Times-Medium-R-Normal--*-80*) \
	    {Times-Roman 10}
    set fmap(-*-Times-Bold-R-Normal--*-100*) \
	    {Times-Bold 16}
    set fmap(-*-Courier-Medium-R-Normal--*-100*) \
	    {Courier 12}
    set fmap(-*-Courier-Bold-R-Normal--*-100*) \
	    {Courier-Bold 12}
    set fmap(-*-Courier-Medium-R-Normal--*-80*) \
	    {Courier-Medium 10}
    set fmap(-*-Courier-Bold-R-Normal--*-80*) \
	    {Courier-Bold 10}

    # used in metric menu creation code
    # unique id for each menu window
    set metMenuCtr 0 
    # state = 1 during met/res selection, 0 otherwise
    set tclSelectionState 0 
    if {[winfo depth .] > 1} {
      # . created before options are added
      . config -bg #e830e830e830

      option add *background #e830e830e830
      option add *Scrollbar*background DimGray

      option add *Scrollbar*foreground grey
      option add *activeBackground LightGrey
      option add *activeForeground black
      option add *Scrollbar*activeForeground LightGrey
      option add *Entry.relief sunken
    } else {
      option add *Background white
      option add *Foreground black
      option add *Entry.relief groove
    }
    option add *TopMenu*font *-New*Century*Schoolbook-Bold-R-*-14-*

#    bind Entry <2> { %W insert insert [selection get] }

#    wm minsize . 400 300
    wm resizable . 1 0

    frame .parent 
#    frame .parent.menub -relief raised -borderwidth 2
    frame .parent.menub -relief raised
#    frame .parent.status  -relief raised -borderwidth 4
    frame .parent.status
    frame .parent.main
#    frame .parent.buttons -relief raised -borderwidth 4 -height 20
    frame .parent.buttons -relief raised -height 20
    mkButtonBar .parent.buttons {} retval {{RUN "paradyn cont"} \
	    {PAUSE "paradyn pause"} {SAVE ""} \
	    {EXIT "paradyn exit"}}

#   Both RUN and PAUSE buttons are disabled when paradyn starts running
#   since there is no process to RUN or to PAUSE
    .parent.buttons.1 configure -state disabled
    .parent.buttons.2 configure -state disabled

    frame .parent.menub.left
    label .parent.menub.left.title -text "Paradyn Main Control" \
          -font *-New*Century*Schoolbook-Bold-R-*-14-* \
          -relief raised -background #b3331e1b53c7 -foreground white -anchor c

    frame .parent.menub.left.men -class TopMenu -borderwidth 2 -relief raised
    menubutton .parent.menub.left.men.b0 -text "File" -menu .parent.menub.left.men.b0.m 
    menubutton .parent.menub.left.men.b1 -text "Setup" -menu .parent.menub.left.men.b1.m 

#     menubutton .parent.menub.left.men.b2 -text "Visi" -menu .parent.menub.left.men.b2.m
#    menubutton .parent.menub.left.men.b3 -text "Metrics"
#    menubutton .parent.menub.left.men.b2 -text "Options" -menu .menub.left.men.b2.m
    menubutton .parent.menub.left.men.b5 -text "Visualization" \
	    -menu .parent.menub.left.men.b5.m 

    menubutton .parent.menub.left.men.b6 -text "Help" 

#    menu .parent.menub.left.men.b5.m -postcommand \
#	    {uimpd drawStartVisiMenu .parent.menub.left.men.b5.m}

    menu .parent.menub.left.men.b0.m
    .parent.menub.left.men.b0.m add command -label "Exit Paradyn" -command "destroy ."
       # the -command is the same as the command executed when "EXIT"
       # button (lower right of screen) is clicked on.  If this is not right,
       # then by all means change it.
 
    menu .parent.menub.left.men.b1.m 
    .parent.menub.left.men.b1.m add command -label "Define A Process" \
	    -command DefineProcess
    .parent.menub.left.men.b1.m add command -label "Performance Consultant" \
	    -command {set shgHack 1; shgInitialize}
#	    -command {paradyn shg start global}
    .parent.menub.left.men.b1.m add command -label "Tunable Constants" \
            -command {tunableEntryPoint}
    .parent.menub.left.men.b1.m add command -label "Where Axis" \
	    -command {wm deiconify .whereAxis; raise .whereAxis}
    .parent.menub.left.men.b1.m add command -label "Options Control" \
	    -state disabled
#
#  added to support phase specification
#
   menu .parent.menub.left.men.b5.m
   .parent.menub.left.men.b5.m add command -label "Start A Visualization" \
  -command  {uimpd drawStartVisiMenu .parent.menub.left.men.b5.m} 


#    menu .parent.menub.left.men.b2.m 
#    .parent.menub.left.men.b2.m add command -label "Error History" \
#	    -command {showErrorHistory}
#    .parent.menub.left.men.b2.m add command -label "Where Axis Postscript C" \
#	    -command ".parent.baseWA.dag._c_ postscript -colormode color \
#	    -file cwhere.ps -pageheight 3.0i"
#    .menub.left.men.b2.m add command -label "Where Axis Postscript Gr" \
#	    -command ".parent.baseWA.dag._c_ postscript -colormode gray \
#	    -file gwhere.ps -pageheight 3.0i"
#    .menub.left.men.b2.m add command -label "Where Axis Postscript BW" \
#	    -command ".parent.baseWA.dag._c_ postscript -colormode mono \
#	    -file mwhere.ps -pageheight 3.0i"

    set mb .parent.menub.left.men
#    tk_menuBar $mb $mb.b1 $mb.b2 $mb.b3 $mb.b4 $mb.b5 $mb.b6
    tk_menuBar $mb $mb.b0 $mb.b1 $mb.b5 $mb.b6

    wm title . "Paradyn"

    pack .parent.menub.left.men.b6 -side right -padx 10
#    pack .parent.menub.left.men.b1 .parent.menub.left.men.b3 \
#    .parent.menub.left.men.b2 .parent.menub.left.men.b5 \
#	    -side left -padx 10
    pack .parent.menub.left.men.b0 .parent.menub.left.men.b1 \
	    .parent.menub.left.men.b5 \
	    -side left -padx 10
    pack .parent.menub.left -side left -fill both -expand 1
    mkLogo .parent.menub.logobox right

#    pack .parent.menub.left.title .parent.menub.left.men -side top -fill both -expand 1
    pack .parent.menub.left.title -side top -fill both -expand true
    pack .parent.menub.left.men -side top -fill x -expand false

    pack .parent.menub -side top -fill x -expand 0
    pack .parent.buttons -side bottom -fill x -expand 0
    pack .parent.status -side bottom -fill x -expand 0

    pack .parent -fill both -expand 1

    # read in error file

#
# not needed any more since errorList will be converted and sourced
#
#   uplevel #0 {source "$PdBitmapDir/errorList.tcl"}
}

