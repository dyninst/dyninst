#
# Copyright (c) 1996-1999 Barton P. Miller
# 
# We provide the Paradyn Parallel Performance Tools (below
# described as Paradyn") on an AS IS basis, and do not warrant its
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
#---------------------------------------------------------------------------
#
# histVisi.tcl
#
# Tcl implementation of the user interface for the Paradyn Histogram visi.
# The RTHist namespace contains the functions for building the main window
# user interface, including menus.  The bulk of the Histogram visi is 
# actually a custom pdgraph Tk widget.
#
#---------------------------------------------------------------------------
# $Id: histVisi.tcl,v 1.5 2001/11/07 05:03:21 darnold Exp $
#---------------------------------------------------------------------------


#
# dbg_niy - a placeholder to alert the user of a feature in development
#
proc dbg_niy {} {

    tk_messageBox -type ok -title "Debug" \
        -message "Not implemented yet" -icon info
}


namespace eval RTHist {

    variable is_kept_on_paradyn_exit 0


    #
    # init - initialize the Histogram visi user interface
    #
    proc init {} \
    {
		option add *Font "Helvetica 9 roman" widgetDefault
        option add *Pdheader*Font "{New Century Schoolbook} 12 bold roman" widgetDefault
		option add *Pdgraph*Font "Helvetica 9 roman" widgetDefault

        # set up main window
        wm title . "Histogram"

        # set up sub-areas within main window
        ::RTHist::init_header
        ::RTHist::init_content .hist
    }


    #
    # init_content - initialize the content portion of the main window
    #
    proc init_content { wname } {

        
        # our frame - this appears to the outside world as our widget
        frame $wname

		# set up our status bar
		frame $wname.statusBarFrame
		pack $wname.statusBarFrame -side bottom -fill x

		label $wname.statusBarFrame.phaseLabel -relief flat \
			-text "Phase: <unknown>"
		pack $wname.statusBarFrame.phaseLabel -side left -expand false

        # set up our scroll bars along the bottom and right side
        # of our window, with a small corner where the scroll bars
        # do not overlap
        frame $wname.panf
        pack $wname.panf -side bottom -fill x

        label $wname.panf.corner -bitmap error -bg [$wname cget -bg] -fg [$wname cget -bg]
        pack $wname.panf.corner -side right -fill y -expand false
        scrollbar $wname.panf.pan -orient horizontal
        pack $wname.panf.pan -side right -fill x -expand true

        scrollbar $wname.zoom -orient vertical
        pack $wname.zoom -side right -fill y

        $wname.panf.pan set 0 1
        $wname.zoom set 0.80 1

        $wname.panf.corner configure -width [$wname.zoom cget -width] -height 0 -borderwidth 0

        # set up our "Pan" and "Zoom" labels by the scroll bars
        frame $wname.labf
        pack $wname.labf -side bottom -fill x
        label $wname.labf.panlab -text Pan 
        pack $wname.labf.panlab -fill x

        label $wname.zoomlab -text "Z\no\no\nm"
        pack $wname.zoomlab -side right -fill y

        # set up our pdgraph widget in the remainder of the area
        pdgraph $wname.graph -relief sunken -borderwidth 2
        pack $wname.graph -side top -fill both -expand true

        pack $wname -fill both -expand true

        # set reasonably-sized initial window
        wm geometry . =600x400

        # associate scrolling actions in scrollbars with our pdgraph widget
        $wname.panf.pan configure -command ::RTHist::pan
        $wname.zoom configure -command ::RTHist::zoom
    }


    #
    # init_menus - initialize the menus for the Histogram visi
    #
    proc init_menus { mbar } {

        # File menu
        menubutton $mbar.filemb -text "File" -menu $mbar.filemb.m
        menu $mbar.filemb.m -tearoff false
        $mbar.filemb.m add checkbutton -label "Keep on Paradyn exit" -variable ::RTHist::is_kept_on_paradyn_exit -onvalue 1 -offvalue 0
        $mbar.filemb.m add command -label "Save ..." -command "ExportHandler"
        $mbar.filemb.m add separator
        $mbar.filemb.m add command -label "Close" -command "::RTHist::shutdown"
        pack $mbar.filemb -side left

        # Actions menu
        menubutton $mbar.actionsmb -text "Curve" -menu $mbar.actionsmb.m
        menu $mbar.actionsmb.m -tearoff false -postcommand "::RTHist::actionsm_posted $mbar.actionsmb.m"
        $mbar.actionsmb.m add command -label "Add..." -command "dg start \"*\" \"*\" "
        $mbar.actionsmb.m add command -label "Remove" -state disabled -command "::RTHist::handle_remove"
        $mbar.actionsmb.m add command -label "Smooth" -state disabled -command "::RTHist::handle_smooth"
        $mbar.actionsmb.m add command -label "Unsmooth" -state disabled -command "::RTHist::handle_unsmooth"
        $mbar.actionsmb.m add command -label "Show" -state disabled -command "::RTHist::handle_show"
        $mbar.actionsmb.m add command -label "Hide" -state disabled -command "::RTHist::handle_hide"
        pack $mbar.actionsmb -side left

        # View menu
# Note - this menu is disabled until the support for black and white
# is fully implemented in the PDGraph widget.
#
# The issue that keeps it out now is that there is a bug in the
# canvas widget that makes line items ignore any "-stipple" option.
#
# One way to workaround the problem would be to use window items
# instead of line items, and to draw the line from C++ code.
#

#        menubutton $mbar.viewmb -text "View" -menu $mbar.viewmb.m
#        menu $mbar.viewmb.m -tearoff false
#        $mbar.viewmb.m add checkbutton -label "Color" -command dbg_niy
#        pack $mbar.viewmb -side left
    }



