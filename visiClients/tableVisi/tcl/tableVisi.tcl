#
# $Id: tableVisi.tcl,v 1.10 2001/11/07 05:03:22 darnold Exp $
#

proc initializeTableVisi {} {
   if {[winfo depth .] > 1} {
   #  . config -bg grey
     option add *Background grey
     option add *activeBackground LightGrey
     option add *activeForeground black
     option add *Scale.activeForeground grey
   } else {
     option add *Background white
     option add *Foreground black
   }
   
   option add *Visi*font *-New*Century*Schoolbook-Bold-R-*-18-*
   option add *Data*font *-Helvetica-*-r-*-12-* 
   option add *MyMenu*font *-New*Century*Schoolbook-Bold-R-*-14-*
   
   # Create the title bar, menu bar, and logo at the top:
   frame .top
   pack .top -side top -fill x
   
   frame .top.left 
   pack .top.left -side left -fill both -expand true
   
   label .top.left.title -relief raised -text "Table Visualization" \
         -font *-New*Century*Schoolbook-Bold-R-*-14-* \
         -foreground white -background HotPink2
   pack .top.left.title -side top -fill both -expand true
   
   # Menus:
   
   frame .top.left.menubar -class MyMenu -borderwidth 2 -relief raised
   pack .top.left.menubar -side top -fill x
   
   menubutton .top.left.menubar.file -text "File" -menu .top.left.menubar.file.m
   menu .top.left.menubar.file.m -selectcolor #6495ED
   .top.left.menubar.file.m add command -label "Save ..." -command {ExportHandler}
   .top.left.menubar.file.m add command -label "Close Table" -command {GracefulShutdown}
   
   menubutton .top.left.menubar.acts -text "Actions" -menu .top.left.menubar.acts.m
   menu .top.left.menubar.acts.m  -selectcolor #6495ED
   .top.left.menubar.acts.m add command -label "Add Entries" -command AddEntry
   .top.left.menubar.acts.m add command -label "Delete Selected Entry" -command DelEntry -state disabled
   
   menubutton .top.left.menubar.opts -text "View" -menu .top.left.menubar.opts.m
   menu .top.left.menubar.opts.m  -selectcolor #6495ED

   global LongNames
   set LongNames 1
   .top.left.menubar.opts.m add check -label "Long Names" -variable LongNames -command {updateNames $LongNames}
   .top.left.menubar.opts.m add separator

   global DataFormat
   set DataFormat 0
   .top.left.menubar.opts.m add radio -label "Current Values" \
	   -variable DataFormat \
	   -value 0 -command formatChanged
   .top.left.menubar.opts.m add radio -label "Average Values" \
	   -variable DataFormat \
	   -value 1 -command formatChanged
   .top.left.menubar.opts.m add radio -label "Total Values" \
	   -variable DataFormat \
	   -value 2 -command formatChanged
   .top.left.menubar.opts.m add separator
   global sortMetrics
   set sortMetrics 0
   .top.left.menubar.opts.m add radio -label "Don't Sort Metrics" \
	   -variable sortMetrics -value 0 -command unsortMetrics
   .top.left.menubar.opts.m add radio -label "Sort Metrics (ascending)" \
	   -variable sortMetrics -value 1 -command sortMetrics
   .top.left.menubar.opts.m add separator
   global sortFoci
   set sortFoci 0
   .top.left.menubar.opts.m add radio -label "Don't Sort Foci" \
	   -variable sortFoci -value 0 -command unsortFoci
   .top.left.menubar.opts.m add radio -label "Sort Foci (ascending)" \
	   -variable sortFoci -value 1 -command sortFociByValues
   .top.left.menubar.opts.m add radio -label "Sort Foci By Values (of selected metric)" \
	   -variable sortFoci -value 2 -command sortFociByValues
   
   # significant digits at the end of the View menu (?)
   global SignificantDigits
   set SignificantDigits 3

   .top.left.menubar.opts.m add separator
   .top.left.menubar.opts.m add radio -label "1 significant digit" \
	   -variable SignificantDigits -value 1 -command "sigFigChange "
   .top.left.menubar.opts.m add radio -label "2 significant digits" \
	   -variable SignificantDigits -value 2 -command "sigFigChange "
   .top.left.menubar.opts.m add radio -label "3 significant digits" \
	   -variable SignificantDigits -value 3 -command "sigFigChange "
   .top.left.menubar.opts.m add radio -label "4 significant digits" \
	   -variable SignificantDigits -value 4 -command "sigFigChange "
   .top.left.menubar.opts.m add radio -label "5 significant digits" \
	   -variable SignificantDigits -value 5 -command "sigFigChange "
   .top.left.menubar.opts.m add radio -label "6 significant digits" \
	   -variable SignificantDigits -value 6 -command "sigFigChange "
   .top.left.menubar.opts.m add radio -label "7 significant digits" \
	   -variable SignificantDigits -value 7 -command "sigFigChange "
   .top.left.menubar.opts.m add radio -label "8 significant digits" \
	   -variable SignificantDigits -value 8 -command "sigFigChange "

   #  Add menu bar to display
   pack .top.left.menubar.file .top.left.menubar.acts .top.left.menubar.opts \
   	-side left -padx 4

   makeLogo .top.logo paradynLogo raised 2 HotPink2
   pack .top.logo -side right

   # Phase Name, below the menus (not filled in yet since asking for the
   # phase name would fail this early.)
   label .phasename -relief groove \
	   -font *-Helvetica-*-r-*-12-* 
   pack  .phasename -side top -fill x -expand false
   
   # Horizontal Scrollbar
   scrollbar .horizScrollbar -orient horizontal \
	   -background gray -activebackground gray -relief sunken \
	   -command horizScrollbarNewScrollPosition
   pack .horizScrollbar -side bottom -fill x

   # Vertical Scrollbar:
   scrollbar .vertScrollbar -orient vertical \
	   -background gray -activebackground gray -relief sunken \
	   -command vertScrollbarNewScrollPosition
   pack .vertScrollbar -side left -fill y -expand false

   # Body (drawn by C++ code -- metrics, units, foci, cells):
   frame .body -width 400 -height 220
   pack  .body -fill both -expand true

   bind .body <Configure> {tableVisiConfigure}
   bind .body <Expose>    {tableVisiExpose}
   bind .body <Button-1>  {tableVisiClick %x %y}

   wm minsize . 300 200
   wm title . "Table Visualization"

   bind .top <Destroy> +{tableVisiDestroyHook}
}
   
#  AddEntry -- Ask paradyn to start a new curve
proc AddEntry {} {
  Dg start "*" "*"
}

#  DelEntry -- Ask paradyn to stop a curve
proc DelEntry {} {
  puts stderr "Delete Entry not yet implemented"
}


#
# GracefulShutdown
# Effects a graceful shutdown of the visi
#
proc GracefulShutdown {} {
    destroy .
}

#  Called by visi library when histos have folded
#    we just update the status line and keep on going
#proc DgFoldCallback {} {
#  UpdateStatus 
#}

#
#  DgPhaseStartCallback -- visi calls this when a phase has started
#
proc DgPhaseStartCallback {phaseId} {
   puts stderr "welcome to DgPhaseStartCallback (tcl code)"
   return
}

#
#  DgPhaseEndCallback -- visi calls this when a phase has started
#
proc DgPhaseEndCallback {phaseId} {
   puts stderr "welcome to DgPhaseEndCallback (tcl code)"
   return
}

#
#  DgPhaseDataCallback -- visi calls this when a phase has started
#
proc DgPhaseDataCallback {} {
   puts stderr "welcome to DgPhaseEndCallback (tcl code)"
   return
}

