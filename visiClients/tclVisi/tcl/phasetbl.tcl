#
#  default display options
#

if {[string match [winfo depth .] color] == 1} {
  # always defaults to bisque so reset it to grey
  . config -bg grey
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
option add *title*font *-New*Century*Schoolbook-Bold-R-*-18-*
option add *phaseName*font *-New*Century*Schoolbook-Bold-R-*-14-*
option add *phaseStart*font *-New*Century*Schoolbook-Bold-R-*-14-*
option add *phaseEnd*font *-New*Century*Schoolbook-Bold-R-*-14-*

proc getWindowWidth {wName} {
   # warning!  This routine will return an old number if an important
   # event (i.e. resize) happened but idle routines haven't yet kicked in.
   # --> *** In such cases, be sure to grab the latest information directly
   #         from the event structure instead of calling this routine!!!!

   set result [winfo width $wName]
   if {$result == 1} {
      # hack for a window that hasn't yet been mapped
      set result [winfo reqwidth $wName]
   }

   return $result
}

proc getWindowHeight {wName} {
   # warning!  This routine will return an old number if an important
   # event (i.e. resize) happened but idle routines haven't yet kicked in.
   # --> *** In such cases, be sure to grab the latest information directly
   #         from the event structure instead of calling this routine!!!!

   set result [winfo height $wName]
   if {$result == 1} {
      # hack for a window that hasn't yet been mapped
      set result [winfo reqheight $wName]
   }

   return $result
}

set W .table

#
#  Create the overall frame
#
frame $W -class Visi -width 4i -height 3i

#
#  Create the title bar, menu bar, and logo at the top
#
frame $W.top
pack $W.top -side top -fill x

frame $W.top.left 
pack $W.top.left -side left -fill both -expand 1

label $W.top.left.title -relief raised -text "Phase Table" \
      -foreground white -background Green4

pack $W.top.left.title -side top -fill both -expand true

#
#  Create the menubar as a frame with many menu buttons
#
frame $W.top.left.menubar -class MyMenu -borderwidth 2 -relief raised
pack $W.top.left.menubar -side top -fill x

#
#  File menu
# 
menubutton $W.top.left.menubar.file -text "File" -menu $W.top.left.menubar.file.m
menu $W.top.left.menubar.file.m
$W.top.left.menubar.file.m add command -label "Close" -command Shutdown

#
#  Actions menu
#
menubutton $W.top.left.menubar.acts -text "Actions" -menu $W.top.left.menubar.acts.m
menu $W.top.left.menubar.acts.m
$W.top.left.menubar.acts.m add command -label "Start Phase" -command PhaseDef 

#
#  Options menu
#

#
#  Help menu
#
menubutton $W.top.left.menubar.help -text "Help" -menu $W.top.left.menubar.help.m
menu $W.top.left.menubar.help.m
$W.top.left.menubar.help.m add command -label "General" -command "NotImpl"
$W.top.left.menubar.help.m add command -label "Context" -command "NotImpl"
#$W.top.left.menubar.help.m disable 0
#$W.top.left.menubar.help.m disable 1

#
#  Build the menu bar and add to display
#
pack $W.top.left.menubar.file $W.top.left.menubar.acts -side left -padx 4
pack $W.top.left.menubar.help -side right 

#
#  Organize all menu buttons into a menubar
#
#tk_menuBar $W.top.left.menubar $W.top.left.menubar.file $W.top.left.menubar.acts $W.top.left.menubar.help 

#
#  Build the logo 
#
label $W.top.logo -relief raised \
                  -bitmap @/p/paradyn/core/paradyn/tcl/logo.xbm \
                  -foreground HotPink4

pack $W.top.logo -side right


frame $W.left 
pack   $W.left -side left -fill both -expand true

#
# Left portion of middle: phase name, name canvas
#
label $W.left.phaseName -text "Phase Name" -foreground Blue 
pack  $W.left.phaseName -side top -expand false
# expand is false; if the window is made taller, we don't want the extra height

canvas $W.left.dataCanvas -relief groove \
        -width 1.3i
pack   $W.left.dataCanvas -side left -fill both -expand true


#
# Middle portion of middle: phase start time, data canvas
#

frame $W.middle 
pack  $W.middle -side left -fill both -expand true 

label $W.middle.phaseStart -text "Start Time" -foreground Blue 
pack   $W.middle.phaseStart -side top -fill x -expand false

canvas $W.middle.dataCanvas -relief groove \
        -width 1.3i
pack  $W.middle.dataCanvas -side top -fill both -expand true 

#
# Right portion of middle: phase end time, data canvas
#

frame $W.right 
pack  $W.right -side right -fill both -expand true

label $W.right.phaseEnd -text "End Time" -foreground Blue 
pack   $W.right.phaseEnd -side top -fill x -expand false

canvas $W.right.dataCanvas -relief groove \
        -width 1.3i
pack  $W.right.dataCanvas -side top -fill both -expand true


#
# display everything
#
pack append . $W {fill expand frame center}
wm minsize . 250 200
wm title . "Phase Table"

#
#  Helper function for "Close" menu option
#
proc Shutdown {} {
  destroy .
}

#
#  Issue a warning about missing function
#
proc NotImpl {} {
}

#
#  Called by visi library when histos have folded
#
proc DgFoldCallback {} {

}

#
#  Called by visi library when met/res space changes
#
proc DgConfigCallback {} {

}

#
# DgValidCallback -- visi calls this when curve becomes valid
#
proc DgValidCallback {m} {
  return
}

#
#  DgDataCallback -- visi calls this command when new data is available
#
proc DgDataCallback {bucket} {

}
set xOffset 5
set yOffset 5

proc NameUpdate {phaseId} {
   global W
   global xOffset 
   global yOffset 

   set theCanvas $W.left.dataCanvas

   set theText [Dg phasename $phaseId] 
   $theCanvas create text $xOffset $yOffset\
	-anchor nw \
	-fill black \
	-font *-Helvetica-*-r-*-12-* \
	-justify center \
	-tag dataTag \
	-text $theText 


   return
}

proc StartUpdate {phaseId} {
   global W
   global xOffset 
   global yOffset 

   set theCanvas $W.middle.dataCanvas

   set theText [TimeLabel [expr int([Dg phasestartT $phaseId])]  ] 
   $theCanvas create text $xOffset $yOffset\
	-anchor nw \
	-fill black \
	-font *-Helvetica-*-r-*-12-* \
	-justify center \
	-tag dataTag \
	-text $theText 
   return
}

proc EndUpdate {phaseId} {
   global W
   global xOffset 
   global yOffset 

   set theCanvas $W.right.dataCanvas

   set theText [TimeLabel [expr int([Dg phaseendT $phaseId])]  ]
   $theCanvas create text $xOffset $yOffset\
	-anchor nw \
	-fill black \
	-font *-Helvetica-*-r-*-12-* \
	-justify center \
	-tag dataTag \
	-text $theText 

   set yOffset [expr $yOffset + 20]
   return
}

#
#  DgPhaseStartCallback -- visi calls this when a phase has started
#
proc DgPhaseStartCallback {phaseId} {
   
   NameUpdate $phaseId
   StartUpdate $phaseId
   return
}

#
#  DgPhaseEndCallback -- visi calls this when a phase has started
#
proc DgPhaseEndCallback {phaseId} { 
   EndUpdate $phaseId
   return
}

#
#  DgPhaseDataCallback -- visi calls this when a phase has started
#
proc DgPhaseDataCallback {} {
   return
}



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


#
# PhaseDef: ask paradyn to start a new phase
#
proc PhaseDef {} {

  Dg phase 

}
