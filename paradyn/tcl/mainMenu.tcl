#
# main tool bar
# $Id: mainMenu.tcl,v 1.69 2004/03/20 20:44:49 pcroth Exp $
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


proc buildMainWindow {} {

    global fmap metMenuCtr

    # setup fontmap for dumping postscript files
#    set fmap(-*-Times-Bold-R-Normal--*-80*)  \
#	    {Times-Bold 10}
#    set fmap(-*-Times-Medium-R-Normal--*-100*) \
#	    {Times-Roman 12}
#    set fmap(-*-Times-Bold-R-Normal--*-140*)  \
#	    {Times-Bold 16}
#    set fmap(-*-Times-Medium-R-Normal--*-140*) \
#	    {Times-Roman 16}
#    set fmap(-*-Times-Medium-R-Normal--*-80*) \
#	    {Times-Roman 10}
#    set fmap(-*-Times-Bold-R-Normal--*-100*) \
#	    {Times-Bold 16}
#    set fmap(-*-Courier-Medium-R-Normal--*-100*) \
#	    {Courier 12}
#    set fmap(-*-Courier-Bold-R-Normal--*-100*) \
#	    {Courier-Bold 12}
#    set fmap(-*-Courier-Medium-R-Normal--*-80*) \
#	    {Courier-Medium 10}
#    set fmap(-*-Courier-Bold-R-Normal--*-80*) \
#	    {Courier-Bold 10}

    # used in metric menu creation code
    # unique id for each menu window
    set metMenuCtr 0 
    # state = 1 during met/res selection, 0 otherwise
    if {[winfo depth .] > 1} {
        # . created before options are added
        . config -bg #e830e830e830

        option add *background #e830e830e830 widgetDefault
        option add *Scrollbar*background DimGray widgetDefault

        option add *Scrollbar*foreground grey widgetDefault
        option add *activeBackground LightGrey widgetDefault
        option add *activeForeground black widgetDefault
        option add *Scrollbar*activeForeground LightGrey widgetDefault
        option add *Entry.relief sunken widgetDefault
    } else {
        option add *Background white widgetDefault
        option add *Foreground black widgetDefault
        option add *Entry.relief groove widgetDefault
    }
    # sadly, there is not enclosing frame for status and procstatus
    option add Paradyn*status.Font {Courier 9 roman} widgetDefault
    option add Paradyn*procstatus.Font {Courier 9 roman} widgetDefault

    option add Paradyn*listRootItemFont {Helvetica 10 bold roman} widgetDefault
    option add Paradyn*listRootItemEmphFont {Helvetica 10 bold italic} widgetDefault
    option add Paradyn*listItemFont {Helvetica 10 bold roman} widgetDefault
    option add Paradyn*listItemEmphFont {Helvetica 10 bold italic} widgetDefault

    # the Paradyn main window can be resized horizontally but not vertically:
####    wm resizable . 1 0
    # it can now!

    frame .parent
    frame .parent.menub -relief raised
    frame .parent.status
    frame .parent.procstatus -borderwidth 2 -relief sunken
    frame .parent.buttons -relief raised -height 20
    mkButtonBar .parent.buttons {} retval { \
	    {Run "paradyn cont"} \
	    {Pause "paradyn pause"} \
	    {Export "drawSaveMenu"} \
	    {Exit "procExit"}}

#   Both RUN and PAUSE buttons are disabled when paradyn starts running
#   since there is no process to RUN or to PAUSE
    .parent.buttons.1 configure -state disabled
    .parent.buttons.2 configure -state disabled

# menubar
#
    frame .parent.menub.left
    pack .parent.menub.left -side left -fill both -expand 1

    frame .parent.menub.left.top -borderwidth 1 -relief raised \
	  -background #b3331e1b53c7
    pack  .parent.menub.left.top -side top -fill both -expand 1

    label .parent.menub.left.top.title -text "Paradyn Main Control" \
          -relief flat \
	  -background #b3331e1b53c7 \
	  -foreground white -anchor c
    pack .parent.menub.left.top.title -side left -fill both -expand true

    frame .parent.menub.left.top.title.versionFrame -background #b3331e1b53c7
    pack  .parent.menub.left.top.title.versionFrame -side right -fill y -expand false
	    
    label .parent.menub.left.top.title.versionFrame.version \
            -text "v3.0" \
	    -background #b3331e1b53c7 \
	    -foreground white \
	    -relief flat \
	    -borderwidth 0
    pack .parent.menub.left.top.title.versionFrame.version -side bottom \
	    -expand false


#    frame .parent.menub.left.men -borderwidth 2 -relief raised
    frame .parent.menub.left.men -borderwidth 2 -relief raised
    menubutton .parent.menub.left.men.b0 -text "File" -menu .parent.menub.left.men.b0.m 
    menubutton .parent.menub.left.men.b1 -text "Setup" -menu .parent.menub.left.men.b1.m 

    menubutton .parent.menub.left.men.b6 -text "Phase" \
	    -menu .parent.menub.left.men.b6.m 

    button .parent.menub.left.men.b8 -text "Visi" \
	    -command "drawVisiMenu" -relief flat -highlightthickness 0

#   menubutton .parent.menub.left.men.b7 -text "Help" -state disabled
    menubutton .parent.menub.left.men.b7 -text "Help" \
            -menu .parent.menub.left.men.b7.m

    menu .parent.menub.left.men.b7.m
    .parent.menub.left.men.b7.m add command \
        -label "General info" -command "paradyn generalInfo"
    .parent.menub.left.men.b7.m add command \
        -label "License info" -command "paradyn licenseInfo"
    .parent.menub.left.men.b7.m add command \
        -label "Release info" -command "paradyn releaseInfo"
    .parent.menub.left.men.b7.m add command \
        -label "Version info" -command "paradyn versionInfo"

    menu .parent.menub.left.men.b0.m
    .parent.menub.left.men.b0.m add command \
        -label "Daemon start-up info" -command "paradyn daemonStartInfo"
    .parent.menub.left.men.b0.m add command \
        -label "Exit" -command "procExit"
       # the -command is the same as the command executed when "Exit"
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
    .parent.menub.left.men.b1.m add command -label "Call Graph" \
            -command {callGraphEntryPoint}

#
#  added to support phase specification
#
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
	    .parent.menub.left.men.b6 .parent.menub.left.men.b8 \
	    -side left -padx 10

    makeLogo .parent.menub.logobox paradynLogo raised 2 #b3331e1b53c7
    pack  .parent.menub.logobox -side right

    pack .parent.menub.left.men -side top -fill x -expand false

    pack .parent.menub -side top -fill x -expand false
    pack .parent.buttons -side bottom -fill x -expand false

    pack .parent.status -side top -fill x -expand false
####pack .parent.status -side bottom -fill both -expand true

    pack .parent.procstatus -side top -fill both -expand true

    pack .parent -fill both -expand true
}

