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

# $Id: tclTunable.tcl,v 1.23 2004/03/23 19:12:17 eli Exp $

# To do list:
# 1) if user deletes tunable descriptions window (using window mgr), then
#    we'll get confused because tunable descriptions will still be true.
#    Would like to detect when a window is destroyed, and set tunable descriptions
#    false.  Something tells me this is doable in tcl.

set devModeColor #b3331e1b53c7          ;# for developersMode constant's labels
set unsetXgeometry "1x1+0+0"            ;# unspecified/default X geometry
set lastTCgeometry $unsetXgeometry      ;# initialize to null/default geometry
set lastTDgeometry $unsetXgeometry      ;# initialize to null/default geometry

# #################### Some Misc Routines #####################
proc max {x y} {
   if {$x > $y} {
      return $x
   } else {
      return $y
   }
}

# warning!  These routines will return an old number if an important
# event (i.e. resize) happened but idle routines haven't yet kicked in.
# --> *** In such cases, be sure to grab the latest information directly
#         from the event structure instead of calling these routines!!!!
proc getWindowWidth {wName} {
   set result [winfo width $wName]
   if {$result == 1} {
      # hack for a window that hasn't yet been mapped
      set result [winfo reqwidth $wName]
   }
   return $result
}

proc getWindowHeight {wName} {
   set result [winfo height $wName]
   if {$result == 1} {
      set result [winfo reqheight $wName]
   }
   return $result
}
# #############################################################

proc tunableInitialize {} {
   global tunableMinHeight
   set tunableMinHeight 175

   toplevel .tune -class Paradyn
   wm title .tune "Tunable Constants"
   # one does not pack a toplevel window...

   #  ################### Default options #################

   if {[winfo depth .] > 1} {
      # You have a color monitor...
      # change primary background color from 'bisque' to 'grey'
      option add *tune*Background grey widgetDefault
      option add *tune*activeBackground LightGrey widgetDefault
      option add *tune*activeForeground black widgetDefault
      option add *tune*Scale.activeForeground grey widgetDefault

      option add *tunableDescriptions*Background grey widgetDefault
      option add *tunableDescriptions*activeBackground LightGrey widgetDefault
      option add *tunableDescriptions*activeForeground black widgetDefault
      option add *tunableDescriptions*Scale.activeForeground grey widgetDefault
   } else {
      # You don't have a color monitor...
      option add *tune*Background white widgetDefault
      option add *tune*Foreground black widgetDefault

      option add *tunableDescriptions*Background white widgetDefault
      option add *tunableDescriptions*Foreground black widgetDefault
   }

   # .tune.top -- stuff at the top (menu bar, title bar, logo, other miscellanea)
   frame .tune.top
   pack  .tune.top -side top -fill x -expand false
      # expand is false; if the window is made taller, we don't want the extra height
   
   # .tune.top.logo -- paradyn logo
   makeLogo .tune.top.logo paradynLogo raised 2 #6495ED
   pack  .tune.top.logo -side right
      # expand is false; if the window is made wider, we don't want the extra width
 
   # .tune.top.left -- stuff to the left of the logo (title bar & menu bar)
   frame .tune.top.left
   pack  .tune.top.left -side left -fill both -expand true
      # expand is true; we'll take extra height (don't worry, .tune.top won't get taller)

   # .tune.top.left.mbar -- Menu Bar   
   frame .tune.top.left.mbar -borderwidth 2 -relief raised
   pack  .tune.top.left.mbar -side bottom -fill x -expand false
      # expand is false; if the window is made taller, we don't want the extra height
   
   menubutton .tune.top.left.mbar.help -text Help -menu .tune.top.left.mbar.help.m
   menu .tune.top.left.mbar.help.m
   .tune.top.left.mbar.help.m add command -label "Show Tunable Descriptions" \
               -command processShowTunableDescriptions

   pack .tune.top.left.mbar.help -side right -padx 4

   # .tune.top.left.titlebar -- Title ("Tunable Constants") (above menu bar)
   label .tune.top.left.titlebar -text "Tunable Constants" -foreground white \
	   -background #6495ED \
	   -anchor c -relief raised
   pack  .tune.top.left.titlebar -side top -fill both -expand true
      # expand is true; we want to fill up .tune.top.left (which itself won't enlarge so OK)

   # .tune.bottom (buttons)
   frame .tune.bottom -relief sunken
   pack  .tune.bottom -side bottom -fill x -expand false -ipadx 4 -ipady 4
      # expand is false; if the window is made taller, we don't want the extra height

   frame  .tune.bottom.buttonFrame
   pack   .tune.bottom.buttonFrame -side top -fill y -expand true

   button .tune.bottom.buttonFrame.accept -text "OK" -anchor c \
           -command processCommitFinalTunableValues
   pack   .tune.bottom.buttonFrame.accept -side left -ipadx 10

   frame  .tune.bottom.buttonFrame.middle -width 20
   pack   .tune.bottom.buttonFrame.middle -side left

   button .tune.bottom.buttonFrame.discard -text "Cancel" -anchor c \
           -command processDiscardFinalTunableValues
   pack   .tune.bottom.buttonFrame.discard -side right -ipadx 10
   
   # .tune.middle -- body of the window (scrollbar & tunable constants canvas)
   frame .tune.middle
   pack  .tune.middle -side top -fill both -expand true
      # expand is true; we want extra width & height if the window is resized

   scrollbar .tune.middle.scrollbar -orient vertical -width 16 \
   	-background gray -activebackground gray -relief sunken \
   	-command ".tune.middle.canvas yview"
   pack      .tune.middle.scrollbar -side left -fill y -expand false
      # expand is false; if the window is made wider, we don't want the extra width
   focus .tune.middle.scrollbar
   
   canvas .tune.middle.canvas -relief flat -yscrollcommand myScroll \
	   -yscrollincrement 1
   pack   .tune.middle.canvas -side left -fill both -expand true

   frame .tune.middle.canvas.names
   frame .tune.middle.canvas.values

   # the following line avoids flickering, but there is a cost: the window won't start off with
   # exactly the right window size.  To be more specific, it won't intelligently
   # set the size of the canvas subwindow to be exactly the sum of sizes of
   # the children.  Instead, it will go with whatever the initial value is provided.
   pack propagate .tune.middle.canvas false
   pack   .tune.middle.canvas -side left -fill both -expand true
      # expand is true; we want extra height & width is the window is resized
}

