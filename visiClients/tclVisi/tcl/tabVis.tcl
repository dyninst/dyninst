#
#  tabVis -- A tabular display visualization for Paradyn
#
#  $Log: tabVis.tcl,v $
#  Revision 1.2  1994/06/01 16:59:30  rbi
#  Better menu bar.  Short names option.
#
# Revision 1.1  1994/05/31  22:18:53  rbi
# Fix for direct execution of tcl visis
#
#

#
#  default display options
#
option add *Table*background IndianRed3
option add *Table*activeBackground IndianRed2
option add *Table*font *-New*Century*Schoolbook-Bold-R-*-18-*
option add *Table*foreground white
option add *Data*font *-Helvetica-*-r-*-12-* 
option add *MyMenu*font *-New*Century*Schoolbook-Bold-R-*-14-*

#
#  Create the overall frame
#
frame .table -class Table

#
#  Create the menubar as a frame with many menu buttons
#
frame .table.menubar -class MyMenu -borderwidth 2 -relief raised

#
#  File menu
# 
menubutton .table.menubar.file -text "File" -menu .table.menubar.file.m
menu .table.menubar.file.m
.table.menubar.file.m add command -label "Close" -command Shutdown


#
#  Options menu
#
menubutton .table.menubar.opts -text "Options" -menu .table.menubar.opts.m
menu .table.menubar.opts.m -selector black
.table.menubar.opts.m add command -label "Signif. Digits" -command SetSignif
.table.menubar.opts.m add check -label "Short Names" -variable ShortNames
.table.menubar.opts.m add separator
.table.menubar.opts.m add radio -label "Current Value" \
    -variable DataFormat \
    -value Instantaneous
.table.menubar.opts.m add radio -label "Average Value" \
    -variable DataFormat \
    -value Average
.table.menubar.opts.m add radio -label "Total Value" \
    -variable DataFormat \
    -value Sum
.table.menubar.opts.m invoke 3

#
#  Put in a frame to space the menu bar
#
frame .table.menubar.pad -width 2i

#
#  Help menu
#
menubutton .table.menubar.help -text "Help" -menu .table.menubar.help.m
menu .table.menubar.help.m
.table.menubar.help.m add command -label "General" -command "NotImpl"
.table.menubar.help.m add command -label "Context" -command "NotImpl"
.table.menubar.help.m disable 0
.table.menubar.help.m disable 1

#
#  Build the menu bar and add to display
#
pack .table.menubar.file .table.menubar.opts -side left
pack .table.menubar.help -side right

blt_table .table .table.menubar 0,0 -cspan 50 -pady 2 -fill both

#
#  Organize all menu buttons into a menubar
#
tk_menuBar .table.menubar .table.menubar.file .table.menubar.opts .table.menubar.help 

#
#  Data space initially blank
#
frame .table.data -height 1i -class Data
blt_table .table .table.data 1,0 -cspan 50 -pady .25i

#
#  Build close button and status field at the bottom
#
button .table.close -text "Close" -command Shutdown -activebackground IndianRed2 -padx 4 -pady 4
label .table.status -text "No Data Yet" \
      -font *-Helvetica-*-o-*-12-* \
      -foreground yellow -padx 6

blt_table .table .table.status 50,1 -cspan 4 \
                 .table.close 50,0 

#
#  Finally display everything
#
pack append . .table {fill expand frame center}

#
#  Significant digits adjustment is done with a scale widget
#
set SignificantDigits 2

proc SetSignif {{w .signif}} {
  global SignificantDigits

  catch {destroy $w}
  toplevel $w -class Table
  wm geometry $w +300+300
  wm title $w "Signif Digits"
  wm iconname $w "SignifDigits"
  scale $w.scale -orient horizontal -length 280 -from 0 -to 8 \
    -tickinterval 1 -bg IndianRed3 -command "set SignificantDigits " \
    -borderwidth 5 -showvalue false -label "Significant Digits" \
    -font *-New*Century*Schoolbook-Bold-R-*-14-*
  $w.scale set $SignificantDigits

  button $w.ok -text OK -command "destroy $w"
  pack $w.scale $w.ok -side top -fill x
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
  catch {destroy .table.data}
  frame .table.data -class Data 
  blt_table .table .table.data 1,0 -cspan 50 -pady .25i 

  set nM [Dg nummetrics]
  set nR [Dg numresources]

  for {set m 0} {$m < $nM} {incr m} {
    set row [expr $m+1]
    label .table.data.rowlabel($m) -text [Dg metricname $m]
    label .table.data.rowunits($m) -text [Dg metricunits $m]
    blt_table .table.data .table.data.rowlabel($m) $row,0 -anchor e -pady 2 \
                          .table.data.rowunits($m) $row,99 -anchor w -pady 2
    for {set r 0} {$r < $nR} {incr r} {
      if {$row == 1} {
        set col [expr $r+1]
        label .table.data.collabel($r) -text [GetResourceName $r]
        blt_table .table.data .table.data.collabel($r) 0,$col -anchor e
	blt_table column .table.data configure $col -padx 2
      }
      label .table.data.label($m,$r)
      blt_table .table.data .table.data.label($m,$r) $row,$col -anchor e
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
  DgDataCallback
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
  set nR [Dg numresources]
  for {set r 0} {$r < $nR} {incr r} {
    .table.data.collabel($r) configure -text [GetResourceName $r]
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
  global DataFormat

  .table.status configure -text "Binwidth = [Dg binwidth], DataFormat: $DataFormat"
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
#  DgDataCallback -- visi calls this command when new data is available
#    we fill in all of the data labels with the new data values
#
proc DgDataCallback {} {
  global SignificantDigits

  set nM [Dg nummetrics]
  set nR [Dg numresources]
  for {set m 0} {$m < $nM} {incr m} {
    for {set r 0} {$r < $nR} {incr r} {
      if {[Dg valid $m $r]} {
        set value [GetValue $m $r]
        .table.data.label($m,$r) configure \
           -text [format "%.[set SignificantDigits]f" $value]
      }
    }
  }
}

