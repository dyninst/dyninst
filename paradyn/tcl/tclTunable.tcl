# tclTunable.tcl

# $Log: tclTunable.tcl,v $
# Revision 1.5  1994/10/31 08:53:04  tamches
# Eliminated "flicker" window startup.
# Added tunable descriptions and coordinated with tunable
# conststants window (e.g. when switch to and from developer
# mode, descriptions window is updated also, if open).
# Cleaned up options menu a bit
#
# Revision 1.4  1994/10/27  08:48:09  tamches
# Commented out help menu until a text widget bug is worked out
#
# Revision 1.3  1994/10/26  23:15:31  tamches
# Changed references of "tclTunable" to "uimpd tclTunable"
#
# Revision 1.2  1994/10/26  22:45:41  tamches
# first version
#

# #################### Some Misc Routines #####################
proc max {x y} {
   if {$x > $y} {
      return $x
   } else {
      return $y
   }
}

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
# #############################################################

proc tunableInitialize {} {
   global tunableMinWidth
   global tunableMinHeight
   global tunableMaxWidth
   global tunableMaxHeight

   set tunableMinWidth 275
   set tunableMinHeight 130
   set tunableMaxWidth 700
   set tunableMaxHeight 600

   toplevel .tune -class Tunable
   wm title .tune "Tunable Constants"
   # one does not pack a toplevel window...
   
   #  ################### Default options #################
   option add *Visi*font *-New*Century*Schoolbook-Bold-R-*-18-*
   option add *Data*font *-Helvetica-*-r-*-12-*
   option add *MyMenu*font *-New*Century*Schoolbook-Bold-R-*-14-*

   if {[string match [tk colormodel .] color] == 1} {
      # You have a color monitor...
      # change primary background color from 'bisque' to 'grey'
      .tune config -bg grey
      option add *Background grey
      option add *activeBackground LightGrey
      option add *activeForeground black
      option add *Scale.activeForeground grey
   } else {
      # You don't have a color monitor...
      option add *Background white
      option add *Foreground black
   }

   # .tune.top -- stuff at the top (menu bar, title bar, logo, other miscellanea)
   frame .tune.top
   pack  .tune.top -side top -fill x -expand false
      # expand is false; if the window is made taller, we don't want the extra height
   
   # .tune.top.logo -- paradyn logo
   label .tune.top.logo -relief raised \
                     -bitmap @/p/paradyn/core/paradyn/tcl/logo.xbm \
                     -foreground #b3331e1b53c7
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
   
   menubutton .tune.top.left.mbar.file -text File -menu .tune.top.left.mbar.file.m
   menu .tune.top.left.mbar.file.m
   .tune.top.left.mbar.file.m add command -label "Accept Change(s)" \
               -command processCommitFinalTunableValues
   .tune.top.left.mbar.file.m add command -label "Cancel" \
               -command processDiscardFinalTunableValues

   menubutton .tune.top.left.mbar.options -text Options -menu .tune.top.left.mbar.options.m
   menu .tune.top.left.mbar.options.m
   .tune.top.left.mbar.options.m add command -label "Enter Developer Mode" \
               -command processDeveloperModeChange

   menubutton .tune.top.left.mbar.help -text Help -menu .tune.top.left.mbar.help.m
   menu .tune.top.left.mbar.help.m
   .tune.top.left.mbar.help.m add command -label "Show Tunable Descriptions" \
               -command processShowTunableDescriptions

   pack .tune.top.left.mbar.options -side left -padx 4
   pack .tune.top.left.mbar.help -side right -padx 4
   tk_menuBar .tune.top.left.mbar .tune.top.left.mbar.options .tune.top.left.mbar.help

   # .tune.top.left.titlebar -- Title ("Tunable Constants") (above menu bar)
   label .tune.top.left.titlebar -text "Tunable Constants" -foreground white \
	   -background "cornflower blue" -font *-New*Century*Schoolbook-Bold-R-*-18-*
   pack  .tune.top.left.titlebar -side top -fill both -expand true
      # expand is true; we want to fill up .tune.top.left (which itself won't enlarge so OK)

   # .tune.bottom (buttons)
   frame .tune.bottom -relief sunken
   pack  .tune.bottom -side bottom -fill x -expand false -ipadx 4 -ipady 4
      # expand is false; if the window is made taller, we don't want the extra height
   
   button .tune.bottom.accept -text "Accept Change(s)" -anchor c \
           -command processCommitFinalTunableValues
   pack   .tune.bottom.accept -side left
   button .tune.bottom.discard -text "Cancel" -anchor c \
           -command processDiscardFinalTunableValues
   pack   .tune.bottom.discard -side right
   
   # .tune.middle -- body of the window (scrollbar & tunable constants canvas)
   frame .tune.middle
   pack  .tune.middle -side top -fill both -expand true
      # expand is true; we want extra width & height if the window is resized

   scrollbar .tune.middle.scrollbar -orient vertical -width 16 \
   	-foreground gray -activeforeground gray -relief sunken \
   	-command procNewScrollPosition
   pack      .tune.middle.scrollbar -side left -fill y -expand false
      # expand is false; if the window is made wider, we don't want the extra width
   
   canvas .tune.middle.canvas -relief flat \
   	-yscrollcommand myScroll -scrollincrement 1 \
	-width 3.5i -height 2.4i
   # the following line avoids flickering at the cost of starting off with
   # just the right window size.  To be more specific, it won't intelligently
   # set the size of the canvas subwindow to be exactly the sum of sizes of
   # the children.  Instead, it will go with whatever the initial value is
   pack propagate .tune.middle.canvas false
   pack   .tune.middle.canvas -side left -fill both -expand true
      # expand is true; we want extra height & width is the window is resized

   # without the following 2 lines, fvwm doesn't let us resize the window
   wm minsize .tune $tunableMinWidth $tunableMinHeight
   wm maxsize .tune $tunableMaxWidth $tunableMaxHeight
}

