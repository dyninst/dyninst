#
#  barChart -- A bar chart display visualization for Paradyn
#
#  $Log: barChart.tcl,v $
#  Revision 1.5  1994/09/08 00:10:43  tamches
#  Added preliminary blt_drag&drop interface.
#  changed window title.
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
# 1) When the window resizes, resize the graph; currently, if the window
#    is enlarged, we waste ALL of the extra space!
# 2) fix the x-axis: displays 3 ticks when only 1 resource is up.
# 3) fix the x-axis, part 2: try to detect when named are becoming
#    "scrunched" and apply some optimizations, such as:
#    --multi-lined names (as in a label widget with line wrap),
#    --changing from "normal" to "narrow" font,
#    --changing to a smaller font
#    --making the window wider
#    But how to detect when names are "scrunched"?  blt_bargraph
#    doesn't seem to provide the means...
# 4) Make chart title font bigger.
# 5) add a blt_drag&drop interface, so metric/resource pairs can
#    be dropped into the window, causing them to be automatically
#    added.  This might not involve changes to this file; it might
#    be that Paradyn core is modified for a drag&drop interface,
#    and that the existing callback interface is sufficient.
# 
# PROBLEMS LIST:
# 1) blt_barchart is not tuned to expect rapidly changing
#    values; to wit, on an "element configure" command that
#    changes only -yvalue, GetName and some others are
#    still called, meaning the resource names are being
#    redrawn each time!!!!  (un-comment the 'puts stderr ...'
#    and 'flush stderr' #    lines from throughout the file to
#    see what we mean...)
# 2) multi-lined names (see 3, above) are not supported by
#    blt_barchart
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

set W .bargrph
frame $W -class Visi

# #################### Title bar #################

frame $W.top
pack $W.top -side top -fill x -expand 1 -anchor n

frame $W.top.left
pack $W.top.left -side left -fill both -expand 1

#label $W.top.left.title -relief sunken -text "BarChart Visualization" -foreground white -background DarkOliveGreen
label $W.top.left.title  -text "BarChart Visualization" -foreground white -background DarkOliveGreen
pack $W.top.left.title -side top -fill both -expand 1

# #################### Paradyn logo #################

label $W.top.logo -relief raised \
                  -bitmap @/p/paradyn/core/paradyn/tcl/logo.xbm \
                  -foreground DarkOliveGreen

pack $W.top.logo -side right 

# ############# Create the menubar as a frame with many menu buttons ###########

frame $W.top.left.mbar -class MyMenu -borderwidth 2 -relief raised
pack $W.top.left.mbar -side top -fill both -expand 1

# #################### File menu #################

menubutton $W.top.left.mbar.file -text File -menu $W.top.left.mbar.file.m
menu $W.top.left.mbar.file.m
$W.top.left.mbar.file.m add command -label "Close Bar chart" -command exit

# #################### Resource menu #################

menubutton $W.top.left.mbar.resource -text Resource -menu $W.top.left.mbar.resource.m
menu $W.top.left.mbar.resource.m
$W.top.left.mbar.resource.m add command -label "Add Resource..." -command AddEntry
$W.top.left.mbar.resource.m add command -label "Remove Selected Resource" -command DelEntry -state disabled

# #################### Options menu #################

menubutton $W.top.left.mbar.opts -text Options -menu $W.top.left.mbar.opts.m
#menu $W.top.left.mbar.opts.m -selector black
menu $W.top.left.mbar.opts.m
$W.top.left.mbar.opts.m add check -label "Long Names" -variable LongNames \
   -command Update -state disabled
$W.top.left.mbar.opts.m add separator
$W.top.left.mbar.opts.m add radio -label "Current Value" \
   -variable DataFormat -command {DgDataCallback 0 0} \
   -value Instantaneous
$W.top.left.mbar.opts.m add radio -label "Average Value" \
   -variable DataFormat -command {DgDataCallback 0 0} \
   -value Average
$W.top.left.mbar.opts.m add radio -label "Total Value" \
   -variable DataFormat -command {DgDataCallback 0 0} \
   -value Sum

# #################### Help menu #################

menubutton $W.top.left.mbar.help -text Help \
          -menu $W.top.left.mbar.help.m
menu $W.top.left.mbar.help.m 
$W.top.left.mbar.help.m add command -label "General" -command "NotImpl" -state disabled
$W.top.left.mbar.help.m add command -label "Context" -command "NotImpl" -state disabled


# #################### Build the menu bar and add to display #################

pack $W.top.left.mbar.file $W.top.left.mbar.resource $W.top.left.mbar.opts \
   -side left -padx 4
