# mets.tcl
# this file contains procedures used in prompting user for metric/resource
# choices for a visualization.  When the resource hierarchy display is 
# complete, this will be changed and will only be used to get the metric
# choices directly.

# $Log: mets.tcl,v $
# Revision 1.5  1994/09/13 05:06:36  karavan
# changes to accommodate multiple simultaneous menus.
#
# Revision 1.4  1994/07/21  01:53:33  rbi
# YANS -- Yet another new style
#
# Revision 1.3  1994/07/15  04:16:25  hollings
# Made the resource entry a useful size.
#
# Revision 1.2  1994/05/07  23:25:43  karavan
# added msg if no metrics defined
#
# Revision 1.1  1994/05/07  18:11:43  karavan
# initial version
#

# bldMetResult
# constructs a list of chosen metrics, based on the variable values for 
# the metric checkbuttons when the user selected "OK"

proc bldMetResult { msgID mList mCount resList} {
	global metmenuCB
	set newList ""
	for {set i 0} {$i < $mCount} {incr i} {
		if {[expr $metmenuCB($i)] > 0} {
		   lappend newList [lindex $mList $i]
         	} 	
        }
       uimpd gotMetrics $msgID $newList $i $resList
}

proc getMetsAndRes {metsAndResID} {
	global metCount metList w metMenuCtr
	set resList ""
    incr metMenuCtr
	set w .metmenunew$metMenuCtr
	set ret 0
	set msg1 "Select Visualization Metric(s)"
	set msg2 "Enter Focus:"
	set msg3 "No Metrics Currently Defined"

    # toplevel window
    toplevel $w  -bd 0
    wm title $w "Paradyn Metrics Menu"
    wm iconname $w "PD Metrics Menu"
    wm geometry $w +425+300
    focus $w

    mkFrame $w.top {top fill expand} -relief raised -border 1
    mkMessage $w.top.msg $msg1 {top expand padx 20 pady 20} \
	    -aspect 1000 -font -Adobe-times-bold-r-normal--*-120*

	if {$metCount == 0} {
	  mkMessage $w.top.nometsmsg $msg3 {top expand} \
		-font -Adobe-times-medium-r-normal--*-120*
        } else {
	 for {set i 0} {$i < $metCount} {incr i} {
	  checkbutton $w.top.cb$i  -width 20 -anchor w -padx 2 \
	    -variable metmenuCB([expr $i]) \
	    -text [lindex $metList [expr $i]]
	  pack $w.top.cb$i -side top
         }
        }

	# typed, text focus selection
        mkFrame $w.mid {top fill expand} -relief raised -border 1
        mkMessage $w.mid.msg $msg2 {top expand padx 20 pady 20} \
                -aspect 1000 -font -Adobe-times-medium-r-normal--*-140*

        set f [mkEntry $w.mid.file {top expand padx 20 pady 20} -textvar \
		resList -bg white -borderwidth 2 -width 80]

        mkFrame $w.bot {top fill expand} -relief raised -border 1
	button $w.bot.b1 -text "Cancel" -command {destroy $w} -bg red -fg white

	# buttons
     button $w.bot.b2 -text "Accept" \
       -command \
       "bldMetResult $metsAndResID \$metList \$metCount \$resList; destroy $w" \
	-bg white
	pack $w.bot.b1 -side left -expand yes
	pack $w.bot.b2 -side right -expand yes
}


