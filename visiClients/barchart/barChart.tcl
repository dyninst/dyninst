#
#  barChart -- A bar chart display visualization for Paradyn
#
#  $Log: barChart.tcl,v $
#  Revision 1.8  1994/10/13 00:51:39  tamches
#  Fixed deletion of resources.
#  Implemented sorting of resources.
#  Reorganized menus to be more standars-ish
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
# 0) too much flickering on resize
# 1) sort after delete or delete after sort doesn't work -- both axes
# 2) swap axes
# 3) multiple metrics: put a "key" on screen
# 4) multiple metrics: make them show on y axis
# 5) multiple metrics: allow deletion
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

set metricsAxisWidth 0.75i
set resourcesAxisHeight 0.65i

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
$Wmbar.file.m add command -label "Close Bar chart" -command exit

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

# ####################  Barchart Title ("Metric: xxxxxxxx") #################

label $W.titleLabel -text "(no barchart currently loaded)" \
                    -font "-adobe-helvetica-bold-o-*-*-*-*-*-*-*-*-iso8859-1"
pack  $W.titleLabel -side top -fill x -expand false
   # expand is set to false; if the window is made taller, we don't want
   # any of the extra space to go to the title (we want it to go to the
   # bar graph main area in the middle)


# ####################  Barchart Resources Axis Title ("Resource(s)") ###############

label $W.resourcesAxisTitle -text "Resource(s)" \
                    -font "-adobe-helvetica-bold-o-*-*-*-*-*-*-*-*-iso8859-1"
pack  $W.resourcesAxisTitle -side bottom -fill x -expand false
   # expand is set to false; if the window is made taller, we don't want
   # extra height to go to us.

# ###############  A sub-window to hold x-axis scrollbar & some padding ##########

canvas $W.sbRegion -height 16
   # on configure events, must update the -scrollregion
pack $W.sbRegion -side bottom -expand false -fill x
   # expand is set to false; if the window is made taller, we don't want
   # any of the extra height; let the middle section have it.

canvas $W.sbRegion.padding -width $metricsAxisWidth -height 16
pack $W.sbRegion.padding -side left -expand false
   # expand is set to false; if the window is made wider, we don't
   # want any of the extra width.

scrollbar $W.sbRegion.resourcesAxisScrollbar -orient horizontal -width 16 -foreground gray \
                            -activeforeground gray -relief sunken \
			    -command processNewScrollPosition
pack $W.sbRegion.resourcesAxisScrollbar -side right -fill x -expand true
   # expand is set to true; if the window is made wider, we want
   # extra width.

# #############  A sub-window to hold x-axis canvas & some padding #########

canvas $W.bottom -height $resourcesAxisHeight
pack   $W.bottom -side bottom -expand false -fill x
   # expand is set to false; if the window is made taller, we don't want
   # any of the extra height; let the middle section have it.

canvas $W.bottom.padding -width $metricsAxisWidth -height $resourcesAxisHeight
pack   $W.bottom.padding -side left -expand false
   # expand is set to false; if the window is made wider, we don't
   # want any of the extra width.
   # We are the blank space at (0,0); we exist to keep the X and Y
   # axis from getting any of this area, which would be difficult to manage.

set WresourcesCanvas $W.bottom.resourcesAxisCanvas
canvas $WresourcesCanvas -height $resourcesAxisHeight -relief groove \
                             -xscrollcommand myXScroll \
			     -scrollincrement 1
pack   $WresourcesCanvas -side right -expand true -fill x
   # expand is set to true; if the window is made wider, we want
   # extra width.

# ####################  Y Axis Canvas ##################################

canvas $W.metricsAxisCanvas -width $metricsAxisWidth
pack   $W.metricsAxisCanvas -side left -fill y -expand false
   # expand is set to false; if the window is made wider, we don't want
   # extra width to go to the y axis

# ####################  Barchart Area ($W.body) #################

