# main tool bar


button .b1 -text "Application Control" 
button .b2 -text "Performance Consultant"
button .b3 -text "Metric Information"
button .b4 -text "Options Control"
button .b5 -text "Visualizations"
button .b6 -text "Save State"
button .b7 -text "PAUSE"
button .b8 -text "Program Status"
button .b9 -text "EXIT" -bg red -command {destroy .}

wm title . "Paradyn"
pack .b1 .b2 .b3 .b4 .b5 .b6 .b7 .b8 .b9

bind .b1 <Enter> {puts Hello!}