    #
    # init_header - initializes the window header (including menu bar,
    # "title banner", and Paradyn logo
    #
    proc init_header {} {

        # frame around header widgets
        frame .header -class Pdheader
        pack .header -side top -fill x

        # logo
        makeLogo .header.logo paradynLogo raised 2 DarkSlateBlue
        pack .header.logo -side right -fill y

        # title bar
        label .header.title -text "Histogram Visualization" \
            -fg white -bg DarkSlateBlue -relief raised
        pack .header.title -side top -fill both -expand true

        # menu bar
        frame .header.menubar -borderwidth 2 -relief raised
        pack .header.menubar -side bottom -fill both
        ::RTHist::init_menus .header.menubar
    }



    #
    # shutdown - initiate a graceful shutdown of the program
    #
    proc shutdown {} \
    {
        destroy .
    }


    #
    # pan - called by the panning scroll bar.  Conforms to the 
    # interface expected by scroll-handling functions in Tk.
    # (I.e., it supports the 'moveto' and 'scroll' arguments.)
    #
    proc pan { args } {

        # tell the graph to scroll itself
        #
        # (this would be cleaner if we had a way to extract all the 
        # elements of a list into a string - then we could apply this to
        # the args list and obtain individual arguments to pass to the pan command)
        #
        if { [llength $args] == 2 } {
            set posspec [.hist.graph pan [lindex $args 0] [lindex $args 1]]
        } else {
            set posspec [.hist.graph pan [lindex $args 0] [lindex $args 1] [lindex $args 2]]
        }

        # update our scrollbar position from the result
        .hist.panf.pan set [lindex $posspec 0] [lindex $posspec 1]
    }

    #
    # zoom - called by the zooming scroll bar.  Conforms to the 
    # interface expected by scroll-handling functions in Tk.
    # (I.e., it supports the 'moveto' and 'scroll' arguments.)
    #
    proc zoom { args } {
        
        # tell the graph to zoom itself
        #
        # (this would be cleaner if we had a way to extract all the
        # elements of a list into a string - then we could apply this to
        # the args list and obtain individual arguments to pass to the zoom command)
        if { [llength $args] == 2 } {
            set posspec [.hist.graph zoom [lindex $args 0] [lindex $args 1]]
        } else {
            set posspec [.hist.graph zoom [lindex $args 0] [lindex $args 1] [lindex $args 2]]
        }

        #update our scrollbar positions based on the result
        .hist.zoom set [lindex $posspec 0] [lindex $posspec 1]
        .hist.panf.pan set [lindex $posspec 2] [lindex $posspec 3]
    }


    #
    # actionsm_posted - callback for the Actions menu.
    # Called when the Actions menu is posted, so as to update
    # the state of the menu items in that menu.
    #
    proc actionsm_posted { amenu } {

        # examine state of the pdgraph widget to determine 
        # whether any items are selected
        set sellist [.hist.graph getselected]

        if { [llength $sellist] > 0 } {
            set mstate "normal"
        } else {
            set mstate "disabled"
        }

        # update the state of our menu items based on what we found
        $amenu entryconfigure 1 -state $mstate
        $amenu entryconfigure 2 -state $mstate
        $amenu entryconfigure 3 -state $mstate
        $amenu entryconfigure 4 -state $mstate
        $amenu entryconfigure 5 -state $mstate
        $amenu entryconfigure 6 -state $mstate
    }


    #
    # handle_remove - menu item handler, invoked when
    # the "Actions->Remove" menu item is chosen.
    # Handles the removal of the selected PDGraph curves.
    #
    proc handle_remove {} {

        # user has chosen to remove the selected curves...

        # ...first find out the selected curves...
        set sellist [.hist.graph getselected]

        # first stop data for the selected curves
        foreach item $sellist {

            rthist_handle_remove_internal $item
        }

        # ...then remove the selected curves from the graph
        .hist.graph remove $sellist
    }


    #
    # handle_smooth - menu item handler, invoked when
    # the Actions->Smooth menu item is chosen.
    # Handles the smoothing of the selected PDGraph curves.
    #
    proc handle_smooth {} {

        # user has chosen to smooth the selected curves...

        # ...first find out the selected curves
        set sellist [.hist.graph getselected]

        # ...then indicate to smooth the selected curves
        .hist.graph smooth $sellist
    }

    #
    # handle_unsmooth - menu item handler, invoked when
    # the Actions->Unsmooth menu item is chosen.
    # Handles the unsmoothing of the selected PDGraph curves.
    #
    proc handle_unsmooth {} {

        # user has chosen to unsmooth the selected curves...

        # ...first find out the selected curves
        set sellist [.hist.graph getselected]

        # ...then indicate to unsmooth the selected curves
        .hist.graph unsmooth $sellist
    }


    #
    # handle_show - menu item handler, invoked when
    # the Actions->Show menu item is chosen.
    # Handles the showing (making visible) of the selected
    # PDGraph curves.
    #
    proc handle_show {} {

        # user has chosen to show (make visible) the selected curves...

        # ...first find out the selected curves
        set sellist [.hist.graph getselected]

        # ...then indicate to show the selected curves
        .hist.graph show $sellist
    }


    #
    # handle_hide - menu item handler, invoked when
    # the Actions->Show menu item is chosen.
    # Handles the hiding of the selected PDGraph curves.
    #
    proc handle_hide {} {

        # user has chosen to hide the selected curves...

        # ...first find out the selected curves
        set sellist [.hist.graph getselected]

        # ...then indicate to hide the selected curves
        .hist.graph hide $sellist
    }

	#
	# update_phase_name - updates the name of the phase as shown 
	# in the status bar
	#
	proc update_phase_name { pname } {
		.hist.statusBarFrame.phaseLabel configure -text "Phase: $pname"
	}
}

