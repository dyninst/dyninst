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
#---------------------------------------------------------------------------
#
# pdgraph.tcl
#
# Tcl portion of the implementation of the PDGraph megawidget.  The bulk
# of the implementation of the megawidget is contained in the implementation
# of subwidgets representing the time axis, the value axis, and the data
# window.  The legend subwidget is implemented mostly in Tcl using a
# canvas widget.
#
#---------------------------------------------------------------------------
# $Id: pdgraph.tcl,v 1.3 2004/03/23 01:12:48 eli Exp $
#---------------------------------------------------------------------------


namespace eval PDGraph {

    # 
    # init - initialize the PDGraph megawidget.
    # handles creation of subwidgets and packing into the given frame.
    #
    proc init { wname } {

        # build the sub-widgets for the PDGraph
        canvas $wname.legend -height 4
        pdgraph_valueaxis $wname.valaxis -width 60
        pdgraph_timeaxis $wname.timeaxis -height 50
        pdgraph_data $wname.data 

        # perform any additional initialization needed
        ::PDGraph::Legend::init $wname.legend

        # pack the sub-widgets as desired
        pack $wname.legend -side bottom -fill x -expand false
        pack $wname.valaxis -side left -fill y -expand false
        pack $wname.timeaxis -side bottom -fill x -expand false
        pack $wname.data -side bottom -fill both -expand true
    }


    #------------------------------------------------------------------------
    # PDGraph::Legend implementation
    #------------------------------------------------------------------------
    namespace eval Legend {

        variable legendFont

        #
        # init - initialize the legend widget
        #
        proc init { legendName } {

            # bind a handler for mouse clicks outside of any item
            bind $legendName <ButtonPress> {::PDGraph::Legend::handle_click %W}
        }

        proc init_font { wname } {
            
            set ::PDGraph::Legend::legendFont [$wname cget -font]
        }


        #
        # add_item - add a line and label for a new curve
        # handles the adjustment of the legend window height
        #
        proc add_item { legendName curveIndex curveId \
                        metricName lineColor lineStipple } {

            # build a label for metric description
            set labName $legendName.label$curveId
            label $labName -text $metricName -font $::PDGraph::Legend::legendFont

            # bind routines to respond to mouse behavior over the label
            bind $labName <Enter> {::PDGraph::Legend::handle_enter_item %W}
            bind $labName <Leave> {::PDGraph::Legend::handle_leave_item %W}
            bind $labName <ButtonPress> \
                {::PDGraph::Legend::handle_click_item %W}

            # expand the size of our window for the new items
            set labHeight [winfo reqheight $labName]
            $legendName configure -height [expr [$legendName cget -height] \
                + $labHeight]

            # determine y location for new items
            set yloc [expr ($curveIndex * $labHeight) + ($labHeight / 2) + 4]

            # place the label
            $legendName create window 50 $yloc -window $labName \
                -anchor w -tags tagCurve$curveId

            # place the line describing the curve
            if { $lineStipple != "solid" } {

                $legendName create line 10 $yloc 40 $yloc \
                    -fill $lineColor -stipple $lineStipple \
                    -tags tagCurve$curveId
            } else {

                $legendName create line 10 $yloc 40 $yloc \
                    -fill $lineColor -tags tagCurve$curveId
            }
        }


        #
        # remove_item - remove a line and label for the given curve
        # handles the adjustment of the legend window height
        #
        proc remove_item { legendName curveId } {

            # find the label to be removed
            set labName $legendName.label$curveId

            # determine the height and y position of the label item
            set labHeight [winfo height $labName]
            set labYLoc [lindex [$legendName bbox tagCurve$curveId] 1]

            # remove the items related to the specified curve
            $legendName delete tagCurve$curveId
            destroy $legendName.label$curveId

            set childBaseNameLen [string length $legendName.label]

            # shift any remaining items below the removed items upward
            foreach child [winfo children $legendName] {

                # only consider the label items for simplicity
                # (since the label and the line are aligned horizontally,
                # and we can move both at once since they share the
                # same tag value)
                if { [winfo class $child] == "Label" } {
                
                    # extract curve id from the label's pathname
                    set cid [string range $child $childBaseNameLen end]
                    
                    # determine if item should be moved
                    set childYLoc [lindex [$legendName bbox tagCurve$cid] 1]
                    if { $childYLoc > $labYLoc } {

                        # shift the items up by the height of the removed item
                        $legendName move tagCurve$cid 0 -$labHeight
                    }
                }
            }

            # reduce the height of our window to reflect the removal
            $legendName configure -height [expr [$legendName cget -height] \
                - $labHeight]
        }


        #
        # handle_enter_item - called when mouse enters a legend item
        # currently, highlights the item with groove relief (unless
        # it is already selected)
        #
        proc handle_enter_item {wname} {

            # highlight widget, unless it is already selected
            if { [$wname cget -relief] != "sunken" } {
                $wname configure -relief groove
            }
        }

        #
        # handle_leave_item - called when mouse exits a legend item
        # currently, unhighlights the item with groove relief (unless
        # it is already selected)
        #
        proc handle_leave_item {wname} {

            # unhighlight widget if it was highlighted
            if { [$wname cget -relief] == "groove" } {
                $wname configure -relief flat
            }
        }

        #
        # handle_click_item - called when mouse clicked on item
        # if item was not selected, selects item (visually, draws
        # it with a sunken relief)
        # if item was already selected, deselects item
        #
        proc handle_click_item {wname} {

            if { [$wname cget -relief] == "sunken" } {
                
                # item was already selected -
                # deselect it
                $wname configure -relief groove

            } else {

                # item was not already selected
                # select it
                $wname configure -relief sunken
            }
        }


        #
        # handle_click - called when mouse is clicked in legend
        # but outside of any item  - deselects all selected items
        #
        proc handle_click {legendName} {
            
            # deselect all items in the legend
            foreach child [winfo children $legendName] {
                
                # deselect only the label items
                if { [winfo class $child] == "Label" } {
                    
                    $child configure -relief flat
                }
            }
        }


        #
        # get_selected - builds a list of curve id of all selected items
        # 
        proc get_selected { legendName } {

            set retval [list]
            set childBaseNameLen [string length $legendName.label]

            foreach child [winfo children $legendName] {

                # only consider the label items
                if { [winfo class $child] == "Label" } {

                    # determine it the label is selected
                    if { [$child cget -relief] == "sunken" } {

                        # extract curve id from the label's pathname
                        set cid [string range $child $childBaseNameLen end]

                        # add the curve id to the list of selected items
                        lappend retval $cid
                    }
                }
            }

            return $retval
        }


        #
        # update_item_name - updates the name associated with 
        # an item
        #
        proc update_item_name { legendName curveId name } {

            set labName $legendName.label$curveId
            $labName configure -text $name
        }
    }
}


