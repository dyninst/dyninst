#
#  barChart -- A bar chart display visualization for Paradyn
#
#  $Log: barChart.tcl,v $
#  Revision 1.10  1994/10/28 21:53:44  tamches
#  Fixed a rather flaming bug that could cause any resource add to
#  potentially crash after doing a sort (c++ code's numResources
#  wasn't updated before ::rethinkIndirectResources was called, leading
#  to an assertion check failure)
#
# Revision 1.9  1994/10/14  10:28:49  tamches
# Swapped the x and y axes -- now resources print vertically and
# metrics print horizontally.  Can fit many, many more resources
# on screen at once with no label overlap.  Multiple metrics
# are now shown in the metrics axis.  Metric names are shown in
# a "key" in the lower-left.
#
# Revision 1.8  1994/10/13  00:51:39  tamches
# Fixed deletion of resources.
# Implemented sorting of resources.
# Reorganized menus to be more standars-ish
#
# Revision 1.7  1994/10/11  22:04:18  tamches
# Fixed bug whereupon a resize while paused would erase the bars
# until you continued.  Flickers too much on resize now, however...
#
# Delete resources should now work
#
# Revision 1.6  1994/10/10  23:08:41  tamches
# preliminary changes on the way to swapping the x and y axes
#
# Revision 1.5  1994/10/07  22:06:36  tamches
# Fixed some bugs w.r.t. resizing the window (bars and resources were
# sometimes redrawn at the old locations, instead of adapting to the
# resize).  The problem was related to [winfo width ...] returning
# the old value while in the middle of a resize event.  The solution
# was to include %w and %h in the configure-even callback (see the
# tk "bind" command man page)
#
# Revision 1.4  1994/10/04  22:10:56  tamches
# more color fixing (moved codes from barChart.C to here)
#
# Revision 1.3  1994/10/04  19:00:23  tamches
# implemented resourceWidth algorithm: try to make resources the maximum
# pixel width, but if they don't all fit in the window, shrink (down
# to a fixed minimum).  Reapply algorithm when: window resizes, resources
# are added/deleted.
#
# Revision 1.2  1994/10/01  02:22:25  tamches
# Fixed some bugs related to scrolling; now, the user can't accidentally
# scroll to the left of the leftmost bar or to the right of the rightmost
# bar.
#
# Revision 1.1  1994/09/29  19:49:50  tamches
# rewritten for new version of barchart; the bars are now drawn
# with xlib code in C++ (no more blt_barchart) in barChart.C.
# See also barChartTcl.C and barChartDriver.C
#
# Revision 1.5  1994/09/08  00:10:43  tamches
# Added preliminary blt_drag&drop interface.
# changed window title.
#
# Revision 1.4  1994/09/04  23:55:29  tamches
# added 'to do' and 'problems' lists.  tightened code around speed-critical
# areas.  improved look of resources axis.
#
# Revision 1.3  1994/09/03  01:24:40  tamches
# Cleaned up syntax some more, e.g. longer variable names.
# Cleaned up menus
# Added many comments
#
# Revision 1.2  1994/09/02  21:00:30  tamches
# minor get-acquainted formatting cleanups
#
# Revision 1.1  1994/08/06  22:50:47  rbi
# Bar Chart Visi originally written by Sherri Frizell.
# Initial revision includes bug fixes and cleanups by rbi.
#

# ######################################################
# TO DO LIST:
# 1) multiple metrics: allow deletion
# 2) too much flickering on resize
# ######################################################

#  ################### Default options #################

option add *Visi*font *-New*Century*Schoolbook-Bold-R-*-18-*
option add *Data*font *-Helvetica-*-r-*-12-*
option add *MyMenu*font *-New*Century*Schoolbook-Bold-R-*-14-*

if {[string match [tk colormodel .] color] == 1} {
   # You have a color monitor...
   # change primary background color from 'bisque' to 'grey'
   . config -bg grey
   option add *Background grey
   option add *activeBackground LightGrey
   option add *activeForeground black
   option add *Scale.activeForeground grey
} else {
   # You don't have a color monitor...
   option add *Background white
   option add *Foreground black
}

# ####################  Overall frame ###########################

set resourcesAxisWidth 1.4i
set metricsAxisHeight 0.65i

set W .bargrph
frame $W -class Visi

frame $W.top
pack $W.top -side top -fill x -expand false -anchor n
   # this area will encompass the title bar, menu bar, and logo
   # expand is set to false; if the window is made taller,
   # we don't want to get any taller.

frame $W.top.left
pack $W.top.left -side left -fill both -expand true
   # this area encompasses the title bar and menu bar
   # expand is set to true so that if the window is made
   # wider, we get the extra space (as opposed to the logo
   # or as opposed to nobody, which would leave ugly blank
   # space)

# #################### Paradyn logo #################

label $W.top.logo -relief raised \
                  -bitmap @/p/paradyn/core/paradyn/tcl/logo.xbm \
                  -foreground #b3331e1b53c7

pack $W.top.logo -side right -expand false
   # we set expand to false; if the window is made wider, we
   # don't want any of the extra space; let the menu bar and
   # title bar have it.

# #################### Title bar #################

label $W.top.left.titlebar  -text "BarChart Visualization" -foreground white -background lightslategray
pack $W.top.left.titlebar -side top -fill both -expand true
   # expand is set to true, not because we want more space if the window
   # is made taller (which won't happen, since the expand flag of our
   # parent was set to false), but because we want to take up any padding
   # space left after we and the menu bar are placed (if the logo is
   # taller than the two of us, which it currently is)

# ##################### Menu bar ###################

set Wmbar $W.top.left.mbar
frame $Wmbar -class MyMenu -borderwidth 2 -relief raised
pack  $Wmbar -side top -fill both -expand false

# #################### File menu #################

