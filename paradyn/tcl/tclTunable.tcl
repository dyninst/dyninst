# tclTunable.tcl

# $Log: tclTunable.tcl,v $
# Revision 1.4  1994/10/27 08:48:09  tamches
# Commented out help menu until a text widget bug is worked out
#
# Revision 1.3  1994/10/26  23:15:31  tamches
# Changed references of "tclTunable" to "uimpd tclTunable"
#
# Revision 1.2  1994/10/26  22:45:41  tamches
# first version
#

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
   
   # .tune.top.logo -- paradyn logo
   label .tune.top.logo -relief raised \
                     -bitmap @/p/paradyn/core/paradyn/tcl/logo.xbm \
                     -foreground #b3331e1b53c7
   pack  .tune.top.logo -side right -fill y
   
   # .tune.top.left -- stuff to the left of the logo (title bar & menu bar)
   frame .tune.top.left
   pack  .tune.top.left -side left -fill both -expand true
   
   label .tune.top.left.titlebar -text "Tunable Constants" -foreground white -background "cornflower blue" -font *-New*Century*Schoolbook-Bold-R-*-18-*
   pack  .tune.top.left.titlebar -side top -fill both -expand true
   
   frame .tune.top.left.mbar -borderwidth 2 -relief raised
   pack  .tune.top.left.mbar -side bottom -fill x
   
   menubutton .tune.top.left.mbar.file -text File -menu .tune.top.left.mbar.file.m
   menu .tune.top.left.mbar.file.m
   .tune.top.left.mbar.file.m add command -label "Accept Change(s)" -command processCommitFinalTunableValues
   .tune.top.left.mbar.file.m add command -label "Cancel" -command processDiscardFinalTunableValues

   menubutton .tune.top.left.mbar.options -text Options -menu .tune.top.left.mbar.options.m
   menu .tune.top.left.mbar.options.m
   .tune.top.left.mbar.options.m add checkbutton -label "Developer Mode" -command processDeveloperModeChange \
	   -variable DeveloperModeFlag

   menubutton .tune.top.left.mbar.help -text Help -menu .tune.top.left.mbar.help.m
   menu .tune.top.left.mbar.help.m
   .tune.top.left.mbar.help.m add command -label "Show Tunable Descriptions" -command processShowTunableDescriptions -state disabled

   pack .tune.top.left.mbar.options -side left -padx 4
   pack .tune.top.left.mbar.help -side right -padx 4
   tk_menuBar .tune.top.left.mbar .tune.top.left.mbar.options .tune.top.left.mbar.help

   # .tune.bottom (buttons)
   frame .tune.bottom -relief sunken
   pack  .tune.bottom -side bottom -fill x -expand false -ipadx 4 -ipady 4
   
   button .tune.bottom.accept -text "Accept Change(s)" -anchor c -command processCommitFinalTunableValues
   pack   .tune.bottom.accept -side left
   button .tune.bottom.discard -text "Cancel" -anchor c -command processDiscardFinalTunableValues
   pack   .tune.bottom.discard -side right
   
   # .tune.middle -- body of the window (scrollbar & tunable constants canvas)
   frame .tune.middle
   pack  .tune.middle -side top -fill both -expand true

   scrollbar .tune.middle.scrollbar -orient vertical -width 16 \
   	-foreground gray -activeforeground gray -relief sunken \
   	-command procNewScrollPosition
   pack      .tune.middle.scrollbar -side left -fill y -expand false
   
   canvas .tune.middle.canvas -relief flat \
   	-yscrollcommand myScroll -scrollincrement 10
   pack   .tune.middle.canvas -side bottom -fill both -expand true
   
   wm minsize .tune $tunableMinWidth $tunableMinHeight
   wm maxsize .tune $tunableMaxWidth $tunableMaxHeight
   wm title .tune "Tunable Constants"
}

