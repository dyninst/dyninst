#
#  tabVis -- A tabular display visualization for Paradyn
#
#  $Log: tabVis.tcl,v $
#  Revision 1.8  1994/11/08 00:23:08  tamches
#  Major update/rewrite.  blt_table influences are gone.
#  Main drawing area is a canvas with horizontal and vertical
#  scrollbars.
#  Non-printing metric/focus bugs gone with the change.
#  Still very slow, however, since it uses tcl extensively on each
#  new data callback.
#
# Revision 1.7  1994/09/05  18:40:25  jcargill
# Changed title to be more uniform.
#
# Revision 1.6  1994/08/08  16:21:30  rbi
# Tiny color fix.
#
# Revision 1.5  1994/08/06  22:01:02  rbi
# New status line.  Full path to logo. "Since" dataformat. Eliminated
# variable traces.
#
# Revision 1.4  1994/07/20  21:52:27  rbi
# Better support for BW and standard color scheme
# Added "Actions" menu
#
# Revision 1.3  1994/06/14  18:58:29  rbi
# Updated layout and added curve validation callback.
#
# Revision 1.2  1994/06/01  16:59:30  rbi
# Better menu bar.  Short names option.
#
# Revision 1.1  1994/05/31  22:18:53  rbi
# Fix for direct execution of tcl visis
#
#

#
#  default display options
#