menubutton $Wmbar.file -text File -menu $Wmbar.file.m
menu $Wmbar.file.m
$Wmbar.file.m add command -label "Close Bar chart" -command GracefulClose

# #################### Actions Menu ###################

menubutton $Wmbar.actions -text Actions -menu $Wmbar.actions.m
menu $Wmbar.actions.m
$Wmbar.actions.m add command -label "Add Bars..." -command AddMetricDialog
$Wmbar.actions.m add separator
$Wmbar.actions.m add command -label "Remove Selected Metric" -state disabled
$Wmbar.actions.m add command -label "Remove Selected Resource" -state disabled

# #################### View menu ###################

menubutton $Wmbar.view -text View -menu $Wmbar.view.m
menu $Wmbar.view.m
$Wmbar.view.m add radio -label "Order Resources by Name (ascending)" -variable SortPrefs -command ProcessNewSortPrefs -value ByName
$Wmbar.view.m add radio -label "Order Resources by Name (descending)" -variable SortPrefs -command ProcessNewSortPrefs -value ByNameDescending
$Wmbar.view.m add radio -label "Order Resources as Inserted by User" -variable SortPrefs -command ProcessNewSortPrefs -value NoParticular

$Wmbar.view.m add separator

$Wmbar.view.m add radio -label "Current Values" \
   -variable DataFormat -command {rethinkDataFormat} \
   -value Instantaneous
$Wmbar.view.m add radio -label "Average Values" \
   -variable DataFormat -command {rethinkDataFormat} \
   -value Average
$Wmbar.view.m add radio -label "Total Values" \
   -variable DataFormat -command {rethinkDataFormat} \
   -value Sum

# #################### Help menu #################

#menubutton $Wmbar.help -text Help \
#          -menu $Wmbar.help.m
#menu $Wmbar.help.m 
#$Wmbar.help.m add command -label "General" -command "NotImpl" -state disabled
#$Wmbar.help.m add command -label "Context" -command "NotImpl" -state disabled


# #################### Build the menu bar and add to display #################

pack $Wmbar.file $Wmbar.actions $Wmbar.view -side left -padx 4
#pack $Wmbar.help -side right

# #################### Organize all menu buttons into a menubar #################

tk_menuBar $Wmbar $Wmbar.file $Wmbar.actions \
   $Wmbar.view

# ############  Scrollbar ##############

canvas $W.farLeft
pack $W.farLeft -side left -expand false -fill y
   # expand is set to false; if the window is made wider, don't change width

#canvas $W.farLeft.sbRegion
#pack $W.farLeft.sbRegion -side top -expand true -fill y
#   # expand is set to true; if the window is made taller, we want the extra height

scrollbar $W.farLeft.resourcesAxisScrollbar -orient vertical -width 16 \
	-foreground gray -activeforeground gray -relief sunken \
	-command processNewScrollPosition
pack $W.farLeft.resourcesAxisScrollbar -side top -fill y -expand true
   # expand is set to true; if the window is made taller, we want
   # extra height.

canvas $W.farLeft.sbPadding -height $metricsAxisHeight -width 16 -relief flat
pack $W.farLeft.sbPadding -side bottom -expand false -fill x
   # expand is set to false; if the window is made taller, we don't
   # want any of the extra height.

# #############  Resources Axis ###########3

canvas $W.left -width $resourcesAxisWidth
pack   $W.left -side left -expand false -fill y
   # expand is set to false; if the window is made wider, we don't want
   # any of the extra width

canvas $W.left.metricsKey -height $metricsAxisHeight -width $resourcesAxisWidth\
	-relief groove
pack   $W.left.metricsKey -side bottom -expand false
   # expand is set to false; if the window is made taller, we don't
   # want any of the extra height

set WresourcesCanvas $W.left.resourcesAxisCanvas
canvas $WresourcesCanvas -width $resourcesAxisWidth -relief groove \
                             -yscrollcommand myXScroll \
			     -scrollincrement 1
pack   $WresourcesCanvas -side top -expand true -fill y
   # expand is set to true; if the window is made taller, we want the
   # extra height.

# ####################  Metrics Axis Canvas ##################################

canvas $W.metricsAxisCanvas -height $metricsAxisHeight -relief groove
pack   $W.metricsAxisCanvas -side bottom -fill x -expand false
   # expand is set to false; if the window is made wider, we don't want
   # extra width to go to the metrics axis

# ####################  Barchart Area ($W.body) #################

canvas $W.body -height 2.5i -width 3.5i -relief groove
#frame $W.body -height 3i -width 2.5i -relief groove
pack  $W.body -side top -fill both -expand true
   # expand is set to true; if the window is made taller, we want the
   # extra height to go to us

# ######### pack $W (and all its subwindows) into the main (top-level) window such that
# ######### it basically consumes the entire window...
pack append . $W {fill expand frame center}

# set some window manager hints:
wm minsize  . 350 250
wm title    . "Barchart"

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

# isMetricValid -- true iff at least one metric/focus pair for this metric is enabled
#                  pas a true metric index, not a sorted one
proc isMetricValid {mindex} {
   global numResources

   for {set resourcelcv 0} {$resourcelcv<$numResources} {incr resourcelcv} {
      if {[Dg enabled $mindex $resourcelcv]} {
         # true
         return 1
      }
   }

   # false
   return 0
}

# isResourceValid -- true iff at least one metric/focus pair for this resource is enabled
#                  pas a true resource index, not a sorted one
proc isResourceValid {rindex} {
   global numMetrics

   for {set metriclcv 0} {$metriclcv<$numMetrics} {incr metriclcv} {
      if {[Dg enabled $metriclcv $rindex]} {
         # true
         return 1
      }
   }

   # false
   return 0
}