proc myScroll {left right} {
   # gets called whenever the canvas view changes [scroll] or gets resized.
   # Gives us a chance to rethink the bounds of the scrollbar
   #puts stderr "welcome to myScroll (canvas must've scrolled or been resized)"

   global lastVisibleHeight lastVisibleWidth

   set newWidth  [getWindowWidth  .tune.middle.canvas]
   set newHeight [getWindowHeight .tune.middle.canvas]
   
   if {$lastVisibleHeight != $newHeight || $lastVisibleWidth != $newWidth} {
      #puts stderr "myScroll: redrawing tunables on canvas due to apparant resize"

      drawTunables $newWidth $newHeight
   }

   .tune.middle.scrollbar set $left $right

   set lastVisibleWidth $newWidth
   set lastVisibleHeight $newHeight
}

proc tunableBoolLabelEnter {lcv} {
   set buttonLabelWin .tune.middle.canvas.names.tunable$lcv.label
   set dummyLabelWin  .tune.middle.canvas.values.tunable$lcv.dummy
   set valuesLabelWin .tune.middle.canvas.values.tunable$lcv.box

   $buttonLabelWin configure -background lightGray 
   $dummyLabelWin  configure -background lightGray
   $valuesLabelWin configure -background lightGray
}

proc tunableBoolLabelLeave {lcv} {
   set buttonLabelWin .tune.middle.canvas.names.tunable$lcv.label
   set dummyLabelWin .tune.middle.canvas.values.tunable$lcv.dummy
   set valuesLabelWin .tune.middle.canvas.values.tunable$lcv.box

   $buttonLabelWin configure -background gray
   $dummyLabelWin  configure -background gray
   $valuesLabelWin configure -background gray
}

proc tunableBoolLabelPress {lcv} {
# only the value box can be nicely "sunk"
#   set buttonLabelWin .tune.middle.canvas.names.tunable$lcv.label
#   set dummyLabelWin .tune.middle.canvas.values.tunable$lcv.dummy
   set valuesLabelWin .tune.middle.canvas.values.tunable$lcv.box

#   $buttonLabelWin configure -relief sunken
#   $dummyLabelWin configure -relief sunken
   $valuesLabelWin configure -relief sunken
}

proc tunableBoolLabelRelease {lcv} {
   set buttonLabelWin .tune.middle.canvas.names.tunable$lcv.label
   set dummyLabelWin .tune.middle.canvas.values.tunable$lcv.dummy
   set valuesLabelWin .tune.middle.canvas.values.tunable$lcv.box

#   $buttonLabelWin invoke
#   $dummyLabelWin  invoke
   $valuesLabelWin invoke

   $buttonLabelWin configure -relief flat
   $dummyLabelWin configure -relief flat
   $valuesLabelWin configure -relief flat
}

proc buttonBindJustHighlight {theButton lcv} {
   bind $theButton <Enter> "tunableBoolLabelEnter $lcv"
   bind $theButton <Leave> "tunableBoolLabelLeave $lcv"
}

proc buttonBind {theButton lcv} {
   buttonBindJustHighlight $theButton $lcv
   bind $theButton <ButtonPress-1> "tunableBoolLabelPress $lcv"
   bind $theButton <ButtonRelease-1> "tunableBoolLabelRelease $lcv"
}

proc drawBoolTunable {theName onlyDeveloperTunable} {
   global namesWidth
   global numTunablesDrawn
   global devModeColor
	global maxRowHeight


   # the following important vrbles are (associative) arrays (indexed by name) of
   # boolean tunable constant descriptions and newvalues.
   global boolTunableNewValues

   set namesWin  .tune.middle.canvas.names.tunable$numTunablesDrawn
   set valuesWin .tune.middle.canvas.values.tunable$numTunablesDrawn

   frame $valuesWin
   pack propagate $valuesWin false
   pack  $valuesWin -side top -fill x -expand true

   frame $namesWin
   pack propagate $namesWin false
   pack  $namesWin -side top -fill x -expand true

   set buttonLabelWin $namesWin.label
   set valuesLabelWin $valuesWin.box
   
   # dummy label.  At first, I used a checkbutton to guarantee that
   # all 3 widgets would have the same height.  But using
   # -highlightthickness 0 for the realcheckbutton made them all the
   # same anyway in tk 4.0
   label $valuesWin.dummy -relief flat
   pack  $valuesWin.dummy -side left -fill y

   # In order to get the appearance of a checkbutton with the on/off red square
   # on the right instead of on the left, we use 2 labels & a checkbutton.
   # The second one is the checkbutton, it has an indicator but no text.

   label $buttonLabelWin -text $theName -anchor w -height 1 -relief flat
   if {$onlyDeveloperTunable} { $buttonLabelWin config -foreground $devModeColor }
   pack  $buttonLabelWin -side top -fill both -expand true

   checkbutton $valuesLabelWin -variable boolTunableNewValues($theName) -anchor w \
	   -relief flat -highlightthickness 0
   pack $valuesLabelWin -side left -fill both -expand true

   # now make the label and the checkbutton appear as 1; we play some bind tricks

   set leftButton $buttonLabelWin
   set dummyButton $valuesWin.dummy
   set rightButton $valuesLabelWin

   buttonBind $leftButton $numTunablesDrawn
   buttonBind $dummyButton $numTunablesDrawn
   buttonBindJustHighlight $rightButton $numTunablesDrawn

   set namesWidth  [max $namesWidth [getWindowWidth $leftButton]]
   set namesWidth  [max $namesWidth [getWindowWidth $namesWin]]

	# determine the height of the tallest row
	set boolRowHeight [max [getWindowHeight $namesWin] [getWindowHeight $valuesWin]]
	set maxRowHeight [max $maxRowHeight $boolRowHeight]

   incr numTunablesDrawn
}

