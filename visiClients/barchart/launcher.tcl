# launcher.tcl
# introduces a pause (until button is pressed in a certain widget)
# that allows me to start gdb, do a process attachment, etc.
# before running the tcl code of barChart.tcl

button .launch -text "Launch" -command "source barChart.tcl"
pack append . .launch {top}
