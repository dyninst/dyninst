#
#  barChart -- A bar chart display visualization for Paradyn
#
#  $Log: barChart.tcl,v $
#  Revision 1.25  1995/12/15 22:34:30  tamches
#  fixed bug where Wmbar wasn't being made a global vrble
#
#  Revision 1.24  1995/11/29 00:37:56  tamches
#  grouped window initialization into routine init_barchart_window.
#  We now use the makeLogo cmd
#  We no longer call Initialize in this file
#
#  Revision 1.23  1995/11/17 17:32:20  newhall
#  changed Dg start command to take no arguments, replaced call to MetricUnits
#  with call to MetricLabel
#
# Revision 1.22  1995/11/08  02:15:31  tamches
# Changed logo and title bar background from slategray to indianred
#
# Revision 1.21  1995/08/24  15:15:08  hollings
# AIX/SP-2 port (including option for split instruction/data heaps)
# Tracing of rexec (correctly spawns a paradynd if needed)
# Added rtinst function to read getrusage stats (can now be used in metrics)
# Critical Path
# Improved Error reporting in MDL sematic checks
# Fixed MDL Function call statement
# Fixed bugs in TK usage (strings passed where UID expected)
#
# Revision 1.20  1995/08/06  22:10:17  tamches
# Procedure calls metric default changed to 100
#
# Revision 1.19  1995/07/06  18:54:59  tamches
# Update for tk4.0
#
# Revision 1.18  1995/05/10  22:28:24  tamches
# Removed warning message in getMetricHints when encountering an
# unknown metric...but we still default to min=0 max=1.  This information
# should really be kept in the data manger, imho.
#
# Revision 1.17  1995/04/01  01:34:21  tamches
# Metric axis lines now 2 pix wide, not 1.  Metric key items now more
# color-coordinated (i.e., now, not just the axis lines are drawn in the
# bar's color).
#
# Revision 1.16  1995/02/26  02:01:45  newhall
# added callback functions for new visiLib phase info.
#
# Revision 1.15  1994/11/11  06:46:34  tamches
# more configure event handlers for other subwindows has helped fix
# some resize bugs, especially when switching from short to long
# names.  Window now does not resize itself when adding new resources
# with longer names; pack propagate for the toplevel is off.
#
# Revision 1.14  1994/11/09  04:44:40  tamches
# Deleting multiple metrics at a time is now supported.
#
# Revision 1.13  1994/11/09  03:26:25  tamches
# Clicking in a "neutral" area of the resources axis will now un-select
# anything that may have been selected.
#
# Revision 1.12  1994/11/09  02:25:19  tamches
# Re-implemented a feature of old: Long Names.  The option
# (which is off by default) is found at the bottom of the options menu.
# Fixed a bug whereby resourcesAxisWidth stayed unchanged at "1.4i"
# forever.
#
# Revision 1.11  1994/11/06  10:36:48  tamches
# changed title font to 14 point
# beefed up validResources(), numValidResources, indirectResources()
# throughput the code.
# implemented a maximum individual color height of 25 pixels
# fixed a major bug when deleted resources (deleted resources were still
# being counted when calculating resource heights)
# Fixed deletion bug by having myXScroll do the callback to C++ code
# that updates bar offsets.
#
# Revision 1.10  1994/10/28  21:53:44  tamches
# Fixed a rather flaming bug that could cause any resource add to
# potentially crash after doing a sort (c++ code's numResources
# wasn't updated before ::rethinkIndirectResources was called, leading
# to an assertion check failure)
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
# 2) multiple metrics: allow deletion
# 3) too much flickering on resize
# 4) No room for scrollbar unless needed
# ######################################################

#  ################### Default options #################