# ################ Initialization and LaunchBarChart ######################
proc Initialize {} {
   # a subset of DgConfigCallback that sets important global vrbles
   # stuff that needs to be in order **BEFORE** the call to launchBarChart
   # (i.e. launchBarChart depends on these settings)

   # puts stderr "Welcome to Initialize!"
   # flush stderr

   global W

   global numMetrics
   global numMetricsDrawn
   global numMetricLabelsDrawn
   global metricNames
   global validMetrics

   global metricUnits
   global metricMinValues
   global metricMaxValues

   global metricsLabelFont
   global resourceNameFont
   global prevLeftSectionWidth

   global numResources
   global resourceNames
   global indirectResources

   global currResourceHeight
   global minResourceHeight
   global maxResourceHeight

   global DataFormat

   global clickedOnResource
   global clickedOnResourceText
   global numLabelsDrawn
   global numResourcesDrawn

   global SortPrefs
   global barColors
   global numBarColors

   set SortPrefs NoParticular
   
   set clickedOnResource ""
   set clickedOnResourceText ""
   set numLabelsDrawn 0
   set numResourcesDrawn 0

   set DataFormat Instantaneous

   set numMetrics [Dg nummetrics]
   set numResources [Dg numresources]
   set numMetricsDrawn 0
   set numMetricLabelsDrawn 0

   for {set metriclcv 0} {$metriclcv < $numMetrics} {incr metriclcv} {
      set metricNames($metriclcv) [Dg metricname  $metriclcv]
      set validMetrics($metriclcv) [isMetricValid $metriclcv]
      set metricUnits($metriclcv) [Dg metricunits $metriclcv]
      set metricMinValues($metriclcv) [lindex [getMetricHints $metricNames($metriclcv)] 1]
      set metricMaxValues($metriclcv) [lindex [getMetricHints $metricNames($metriclcv)] 2]
   }

   for {set resourcelcv 0} {$resourcelcv < $numResources} {incr resourcelcv} {
      set resourceNames($resourcelcv) [Dg resourcename $resourcelcv]
      set indirectResources($resourcelcv) $resourcelcv
  }

   set minResourceHeight 18
   set maxResourceHeight 50
   set currResourceHeight $maxResourceHeight
      # as resources are added, we try to shrink the resource height down to a minimum of
      # (minResourceHeight) rather than having to invoke the scrollbar.
   set prevLeftSectionWidth 1

   # bar colors: (see /usr/lib/X11/rgb.txt)
   # purple is reserved for the where axis.
   # red should not be next to green; they look equal to colorblind
   # use greyscales if b&w monitor
#   set barColors(0) "tomato"
   set barColors(0) "cornflower blue"
   set barColors(1) "medium sea green"
   set barColors(2) "hotpink"
   set barColors(3) "chocolate"
   set barColors(4) "orange"
   set numBarColors 5

#   set resourceNameFont -adobe-courier-medium-r-normal-*-12-120-*-*-*-*-iso8859-1
#   set metricsLabelFont -adobe-courier-medium-r-normal-*-12-120-*-*-*-*-iso8859-1
   set resourceNameFont *-Helvetica-*-r-*-12-*
   set metricsLabelFont *-Helvetica-*-r-*-12-*


   # launch our C++ barchart code
   # launchBarChart $W.body.barCanvas doublebuffer noflicker $numMetrics $numResources 0
   launchBarChart $W.body doublebuffer noflicker $numMetrics $numResources 0

   # trap window resize and window expose events --- for the subwindow
   # containing the bars only, however.  Note that using
   # $W.body instead of "." avoids LOTS of unneeded
   # configuration callbacks.  In particular, when a window is just
   # moved, it won't create a callback (yea!)  This means we
   # can treat a configuration event as a true resize.

   # [sec 19.2: 'event patterns' in tk/tcl manual]

   bind $W.body <Configure> {myConfigureEventHandler %w %h}
   bind $W.body <Expose>    {myExposeEventHandler}
}

# ############################################################################
# ################################ procedures ################################
# ############################################################################
#
# selectResource
# unSelectResource
# processEnterResource
# processExitResource
# rethinkResourceWidths
# drawResourcesAxis
# processNewMetricMax
# drawMetricAxis
# myConfigureEventHandler
# myExposeEventHandler
# delResource
# delResourceByName
# getMetricHints
# addMetric
# delMetric
# processNewScrollPosition
# myXScroll
# DgFoldCallback
# DgConfigCallback
# AddMetricDialog
# AddResourceDialog
# ProcessNewSortPrefs
# rethinkIndirectResources
# rethinkDataFormat
#
# ############################################################################

# selectResource -- assuming this resource was clicked on, select it
proc selectResource {widgetName} {
   global clickedOnResource
   global clickedOnResourceText
   global Wmbar

   # if someone else was previous selected, un-select him
   if {$clickedOnResource != ""} {
      unSelectResource $clickedOnResource
   }

   # select
   set clickedOnResource $widgetName
   set clickedOnResourceText [lindex [$widgetName configure -text] 4]
   $widgetName configure -relief sunken

   # update "delete resource xxx" menu item s.t. delResourceByName is called automatically
   # on menu choice selection
   $Wmbar.actions.m entryconfigure 4 -state normal \
           -label "Remove Resource \"$clickedOnResourceText\"" \
	   -command {delResourceByName $clickedOnResourceText}
}

# unSelectResource -- pretend we never clicked on this resource
proc unSelectResource {widgetName} {
   global clickedOnResource
   global clickedOnResourceText
   global Wmbar

   set clickedOnResource ""
   set clickedOnResourceText ""
   $widgetName configure -relief flat
   $Wmbar.actions.m entryconfigure 4 -state disabled \
	   -label "Remove Selected Resource" \
           -command {puts "ignoring unexpected deletion..."}
}

# processEnterProcess -- routine to handle entry of mouse in a resource name
proc processEnterResource {widgetName} {
   global clickedOnResource
   global clickedOnResourceText

   # if this widget has already been clicked on, do nothing
   if {$widgetName == $clickedOnResource} {
      return
   }

   $widgetName configure -relief groove
}

