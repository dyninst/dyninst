# main tool bar
# $Log: mainMenu.tcl,v $
# Revision 1.5  1994/05/26 20:56:36  karavan
# changed location of bitmap file for logo.
#
# Revision 1.4  1994/05/23  01:55:41  karavan
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

global PdApplicState PdMainBgColor PdBitmapDir
set PdApplicState 0
set PdMainBgColor "#fb63e620d36b"

wm geometry . =750x750
frame .menub -relief raised -bg $PdMainBgColor
label .menub.logobox -bitmap @$PdBitmapDir/logo.xbm -foreground #b3331e1b53c7 \
	-background $PdMainBgColor
frame .where -height 100 
frame .main -height 400 -width 400 -bg $PdMainBgColor
frame .buttons -bg $PdMainBgColor
mkButtonBar .buttons {} retval {{RUN "paradyn cont"} \
	{PAUSE "paradyn pause"} {REPORT "paradyn status"} {SAVE ""} \
	{EXIT "destroy ."}}
.buttons.2 configure -state disabled

menubutton .menub.b1 -text "Setup" -menu .menub.b1.m -bg $PdMainBgColor
menubutton .menub.b3 -text "Metrics" -bg $PdMainBgColor
menubutton .menub.b5 -text "Start Visual" \
	-menu .menub.b5.m -bg $PdMainBgColor
menubutton .menub.b6 -text "Help" -bg $PdMainBgColor
menu .menub.b5.m -postcommand \
	{uimpd drawStartVisiMenu .menub.b5.m}
menu .menub.b1.m 
.menub.b1.m add command -label "Application Control" \
	-command ApplicDefn
.menub.b1.m add command -label "Start Perf Consultant" \
	-command {paradyn shg start}
.menub.b1.m add command -label "Options Control" \
	-state disabled


wm title . "Paradyn"

pack .buttons -side bottom -fill x
pack .where -side bottom -fill x
pack .menub .main  -side top -fill x
pack .menub.logobox -side left -padx 5
pack .menub.b6 .menub.b3 .menub.b5 \
	.menub.b1 -side right -padx 10

InitApplicDefnScreen 

}