canvas $W.body -height 3i -width 2.5i -relief groove
#frame $W.body -height 3i -width 2.5i -relief groove
pack  $W.body -side top -fill both -expand true
   # expand is set to true; if the window is made taller, we want the
   # extra height to go to us (and our slave window $W.body.barCanvas)

# ####################  Bar Canvas ##############################

#frame  $W.body.barCanvas -relief groove -background red
#pack   $W.body.barCanvas -side top -fill both -expand true

#canvas $W.body.barCanvas -relief groove
#pack   $W.body.barCanvas -side top -fill both -expand true

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
      set result [winfo reqwidth $wName]
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
   global metricNames
   global validMetrics

   global metricUnits
   global metricMinValues
   global metricMaxValues

   global numResources
   global resourceNames
   global indirectResources

   global currResourceWidth
   global minResourceWidth
   global maxResourceWidth

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

   set minResourceWidth 45
   set maxResourceWidth 90
   set currResourceWidth $maxResourceWidth
      # as resources are added, we try to shrink the resource width down to a minimum of
      # (minResourceWidth) rather than having to invoke the scrollbar.
      # The question then becomes, if the window is made wider, should we
      # tend "resourceWidth" back toward "maxResourceWidth", if it had been shrunk
      # toward "minResourceWidth"?  Or once a resource is shrunk, should it never
      # be enlarged?

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
# drawResourceAxis
# processNewMetricMax
# drawMetricAxis
# drawTitle
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

proc rethinkResourceWidths {screenWidth} {
   # When resources are added or deleted, this routine is called.
   # Its sole purpose is to rethink the value of currResourceWidth,
   # depending on the resources.

   # algorithm: get current window width.  set resource width equal
   # to window width / num resources.  If that would make the resource
   # width too small, make the resource width currResourceWidth
   global minResourceWidth
   global maxResourceWidth
   global currResourceWidth
   global numResources
   global WresourcesCanvas

   set tentativeResourceWidth [expr $screenWidth / $numResources]
   if {$tentativeResourceWidth < $minResourceWidth} {
      set tentativeResourceWidth $minResourceWidth
   } elseif {$tentativeResourceWidth > $maxResourceWidth} {
      set tentativeResourceWidth $maxResourceWidth
   }

   set currResourceWidth $tentativeResourceWidth

   # puts stderr "Leaving rethinkResourceWidths; we have decided upon $currResourceWidth"
}