if {[string match [tk colormodel .] color] == 1} {
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
frame $W -class Visi -width 3i -height 2i

#
#  Create the title bar, menu bar, and logo at the top
#
frame $W.top
pack $W.top -side top -fill x

frame $W.top.left 
pack $W.top.left -side left -fill both -expand 1

label $W.top.left.title -relief raised -text "Table Visualization" \
      -foreground white -background HotPink4

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
$W.top.left.menubar.file.m add command -label "Close Table" -command Shutdown

#
#  Actions menu
#
menubutton $W.top.left.menubar.acts -text "Actions" -menu $W.top.left.menubar.acts.m
menu $W.top.left.menubar.acts.m
$W.top.left.menubar.acts.m add command -label "Add Entry" -command AddEntry
$W.top.left.menubar.acts.m add command -label "Delete Entry" -command DelEntry
$W.top.left.menubar.acts.m disable 1

#
#  Options menu
#
menubutton $W.top.left.menubar.opts -text "Options" -menu $W.top.left.menubar.opts.m
#menu $W.top.left.menubar.opts.m -selector black
menu $W.top.left.menubar.opts.m
$W.top.left.menubar.opts.m add command -label "Signif. Digits" -command SetSignif
$W.top.left.menubar.opts.m add check -label "Short Names" -variable ShortNames -command UpdateNames
$W.top.left.menubar.opts.m add separator
$W.top.left.menubar.opts.m add radio -label "Current Value" \
  -variable DataFormat \
  -value Instantaneous -command FormatChanged
$W.top.left.menubar.opts.m add radio -label "Average Value" \
  -variable DataFormat \
  -value Average -command FormatChanged
$W.top.left.menubar.opts.m add radio -label "Total Value" \
  -variable DataFormat \
  -value Sum -command FormatChanged
#$W.top.left.menubar.opts.m add radio -label "Enabled Since" \
#  -variable DataFormat \
#  -value Since -command FormatChanged

#
#  Help menu
#
menubutton $W.top.left.menubar.help -text "Help" -menu $W.top.left.menubar.help.m
menu $W.top.left.menubar.help.m
$W.top.left.menubar.help.m add command -label "General" -command "NotImpl"
$W.top.left.menubar.help.m add command -label "Context" -command "NotImpl"
$W.top.left.menubar.help.m disable 0
$W.top.left.menubar.help.m disable 1

#
#  Build the menu bar and add to display
#
pack $W.top.left.menubar.file $W.top.left.menubar.acts $W.top.left.menubar.opts -side left -padx 4
pack $W.top.left.menubar.help -side right 

#
#  Organize all menu buttons into a menubar
#
tk_menuBar $W.top.left.menubar $W.top.left.menubar.file $W.top.left.menubar.acts $W.top.left.menubar.opts $W.top.left.menubar.help 

#
#  Build the logo 
#
label $W.top.logo -relief raised \
                  -bitmap @/p/paradyn/core/paradyn/tcl/logo.xbm \
                  -foreground HotPink4

pack $W.top.logo -side right

#
#  Status field at the bottom:
#
canvas $W.status -relief groove
pack   $W.status -side bottom -fill x -ipady 4 -ipadx 4 -expand false

label $W.status.label -text "No Data Yet" \
        -font *-Helvetica-*-o-*-12-* \
        -foreground blue -padx 6
pack  $W.status.label -expand true -side top

# Horizontal Scrollbar
frame $W.scrollBarArea
pack  $W.scrollBarArea -side bottom -fill x

frame $W.scrollBarArea.bottomLeftPadding
pack  $W.scrollBarArea.bottomLeftPadding -side left
# we will repack this soon

scrollbar $W.scrollBarArea.scrollbar -orient horizontal -width 16 \
        -foreground gray -activeforeground gray -relief sunken \
        -command horizScrollbarNewScrollPosition
pack $W.scrollBarArea.scrollbar -side bottom -fill x

#
# Left portion of middle: padding (on top), vertical scrollbar, resources axis:
#

canvas $W.left -relief groove -width 1i
pack   $W.left -side left -fill y -expand false

canvas $W.left.bottom -relief groove
pack   $W.left.bottom -side bottom -fill y -expand true

frame $W.left.top -relief groove -height 0.2i
pack  $W.left.top -side top -fill x -expand false
   # expand is false; if the window is made taller, we don't want the extra height

scrollbar $W.left.bottom.scrollbar -orient vertical -width 16 \
        -foreground gray -activeforeground gray -relief sunken \
        -command vertScrollbarNewScrollPosition
pack      $W.left.bottom.scrollbar -side left -fill y -expand false

canvas $W.left.bottom.resourcesAxisCanvas -relief groove \
        -yscrollcommand resourcesAxisCanvasYScrollCommand \
        -scrollincrement 1 -width 0.7i
pack   $W.left.bottom.resourcesAxisCanvas -side left -fill both

# let's revisit bottom left padding now:
$W.scrollBarArea.bottomLeftPadding configure -width [expr [getWindowWidth $W.left] - 8]
   # sorry, I don't know why the 8 works (tho' it is half of 16 which is the
   # scrollbar width---does that help?)
pack  $W.scrollBarArea.bottomLeftPadding -side left

#
# Right portion of middle: metrics axis, data canvas
#

frame $W.right
pack  $W.right -side right -fill both -expand true

canvas $W.right.metricsAxisCanvas -relief groove \
        -xscrollcommand metricsAxisCanvasXScrollCommand \
        -scrollincrement 1 \
        -height 0.2i
pack   $W.right.metricsAxisCanvas -side top -fill x -expand false

canvas $W.right.dataCanvas -relief groove \
        -scrollincrement 1
#        -xscrollcommand dataCanvasXScrollCommand \
#        -yscrollcommand dataCanvasYScrollCommand \
pack  $W.right.dataCanvas -side top -fill both -expand true

proc horizScrollbarNewScrollPosition {newTop} {
   global W

   # Called when the horizontal scrollbar is repositioned.
   # Interestingly, the screen is not yet updated with the new scrollbar position!
   if {$newTop < 0} {
      set newTop 0
   }

   set currSettings [$W.scrollBarArea.scrollbar get]
   set totalSize    [lindex $currSettings 0]
   set visibleSize  [lindex $currSettings 1]

   if {$visibleSize > $totalSize} {
      set newTop 0
   } elseif {[expr $newTop + $visibleSize] > $totalSize} {
      set newTop [expr $totalSize - $visibleSize]
   }

   # update metrics axis canvas
   $W.right.metricsAxisCanvas xview $newTop

   # update data canvas
   $W.right.dataCanvas xview $newTop
}

proc vertScrollbarNewScrollPosition {newTop} {
   global W

   # Called when the vert scrollbar is repositioned.
   # Interestingly, the screen is not yet updated with the new scrollbar position!
   if {$newTop < 0} {
      set newTop 0
   }

   set currSettings [$W.left.bottom.scrollbar get]
   set totalSize    [lindex $currSettings 0]
   set visibleSize  [lindex $currSettings 1]

   if {$visibleSize > $totalSize} {
      set newTop 0
   } elseif {[expr $newTop + $visibleSize] > $totalSize} {
      set newTop [expr $totalSize - $visibleSize]
   }

   # update resources axis canvas
   $W.left.bottom.resourcesAxisCanvas yview $newTop

   # update data canvas
   $W.right.dataCanvas yview $newTop
}

set metricsAxisXScrollCommandLastVisibleWidth 0
set metricsAxisXScrollCommandLastVisibleHeight 0

proc metricsAxisCanvasXScrollCommand {totalSize visibleSize left right} {
   global W
   global metricsAxisXScrollCommandLastVisibleWidth metricsAxisXScrollCommandLastVisibleHeight

   # Gets called whenever the metrics canvas view changes or gets resized.
   # This includes every horizontal scroll the user makes (yikes)
   # Gives us a chance to rethink the bounds of the scrollbar

   set newWidth  [getWindowWidth  $W.right.metricsAxisCanvas]
   set newHeight [getWindowHeight $W.right.metricsAxisCanvas]
   
   if {$metricsAxisXScrollCommandLastVisibleHeight != $newHeight || $metricsAxisXScrollCommandLastVisibleWidth != $newWidth} {
      redrawMetricsAxisCanvas
   } else {
      $W.scrollBarArea.scrollbar set $totalSize $visibleSize $left $right
   }

   set metricsAxisXScrollCommandLastVisibleWidth  $newWidth
   set metricsAxisXScrollCommandLastVisibleHeight $newHeight
}

set resourcesAxisCanvasYScrollCommandLastVisibleWidth 0
set resourcesAxisCanvasYScrollCommandLastVisibleHeight 0

proc resourcesAxisCanvasYScrollCommand {totalSize visibleSize top bottom} {
   global W
   global resourcesAxisCanvasYScrollCommandLastVisibleWidth
   global resourcesAxisCanvasYScrollCommandLastVisibleHeight

   # Gets called whenever the metrics canvas view changes or gets resized.
   # This includes every horizontal scroll the user makes (yikes)
   # Gives us a chance to rethink the bounds of the scrollbar

   set newWidth  [getWindowWidth  $W.left.bottom.resourcesAxisCanvas]
   set newHeight [getWindowHeight $W.left.bottom.resourcesAxisCanvas]
   
   if {$resourcesAxisCanvasYScrollCommandLastVisibleHeight != $newHeight || $resourcesAxisCanvasYScrollCommandLastVisibleWidth != $newWidth} {
      redrawResourcesAxisCanvas
   } else {
      $W.left.bottom.scrollbar set $totalSize $visibleSize $top $bottom
   }

   set resourcesAxisCanvasYScrollCommandLastVisibleWidth  $newWidth
   set resourcesAxisCanvasYScrollCommandLastVisibleHeight $newHeight
}

# redrawMetricsAxisCanvas
# Given: updated numMetrics, metricNames()
# Does:  draws metrics axis; updates metricOffsets()

set numMetrics 0
set numResources 0
proc redrawMetricsAxisCanvas {} {
   global W
   global numMetrics metricNames metricOffsets

   set theCanvas $W.right.metricsAxisCanvas
   set theDataCanvas $W.right.dataCanvas
   set theResourcesCanvas $W.left.bottom.resourcesAxisCanvas

   set maxWidth 0
   set minItemWidth 50

   set startX 0
   set top 0

   # first delete everything left over from last time:
   $theCanvas delete metricDependentTag

   set bottom [expr [getWindowHeight $theCanvas] - 1]
   set currX $startX

   for {set metriclcv 0} {$metriclcv < $numMetrics} {incr metriclcv} {
      if {$metriclcv > 0} {
          if {$metricOffsets($metriclcv) != $currX} {
             puts stderr "redrawMetricsAxisCanvas: $metricOffsets($metriclcv)<>$currX"
          }
      }
      set metricOffsets($metriclcv) $currX

      $theCanvas create line $currX $top $currX $bottom \
              -fill lightBlue -tag tableVisiLineTag

      set currX [expr $currX + 6]

      set theMetricName $metricNames($metriclcv)
      $theCanvas create text $currX [expr $top+2] \
              -text $theMetricName \
              -anchor nw \
              -fill blue \
              -font *-Helvetica-*-r-*-12-* \
              -justify left \
              -tag latestItemTag

         set theBBox [$theCanvas bbox latestItemTag]
         set itemWidth [expr [lindex $theBBox 2] - [lindex $theBBox 0] + 1]
         if {$itemWidth < $minItemWidth} {
            set itemWidth $minItemWidth
         }

         $theCanvas itemconfigure latestItemTag -width $itemWidth -justify center
      $theCanvas itemconfigure latestItemTag -tag tableVisiTag
      $theCanvas dtag latestItemTag

      set currX [expr $currX + $itemWidth + 3]
      set metricOffsets([expr $metriclcv + 1]) $currX

      $theCanvas create line $currX $top $currX $bottom \
              -fill lightBlue -tag tableVisiLineTag
   }

   # 
   # rethink the region of the horizontal scrollbar now
   # 

   set regionList {0 0 0 0}
   set regionList [lreplace $regionList 2 2 $currX]
   set regionList [lreplace $regionList 3 3 [getWindowHeight $theCanvas]]
   $theCanvas configure -scrollregion $regionList

   set oldconfig [$W.scrollBarArea.scrollbar get]
   set oldTotalWidth [lindex $oldconfig 0]

   set firstUnit [lindex $oldconfig 2] 
   if {$oldTotalWidth != $currX} {
      set firstUnit 0
   }
   set lastUnit [expr $firstUnit + $currX - 1]

   # total, visible, first, last
   set visibleX [getWindowWidth $theCanvas]
   $W.scrollBarArea.scrollbar set $currX $visibleX $firstUnit $lastUnit
}

set lastResourcesAxisWidth 0

# redrawResourcesAxisCanvas
# Given: updated numResources, resourceNames()
# Does:  draws resources axis; updates resourceOffsets()

proc redrawResourcesAxisCanvas {} {
   global W
   global numResources resourceNames resourceOffsets
   global lastResourcesAxisWidth ShortNames
   global numMetrics metricOffsets

   # Redraw resources axis, and forcibly resize its canvas' width when done   
   set theCanvas $W.left.bottom.resourcesAxisCanvas

   set newWidth 0

   # first, clean up
   $theCanvas delete resourcesDependentTag

   # next, draw the resources axis
   set startY 3
   set left 3
   set right [expr [getWindowWidth $theCanvas] - 3]

   set currY $startY

   for {set resourcelcv 0} {$resourcelcv < $numResources} {incr resourcelcv} {
      if {$resourcelcv > 0} {
         if {$resourceOffsets($resourcelcv)!=$currY} {
            puts stderr "redrawResourcesAxis: $resourceOffsets($resourcelcv) != $currY"
         }
      }

      set resourceOffsets($resourcelcv) $currY

      # draw horizontal line...
      $theCanvas create line $left $currY $right $currY \
              -fill lightBlue -tag resourcesDependentTag

      # draw text:
      set theText $resourceNames($resourcelcv)
      if {$ShortNames} {
         set theText [file tail $theText]
      }
      $theCanvas create text [expr $right - 3] [expr $currY+2] \
              -text $theText \
              -anchor ne \
              -fill darkGreen \
              -font *-Helvetica-*-r-*-12-* \
              -justify right \
              -tag latestItemTag

         set theBBox [$theCanvas bbox latestItemTag]
         set thisTextWidth [expr [lindex $theBBox 2] - [lindex $theBBox 0] + 1]
      $theCanvas itemconfigure latestItemTag -tag resourcesDependentTag
      $theCanvas dtag latestItemTag

      set thisWidth [expr $left + $thisTextWidth + $left]
      if {$thisWidth > $newWidth} {
         set newWidth $thisWidth
      }

      set thisTextHeight [expr [lindex $theBBox 3] - [lindex $theBBox 1] + 1]
      set currY [expr $currY + 3 + $thisTextHeight + 3]

      set resourceOffsets([expr $resourcelcv + 1]) $currY

      # draw second horizontal line...
      $theCanvas create line $left $currY $right $currY \
              -fill lightBlue -tag resourcesDependentTag
   }   

   if {$newWidth != $lastResourcesAxisWidth} {
#      puts stderr "redrawResourcesAxis: resizing axis since $newWidth<>$lastResourcesAxisWidth"

      set lastResourcesAxisWidth $newWidth

#      pack propagate $W false
         pack $W.scrollBarArea -side bottom -fill x

         pack $W.scrollBarArea.scrollbar -side bottom -fill x

         $W.left configure -width [expr 16 + $newWidth]
         pack $W.left -side left -fill y -expand false

         $W.left.top configure -height 0.2i
         pack $W.left.top -side top -fill x -expand false

         pack $W.left.bottom -side bottom -fill y -expand true

         pack $W.left.bottom.scrollbar -side left -fill y -expand false
         $W.left.bottom.resourcesAxisCanvas configure -width $newWidth -relief groove
         pack $W.left.bottom.resourcesAxisCanvas -side left -fill both

         $W.scrollBarArea.bottomLeftPadding configure -width [getWindowWidth $W.left]
         pack $W.scrollBarArea.bottomLeftPadding -side left
         pack $W.scrollBarArea.scrollbar -side bottom -fill x

         pack $W.right -side right -fill both -expand true
         pack $W.right.metricsAxisCanvas -side top -fill x -expand false

         pack $W.right.dataCanvas -side top -fill both -expand true         
#      pack propagate $W true

#      puts stderr "recursively calling redrawResourcesAxisCanvas..."
      redrawResourcesAxisCanvas
   }

   # 
   # rethink the region of the horizontal scrollbar now
   # 

   set regionList {0 0 0 0}
   if {$numMetrics > 0} {
      set regionList [lreplace $regionList 2 2 $metricOffsets($numMetrics)]
   }
   set regionList [lreplace $regionList 3 3 $currY]
   $theCanvas configure -scrollregion $regionList

   set oldconfig [$W.scrollBarArea.scrollbar get]
   set oldTotalHeight [lindex $oldconfig 0]

   set firstUnit [lindex $oldconfig 2] 
   if {$oldTotalHeight != $currY} {
      set firstUnit 0
   }
   set lastUnit [expr $firstUnit + $currY - 1]

   # total, visible, first, last
   set visibleY [getWindowHeight $theCanvas]
   $W.scrollBarArea.scrollbar set $currY $visibleY $firstUnit $lastUnit
}

proc redrawDataCanvas {} {
   global W
   global numMetrics metricOffsets
   global numResources resourceOffsets
   global lastBucket

   redrawGridLines
   DataUpdate $lastBucket
}

# redrawGridLines
# Given: updated numMetrics, numResources, metricOffsets(), resourceOffsets()
# Does:  updates grid lines in data canvases
proc redrawGridLines {} {
   global W
   global numMetrics numResources
   global metricOffsets resourceOffsets

#   puts stderr "Welcome to redrawGridLines; numMetrics=$numMetrics, numResources=$numResources"

   set theCanvas $W.right.dataCanvas
   $theCanvas delete tableVisiLineTag

   # First, the vertical lines
   set lowY 3
   set maxY $resourceOffsets($numResources)
#   puts stderr "redrawGridLines: maxY is $maxY"

   for {set metriclcv 0} {$metriclcv <= $numMetrics} {incr metriclcv} {
      set currX $metricOffsets($metriclcv)

      $theCanvas create line $currX $lowY $currX $maxY \
              -fill lightBlue -tag tableVisiLineTag
   }

   # Next, the horizontal lines
   set lowX 3
   set hiX $metricOffsets($numMetrics)
#   puts stderr "redrawGridLines: hiX is $hiX"

   for {set resourcelcv 0} {$resourcelcv <= $numResources} {incr resourcelcv} {
      set currY $resourceOffsets($resourcelcv)

      $theCanvas create line $lowX $currY $hiX $currY \
	      -fill lightBlue -tag tableVisiLineTag
   }

#   puts stderr "leaving redrawgridlines"
}

# display everything
pack append . $W {fill expand frame center}
wm minsize . 240 200
wm title . "Table Visualization"

#
#  Significant digits adjustment is done with a scale widget
#
set SignificantDigits 2

proc SetSignif {{w .signif}} {
  global SignificantDigits

  catch {destroy $w}
  toplevel $w -class Visi
  wm geometry $w +300+300
  wm title $w "Signif Digits"
  wm iconname $w "SignifDigits"
  scale $w.scale -orient horizontal -length 280 -from 0 -to 8 \
    -tickinterval 1 -command "set SignificantDigits " \
    -borderwidth 5 -showvalue false -label "Significant Digits" \
    -font *-New*Century*Schoolbook-Bold-R-*-14-*
  $w.scale set $SignificantDigits

  button $w.ok -text OK -command "destroy $w"
  pack $w.scale $w.ok -side top -fill x
}
    
#
#  AddEntry -- Ask paradyn to start a new curve
#
proc AddEntry {} {
  Dg start "*" "*"
}

#
#  DelEntry -- Ask paradyn to stop a curve
#
proc DelEntry {} {
  puts stderr "Delete Entry not yet implemented"
}

#
#  Helper function for "Close" menu option and "Close" button
#
proc Shutdown {} {
  destroy .
}

#
#  Issue a warning about missing function
#
proc NotImpl {} {
  puts stderr "TABVIS2: That function is not yet implemented!!"
}

#
#  Called by visi library when histos have folded
#    we just update the status line and keep on going
#
proc DgFoldCallback {} {
  puts stderr "fold detected..."

  UpdateStatus 
}

#
#  Called by visi library when met/res space changes
#    we rebuild the table
#
proc DgConfigCallback {} {
   global W
   global numResources numMetrics
   global resourceNames metricNames

#   puts stderr "Welcome to DgConfigCallback"

   set numMetrics [Dg nummetrics]
   set numResources [Dg numresources]

   for {set metriclcv 0} {$metriclcv < $numMetrics} {incr metriclcv} {
      set metricNames($metriclcv) [Dg metricname $metriclcv]
   }
   for {set resourcelcv 0} {$resourcelcv < $numResources} {incr resourcelcv} {
      set resourceNames($resourcelcv) [Dg resourcename $resourcelcv]
   }

   redrawMetricsAxisCanvas
   redrawResourcesAxisCanvas
   redrawDataCanvas

   UpdateStatus
}

#
#  Default dataformat is Instantaneous
#    we also support Sum and Average
#
set DataFormat Instantaneous

proc FormatChanged {} {
   global lastBucket

   DataUpdate $lastBucket
   UpdateStatus
}

#
#  Default to long names
#
set ShortNames 0

proc UpdateNames {} {
   global ShortNames

   redrawResourcesAxisCanvas
}

proc GetResourceName {resid} {
  global ShortNames

  set Resource [Dg resourcename $resid]
  if {$ShortNames == 1} {
    set Resource [file tail $Resource]
  }
  return $Resource
}

#
#  Update the status line
#
proc UpdateStatus {} {
  global DataFormat W
  global Callbacks UpdateLimit lastBucket

  set bw [Dg binwidth]

  $W.status.label configure -text [format "Time: %-10s  Format: %s" [TimeLabel [expr int($bw * $lastBucket)]] $DataFormat]
}

#
#  GetValue asks visi library for the data value for the met/res pair 
#    we ask visi for the data in the correct DataFormat
#
proc GetValue {m r bucket} {
  global DataFormat
  global numMetrics numResources

   if {![Dg valid $m $r] || ![Dg enabled $m $r]} {
      # there is a hole in valid datagrid entries -- happens all the time
      return ""
   }

   if {[string match $DataFormat Instantaneous]} {
      return [Dg value $m $r $bucket]
   } elseif {[string match $DataFormat Average]} {
      return [Dg aggregate $m $r]
   } elseif {[string match $DataFormat Sum]} {
      return [Dg sum $m $r]
   } else {
      puts stderr "GetValue: unknwon data format: $DataFormat"
      return 0
   }
}

#
# DgValidCallback -- visi calls this when curve becomes valid
#
proc DgValidCallback {m} {
  return
  puts stderr "Bucket $m is now valid"
}

set UpdateCounter 0
set UpdateLimit 1

# TimeToUpdate - returns true (1) iff it is time to redraw
#                the status (amount of time) at the bottom of the screen
proc TimeToUpdate {} {
  global UpdateCounter UpdateLimit

  if {$UpdateCounter <= 0} {
    set UpdateCounter [expr $UpdateLimit / [Dg binwidth]]
    return 1
  } else {
    set UpdateCounter [expr $UpdateCounter - 1]
    return 0
  }
}

set Callbacks 0

#
#  DgDataCallback -- visi calls this command when new data is available
#    we fill in all of the data labels with the new data values
#
proc DgDataCallback {bucket} {
   global Callbacks

   incr Callbacks
   if {[TimeToUpdate]} {
      DataUpdate $bucket
      UpdateStatus
   }
}

set lastBucket 0
proc DataUpdate {bucket} {
  global W
  global DataFormat
  global SignificantDigits
  global numMetrics numResources
  global metricOffsets resourceOffsets
  global lastBucket

#   puts stderr "Welcome to DataUpdate"

   set theCanvas $W.right.dataCanvas
   $theCanvas delete dataTag

   for {set metriclcv 0} {$metriclcv < $numMetrics} {incr metriclcv} {
      set theWidth [expr $metricOffsets([expr $metriclcv+1])-$metricOffsets($metriclcv)+1]

      set xval $metricOffsets($metriclcv)

      for {set resourcelcv 0} {$resourcelcv < $numResources} {incr resourcelcv} {
         set yval [expr $resourceOffsets($resourcelcv) + 3]

         set theText [GetValue $metriclcv $resourcelcv $bucket]

         if {[string length $theText]>0} {
            set theText [format "%.[set SignificantDigits]f" $theText]
	 }

         $theCanvas create text $xval $yval\
		 -anchor nw \
		 -fill black \
		 -font *-Helvetica-*-r-*-12-* \
		 -justify center \
		 -tag dataTag \
		 -text $theText \
		 -width $theWidth
      }
   }

   set lastBucket $bucket
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
