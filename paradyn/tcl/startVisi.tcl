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

# $Id: startVisi.tcl,v 1.11 2004/03/23 19:12:15 eli Exp $

proc AcceptProc {} {
   global W 
   global phase
   global visiIds
   
   set thesel [$W.bottom.menu.list curselection]
   if {$thesel!=""} {
     $W.bottom.button.accept configure -state disabled
     set itemif [lindex $visiIds $thesel]
   
     if {$phase=="CurrentPhase"} {
       paradyn visi create [expr $itemif ]  1
     } else {
       paradyn visi create [expr $itemif ]  0
     }

#    for {set i 0} { $i < [llength $thesel] } {incr i} {
#       set item [lindex $thesel $i]
#       set itemstring [$W.bottom.menu.list get $item]
#       set itemid [lindex $visiIds $i]
#       puts stdout $itemstring
#       puts stdout $itemid
#       puts stdout $i
#    }
     destroy $W
   }
}

# set default phase choice to GlobalPhase
if {[catch {set phase}]} {
  set phase GlobalPhase 
}

#
#  display a menu that prompts user for visualization 
#  and its phase info arguments
#
#
proc drawVisiMenu {} {
  global W visiIds vnames vnums vcount

  # make sure that Paradyn knows about at least one application,
  # so that it has metrics to define
  if {![paradyn applicationDefined]} {
	showError 115 {}
	return
  }

  set W .vStart
  catch {destroy $W}
  toplevel $W -class Paradyn
  wm title $W "Start Visi"
  wm iconname $W "Start Visi"

# force the window to a happy location
  set baseGeom [wm geometry .]
  set Xbase 0
  set Ybase 0
  set Xoffset 30
  set Yoffset 30
  scan $baseGeom "%*dx%*d+%d+%d" Xbase Ybase
  wm geometry .vStart [format "+%d+%d" [expr $Xbase + $Xoffset] \
                                      [expr $Ybase + $Yoffset]]
 
# define all of the main frames
  set T $W.title
  label $T -text "Start A Visualization" \
            -anchor center -relief raised \
	    -foreground seashell1 \
	    -background SeaGreen4
  pack $T -side top -expand false -fill both 

  frame $W.bottom
  pack $W.bottom -side top -expand yes -fill y


  set M $W.bottom.menu
  frame $M
  pack $M -side top -expand yes -fill y

  frame $W.bottom.button
  pack $W.bottom.button -side bottom -expand y -fill x -pady 2m
  button $W.bottom.button.accept -text Start -command AcceptProc 
  button $W.bottom.button.dismiss -text Cancel -command "destroy $W"
  pack $W.bottom.button.accept $W.bottom.button.dismiss -side left -expand 1


  scrollbar $M.scroll -command "$M.list yview"

  listbox $M.list -yscroll "$M.scroll set" -setgrid 1 -height 8 -width 29 \
	-selectmode browse
  pack $M.scroll -side right -fill y 
  pack $M.list -side left -expand 1 -fill both

  bind $M.list <Double-1> {
     
  }

  # fill list box entries 
  for {set i 0} { $i < $vcount } {incr i} {
      set item [lindex $vnames $i]
      $M.list insert end $item
  }
  set visiIds $vnums

  set B $W.bottom.buttons
  frame $B 
  pack $W.bottom.menu $W.bottom.buttons -expand yes  -pady .2c -padx .2c

  global phase

  radiobutton $B.bg -text "Global Phase" -variable phase \
	-relief sunken -value GlobalPhase 
  pack $B.bg -side left -anchor w 

  radiobutton $B.bc -text "Current Phase" -variable phase \
	-relief sunken -value CurrentPhase 
  pack $B.bc -side right -anchor w

}