# processExitResource -- routine to handle mouse leaving resource name area
#                        we may or may not have done a mouse-click in the meantime
proc processExitResource {widgetName} {
   global clickedOnResource
   global clickedOnResourceText

   # if we clicked on this guy, then do nothing (keep him selected),
   # otherwise undo the -relief groove
   if {$clickedOnResource != $widgetName} {
      $widgetName configure -relief flat
   }
}

proc rethinkResourceHeights {screenHeight} {
   # When resources are added or deleted, this routine is called.
   # Its sole purpose is to rethink the value of currResourceHeight,
   # depending on the resources.

   # algorithm: get current window height.  set resource width equal
   # to window height / num resources.  If that would make the resource
   # height too small, make it minResourceHeight
   global minResourceHeight
   global maxResourceHeight
   global currResourceHeight
   global numResources
   global WresourcesCanvas

   set tentativeResourceHeight [expr $screenHeight / $numResources]
   if {$tentativeResourceHeight < $minResourceHeight} {
      set tentativeResourceHeight $minResourceHeight
   } elseif {$tentativeResourceHeight > $maxResourceHeight} {
      set tentativeResourceHeight $maxResourceHeight
   }

   set currResourceHeight $tentativeResourceHeight

   # puts stderr "Leaving rethinkResourceHeights; we have decided upon $currResourceHeight"
}

# Upon changes to the resources axis or the metrics key, call this routine to rethink
# how wide the left portion of the screen (which is what holds these guys) should be.
proc rethinkLeftSectionWidth {} {
   global W
   global WresourcesCanvas
   global prevLeftSectionWidth
   global numResourcesDrawn
   global numMetricsDrawn

   set maxWidthSoFar 20
   set tickWidth 5

   # loop through the resourcs on screen in sorted order
   for {set rindex 0} {$rindex < $numResourcesDrawn} {incr rindex} {
      set thisLabelWidth [getWindowWidth $WresourcesCanvas.message$rindex]
      if {$thisLabelWidth > $maxWidthSoFar} {
         set maxWidthSoFar $thisLabelWidth
      }
   }

   # loop through the metrics key on screen in sorted order
   for {set mindex 0} {$mindex < $numMetricsDrawn} {incr mindex} {
      set thisLabelWidth [getWindowWidth $W.left.metricsKey.key$mindex]
      if {$thisLabelWidth > $maxWidthSoFar} {
         set maxWidthSoFar $thisLabelWidth
      }
   }

   if {$maxWidthSoFar != $prevLeftSectionWidth} {
      # resize the resourcse axis to consume just the right amount of width
      # we use the "pack propagate" command to avoid resizing the entire window
      # syntax: "pack progagate master flag"
      pack propagate . false
         set newWidth [expr 2 + $maxWidthSoFar + $tickWidth + 2]
         $WresourcesCanvas configure -width $newWidth -relief groove
         pack $WresourcesCanvas -side top -expand true -fill y
            # expand is set to true; if the window is made taller, we want the
            # extra height.
         $W.left.metricsKey configure -width $newWidth
      pack propagate . true
   }

   set prevLeftSectionWidth $maxWidthSoFar
}

proc drawResourcesAxis {theHeight} {
   # how it works: deletes all canvas items with the tag "resourcesAxisItemTag", including
   # the window items.  message widgets have to be deleted separately, notwithstanding
   # the fact that the canvas window items were deleted already.
   # (it knows how many message widgets there are via numResourcesDrawn, which at the
   # time this routine is called, may not be up-to-date with respect to numResources),
   # and then redraws by re-recreating the canvas items and message widgets

   global W
   global Wmbar
   global WresourcesCanvas

   global resourceNameFont
   global resourcesAxisWidth
   global metricsAxisHeight

   global numResources
   global numResourcesDrawn
   global resourceNames
   global indirectResources

   global clickedOnResource
   global clickedOnResourceText

   global minResourceHeight
   global maxResourceHeight
   global currResourceHeight

   global SortPrefs

   # delete leftover stuff (canvas widgets in 1 step, then message widgets manually)
   $WresourcesCanvas delete resourcesAxisItemTag
   for {set rindex 0} {$rindex < $numResourcesDrawn} {incr rindex} {
      destroy $WresourcesCanvas.message$rindex
   }

   set tickWidth 5
   set right [expr [getWindowWidth $WresourcesCanvas] - 3]
   set top 3
   set bottom 3
   set numResourcesDrawn 0

   # loop through resources in sorted order
   for {set rindex 0} {$rindex < $numResources} {incr rindex} {
      set actualResource $indirectResources($rindex)

      if {[isResourceValid $actualResource]} {
         set bottom [expr $top + $currResourceHeight - 1]
         set middle [expr ($top + $bottom) / 2]
   
         # create a tick line for this resource
         $WresourcesCanvas create line [expr $right-$tickWidth] $middle $right $middle -tag resourcesAxisItemTag
   
         # create a message widget, bind some commands to it, and attach it to the
         # canvas via "create window"

         set theName $resourceNames($actualResource)
   
         label $WresourcesCanvas.message$numResourcesDrawn -text $theName \
		 -font $resourceNameFont

         bind $WresourcesCanvas.message$numResourcesDrawn <Enter> \
                             {processEnterResource %W}
         bind $WresourcesCanvas.message$numResourcesDrawn <Leave> \
                             {processExitResource %W}
         bind $WresourcesCanvas.message$numResourcesDrawn <ButtonPress> \
		             {selectResource %W}

         $WresourcesCanvas create window [expr $right-$tickWidth] $middle \
		 -anchor e -tag resourcesAxisItemTag \
		 -window $WresourcesCanvas.message$numResourcesDrawn

         set top [expr $top + $currResourceHeight]
         incr numResourcesDrawn    
      } else {
         #puts stderr "drawResourcesAxis: not drawing resource #$rindex because its valid flag is false"
      }
   }

   # the axis itself--a horizontal line
   $WresourcesCanvas create line $right 0 $right $top -tag resourcesAxisItemTag

   rethinkLeftSectionWidth

   # Update the scrollbar's scrollregion configuration:
   set regionList {0 0 0 0}
   set regionList [lreplace $regionList 2 2 $resourcesAxisWidth]
   set regionList [lreplace $regionList 3 3 $bottom]
   $WresourcesCanvas configure -scrollregion $regionList

   set screenHeight $theHeight

   set oldconfig [$W.farLeft.resourcesAxisScrollbar get]
   set oldTotalHeight [lindex $oldconfig 0]

   if {$oldTotalHeight != $bottom} {
      # puts stderr "drawResourcesAxis: detected major change in resources ($oldTotalWidth != $right), resetting"
      set firstUnit 0
   } else {
      # no change
      set firstUnit [lindex $oldconfig 2]
   }

   set lastUnit [expr $firstUnit + $screenHeight - 1]
   $W.farLeft.resourcesAxisScrollbar set $bottom $screenHeight $firstUnit $lastUnit
}