proc drawResourcesAxis {resourcesAxisWidth} {
   # how it works: deletes all canvas items with the tag "resourcesAxisItemTag", including
   # the window items.  message widgets have to be deleted separately, notwithstanding
   # the fact that the canvas window items were deleted already.
   # (it knows how many message widgets there are via numResourcesDrawn, which at the
   # time this routine is called, may not be up-to-date with respect to numResources),
   # and then redraws by re-recreating the canvas items and message widgets

   global W
   global Wmbar
   global WresourcesCanvas

   global resourcesAxisHeight
   global metricsAxisWidth

   global numResources
   global numResourcesDrawn
   global resourceNames
   global indirectResources

   global clickedOnResource
   global clickedOnResourceText

   global minResourceWidth
   global maxResourceWidth
   global currResourceWidth

   global SortPrefs

   # puts stderr "Welcome to drawResourcesAxis; numResources=$numResources"

   set top 3
   set tickHeight 5
   set resourceNameFont -adobe-courier-medium-r-normal-*-12-120-*-*-*-*-iso8859-1

   # ###### delete leftover stuff.  First the canvas items in 1 step, then the message widgets manually
   #        (impossible to delete the message widgets in 1 step since they are not canvas items)
   $WresourcesCanvas delete resourcesAxisItemTag

   # loop through the message widgets in sorted order   
   for {set rindex 0} {$rindex < $numResourcesDrawn} {incr rindex} {
      set actualResource $indirectResources($rindex)
      if {[isResourceValid $actualResource]} {
         destroy $WresourcesCanvas.message$rindex
      }
   }

   # next, several tick marks extending down a few pixels from the resources axis
   # (one per resource), plus (while we're at it) the text of the given resources
   # (centered at their respective tick marks)

   # yet to implement: STAGGERED TEXT
   #         needed  : a way to detect when two message widgets have collided.
   #                   doesn't sound too difficult... (use winfo -geometry for each widget)

   set left 0
   set right 0
   # loop through resources in sorted order
   for {set rindex 0} {$rindex < $numResources} {incr rindex} {
      set actualResource $indirectResources($rindex)

      if {[isResourceValid $actualResource]} {
         set right [expr $left + $currResourceWidth - 1]
         set middle [expr ($left + $right) / 2]
   
         # create a tick line
         $WresourcesCanvas create line $middle $top $middle [expr $top+$tickHeight-1] -tag resourcesAxisItemTag
   
         # create a message widget, bind some commands to it, and attach it to the
         # canvas via "create window"

         set theName $resourceNames($actualResource)
   
         message $WresourcesCanvas.message$rindex -text $theName \
		 -justify center -width $currResourceWidth \
		 -font $resourceNameFont

         bind $WresourcesCanvas.message$rindex <Enter> \
                             {processEnterResource %W}
         bind $WresourcesCanvas.message$rindex <Leave> \
                             {processExitResource %W}
         bind $WresourcesCanvas.message$rindex <ButtonPress> \
		             {selectResource %W}

         $WresourcesCanvas create window $middle [expr $top+$tickHeight] \
		 -anchor n -tag resourcesAxisItemTag \
		 -window $WresourcesCanvas.message$rindex
    
         set left [expr $left + $currResourceWidth]
      } else {
         #puts stderr "drawResourcesAxis: not drawing resource #$rindex because its valid flag is false"
      }
   }

   # the axis itself--a horizontal line
   $WresourcesCanvas create line 0 $top $right $top -tag resourcesAxisItemTag

   # Update the scrollbar's scrollregion configuration:
   set regionList {0 0 0 0}
   set regionList [lreplace $regionList 2 2 $right]
   set regionList [lreplace $regionList 3 3 $resourcesAxisHeight]
   $WresourcesCanvas configure -scrollregion $regionList

   set screenWidth $resourcesAxisWidth

   set oldconfig [$W.sbRegion.resourcesAxisScrollbar get]
   set oldTotalWidth [lindex $oldconfig 0]

   if {$oldTotalWidth != $right} {
      # puts stderr "drawResourcesAxis: detected major change in resources ($oldTotalWidth != $right), resetting"
      set firstUnit 0
   } else {
      # no change
      set firstUnit [lindex $oldconfig 2]
   }

   set lastUnit [expr $firstUnit + $screenWidth - 1]
   # puts stderr "setting sb: $right $screenWidth $firstUnit $lastUnit"   
   $W.sbRegion.resourcesAxisScrollbar set $right $screenWidth $firstUnit $lastUnit
                                  
   # set the maximum width of the window to be $right + $metricsAxisWidth
   # wm maxsize . [expr $right + inches2pixels($metricsAxisWidth)] [unlimited-y-size]

   set numResourcesDrawn $numResources
}

proc processNewMetricMax {mindex newmaxval} {
   # called from barChart.C when y-axis overflow is detected and
   # a new max value is chosen
   global metricMinValues
   global metricMaxValues
   global W

   set metricMaxValues($mindex) $newmaxval

   drawMetricsAxis [getWindowHeight $W.metricsAxisCanvas]
}