proc procNewScrollPosition {newTop} {
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
   # we are supposed to rethink the scrollbar settings now.
   global lastVisibleSize

#   puts stderr "Welcome to myScroll; total=$totalSize, visible=$visibleSize, left=$left, right=$right"

   if {$lastVisibleSize != $visibleSize} {
      drawTunables [getWindowWidth .tune.middle.canvas] [getWindowHeight .tune.middle.canvas] 
      set lastVisibleSize $visibleSize
   }

   .tune.middle.scrollbar set $totalSize $visibleSize $left $right
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

proc drawBoolTunable {lcv} {
   global nextStartY
   global numTunablesDrawn
   global newTunableValues

   set tunableName        [uimpd tclTunable getname $lcv]
   set tunableDescription [uimpd tclTunable getdescription $lcv]

   checkbutton .tune.middle.canvas.tunable$numTunablesDrawn -text $tunableName \
	   -variable newTunableValues($lcv) \
	   -anchor w
   pack .tune.middle.canvas.tunable$numTunablesDrawn -side top -fill x
#  place .tune.middle.canvas.tunable$numTunablesDrawn -x 0 -y $nextStartY -anchor nw -relwidth 1.0

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

proc max {x y} {
   if {$x > $y} {
      return $x
   } else {
      return $y
   }
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

   # sub-window (will hold scale and entry widgets
   frame .tune.middle.canvas.tunable$numTunablesDrawn
   pack  .tune.middle.canvas.tunable$numTunablesDrawn -side top -fill x -expand true

   # entry widget
   # other options to try: -font -width (# chars, not #pix)
   entry .tune.middle.canvas.tunable$numTunablesDrawn.entry -relief sunken \
	   -textvariable newTunableValues($lcv) \
	   -width 8
   pack .tune.middle.canvas.tunable$numTunablesDrawn.entry -side right -expand false

   # label widget for the floating tunable's name
   label .tune.middle.canvas.tunable$numTunablesDrawn.label -relief flat \
	   -text $tunableName
   pack .tune.middle.canvas.tunable$numTunablesDrawn.label -side left -expand false

   # scale widget 
   if {$tunableMin!=0 || $tunableMax!= 0} {
      # other options to try: -font -length -sliderlength -width -showValue
      scale .tune.middle.canvas.tunable$numTunablesDrawn.scale -orient horizontal \
	      -relief flat \
	      -command "everyChangeCommand $lcv" \
	      -from [expr $integerScaleFactor * $tunableMin] \
	      -to   [expr $integerScaleFactor * $tunableMax] \
	      -showvalue false
      pack .tune.middle.canvas.tunable$numTunablesDrawn.scale -side right
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

#   puts stderr "Welcome to drawTunables; width=$newWidth, height=$newHeight"

   # First, erase old stuff on the screen
   for {set lcv 0} {$lcv < $numTunablesDrawn} {incr lcv} {
      destroy .tune.middle.canvas.tunable$lcv
   }
   .tune.middle.canvas delete tunableTag

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
}

proc rethinkScrollBarRegion {newWidth newHeight} {
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

   if {$DeveloperModeFlag == 1} {
      puts stderr "Warning: you are now in 'developer mode'"
   } else {
      puts stderr "Leaving 'developer mode'..."
   }

   # simulate a configure event in the canvas
   set newWidth [getWindowWidth .tune.middle.canvas]
   set newHeight [getWindowHeight .tune.middle.canvas]

   drawTunables $newWidth $newHeight
   rethinkScrollBarRegion $newWidth $newHeight
}

proc processDiscardFinalTunableValues {} {
   tunableExitPoint
}

proc drawTunableDescriptions {} {
   global DeveloperModeFlag

   set numTunables [uimpd tclTunable getnumtunables]

   puts stderr "Welcome to drawTunableDescriptions; numTunables=$numTunables"

   for {set lcv 0} {$lcv < $numTunables} {incr lcv} {
      set tunableUse  [uimpd tclTunable getusebyindex  $lcv]
      if {$tunableUse=="developer" && $DeveloperModeFlag==0} continue

      puts stderr "Going to draw description #$lcv"

      set tunableName        [uimpd tclTunable getname $lcv]
      set tunableDescription [uimpd tclTunable getdescription $lcv]

      # in name tag:
      .tunableDescriptions.top.text insert current $tunableName
      .tunableDescriptions.top.text insert current "\n\n"

      .tunableDescriptions.top.text insert current $tunableDescription
      .tunableDescriptions.top.text insert current "\n\n"
   }

   for {set lcv 0} {$lcv < $numTunables} {incr lcv} {
      set tunableUse  [uimpd tclTunable getusebyindex  $lcv]
      if {$tunableUse=="developer" && $DeveloperModeFlag==0} continue

      set tunableName        [uimpd tclTunable getname $lcv]
      set tunableDescription [uimpd tclTunable getdescription $lcv]

      # in name tag:
      set startline [expr 1+(4*$lcv)]
      set endcol    [expr [string length $tunableName] - 1]
      .tunableDescriptions.top.text tag add nametag \
	      @$startline,0 @$startline,$endcol
      
      # in description tag:      
      set startline [expr 3+(4*$lcv)]
      set endcol  [expr [string length $tunableDescription] - 1]
      .tunableDescriptions.top.text tag add descriptiontag @$startline,0 @$startline,$endcol
   }
}

proc processShowTunableDescriptions {} {
   global tunableDescriptionsIsUp
   global tunableDescriptionFont

   toplevel .tunableDescriptions -class TunableDescriptions

   set tunableDescriptionsIsUp 1

   frame .tunableDescriptions.bottom
   pack  .tunableDescriptions.bottom -side bottom -fill x

   button .tunableDescriptions.bottom.okay -text "Dismiss" -command closeTunableDescriptions
   pack   .tunableDescriptions.bottom.okay -side right

   frame .tunableDescriptions.top
   pack  .tunableDescriptions.top -side top

   scrollbar .tunableDescriptions.top.scrollbar -orient vertical -width 16 \
	   -foreground gray -activeforeground gray -relief sunken \
	   -command tunableDescriptionsNewScrollPosition
   pack      .tunableDescriptions.top.scrollbar -side left -fill y

   text .tunableDescriptions.top.text -width 40 \
	   -wrap word
   pack   .tunableDescriptions.top.text -side right

   .tunableDescriptions.top.text tag configure nametag 
   .tunableDescriptions.top.text tag configure descriptiontag -font $tunableDescriptionFont

   global DeveloperModeFlag

   set numTunables [uimpd tclTunable getnumtunables]

   puts stderr "Welcome to drawTunableDescriptions; numTunables=$numTunables"

#   drawTunableDescriptions
   for {set lcv 0} {$lcv < $numTunables} {incr lcv} {
      set tunableUse  [uimpd tclTunable getusebyindex  $lcv]
      if {$tunableUse=="developer" && $DeveloperModeFlag==0} continue

      puts stderr "Going to draw description #$lcv"

      set tunableName        [uimpd tclTunable getname $lcv]
      set tunableDescription [uimpd tclTunable getdescription $lcv]

      # in name tag:
      .tunableDescriptions.top.text insert current $tunableName
      .tunableDescriptions.top.text insert current "\n\n"

      .tunableDescriptions.top.text insert current $tunableDescription
      .tunableDescriptions.top.text insert current "\n\n"
   }

   for {set lcv 0} {$lcv < $numTunables} {incr lcv} {
      set tunableUse  [uimpd tclTunable getusebyindex  $lcv]
      if {$tunableUse=="developer" && $DeveloperModeFlag==0} continue

      set tunableName        [uimpd tclTunable getname $lcv]
      set tunableDescription [uimpd tclTunable getdescription $lcv]

      # in name tag:
      set startline [expr 1+(4*$lcv)]
      set endcol    [expr [string length $tunableName] - 1]
      .tunableDescriptions.top.text tag add nametag \
	      @$startline,0 @$startline,$endcol
      
      # in description tag:      
      set startline [expr 3+(4*$lcv)]
      set endcol  [expr [string length $tunableDescription] - 1]
      .tunableDescriptions.top.text tag add descriptiontag @$startline,0 @$startline,$endcol
   }

   puts stderr [.tunableDescriptions.top.text tag names]
   puts stderr [.tunableDescriptions.top.text tag ranges -nametag]
   puts stderr [.tunableDescriptions.top.text tag ranges -descriptiontag]


   # forbid further modifications (e.g. user type-ins!)
   .tunableDescriptions.top.text configure -state disabled

   set maxTunableDescriptionsHeight 500
//   wm maxsize .tunableDescriptions [getWindowWidth .tunableDescriptions] $maxTunableDescriptionsHeight
}

proc tunableDescriptionsNewScrollPosition {newTop} {
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
   .tunableDescriptions.top.scrollbar yview $newTop
}

proc tunableDescriptionsNewScroll {totalSize visibleSize left right} {
   # gets called whenever the canvas view changes or gets resized.
   # we are supposed to rethink the scrollbar settings now.

   .tunableDescriptions.top.scrollbar set $totalSize $visibleSize $left $right
}

proc closeTunableDescriptions {} {
   global tunableDescriptionsIsUp

   set tunableDescriptionsIsUp 0
   destroy .tunableDescriptions
}


proc tunableEntryPoint {} {
   global numTunablesDrawn
   global nextStartY
   global integerScaleFactor
   global DeveloperModeFlag
   global tunableDescriptionsIsUp
   global lastVisibleSize
   global tunableDescriptionFont

   set lastVisibleSize 0
   set tunableDescriptionsIsUp 0

   tunableInitialize

   gatherInitialTunableValues
   set numTunablesDrawn 0
   set nextStartY 0
   set integerScaleFactor 20
   set developerModeFlag 0
   set tunableDescriptionFont *-Helvetica-*-r-*-12-*

   set newWidth [getWindowWidth .tune.middle.canvas]
   set newHeight [getWindowHeight .tune.middle.canvas]

   drawTunables $newWidth $newHeight
   rethinkScrollBarRegion $newWidth $newHeight
   
#   bind .tune.middle.canvas <Configure> {tunableConfigureEventHandler %w %h}
}

proc tunableExitPoint {} {
   global numTunablesDrawn
   global nextStartY
   global integerScaleFactor
   global origTunableValues
   global  newTunableValues
   global DeveloperModeFlag
   global tunableMinWidth
   global tunableMinHeight
   global tunableMaxWidth
   global tunableMaxHeight
   global tunableDescriptionsIsUp
   global tunableDescriptionFont

   # destroy our toplevel windows (and all their subwindows)
   destroy .tune

   if {$tunableDescriptionsIsUp == 1} {
      set tunableDescriptionsIsUp false
      destroy .tunableDescriptions
   }

   unset numTunablesDrawn nextStartY integerScaleFactor origTunableValues newTunableValues
   unset DeveloperModeFlag tunableMaxWidth tunableMaxHeight tunableDescriptionsIsUp
   unset tunableDescriptionFont
}