proc everyChangeCommand {name newValue} {
   # A scale widget's value has changed.
   # We are passed the tunable index number and the new integer value.
 
   # I repeat: integer value
   # You may be wondering if this means it's impossible to do our scale
   # widgets, since we need floating point numbers.  Well, we use some
   # tricks to get around this limitation.  First of all, the scale widget
   # does not show any ticks.  That means the numbers can be whatever we
   # want them to be.  We choose to multiply the min and max by
   # $integerScaleFactor and then divide it back here...

   # the following important vrbles are (associative) arrays (indexed by name)
   # of float tunable constant descriptions and newvalues.
   global floatTunableNewValues

   global integerScaleFactor

   set oldValue $newValue
   set newValue [expr ( double($oldValue) / $integerScaleFactor )]

   set floatTunableNewValues($name) $newValue
   # This command automagically updates the entry widget because the
   # entry widget had its -textvariable set to newTunableValues($name)
}

proc bindFloatEnter {lcv} {
   .tune.middle.canvas.names.tunable$lcv.label configure -background lightGray
   if {[winfo exists .tune.middle.canvas.names.tunable$lcv.padding]} {
      .tune.middle.canvas.names.tunable$lcv.padding configure -background lightGray
   }

   .tune.middle.canvas.values.tunable$lcv.label configure -background lightGray
   if {[winfo exists .tune.middle.canvas.values.tunable$lcv.left.top]} {
      .tune.middle.canvas.values.tunable$lcv.left.top configure -background lightGray
   }
   .tune.middle.canvas.values.tunable$lcv.left configure -background lightGray
   if {[winfo exists .tune.middle.canvas.values.tunable$lcv.left.leftTick]} {
      .tune.middle.canvas.values.tunable$lcv.left.leftTick configure -background lightGray
      .tune.middle.canvas.values.tunable$lcv.left.rightTick configure -background lightGray
   }
   if {[winfo exists .tune.middle.canvas.values.tunable$lcv.left.padAfterRightTick]} {
      .tune.middle.canvas.values.tunable$lcv.left.padAfterRightTick configure -background lightGray
   }

   set entryWin .tune.middle.canvas.values.tunable$lcv.right.top.entry
   if {[winfo exists $entryWin]} {
      $entryWin configure -background lightGray
   }
}
proc bindFloatLeave {lcv} {
   .tune.middle.canvas.names.tunable$lcv.label configure -background gray
   if {[winfo exists .tune.middle.canvas.names.tunable$lcv.padding]} {
      .tune.middle.canvas.names.tunable$lcv.padding configure -background gray
   }

   .tune.middle.canvas.values.tunable$lcv.label configure -background gray
   if {[winfo exists .tune.middle.canvas.values.tunable$lcv.left.top]} {
      .tune.middle.canvas.values.tunable$lcv.left.top configure -background gray
   }
   .tune.middle.canvas.values.tunable$lcv.left configure -background gray
   if {[winfo exists .tune.middle.canvas.values.tunable$lcv.left.leftTick]} {
      .tune.middle.canvas.values.tunable$lcv.left.leftTick configure -background gray
      .tune.middle.canvas.values.tunable$lcv.left.rightTick configure -background gray
   }
   if {[winfo exists .tune.middle.canvas.values.tunable$lcv.left.padAfterRightTick]} {
      .tune.middle.canvas.values.tunable$lcv.left.padAfterRightTick configure -background gray
   }

   set entryWin .tune.middle.canvas.values.tunable$lcv.right.top.entry
   if {[winfo exists $entryWin]} {
      $entryWin configure -background gray
   }
}

proc bindFloatSet {lcv scaleWin theName} {
   global integerScaleFactor
   global floatTunableNewValues

   $scaleWin set [expr round($integerScaleFactor * $floatTunableNewValues($theName))]
}

proc dummySuppressChar {} {
}

proc entryBind {theWindow lcv theScale theName} {
   bind $theWindow <Return>   "bindFloatSet $lcv $theScale $theName"
   bind $theWindow <KP_Enter> "bindFloatSet $lcv $theScale $theName"
###bind $theWindow <Tab>      "bindFloatSet $lcv $theScale $theName"
}

proc valueBind {theWindow lcv} {
   bind $theWindow <Enter> "bindFloatEnter $lcv"
   bind $theWindow <Leave> "bindFloatLeave $lcv"
}