proc init_barchart_window {} {
#option add *Visi*font *-New*Century*Schoolbook-Bold-R-*-14-*
option add *Data*font *-Helvetica-*-r-*-12-*
option add *MyMenu*font *-New*Century*Schoolbook-Bold-R-*-14-*

   if {[winfo depth .] > 1} {
      # You have a color monitor...
   #   . config -bg grey
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

   global W   
   set W .bargrph
#   frame $W -class Visi
   frame $W
   
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
   
   #label $W.top.logo -relief raised \
   #		  -bitmap @/p/paradyn/core/paradyn/tcl/logo.xbm \
   #                  -foreground indianred
   makeLogo $W.top.logo paradynLogo raised 2 indianred
   
   pack $W.top.logo -side right -expand false
      # we set expand to false; if the window is made wider, we
      # don't want any of the extra space; let the menu bar and
      # title bar have it.
   
   # #################### Title bar #################
   
   label $W.top.left.titlebar  -text "Barchart Visualization" -foreground white -background indianred -relief raised
   pack $W.top.left.titlebar -side top -fill both -expand true
      # expand is set to true, not because we want more space if the window
      # is made taller (which won't happen, since the expand flag of our
      # parent was set to false), but because we want to take up any padding
      # space left after we and the menu bar are placed (if the logo is
      # taller than the two of us, which it currently is)
   
   # ##################### Menu bar ###################

   global Wmbar   
   set Wmbar $W.top.left.mbar
   frame $Wmbar -class MyMenu -borderwidth 2 -relief raised
   pack  $Wmbar -side top -fill both -expand false
   
   # #################### File menu #################
   
   menubutton $Wmbar.file -text File -menu $Wmbar.file.m
   menu $Wmbar.file.m -selectcolor tomato
   $Wmbar.file.m add command -label "Close Bar chart" -command GracefulClose
   
   # #################### Actions Menu ###################
   
   menubutton $Wmbar.actions -text Actions -menu $Wmbar.actions.m
   menu $Wmbar.actions.m -selectcolor tomato
   $Wmbar.actions.m add command -label "Add Bars..." -command AddMetricDialog
   $Wmbar.actions.m add separator
   $Wmbar.actions.m add command -label "Remove Selected Metric(s)" -state disabled
   $Wmbar.actions.m add command -label "Remove Selected Resource(s)" -state disabled
   
   # #################### View menu ###################
   
   menubutton $Wmbar.view -text View -menu $Wmbar.view.m
   menu $Wmbar.view.m -selectcolor tomato
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
   
   $Wmbar.view.m add separator

   global LongNames
   set LongNames 0
   
   $Wmbar.view.m add checkbutton -label "Long Names" -variable LongNames \
   	-command ProcessLongNamesChange
   
   # #################### Help menu #################
   
   #menubutton $Wmbar.help -text Help \
   #          -menu $Wmbar.help.m
   #menu $Wmbar.help.m 
   #$Wmbar.help.m add command -label "General" -command "NotImpl" -state disabled
   #$Wmbar.help.m add command -label "Context" -command "NotImpl" -state disabled
   
   
   # #################### Build the menu bar and add to display #################
   
   pack $Wmbar.file $Wmbar.actions $Wmbar.view -side left -padx 4
   #pack $Wmbar.help -side right
   
   # #######################  Scrollbar ######################
   
   canvas $W.farLeft
   pack $W.farLeft -side left -expand false -fill y
      # expand is set to false; if the window is made wider, don't change width
   
   scrollbar $W.farLeft.resourcesAxisScrollbar -orient vertical -width 16 \
           -background gray -activebackground gray -relief sunken \
           -command ".bargrph.left.resourcesAxisCanvas yview"
   
   pack $W.farLeft.resourcesAxisScrollbar -side top -fill y -expand true
      # expand is set to true; if the window is made taller, we want
      # extra height.
   
   canvas $W.farLeft.sbPadding -height $metricsAxisHeight -width 16 -relief flat
   pack $W.farLeft.sbPadding -side bottom -expand false -fill x
      # expand is set to false; if the window is made taller, we don't
      # want any of the extra height.
   
   # #####################  Resources Axis #################
   
   canvas $W.left -width $resourcesAxisWidth
   pack   $W.left -side left -expand false -fill y
      # expand is set to false; if the window is made wider, we don't want
      # any of the extra width
   
   canvas $W.left.metricsKey -height $metricsAxisHeight -width $resourcesAxisWidth\
           -relief groove
   pack   $W.left.metricsKey -side bottom -expand false
      # expand is set to false; if the window is made taller, we don't
      # want any of the extra height

   global WresourcesCanvas   
   set WresourcesCanvas $W.left.resourcesAxisCanvas
   canvas $WresourcesCanvas -width $resourcesAxisWidth -relief groove \
                                -yscrollcommand myYScroll \
                                -yscrollincrement 1
   pack   $WresourcesCanvas -side top -expand true -fill y
      # expand is set to true; if the window is made taller, we want the
      # extra height.
   
   # ####################  Metrics Axis Canvas ############################
   
   canvas $W.metricsAxisCanvas -height $metricsAxisHeight -relief groove
   pack   $W.metricsAxisCanvas -side bottom -fill x -expand false
      # expand is set to false; if the window is made wider, we don't want
      # extra width to go to the metrics axis
   
   # ####################  Barchart Area ($W.body) #################
   
   canvas $W.body -height 2.5i -width 3.5i -relief groove
   pack  $W.body -side top -fill both -expand true
      # expand is set to true; if the window is made taller, we want the
      # extra height to go to us
   
   # ######### pack $W (and all its subwindows) into the main (top-level)
   # ######### window such that it basically consumes the entire window...
   pack append . $W {fill expand frame center}

   # set some window manager hints:
   #wm minsize  . 350 250
   wm title    . "Barchart"
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

# isMetricValid -- true iff at least one metric/focus pair for this metric
#                  is a enabled (not deleted).  Pass a true (not sorted)
#                  metric index.
# Given: updated numResources
# Does:  returns number of enabled (non-deleted?) metrics
proc isMetricValid {mindex} {
   global numResources

   for {set resourcelcv 0} {$resourcelcv<$numResources} {incr resourcelcv} {
      if {[Dg enabled $mindex $resourcelcv]} {
         return 1
      }
   }

   # false
   return 0
}

# isResourceValid -- true iff at least one metric/focus pair for this
#                    resource is enabled.  Pass a true resource index, not
#                    a sorted one
# Given: updated numMetrics
# Does:  returns number of enabled (non-deleted?) resources
proc isResourceValid {rindex} {
   global numMetrics

   for {set metriclcv 0} {$metriclcv<$numMetrics} {incr metriclcv} {
      if {[Dg enabled $metriclcv $rindex]} {
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

   global numMetrics numMetricsDrawn
   global numMetricLabelsDrawn
   global metricNames
   global validMetrics

   global metricUnits
   global metricMinValues metricMaxValues

   global metricsLabelFont resourceNameFont
   global prevLeftSectionWidth

   global numResources
   global numValidResources validResources
   global indirectResources
   global resourceNames

   global currResourceHeight
   global minResourceHeight maxResourceHeight maxIndividualColorHeight minIndividualColorHeight

   global DataFormat

   global numLabelsDrawn numResourcesDrawn

   global SortPrefs
   global barColors numBarColors

   set SortPrefs NoParticular
   
   set numLabelsDrawn 0
   set numResourcesDrawn 0

   set DataFormat Instantaneous

   # keep both of the following lines up here:
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

   set numValidResources 0
   for {set resourcelcv 0} {$resourcelcv < $numResources} {incr resourcelcv} {
      set validResources($resourcelcv) [isResourceValid $resourcelcv]
      if {$validResources($resourcelcv)} {
         set indirectResources($numValidResources) $resourcelcv
         incr numValidResources
      }

      set resourceNames($resourcelcv) [Dg resourcename $resourcelcv]
   }

   set minResourceHeight 20
   set maxResourceHeight 50
   set maxIndividualColorHeight 25
   set minIndividualColorHeight 0
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

  set resourceNameFont *-Helvetica-*-r-*-12-*
  set metricsLabelFont *-Helvetica-*-r-*-12-*
#   set resourceNameFont "7x13bold"
#   set metricsLabelFont "7x13bold"


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

   bind $W.body <Configure> {bodyConfigureEventHandler %w %h}
   bind $W.left.resourcesAxisCanvas <Configure> {resourcesAxisConfigureEventHandler %w %h}
   bind $W.metricsAxisCanvas <Configure> {metricsAxisConfigureEventHandler %w %h}
   bind $W.left.metricsKey <Configure> {metricsKeyConfigureEventHandler %w %h}
   bind $W.body <Expose>    {myExposeEventHandler}
}

# selectResource -- assuming this resource was clicked on, select it
proc selectResource {widgetName} {
   global Wmbar

   set theRelief [lindex [$widgetName configure -relief] 4]
   if {$theRelief!="groove"} {
      # Hmmm.. this guy was already selected.  Let's unselect him! (not implemented since
      # we would have to possibly update the menu too and there's no easy way to do that
      # without checking whether there exist any still-selected resources)

      #$widgetName configure -relief flat
      return
   }

   $widgetName configure -relief sunken

   # update delete resource menu item
   $Wmbar.actions.m entryconfigure 4 -state normal \
           -command {delSelectedResources}
}

# processEnterResource -- routine to handle entry of mouse in a resource name
proc processEnterResource {widgetName} {
   # if this widget has already been clicked on, do nothing (leave it sunken)
   set theRelief [lindex [$widgetName configure -relief] 4]
   if {$theRelief=="sunken"} return

   $widgetName configure -relief groove
}

# processExitResource -- routine to handle mouse leaving resource name area
#                        we may or may not have done a mouse-click in the meantime
proc processExitResource {widgetName} {
   # If we had clicked on this guy, then do nothing (keep selected), else undo the -relief groove
   set theRelief [lindex [$widgetName configure -relief] 4]
   if {$theRelief=="groove"} {
      $widgetName configure -relief flat
   }
}

proc clickNeutralResourceArea {} {
   global Wmbar WresourcesCanvas
   global numResources numResourcesDrawn resourceNames
   global numValidResources indirectResources

   # unselect whatever was selected
   for {set resourcelcv 0} {$resourcelcv < $numResourcesDrawn} {incr resourcelcv} {
      set widgetName $WresourcesCanvas.message$resourcelcv

      $widgetName configure -relief flat
   }

   # update delete resource menu item
   $Wmbar.actions.m entryconfigure 4 -state disabled \
           -command {puts stderr "ignoring unexpected deletion"}
}

proc rethinkResourceHeights {screenHeight} {
   # When resources are added or deleted, or a resize occurs,
   # this routine is called.  Its sole purpose is to rethink the value
   # of currResourceHeight, depending on the resources and window height.

   # algorithm: current window height is passed as a parameter.  Set
   # resource height equal to window height / num **valid** resources (don't
   # want to include deleted ones!).  If that would make the resource
   # height too small, make it minResourceHeight.
   global minResourceHeight maxResourceHeight
   global currResourceHeight
   global numResources numValidResources
   global validResources
   global WresourcesCanvas

   if {$numValidResources==0} {
      set tentativeResourceHeight 0
   } else {
      set tentativeResourceHeight [expr $screenHeight / $numValidResources]
      if {$tentativeResourceHeight < $minResourceHeight} {
         set tentativeResourceHeight $minResourceHeight
      } elseif {$tentativeResourceHeight > $maxResourceHeight} {
         set tentativeResourceHeight $maxResourceHeight
      }
   }

   set currResourceHeight $tentativeResourceHeight

#   puts stderr "Leaving rethinkResourceHeights(tcl); we have decided upon $currResourceHeight (max is $maxResourceHeight)"
}

# Upon changes to the resources axis or the metrics key, call this routine to
# rethink how wide the left portion of the screen (which is what holds these
# guys) should be.
proc rethinkLeftSectionWidth {} {
   global W
   global WresourcesCanvas
   global prevLeftSectionWidth
   global numResourcesDrawn numMetricsDrawn

   set maxWidthSoFar 20
   set tickWidth 5

   # loop through the resources on screen in sorted order
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
#      pack propagate . false
         set newWidth [expr 2 + $maxWidthSoFar + $tickWidth + 2]
         $WresourcesCanvas configure -width $newWidth -relief groove
         pack $WresourcesCanvas -side top -expand true -fill y
            # expand is set to true; if the window is made taller, we want the
            # extra height.
         $W.left.metricsKey configure -width $newWidth
#      pack propagate . true
   }

   set prevLeftSectionWidth $maxWidthSoFar
}

# how it works: deletes canvas items with the tag "resourcesAxisItemTag",
# including window items.  message widgets have to be deleted separately,
# notwithstanding that canvas window items were deleted already.
# (it knows how many message widgets there are via numResourcesDrawn, which
# at the time this routine is called, may not be up-to-date with respect to
# numResources), and then redraws by re-recreating the canvas items and
# message widgets
proc drawResourcesAxis {windowHeight} {
   global W
   global Wmbar
   global WresourcesCanvas

   global resourceNameFont
   global resourcesAxisWidth
   global metricsAxisHeight

   global numResources numValidResources
   global validResources
   global numResourcesDrawn
   global resourceNames
   global indirectResources

   global minResourceHeight maxResourceHeight currResourceHeight

   global SortPrefs

   set resourcesAxisWidth [getWindowWidth $WresourcesCanvas]
#   puts stderr "Welcome to drawResourcesAxis; width=$resourcesAxisWidth"

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
   for {set rindex 0} {$rindex < $numValidResources} {incr rindex} {
      set actualResource $indirectResources($rindex)
      if {!$validResources($actualResource)} {
         puts stderr "drawResourcesAxis -- detected an invalid resource"
         return
      }

      set bottom [expr $top + $currResourceHeight - 1]
      set middle [expr ($top + $bottom) / 2]
   
      # create a tick line for this resource
      $WresourcesCanvas create line [expr $right-$tickWidth] $middle $right \
	      $middle -tag resourcesAxisItemTag
   
      # create a message widget, bind some commands to it, and attach it to
      # the canvas via "create window"

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
   }

   bind $WresourcesCanvas <ButtonPress> "clickNeutralResourceArea"

   # the axis itself--a horizontal line
   $WresourcesCanvas create line $right 0 $right $top -tag resourcesAxisItemTag

   # rethink width of resources axis and metrics key.
   # May forcibly resize the width of those windows as it sees fit.
   rethinkLeftSectionWidth

   # Update the scrollbar's scrollregion configuration:
   set regionList {0 0 0 0}
   set regionList [lreplace $regionList 2 2 $resourcesAxisWidth]
#   set regionList [lreplace $regionList 3 3 $bottom]
   set regionList [lreplace $regionList 3 3 $top]
   $WresourcesCanvas configure -scrollregion $regionList

   set oldconfig [$W.farLeft.resourcesAxisScrollbar get]
   set oldFirst [lindex $oldconfig 0]
   set oldLast  [lindex $oldconfig 1]

   $W.farLeft.resourcesAxisScrollbar set $oldFirst $oldLast
}

# ProcessNewMetricMax {metricid newMaxVal}
# Called from barChart.C when y-axis overflow is detected
proc processNewMetricMax {mindex newmaxval} {
   global metricMinValues metricMaxValues
   global W

   set metricMaxValues($mindex) $newmaxval

   drawMetricsAxis [getWindowWidth $W.metricsAxisCanvas]
}

# drawMetricsAxis windwidth
# The metrics axis changes to reflect the new width (in pixels).
#
# Call if the window is resized and/or metrics are changed.
#
# Algorithm: delete leftover canvas items
proc drawMetricsAxis {metricsAxisWidth} {
   global W
   global numMetrics numMetricsDrawn numMetricLabelsDrawn

   global metricNames validMetrics

   global metricUnits metricUnitTypes
   global metricMinValues metricMaxValues
   global metricsLabelFont

   set keyWindow $W.left.metricsKey

   $W.metricsAxisCanvas delete metricsAxisTag
   $keyWindow delete metricsAxisTag

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
      if {!$validMetrics($metriclcv)} continue

      set numericalStep [expr (1.0 * $metricMaxValues($metriclcv)-$metricMinValues($metriclcv)) / ($numticks-1)]

      # draw horiz line for this metric; color-coded for the metric
      $W.metricsAxisCanvas create line $fixedLeft $top $fixedRight $top \
                 -tag metricsAxisTag \
                 -fill [getMetricColor $metriclcv] \
		 -width 2

      # draw tick marks and create labels for this metric axis
      for {set ticklcv 0} {$ticklcv < $numticks} {incr ticklcv} {
         set tickx [expr $fixedLeft + ($ticklcv * $tickStepPix)]
         $W.metricsAxisCanvas create line $tickx $top $tickx \
                    [expr $top + $tickHeight] \
                    -tag metricsAxisTag -fill [getMetricColor $metriclcv] \
		    -width 2

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

         # msg widgets instead of labels help us get the justification right
         # (I'm not convinced anymore that we couldn't do this somehow with labels)
         message $W.metricsAxisCanvas.label$labelDrawnCount -text $labelText \
                    -justify $theJust -font $metricsLabelFont \
		    -width [getWindowWidth $W.metricsAxisCanvas]
         $W.metricsAxisCanvas create window $tickx [expr $top+$tickHeight] \
                    -anchor $theAnchor -tag metricsAxisItemTag \
                    -window $W.metricsAxisCanvas.label$labelDrawnCount

         incr labelDrawnCount
      }

      # Draw "key" entry
      $keyWindow create line 5 $top [expr [getWindowWidth $keyWindow] - 5] \
              $top -tag metricsAxisTag -fill [getMetricColor $metriclcv] \
	      -width 2
      set theText $metricNames($metriclcv)
      label $keyWindow.key$numMetricsDrawn -text $theText \
              -font $metricsLabelFont \
	      -foreground [getMetricColor $metriclcv]
      $keyWindow create window [expr [getWindowWidth $keyWindow] - 5] \
              [expr $top + $tickHeight] -tag metricsAxisTag \
              -window $keyWindow.key$numMetricsDrawn -anchor ne

      # prepare for next metric down.  WARNING: "30" is a hack!
      set top [expr $top + $tickHeight + 30]
      incr numMetricsDrawn
   }

   set numMetricLabelsDrawn $labelDrawnCount

   if {$numMetricLabelsDrawn==0} {
      set newMetricsAxisHeight 5
   } else {
      set newMetricsAxisHeight $top
   }

   # This may forcibly resize key and resources axis:
   rethinkLeftSectionWidth

   # Want metrics axis to consume right amount of height.
   $W.metricsAxisCanvas configure -height $newMetricsAxisHeight
   pack $W.metricsAxisCanvas -side bottom -fill x -expand false

   $W.farLeft.sbPadding configure -height $newMetricsAxisHeight
   $W.left.metricsKey   configure -height $newMetricsAxisHeight
}

proc getMetricColor {mindex} {
   global barColors numBarColors

   set theindex [expr $mindex % $numBarColors]
   return $barColors($theindex)
}

proc bodyConfigureEventHandler {newWidth newHeight} {
   # the following routines will clear the bar window (ouch! But no
   # choice since window size change can greatly affect bar layout --- well,
   # sometimes) so resizeCallback has built-in hacks to simulate one
   # new-data callback

   resourcesAxisHasChanged $newHeight

   resizeCallback $newWidth $newHeight

   # the following is only needed once (the first time this routine
   # is executed)
   pack propagate . false
}

proc resourcesAxisConfigureEventHandler {newWidth newHeight} {
   global W
   # rethink how tall the resources should be
   rethinkResourceHeights $newHeight

   # only needed if the height has changed:
   drawResourcesAxis $newHeight

   # inform our C++ code
   resourcesAxisHasChanged $newHeight
}

proc metricsAxisConfigureEventHandler {newWidth newHeight} {
   global W

   drawMetricsAxis $newWidth
   metricsAxisHasChanged $newWidth
}

proc metricsKeyConfigureEventHandler {newWidth newHeight} {
   global W

   drawMetricsAxis [getWindowWidth $W.metricsAxisCanvas]
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

# del1SelectedResources
# Given: a true (not sorted) resource number
# Does: deletes that resource from our internal structures (validResources(),
#       numValidResources), calls [Dg stop] on all its met/res combos.
# Does not: redraw anything; update the resources axis, etc.
proc del1SelectedResource {rindex} {
   global numResources resourceNames numValidResources validResources
   global numMetrics

   if {!$validResources($rindex)} {
      puts stderr "del1SelectedResource: resource #$rindex is invalid (already deleted?)"
      return
   }

   # Inform that visi lib that we don't want anything more from this resource
   for {set mindex 0} {$mindex < $numMetrics} {incr mindex} {
      if {[Dg enabled $mindex $rindex]} {
         Dg stop $mindex $rindex
      }
   }

   # If the [Dg stop...] worked, then this resource is should now be invalid.
   if {[isResourceValid $rindex]} {
      puts stderr "delResource -- valid flag wasn't changed to false after the deletion"
      return
   }

   set validResources($rindex) 0
   set numValidResources [expr $numValidResources - 1]

   if {$numValidResources<0} {
      puts stderr "del1SelectedResource warning: numValidResources now $numValidResources!"
      return
   }

}

# delSelectedResources
# Given: some resources with -configure relief groove
# Does: calls del1SelectedResource on those resources, updates menus,
#       updates sorting order, redraws resources, redraws bars
proc delSelectedResources {} {
   global numValidResources validResources indirectResources
   global numResources resourceNames
   global Wmbar WresourcesCanvas W
   global numResourcesDrawn

   # Loop through all visible resources
   for {set resourcelcv 0} {$resourcelcv < $numResourcesDrawn} {incr resourcelcv} {
      set widgetName $WresourcesCanvas.message$resourcelcv

      # If this widget has -relief sunken, then it has been selected
      set theRelief [lindex [$widgetName configure -relief] 4]
      if {$theRelief!="sunken"} continue

      set actualResource $indirectResources($resourcelcv)
      del1SelectedResource $actualResource
   }

   $Wmbar.actions.m entryconfigure 4 -state disabled \
           -command {puts stderr "ignoring unexpected deletion..."}

   # Rethink sorting order, and inform our C++ code to do the same
   # Does no redrawing whatsoever
   rethinkIndirectResources true

   # rethink height of each resource (does no redrawing whatsoever; does not
   # inform our C++ code of the change)
   rethinkResourceHeights [getWindowHeight $W.body]

   # This may forcibly change the width of the resources axis and metrics key:
   rethinkLeftSectionWidth

   # Redraw resources:
   drawResourcesAxis      [getWindowHeight $W.body]

   # Redraw body:
   bodyConfigureEventHandler [getWindowWidth $W.body] [getWindowHeight $W.body]
}

proc getMetricHints {theMetric} {
   # #pragma HACK begin
   # return metric unit type, hinted min, hinted max, hinted step
   # depending on the metric (a hardcoded temporary hack.  This information
   # should be [but isn't currently] present in the data manager)
   switch $theMetric {
      "exec_time"        {return {percentage 0.0 1.0 0.1}}
      "hybrid_cost"      {return {real 0.0 1.0 0.1}}
      "procedure_calls"  {return {integer 0 100 10}}
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

#   puts stderr "getMetricHints--unexpected metric: $theMetric...continuing"
   return {real 0.0 1.0 0.1}
   # #pragma HACK done
}

#proc addMetric {theName theUnits} {
#   global numMetrics
#   global metricNames
#   global validMetrics
#
#   global metricUnits
#   global metricUnitTypes
#   global metricMinValues
#   global metricMaxValues
#   global W
#
#   puts stderr "Welcome to addMetric; name is $theName; units are $theUnits"
#
#   # first make sure that this metric isn't already present (if so, do nothing)
#   for {set metricIndex 0} {$metricIndex < $numMetrics} {incr metricIndex} {
#      if {$metricNames($metricIndex) == $theName && $validMetrics($metricIndex)} {
#         puts stderr "addMetric: ignoring request to add $theName (already present)"
#         return
#      }
#   }
#
#   # make the addition
#   set metricNames($numMetrics) $theName
#   set metricUnits($numMetrics) $theUnits
#
#   set theHints [getMetricHints $theName]
#
#   set metricUnitTypes($numMetrics) [lindex $theHints 0]
#   set metricMinValues($numMetrics) [lindex $theHints 1]
#   set metricMaxValues($numMetrics) [lindex $theHints 2]
#
#   incr numMetrics
#
#   drawResourcesAxis [getWindowHeight $W.left.resourcesAxisCanvas]
#   drawMetricsAxis [getWindowWidth $W.metricsAxisCanvas]
#}

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

# myYScroll -- the -scrollcommand config of the resources axis canvas.
#          Gets called whenever the canvas view changes or gets resized.
#          This includes every scroll the user makes (yikes!)
#          Gives us an opportunity to rethink the bounds of the scrollbar.
proc myYScroll {first last} {
   global W WresourcesCanvas

   $W.farLeft.resourcesAxisScrollbar set $first $last

   # call our C++ code to update the bar offsets
   set totalCanvasHeight [lindex [$WresourcesCanvas cget -scrollregion] 3]
#   puts stderr "myYscroll: totalCanvasHeight is $totalCanvasHeight"

   set newFirstFrac [lindex [$W.farLeft.resourcesAxisScrollbar get] 0]
#   puts stderr "myYscroll: newFirstFrac is $newFirstFrac"

   set firstPix [expr round($totalCanvasHeight * $newFirstFrac)]
#   puts stderr "myYscroll: so, first pixel is $firstPix" 

   newScrollPosition $firstPix
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
   return

   puts stderr "Welcome to dragAndDropTargetHandler(); DragDrop(text) is $DragDrop(text)"
   addResource $DragDrop(text)
}

# blt_drag&drop target . handler text dragAndDropTargetHandler
#...that cryptic line reads: "declare the window '.' to be a drag n' drop
#   handler for sources of type 'text'; routine dragAndDropTargetHandler
#   gets called (via a "send" from the source...)  Using window '.' means
#   the entire barchart...

# #################### Called by visi library when histos have folded #########

proc DgFoldCallback {} {
#   puts stderr "FOLD detected..."
}

# ########### Called by visi library when metric/resource space changes.
#
# note: this routine is too generic; in the future, we plan to
# implement callbacks that actually tell what was added (as opposed
# to what was already there...)
#
# ######################################################################

proc DgConfigCallback {} {
   # puts stderr "Welcome to DgConfigCallback"
   # flush stderr

   global W

   global numMetrics metricNames
   global validMetrics
   global metricUnits
   global metricMinValues metricMaxValues

   global numResources numValidResources resourceNames
   global validResources
   global numResourcesDrawn
   global indirectResources
   global LongNames

   set numMetrics [Dg nummetrics]
   # the next line must remain up here or else calls to isMetricValid will be wrong!
   set numResources [Dg numresources]

   for {set metriclcv 0} {$metriclcv < $numMetrics} {incr metriclcv} {
      set metricNames($metriclcv) [Dg metricname  $metriclcv]
      set metricUnits($metriclcv) [Dg metricunits $metriclcv]
      set validMetrics($metriclcv) [isMetricValid $metriclcv]

      # note -- the following 2 lines are very dubious for already-existing
      #         resources (i.e. we should try to stick with the initial
      #         values)
      set metricMinValues($metriclcv) [lindex [getMetricHints $metricNames($metriclcv)] 1]
      set metricMaxValues($metriclcv) [lindex [getMetricHints $metricNames($metriclcv)] 2]
   }

   set numValidResources 0
   for {set resourcelcv 0} {$resourcelcv < $numResources} {incr resourcelcv} {
      if {$LongNames==1} {
         set resourceNames($resourcelcv) [Dg resourcename $resourcelcv]
      } else {
         set resourceNames($resourcelcv) [file tail [Dg resourcename $resourcelcv]]
      }

      if {[isResourceValid $resourcelcv]} {
         set validResources($resourcelcv) 1
         incr numValidResources
      } else {
         set validResources($resourcelcv) 0
      }
   }

   # rethink the sorting order (false --> don't do callback to c++ code
   # because it would crash since C++ code doesn't update its value of
   # numMetrics and numResources until a 'resourcesAxisHasChanged' or
   # 'metricsAxisHasChanged'.  When those do indeed get called below, they
   # also update the sorting order, so we're OK.)
   rethinkIndirectResources false

   # rethink the layout of the axes
   rethinkResourceHeights [getWindowHeight $W.left.resourcesAxisCanvas]
   drawResourcesAxis [getWindowHeight $W.left.resourcesAxisCanvas]
   drawMetricsAxis [getWindowWidth $W.metricsAxisCanvas]

   # inform our C++ code that stuff has changed (nummetrics, numresources
   # gets updated, structures are recalculated based on new #metrics,
   # #resources, etc.)
   resourcesAxisHasChanged [getWindowHeight $W.left.resourcesAxisCanvas]
   metricsAxisHasChanged   [getWindowWidth $W.metricsAxisCanvas]
}
# ###########  Callback invoked on a PHASESTART event from paradyn  ########
proc DgPhaseCallback {} {
  return
}

# ###########  AddMetricDialog -- Ask paradyn for another metric ########

proc AddMetricDialog {} {
   Dg start 
}

# #########  AddResourceDialog -- Ask paradyn for another resource #######

proc AddResourceDialog {} {
   Dg start
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

# Given: a change in sorted order and/or deleted/added resources
# Does: rethinks indirectResources(), and (if docallback==true)
#       informs our C++ code of the change in sorting order.
proc rethinkIndirectResources {docallback} {
   # sorting order has changed; rethink indirectResources array
   global SortPrefs

   global numResources numValidResources
   global validResources
   global resourceNames
   global indirectResources

   # sorting works as follows: create a temporary list of {index,name} pairs;
   # sort the list; extract the indexes in sequence; delete temporary list
   set templist {}

   # Note that we exclude invalid resources
   for {set resourcelcv 0} {$resourcelcv < $numResources} {incr resourcelcv} {
      if {$validResources($resourcelcv)} {
         lappend templist [list $resourcelcv $resourceNames($resourcelcv)]
      }
   }

   if {$SortPrefs == "ByName"} {
      set templist [lsort -ascii -increasing -command sortCmd $templist]
   } elseif {$SortPrefs == "ByNameDescending"} {
      set templist [lsort -ascii -decreasing -command sortCmd $templist]
   }

   # puts stderr "rethinkIndirectResources: sorted templist is $templist"

   # Now go through in sorted order:
   for {set resourcelcv 0} {$resourcelcv < $numValidResources} {incr resourcelcv} {
      set actualResource [lindex [lindex $templist $resourcelcv] 0]
      if {$actualResource<0 || $actualResource>=$numResources} {
         puts stderr "rethinkIndirectResources -- actualResource=$actualResource (valid range is (0,$numResources)"
         return
      }
      if {!$validResources($actualResource)} {
         puts stderr "rethinkIndirectResources -- invalid resource detected"
         return
      }

      set indirectResources($resourcelcv) $actualResource
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

   # redraw the metrics axis
   drawMetricsAxis [getWindowWidth $W.metricsAxisCanvas]
}

proc ProcessLongNamesChange {} {
   global LongNames W numResources resourceNames

   # side effect: any selected resources will become un-selected

   # rethink resource names and redraw the resources axis --- that's all that is needed
   for {set resourcelcv 0} {$resourcelcv < $numResources} {incr resourcelcv} {
      if {$LongNames==1} {
         set resourceNames($resourcelcv) [Dg resourcename $resourcelcv]
      } else {
         set resourceNames($resourcelcv) [file tail [Dg resourcename $resourcelcv]]
      }
   }

   drawResourcesAxis [getWindowHeight $W.metricsAxisCanvas]
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
