# tableVisi.tcl
# Ariel Tamches

#
# $Log: tableVisi.tcl,v $
# Revision 1.3  1995/11/29 00:44:19  tamches
# We now call makeLogo
#
# Revision 1.2  1995/11/08 21:15:33  tamches
# choosing sig figs is no longer a slider widget; it is a bunch of menu
# choices
#
# Revision 1.1  1995/11/04 00:43:12  tamches
# First version of new table visi
#
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
   menu .top.left.menubar.file.m -selectcolor cornflowerblue
   .top.left.menubar.file.m add command -label "Close Table" -command {destroy .}
   
   menubutton .top.left.menubar.acts -text "Actions" -menu .top.left.menubar.acts.m
   menu .top.left.menubar.acts.m  -selectcolor cornflowerblue
   .top.left.menubar.acts.m add command -label "Add Entries" -command AddEntry
   .top.left.menubar.acts.m add command -label "Delete Selected Entry" -command DelEntry -state disabled
   
   menubutton .top.left.menubar.opts -text "View" -menu .top.left.menubar.opts.m
   menu .top.left.menubar.opts.m  -selectcolor cornflowerblue

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
	   -variable sortFoci -value 1 -command sortFoci
   
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
	   -variable SignificantDigits -value 8 -command "sigFigChange "
   .top.left.menubar.opts.m add radio -label "8 significant digits" \
	   -variable SignificantDigits -value 8 -command "sigFigChange "

   #  Add menu bar to display
   pack .top.left.menubar.file .top.left.menubar.acts .top.left.menubar.opts \
   	-side left -padx 4
   
   #  Logo:
#   label .top.logo -relief raised \
#                     -bitmap @/p/paradyn/core/paradyn/tcl/logo.xbm \
#                     -foreground HotPink2
   makeLogo .top.logo paradynLogo raised 2 HotPink2

   pack .top.logo -side right

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

   wm minsize . 300 200
   wm title . "Table Visualization"
}
   
#  AddEntry -- Ask paradyn to start a new curve
proc AddEntry {} {
  Dg start "*" "*"
}

#  DelEntry -- Ask paradyn to stop a curve
proc DelEntry {} {
  puts stderr "Delete Entry not yet implemented"
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

#
#  Update the status line
#
proc UpdateStatus {} {
  global DataFormat W
  global Callbacks UpdateLimit lastBucket

  set bw [Dg binwidth]

  .status.label configure -text [format "Time: %-10s  Format: %s" [TimeLabel [expr int($bw * $lastBucket)]] $DataFormat]
}

#
#  GetValue asks visi library for the data value for the met/res pair 
#    we ask visi for the data in the correct DataFormat
#
#proc GetValue {m r bucket} {
#  global DataFormat
#  global numMetrics numResources
#
#   if {![Dg valid $m $r] || ![Dg enabled $m $r]} {
#      # there is a hole in valid datagrid entries -- happens all the time
#      return ""
#   }
#
#   if {[string match $DataFormat Instantaneous]} {
#      return [Dg value $m $r $bucket]
#   } elseif {[string match $DataFormat Average]} {
#      return [Dg aggregate $m $r]
#   } elseif {[string match $DataFormat Sum]} {
#      return [Dg sum $m $r]
#   } else {
#      puts stderr "GetValue: unknown data format: $DataFormat"
#      return 0
#   }
#}

#
# DgValidCallback -- visi calls this when curve becomes valid
#
#proc DgValidCallback {m} {
#  return
#  puts stderr "Bucket $m is now valid"
#}

set UpdateCounter 0
set UpdateLimit 1

# TimeToUpdate - returns true (1) iff it is time to redraw
#                the status (amount of time) at the bottom of the screen
proc TimeToUpdate {} {
  global UpdateCounter UpdateLimit

  if {$UpdateCounter <= 0} {
    set UpdateCounter [expr $UpdateLimit / [Dg binwidth]]
    return 1
  } else {
    set UpdateCounter [expr $UpdateCounter - 1]
    return 0
  }
}

set Callbacks 0

#
#  DgDataCallback -- visi calls this command when new data is available
#    we fill in all of the data labels with the new data values
#
#proc DgDataCallback {bucket} {
#   global Callbacks
#
#   incr Callbacks
#   if {[TimeToUpdate]} {
#      DataUpdate $bucket
#      UpdateStatus
#   }
#}

#
#  TimeLabel -- given a time value in seconds, format a nice label
#
#  note: If called often, this routine should be rewritten in C++.
proc TimeLabel {val} {
  if {($val > 60) && ($val < 3600)} {
    set min [expr $val / 60]
    set sec [expr $val - ($min * 60)]
    return [format "%d m %d s" $min $sec]
  }
  if {$val < 60} {
    return [format "%d s" $val]
  }
  if {$val > 3600} {
    set hr [expr $val / 3600]
    set left [expr $val - ($hr * 3600)]
    set min [expr $left / 60]
    set sec [expr $left - ($min * 60)]
    return [format "%d h %d m %d s" $hr $min $sec]
  }
  return "$val s"
}
