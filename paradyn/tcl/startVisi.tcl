# $Id: startVisi.tcl,v 1.10 2004/03/20 20:44:50 pcroth Exp $

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