pack $W.top.left.mbar.help -side right

# #################### Organize all menu buttons into a menubar #################

tk_menuBar $W.top.left.mbar $W.top.left.mbar.file $W.top.left.mbar.resource \
   $W.top.left.mbar.opts $W.top.left.mbar.help

# ####################  Barchart Area (Data space initially blank} #################

frame $W.middle -height 1i -class Data
pack $W.middle -side top -fill both -expand 1

# ############################################################################
# ######### blt_drag&drop: declare that we are willing and able ##############
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
   flush stderr
}

blt_drag&drop target . handler text dragAndDropTargetHandler
#...that cryptic line reads: "declare the window '.' to be a drag n' drop
#   handler for sources of type 'text'; routine dragAndDropTargetHandler
#   gets called (via a "send" from the source...)  Using window '.' means
#   the entire barchart...

# NOTE: We do not attempt to make yet another callback to paradyn's core
#       to inform it of the addition; if so desired, it's the responsibility
#       of the drag n' drop source program...

# ###########################################################

pack append . $W {fill expand frame center}
wm minsize . 60 60
wm title . "Barchart"

# trap window resize events: (for some reason, <ResizeRequest> doesn't
# seem to work, so instead we revert to <Configure>, which is a superset
# of what we really wanted.   <Configure> gets called for any change of
# window characteristics, including simple window-movement)
# [sec 19.2: 'event patterns' in tk/tcl manual]

bind . <Configure> {myResizeHandler}

set DataFormat Instantaneous

# ###########################################################
# The remainder of the file are procedures which are waiting
# to be invoked as callbacks.  You may be wondering when
# (if ever) this program quits.  The answer is only through
# a direct call to "exit", which is the tcl command invoked
# on selection of the menu item "Close bar chart"...
# ###########################################################

proc rethinkLayout {} {
   # rethinkLayout - when a window size has changed, rethink the
   # position of the bar graph (most of the other widgets will
   # automatically do this, so there's no code for them...)

   puts stderr "Welcome to rethinkLayout"
   flush stderr

   # Since the bar chart is configured to always be in the window center,
   # I don't think anything has to be moved, per se---just resized.

#   DgConfigCallback
 
   global W
   
pack $W.middle -side top -fill both -expand 1

}

proc myResizeHandler {} {
   # called on a resize event
   # (currently, unfortunately, it is also called when a window is moved)

   rethinkLayout
}

# ###########################################################
# ###################### proc dialog   ######################
# ###########################################################
#
# A fairly generic dialog box, suitable for displaying
# quickie error messages.
#
# ###########################################################

proc dialog {w title text bitmap default args} {
   global button

   # 1. Create the top-level window and divide it into top
   # and bottom parts.

   catch {destroy $w}
   toplevel $w -class Dialog
   wm title $w $title
   wm iconname $w Dialog
   frame $w.top -relief raised -bd 1
   pack $w.top -side top -fill both
   frame $w.bot -relief raised -bd 1
   pack $w.bot -side bottom -fill both

   # 2. Fill the top part with bitmap and message.

   message $w.top.msg -width 2.5i -text $text \
     -font -Adobe-Times-Medium-R-Normal-*-180-*
   pack $w.top.msg -side right -expand 1 -fill both -padx 3m -pady 3m
   if {$bitmap != ""} {
      label $w.top.bitmap -bitmap $bitmap
      pack $w.top.bitmap -side left -padx 3m -pady 3m
   }

   # 3. Create buttons at the bottom of the dialog.

   set i 0
   foreach but $args {
      button $w.bot.button$i -text $but -command "set button $i"
      if {$i == $default} {
         frame $w.bot.default -relief sunken -bd 1
         raise $w.bot.button$i
         pack $w.bot.default -side left -expand 1 -padx 3m -pady 2m
         pack $w.button$i -in $w.bot.default -side left -padx 2m -pady 2m \
            -ipadx 2m -ipady 1m
      } else {
         pack $w.bot.button$i -side left -expand 1 \
            -padx 3m -pady 3m -ipadx 2m -ipady 1m
      }
      incr i
   }

   if {$default >= 0} {
      bind $w <Return> "$w.bot.button$default flash; \
      set button $default"
   }
   set oldFocus [focus]
   grab set $w
   focus $w

   tkwait variable button
   destroy $w
   focus $oldFocus
   return $button
}

# #################### Called by visi library when histos have folded #################

proc DgFoldCallback {} {
}


# #################### Called by visi library when metric/resource space changes.
# #################### Creates the barchart

