proc AcceptProc {} {
   global W 
   global phase
   global visiIds
   
   set thesel [$W.bottom.menu.list curselection]
   if {$thesel!=""} {
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
#  and it's phase info arguments
#
#
proc StartVisi {title lables ids numchoices} {
  global W
  global visiIds
  set W .vStart
  catch {destroy $W}
  toplevel $W
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
	    -background SeaGreen4 \
            -font "-Adobe-times-bold-r-normal--*-120*" 
  pack $T -side top -expand false -fill both 

  frame $W.bottom
  pack $W.bottom -side top -expand yes -fill y


  set M $W.bottom.menu
  frame $M
  pack $M -side top -expand yes -fill y

#  label $M.title -text "Choose a Visualization" \
#            -anchor center -relief flat \
#            -font "-Adobe-times-bold-r-normal--*-120*" 
#  pack $M.title -side top -expand true -fill both 

  frame $W.bottom.button
  pack $W.bottom.button -side bottom -expand y -fill x -pady 2m
  button $W.bottom.button.accept -text Accept -command AcceptProc 
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
  for {set i 0} { $i < $numchoices } {incr i} {
      set item [lindex $lables $i]
      set item2 [lindex $ids $i]
      $M.list insert end $item
#     puts stdout $item2 
#     puts stdout $item 
  }
  set visiIds $ids

  set B $W.bottom.buttons
  frame $B 
  pack $W.bottom.menu $W.bottom.buttons -expand yes  -pady .2c -padx .2c

#  label $B.title -text "Choose a Phase" \
#            -anchor center \
#	    -relief flat \
#            -font "-Adobe-times-bold-r-normal--*-120*" 
#
#  pack $B.title -side top -expand true -fill both 
 
 global phase

  radiobutton $B.bg -text "Global Phase" -variable phase \
	-relief sunken -value GlobalPhase 
  pack $B.bg -side left -anchor w 

  radiobutton $B.bc -text "Current Phase" -variable phase \
	-relief sunken -value CurrentPhase 
  pack $B.bc -side right -anchor w

#  pack $T $M $B -side top -expand yes -fill both


}

