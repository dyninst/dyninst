#
#  barChart -- A bar chart display visualization for Paradyn
#
#  $Log: barChart.tcl,v $
#  Revision 1.3  1994/10/04 19:00:23  tamches
#  implemented resourceWidth algorithm: try to make resources the maximum
#  pixel width, but if they don't all fit in the window, shrink (down
#  to a fixed minimum).  Reapply algorithm when: window resizes, resources
#  are added/deleted.
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
# areas.  improved look of x axis.
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
# 1) draw numerical values on the bars (menu option) (default=on?)
# 2) resources: make deletion work
# 3) staggered x-axis names
# 4) multiple metrics: put a "key" on screen
# 5) multiple metrics: make them show on y axis
# 6) multiple metrics: allow deletion
# 7) option to sort resources (will be difficult--would need to map resourceid
#    as given by visi to our new ordering)
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

set yAxisWidth 0.75i
set xAxisHeight 0.65i

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
                  -foreground DarkOliveGreen

pack $W.top.logo -side right -expand false
   # we set expand to false; if the window is made wider, we
   # don't want any of the extra space; let the menu bar and
   # title bar have it.

# #################### Title bar #################

label $W.top.left.titlebar  -text "BarChart Visualization" -foreground white -background DarkOliveGreen
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

# #################### Metrics menu ###################

menubutton $Wmbar.metrics -text Metrics -menu $Wmbar.metrics.m
menu $Wmbar.metrics.m
$Wmbar.metrics.m add command -label "Add Metric..." -command AddMetricDialog
$Wmbar.metrics.m add command -label "Remove Selected Metric" -state disabled

# #################### Resources menu #################

menubutton $Wmbar.resources -text Resources -menu $Wmbar.resources.m
menu $Wmbar.resources.m
$Wmbar.resources.m add command -label "Add Resource..." -command AddResourceDialog
$Wmbar.resources.m add command -label "Remove Selected Resource" -state disabled
$Wmbar.resources.m add separator
$Wmbar.resources.m add check -label "Long Resource Names" -variable LongNames -command LongNamesChange
$Wmbar.resources.m add separator
$Wmbar.resources.m add radio -label "Order by Name (ascending)" -variable SortPrefs -command ProcessNewSortPrefs -value ByName
$Wmbar.resources.m add radio -label "Order by Name (descending)" -variable SortPrefs -command ProcessNewSortPrefs -value ByNameDescending
$Wmbar.resources.m add radio -label "Order as Inserted by User" -variable SortPrefs -command ProcessNewSortPrefs -value NoParticular

# #################### Display menu #################

menubutton $Wmbar.opts -text "Display" -menu $Wmbar.opts.m
menu $Wmbar.opts.m
#$Wmbar.opts.m add check -label "Long Names" -variable LongNames \
#   -command Update -state disabled
#$Wmbar.opts.m add separator
$Wmbar.opts.m add radio -label "Current Value" \
   -variable DataFormat -command {rethinkDataFormat} \
   -value Instantaneous
$Wmbar.opts.m add radio -label "Average Value" \
   -variable DataFormat -command {rethinkDataFormat} \
   -value Average
$Wmbar.opts.m add radio -label "Total Value" \
   -variable DataFormat -command {rethinkDataFormat} \
   -value Sum

# #################### Help menu #################

menubutton $Wmbar.help -text Help \
          -menu $Wmbar.help.m
menu $Wmbar.help.m 
$Wmbar.help.m add command -label "General" -command "NotImpl" -state disabled
$Wmbar.help.m add command -label "Context" -command "NotImpl" -state disabled


# #################### Build the menu bar and add to display #################

pack $Wmbar.file $Wmbar.metrics \
     $Wmbar.resources $Wmbar.opts \
     -side left -padx 4
pack $Wmbar.help -side right

# #################### Organize all menu buttons into a menubar #################

tk_menuBar $Wmbar $Wmbar.file $Wmbar.metrics \
   $Wmbar.resources $Wmbar.opts $Wmbar.help

