# main tool bar

# $Log: mainMenu.tcl,v $
# Revision 1.53  1996/04/01 22:34:56  tamches
# removed tclSelectionState
#
# Revision 1.52  1996/02/12 18:31:35  tamches
# shgInitialize now takes 3 params
#
# Revision 1.51  1996/02/08 01:01:49  tamches
# starting a new phase w/ visis greyed out since not yet implemented
#
# Revision 1.50  1996/02/08 00:01:44  tamches
# Dimmed the Help menu, since it's not yet implemented
#
# Revision 1.49  1996/02/07 19:16:14  tamches
# added new phase menu
#
# Revision 1.48  1996/01/11 23:43:27  tamches
# shgInitialize now takes a parameter
#
# Revision 1.47  1995/12/08 16:08:07  naim
# Disabling SAVE button because it is not implemented yet - naim
#
# Revision 1.46  1995/11/29  00:21:30  tamches
# removed refs to pdBitmapDir; we now call makeLogo (pdLogo.C)
#
# Revision 1.45  1995/11/16 00:46:40  tamches
# removed obsolete menu item "options control"
#
# Revision 1.44  1995/11/09 02:13:27  tamches
# some general cleanup, such as removing references to tk_menuBar
# (obsolete in tk4.0), and removing old code that had until now been
# commented out.
#
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
    global fmap metMenuCtr

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

    # the paradyn main window can be resized horizontally but not vertically:
    wm resizable . 1 0

    frame .parent 
    frame .parent.menub -relief raised
    frame .parent.status
    frame .parent.main
    frame .parent.buttons -relief raised -height 20
    mkButtonBar .parent.buttons {} retval {{RUN "paradyn cont"} \
	    {PAUSE "paradyn pause"} {SAVE ""} \
	    {EXIT "paradyn exit"}}

#   Both RUN and PAUSE buttons are disabled when paradyn starts running
#   since there is no process to RUN or to PAUSE
    .parent.buttons.1 configure -state disabled
    .parent.buttons.2 configure -state disabled

#   SAVE button will be disabled for the time being since it is not 
#   implemented yet.
    .parent.buttons.3 configure -state disabled

    frame .parent.menub.left
    label .parent.menub.left.title -text "Paradyn Main Control" \
          -font *-New*Century*Schoolbook-Bold-R-*-14-* \
          -relief raised -background #b3331e1b53c7 -foreground white -anchor c

    frame .parent.menub.left.men -class TopMenu -borderwidth 2 -relief raised
    menubutton .parent.menub.left.men.b0 -text "File" -menu .parent.menub.left.men.b0.m 
    menubutton .parent.menub.left.men.b1 -text "Setup" -menu .parent.menub.left.men.b1.m 

    menubutton .parent.menub.left.men.b5 -text "Visualization" \
	    -menu .parent.menub.left.men.b5.m 
    menubutton .parent.menub.left.men.b6 -text "Phase" \
	    -menu .parent.menub.left.men.b6.m 

    menubutton .parent.menub.left.men.b7 -text "Help" -state disabled

    menu .parent.menub.left.men.b0.m
    .parent.menub.left.men.b0.m add command -label "Exit Paradyn" -command "destroy ."
       # the -command is the same as the command executed when "EXIT"
       # button (lower right of screen) is clicked on.  If this is not right,
       # then by all means change it.
 
    menu .parent.menub.left.men.b1.m 
    .parent.menub.left.men.b1.m add command -label "Define A Process" \
	    -command DefineProcess
    .parent.menub.left.men.b1.m add command -label "Performance Consultant" \
	    -command {shgInitialize [uimpd tclTunable getvaluebyname developerMode] [uimpd tclTunable getvaluebyname showShgKey] [uimpd tclTunable getvaluebyname showShgTips]}
    .parent.menub.left.men.b1.m add command -label "Tunable Constants" \
            -command {tunableEntryPoint}
    .parent.menub.left.men.b1.m add command -label "Where Axis" \
	    -command {wm deiconify .whereAxis; raise .whereAxis}

#
#  added to support phase specification
#
   menu .parent.menub.left.men.b5.m
   .parent.menub.left.men.b5.m add command -label "Start A Visualization" \
  -command  {uimpd drawStartVisiMenu .parent.menub.left.men.b5.m} 

   menu .parent.menub.left.men.b6.m
   .parent.menub.left.men.b6.m add command -label "Start" \
	   -command "uimpd startPhase plain"
   .parent.menub.left.men.b6.m add command -label "Start With Perf Consultant" \
	   -command "uimpd startPhase pc"
   .parent.menub.left.men.b6.m add command -label "Start With Visis" \
	   -command "uimpd startPhase visis" -state disabled
   .parent.menub.left.men.b6.m add command -label "Start With Perf Consultant & Visis" \
	   -command "uimpd startPhase both" -state disabled


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

    wm title . "Paradyn"

    pack .parent.menub.left.men.b7 -side right -padx 10
    pack .parent.menub.left.men.b0 .parent.menub.left.men.b1 \
	    .parent.menub.left.men.b5 .parent.menub.left.men.b6 \
	    -side left -padx 10
    pack .parent.menub.left -side left -fill both -expand 1

    makeLogo .parent.menub.logobox paradynLogo raised 2 #b3331e1b53c7
    pack  .parent.menub.logobox -side right

    pack .parent.menub.left.title -side top -fill both -expand true
    pack .parent.menub.left.men -side top -fill x -expand false

    pack .parent.menub -side top -fill x -expand 0
    pack .parent.buttons -side bottom -fill x -expand 0
    pack .parent.status -side bottom -fill x -expand 0

    pack .parent -fill both -expand 1
}
