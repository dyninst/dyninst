# main tool bar
# $Log: mainMenu.tcl,v $
# Revision 1.3  1994/05/06 06:42:13  karavan
# buttons now functional: Performance Consultant; RUN/PAUSE; Status
#
# Revision 1.2  1994/05/05  19:51:24  karavan
# added call to uimpd command for visi menu.
#
# Revision 1.1  1994/05/03  06:36:02  karavan
# Initial version.
#

proc PdPause {butt} {
global PdRunState
if {$PdRunState == 1} {
 set PdRunState 0
 $butt configure -text "RUN"
 paradyn pause
} else {
 set PdRunState 1
 $butt configure -text "PAUSE"
 paradyn cont
}
}

proc drawToolBar {} {

global PdRunState
set PdRunState 0
button .b1 -text "Application Control" -command {puts "COMING SOON!"}
button .b2 -text "Performance Consultant" -command {paradyn shg start}
button .b3 -text "Metric Information" -command {puts "COMING SOON!"}
button .b4 -text "Options Control"  -command {puts "COMING SOON!"}
menubutton .b5 -text "Start Visualization" \
	-menu .b5.m -relief raised
button .b6 -text "Save State" -command {puts "COMING SOON!"}
button .b7 -text "RUN" -command {PdPause .b7}
button .b8 -text "Program Status" -command {paradyn status}
button .b9 -text "EXIT" -bg red -command {destroy .}

menu .b5.m -postcommand {uimpd drawStartVisiMenu .b5.m}
wm title . "Paradyn"
pack .b1 .b2 .b3 .b4 .b5 .b6 .b7 .b8 .b9 -side top -fill both -expand yes
}