proc drawFloatTunable {theName onlyDeveloperTunable leftTickWidth rightTickWidth} {
   global nextStartY
   global namesWidth
   global maxRowHeight

   global devModeColor
   global numTunablesDrawn
   global integerScaleFactor

   # the following important vrbles are (associative) arrays (indexed by name) of
   # float tunable constant description/use/min/max and newvalues.
   global floatTunableNewValues

   set tunableDescription [uimpd tclTunable getdescription $theName]

   # if both 0.0, then as far as we're concerned, there are no min/max values.
   set tunableMin [lindex [uimpd tclTunable getfloatrangebyname $theName] 0]
   set tunableMax [lindex [uimpd tclTunable getfloatrangebyname $theName] 1]

   set namesWin  .tune.middle.canvas.names.tunable$numTunablesDrawn
   set valuesWin .tune.middle.canvas.values.tunable$numTunablesDrawn
   
   # label widget for the floating tunable's name
   frame $valuesWin
   pack propagate $valuesWin false
   pack  $valuesWin -side top -fill x -expand true

   # dummy label widget (so the right side of the screen will be as tall as the left)
   label $valuesWin.label -relief flat -height 1
   pack  $valuesWin.label -side left -fill y
   valueBind $valuesWin.label $numTunablesDrawn

   frame $valuesWin.left
   pack  $valuesWin.left -side left -fill both -expand true
   valueBind $valuesWin.left $numTunablesDrawn

   frame $valuesWin.right
   pack  $valuesWin.right -side right -fill y

   frame $valuesWin.right.top
   pack  $valuesWin.right.top -side top -fill x
   
   frame $valuesWin.right.bottom
   pack  $valuesWin.right.bottom -side bottom -fill both -expand true
   
   # entry widget
   set entryWin $valuesWin.right.top.entry
   entry $entryWin -relief sunken -width 8 \
        -highlightthickness 0 -textvariable floatTunableNewValues($theName)

   # turn off some useless characters (such as "return" key)
##   bind $entryWin <Return>   {dummySuppressChar %K}
##   bind $entryWin <Tab>      {dummySuppressChar %K}
##   bind $entryWin <KP_Enter> {dummySuppressChar %K}
# NB: see later for entryBind!
   valueBind $entryWin $numTunablesDrawn

   pack $entryWin -side right -expand false
      # expand is false; if the window is made taller, we don't want the extra height

   # scale widget 

   set tickWin $valuesWin.left
   # a bit of padding between the rightmost tick and the entry widget
   # (even if a rightmost tick doesn't exist)
   frame $tickWin.padAfterRightTick -width 10
   pack  $tickWin.padAfterRightTick -side right
   valueBind $tickWin.padAfterRightTick $numTunablesDrawn

   if {$tunableMin!=0 || $tunableMax!= 0} {
      label $tickWin.leftTick -text $tunableMin -width $leftTickWidth -anchor e
      pack $tickWin.leftTick -side left
      valueBind $tickWin.leftTick $numTunablesDrawn

      label $tickWin.rightTick -text $tunableMax -width $rightTickWidth -anchor w
      pack $tickWin.rightTick -side right -fill y
      valueBind $tickWin.rightTick $numTunablesDrawn

      # [other options to try: -length -sliderlength -width -showValue]
      set scaleWin $valuesWin.left.top

      scale $scaleWin -orient horizontal \
	      -relief flat \
	      -command "everyChangeCommand $theName" \
	      -from [expr $integerScaleFactor * $tunableMin] \
	      -to   [expr $integerScaleFactor * $tunableMax] \
	      -showvalue false \
	      -highlightthickness 0

#	      -width [winfo reqheight $entryWin] (makes it too tall for some reason)

      valueBind $scaleWin $numTunablesDrawn
      entryBind $entryWin $numTunablesDrawn $scaleWin $theName

      # initialize the scale setting
      $scaleWin set [expr round($integerScaleFactor * $floatTunableNewValues($theName))]
      $entryWin config -textvariable floatTunableNewValues($theName)
      pack $scaleWin -side top -fill x -expand true
   }

   # finding the height of the values window is difficult; the frames don't
   # seem to have a size at this point, even though they and all their children
   # have been packed.   
   set valuesWinHeight [getWindowHeight $entryWin]
   if {$tunableMin!=0 || $tunableMax!=0} {
      set valuesWinHeight [max $valuesWinHeight [getWindowHeight $scaleWin]]
   }

   # Now for the left (the name label widget)
   frame $namesWin
#   pack propagate $namesWin false
   pack  $namesWin -side top -fill x -expand true

   label $namesWin.label -text $theName -anchor w -height 1
   if {$onlyDeveloperTunable} { $namesWin.label config -foreground $devModeColor }
   pack  $namesWin.label -side top -fill x -expand true
   valueBind $namesWin.label $numTunablesDrawn

   set paddingHeight [expr $valuesWinHeight - [getWindowHeight $namesWin.label]]
   if {$tunableMin==0 && $tunableMax==0} {
      incr paddingHeight
   }
   if {$paddingHeight > 0} {
      frame $namesWin.padding -height $paddingHeight
      pack propagate $namesWin.padding false
      pack  $namesWin.padding -side bottom -fill both -expand true
      valueBind $namesWin.padding $numTunablesDrawn
   }

   # finding the height of the left window is easy; it has no frames to confuse us
   set namesWinHeight [max [getWindowHeight $namesWin] [getWindowHeight $namesWin.label]]

   set theHeight [max $namesWinHeight $valuesWinHeight]
   set maxRowHeight [max $theHeight $maxRowHeight]

   # update the frames' (plural) heights so they're equal
   $namesWin  configure -height $maxRowHeight
   $valuesWin configure -height $maxRowHeight

   set namesWidth  [max $namesWidth  [getWindowWidth $namesWin.label]]

   incr numTunablesDrawn
}