proc processNewMetricMax {mindex newmaxval} {
   # called from barChart.C when y-axis overflow is detected and
   # a new max value is chosen
   global metricMinValues
   global metricMaxValues
   global W

   set metricMaxValues($mindex) $newmaxval

   drawMetricsAxis [getWindowWidth $W.metricsAxisCanvas]
}

proc drawMetricsAxis {metricsAxisWidth} {
   # the y axis changes to reflect the units of the current metric(s).
   # It is not necessary to call drawMetricsAxis if the window width is
   # resized; it IS necessary to call drawMetricsAxis if the window
   # width is changed.
   # It is not necessary to call drawMetricsAxis if resources are
   # added or removed; it IS necessary to call drawMetricsAxis if
   # metrics are added or removed.

   global W
   global numMetrics
   global numMetricsDrawn
   global numMetricLabelsDrawn

   global metricNames
   global validMetrics

   global metricUnits
   global metricUnitTypes
   global metricMinValues
   global metricMaxValues
   global metricsLabelFont

   set keyWindow $W.left.metricsKey

   # first, delete all leftover canvas items (those with a metricsAxis tag to them)
   $W.metricsAxisCanvas delete metricsAxisTag
   $keyWindow delete metricsAxisTag

   # we still have to delete the label widgets, which can't have tags...
   for {set labelindex 0} {$labelindex < $numMetricLabelsDrawn} {incr labelindex} {
      destroy $W.metricsAxisCanvas.label$labelindex
   }
   for {set metriclcv 0} {$metriclcv < $numMetricsDrawn} {incr metriclcv} {
      destroy $keyWindow.key$metriclcv
   }

   set numticks 3
   set fixedLeft 5
   set fixedRight [expr $metricsAxisWidth - 5]
   set top 5
   set tickHeight 5
   set tickStepPix [expr ($fixedRight - $fixedLeft + 1) / ($numticks-1)]

   set labelDrawnCount 0
   set numMetricsDrawn 0

   for {set metriclcv 0} {$metriclcv<$numMetrics} {incr metriclcv} {
      # draw this axis, if it is a legitimate metric
      if {$validMetrics($metriclcv)} {
         set numericalStep [expr (1.0 * $metricMaxValues($metriclcv)-$metricMinValues($metriclcv)) / ($numticks-1)]

         # draw horizontal line for this metric axis; color-coded for the metric in question
         $W.metricsAxisCanvas create line $fixedLeft $top $fixedRight $top -tag metricsAxisTag \
		 -fill [getMetricColor $metriclcv]

         # draw tick marks and create labels for this metric axis
         for {set ticklcv 0} {$ticklcv < $numticks} {incr ticklcv} {
            set tickx [expr $fixedLeft + ($ticklcv * $tickStepPix)]
            $W.metricsAxisCanvas create line $tickx $top $tickx [expr $top + $tickHeight] \
		    -tag metricsAxisTag -fill [getMetricColor $metriclcv]

            set labelText [expr $metricMinValues($metriclcv) + $ticklcv * $numericalStep]

	    if {$ticklcv==0} {
               set theAnchor nw
               set theJust left
	    } elseif {$ticklcv==[expr $numticks-1]} {
               set theAnchor ne
               set theJust center
	    } else {
               set theAnchor n
               set theJust right
	    }

            # we use message widgets instead of labels to get justification correct
            message $W.metricsAxisCanvas.label$labelDrawnCount -text $labelText \
		    -justify $theJust -font $metricsLabelFont -width [getWindowWidth $W.metricsAxisCanvas]
            $W.metricsAxisCanvas create window $tickx [expr $top+$tickHeight] -anchor $theAnchor \
		    -tag metricsAxisItemTag -window $W.metricsAxisCanvas.label$labelDrawnCount

            incr labelDrawnCount
         }

         # draw an appropriate entry in the "key" (like the histogram has),
         # to the left of the metrics axis
         $keyWindow create line 5 $top [expr [getWindowWidth $keyWindow] - 5] $top \
		 -tag metricsAxisTag -fill [getMetricColor $metriclcv]
         set theText $metricNames($metriclcv)
         label $keyWindow.key$numMetricsDrawn -text $theText \
		 -font $metricsLabelFont
         $keyWindow create window [expr [getWindowWidth $keyWindow] - 5] [expr $top + $tickHeight] \
		 -tag metricsAxisTag -window $keyWindow.key$numMetricsDrawn \
		 -anchor ne

         # prepare for next metric down:
         set top [expr $top + $tickHeight + 30]
         incr numMetricsDrawn
      }
   }

   set numMetricLabelsDrawn $labelDrawnCount

   if {$numMetricLabelsDrawn==0} {
      set newMetricsAxisHeight 5
   } else {
      set newMetricsAxisHeight $top
   }

   rethinkLeftSectionWidth

   # resize the metrics axis to consume just the right amount of height
   # we use the "pack progagate" command to avoid resizing the entire window
   # syntax "pack progagate master flag"
   pack propagate . false
      $W.metricsAxisCanvas configure -height $newMetricsAxisHeight
      pack $W.metricsAxisCanvas -side bottom -fill x -expand false
         # expand is set to false; if the window is made wider, we don't want
         # extra width to go to the metrics axis
      $W.farLeft.sbPadding configure -height $newMetricsAxisHeight
      $W.left.metricsKey   configure -height $newMetricsAxisHeight
   pack propagate . true
}