proc procNewScrollPosition {newTop} {
   # invoked to change the view of the canvas associated with this scrollbar.
   # called when the scrollbar is repositioned.
   # interestingly, the screen is not yet updated with the new scrollbar position!
   # that happens in "myScroll" (below) which is invoked as a result of the canvas
   # view change that we do here (...canvas.yview $newTop)
   if {$newTop < 0} {
      set newTop 0
   }

   set currSettings [.tune.middle.scrollbar get]
   set totalSize    [lindex $currSettings 0]
   set visibleSize  [lindex $currSettings 1]

   if {$visibleSize > $totalSize} {
      set newTop 0
   } elseif {[expr $newTop + $visibleSize] > $totalSize} {
      set newTop [expr $totalSize - $visibleSize]
   }

   # update the canvas
   # will automatically generate a canvas-yscroll command.  Only then does the
   # scrollbar setting actually change.
   .tune.middle.canvas yview $newTop
}

proc myScroll {totalSize visibleSize left right} {
   # gets called whenever the canvas view changes or gets resized.
   # This includes every scroll the user makes (yikes)
   # Gives us a chance to rethink the bounds of the scrollbar
   global lastVisibleHeight
   global lastVisibleWidth

#   puts stderr "Welcome to myScroll; total=$totalSize, visible=$visibleSize, left=$left, right=$right"

   set newWidth  [getWindowWidth  .tune.middle.canvas]
   set newHeight [getWindowHeight .tune.middle.canvas]
   
   if {$lastVisibleHeight != $newHeight || $lastVisibleWidth != $newWidth} {
#      puts stderr "myScroll: expensive drawTunables cuz lastVisibleHeight=$lastVisibleHeight<>newHeight=$newHeight or lastVisibleWidth=$lastVisibleWidth<>newWidth=$newWidth"
#      puts stderr "btw, totalSize=$totalSize left=$left right=$right"

      drawTunables $newWidth $newHeight
   } else {
      .tune.middle.scrollbar set $totalSize $visibleSize $left $right
   }

   set lastVisibleWidth $newWidth
   set lastVisibleHeight $newHeight
}