# ####################  Barchart Title ("Metric: xxxxxxxx") #################

label $W.titleLabel -text "(no barchart currently loaded)" \
                    -font "-adobe-helvetica-bold-o-*-*-*-*-*-*-*-*-iso8859-1"
pack  $W.titleLabel -side top -fill x -expand false
   # expand is set to false; if the window is made taller, we don't want
   # any of the extra space to go to the title (we want it to go to the
   # bar graph main area in the middle)


# ####################  Barchart X Axis Title ("Resource(s)") ###############

label $W.xAxisTitle -text "Resource(s)" \
                    -font "-adobe-helvetica-bold-o-*-*-*-*-*-*-*-*-iso8859-1"
pack  $W.xAxisTitle -side bottom -fill x -expand false
   # expand is set to false; if the window is made taller, we don't want
   # extra height to go to us.

# ###############  A sub-window to hold x-axis scrollbar & some padding ##########

canvas $W.sbRegion -height 16
   # on configure events, must update the -scrollregion
pack $W.sbRegion -side bottom -expand false -fill x
   # expand is set to false; if the window is made taller, we don't want
   # any of the extra height; let the middle section have it.

canvas $W.sbRegion.padding -width $yAxisWidth -height 16
pack $W.sbRegion.padding -side left -expand false
   # expand is set to false; if the window is made wider, we don't
   # want any of the extra width.

scrollbar $W.sbRegion.xAxisScrollbar -orient horizontal -width 16 -foreground gray \
                            -activeforeground gray -relief sunken \
			    -command processNewScrollPosition
pack $W.sbRegion.xAxisScrollbar -side right -fill x -expand true
   # expand is set to true; if the window is made wider, we want
   # extra width.

# #############  A sub-window to hold x-axis canvas & some padding #########

canvas $W.bottom -height $xAxisHeight
pack   $W.bottom -side bottom -expand false -fill x
   # expand is set to false; if the window is made taller, we don't want
   # any of the extra height; let the middle section have it.

canvas $W.bottom.padding -width $yAxisWidth -height $xAxisHeight
pack   $W.bottom.padding -side left -expand false
   # expand is set to false; if the window is made wider, we don't
   # want any of the extra width.
   # We are the blank space at (0,0); we exist to keep the X and Y
   # axis from getting any of this area, which would be difficult to manage.

set Wxcanvas $W.bottom.xAxisCanvas
canvas $Wxcanvas -height $xAxisHeight -relief groove \
                             -xscrollcommand myXScroll \
			     -scrollincrement 1
pack   $Wxcanvas -side right -expand true -fill x
   # expand is set to true; if the window is made wider, we want
   # extra width.

# ####################  Y Axis Canvas ##################################

canvas $W.yAxisCanvas -width $yAxisWidth
pack   $W.yAxisCanvas -side left -fill y -expand false
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
   set result [winfo width $wName]
   if {$result == 1} {
      set result [winfo reqwidth $wName]
   }

   return $result
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
   global metricUnits
   global metricMinValues
   global metricMaxValues

   global numResources
   global resourceNames

   global currResourceWidth
   global minResourceWidth
   global maxResourceWidth

   global DataFormat

   global clickedOnResource
   global clickedOnResourceText
   global numLabelsDrawn
   global numResourcesDrawn

   global SortPrefs

   set SortPrefs NoParticular
   
   set clickedOnResource ""
   set clickedOnResourceText ""
   set numLabelsDrawn 0
   set numResourcesDrawn 0

   set DataFormat Instantaneous
   set numMetrics [Dg nummetrics]

   for {set metriclcv 0} {$metriclcv < $numMetrics} {incr metriclcv} {
      set metricNames($metriclcv) [Dg metricname  $metriclcv]
      set metricUnits($metriclcv) [Dg metricunits $metriclcv]
      set metricMinValues($metriclcv) [lindex [getMetricHints $metricNames($metriclcv)] 1]
      set metricMaxValues($metriclcv) [lindex [getMetricHints $metricNames($metriclcv)] 2]
   }

   set numResources [Dg numresources]
   for {set resourcelcv 0} {$resourcelcv < $numResources} {incr resourcelcv} {
      set resourceNames($resourcelcv) [Dg resourcename $resourcelcv]
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

   # launch our C++ barchart code
   # launchBarChart $W.body.barCanvas doublebuffer noflicker $numMetrics $numResources 0
   launchBarChart $W.body doublebuffer noflicker $numMetrics $numResources 0

   # trap window resize and window expose events --- for the subwindow
   # containing the bars only, however.  Note that using
   # $W.body.barCanvas instead of "." avoids LOTS of unneeded
   # configuration callbacks.  In particular, when a window is just
   # moved, it won't create a callback (yea!)  This means we
   # can treat a configuration event as a true resize.

   # [sec 19.2: 'event patterns' in tk/tcl manual]

   bind $W.body <Configure> +{myConfigureEventHandler}
   bind $W.body <Expose>    +{myExposeEventHandler}
}