proc drawMetricsAxis {metricsAxisHeight} {
   # the y axis changes to reflect the units of the current metric(s).
   # It is not necessary to call drawMetricsAxis if the window width is
   # resized; it IS necessary to call drawMetricsAxis if the window
   # height is changed.
   # It is not necessary to call drawMetricsAxis if resources are
   # added or removed; it IS necessary to call drawMetricsAxis if
   # metrics are added or removed.

   global W
   global numMetrics
   global numLabelsDrawn

   global metricNames
   global validMetrics

   global metricUnits
   global metricUnitTypes
   global metricMinValues
   global metricMaxValues

   # puts stderr "welcome to drawMetricsAxis"

   # first, delete all leftover canvas items (those with a metricsAxis tag to them)
   $W.metricsAxisCanvas delete metricsAxisTag

   # we still have to delete the label widgets, which can't have tags...
   set numlabels 3
   for {set labelindex 0} {$labelindex < $numLabelsDrawn} {incr labelindex} {
      destroy $W.metricsAxisCanvas.label$labelindex
   }

   # canvas item: the axis itself (a vertical line)
   set winHeight $metricsAxisHeight
   set winWidth [getWindowWidth $W.metricsAxisCanvas]

   set tickWidth 5
   set left 5
   set right [expr $winWidth - 5]
   set tickLeft [expr $right-$tickWidth+1]
   set top   5
   set bottom [expr $winHeight - 5]
   set labelRight [expr $tickLeft]
   set labelWidth [expr $labelRight - $left + 1]
   set height [expr $bottom - $top + 1]
   set tickStep [expr ($height-1) / ($numlabels-1)]

   set numericalStep [expr (1.0 * $metricMaxValues(0)-$metricMinValues(0)) / ($numlabels-1)]

   # puts stderr "drawMetricsAxis: numericalStep is $numericalStep; metric-min is $metricMinValues(0); metric-max is $metricMaxValues(0)"
   # flush stderr

   $W.metricsAxisCanvas create line $right $top $right $bottom -tag metricsAxisTag

   set labelFont -adobe-courier-medium-r-normal-*-12-120-*-*-*-*-iso8859-1
   # draw tick marks, and while we're at it, their associated message widgets
   for {set labelindex 0} {$labelindex < $numlabels} {incr labelindex} {
      set currY [expr $bottom - round($labelindex*$tickStep)]

      $W.metricsAxisCanvas create line $tickLeft $currY $right $currY -tag metricsAxisTag

      set labelText [expr $metricMinValues(0) + $labelindex*$numericalStep]
      message $W.metricsAxisCanvas.label$labelindex -text $labelText \
              -justify right -width $labelWidth \
	      -font $labelFont

      $W.metricsAxisCanvas create window $labelRight $currY -anchor e \
                        -tag metricsAxisItemTag \
			-window $W.metricsAxisCanvas.label$labelindex
   }

   set numLabelsDrawn $numlabels
}

proc drawTitle {} {
   # the title changes to reflect the current metric(s)
   # currently, only a single metric is supported

   global W
   global numMetrics

   global metricNames
   global validMetrics

   global metricUnits

   if {$numMetrics == 0} {
      set newTitle "(no metrics currently defined)"
   } else {
      set newTitle "Metric: $metricNames(0) (Units: $metricUnits(0))"
   }

   $W.titleLabel configure -text $newTitle
}

