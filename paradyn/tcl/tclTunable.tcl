# tclTunable.tcl

# $Log: tclTunable.tcl,v $
# Revision 1.12  1995/07/02 20:01:08  tamches
# port to tk4.0
#
# Revision 1.11  1995/02/27  18:38:28  tamches
# Changes to reflect the new TCthread and, as a result, the extensively
# revised tclTunable.C
#
# Revision 1.10  1994/11/11  07:00:16  tamches
# fixed a bug that would change the background of all future windows
# to grey.  In other words, the "option add ..." in tcl was affecting
# more than just tunable windows, which was not nice.
#
# Revision 1.9  1994/11/08  06:09:57  tamches
# An entire float tunable gets "hilited" when the mouse enters, just like
# a checkbutton.
# Added numbering information to the left and right of the scale
# Improved sizing algorithms.  Now have a good minsize.
#
# Revision 1.8  1994/11/04  15:57:45  tamches
# Developmode flag is now read from the "developerMode" tc, and is treated
# as any other tc.  "Enter Developer Mode" menu removed; only the help
# menu remains.
#
# Centered "dismiss" button in tunable descriptions
#
# Revision 1.7  1994/11/02  19:57:45  tamches
# Improved look by going to helvetica 14 font.
# Names are now aligned (i.e. bools and floats each have their
# names on the left).  This required some hacking of checkbuttons
# for the boolean tunables.
# Tunable descriptions completed.
#
# Revision 1.6  1994/10/31  22:02:30  tamches
# Typing return/enter/tab in one of the entry widgets for floating
# tunables is suppresses (return would enter \0x0d which is especially ugly)
#
# Revision 1.5  1994/10/31  08:53:04  tamches
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

# To do list:
# 1) if user deletes tunable descriptions window (using window mgr), then
#    we'll get confused because tunable descriptions will still be true.
#    Would like to detect when a window is destroyed, and set tunable descriptions
#    false.  Something tells me this is doable in tcl.

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

   toplevel .tune -class Tunable
   wm title .tune "Tunable Constants"
   # one does not pack a toplevel window...
   
   #  ################### Default options #################
   option add *Visi*font *-New*Century*Schoolbook-Bold-R-*-18-*
   option add *Data*font *-Helvetica-*-r-*-12-*
   option add *MyMenu*font *-New*Century*Schoolbook-Bold-R-*-14-*

   if {[winfo depth .] > 1} {
      # You have a color monitor...
      # change primary background color from 'bisque' to 'grey'
      option add *tune*Background grey
      option add *tune*activeBackground LightGrey
      option add *tune*activeForeground black
      option add *tune*Scale.activeForeground grey

      option add *tunableDescriptions*Background grey
      option add *tunableDescriptions*activeBackground LightGrey
      option add *tunableDescriptions*activeForeground black
      option add *tunableDescriptions*Scale.activeForeground grey
   } else {
      # You don't have a color monitor...
      option add *tune*Background white
      option add *tune*Foreground black

      option add *tunableDescriptions*Background white
      option add *tunableDescriptions*Foreground black
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
   
   menubutton .tune.top.left.mbar.help -text Help -menu .tune.top.left.mbar.help.m
   menu .tune.top.left.mbar.help.m
   .tune.top.left.mbar.help.m add command -label "Show Tunable Descriptions" \
               -command processShowTunableDescriptions

   pack .tune.top.left.mbar.help -side right -padx 4
   tk_menuBar .tune.top.left.mbar .tune.top.left.mbar.help

   # .tune.top.left.titlebar -- Title ("Tunable Constants") (above menu bar)
   label .tune.top.left.titlebar -text "Tunable Constants" -foreground white \
	   -background "cornflower blue" -font *-New*Century*Schoolbook-Bold-R-*-14-*
   pack  .tune.top.left.titlebar -side top -fill both -expand true
      # expand is true; we want to fill up .tune.top.left (which itself won't enlarge so OK)

   # .tune.bottom (buttons)
   frame .tune.bottom -relief sunken
   pack  .tune.bottom -side bottom -fill x -expand false -ipadx 4 -ipady 4
      # expand is false; if the window is made taller, we don't want the extra height

   frame  .tune.bottom.buttonFrame
   pack   .tune.bottom.buttonFrame -side top -fill y -expand true

   button .tune.bottom.buttonFrame.accept -text "Accept" -anchor c \
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
   
   canvas .tune.middle.canvas -relief flat -yscrollcommand myScroll -yscrollincrement 1
   pack   .tune.middle.canvas -side left -fill both -expand true

   frame .tune.middle.canvas.names
   frame .tune.middle.canvas.values

   # the following line avoids flickering, but there is a cost: the window won't start off with
   # exactly the right window size.  To be more specific, it won't intelligently
   # set the size of the canvas subwindow to be exactly the sum of sizes of
   # the children.  Instead, it will go with whatever the initial value is
   pack propagate .tune.middle.canvas false
   pack   .tune.middle.canvas -side left -fill both -expand true
      # expand is true; we want extra height & width is the window is resized
}

