#
# Copyright (c) 1996-2004 Barton P. Miller
# 
# We provide the Paradyn Parallel Performance Tools (below
# described as "Paradyn") on an AS IS basis, and do not warrant its
# validity or performance.  We reserve the right to update, modify,
# or discontinue this software at any time.  We shall have no
# obligation to supply such updates or modifications or any other
# form of support to you.
# 
# This license is for research uses.  For such uses, there is no
# charge. We define "research use" to mean you may freely use it
# inside your organization for whatever purposes you see fit. But you
# may not re-distribute Paradyn or parts of Paradyn, in any form
# source or binary (including derivatives), electronic or otherwise,
# to any other organization or entity without our permission.
# 
# (for other uses, please contact us at paradyn@cs.wisc.edu)
# 
# All warranties, including without limitation, any warranty of
# merchantability or fitness for a particular purpose, are hereby
# excluded.
# 
# By your use of Paradyn, you understand and agree that we (or any
# other person or entity with proprietary rights in Paradyn) are
# under no obligation to provide either maintenance services,
# update services, notices of latent defects, or correction of
# defects for Paradyn.
# 
# Even if advised of the possibility of such damages, under no
# circumstances shall we (or any other person or entity with
# proprietary rights in the software licensed hereunder) be liable
# to you or any third party for direct, indirect, or consequential
# damages of any character regardless of type of action, including,
# without limitation, loss of profits, loss of use, loss of good
# will, or computer failure or malfunction.  You agree to indemnify
# us (and any other person or entity with proprietary rights in the
# software licensed hereunder) for any and all liability it may
# incur to third parties resulting from your use of Paradyn.
#

#
# $Id: tableVisi.tcl,v 1.12 2004/03/23 19:13:55 eli Exp $
#

proc initializeTableVisi {} {
   if {[winfo depth .] > 1} {
   #  . config -bg grey
     option add *Background grey widgetDefault
     option add *activeBackground LightGrey widgetDefault
     option add *activeForeground black widgetDefault
     option add *Scale.activeForeground grey widgetDefault
   } else {
     option add *Background white widgetDefault
     option add *Foreground black widgetDefault
   }
   option add Paradyn*metricFont {Helvetica 10 bold} widgetDefault
   option add Paradyn*metricUnitsFont {Helvetica 10 bold} widgetDefault
   option add Paradyn*focusFont {Helvetica 10 bold} widgetDefault
   option add Paradyn*cellFont {Helvetica 10 roman} widgetDefault
   
   # Create the title bar, menu bar, and logo at the top:
   frame .top
   pack .top -side top -fill x
   
   frame .top.left 
   pack .top.left -side left -fill both -expand true
   
   label .top.left.title -relief raised -text "Table Visualization" \
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
   label .phasename -relief groove
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