proc getMetricColor {mindex} {
   global barColors
   global numBarColors

   set theindex [expr $mindex % $numBarColors]
   return $barColors($theindex)
}

# myConfigureEventHandler - handle a resize of the bar sub-window
proc myConfigureEventHandler {newWidth newHeight} {
   # rethink how wide the resources should be
   rethinkResourceHeights $newHeight

   # Call drawResourcesAxis to rethink the scrollbar and to
   # rethink how many resources can fit on the screen, now that
   # we've changed the window size
   drawResourcesAxis $newHeight

   # We only need to redraw the metrics axis if the window height has changed
   # (and at the beginning of the program)
   drawMetricsAxis $newWidth

   # the following routines will clear the bar window (ouch! But no
   # choice since window size change can greatly affect bar layout
   # --- sometimes)
   # so resizeCallback has built-in hacks to simulate one new-data callback

   resourcesAxisHasChanged $newHeight
   metricsAxisHasChanged   $newWidth

   resizeCallback $newWidth $newHeight
}

# myExposeEventHandler -- handle an expose in the bar sub-window
#    (no need to handle exposes in the other windows because they're
#     made of tcl widgets which redraw themselves)
proc myExposeEventHandler {} {
   # all tk widgets redraw automatically (though not 'till the next idle)

   # all that's left to do is inform our C++ code of the expose
   exposeCallback
}

#proc addResource {rName} {
#   global numResources
#   global resourceNames
#   global W
#
#   # first, make sure this resource doesn't already exist
#   for {set rindex 0} {$rindex < $numResources} {incr rindex} {
#      if {$validResources($rindex) && $resourceNames($rindex) == $rName} {
#         puts stderr "detected a duplicate resource: $rname (ignoring addition request)"
#         return
#      } 
#   }
#
#   set resourceNames($numResources) $rName
#   set validResources($numResources) [isResourceValid $numResources]
#   incr numResources
#
#   drawResourcesAxis [getWindowWidth $W.left.resourcesAxisCanvas]
#}

# delResource -- delete a resource, given the resource's true (not sorted)
#                index number.
#                Should match the resource we have clicked on.
proc delResource {delindex} {
   global numMetrics
   global numResources
   global validResources
   global resourceNames
   global indirectResources
   global clickedOnResource
   global clickedOnResourceText
   global Wmbar
   global W

   # first, make sure this resource index is valid
   if {$delindex < 0 || $delindex >= $numResources} {
      puts stderr "delResource -- ignoring out of bounds index: $delindex"
      return
   }

   if {![isResourceValid $delindex]} {
      puts stderr "delResource -- ignoring request to delete an invalid (already deleted?) resource"
      return
   }

   # puts stderr "Welcome to delResource--delindex=$delindex"

   # we should be deleting the resource that was clicked on
   if {$clickedOnResourceText == $resourceNames($delindex)} {
      set clickedOnResource ""
      $Wmbar.actions.m entryconfigure 4 -state disabled \
              -label "Remove Selected Resource" \
              -command {puts stderr "ignoring unexpected deletion..."}
   } else {
      puts stderr "delResource -- no mbar changes since $clickedOnResourceText != $resourceNames($delindex)"
   }

   # Inform that visi lib that we don't want to receive anything
   # more about this resource
   # NOTE: unfortunately, [Dg numResources] etc. will not be
   #       lowered, even after this is done!
   #       The temporary solution is to rigidly test the
   #       Valid bit of each metric-resource pair before
   #       using it in any way.
   for {set mindex 0} {$mindex < $numMetrics} {incr mindex} {
      if {[isMetricValid $mindex]} {
         Dg stop $mindex $delindex
      }
   }

   # note that this resource is now invalid
   if {[isResourceValid $delindex]} {
      puts stderr "delResource -- curious that valid flag wasn't changed to false after the deletion"
      flush stderr
   }

   # reminder: no use in rethinking "numResources" with a call to [Dg numresources]
   #           since the visi won't lower that value; instead, it will only clear
   #           the appropriate valid bits

   rethinkIndirectResources true

   # simulate a resize to rethink bar, bar label, and resource axis layouts
   myConfigureEventHandler [getWindowWidth $W.body] [getWindowHeight $W.body]
}

# proc delResourceByName -- user clicked on a resource name and then
#                           chose the menu item to delete it
#      calls delResource when it determines an appropriate index number
proc delResourceByName {rName} {
   global numResources
   global resourceNames
   global indirectResources

   # find the appropriate index and call delResource...
   for {set rindex 0} {$rindex < $numResources} {incr rindex} {
      if {$rName == $resourceNames($rindex)} {
         if {! [isResourceValid $rindex]} {
            puts stderr "delResourceByName -- ignoring request to delete invalid (already-deleted?) resource $rName"
            return
         }

         # note that the number being sent is the true index, not the sorted one
         delResource $rindex
         return
      }
   }

   puts stderr "delResourceByName: ignoring request to delete resource named: $rName (no such resource?)"
}