# myConfigureEventHandler - handle a resize of the bar sub-window
proc myConfigureEventHandler {newWidth newHeight} {
   # rethink how wide the resources should be
   rethinkResourceWidths $newWidth

   # Call drawResourcesAxis to rethink the scrollbar and to
   # rethink how many resources can fit on the screen, now that
   # we've changed the window size
   drawResourcesAxis $newWidth

   # We only need to redraw the metrics axis if the window height has changed
   # (and at the beginning of the program)
   drawMetricsAxis $newHeight

   # Redraw the title (only needed if metrics changed)
   drawTitle
   
   # the following routines will clear the bar window (ouch! But no
   # choice since window size change can greatly affect bar layout
   # --- sometimes)
   # so resizeCallback has built-in hacks to simulate one new-data callback

   resourcesAxisHasChanged $newWidth
   metricsAxisHasChanged   $newHeight

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
#   drawResourcesAxis [getWindowWidth $W.bottom.resourcesAxisCanvas]
#}

# delResource -- delete a resource, given the resource's true (not sorted)
#                index number.
#                Should match the resource we have clicked on.
proc delResource {delindex} {
   global numMetrics
   global numResources
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

   puts stderr "Welcome to delResource--delindex=$delindex"

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
      Dg stop $mindex $delindex
   }

   #puts stderr "Dg stop executed for resource #$delindex"
   #flush stderr

   # note that this resource is now invalid
   if {[isResourceValid $delindex]} {
      puts stderr "delResource -- curious that valid flag wasn't changed to false after the deletion"
      flush stderr
   }

   # reminder: no use in rethinking "numResources" with a call to [Dg numresources]
   #           since the visi won't lower that value; instead, it will only clear
   #           the appropriate valid bits

   rethinkIndirectResources

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

   drawResourcesAxis [getWindowWidth $W.bottom.resourcesAxisCanvas]
   drawMetricsAxis [getWindowHeight $W.metricsAxisCanvas]
   drawTitle
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
   
   drawTitle
   drawResourcesAxis [getWindowWidth $W.bottom.resourcesAxisCanvas]
   drawMetricsAxis [getWindowHeight $W.metricsAxisCanvas]

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
   set currSettings [$W.sbRegion.resourcesAxisScrollbar get]
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
   $WresourcesCanvas xview $newTop

   # call our C++ code to update the bars
   newScrollPosition $newTop
}

# myXScroll -- the -scrollcommand config of the resources axis canvas.
#              Gets called whenever the canvas view changes or gets resized.
#              Gives us an opportunity to rethink the bounds of the scrollbar.
#              We get passed: total size, window size, left, right
proc myXScroll {totalSize visibleSize left right} {
   global W

   $W.sbRegion.resourcesAxisScrollbar set $totalSize $visibleSize $left $right
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

   # rethink the sorting order
   rethinkIndirectResources

   # rethink the layout of the axes
   rethinkResourceWidths [getWindowWidth $W.bottom.resourcesAxisCanvas]
   drawResourcesAxis [getWindowWidth $W.bottom.resourcesAxisCanvas]
   drawMetricsAxis [getWindowHeight $W.metricsAxisCanvas]
   drawTitle

   # inform our C++ code that stuff has changed
   resourcesAxisHasChanged [getWindowWidth $W.bottom.resourcesAxisCanvas]
   metricsAxisHasChanged   [getWindowHeight $W.metricsAxisCanvas]
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
   rethinkIndirectResources

   # redraw the resources axis
   drawResourcesAxis [getWindowWidth $W.bottom.resourcesAxisCanvas]

   # redraw the bars (callback to our C++ code)
   resourcesAxisHasChanged [getWindowWidth $W.bottom.resourcesAxisCanvas]
}

proc sortCmd {x y} {
   set str1 [string toupper [lindex $x 1]]
   set str2 [string toupper [lindex $y 1]]

   return [string compare $str1 $str2]
}

proc rethinkIndirectResources {} {
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

   # inform our C++ code of the changes...
   rethinkIndirectResourcesCallback
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

   # reset y-axis min & max values
   for {set metriclcv 0} {$metriclcv < $numMetrics} {incr metriclcv} {
      set metricMinValues($metriclcv) [lindex [getMetricHints $metricNames($metriclcv)] 1]
      set metricMaxValues($metriclcv) [lindex [getMetricHints $metricNames($metriclcv)] 2]
   }

   # inform our C++ code that the data format has changed
   dataFormatHasChanged

   # redraw the y axis
   drawMetricsAxis [getWindowHeight $W.metricsAxisCanvas]
}

# ######################################################################################
#                           "Main Program"
# ######################################################################################

Initialize
