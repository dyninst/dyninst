# main tool bar

#
# $Log: mainMenu.tcl,v $
# Revision 1.57  1997/01/15 00:15:48  tamches
# added an attach menu item
#
# Revision 1.56  1996/10/31 08:23:33  tamches
# v1.1 --> v1.2 internal
#
# Revision 1.55  1996/09/04 21:22:17  tamches
# added version # on screen
#
# Revision 1.54  1996/05/06 16:41:44  naim
# Adding window to confirm whether the user wants to exit paradyn - naim
#
# Revision 1.53  1996/04/01  22:34:56  tamches
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
	    {EXIT "procExit"}}

#   Both RUN and PAUSE buttons are disabled when paradyn starts running
#   since there is no process to RUN or to PAUSE
    .parent.buttons.1 configure -state disabled
    .parent.buttons.2 configure -state disabled

#   SAVE button will be disabled for the time being since it is not 
#   implemented yet.
    .parent.buttons.3 configure -state disabled

    frame .parent.menub.left
    pack .parent.menub.left -side left -fill both -expand 1

    frame .parent.menub.left.top -borderwidth 2 -relief raised \
	  -background #b3331e1b53c7
    pack  .parent.menub.left.top -side top -fill both -expand 1

    label .parent.menub.left.top.title -text "Paradyn Main Control" \
          -font *-New*Century*Schoolbook-Bold-R-*-14-* \
          -relief flat \
	  -background #b3331e1b53c7 \
	  -foreground white -anchor c
    pack .parent.menub.left.top.title -side left -fill both -expand true

    frame .parent.menub.left.top.title.versionFrame -background #b3331e1b53c7
    pack  .parent.menub.left.top.title.versionFrame -side right -fill y -expand false
	    
    label .parent.menub.left.top.title.versionFrame.version -text "v1.2 internal" \
	    -font "*-Helvetica-*-r-*-12-*" \
	    -background #b3331e1b53c7 \
	    -foreground white \
	    -relief flat \
	    -borderwidth 0
    pack .parent.menub.left.top.title.versionFrame.version -side bottom \
	    -expand false


    frame .parent.menub.left.men -class TopMenu -borderwidth 2 -relief raised
    menubutton .parent.menub.left.men.b0 -text "File" -menu .parent.menub.left.men.b0.m 
    menubutton .parent.menub.left.men.b1 -text "Setup" -menu .parent.menub.left.men.b1.m 

    menubutton .parent.menub.left.men.b5 -text "Visualization" \
	    -menu .parent.menub.left.men.b5.m 
    menubutton .parent.menub.left.men.b6 -text "Phase" \
	    -menu .parent.menub.left.men.b6.m 

    menubutton .parent.menub.left.men.b7 -text "Help" -state disabled

    menu .parent.menub.left.men.b0.m
    .parent.menub.left.men.b0.m add command -label "Exit Paradyn" -command "procExit"
       # the -command is the same as the command executed when "EXIT"
       # button (lower right of screen) is clicked on.  If this is not right,
       # then by all means change it.
 
    menu .parent.menub.left.men.b1.m 
    .parent.menub.left.men.b1.m add command -label "Define a New Process" \
	    -command DefineProcess
    .parent.menub.left.men.b1.m add command -label "Attach to a Process" \
	    -command AttachProcess
    .parent.menub.left.men.b1.m add separator
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

    makeLogo .parent.menub.logobox paradynLogo raised 2 #b3331e1b53c7
    pack  .parent.menub.logobox -side right

    pack .parent.menub.left.men -side top -fill x -expand false

    pack .parent.menub -side top -fill x -expand 0
    pack .parent.buttons -side bottom -fill x -expand 0
    pack .parent.status -side bottom -fill x -expand 0

    pack .parent -fill both -expand 1
}