proc drawTunables {newWidth newHeight} {
   global numTunablesDrawn
   global nextStartY
   global namesWidth
   global maxRowHeight
   global DeveloperModeFlag
   global tunableMinWidth tunableMinHeight

   global boolTunableNewValues
   global floatTunableNewValues

   # First, erase old stuff on the screen
   .tune.middle.canvas delete tunableTag

   destroy .tune.middle.canvas.names
   destroy .tune.middle.canvas.values

   frame .tune.middle.canvas.names
   pack  .tune.middle.canvas.names -side left -expand false

   frame .tune.middle.canvas.values
   pack  .tune.middle.canvas.values -side left -fill x -expand true

   set numTunablesDrawn 0

   set nextStartY 0
   set namesWidth 0

   # Determine the max # chars needed for the ticks (min/max float strings)
   # We simply loop through all float tc's (those with min/max defined), doing
   # a "string length".
   set leftTickWidth 0
   set rightTickWidth 0
   
   set allFloatNames [uimpd tclTunable getfloatallnames]

   set numFloats [llength $allFloatNames]

   for {set floatlcv 0} {$floatlcv < $numFloats} {incr floatlcv} {
      set floatName [lindex $allFloatNames $floatlcv]
      set tunableUse [uimpd tclTunable getusebyname $floatName]
###   if {$tunableUse=="developer" && $DeveloperModeFlag==0} continue

      set tunableBounds [uimpd tclTunable getfloatrangebyname $floatName]
      set tunableMin [lindex $tunableBounds 0]
      set tunableMax [lindex $tunableBounds 1]

      if {$tunableMin!=0 || $tunableMax!=0} {
         set leftTickWidth [max $leftTickWidth [string length $tunableMin]]
         set rightTickWidth [max $rightTickWidth [string length $tunableMax]]
      }
   }

   # make two passes---draw all boolean tunables, then all float tunables.
   # (looks nicer on screen that way...)

   set allBoolNames [uimpd tclTunable getboolallnames]
   set numBoolNames [llength $allBoolNames]

   for {set lcv 0} {$lcv < $numBoolNames} {incr lcv} {
      set theName [lindex $allBoolNames $lcv]

      set theDU [uimpd tclTunable getdescription $theName]
      set tunableUse [uimpd tclTunable getusebyname $theName]

      # If this tunable constant is a "developer" one, and if we
      # are not in developer mode, then skip it.
      set developerTunableOnly [ string compare $tunableUse "developer" ]
      if {$developerTunableOnly==0 && $DeveloperModeFlag==0} continue
      drawBoolTunable $theName [expr ($developerTunableOnly==0 || \
	          ([ string compare $theName "developerMode" ] == 0 && \
                  $DeveloperModeFlag==1))]
   }

   set numFloatNames [llength $allFloatNames]

   for {set lcv 0} {$lcv < $numFloatNames} {incr lcv} {
      set theName [lindex $allFloatNames $lcv]

      set tunableUse [uimpd tclTunable getusebyname $theName]

      set developerTunableOnly [ string compare $tunableUse "developer" ]
      if {$developerTunableOnly==0 && $DeveloperModeFlag==0} continue
      drawFloatTunable $theName [ expr $developerTunableOnly==0] \
        $leftTickWidth $rightTickWidth
   }

	#
	# update heights of rows, now that we know height of tallest row
	#
	for {set lcv 0} {$lcv < $numTunablesDrawn} {incr lcv} {
		.tune.middle.canvas.names.tunable$lcv configure -height $maxRowHeight
		.tune.middle.canvas.values.tunable$lcv configure -height $maxRowHeight
	}
	.tune.middle.canvas.names configure -width $namesWidth

   # the above calls will have updated variables:
   # namesWidth
   # maxRowHeight
	set nextStartY [expr $numTunablesDrawn * $maxRowHeight]

   set namesWidth  [max $namesWidth  [getWindowWidth .tune.middle.canvas.names]]
   set valuesWidth [expr [getWindowWidth .tune.middle.canvas] - $namesWidth]

   .tune.middle.canvas create window 0 0 \
	-anchor nw -tag tunableTag \
	-window .tune.middle.canvas.names

   .tune.middle.canvas create window $namesWidth 0 \
	   -anchor nw -tag tunableTag \
	   -window .tune.middle.canvas.values \
	   -width $valuesWidth

   set goodMinWidth [expr $namesWidth + [getWindowWidth .tune.middle.scrollbar] + 245]
   wm minsize .tune $goodMinWidth $tunableMinHeight

   # reset scroll-increment based on (now-known) row height   
   .tune.middle.canvas configure -yscrollincrement $maxRowHeight

   rethinkScrollBarRegions [getWindowWidth .tune.middle.canvas] \
                           [getWindowHeight .tune.middle.canvas]

   global lastVisibleHeight lastVisibleWidth
   set lastVisibleHeight [getWindowHeight .tune.middle.canvas]
   set lastVisibleWidth  [getWindowWidth .tune.middle.canvas]

   return $goodMinWidth
}

proc rethinkScrollBarRegions {newWidth newHeight} {
   # explicitly called by the program after putting up the tunable
   # constants (e.g. first time, after changing to/from developer mode)
   # We are passed how many pixels the tunables take up, and we adjust
   # the canvas' scrollregion and the scrollbar settings accordingly.

   global nextStartY

   #puts stderr "rethinkScrollbarRegion; window width=$newWidth, height=$newHeight; fullHeight=$nextStartY"

   # update the scrollbar's scrollregion configuration
   set regionList {0 0 0 0}
   set regionList [lreplace $regionList 2 2 $newWidth]
   set regionList [lreplace $regionList 3 3 $nextStartY]
   .tune.middle.canvas configure -scrollregion $regionList

   set newFirst 0
   set fracVisible [expr 1.0 * $newHeight / $nextStartY]
   #puts stderr "rethinkScrollbarRegion: fracVisible=$fracVisible"
   set newLast [expr $newFirst + $fracVisible]

   .tune.middle.scrollbar set $newFirst $newLast
}