proc drawBoolTunable {lcv} {
   global nextStartY
   global numTunablesDrawn
   global newTunableValues

   set tunableName        [uimpd tclTunable getname $lcv]
   set tunableDescription [uimpd tclTunable getdescription $lcv]

   checkbutton .tune.middle.canvas.tunable$numTunablesDrawn -text $tunableName \
	   -variable newTunableValues($lcv) -anchor w
   pack .tune.middle.canvas.tunable$numTunablesDrawn -side top -fill x -expand false
      # expand is false; if the window is made taller, we don't want the extra height
#  place .tune.middle.canvas.tunable$numTunablesDrawn -x 0 -y $nextStartY
#       -anchor nw -relwidth 1.0

   .tune.middle.canvas create window 0 $nextStartY -anchor nw -tag tunableTag \
	   -width [getWindowWidth .tune.middle.canvas] \
	   -window .tune.middle.canvas.tunable$numTunablesDrawn

   set nextStartY [expr $nextStartY + [winfo reqheight .tune.middle.canvas.tunable$numTunablesDrawn]]

   incr numTunablesDrawn
}

proc everyChangeCommand {lcv newValue} {
   # A scale widget's value has changed.
   # We are passed the tunable index # and the new integer value.
 
   # I repeat: integer value
   # You may be wondering if this means it's impossible to do our scale
   # widgets, since we need floating point numbers.  Well, we use some
   # tricks to get around this limitation.  First of all, the scale widget
   # does not show any ticks.  That means the numbers can be whatever we
   # want them to be.  We choose to multiply the min and max by
   # $integerScaleFactor and then divide it back here...

   global newTunableValues
   global integerScaleFactor

   set newValue [expr 1.0 * $newValue / $integerScaleFactor]

   set newTunableValues($lcv) $newValue
   # This command automagically updates the entry widget because the
   # entry widget had its -textvariable set to this vrble
}

proc drawFloatTunable {lcv} {
   global nextStartY
   global numTunablesDrawn
   global newTunableValues
   global integerScaleFactor

   set tunableName        [uimpd tclTunable getname $lcv]
   set tunableDescription [uimpd tclTunable getdescription $lcv]
   set tunableBounds      [uimpd tclTunable getfloatrangebyindex $lcv]

   # if both 0.0, then as far as we're concerned, there are no min/max values.
   set tunableMin [lindex $tunableBounds 0]
   set tunableMax [lindex $tunableBounds 1]

   # sub-window (will hold scale and entry widgets)
   frame .tune.middle.canvas.tunable$numTunablesDrawn
   pack  .tune.middle.canvas.tunable$numTunablesDrawn -side top -fill x -expand false
      # expand is false; if the window is made taller, we don't want the extra height

   # entry widget
   # [other options to try: -font -width (# chars, not #pix)]
   entry .tune.middle.canvas.tunable$numTunablesDrawn.entry -relief sunken \
	   -textvariable newTunableValues($lcv) -width 8
   pack .tune.middle.canvas.tunable$numTunablesDrawn.entry -side right -expand false
      # expand is false; if the window is made taller, we don't want the extra height

   # label widget for the floating tunable's name
   label .tune.middle.canvas.tunable$numTunablesDrawn.label -relief flat \
	   -text $tunableName
   pack .tune.middle.canvas.tunable$numTunablesDrawn.label -side left -expand false
      # expand is false; if the window is made taller, we don't want the extra height

   # scale widget 
   if {$tunableMin!=0 || $tunableMax!= 0} {
      # [other options to try: -font -length -sliderlength -width -showValue]
      scale .tune.middle.canvas.tunable$numTunablesDrawn.scale \
	      -orient horizontal \
	      -relief flat \
	      -command "everyChangeCommand $lcv" \
	      -from [expr $integerScaleFactor * $tunableMin] \
	      -to   [expr $integerScaleFactor * $tunableMax] \
	      -showvalue false
      pack .tune.middle.canvas.tunable$numTunablesDrawn.scale -side right -expand false
         # expand is false; if the window is made taller, we don't want the extra height

      # initialize the scale setting
      .tune.middle.canvas.tunable$numTunablesDrawn.scale set [expr round($integerScaleFactor * $newTunableValues($lcv))]
   }

   set theHeight [winfo reqheight .tune.middle.canvas.tunable$numTunablesDrawn]
   if {$tunableMin!=0 || $tunableMax!= 0} {
      set theHeight [max $theHeight [winfo reqheight .tune.middle.canvas.tunable$numTunablesDrawn.scale]]
   }
   set theHeight [max $theHeight [winfo reqheight .tune.middle.canvas.tunable$numTunablesDrawn.entry]]

   .tune.middle.canvas create window 0 $nextStartY -anchor nw -tag tunableTag \
	    -width [getWindowWidth .tune.middle.canvas] \
	    -height $theHeight \
	    -window .tune.middle.canvas.tunable$numTunablesDrawn

   set nextStartY [expr $nextStartY + $theHeight]

   incr numTunablesDrawn
}