# ############################################################################
# ####################### important procedures ###############################
# ############################################################################
#
# LOW-LEVEL PROCEDURES:
# processEnterResource - given a widget name, let the user know that clicking
#                        on it will enable the "delete" item under the resource
#                        menu. Called when the mouse enters the widget.
# processClickResource - Bound to buttonpress in resource widgets; just
#                        calls selectResource
# processExitResource - Bound to when the mouse leaves a resource message widget.
#                       depending on whether the user clicked, we do something
#                       appropriate user-interface-speaking.
#
# drawXaxis - Completely rethinks the layout of the x axis and its scrollbar, and
#             redraws them.  Call at the beginning of the program, after the window
#             is resized, and when resources (or metrics) are added or deleted.
#
# drawYaxis - analagous to drawXaxis...
#
# drawTitle - rethinks and redraws the chart title (just below the menubar).
#             Call at the beginning of the program and when metrics are
#             added or deleted (since they comprise the title)
#
# myConfigureEventHandler - called whenever the window configuration has changed
#
# HIGH-LEVEL PROCEDURES:
# selectResource - given a widget name, select it and enable the "delete resource"
#                  menu item.
# unSelectResource - un-select the widget and disable the "delete resource" menu item.
# addResource - given a new resource name, adds it (calling drawXaxis, etc.
#               automatically).
#
# delResource - given an index (0 thru numResources-1), deletes it (calling
#               drawXaxis, etc. automatically)
# 
# addMetric - given a new metric name (and its units), adds it (calling drawXaxis,
#             drawYaxis, drawTitle, etc. automatically)
#
# delMetric - analagous to delResource...
#
# ############################################################################

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
   $Wmbar.resources.m entryconfigure 1 -state normal \
           -label "Remove \"$clickedOnResourceText\"" \
	   -command {delResourceByName $clickedOnResourceText}
}

proc unSelectResource {widgetName} {
   global clickedOnResource
   global clickedOnResourceText
   global Wmbar

   set clickedOnResource ""
   set clickedOnResourceText ""
   $widgetName configure -relief flat
   $Wmbar.resources.m entryconfigure 1 -state disabled \
	   -label "Remove Selected Resource" \
           -command {puts "ignoring unexpected deletion..."}
}

proc processEnterResource {widgetName} {
   global clickedOnResource
   global clickedOnResourceText

   # if this widget has already been clicked on, do nothing
   if {$widgetName == $clickedOnResource} {
      return
   }

   $widgetName configure -relief groove
}

proc processClickResource {widgetName} {
   selectResource $widgetName
}

proc processExitResource {widgetName} {
   global clickedOnResource
   global clickedOnResourceText

   # if we clicked on this guy, then do nothing (keep him selected),
   # otherwise undo the -relief groove
   if {$clickedOnResource != $widgetName} {
      $widgetName configure -relief flat
   }
}