proc gatherInitialTunableValues {} {
   # associative array (by name) of description/use
   # associative array (by name) of bool value
   global boolTunableOldValues boolTunableNewValues

   # associative array (by name) of description/use/min/max
   # associative array (by name) of float value
   global floatTunableOldValues floatTunableNewValues

   # First, we initialize all the boolean tunable constants:
   set allBoolNames [uimpd tclTunable getboolallnames]
   set numBoolNames [llength $allBoolNames]

   for {set lcv 0} {$lcv < $numBoolNames} {incr lcv} {
      set theName [lindex $allBoolNames $lcv]

      set theDescription [uimpd tclTunable getdescription $theName]
      set theUse         [uimpd tclTunable getusebyname $theName] 
      set theList [list $theDescription $theUse]

      set boolTunableOldValues($theName) [uimpd tclTunable getvaluebyname $theName]
      set boolTunableNewValues($theName) $boolTunableOldValues($theName)
   }

   #puts stderr "gatherInitialTunableValues -- bool tunable constants have been initialized"

   # Next, we initialize all the float tunable constants:
   set allFloatNames [uimpd tclTunable getfloatallnames]
   set numFloatNames [llength $allFloatNames]
   for {set lcv 0} {$lcv < $numFloatNames} {incr lcv} {
      set theName [lindex $allFloatNames $lcv]

      set floatTunableOldValues($theName) [uimpd tclTunable getvaluebyname $theName]
      set floatTunableNewValues($theName) $floatTunableOldValues($theName)
   }
}

proc processCommitFinalTunableValues {} {
   global boolTunableOldValues boolTunableNewValues
   global floatTunableOldValues floatTunableNewValues

   set devModeSwitch false
   set allBoolNames [uimpd tclTunable getboolallnames]
   set numBoolNames [llength $allBoolNames]
   for {set lcv 0} {$lcv < $numBoolNames} {incr lcv} {
      set theName [lindex $allBoolNames $lcv]
      if {$boolTunableNewValues($theName) != $boolTunableOldValues($theName)} {
#        #puts stderr "processFinalTunableValues: tunable $theName has changed \
#                from $boolTunableOldValues($theName) \
#                to $boolTunableNewValues($theName)!"
         if {$theName == "developerMode" } { set devModeSwitch true }
         uimpd tclTunable setvaluebyname $theName $boolTunableNewValues($theName)
      }
   }

   # Now the same for float tunables:
   set allFloatNames [uimpd tclTunable getfloatallnames]
   set numFloatNames [llength $allFloatNames]
   for {set lcv 0} {$lcv < $numFloatNames} {incr lcv} {
      set theName [lindex $allFloatNames $lcv]
      if {$floatTunableNewValues($theName) != $floatTunableOldValues($theName)} {
         #puts stderr "processFinalTunableValues: tunable $theName has changed from $floatTunableOldValues($theName) to $floatTunableNewValues($theName)!"

         uimpd tclTunable setvaluebyname $theName $floatTunableNewValues($theName)
      }
   }
  
   if { $devModeSwitch } { 
        tunableSwitchPoint
   } else {
        tunableExitPoint
   }
}

proc processDiscardFinalTunableValues {} {
   tunableExitPoint
}

# ******************* Tunable Descriptions Stuff *****************

proc processShowTunableDescriptions {} {
   global numTunableDescriptionsDrawn
   global tunableTitleFont tunableDescriptionFont
   global tunableDescriptionsMinWidth tunableDescriptionsMinHeight
   global lastVisibleDescriptionsWidth lastVisibleDescriptionsHeight
   global lastTDgeometry unsetXgeometry

   if {[winfo exists .tunableDescriptions]} {
      return
   }

   set numTunableDescriptionsDrawn 0

   #set titleFontHeight [ font metrics $tunableTitleFont -linespace ]
   #set descFontHeight [ font metrics $tunableDescriptionFont -linespace ]
   #set diffFontHeight [ expr $titleFontHeight - $descFontHeight ]

   set tunableDescriptionsMinWidth  256
   set tunableDescriptionsMinHeight 150

   set lastVisibleDescriptionsWidth 0
   set lastVisibleDescriptionsHeight 0

   toplevel .tunableDescriptions -class Paradyn
   wm title .tunableDescriptions "Tunable Constants Descriptions"
   # one does not pack a toplevel window

   frame .tunableDescriptions.bottom -relief groove
   pack  .tunableDescriptions.bottom -side bottom -fill x -expand false
      # expand is false; if the window is made taller, we don't want the extra height

   frame .tunableDescriptions.bottom.frame
   pack  .tunableDescriptions.bottom.frame -side bottom -fill y -expand true

   button .tunableDescriptions.bottom.frame.okay -text "OK" -command closeTunableDescriptions
   pack   .tunableDescriptions.bottom.frame.okay -side left -pady 6

   frame .tunableDescriptions.top
   pack  .tunableDescriptions.top -side top -fill both -expand true
      # expand is true; if the window is made taller, we want the extra height

   scrollbar .tunableDescriptions.top.scrollbar -orient vertical -width 16 \
	   -background gray -activebackground gray -relief sunken \
	   -command ".tunableDescriptions.top.canvas yview"
   pack      .tunableDescriptions.top.scrollbar -side left -fill y -expand false
      # expand is false; if the window is made wider, we don't want the extra width

   canvas .tunableDescriptions.top.canvas \
   	-yscrollcommand myDescriptionsScroll \
	-yscrollincrement 1 \
	-width 4i -height 3i
   pack propagate .tunableDescriptions.top.canvas false
   pack   .tunableDescriptions.top.canvas -side left -fill both -expand true
      # expand is true; we want extra width & height if window is resized

   wm minsize .tunableDescriptions $tunableDescriptionsMinWidth $tunableDescriptionsMinHeight

   if {$lastTDgeometry!=$unsetXgeometry} {
       #puts stderr "Recycling lastTDgeometry=$lastTDgeometry"
       wm geometry .tunableDescriptions $lastTDgeometry
   }

   drawTunableDescriptions
}