proc drawTunables {newWidth newHeight} {
   global numTunablesDrawn
   global nextStartY
   global DeveloperModeFlag

#   pack propagate .tune.middle.canvas false

#   puts stderr "Welcome to drawTunables; width=$newWidth, height=$newHeight"

   # First, erase old stuff on the screen
   .tune.middle.canvas delete tunableTag
   for {set lcv 0} {$lcv < $numTunablesDrawn} {incr lcv} {
      destroy .tune.middle.canvas.tunable$lcv
   }

   set numTunables [uimpd tclTunable getnumtunables]
   set numTunablesDrawn 0
   set nextStartY 0

   # make two passes---draw all boolean tunables, then all float tunables.
   # (looks nicer on screen that way...)
   for {set lcv 0} {$lcv < $numTunables} {incr lcv} {
      set tunableType [uimpd tclTunable gettypebyindex $lcv]
      set tunableUse  [uimpd tclTunable getusebyindex  $lcv]

      # If this tunable constant is a "developer" one, and if we
      # are not in developer mode, then skip it.
      if {$tunableUse=="developer" && $DeveloperModeFlag==0} continue

      if {$tunableType == "bool"} {
         drawBoolTunable $lcv
      }
   }
   for {set lcv 0} {$lcv < $numTunables} {incr lcv} {
      set tunableType [uimpd tclTunable gettypebyindex $lcv]
      set tunableUse  [uimpd tclTunable getusebyindex  $lcv]

      # If this tunable constant is a "developer" one, and if we
      # are not in developer mode, then skip it.
      if {$tunableUse=="developer" && $DeveloperModeFlag==0} continue

      if {$tunableType == "float"} {
         drawFloatTunable $lcv
      }
   }

   rethinkScrollBarRegion $newWidth $newHeight
#   pack propagate .tune.middle.canvas true
}

proc rethinkScrollBarRegion {newWidth newHeight} {
   # explicitly called by the program after putting up the tunable
   # constants (e.g. first time, after changing to/from developer mode)
   # We are passed how many pixels the tunables take up, and we adjust
   # the canvas' scrollregion and the scrollbar's settings accordingly.

   global nextStartY

#   puts stderr "welcome to rethinkScrollbarRegion; width=$newWidth, height=$newHeight"

   # update the scrollbar's scrollregion configuration
   set regionList {0 0 0 0}
   set regionList [lreplace $regionList 2 2 $newWidth]
   set regionList [lreplace $regionList 3 3 $nextStartY]
   .tune.middle.canvas configure -scrollregion $regionList

   set oldconfig [.tune.middle.scrollbar get]
   set oldTotalHeight [lindex $oldconfig 0]

   if {$oldTotalHeight != $nextStartY} {
      set firstUnit 0
   } else {
      # no change
      set firstUnit [lindex $oldconfig 2]
   }

   set lastUnit [expr $firstUnit + $nextStartY - 1]

   # args to "scrollbar set": total units, visible units, first unit, last unit
   .tune.middle.scrollbar set $nextStartY $newHeight $firstUnit $lastUnit
}

proc gatherInitialTunableValues {} {
   global origTunableValues
   global  newTunableValues

   set numTunables [uimpd tclTunable getnumtunables]

   for {set lcv 0} {$lcv<$numTunables} {incr lcv} {
      set tunableType [uimpd tclTunable gettypebyindex $lcv]

      set origTunableValues($lcv) [uimpd tclTunable getvaluebyindex $lcv]
      set newTunableValues($lcv) $origTunableValues($lcv)

#      puts stderr "initial value for #$lcv=$origTunableValues($lcv)"
   }
}