proc rethinkResourceWidths {} {
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
   global Wxcanvas

   set screenWidth [getWindowWidth $Wxcanvas]
   # puts stderr "Welcome to rethinkResourceWidths; screenwidth=$screenWidth"
   
   set tentativeResourceWidth [expr $screenWidth / $numResources]
   if {$tentativeResourceWidth < $minResourceWidth} {
      set tentativeResourceWidth $minResourceWidth
   } elseif {$tentativeResourceWidth > $maxResourceWidth} {
      set tentativeResourceWidth $maxResourceWidth
   }

   set currResourceWidth $tentativeResourceWidth

   # puts stderr "Leaving rethinkResourceWidths; we have decided upon $currResourceWidth"
}

proc drawXaxis {} {
   # how it works: deletes all canvas items with the tag "xAxisItemTag", including
   # the window items.  message widgets have to be deleted separately, notwithstanding
   # the fact that the canvas window items were deleted already.
   # (it knows how many message widgets there are via numResourcesDrawn, which at the
   # time this routine is called, may not be up-to-date with respect to numResources),
   # and then redraws by re-recreating the canvas items and message widgets

   global W
   global Wmbar
   global Wxcanvas

   global xAxisHeight
   global yAxisWidth

   global numResources
   global numResourcesDrawn
   global resourceNames
   global clickedOnResource
   global clickedOnResourceText

   global minResourceWidth
   global maxResourceWidth
   global currResourceWidth

   global LongNames
   global SortPrefs

   # puts stderr "Welcome to drawXaxis"
   # flush stderr

   set top 3
   set tickHeight 5
   set resourceNameFont -adobe-courier-medium-r-normal-*-12-120-*-*-*-*-iso8859-1

   # ###### delete leftover stuff
   $Wxcanvas delete xAxisItemTag
   for {set rindex 0} {$rindex < $numResourcesDrawn} {incr rindex} {
      destroy $Wxcanvas.message$rindex
   }

   # next, several tick marks extending down a few pixels from the x axis (one per resource),
   # plus (while we're at it) the text of the given resources (centered at their respective
   # tick marks)

   # yet to implement: STAGGERED TEXT
   #         needed  : a way to detect when two message widgets have collided.
   #                   doesn't sound too difficult... (use winfo -geometry for each widget)

   set right 0
   for {set rindex 0} {$rindex < $numResources} {incr rindex} {
      set left [expr $rindex * $currResourceWidth]
      set right [expr $left + $currResourceWidth - 1]
      set middle [expr ($left + $right) / 2]

      # create a tick line
      $Wxcanvas create line $middle $top $middle [expr $top+$tickHeight-1] -tag xAxisItemTag

      # create a message widget, bind some commands to it, and attach it to the
      # canvas via "create window"

      set theText $resourceNames($rindex)
      if {$LongNames == 0} {
         set theText [file tail $theText]
      }

      message $Wxcanvas.message$rindex -text $theText \
                                           -justify center -width $currResourceWidth \
					   -font $resourceNameFont
      bind $Wxcanvas.message$rindex <Enter> \
                          {processEnterResource %W}
      bind $Wxcanvas.message$rindex <Leave> \
                          {processExitResource %W}
      bind $Wxcanvas.message$rindex <ButtonPress> \
                          {processClickResource %W}
      $Wxcanvas create window $middle [expr $top+$tickHeight] \
                                         -anchor n -tag xAxisItemTag \
					 -window $Wxcanvas.message$rindex
   }

   # the axis itself--a horizontal line
   $Wxcanvas create line 0 $top $right $top -tag xAxisItemTag

   # Update the scrollbar's scrollregion configuration:
   set regionList {0 0 0 0}
   set regionList [lreplace $regionList 2 2 $right]
   set regionList [lreplace $regionList 3 3 $xAxisHeight]
   $Wxcanvas configure -scrollregion $regionList

   set screenWidth [getWindowWidth $Wxcanvas]   

   set oldconfig [$W.sbRegion.xAxisScrollbar get]
   set oldTotalWidth [lindex $oldconfig 0]

   if {$oldTotalWidth != $right} {
      # puts stderr "drawXaxis: detected major change in resources ($oldTotalWidth != $right), resetting"
      set firstUnit 0
   } else {
      # no change
      set firstUnit [lindex $oldconfig 2]
   }

   set lastUnit [expr $firstUnit + $screenWidth - 1]
   # puts stderr "setting sb: $right $screenWidth $firstUnit $lastUnit"   
   $W.sbRegion.xAxisScrollbar set $right $screenWidth $firstUnit $lastUnit
                                  
   # set the maximum width of the window to be $right + $yAxisWidth
   # wm maxsize . [expr $right + inches2pixels($yAxisWidth)] [unlimited-y-size]

   set numResourcesDrawn $numResources
}