proc getMetricHints {theMetric} {
   # #pragma HACK begin
   # return metric unit type, hinted min, hinted max, hinted step
   # depending on the metric (a hardcoded temporary hack)
   switch $theMetric {
      "exec_time"        {return {percentage 0.0 1.0 0.1}}
      "hybrid_cost"      {return {real 0.0 1.0 0.1}}
      "procedure_calls"  {return {integer 0 1000 100}}
      "predicted_cost"   {return {real 0.0 1.0 .1}}
      "msgs"             {return {integer 0 10 10}}
      "msg_bytes"        {return {integer 0 100 100}}
      "pause_time"       {return {percentage 0.0 1.0 .1}}
      "msg_bytes_sent"   {return {integer 0 100 100}}
      "msg_bytes_recv"   {return {integer 0 100 100}}
      "active_processes" {return {integer 0 1 1}}
      "sync_ops"         {return {real 0.0 5 1}}
      "observed_cost"    {return {real 0.0 1.0 0.1}}
      "sync_wait"        {return {integer 0.0 5 1}}
      "active_slots"     {return {integer 0.0 2 1}}
      "cpu"              {return {real 0.0 1.0 0.1}}
   }

   puts stderr "getMetricHints--unexpected metric: $theMetric...continuing"
   return {real 0.0 1.0 0.1}
   # #pragma HACK done
}

proc addMetric {theName theUnits} {
   global numMetrics
   global metricNames
   global validMetrics

   global metricUnits
   global metricUnitTypes
   global metricMinValues
   global metricMaxValues
   global W

   puts stderr "Welcome to addMetric; name is $theName; units are $theUnits"

   # first make sure that this metric isn't already present (if so, do nothing)
   for {set metricIndex 0} {$metricIndex < $numMetrics} {incr metricIndex} {
      if {$metricNames($metricIndex) == $theName && $validMetrics($metricIndex)} {
         puts stderr "addMetric: ignoring request to add $theName (already present)"
         return
      }
   }

   # make the addition
   set metricNames($numMetrics) $theName
   set metricUnits($numMetrics) $theUnits

   set theHints [getMetricHints $theName]

   set metricUnitTypes($numMetrics) [lindex $theHints 0]
   set metricMinValues($numMetrics) [lindex $theHints 1]
   set metricMaxValues($numMetrics) [lindex $theHints 2]

   incr numMetrics

   drawResourcesAxis [getWindowHeight $W.left.resourcesAxisCanvas]
   drawMetricsAxis [getWindowWidth $W.metricsAxisCanvas]
}

proc delMetric {delIndex} {
   global numMetrics
   global metricNames
   global metricUnits
   global W

   # first, make sure this metric index is valid
   if {$delIndex < 0 || $delIndex >= $numMetrics} {
      puts stderr "delMetric: ignoring out of bounds index: $delIndex"
      return
   }

   # shift
   for {set mindex $delIndex} {$mindex < [expr $numMetrics-1]} {incr mindex} {
      set metricNames($mindex) $metricNames([expr $mindex + 1])
      set metricUnits($mindex) $metricUnits([expr $mindex + 1])
   }

   set numMetrics [expr $numMetrics - 1]
   
   drawResourcesAxis [getWindowHeight $W.left.resourcesAxisCanvas]
   drawMetricsAxis [getWindowWidth $W.metricsAxisCanvas]

   # don't we need to tell paradyn to stop sending us data on
   # this metric?
}

proc processNewScrollPosition {newTop} {
   # the -command configuration of the scrollbar is fixed to call this procedure.
   # This happens whenever scrolling takes place.  We update the x-axis canvas
   global WresourcesCanvas
   global W

   if {$newTop < 0} {
      set newTop 0
   }

   # if max <= visible then set newTop 0
   set currSettings [$W.farLeft.resourcesAxisScrollbar get]
   set totalSize [lindex $currSettings 0]
   set visibleSize [lindex $currSettings 1]

   if {$visibleSize > $totalSize} {
      set newTop 0
   } elseif {[expr $newTop + $visibleSize] > $totalSize} {
      set newTop [expr $totalSize - $visibleSize]
   }

   # update the x-axis canvas
   # will automatically generate a call to myXScroll, which then updates the
   # look of the scrollbar to reflect the new position...
   $WresourcesCanvas yview $newTop

   # call our C++ code to update the bars
   newScrollPosition $newTop
}

# myXScroll -- the -scrollcommand config of the resources axis canvas.
#              Gets called whenever the canvas view changes or gets resized.
#              Gives us an opportunity to rethink the bounds of the scrollbar.
#              We get passed: total size, window size, left, right
proc myXScroll {totalSize visibleSize left right} {
   global W

   $W.farLeft.resourcesAxisScrollbar set $totalSize $visibleSize $left $right
}

# ############################################################################
# ############# blt_drag&drop: declare that we are willing and able ##########
# ######### to receive drag n' drops of type "text" (the type may change) ####
# ############################################################################

proc dragAndDropTargetHandler {} {
   # according to the drag n' drop interface, this routine will be
   # called via a "send" command from the source.  So don't expect
   # to see this routine called from elsewhere in this file...

   # the variable DragDrop(text) contains what should be added
   global DragDrop

   # not yet implemented...
   puts stderr "Welcome to dragAndDropTargetHandler(); DragDrop(text) is $DragDrop(text)"
   addResource $DragDrop(text)
}

#blt_drag&drop target . handler text dragAndDropTargetHandler
#...that cryptic line reads: "declare the window '.' to be a drag n' drop
#   handler for sources of type 'text'; routine dragAndDropTargetHandler
#   gets called (via a "send" from the source...)  Using window '.' means
#   the entire barchart...

# #################### Called by visi library when histos have folded #################

proc DgFoldCallback {} {
   puts stderr "FOLD detected..."
   flush stderr
}

# ########### Called by visi library when metric/resource space changes. #######
#
# note: this routine is too generic; in the future, we plan to
# implement callbacks that actually tell what was added (as opposed
# to what was already there...)
#
# ###############################################################################