proc myScroll {left right} {
   # gets called whenever the canvas view changes or gets resized.
   # This includes every scroll the user makes (yikes)
   # Gives us a chance to rethink the bounds of the scrollbar
   global lastVisibleHeight lastVisibleWidth

   set newWidth  [getWindowWidth  .tune.middle.canvas]
   set newHeight [getWindowHeight .tune.middle.canvas]
   
   if {$lastVisibleHeight != $newHeight || $lastVisibleWidth != $newWidth} {
#      puts stderr "myScroll: expensive drawTunables cuz lastVisibleHeight=$lastVisibleHeight<>newHeight=$newHeight or lastVisibleWidth=$lastVisibleWidth<>newWidth=$newWidth"
#      puts stderr "btw, totalSize=$totalSize left=$left right=$right"

      drawTunables $newWidth $newHeight
   } else {
      .tune.middle.scrollbar set $left $right
   }

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
   set buttonLabelWin .tune.middle.canvas.names.tunable$lcv.label
   set dummyLabelWin .tune.middle.canvas.values.tunable$lcv.dummy
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
proc drawBoolTunable {theName} {
   global nextStartY
   global namesWidth
   global numTunablesDrawn

   # the following important vrbles are (associative) arrays (indexed by name) of
   # boolean tunable constant descriptions and newvalues.
   global boolTunableDescriptionU boolTunableNewValues

   set namesWin  .tune.middle.canvas.names.tunable$numTunablesDrawn
   set valuesWin .tune.middle.canvas.values.tunable$numTunablesDrawn

   frame $namesWin
   pack  $namesWin -side top -fill x -expand true

   frame $valuesWin
   pack  $valuesWin -side top -fill x -expand true

   set buttonLabelWin $namesWin.label
   set valuesLabelWin $valuesWin.box
   
   set labelFont *-Helvetica-*-r-*-14-*

   # dummy label.  At first, I used a checkbutton to guarantee that
   # all 3 widgets would have the same height.  But using
   # -highlightthickness 0 for the realcheckbutton made them all the
   # same anyway in tk 4.0
   label $valuesWin.dummy -relief flat -font $labelFont 
   pack  $valuesWin.dummy -side left -fill y

   # In order to get the appearance of a checkbutton with the on/off red square
   # on the right instead of on the left, we use 2 labels & a checkbutton.
   # The second one is the checkbutton, it has an indicator but no text.

   label $buttonLabelWin -text $theName -anchor w -relief flat -font $labelFont
   pack  $buttonLabelWin -side left -fill x -expand true

   checkbutton $valuesLabelWin -variable boolTunableNewValues($theName) -anchor w \
	   -relief flat -font $labelFont -highlightthickness 0 -selectcolor blue
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

   set theHeight [max [getWindowHeight $leftButton] [getWindowHeight $rightButton]]

   set nextStartY [expr $nextStartY + $theHeight]
   incr numTunablesDrawn
}

proc everyChangeCommand {name newValue} {
   # A scale widget's value has changed.
   # We are passed the tunable index # and the new integer value.
 
   # I repeat: integer value
   # You may be wondering if this means it's impossible to do our scale
   # widgets, since we need floating point numbers.  Well, we use some
   # tricks to get around this limitation.  First of all, the scale widget
   # does not show any ticks.  That means the numbers can be whatever we
   # want them to be.  We choose to multiply the min and max by
   # $integerScaleFactor and then divide it back here...

   # the following important vrbles are (associative) arrays (indexed by name) of
   # float tunable constant descriptions and newvalues.
   global floatTunableDescriptionUMM floatTunableNewValues

   global integerScaleFactor

   set newValue [expr 1.0 * $newValue / $integerScaleFactor]

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

proc dummySuppressChar {} {
}
proc valueBind {theWindow lcv} {
   bind $theWindow <Enter> "bindFloatEnter $lcv"
   bind $theWindow <Leave> "bindFloatLeave $lcv"
}
proc drawFloatTunable {theName leftTickWidth rightTickWidth} {
   global nextStartY
   global namesWidth

   global numTunablesDrawn
   global integerScaleFactor

   # the following important vrbles are (associative) arrays (indexed by name) of
   # float tunable constant description/use/min/max and newvalues.
   global floatTunableDescriptionUMM floatTunableNewValues

   set tunableDescription [lindex $floatTunableDescriptionUMM($theName) 0]

   # if both 0.0, then as far as we're concerned, there are no min/max values.
   set tunableMin [lindex $floatTunableDescriptionUMM($theName) 2]
   set tunableMax [lindex $floatTunableDescriptionUMM($theName) 3]

   set namesWin  .tune.middle.canvas.names.tunable$numTunablesDrawn
   set valuesWin .tune.middle.canvas.values.tunable$numTunablesDrawn
   
   # label widget for the floating tunable's name
   frame $valuesWin
   pack  $valuesWin -side top -fill x -expand true

   # dummy label widget (so the right side of the screen will be as tall as the left)
   set labelFont *-Helvetica-*-r-*-14-*
   label $valuesWin.label -relief flat -height 1 -font $labelFont
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
   entry $entryWin -relief sunken -textvariable floatTunableNewValues($theName) -width 8 -font $labelFont -highlightthickness 0

   # turn off some useless characters (such as "return" key)
#   bind $entryWin <Key> {puts stderr "hello %K"}
   bind $entryWin <Return>   {dummySuppressChar}
   bind $entryWin <Tab>      {dummySuppressChar}
   bind $entryWin <KP_Enter> {dummySuppressChar}
   valueBind $entryWin $numTunablesDrawn

   pack $entryWin -side right -expand false
      # expand is false; if the window is made taller, we don't want the extra height

   # scale widget 
   set tickFont *-Helvetica-*-r-*-12-*

   set tickWin $valuesWin.left
   # a bit of padding between the rightmost tick and the entry widget
   # (even if a rightmost tick doesn't exist)
   frame $tickWin.padAfterRightTick -width 10
   pack  $tickWin.padAfterRightTick -side right
   valueBind $tickWin.padAfterRightTick $numTunablesDrawn

   if {$tunableMin!=0 || $tunableMax!= 0} {
      label $tickWin.leftTick -text $tunableMin -font $tickFont -width $leftTickWidth -anchor e
      pack $tickWin.leftTick -side left
      valueBind $tickWin.leftTick $numTunablesDrawn

      label $tickWin.rightTick -text $tunableMax -font $tickFont -width $rightTickWidth -anchor w
      pack $tickWin.rightTick -side right -fill y
      valueBind $tickWin.rightTick $numTunablesDrawn

      # [other options to try: -font -length -sliderlength -width -showValue]
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

      # initialize the scale setting
      $scaleWin set [expr round($integerScaleFactor * $floatTunableNewValues($theName))]
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
   frame $namesWin -height $valuesWinHeight
   pack  $namesWin -side top -fill x -expand true

   label $namesWin.label -text $theName -anchor w -font $labelFont -height 1
   pack  $namesWin.label -side top -fill x
   valueBind $namesWin.label $numTunablesDrawn

   set paddingHeight [expr $valuesWinHeight - [getWindowHeight $namesWin.label]]
   if {$tunableMin==0 && $tunableMax==0} {
      incr paddingHeight
   }
#   puts stderr "paddingHeight for $theName is $paddingHeight"
   if {$paddingHeight > 0} {
      frame $namesWin.padding -height $paddingHeight
      pack  $namesWin.padding -side bottom -fill both
      valueBind $namesWin.padding $numTunablesDrawn
   }

   # finding the height of the left window is easy; it has no frames to confuse us
   set namesWinHeight [max [getWindowHeight $namesWin] [getWindowHeight $namesWin.label]]

   set theHeight [max $namesWinHeight $valuesWinHeight]

   # update the frames' (plural) heights so they're equal
#   puts stderr "theHeight=$theHeight"

   $namesWin  configure -height $theHeight
   $valuesWin configure -height $theHeight

   set namesWidth  [max $namesWidth  [getWindowWidth $namesWin.label]]

   set nextStartY [expr $nextStartY + $theHeight]
#   puts stderr "nextStartY now $nextStartY"
   incr numTunablesDrawn
}

proc drawTunables {newWidth newHeight} {
   global numTunablesDrawn
   global nextStartY
   global namesWidth
   global DeveloperModeFlag
   global tunableMinWidth tunableMinHeight

   global boolTunableDescriptionU boolTunableNewValues
   global floatTunableDescriptionUMM floatTunableNewValues

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
   
   set mySearchId [array startsearch floatTunableDescriptionUMM]
   set theSize [array size floatTunableDescriptionUMM]

   while {[array anymore floatTunableDescriptionUMM $mySearchId]} {
      set theUMM [array nextelement floatTunableDescriptionUMM $mySearchId]
      
      set tunableUse [lindex $theUMM 0]

      if {$tunableUse=="developer" && $DeveloperModeFlag==0} continue

      set tunableMin [lindex $theUMM 1]
      set tunableMax [lindex $theUMM 2]
      # if both 0.0, then as far as we're concerned, there are no min/max values.

      if {$tunableMin!=0 || $tunableMax!=0} {
         set leftTickWidth [max $leftTickWidth [string length $tunableMin]]
         set rightTickWidth [max $rightTickWidth [string length $tunableMax]]
      }
   }

   array donesearch floatTunableDescriptionUMM $mySearchId

   # make two passes---draw all boolean tunables, then all float tunables.
   # (looks nicer on screen that way...)

   set allBoolNames [array names boolTunableDescriptionU]
   set numBoolNames [llength $allBoolNames]

   for {set lcv 0} {$lcv < $numBoolNames} {incr lcv} {
      set theName [lindex $allBoolNames $lcv]

      set theDU $boolTunableDescriptionU($theName)
      set tunableUse [lindex $theDU 1]

      # If this tunable constant is a "developer" one, and if we
      # are not in developer mode, then skip it.
      if {$tunableUse=="developer" && $DeveloperModeFlag==0} continue
      drawBoolTunable $theName
   }

   set allFloatNames [array names floatTunableDescriptionUMM]
   set numFloatNames [llength $allFloatNames]

   for {set lcv 0} {$lcv < $numFloatNames} {incr lcv} {
      set theName [lindex $allFloatNames $lcv]

      set theDUMM $floatTunableDescriptionUMM($theName)
      set tunableUse [lindex $theDUMM 1]

      if {$tunableUse=="developer" && $DeveloperModeFlag==0} continue
      drawFloatTunable $theName $leftTickWidth $rightTickWidth
   }

   # the above calls will have updated variables:
   # namesWidth
   # nextStartY (will now be total height, in pixels, of the canvas)

   set namesWidth  [max $namesWidth  [getWindowWidth .tune.middle.canvas.names]]
   set valuesWidth [expr [getWindowWidth .tune.middle.canvas] - $namesWidth]

   .tune.middle.canvas create window 0 0 \
	-anchor nw -tag tunableTag \
	-window .tune.middle.canvas.names

   .tune.middle.canvas create window $namesWidth 0 \
	   -anchor nw -tag tunableTag \
	   -window .tune.middle.canvas.values \
	   -width $valuesWidth

   set goodMinWidth [expr $namesWidth + [getWindowWidth .tune.middle.scrollbar] + 260]
   wm minsize .tune $goodMinWidth $tunableMinHeight

   set oldGeometry [wm geometry .tune]
   if {$oldGeometry!="1x1+0+0"} {
      set numscanned [scan $oldGeometry "%dx%d+%d+%d" oldWidth oldHeight oldx oldy]
      if {$numscanned==4} {
         if {$oldWidth < $goodMinWidth} {
            wm geometry .tune [format "%dx%d+%d+%d" $goodMinWidth $oldHeight $oldx $oldy]
         }
      } else {
         puts stderr "tclTunable.tcl: could not scan geometry...won't try to resize"
      }
   }

   rethinkScrollBarRegions [getWindowWidth .tune.middle.canvas] [getWindowHeight .tune.middle.canvas]
}

proc rethinkScrollBarRegions {newWidth newHeight} {
   # explicitly called by the program after putting up the tunable
   # constants (e.g. first time, after changing to/from developer mode)
   # We are passed how many pixels the tunables take up, and we adjust
   # the canvas' scrollregion and the scrollbar settings accordingly.

   global nextStartY

#   puts stderr "welcome to rethinkScrollbarRegion; window width=$newWidth, height=$newHeight; fullHeight=$nextStartY"

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
   # associative array (by name) of description/use
   global boolTunableDescriptionU
   # associative array (by name) of bool value
   global boolTunableOldValues boolTunableNewValues

   # associative array (by name) of description/use/min/max
   global floatTunableDescriptionUMM
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

      set boolTunableDescriptionU($theName) $theList
      set boolTunableOldValues($theName) [uimpd tclTunable getvaluebyname $theName]
      set boolTunableNewValues($theName) $boolTunableOldValues($theName)
   }

#   puts stderr "gatherInitialTunableValues -- bool tunable constants have been initialized"

   # Next, we initialize all the float tunable constants:
   set allFloatNames [uimpd tclTunable getfloatallnames]
   set numFloatNames [llength $allFloatNames]
   for {set lcv 0} {$lcv < $numFloatNames} {incr lcv} {
      set theName [lindex $allFloatNames $lcv]

      set theDescription [uimpd tclTunable getdescription $theName]
      set theUse [uimpd tclTunable getusebyname $theName]

      set theMinMax [uimpd tclTunable getfloatrangebyname $theName]
      set theMin [lindex $theMinMax 0]
      set theMax [lindex $theMinMax 1]

      set theList [list $theDescription $theUse $theMin $theMax]

      set floatTunableDescriptionUMM($theName) $theList
      set floatTunableOldValues($theName) [uimpd tclTunable getvaluebyname $theName]
      set floatTunableNewValues($theName) $floatTunableOldValues($theName)
   }
}

proc processCommitFinalTunableValues {} {
   global boolTunableOldValues boolTunableNewValues
   global floatTunableOldValues floatTunableNewValues

   set allBoolNames [uimpd tclTunable getboolallnames]
   set numBoolNames [llength $allBoolNames]
   for {set lcv 0} {$lcv < $numBoolNames} {incr lcv} {
      set theName [lindex $allBoolNames $lcv]
      if {$boolTunableNewValues($theName) != $boolTunableOldValues($theName)} {
#         puts stderr "processFinalTunableValues: tunable $theName has changed from $boolTunableOldValues($theName) to $boolTunableNewValues($theName)!"

         uimpd tclTunable setvaluebyname $theName $boolTunableNewValues($theName)
      }
   }

   # Now the same for float tunables:
   set allFloatNames [uimpd tclTunable getfloatallnames]
   set numFloatNames [llength $allFloatNames]
   for {set lcv 0} {$lcv < $numFloatNames} {incr lcv} {
      set theName [lindex $allFloatNames $lcv]
      if {$floatTunableNewValues($theName) != $floatTunableOldValues($theName)} {
#         puts stderr "processFinalTunableValues: tunable $theName has changed from $floatTunableOldValues($theName) to $floatTunableNewValues($theName)!"

         uimpd tclTunable setvaluebyname $theName $floatTunableNewValues($theName)
      }
   }
  
   tunableExitPoint
}

proc processDiscardFinalTunableValues {} {
   tunableExitPoint
}

# ******************* Tunable Descriptions Stuff *****************

proc processShowTunableDescriptions {} {
   global numTunableDescriptionsDrawn
   global tunableTitleFont tunableDescriptionFont
   global tunableDescriptionsMinWidth tunableDescriptionsMinHeight
   global tunableDescriptionsMaxWidth tunableDescriptionsMaxHeight
   global lastVisibleDescriptionsWidth lastVisibleDescriptionsHeight

   if {[winfo exists .tunableDescriptions]} {
      return
   }

   set numTunableDescriptionsDrawn 0

   set tunableTitleFont *-Helvetica-*-r-*-14-*
   set tunableDescriptionFont *-Helvetica-*-r-*-12-*

   set tunableDescriptionsMinWidth  150
   set tunableDescriptionsMinHeight 150

   set lastVisibleDescriptionsWidth 0
   set lastVisibleDescriptionsHeight 0

   toplevel .tunableDescriptions -class TunableDescriptions
   wm title .tunableDescriptions "Tunable Descriptions"
   # one does not pack a toplevel window

   frame .tunableDescriptions.bottom -relief groove
   pack  .tunableDescriptions.bottom -side bottom -fill x -expand false
      # expand is false; if the window is made taller, we don't want the extra height

   frame .tunableDescriptions.bottom.frame
   pack  .tunableDescriptions.bottom.frame -side bottom -fill y -expand true

   button .tunableDescriptions.bottom.frame.okay -text "Dismiss" -command closeTunableDescriptions
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

   drawTunableDescriptions
}

proc draw1TunableDescription {theName theDescription} {
   global numTunableDescriptionsDrawn
   global tunableDescriptionsTotalHeight
   global tunableTitleFont
   global tunableDescriptionFont

   global boolTunableDescriptionU floatTunableDescriptionUMM

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
      -text $theName -foreground "blue" -justify left -width 3i \
      -font $tunableTitleFont
   pack  .tunableDescriptions.top.canvas.frame$numTunableDescriptionsDrawn.top.left.label \
      -side top -fill x -expand false
      # we don't want extra height after resizing

   set tunableDescriptionsTotalHeight [expr $tunableDescriptionsTotalHeight + [getWindowHeight .tunableDescriptions.top.canvas.frame$numTunableDescriptionsDrawn.top.left.label]]

   message .tunableDescriptions.top.canvas.frame$numTunableDescriptionsDrawn.bottom.right.msg \
	 -width 3i -justify left -text $theDescription \
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

   global boolTunableDescriptionU floatTunableDescriptionUMM
   
   # delete old stuff...
   .tunableDescriptions.top.canvas delete description
   for {set lcv 0} {$lcv<$numTunableDescriptionsDrawn} {incr lcv} {
      destroy .tunableDescriptions.top.canvas.frame$lcv
   }

   set numTunableDescriptionsDrawn 0
   set tunableDescriptionsTotalHeight 0

   # First, draw boolean descriptions
   set allBoolNames [uimpd tclTunable getboolallnames]
   set numBoolNames [llength $allBoolNames]
   for {set lcv 0} {$lcv < $numBoolNames} {incr lcv} {
      set theName [lindex $allBoolNames $lcv]
      set theDescription [lindex $boolTunableDescriptionU($theName) 0]
      set theUse [lindex $boolTunableDescriptionU($theName) 1]
      
      if {$theUse=="developer" && $DeveloperModeFlag==0} continue

      draw1TunableDescription $theName $theDescription
   }      

   # Next, draw float descriptions
   set allFloatNames [uimpd tclTunable getfloatallnames]
   set numFloatNames [llength $allFloatNames]
   for {set lcv 0} {$lcv < $numFloatNames} {incr lcv} {
      set theName [lindex $allFloatNames $lcv]
      set theDescription [lindex $floatTunableDescriptionUMM($theName) 0]
      set theUse [lindex $floatTunableDescriptionUMM($theName) 1]
      
      if {$theUse=="developer" && $DeveloperModeFlag==0} continue

      draw1TunableDescription $theName $theDescription
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

   if {$lastVisibleDescriptionsHeight != $newHeight || $lastVisibleDescriptionsWidth != $newWidth} {
      drawTunableDescriptions
   } else {
      .tunableDescriptions.top.scrollbar set $left $right
   }

   set lastVisibleDescriptionsHeight $newHeight
   set lastVisibleDescriptionsWidth $newWidth
}

proc closeTunableDescriptions {} {
   global numTunableDescriptionsDrawn
   set numTunableDescriptionsDrawn 0

   destroy .tunableDescriptions
}

# ###################### Entrypoint Routine ####################

proc tunableEntryPoint {} {
   global numTunablesDrawn
   global nextStartY
   global integerScaleFactor
   global DeveloperModeFlag
   global lastVisibleWidth lastVisibleHeight

   if {[winfo exists .tune]} {
      # tunable constants is already up!
      return
   }

   set DeveloperModeFlag [uimpd tclTunable getvaluebyname developerMode]

   set lastVisibleWidth 0
   set lastVisibleHeight 0

   tunableInitialize

   gatherInitialTunableValues

   set numTunablesDrawn 0
   set nextStartY 0
   set integerScaleFactor 20

   drawTunables [getWindowWidth .tune.middle.canvas] [getWindowHeight .tune.middle.canvas]
}

proc tunableExitPoint {} {
   global numTunablesDrawn
   global nextStartY
   global integerScaleFactor
   global DeveloperModeFlag
   global boolTunableDescriptionU boolTunableOldValues boolTunableNewValues
   global floatTunableDescriptionUMM floatTunableOldValues floatTunableNewValues
   global tunableMinHeight
   global lastVisibleWidth lastVisibleHeight

   # destroy our toplevel windows (and all their subwindows)
   destroy .tune

   if {[winfo exists .tunableDescriptions]} {
      destroy .tunableDescriptions
   }

   unset boolTunableDescriptionU boolTunableOldValues boolTunableNewValues
   unset floatTunableDescriptionUMM floatTunableOldValues floatTunableNewValues
   unset numTunablesDrawn nextStartY integerScaleFactor
   unset DeveloperModeFlag tunableMinHeight
   unset lastVisibleWidth lastVisibleHeight
}
