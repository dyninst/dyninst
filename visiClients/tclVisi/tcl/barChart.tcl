#
#  barChart -- A bar chart display visualization for Paradyn
#
#  $Log: barChart.tcl,v $
#  Revision 1.3  1994/09/03 01:24:40  tamches
#  Cleaned up syntax some more, e.g. longer variable names.
#  Cleaned up menus
#  Added many comments
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
# 3) fix the x-axis, part 2: display resource names differently; the 90 degree
#    rotation may be cute, but it isn't useful; make it like a tcl label
#    which automatically does line wrapping and can be made to justify
#    as desired.  Make the font smaller, too.
# 4) Make chart title font bigger.
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
#$W.top.left.mbar.resource.m disable 1

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

pack append . $W {fill expand frame center}
wm minsize . 60 60

# ###########################################################
# The remainder of the file are procedures which are waiting
# to be invoked as callbacks.  You may be wondering when
# (if ever) this program quits.  The answer is only through
# a direct call to "exit", which is the tcl command invoked
# on selection of the menu item "Close bar chart"...
# ###########################################################


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

   global W
  
   set numResources [Dg numresources]
   set numMetrics   [Dg nummetrics]
   set metricName   [Dg metricname 0]
   set metricUnits  [Dg metricunits 0]

#   puts stderr "DgConfigCallback: $numResources resources..."
#   puts stderr "DgConfigCallback: metricName is $metricName"
#   puts stderr "DgConfigCallback: $numMetrics metrics..."
#   puts stderr "DgConfigCallback: metricUnits is $metricUnits"

   if {$numMetrics > 1} {
      dialog .d {Error} {The number of metrics can not exceed 1.} warning -1 OK
      destroy .
   } else { 
      catch {destroy $W.bottom.status}
      catch {destroy $W.middle.chart}

      # create the chart in the middle of the window.
      blt_barchart $W.middle.chart 
      $W.middle.chart xaxis configure -command GetName -rotate 90 \
         -stepsize 1 -subticks 0 -max [expr $numResources + 1]
      $W.middle.chart yaxis configure -min 0 -subticks 10 -loose true
      pack $W.middle.chart

      # titles for axes
      $W.middle.chart yaxis configure -title $metricUnits
      $W.middle.chart xaxis configure -title "Resources"

      # overall chart title
      $W.middle.chart configure -title $metricName

      # chart legend
      $W.middle.chart legend configure -mapped false

      # Loops through the resources, drawing their current values now, if those
      # values are valid.   
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

# #################### Sets default dataformat #################

set DataFormat Instantaneous

# #################### Returns resource name #################

proc GetName {w value} {
#   puts stderr "Welcome to GetName..."

   global W
   set r [Dg resourcename $value]
   set r [file tail $r]
   set numResources [Dg numresources]
   if {$value < $numResources} {
      return $r
   } else {
      return "    "
   }
}


#
#
#
proc DgValidCallback {m r} {
}  


# ################# Asks visi library for the data value for the met/res pair ###########

proc GetValue {m n} {
#   puts stderr "Welcome to GetValue"

   global DataFormat

   if {[string match $DataFormat Average]} {
      return [Dg aggregate $m $n]
   }
   if {[string match $DataFormat Sum]} {
      return [Dg sum $m $n]
   }
   return [Dg value $m $n [Dg lastbucket $m $n]]
}

# #################### Calls this command when new data is available #################

proc DgDataCallback {first last} {
#   puts stderr "Welcome to DgDataCallback"

   global W

   set numResources [Dg numresources]
   for {set r 0} {$r < $numResources} {incr r} {
      set Resource [Dg resourcename $r]
      set Resource [file tail $Resource]
      set value [GetValue 0 $r]
      $W.middle.chart element configure $Resource -ydata $value
   }
}

# #################### Update labels when option longnames is selected #################

proc Update {}  {
#   puts stderr "Welcome to Update()"

   global W

   set numResources [Dg numresources]
   for {set r 0} {$r < $numResources} {incr r} {
      set Resource [Dg resourcename $r]
      set Resource [file tail $Resource]
      $W.middle.chart element configure $Resource -label [Dg resourcename $r]
   }
}


# ####################  AddEntry -- Ask paradyn to start a new curve #################

proc AddEntry {} {
#   puts stderr "Welcome to AddEntry()"

   Dg start "*" "*"
}

# #################### DelEntry -- Ask paradyn to stop a curve #################

proc DelEntry {} {
#   puts stderr "Welcome to DelEntry()"

   puts stderr "Delete Entry not yet implemented"
}