proc draw1TunableDescription {theName theDescription onlyDeveloperTunable} {
   global numTunableDescriptionsDrawn
   global tunableDescriptionsTextWidth
   global tunableDescriptionsTotalHeight
   global tunableTitleFont tunableDescriptionFont
   global devModeColor

   frame .tunableDescriptions.top.canvas.frame$numTunableDescriptionsDrawn
   pack  .tunableDescriptions.top.canvas.frame$numTunableDescriptionsDrawn \
	   -side top -fill x -expand true
      # we don't want extra height after resizing

   frame .tunableDescriptions.top.canvas.frame$numTunableDescriptionsDrawn.top
   pack  .tunableDescriptions.top.canvas.frame$numTunableDescriptionsDrawn.top \
	   -side top -fill x -expand false

   frame .tunableDescriptions.top.canvas.frame$numTunableDescriptionsDrawn.top.left
   pack  .tunableDescriptions.top.canvas.frame$numTunableDescriptionsDrawn.top.left \
	   -side left -expand false

   frame .tunableDescriptions.top.canvas.frame$numTunableDescriptionsDrawn.top.right
   pack  .tunableDescriptions.top.canvas.frame$numTunableDescriptionsDrawn.top.right \
	   -side right -fill x

   frame .tunableDescriptions.top.canvas.frame$numTunableDescriptionsDrawn.bottom
   pack  .tunableDescriptions.top.canvas.frame$numTunableDescriptionsDrawn.bottom \
	   -side top -fill x -expand true

   frame .tunableDescriptions.top.canvas.frame$numTunableDescriptionsDrawn.bottom.left \
           -width 20
   pack  .tunableDescriptions.top.canvas.frame$numTunableDescriptionsDrawn.bottom.left \
	   -side left

   frame .tunableDescriptions.top.canvas.frame$numTunableDescriptionsDrawn.bottom.right
   pack  .tunableDescriptions.top.canvas.frame$numTunableDescriptionsDrawn.bottom.right \
	   -side right -fill x -expand true

   label .tunableDescriptions.top.canvas.frame$numTunableDescriptionsDrawn.top.left.label \
      -text $theName -foreground "blue" -anchor sw
   if {$onlyDeveloperTunable} {
        .tunableDescriptions.top.canvas.frame$numTunableDescriptionsDrawn.top.left.label \
            config -foreground $devModeColor }

   pack  .tunableDescriptions.top.canvas.frame$numTunableDescriptionsDrawn.top.left.label \
      -side top -fill x -expand false
      # we don't want extra height after resizing

   set tunableDescriptionsTotalHeight [expr $tunableDescriptionsTotalHeight + \
        [getWindowHeight .tunableDescriptions.top.canvas.frame$numTunableDescriptionsDrawn.top.left.label]]

   message .tunableDescriptions.top.canvas.frame$numTunableDescriptionsDrawn.bottom.right.msg \
	 -justify left -anchor nw -text "$theDescription" \
         -width $tunableDescriptionsTextWidth -pady 0

   pack .tunableDescriptions.top.canvas.frame$numTunableDescriptionsDrawn.bottom.right.msg \
	 -side top -fill x -expand true
         # we don't want extra height after resizing

   set tunableDescriptionsTotalHeight [expr $tunableDescriptionsTotalHeight + \
        [getWindowHeight .tunableDescriptions.top.canvas.frame$numTunableDescriptionsDrawn.bottom.right.msg]]

   .tunableDescriptions.top.canvas create window \
	      0 $tunableDescriptionsTotalHeight \
	      -anchor sw \
	      -window .tunableDescriptions.top.canvas.frame$numTunableDescriptionsDrawn \
	      -tag description
	      
   incr numTunableDescriptionsDrawn
}

proc drawTunableDescriptions {} {
   global DeveloperModeFlag
   global numTunableDescriptionsDrawn
   global tunableDescriptionsTotalHeight
   global tunableDescriptionsTextWidth
   global tunableDescriptionsMinWidth
   global tunableDescriptionFont

   # delete old stuff...
   .tunableDescriptions.top.canvas delete description
   for {set lcv 0} {$lcv<$numTunableDescriptionsDrawn} {incr lcv} {
      destroy .tunableDescriptions.top.canvas.frame$lcv
   }

   set numTunableDescriptionsDrawn 0
   set tunableDescriptionsTotalHeight 0

   set tunableDescriptionsTextWidth \
        [ expr [getWindowWidth .tunableDescriptions.top.canvas] - 40 ]

   # First, draw boolean descriptions
   set allBoolNames [uimpd tclTunable getboolallnames]
   set numBoolNames [llength $allBoolNames]
   for {set lcv 0} {$lcv < $numBoolNames} {incr lcv} {
      set theName [lindex $allBoolNames $lcv]

      set theDescription [uimpd tclTunable getdescription $theName]

      set theUse [uimpd tclTunable getusebyname $theName]
      
      set developerTunableOnly [ string compare $theUse "developer" ]
      if {$developerTunableOnly==0 && $DeveloperModeFlag==0} continue

      draw1TunableDescription $theName $theDescription \
                  [expr ($developerTunableOnly==0 || \
	          ([ string compare $theName "developerMode" ] == 0 && \
                  $DeveloperModeFlag==1))]
   }      

   # Next, draw float descriptions
   set allFloatNames [uimpd tclTunable getfloatallnames]
   set numFloatNames [llength $allFloatNames]

   for {set lcv 0} {$lcv < $numFloatNames} {incr lcv} {
      set theName [lindex $allFloatNames $lcv]
      
      set theDescription [uimpd tclTunable getdescription $theName]
      set theUse [uimpd tclTunable getusebyname $theName]
      
      set developerTunableOnly [ string compare $theUse "developer" ]
      if {$developerTunableOnly==0 && $DeveloperModeFlag==0} continue

      draw1TunableDescription $theName $theDescription \
        [expr ($developerTunableOnly==0)]
   }      

   rethinkTunableDescriptionsScrollbarRegion
}