proc processNewMetricMax {mindex newmaxval} {
   # called from barChart.C when y-axis overflow is detected and
   # a new max value is chosen
   global metricMinValues
   global metricMaxValues

   set metricMaxValues($mindex) $newmaxval

   drawYaxis
}

proc drawYaxis {} {
   # the y axis changes to reflect the units of the current metric(s).
   # It is not necessary to call drawYaxis if the window width is
   # resized; it IS necessary to call drawYaxis if the window
   # height is changed.
   # It is not necessary to call drawYaxis if resources are
   # added or removed; it IS necessary to call drawYaxis if
   # metrics are added or removed.

   global W
   global numMetrics
   global numLabelsDrawn
   global metricNames
   global metricUnits
   global metricUnitTypes
   global metricMinValues
   global metricMaxValues

   # puts stderr "welcome to drawYaxis"

   # first, delete all leftover canvas items (those with a yaxis tag to them)
   $W.yAxisCanvas delete yAxisTag

   # we still have to delete the label widgets, which can't have tags...
   set numlabels 3
   for {set labelindex 0} {$labelindex < $numLabelsDrawn} {incr labelindex} {
      destroy $W.yAxisCanvas.label$labelindex
   }

   # canvas item: the axis itself (a vertical line)
   set winHeight [winfo height $W.yAxisCanvas]
   if {$winHeight == 1} {
      set winHeight [winfo reqheight $W.yAxisCanvas]
   }
   set winWidth [getWindowWidth $W.yAxisCanvas]

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

   set numericalStep [expr ($metricMaxValues(0)-$metricMinValues(0)) / ($numlabels-1)]

   puts stderr "drawYaxis: numericalStep is $numericalStep; metric-min is $metricMinValues(0); metric-max is $metricMaxValues(0)"
   flush stderr

   $W.yAxisCanvas create line $right $top $right $bottom -tag yAxisTag

   set labelFont -adobe-courier-medium-r-normal-*-12-120-*-*-*-*-iso8859-1
   # draw tick marks, and while we're at it, their associated message widgets
   for {set labelindex 0} {$labelindex < $numlabels} {incr labelindex} {
      set currY [expr $bottom - round($labelindex*$tickStep)]

      $W.yAxisCanvas create line $tickLeft $currY $right $currY -tag yAxisTag

      set labelText [expr $metricMinValues(0) + $labelindex*$numericalStep]
      message $W.yAxisCanvas.label$labelindex -text $labelText \
              -justify right -width $labelWidth \
	      -font $labelFont

      $W.yAxisCanvas create window $labelRight $currY -anchor e \
                        -tag yAxisItemTag \
			-window $W.yAxisCanvas.label$labelindex
   }

   set numLabelsDrawn $numlabels
}

proc drawTitle {} {
   # the title changes to reflect the current metric(s)
   # currently, only a single metric is supported

   global W
   global numMetrics
   global metricNames
   global metricUnits

   if {$numMetrics == 0} {
      set newTitle "(no metrics currently defined)"
   } else {
      set newTitle "Metric: $metricNames(0) (Units: $metricUnits(0))"
   }

   $W.titleLabel configure -text $newTitle
}

