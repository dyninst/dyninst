# main tool bar
# $Log: mainMenu.tcl,v $
# Revision 1.1  1994/05/03 06:36:02  karavan
# Initial version.
#

proc drawToolBar {} {

button .b1 -text "Application Control" -command {puts "COMING SOON!"}
button .b2 -text "Performance Consultant" -command {puts "COMING SOON!"}
button .b3 -text "Metric Information" -command {puts "COMING SOON!"}
button .b4 -text "Options Control"  -command {puts "COMING SOON!"}
menubutton .b5 -text "Start Visualization" \
	-menu .b5.m -relief raised
button .b6 -text "Save State" -command {puts "COMING SOON!"}
button .b7 -text "PAUSE" -command {puts "COMING SOON!"}
button .b8 -text "Program Status" -command {puts "COMING SOON!"}
button .b9 -text "EXIT" -bg red -command {destroy .}

menu .b5.m -postcommand {pdUIM_drawStartVisiMenu .b5.m}
wm title . "Paradyn"
pack .b1 .b2 .b3 .b4 .b5 .b6 .b7 .b8 .b9 -side top -fill both -expand yes
}