proc processCommitFinalTunableValues {} {
   global origTunableValues
   global  newTunableValues

   set numTunables [uimpd tclTunable getnumtunables]

   for {set lcv 0} {$lcv < $numTunables} {incr lcv} {
      if {$newTunableValues($lcv) != $origTunableValues($lcv)} {
         set tunableName [uimpd tclTunable getname $lcv]
         puts stderr "processFinalTunableValues: tunable #$lcv ($tunableName) has changed from $origTunableValues($lcv) to $newTunableValues($lcv)!"

         uimpd tclTunable setvaluebyindex $lcv $newTunableValues($lcv)
      }
   }
  
   tunableExitPoint
}

proc processDeveloperModeChange {} {
   global DeveloperModeFlag
   global tunableDescriptionsIsUp

   set DeveloperModeFlag [expr 1-$DeveloperModeFlag]

   if {$DeveloperModeFlag == 1} {
      .tune.top.left.mbar.options.m entryconfigure 1 -label "Leave Developer Mode"
      puts stderr "Warning: you are now in 'developer mode'"
   } else {
      .tune.top.left.mbar.options.m entryconfigure 1 -label "Enter Developer Mode"
      puts stderr "Leaving 'developer mode'..."
   }

   # simulate a configure event in the canvas
   set newWidth  [getWindowWidth  .tune.middle.canvas]
   set newHeight [getWindowHeight .tune.middle.canvas]

   drawTunables $newWidth $newHeight

   # and if the descriptions window is up, make some changes there too
   if {$tunableDescriptionsIsUp == 1} {
      drawTunableDescriptions
   }
}

proc processDiscardFinalTunableValues {} {
   tunableExitPoint
}

# ******************* Tunable Descriptions Stuff *****************

proc processShowTunableDescriptions {} {
   global tunableDescriptionsIsUp
   global numTunableDescriptionsDrawn
   global tunableDescriptionFont
   global tunableDescriptionsMinWidth
   global tunableDescriptionsMinHeight
   global tunableDescriptionsMaxWidth
   global tunableDescriptionsMaxHeight
   global lastVisibleDescriptionsWidth
   global lastVisibleDescriptionsHeight

   if {$tunableDescriptionsIsUp == 1} {
      puts stderr "tunable descriptions are already up!"
      return
   }
   set tunableDescriptionsIsUp 1

   set numTunableDescriptionsDrawn 0

   set tunableDescriptionFont *-Helvetica-*-r-*-12-*

   set tunableDescriptionsMinWidth  150
   set tunableDescriptionsMinHeight 150
   set tunableDescriptionsMaxWidth  800
   set tunableDescriptionsMaxHeight 800

   set lastVisibleDescriptionsWidth 0
   set lastVisibleDescriptionsHeight 0

   toplevel .tunableDescriptions -class TunableDescriptions
   wm title .tunableDescriptions "Tunable Descriptions"
   # one does not pack a toplevel window

   frame .tunableDescriptions.bottom -relief groove
   pack  .tunableDescriptions.bottom -side bottom -fill x -expand false
      # expand is false; if the window is made taller, we don't want the extra height

   button .tunableDescriptions.bottom.okay -text "Dismiss" -command closeTunableDescriptions
   pack   .tunableDescriptions.bottom.okay -side right

   frame .tunableDescriptions.top
   pack  .tunableDescriptions.top -side top -fill both -expand true
      # expand is true; if the window is made taller, we want the extra height

   scrollbar .tunableDescriptions.top.scrollbar -orient vertical -width 16 \
	   -foreground gray -activeforeground gray -relief sunken \
	   -command tunableDescriptionsNewScrollPosition
   pack      .tunableDescriptions.top.scrollbar -side left -fill y -expand false
      # expand is false; if the window is made wider, we don't want the extra width

   canvas .tunableDescriptions.top.canvas \
   	-yscrollcommand myDescriptionsScroll -relief flat \
	-scrollincrement 1 \
	-width 4i -height 2.2i
   pack propagate .tunableDescriptions.top.canvas false
   pack   .tunableDescriptions.top.canvas -side left -fill both -expand true
      # expand is true; we want extra width & height if window is resized

   wm minsize .tunableDescriptions $tunableDescriptionsMinWidth $tunableDescriptionsMinHeight
   wm maxsize .tunableDescriptions $tunableDescriptionsMaxWidth $tunableDescriptionsMaxHeight

   drawTunableDescriptions
}

