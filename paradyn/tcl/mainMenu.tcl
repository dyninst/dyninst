# main tool bar
# $Log: mainMenu.tcl,v $
# Revision 1.4  1994/05/23 01:55:41  karavan
# its a whole new look for paradyn!
#
# Revision 1.3  1994/05/06  06:42:13  karavan
# buttons now functional: Performance Consultant; RUN/PAUSE; Status
#
# Revision 1.2  1994/05/05  19:51:24  karavan
# added call to uimpd command for visi menu.
#
# Revision 1.1  1994/05/03  06:36:02  karavan
# Initial version.
#


proc changeApplicState {newVal} {
global PdApplicState

puts \
"changeApplicState called with PdApplicState= $PdApplicState newVal = $newVal"
if {$newVal} {
	if {! $PdApplicState} {
	puts "changing state from 0 to 1"
	set PdApplicState 1
#	.buttons.2 configure -state normal
#	.buttons.1 configure -state disabled
	} 
} elseif {$PdApplicState} {
	puts "changing state from 1 to 0"
	set PdApplicState 0
#	.buttons.2 configure -state disabled
#	.buttons.1 configure -state normal
	}
update
}

proc drawToolBar {} {

global PdApplicState
set PdApplicState 0

wm geometry . =750x750
frame .leftbar -width 50
frame .rightbox
frame .rightbox.menub -relief raised
label .rightbox.menub.logobox -bitmap @logo.xbm -foreground red 
frame .rightbox.where -height 100 
frame .rightbox.main -height 400 -width 400
frame .buttons
mkButtonBar .buttons {} retval {{RUN "paradyn cont"} \
	{PAUSE "paradyn pause"} {REPORT "paradyn status"} {SAVE ""} \
	{EXIT "destroy ."}}
.buttons.2 configure -state disabled

menubutton .rightbox.menub.b1 -text "Setup" -menu .rightbox.menub.b1.m
menubutton .rightbox.menub.b3 -text "Metrics" 
menubutton .rightbox.menub.b5 -text "Start Visual" \
	-menu .rightbox.menub.b5.m
menubutton .rightbox.menub.b6 -text "Help" 
menu .rightbox.menub.b5.m -postcommand \
	{uimpd drawStartVisiMenu .rightbox.menub.b5.m}
menu .rightbox.menub.b1.m 
.rightbox.menub.b1.m add command -label "Application Control" \
	-command ApplicDefn
.rightbox.menub.b1.m add command -label "Start Perf Consultant" \
	-command {paradyn shg start}
.rightbox.menub.b1.m add command -label "Options Control" \
	-state disabled


wm title . "Paradyn"

pack .buttons -side bottom -fill x
#pack .leftbar -side left -expand no -fill y
pack .rightbox -side left -fill both -expand yes
#pack .leftbar.logobox \
#	 .leftbar.b6 .leftbar.b7 .leftbar.b3 .leftbar.b8 .leftbar.b9 \
#	-side top -pady 4
pack .rightbox.where -side bottom -fill x
pack .rightbox.menub .rightbox.main  -side top -fill x
pack .rightbox.menub.logobox -side left -padx 5
pack .rightbox.menub.b6 .rightbox.menub.b3 .rightbox.menub.b5 \
	.rightbox.menub.b1 -side right -padx 10

InitApplicDefnScreen 

}