proc rethinkTunableDescriptionsScrollbarRegion {} {
   # Explicitly called by us.
   # Recalculates the canvas region and adjusts the scrollbar accordingly
   global tunableDescriptionsTotalHeight

   # update the scrollbar's scrollregion configuration
   set regionList {0 0 0 0}
   set regionList [lreplace $regionList 2 2 [getWindowWidth .tunableDescriptions.top.canvas]]
   set regionList [lreplace $regionList 3 3 $tunableDescriptionsTotalHeight]
   .tunableDescriptions.top.canvas configure -scrollregion $regionList

   set oldconfig [.tunableDescriptions.top.scrollbar get]
   set oldTotalHeight [lindex $oldconfig 0]

   if {$oldTotalHeight != $tunableDescriptionsTotalHeight} {
      set firstUnit 0
   } else {
      # no change
      set firstUnit [lindex $oldconfig 2]
   }

   set lastUnit [expr $firstUnit + $tunableDescriptionsTotalHeight - 1]
   .tunableDescriptions.top.scrollbar set $tunableDescriptionsTotalHeight \
	   [getWindowHeight .tunableDescriptions.top.canvas] \
	   $firstUnit $lastUnit
}

proc myDescriptionsScroll {left right} {
   # gets called whenever the canvas view changes or gets resized.
   # gets called on each movement of scrollbar (ack!)
   # we are supposed to rethink the scrollbar settings now.
   global lastVisibleDescriptionsHeight
   global lastVisibleDescriptionsWidth

   set newWidth  [getWindowWidth  .tunableDescriptions.top.canvas]
   set newHeight [getWindowHeight .tunableDescriptions.top.canvas]

   if {$lastVisibleDescriptionsHeight != $newHeight || \
       $lastVisibleDescriptionsWidth != $newWidth} {
      drawTunableDescriptions
   } else {
      .tunableDescriptions.top.scrollbar set $left $right
   }

   set lastVisibleDescriptionsHeight $newHeight
   set lastVisibleDescriptionsWidth $newWidth
}

proc closeTunableDescriptions {} {
   global numTunableDescriptionsDrawn
   global lastTDgeometry

   set numTunableDescriptionsDrawn 0
   set lastTDgeometry [wm geometry .tunableDescriptions]
   #puts stderr "Saving lastTDgeometry=$lastTDgeometry"

   destroy .tunableDescriptions
}

# ###################### Entrypoint Routine ####################

proc tunableEntryPoint {} {
   global numTunablesDrawn
   global nextStartY
   global integerScaleFactor
   global DeveloperModeFlag
   global lastVisibleWidth lastVisibleHeight
   global maxRowHeight
   global lastTCgeometry unsetXgeometry

   if {[winfo exists .tune]} {
      # tunable constants window already exists; let's de-iconify it
      # (if necessary) and raise it to the front of all other toplevel windows.
      wm deiconify .tune
      raise .tune

      return
   }

   set DeveloperModeFlag [uimpd tclTunable getvaluebyname developerMode]

   set lastVisibleWidth 0
   set lastVisibleHeight 0
   set maxRowHeight 0

   tunableInitialize

   gatherInitialTunableValues

   set numTunablesDrawn 0
   set nextStartY 0
   set integerScaleFactor 100          ;# was 20!

   # since the "goodMinWidth" depends on the maximum length of TC string names
   # (and that can be different depending whether we're in developerMode
   # we'll later expand the display width as necessary
   set goodMinWidth [drawTunables [getWindowWidth .tune.middle.canvas] \
                                  [getWindowHeight .tune.middle.canvas]]

   #puts stderr "Reading lastTCgeometry=$lastTCgeometry"
   if {$lastTCgeometry!=$unsetXgeometry} {
      set numscanned [scan $lastTCgeometry "%dx%d+%d+%d" \
                            oldWidth oldHeight oldx oldy]
      if {$numscanned==4} {
         if {$oldWidth < $goodMinWidth} {
            #puts stderr "Expanding lastTCgeometry width to $goodMinWidth"
            wm geometry .tune [format "%dx%d+%d+%d" \
                $goodMinWidth $oldHeight $oldx $oldy ]
         } else {
            #puts stderr "Recycling lastTCgeometry=$lastTCgeometry"
            wm geometry .tune $lastTCgeometry
         }
      } else {
         #puts stderr "tclTunable.tcl: could not scan geometry...won't try to resize"
      }
   } else {
      #puts stderr "No TC geometry set yet; we have free reign to size"
      set defaultHeight 330
      wm geometry .tune [format "%dx%d" $goodMinWidth $defaultHeight]
   }
}

proc tunableSwitchPoint {} {
   # destroy our toplevel windows (and all their subwindows)
   # and re-open them after switching developerMode
   global lastTCgeometry lastTDgeometry

   set lastTCgeometry [wm geometry .tune]
   #puts stderr "Saving lastTCgeometry=$lastTCgeometry"

   destroy .tune

   tunableEntryPoint 

   if {[winfo exists .tunableDescriptions]} {
      set lastTDgeometry [wm geometry .tunableDescriptions]
      #puts stderr "Saving lastTDgeometry=$lastTDgeometry"
      destroy .tunableDescriptions

      processShowTunableDescriptions
   }
}

proc tunableExitPoint {} {
   # destroy our toplevel windows (and all their subwindows)
   global lastTCgeometry lastTDgeometry

   set lastTCgeometry [wm geometry .tune]
   #puts stderr "Saving lastTCgeometry=$lastTCgeometry"

   destroy .tune

   if {[winfo exists .tunableDescriptions]} {
      set lastTDgeometry [wm geometry .tunableDescriptions]
      #puts stderr "Saving lastTDgeometry=$lastTDgeometry"
      destroy .tunableDescriptions
   }
}

