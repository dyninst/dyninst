#
# Copyright (c) 1996-2004 Barton P. Miller
# 
# We provide the Paradyn Parallel Performance Tools (below
# described as "Paradyn") on an AS IS basis, and do not warrant its
# validity or performance.  We reserve the right to update, modify,
# or discontinue this software at any time.  We shall have no
# obligation to supply such updates or modifications or any other
# form of support to you.
# 
# This license is for research uses.  For such uses, there is no
# charge. We define "research use" to mean you may freely use it
# inside your organization for whatever purposes you see fit. But you
# may not re-distribute Paradyn or parts of Paradyn, in any form
# source or binary (including derivatives), electronic or otherwise,
# to any other organization or entity without our permission.
# 
# (for other uses, please contact us at paradyn@cs.wisc.edu)
# 
# All warranties, including without limitation, any warranty of
# merchantability or fitness for a particular purpose, are hereby
# excluded.
# 
# By your use of Paradyn, you understand and agree that we (or any
# other person or entity with proprietary rights in Paradyn) are
# under no obligation to provide either maintenance services,
# update services, notices of latent defects, or correction of
# defects for Paradyn.
# 
# Even if advised of the possibility of such damages, under no
# circumstances shall we (or any other person or entity with
# proprietary rights in the software licensed hereunder) be liable
# to you or any third party for direct, indirect, or consequential
# damages of any character regardless of type of action, including,
# without limitation, loss of profits, loss of use, loss of good
# will, or computer failure or malfunction.  You agree to indemnify
# us (and any other person or entity with proprietary rights in the
# software licensed hereunder) for any and all liability it may
# incur to third parties resulting from your use of Paradyn.
#

##!/s/std/bin/wish
##!/usr/bin/wish

#button .pop -text "Export ..." \
  #-command ::Export::ExportHandler
#pack .pop -padx 20 -pady 10

proc ExportHandler {} {
  set win .export_window

  toplevel $win
  wm title $win "Paradyn Export"
  wm iconname $win "Paradyn Export"
  
  frame $win.top_frame
  frame $win.checkbutton_frame
  frame $win.dirent_frame
  frame $win.bottom_frame
  pack $win.top_frame $win.checkbutton_frame $win.dirent_frame \
       $win.bottom_frame -side top -fill both  -padx 2 -pady 5

  label $win.top_frame.la -text "Paradyn Data Export Menu" \
	    -foreground white -anchor c \
	    -font { Times 14 bold } \
	    -relief raised \
	    -background purple
  pack  $win.top_frame.la -side top -fill both -expand true

# Organize the checkbutton list!
# get_subscribed_mrpairs() creates and initializes 2 global lists:
#   mrIDbyindex[]: list containing formatted strings: "metricid:resourceid"
#   mrlabelbyindex[]: list containing formatted strings: "metriclabel:resourcelabel"

  set num_MRpairs [get_subscribed_mrpairs]
  set num_MRpairs

  if { $num_MRpairs < 1 } {
    puts { get_subscribed_mrpairs() failed: Save Aborted}
    destroy .export_window
  }    

  global mrIdByIndex mrLabelByIndex
  global SelectedMRPairs num_SelectedMRPairs
  #global saveDirectory

  frame $win.checkbutton_frame.1
  frame $win.checkbutton_frame.2
  frame $win.checkbutton_frame.3
  if { [expr $num_MRpairs % 3] > 0} {
    set colSize [expr (($num_MRpairs/3) + 1)]
  } else {
    set colSize [expr ($num_MRpairs/3)]
  }
  set colNum 1
  set cCnt 1

  for {set i 0} {$i < $num_MRpairs} {incr i} {
    set mrId    [lindex $mrIdByIndex $i]
    set mrLabel [lindex $mrLabelByIndex $i]

    set SelectedMRPairs($mrId) 1

    checkbutton $win.checkbutton_frame.$colNum.cb$i  -width 30 -anchor w \
                -padx 2 -text $mrLabel -variable SelectedMRPairs($mrId) \
                -relief groove -command "ExportOK $SelectedMRPairs($mrId)"

    pack $win.checkbutton_frame.$colNum.cb$i -side top
    if { $cCnt >= $colSize} { 
      incr colNum
      set cCnt 1
    } else {
      incr cCnt
    }
  }

  #pack the 3 columns of checkbuttons beside each other from the left
  pack $win.checkbutton_frame.1 $win.checkbutton_frame.2 \
       $win.checkbutton_frame.3 \
       -side left -fill both -expand 1

  #label $win.dirent_frame.la -text "Directory:" -padx 0
  #entry $win.dirent_frame.direntry -width 50 -textvariable saveDirectory -relief sunken
  #pack $win.dirent_frame.la $win.dirent_frame.direntry -side left -fill x

  #label $win.bottom_frame.msg -text "Enter name of Directory for Data/Resource Files" -padx 0
  #pack $win.bottom_frame.msg -side top

  #bind $win.dirent_frame.direntry <KeyRelease> {ExportOK -1}

  button $win.bottom_frame.export -text EXPORT -height 1 \
         -command "HandleExport"
  button $win.bottom_frame.all -text "SELECT ALL" -height 1 \
      -command "SelectAll"
  button $win.bottom_frame.clear -text CLEAR -height 1 \
      -command "ClearAll"
  button $win.bottom_frame.cancel -text CANCEL -height 1 \
         -command "destroy .export_window"

  global $win.bottom_frame.export
  SelectAll

 pack $win.bottom_frame.export $win.bottom_frame.all \
      $win.bottom_frame.clear $win.bottom_frame.cancel \
      -side left -expand true -padx 15 -pady 4
}

proc ClearAll {} {
  global SelectedMRPairs num_SelectedMRPairs

  foreach mrId $::mrIdByIndex {
    set SelectedMRPairs($mrId) 0
  }
  set num_SelectedMRPairs 0
  ExportOK -1
}

proc SelectAll {} {
  global SelectedMRPairs num_SelectedMRPairs

  set num_SelectedMRPairs 0
  foreach mrId $::mrIdByIndex {
    set SelectedMRPairs($mrId) 1
    incr num_SelectedMRPairs
  }

  ExportOK -1
}

proc HandleExport {} {
  list filteredMRPairs
  global SelectedMRPairs

  foreach mrId $::mrIdByIndex {
    if { $SelectedMRPairs($mrId) == 1} {
      lappend filteredMRPairs $mrId
    }
  }

  destroy .export_window
  set saveFile [tk_getSaveFile -initialdir "." -initialfile "histograms" -title "Filename Query" ]

  set return_val [DoExport $saveFile $filteredMRPairs] 
  unset SelectedMRPairs saveFile ::mrIdByIndex ::mrLabelByIndex
}

proc ExportOK {i} {
  set win .export_window
  global $win.bottom_frame.export
  global SelectedMRPairs

  set num_SelectedMRPairs 0

  foreach mrId $::mrIdByIndex {
    if { $SelectedMRPairs($mrId) == 1} {
      incr num_SelectedMRPairs
    }
  }  

  if { $num_SelectedMRPairs == 0 } {
    #disable export
    #puts {Disabling: number of pairs selected:}
    #puts $num_SelectedMRPairs
    $win.bottom_frame.export configure -state disabled
  } else {
    #enable export
    #puts {Enabling: number of pairs selected:}
    #puts $num_SelectedMRPairs
    $win.bottom_frame.export configure -state normal
  }
}