proc DgConfigCallback {} {
#   puts stderr "Welcome to DgConfigCallback; metric/resource space must have changed..."
#   flush stderr

   global W
  
   set numResources [Dg numresources]
   set numMetrics   [Dg nummetrics]
   set metricName   [Dg metricname 0]
   set metricUnits  [Dg metricunits 0]

#   puts stderr "DgConfigCallback: $numResources resources..."
#   puts stderr "DgConfigCallback: metricName is $metricName"
#   puts stderr "DgConfigCallback: $numMetrics metrics..."
#   puts stderr "DgConfigCallback: metricUnits is $metricUnits"
#   flush stderr

   if {$numMetrics > 1} {
      dialog .d {Error} {The number of metrics can not exceed 1.} warning -1 OK
      destroy .
   } else { 
      catch {destroy $W.bottom.status}
      catch {destroy $W.middle.chart}

      # create the chart in the middle of the window.
      # blt_barchart is an external tk/tcl package.  see /p/paradyn/packages/blt-1.7
      blt_barchart $W.middle.chart

      # configure the x-axis
      $W.middle.chart xaxis configure -command GetName \
         -stepsize 1 -subticks 0 -max [expr $numResources] \
	 -font -*-helvetica-medium-r-*-*-*-100-*-*-*-*-* \
	 -loose true

      # configure the y-axis
      $W.middle.chart yaxis configure -min 0 -subticks 5 -loose true

      pack $W.middle.chart

      # titles for axes
      $W.middle.chart yaxis configure -title $metricUnits
      $W.middle.chart xaxis configure -title "Resources"

      # overall chart title
      $W.middle.chart configure -title $metricName

      # chart legend
      $W.middle.chart legend configure -mapped false

      # Loops through the resources, drawing their current values now, if those
      # values are valid. (maybe we should instead call DgDataCallback directly...
      for {set r 0} {$r < $numResources} {incr r} {
         set Resource [Dg resourcename $r]
         set Resource [file tail $Resource]
         if {[Dg valid 0 $r]} {
            set value [GetValue 0 $r]
         } else {
            set value 0
         }

         # draw the bar by simply declaring a chart element whose x-value is
         # the resource name and whose y-value is $value
         $W.middle.chart element create $Resource -xdata $r -ydata $value
      }
   }
}

# #################### procedure GetName #################
# called by blt_barchart whenever it needs the name of an x-axis
# component.  value is a numerical x-axis value

proc GetName {w value} {
#   puts stderr "Welcome to GetName; value is $value"
#   flush stderr

   if {$value < 0 || $value >= [Dg numresources]} {
      return " "
   } else {
      return [file tail [Dg resourcename $value]]
   }
}


#
#
#
proc DgValidCallback {m r} {
}  


# ################# GetValue: called from DgDataCallback, below ###########

proc GetValue {m n} {
#   puts stderr "Welcome to GetValue; m is $m and n is $n"
#   flush stderr

   # DataFormat is one of {Average, Sum, Instantaneous}
   global DataFormat

   if {[string match $DataFormat Average]} {
      return [Dg aggregate $m $n]
   }
   if {[string match $DataFormat Sum]} {
      return [Dg sum $m $n]
   }
   return [Dg value $m $n [Dg lastbucket $m $n]]
}

# #################### DgDataCallback -- called when new data is available #################

proc DgDataCallback {first last} {
#   puts stderr "Welcome to DgDataCallback; first is $first and last is $last"
#   flush stderr

   global W

   set numResources [Dg numresources]
   for {set r 0} {$r < $numResources} {incr r} {
      set theResource [file tail [Dg resourcename $r]]
      $W.middle.chart element configure $theResource -ydata [GetValue 0 $r]
   }
}

# #################### Update labels when option longnames is selected #################

proc Update {}  {
#   puts stderr "Welcome to Update()"
#   flush stderr

   global W

   set numResources [Dg numresources]

   for {set r 0} {$r < $numResources} {incr r} {
      set theResource [file tail [Dg resourcename $r]]
      $W.middle.chart element configure $theResource -label [Dg resourcename $r]
   }
}


# ####################  AddEntry -- Ask paradyn to start a new curve #################

proc AddEntry {} {
#   puts stderr "Welcome to AddEntry()"
#   flush stderr

   Dg start "*" "*"
}

# #################### DelEntry -- Ask paradyn to stop a curve #################

proc DelEntry {} {
#   puts stderr "Welcome to DelEntry()"
#   flush stderr

   puts stderr "Delete Entry not yet implemented"
}
