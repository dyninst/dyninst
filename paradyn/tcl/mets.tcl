# mets.tcl
# this file contains procedures used in prompting user for metric/resource
# choices for a visualization.  When the resource hierarchy display is 
# complete, this will be changed and will only be used to get the metric
# choices directly.

# $Log: mets.tcl,v $
# Revision 1.7  1994/10/25 17:55:11  karavan
# Implemented Resource Display Objects, which manage display of multiple
# resource Abstractions.
#
# Revision 1.6  1994/10/09  01:15:28  karavan
# Implemented new UIM/visithread interface with metrespair data structure
# and selection of resources directly on the where axis.
#
# Revision 1.5  1994/09/13  05:06:36  karavan
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


proc acceptMetChoices {} {
    global tclSelectedMetrics tclSelectionState metCount metmenuCB
    if {($tclSelectionState == 1) || ($tclSelectionState == 4)} {
	puts "Please Choose a Focus First"
    } else {
	set tclSelectedMetrics ""
	for {set i 0} {$i < $metCount} {incr i} {
	    if {[expr $metmenuCB($i)] > 0} {
		lappend tclSelectedMetrics $i
	    } 	
	}
	# update state variable to reflect metric choices made; 
	# if focus already chosen, save metric-focus pairs
	if {$tclSelectionState == 2} {
	    incr tclSelectionState 
	    uimpd processVisiSelection $tclSelectedMetrics
	} else { 
	    set tclSelectionState 1
	}
    }
}

proc sendAllSelections {visiToken pdoToken cancelFlag} {
#    acceptMetChoices
#    puts "sendAllSelections $pdoToken"
#    uimpd processResourceSelection $pdoToken
    uimpd sendVisiSelections $visiToken $cancelFlag
}
	
proc getMetsAndRes {metsAndResID rdo} {
    global metCount metList w metMenuCtr tclSelectionState selectionPrompt
    set tclSelectionState 0
    incr metMenuCtr
    set w .metmenunew$metMenuCtr
    set ret 0
    set selectionPrompt "Select Visualization Metric(s) and press ACCEPT"
    set msg2 "Select Visualization Metric(s) and press ACCEPT"
    set msg3 "No Metrics Currently Defined"
    set msg4 "\"Now Accept a Focus on the Where Axis Display\""
    puts "getMetsAndRes $rdo"
    # toplevel window
    toplevel $w  -bd 0
    wm title $w "Paradyn Metrics Menu"
    wm iconname $w "PD Metrics Menu"
    wm geometry $w +425+100
    focus $w

    mkFrame $w.top {top fill expand} -relief raised -border 1
    mkMessage $w.top.msg "" {top expand padx 20 pady 20} \
	    -aspect 1000 -textvariable selectionPrompt \
	    -font -Adobe-times-bold-r-normal--*-120*
    
    if {$metCount == 0} {
	mkMessage $w.top.nometsmsg $msg3 {top expand} \
		-font -Adobe-times-medium-r-normal--*-120*
    } else {
	frame $w.top.1 
	frame $w.top.2
	frame $w.top.3
	if { [expr $metCount % 3] > 0} {
	    set colSize [expr (($metCount/3) + 1)]
	} else {
	    set colSize [expr ($metCount/3)]
	}
	set colNum 1
	set cCnt 1
	for {set i 0} {$i < $metCount} {incr i} {
	    checkbutton $w.top.$colNum.cb$i  -width 20 -anchor w -padx 2 \
		    -variable metmenuCB([expr $i]) \
		    -text [lindex $metList [expr $i]]
	    pack $w.top.$colNum.cb$i -side top
	    if { $cCnt >= $colSize} { 
		incr colNum
		set cCnt 1
	    } else {
		incr cCnt
	    }
	}
	pack $w.top.1 $w.top.2 $w.top.3 -side left -fill both -expand 1
    }

    # buttons

    mkFrame $w.bot {top fill expand} -relief raised -border 1
    button $w.bot.b1 -text "CANCEL" -bg red -fg white\
	    -command  \
	    "sendAllSelections $metsAndResID 1; unset metList; destroy $w"

    button $w.bot.b2 -text "DONE" -bg white \
	    -command \
	    "sendAllSelections $metsAndResID $rdo 0; \
	    unset metList; destroy $w " 

    button $w.bot.b3 -text "ACCEPT" \
	    -command "acceptMetChoices; set selectionPrompt $msg4"
    
    pack $w.bot.b1 -side left -expand yes
    pack $w.bot.b3 -side left -expand yes
    pack $w.bot.b2 -side right -expand yes
}