proc DgConfigCallback {} {
   # puts stderr "Welcome to DgConfigCallback"
   # flush stderr

   global W

   global numMetrics
   global metricNames
   global validMetrics
   global metricUnits
   global metricMinValues
   global metricMaxValues

   global numResources
   global numResourcesDrawn
   global resourceNames
   global indirectResources

   set numMetrics [Dg nummetrics]
   set numResources [Dg numresources]

   for {set metriclcv 0} {$metriclcv < $numMetrics} {incr metriclcv} {
      set metricNames($metriclcv) [Dg metricname  $metriclcv]
      set metricUnits($metriclcv) [Dg metricunits $metriclcv]
      set validMetrics($metriclcv) [isMetricValid $metriclcv]

      # note -- the following 2 lines are very dubious for already-existing
      #         resources (i.e. we should try to stick with the initial values)
      set metricMinValues($metriclcv) [lindex [getMetricHints $metricNames($metriclcv)] 1]
      set metricMaxValues($metriclcv) [lindex [getMetricHints $metricNames($metriclcv)] 2]
   }

   for {set resourcelcv 0} {$resourcelcv < $numResources} {incr resourcelcv} {
      set resourceNames($resourcelcv) [file tail [Dg resourcename $resourcelcv]]
   }

   # rethink the sorting order (false --> don't do callback to c++ code because it
   # would crash since C++ code doesn't update its value of numMetrics and numResources
   # until a 'resourcesAxisHasChanged' or 'metricsAxisHasChanged'.  When those do
   # indeed get called below, they also update the sorting order; so we're OK.)
   rethinkIndirectResources false

   # rethink the layout of the axes
   rethinkResourceHeights [getWindowHeight $W.left.resourcesAxisCanvas]
   drawResourcesAxis [getWindowHeight $W.left.resourcesAxisCanvas]
   drawMetricsAxis [getWindowWidth $W.metricsAxisCanvas]

   # inform our C++ code that stuff has changed (nummetrics, numresources gets
   # updated, structures are recalculated based on new #metrics,#resources, etc.)
   resourcesAxisHasChanged [getWindowHeight $W.left.resourcesAxisCanvas]
   metricsAxisHasChanged   [getWindowWidth $W.metricsAxisCanvas]
}

# #################  AddMetricDialog -- Ask paradyn for another metric #################

proc AddMetricDialog {} {
   Dg start "*" "*"
}

# #################  AddResourceDialog -- Ask paradyn for another resource #################

proc AddResourceDialog {} {
   Dg start "*" "*"
}

# A menu item was chosen to change the sorting options
proc ProcessNewSortPrefs {} {
   global SortPrefs
   global W

   # change the order...
   rethinkIndirectResources true

   # redraw the resources axis
   drawResourcesAxis [getWindowHeight $W.left.resourcesAxisCanvas]

   # redraw the bars (callback to our C++ code)
   resourcesAxisHasChanged [getWindowHeight $W.left.resourcesAxisCanvas]
}

proc sortCmd {x y} {
   set str1 [string toupper [lindex $x 1]]
   set str2 [string toupper [lindex $y 1]]

   return [string compare $str1 $str2]
}

proc rethinkIndirectResources {docallback} {
   # sorting order has changed; rethink indirectResources array
   global indirectResources
   global SortPrefs
   global numResources
   global resourceNames

   # sorting works as follows: create a temporary list of {index,name} pairs;
   # sort the list; extract the indexes in sequence; delete temporary list
   set templist {}

   for {set resourcelcv 0} {$resourcelcv < $numResources} {incr resourcelcv} {
      lappend templist [list $resourcelcv $resourceNames($resourcelcv)]
   }

   # puts stderr "rethinkIndirectResources: templist is $templist"

   if {$SortPrefs == "ByName"} {
      set templist [lsort -ascii -increasing -command sortCmd $templist]
   } elseif {$SortPrefs == "ByNameDescending"} {
      set templist [lsort -ascii -decreasing -command sortCmd $templist]
   }

   # puts stderr "rethinkIndirectResources: sorted templist is $templist"

   for {set resourcelcv 0} {$resourcelcv < $numResources} {incr resourcelcv} {
      set indirectResources($resourcelcv) [lindex [lindex $templist $resourcelcv] 0]
   }

#   puts stderr "rethinkIndirectResources: leaving with indirectResources="
#   for {set resourcelcv 0} {$resourcelcv < $numResources} {incr resourcelcv} {
#      puts stderr $indirectResources($resourcelcv)
#   }

   if {$docallback} {
      # inform our C++ code of the changes...
      rethinkIndirectResourcesCallback
   }
}

proc rethinkDataFormat {} {
   # invoked when a menu item from among "current, average, total"
   # is selected
   global W
   global DataFormat
   global numMetrics
   global metricMinValues
   global metricMaxValues
   global metricNames

   # reset metrics-axis min & max values
   for {set metriclcv 0} {$metriclcv < $numMetrics} {incr metriclcv} {
      set metricMinValues($metriclcv) [lindex [getMetricHints $metricNames($metriclcv)] 1]
      set metricMaxValues($metriclcv) [lindex [getMetricHints $metricNames($metriclcv)] 2]
   }

   # inform our C++ code that the data format has changed
   dataFormatHasChanged

   # redraw the y axis
   drawMetricsAxis [getWindowWidth $W.metricsAxisCanvas]
}

proc GracefulClose {} {
   # quit barchart

   # release installed commands
   rename launchBarChart ""

   # the above command will render the callback routines harmless
   # by virtue of setting barChartIsValid to false
   # So, we can delete them at our leisure now...
   
   rename rethinkIndirectResourcesCallback ""
   rename dataFormatHasChanged ""
   rename newScrollPosition ""
   rename metricsAxisHasChanged ""
   rename resourcesAxisHasChanged ""
   rename exposeCallback ""
   rename resizeCallback ""

   exit
}

# ######################################################################################
#                           "Main Program"
# ######################################################################################

Initialize
