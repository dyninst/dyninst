#applic.tcl
# window to get application choices from user
# $Log: applic.tcl,v $
# Revision 1.2  1994/05/26 21:23:10  karavan
# changed parent window name.
#
# Revision 1.1  1994/05/23  01:56:22  karavan
# initial version.
#

## initialize application defn screen, without displaying it yet

proc InitApplicDefnScreen {} {

global ADparent PdNumDefinedProcesses definedProcesses applicCmd \
	env ADshow ADapplic applicDaemon

set newdefinedProcesses ""
set PdNumDefinedProcesses 0
set ADparent .main
set ADshow $ADparent.show
set ADapplic $ADparent.applic

for {set i 0} {$i < 12} {incr i} {
	set definedProcesses($i) ""
}

########## screen 1 accepts new process defn from user #######

frame $ADparent.applic
frame $ADparent.applic.top -relief raised
frame $ADparent.applic.bot -relief raised

frame $ADparent.applic.top.user -border 2
frame $ADparent.applic.top.machine -border 2
frame $ADparent.applic.top.daemon -border 2
frame $ADparent.applic.top.cmd -border 2
label $ADparent.applic.title -text "Application Definition Entry"

# single-line user, machine, daemon, command  entries

label $ADparent.applic.top.user.lbl -anchor w -text "User: " \
	-anchor e -width 12
label $ADparent.applic.top.machine.lbl -anchor w -text "Machine: " \
	-anchor e -width 12
label $ADparent.applic.top.daemon.lbl -anchor w -text "Daemon: " \
	-anchor e -width 12
label $ADparent.applic.top.cmd.lbl -anchor w -text "Command: " \
	-anchor e -width 12

entry $ADparent.applic.top.user.ent -width 80 -textvariable applicUser \
	-relief sunken
$ADparent.applic.top.user.ent insert end $env(USER)
bind $ADparent.applic.top.user.ent <Return> \
	"focus $ADparent.applic.top.machine.ent"
entry $ADparent.applic.top.machine.ent -width 80 -textvariable applicMachine \
	-relief sunken
$ADparent.applic.top.machine.ent insert end $env(HOST)
bind $ADparent.applic.top.machine.ent <Return> \
	"focus $ADparent.applic.top.daemon.ent"

entry $ADparent.applic.top.daemon.ent -width 80 -textvariable applicDaemon \
	-relief sunken
bind $ADparent.applic.top.daemon.ent <Return> \
	"focus $ADparent.applic.top.cmd.ent"
entry $ADparent.applic.top.cmd.ent -width 80 -textvariable applicCmd \
	-relief sunken
bind $ADparent.applic.top.cmd.ent <Return> \
	"focus $ADparent.applic.top.user.ent"

pack $ADparent.applic.title $ADparent.applic.top $ADparent.applic.bot \
	-side top -expand yes -fill both
pack $ADparent.applic.top.user $ADparent.applic.top.machine  \
	$ADparent.applic.top.daemon \
	$ADparent.applic.top.cmd -side top \
	 -expand yes -fill both

pack $ADparent.applic.top.user.lbl $ADparent.applic.top.user.ent \
	-side left -expand yes -fill x
pack $ADparent.applic.top.machine.lbl $ADparent.applic.top.machine.ent \
	-side left -expand yes -fill x
pack $ADparent.applic.top.daemon.lbl $ADparent.applic.top.daemon.ent \
	-side left -expand yes -fill x
pack $ADparent.applic.top.cmd.lbl $ADparent.applic.top.cmd.ent \
	-side left -expand yes -fill x

# add/accept/cancel/done

button $ADparent.applic.bot.b1 -text "Accept" \
	-command {AcceptNewApplicDefn $applicUser $applicMachine \
		$applicDaemon $applicCmd definedProcesses}
button $ADparent.applic.bot.b2 -text "Cancel" \
	-command {pack forget $ADparent.applic; pack $ADparent.show}

pack $ADparent.applic.bot.b1 $ADparent.applic.bot.b2 -side left \
	-fill x -padx 10 

############ screen 0 displays defined applications ############3

frame $ADparent.show

label $ADparent.show.title -text "Paradyn Process Definition" \
	-anchor center -relief ridge
frame $ADparent.show.top
frame $ADparent.show.bottom

button $ADparent.show.bottom.b1 -text "Add Process" -command {AddProcess}
button $ADparent.show.bottom.b2 -text "DONE" \
	-command {pack forget $ADparent.show}

label $ADparent.show.top.lbl -text "Currently Defined Processes:" \
	-anchor w -width 84

listbox $ADparent.show.top.alist -geometry 80x12 -bg white

pack $ADparent.show.title $ADparent.show.top $ADparent.show.bottom \
	-side top -expand yes -fill both 
pack $ADparent.show.top.lbl -side top -pady 5
pack $ADparent.show.top.alist -side top -fill both -expand yes 
pack $ADparent.show.bottom.b1 $ADparent.show.bottom.b2 -side left \
	-fill both -padx 10

}

### display application definition screen 0, already defined, on frame

proc ApplicDefn {} {

global ADshow
pack $ADshow
}

proc AddProcess {} {
global applicCmd
global ADparent PdNumDefinedProcesses
if {$PdNumDefinedProcesses < 6} {
	set applicCmd ""
	pack forget $ADparent.show	
	pack $ADparent.applic
	focus $ADparent.applic.top.daemon.ent
}
}

proc AcceptNewApplicDefn {user machine daemon cmd defined} {
global PdNumDefinedProcesses ADparent applicDaemon applicCmd

upvar $defined dlist
set retval [catch \
	{paradyn process -user $user -machine $machine -daemon $daemon $cmd}]

if {$retval == 1} {
	focus $ADparent.applic.top.daemon.ent
} else {
	incr PdNumDefinedProcesses

	$ADparent.show.top.alist insert end $user:$machine:$daemon
	$ADparent.show.top.alist insert end \ \ $cmd
	pack forget $ADparent.applic
	pack $ADparent.show
}
#reset daemon and cmd
set applicDaemon ""
set applicCmd ""

}