proc myConfigureEventHandler {} {
   # not yet implemented: intelligence to detect merely a window-move
   # (nothing to do) verses a true resize (lots to do).

   # BUG: Should implement delayed action (don't do until idle).  It appears
   # necessary in some cases; this is such a case.  Why?  Apparantly, this
   # routine is called BEFORE the change in configuration, ensuring that we
   # process on the OLD (ack!) window values instead of the new ones!

   # puts stderr "barChart.tcl -- welcome to myConfigureEventHandler"
   # flush stderr

   # The only reason we really need to call drawXaxis is to rethink the
   # scrollbar.  the rest of the calculations that will be done are unnecessary
   # here:
   drawXaxis

   # We only need to redraw the y axis if the window height has changed
   # (and at the beginning of the program)
   drawYaxis

   # Redraw the title (only needed if metrics changed)
   drawTitle
   
   # if the x axis has changed then call this: (barChart.C)
   xAxisHasChanged

   # if the y axis has changed then call this: (barChart.C)
   yAxisHasChanged

   # rethink how wide the resources should be
   rethinkResourceWidths

   # inform our C++ code (barChart.C) that a resize has taken place
   resizeCallback
}

proc myExposeEventHandler {} {
   # puts stderr "barChart.tcl -- welcome to myExposeEventHandler"
   # flush stderr

   # all tk widgets redraw automatically (though not 'till the next idle)

   # all that's left to do is inform our C++ code of the expose
   exposeCallback
}

proc addResource {rName} {
   global numResources
   global resourceNames

   # first, make sure this resource doesn't already exist
   for {set rindex 0} {$rindex < $numResources} {incr rindex} {
      if {$resourceNames($rindex) == $rName} {
         puts stderr "detected a duplicate resource: $rname (ignoring addition request)"
         return
      } 
   }

   set resourceNames($numResources) $rName
   incr numResources

   drawXaxis
}

proc delResource {delindex} {
   global numMetrics
   global numResources
   global resourceNames
   global clickedOnResource
   global clickedOnResourceText
   global Wmbar

   # first, make sure this resource index is valid
   if {$delindex < 0 || $delindex >= $numResources} {
      puts stderr "delResource -- ignoring out of bounds index: $delindex"
      return
   }

   if {$clickedOnResourceText == $resourceNames($delindex)} {
      set clickedOnResource ""
      $Wmbar.resources.m entryconfigure 1 -state disabled \
              -label "Remove Selected Resource" \
              -command {puts stderr "ignoring unexpected deletion..."}
   } else {
      puts stderr "delResource -- no mbar changes since $clickedOnResourceText != $resourceNames($delindex)"
   }

   # inform that visi lib that we don't want to receive anything
   # more about this resource
   # NOTE: unfortunately, [Dg numResources] etc. will not be
   #       lowered, even after this is done!  (******A bug********)
   #       The temporary solution is to rigidly test the
   #       Valid bit of each metric-resource pair before
   #       drawing, etc.
   for {set mindex 0} {$mindex < $numMetrics} {incr mindex} {
      Dg stop $mindex $delindex
   }
   
   # shift resourceNames
   for {set rindex $delindex} {$rindex < [expr $numResources - 1]} {incr rindex} {
      set resourceNames($rindex) $resourceNames([expr $rindex + 1])
   }

   set numResources [Dg numresources]
      # as mentioned above, the current visi-lib won't
      # lower the value by 1...

   # callback to barChart.C
   xAxisHasChanged

   drawXaxis
}

proc delResourceByName {rName} {
   global numResources
   global resourceNames

   # find the appropriate index and call delResource...
   for {set rindex 0} {$rindex < $numResources} {incr rindex} {
      if {$rName == $resourceNames($rindex)} {
         delResource $rindex
         return
      }
   }

   puts stderr "delResourceByName: ignoring request to delete resource named: $rName (no such resource)"
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

   puts stderr "NOTICE -- getMetricHints: unexpected metric: $theMetric...continuing"
   return {real 0.0 1.0 0.1}
   # #pragma HACK done
}

