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

#This is used to create the actual call graph window. 
# $Id: callGraph.tcl,v 1.7 2004/03/23 19:12:07 eli Exp $

proc callGraphChangeCurrLabelHeight {numlines} {
   if {[winfo exists .callGraph.nontop.labelarea.current]} {
      .callGraph.nontop.labelarea.current config -height $numlines
      pack .callGraph.nontop.labelarea.current -side left -fill both -expand true
   }
}

# ####################################################################

proc callGraphEntryPoint {} {
   if {![paradyn applicationDefined]} {
      # No application has been started yet!!!
      # Put up an error dialog:
      showError 110 {}
      return 
   }
   return [callGraphInitialize]
}

proc callGraphInitialize {} {
   if {[winfo exists .callGraph]} {
      wm deiconify .callGraph
      raise .callGraph
      return
   }
   
   toplevel .callGraph -class Paradyn
   option add *callGraph*Background grey widgetDefault
   option add *callGraph*activeBackground LightGrey widgetDefault
   option add *callGraph*activeForeground black widgetDefault
   wm protocol .callGraph WM_DELETE_WINDOW {wm iconify .callGraph}

   # area for title, menubar, logo
   frame .callGraph.titlearea
   pack  .callGraph.titlearea -side top -fill x -expand false -anchor n

   frame .callGraph.titlearea.right
   pack  .callGraph.titlearea.right -side right -fill y -expand false

   makeLogo .callGraph.titlearea.right.logo paradynLogo raised 2 navy
   pack  .callGraph.titlearea.right.logo -side top

   frame .callGraph.titlearea.left
   pack  .callGraph.titlearea.left -side left -fill both -expand true

   label .callGraph.titlearea.left.title -text "Call Graph" \
	   -foreground white -anchor c -relief raised \
	   -background navy
   pack  .callGraph.titlearea.left.title -side top -fill both -expand true

   # area for menubar:
   frame .callGraph.titlearea.left.menu
   pack  .callGraph.titlearea.left.menu -side top -fill x -expand false -anchor n

   frame .callGraph.titlearea.left.menu.mbar -borderwidth 2 -relief raised
   pack  .callGraph.titlearea.left.menu.mbar -side top -fill both -expand false

   menubutton .callGraph.titlearea.left.menu.mbar.program -text "Programs" -menu .callGraph.titlearea.left.menu.mbar.program.m
   menu .callGraph.titlearea.left.menu.mbar.program.m -selectcolor #6495ED 
   menubutton .callGraph.titlearea.left.menu.mbar.view -text "View" -menu .callGraph.titlearea.left.menu.mbar.view.m
   menu .callGraph.titlearea.left.menu.mbar.view.m -selectcolor #6495ED
   .callGraph.titlearea.left.menu.mbar.view.m add command \
	   -label "Simple function names" -command "callGraphHideFullPath"
   .callGraph.titlearea.left.menu.mbar.view.m add command \
	   -label "Function names show full path" -command "callGraphShowFullPath"
   
   pack .callGraph.titlearea.left.menu.mbar.program -side left -padx 4
   pack .callGraph.titlearea.left.menu.mbar.view -side left -padx 4
   
   # -----------------------------------------------------------

   frame .callGraph.nontop
   pack  .callGraph.nontop -side bottom -fill both -expand true

   # -----------------------------------------------------------

   frame .callGraph.nontop.currprogramarea
   pack  .callGraph.nontop.currprogramarea -side top -fill x -expand false

    label .callGraph.nontop.currprogramarea.label1 -text "Current Program: " \
            -anchor e
    pack  .callGraph.nontop.currprogramarea.label1 -side left -fill both -expand true

   label .callGraph.nontop.currprogramarea.label2 -text "" \
            -anchor w
   pack  .callGraph.nontop.currprogramarea.label2 -side left -fill both -expand true

   # -----------------------------------------------------------

   frame .callGraph.nontop.main
   pack  .callGraph.nontop.main -side top -fill both -expand true

   scrollbar .callGraph.nontop.main.leftsb -orient vertical -width 16 \
	   -background gray \
	   -activebackground gray \
	   -command "callGraphNewVertScrollPosition"
   
   pack .callGraph.nontop.main.leftsb -side left -fill y -expand false

   scrollbar .callGraph.nontop.main.bottsb -orient horizontal -width 16 \
	   -activebackground gray \
	   -command "callGraphNewHorizScrollPosition"
   
   pack .callGraph.nontop.main.bottsb -side bottom -fill x -expand false
   
   frame .callGraph.nontop.main.all -relief flat -width 150m -height 75m
   pack .callGraph.nontop.main.all -side left -fill both -expand true
   
   # -----------------------------------------------------------
   frame .callGraph.nontop.find
   pack  .callGraph.nontop.find -side top -fill both -expand false
   
   label .callGraph.nontop.find.label -relief sunken -text "Search:"
   pack  .callGraph.nontop.find.label -side left -fill y -expand false

   entry .callGraph.nontop.find.entry -relief sunken
   pack  .callGraph.nontop.find.entry -side left -fill x -expand true
   
   bind  .callGraph.nontop.find.entry <Return> {callGraphFindHook [.callGraph.nontop.find.entry get]}
   
   # -----------------------------------------------------------

   # we need notification when the window is to be destroyed so
   # we can release the fonts it uses
   bind .callGraph.titlearea <Destroy> +{callGraphDestroyHook}

   bind .callGraph.nontop.main.all <Configure> {callGraphConfigureHook}
   bind .callGraph.nontop.main.all <Expose>    {callGraphExposeHook %c}
   bind .callGraph.nontop.main.all <Button-1>  {callGraphSingleClickHook %x %y}
   bind .callGraph.nontop.main.all <Double-Button-1> {callGraphDoubleClickHook %x %y}
   bind .callGraph.nontop.main.all <Alt-Motion> {callGraphAltPressHook %x %y}
   bind .callGraph.nontop.main.all <Motion> {callGraphAltReleaseHook}
   callGraphCreateHook
}