proc draw1TunableDescription {lcv} {
   global numTunableDescriptionsDrawn
   global tunableDescriptionsTotalHeight
   global tunableDescriptionFont

   set tunableName [uimpd tclTunable getname $lcv]
   set tunableDescription [uimpd tclTunable getdescription $lcv]

   frame .tunableDescriptions.top.canvas.frame$numTunableDescriptionsDrawn
   pack  .tunableDescriptions.top.canvas.frame$numTunableDescriptionsDrawn \
	   -side top -fill x -expand false
      # we don't want extra height after resizing

   frame .tunableDescriptions.top.canvas.frame$numTunableDescriptionsDrawn.top
   pack  .tunableDescriptions.top.canvas.frame$numTunableDescriptionsDrawn.top \
	   -side top -fill x -expand false

   frame .tunableDescriptions.top.canvas.frame$numTunableDescriptionsDrawn.top.left
   pack  .tunableDescriptions.top.canvas.frame$numTunableDescriptionsDrawn.top.left \
	   -side left

   frame .tunableDescriptions.top.canvas.frame$numTunableDescriptionsDrawn.top.right
   pack  .tunableDescriptions.top.canvas.frame$numTunableDescriptionsDrawn.top.right \
	   -side right -fill x

   frame .tunableDescriptions.top.canvas.frame$numTunableDescriptionsDrawn.bottom
   pack  .tunableDescriptions.top.canvas.frame$numTunableDescriptionsDrawn.bottom \
	   -side top -fill x -expand false

   frame .tunableDescriptions.top.canvas.frame$numTunableDescriptionsDrawn.bottom.left -width 20
   pack  .tunableDescriptions.top.canvas.frame$numTunableDescriptionsDrawn.bottom.left \
	   -side left

   frame .tunableDescriptions.top.canvas.frame$numTunableDescriptionsDrawn.bottom.right
   pack  .tunableDescriptions.top.canvas.frame$numTunableDescriptionsDrawn.bottom.right \
	   -side right -fill x -expand false

   message .tunableDescriptions.top.canvas.frame$numTunableDescriptionsDrawn.top.left.label \
      -text $tunableName -foreground "cornflower blue" -justify left -width 3i \
      -font $tunableDescriptionFont
   pack  .tunableDescriptions.top.canvas.frame$numTunableDescriptionsDrawn.top.left.label \
      -side top -fill x -expand false
      # we don't want extra height after resizing

   set tunableDescriptionsTotalHeight [expr $tunableDescriptionsTotalHeight + [getWindowHeight .tunableDescriptions.top.canvas.frame$numTunableDescriptionsDrawn.top.left.label]]

   message .tunableDescriptions.top.canvas.frame$numTunableDescriptionsDrawn.bottom.right.msg \
	 -width 3i -justify left -text $tunableDescription \
	 -font $tunableDescriptionFont
   pack .tunableDescriptions.top.canvas.frame$numTunableDescriptionsDrawn.bottom.right.msg \
	 -side top -fill x -expand false
         # we don't want extra height after resizing

   set tunableDescriptionsTotalHeight [expr $tunableDescriptionsTotalHeight + [getWindowHeight .tunableDescriptions.top.canvas.frame$numTunableDescriptionsDrawn.bottom.right.msg]]

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
   global tunableDescriptionFont
   
   # delete old stuff...
   .tunableDescriptions.top.canvas delete description
   for {set lcv 0} {$lcv<$numTunableDescriptionsDrawn} {incr lcv} {
      destroy .tunableDescriptions.top.canvas.frame$lcv
   }

   set numTunableDescriptionsDrawn 0
   set tunableDescriptionsTotalHeight 0

   set numTunables [uimpd tclTunable getnumtunables]

   # to keep in sync with .tune, we draw boolean ones first
   for {set lcv 0} {$lcv < $numTunables} {incr lcv} {
      set tunableUse  [uimpd tclTunable getusebyindex  $lcv]
      if {$tunableUse=="developer" && $DeveloperModeFlag==0} continue

      set tunableType [uimpd tclTunable gettypebyindex $lcv]

      if {$tunableType == "bool"} {
         draw1TunableDescription $lcv
      }
   }
   for {set lcv 0} {$lcv < $numTunables} {incr lcv} {
      set tunableUse  [uimpd tclTunable getusebyindex  $lcv]
      if {$tunableUse=="developer" && $DeveloperModeFlag==0} continue

      set tunableType [uimpd tclTunable gettypebyindex $lcv]

      if {$tunableType == "float"} {
         draw1TunableDescription $lcv
      }
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

proc myDescriptionsScroll {totalSize visibleSize left right} {
   # gets called whenever the canvas view changes or gets resized.
   # gets called on each movement of scrollbar (ack!)
   # we are supposed to rethink the scrollbar settings now.
   global lastVisibleDescriptionsHeight
   global lastVisibleDescriptionsWidth

#   puts stderr "Welcome to myDescriptionsScroll; total=$totalSize, visible=$visibleSize, left=$left, right=$right"

   set newWidth  [getWindowWidth  .tunableDescriptions.top.canvas]
   set newHeight [getWindowHeight .tunableDescriptions.top.canvas]

   if {$lastVisibleDescriptionsHeight != $newHeight || $lastVisibleDescriptionsWidth != $newWidth} {
      puts stderr "myDescriptionsScroll: performing expensive redraw"
      drawTunableDescriptions
   } else {
      .tunableDescriptions.top.scrollbar set $totalSize $visibleSize $left $right
   }

   set lastVisibleDescriptionsHeight $newHeight
   set lastVisibleDescriptionsWidth $newWidth
}

proc tunableDescriptionsNewScrollPosition {newTop} {
   # invoked to change the view of the canvas associated with this scrollbar.
   # called when the scrollbar is repositioned.
   # interestingly, the screen is not yet updated with the new scrollbar position!
   # that happens in "myScroll" (below) which is invoked as a result of the canvas
   # view change that we do here (...canvas.yview $newTop)

   if {$newTop < 0} {
      set newTop 0
   }

   set currSettings [.tunableDescriptions.top.scrollbar get]
   set totalSize    [lindex $currSettings 0]
   set visibleSize  [lindex $currSettings 1]

   if {$visibleSize > $totalSize} {
      set newTop 0
   } elseif {[expr $newTop + $visibleSize] > $totalSize} {
      set newTop [expr $totalSize - $visibleSize]
   }

   # update the canvas
   # will automatically generate a canvas-yscroll command.  Only then does the
   # scrollbar setting actually change.
   .tunableDescriptions.top.canvas yview $newTop
}

proc closeTunableDescriptions {} {
   global tunableDescriptionsIsUp
   global numTunableDescriptionsDrawn

   set numTunableDescriptionsDrawn 0

   set tunableDescriptionsIsUp 0
   destroy .tunableDescriptions
}

# ###################### Entrypoint Routine ####################

proc tunableEntryPoint {} {
   global numTunablesDrawn
   global nextStartY
   global integerScaleFactor
   global DeveloperModeFlag
   global tunableDescriptionsIsUp
   global lastVisibleWidth
   global lastVisibleHeight

   set DeveloperModeFlag 0

   set lastVisibleWidth 0
   set lastVisibleHeight 0

   set tunableDescriptionsIsUp 0

   tunableInitialize

   gatherInitialTunableValues
   set numTunablesDrawn 0
   set nextStartY 0
   set integerScaleFactor 20
   set developerModeFlag 0

   set newWidth  [getWindowWidth  .tune.middle.canvas]
   set newHeight [getWindowHeight .tune.middle.canvas]

   drawTunables $newWidth $newHeight
}

proc tunableExitPoint {} {
   global numTunablesDrawn
   global nextStartY
   global integerScaleFactor
   global DeveloperModeFlag
   global origTunableValues
   global  newTunableValues
   global tunableMinWidth
   global tunableMinHeight
   global tunableMaxWidth
   global tunableMaxHeight
   global tunableDescriptionsIsUp

   # destroy our toplevel windows (and all their subwindows)
   destroy .tune

   if {$tunableDescriptionsIsUp == 1} {
      set tunableDescriptionsIsUp false
      destroy .tunableDescriptions
   }

   unset numTunablesDrawn nextStartY integerScaleFactor origTunableValues newTunableValues
   unset DeveloperModeFlag tunableMaxWidth tunableMaxHeight tunableDescriptionsIsUp
}