proc addMetric {theName theUnits} {
   global numMetrics
   global metricNames
   global metricUnits
   global metricUnitTypes
   global metricMinValues
   global metricMaxValues

   puts stderr "Welcome to addMetric; name is $theName; units are $theUnits"

   # first make sure that this metric isn't already present (if so, do nothing)
   for {set metricIndex 0} {$metricIndex < $numMetrics} {incr metricIndex} {
      if {$metricNames($metricIndex) == $theName} {
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

   drawXaxis
   drawYaxis
   drawTitle
}

proc delMetric {delIndex} {
   global numMetrics
   global metricNames
   global metricUnits

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
   drawYaxis
   drawXaxis

   # don't we need to tell paradyn to stop sending us data on
   # this metric?
}

proc processNewScrollPosition {newTop} {
   # the -command configuration of the scrollbar is fixed to call this procedure.
   # This happens whenever scrolling takes place.  We update the x-axis canvas
   global Wxcanvas
   global W

   # puts stderr "barChart.tcl: welcome to processNewScrollPosition: newTop is now: $newTop"

   if {$newTop < 0} {
      set newTop 0
   }

   # if max <= visible then set newTop 0
   set currSettings [$W.sbRegion.xAxisScrollbar get]
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
   $Wxcanvas xview $newTop

   # call our C++ code to update the bars
   newScrollPosition $newTop
}

proc myXScroll {totalSize visibleSize left right} {
   # the -scrollcommand configuration of the x axis canvas.
   # gets called whenever the canvas view changes or is resized.
   # The idea is to give us the opportunity to rethink the scrollbar that
   # we are associated with.  Four parameters are passed: total size,
   # window size, left, right---arguments which we simply pass to the "set"
   # command of the scrollbar

   global W

   $W.sbRegion.xAxisScrollbar set $totalSize $visibleSize $left $right
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
   global metricUnits
   global metricMinValues
   global metricMaxValues

   global numResources
   global numResourcesDrawn
   global resourceNames

   set numMetrics [Dg nummetrics]
   for {set metriclcv 0} {$metriclcv < $numMetrics} {incr metriclcv} {
      set metricNames($metriclcv) [Dg metricname  $metriclcv]
      set metricUnits($metriclcv) [Dg metricunits $metriclcv]

      # note -- the following 2 lines are very dubious for already-existing
      #         resources (i.e. we should try to stick with the initial values)
      set metricMinValues($metriclcv) [lindex [getMetricHints $metricNames($metriclcv)] 1]
      set metricMaxValues($metriclcv) [lindex [getMetricHints $metricNames($metriclcv)] 2]
   }

   set numResources [Dg numresources]
   for {set resourcelcv 0} {$resourcelcv < $numResources} {incr resourcelcv} {
      set resourceNames($resourcelcv) [file tail [Dg resourcename $resourcelcv]]
   }

   # rethink the layout of the axes
   rethinkResourceWidths
   drawXaxis
   drawYaxis
   drawTitle

   # inform our C++ code that stuff has changed
   xAxisHasChanged
   yAxisHasChanged
}

# #################  AddMetricDialog -- Ask paradyn for another metric #################

proc AddMetricDialog {} {
   Dg start "*" "*"
}

# #################  AddResourceDialog -- Ask paradyn for another resource #################

proc AddResourceDialog {} {
   Dg start "*" "*"
}

proc LongNamesChange {} {
   global LongNames

   # rethink the text within the x-axis labels:
   drawXaxis
}

proc ProcessNewSortPrefs {} {
   global SortPrefs

   # redraw the x axis
   drawXaxis

   # redraw the bars (callback to our C++ code)
   xAxisHasChanged
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
   drawYaxis
}

# ######################################################################################
#                           "Main Program"
# ######################################################################################

Initialize
