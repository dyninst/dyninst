#
#  tabVis -- A tabular display visualization for Paradyn
#
#  $Log: tabVis.tcl,v $
#  Revision 1.4  1994/07/20 21:52:27  rbi
#  Better support for BW and standard color scheme
#  Added "Actions" menu
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

#
#  Create the overall frame
#
set W .table
frame $W -class Visi

#
#  Create the title bar, menu bar, and logo at the top
#
frame $W.top
pack $W.top -side top -fill x -expand 1 -anchor n

frame $W.top.left 
pack $W.top.left -side left -fill both -expand 1

label $W.top.left.title -relief raised -text "Table Visualization" \
      -foreground white -background HotPink4

pack $W.top.left.title -side top -fill both -expand 1

#
#  Create the menubar as a frame with many menu buttons
#
frame $W.top.left.menubar -class MyMenu -borderwidth 2 -relief raised
pack $W.top.left.menubar -side top -fill both -expand 1

#
#  File menu
# 
menubutton $W.top.left.menubar.file -text "File" -menu $W.top.left.menubar.file.m
menu $W.top.left.menubar.file.m
$W.top.left.menubar.file.m add command -label "Close" -command Shutdown

#
#  Actions menu
#
menubutton $W.top.left.menubar.acts -text "Actions" -menu $W.top.left.menubar.acts.m
menu $W.top.left.menubar.acts.m
$W.top.left.menubar.acts.m add command -label "Add Entry" -command AddEntry
$W.top.left.menubar.acts.m add command -label "Delete Entry" -command DelEntry

#
#  Options menu
#
menubutton $W.top.left.menubar.opts -text "Options" -menu $W.top.left.menubar.opts.m
menu $W.top.left.menubar.opts.m -selector black
$W.top.left.menubar.opts.m add command -label "Signif. Digits" -command SetSignif
$W.top.left.menubar.opts.m add check -label "Short Names" -variable ShortNames
$W.top.left.menubar.opts.m add separator
$W.top.left.menubar.opts.m add radio -label "Current Value" \
    -variable DataFormat \
    -value Instantaneous
$W.top.left.menubar.opts.m add radio -label "Average Value" \
    -variable DataFormat \
    -value Average
$W.top.left.menubar.opts.m add radio -label "Total Value" \
    -variable DataFormat \
    -value Sum
$W.top.left.menubar.opts.m invoke 3

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
pack $W.top.left.menubar.file $W.top.left.menubar.acts $W.top.left.menubar.opts -side left 
pack $W.top.left.menubar.help -side right 

#
#  Organize all menu buttons into a menubar
#
tk_menuBar $W.top.left.menubar $W.top.left.menubar.file $W.top.left.menubar.acts $W.top.left.menubar.opts $W.top.left.menubar.help 

#
#  Build the logo 
#
label $W.top.logo -relief raised -bitmap @~paradyn/core/paradyn/tcl/logo.xbm \
                  -foreground #b3331e1b53c7

pack $W.top.logo -side right

#
#  Data space initially blank
#
frame $W.middle -height 1i -class Data
pack $W.middle -side top -fill both -expand 1

#
#  Build close button and status field at the bottom
#
frame $W.bottom
pack $W.bottom -side bottom -fill x -expand 1 -anchor s

button $W.bottom.close -text "Close" -command Shutdown -padx 4 -pady 4
label $W.bottom.status -text "No Data Yet" \
      -font *-Helvetica-*-o-*-12-* \
      -foreground blue -padx 6

pack $W.bottom.close -side left -fill both 
pack $W.bottom.status -side left

#
#  Finally display everything
#
pack append . $W {fill expand frame center}
wm minsize . 50 50

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
  UpdateStatus 
}

#
#  Called by visi library when met/res space changes
#    we rebuild the table
#
proc DgConfigCallback {} {
  global W

  catch {destroy $W.middle.data}
  frame $W.middle.data -class Data 
  pack $W.middle.data -side top -fill both -expand 1

  set nM [Dg nummetrics]
  set nR [Dg numresources]

  for {set m 0} {$m < $nM} {incr m} {
    set row [expr $m+1]
    label $W.middle.data.rowlabel($m) -text [Dg metricname $m]
    label $W.middle.data.rowunits($m) -text [Dg metricunits $m]
    blt_table $W.middle.data $W.middle.data.rowlabel($m) $row,0 -anchor e -pady 2 \
                          $W.middle.data.rowunits($m) $row,99 -anchor w -pady 2
    for {set r 0} {$r < $nR} {incr r} {
      if {$row == 1} {
        set col [expr $r+1]
        label $W.middle.data.collabel($r) -text [GetResourceName $r]
        blt_table $W.middle.data $W.middle.data.collabel($r) 0,$col -anchor e
	blt_table column $W.middle.data configure $col -padx 2
      }
      label $W.middle.data.label($m,$r)
      blt_table $W.middle.data $W.middle.data.label($m,$r) $row,$col -anchor e
    }
  }

  UpdateStatus
}

#
#  Default dataformat is Average
#    we also support Sum and Instantaneous
#
set DataFormat Average

#
#  Whenever DataFormat is changed, get new data and update status line
#
trace variable DataFormat w FormatChanged

proc FormatChanged {name1 name2 how} {
  DgDataCallback 0 0
  UpdateStatus
}

#
#  Default to long names
#
set ShortNames 0

#
#  Whenever ShortNames is changed, redo resource labels
#  
trace variable ShortNames w UpdateResourceLabels

proc UpdateResourceLabels {name1 name2 how} {
  global W

  set nR [Dg numresources]
  for {set r 0} {$r < $nR} {incr r} {
    $W.middle.data.collabel($r) configure -text [GetResourceName $r]
  }    
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

  $W.bottom.status configure -text "Interval = [Dg binwidth] s, DataFormat: $DataFormat"
}

#
#  GetValue asks visi library for the data value for the met/res pair 
#    we ask visi for the data in the correct DataFormat
#
proc GetValue {m n} {
  global DataFormat

  if {[string match $DataFormat Average]} {
    return [Dg aggregate $m $n]
  }
  if {[string match $DataFormat Sum]} {
    return [Dg sum $m $n]
  }
  return [Dg value $m $n [Dg lastbucket $m $n]]
}

#
# DgValidCallback -- visi calls this when curve becomes valid
#
proc DgValidCallback {m r} {
  puts stderr [format "Curve %d %d is now valid" $m $r]
}

#
#  DgDataCallback -- visi calls this command when new data is available
#    we fill in all of the data labels with the new data values
#
proc DgDataCallback {first last} {
  global SignificantDigits W

  set nM [Dg nummetrics]
  set nR [Dg numresources]
  for {set m 0} {$m < $nM} {incr m} {
    for {set r 0} {$r < $nR} {incr r} {
      if {[Dg valid $m $r]} {
        set value [GetValue $m $r]
        $W.middle.data.label($m,$r) configure \
           -text [format "%.[set SignificantDigits]f" $value]
      }
    }
  }
}

