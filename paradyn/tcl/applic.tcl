#applic.tcl
# window to get application choices from user
# $Log: applic.tcl,v $
# Revision 1.9  1994/11/03 20:23:17  karavan
# Changes to overall look to fit into paradyn UI.
#
# Revision 1.8  1994/11/03  17:59:38  rbi
# Added a little bit of error handling.
#
# Revision 1.7  1994/11/03  16:10:42  rbi
# New process definition interface.
#
# Revision 1.6  1994/08/23  18:01:11  karavan
# fixed argument parsing to paradyn process command.
#
# Revision 1.5  1994/07/21  17:47:43  rbi
# No more jumpy resizes.
#
# Revision 1.4  1994/07/21  01:53:30  rbi
# YANS -- Yet another new style
#
# Revision 1.3  1994/06/18  20:31:10  hollings
# Changed default host to empty string since the environment variable host is
# not guarnteed to be defined.
#
# Revision 1.2  1994/05/26  21:23:10  karavan
# changed parent window name.
#
# Revision 1.1  1994/05/23  01:56:22  karavan
# initial version.
#

proc DefineProcess {} {
global env
  set W .pDefn
  catch {destroy $W}
  toplevel $W
  wm title $W "Process Defn"
  wm iconname $W "Process Defn"

# define all of the main frames
  set T $W.title
  label $T -text "Define A Process" \
            -anchor center -relief raised \
            -font "-Adobe-times-bold-r-normal--*-120*" 
  set D $W.data
  frame $D
  set B $W.buttons
  frame $B
  pack $T $D $B -side top -expand yes -fill both

#
#  In the data area, each line contains a label and an entry
#  
  frame $D.user -border 2
  label $D.user.lbl -text "User: " -anchor e -width 12
  entry $D.user.ent -width 50 -textvariable applicUser -relief sunken
  $D.user.ent delete 0 end 
  $D.user.ent insert end $env(USER)
  bind $D.user.ent <Tab> "focus $D.machine.ent"
  bind $D.user.ent <Return> "$B.1 invoke"
  pack $D.user -side top -expand yes -fill x
  pack $D.user.lbl $D.user.ent -side left -expand yes -fill x

  frame $D.machine -border 2
  label $D.machine.lbl -text "Machine: " -anchor e -width 12
  entry $D.machine.ent -width 50 -textvariable applicMachine -relief sunken
  bind $D.machine.ent <Tab> "focus $D.daemon.ent"
  bind $D.machine.ent <Return> "$B.1 invoke"
  pack $D.machine -side top -expand yes -fill x
  pack $D.machine.lbl $D.machine.ent -side left -expand yes -fill x

  frame $D.daemon -border 2
  label $D.daemon.lbl -text "Daemon: " -anchor e -width 12
  entry $D.daemon.ent -width 50 -textvariable applicDaemon -relief sunken
  bind $D.daemon.ent <Tab> "focus $D.cmd.ent"
  bind $D.daemon.ent <Return> "$B.1 invoke"
  pack $D.daemon -side top -expand yes -fill x
  pack $D.daemon.lbl $D.daemon.ent -side left -expand yes -fill x

  frame $D.cmd -border 2
  label $D.cmd.lbl -text "Command: " -anchor e -width 12
  entry $D.cmd.ent -width 50 -textvariable applicCmd -relief sunken
  bind $D.cmd.ent <Tab> "focus $D.user.ent"
  bind $D.cmd.ent <Return> "$B.1 invoke"
  pack $D.cmd -side top -expand yes -fill x
  pack $D.cmd.lbl $D.cmd.ent -side left -expand yes -fill x

  mkButtonBar $B {} retVal \
  {{"ACCEPT" {AcceptNewApplicDefn $applicUser $applicMachine \
	  $applicDaemon $applicCmd}} \
  {"CANCEL" {destroy .pDefn}}}

  focus $D.user.ent
}

proc AcceptNewApplicDefn {user machine daemon cmd} {
  set pcmd [list paradyn process]

  if {[string length $user] > 0} {
    lappend pcmd "-user" $user
  }

  if {[string length $machine] > 0} {
    lappend pcmd "-machine" $machine
  }

  if {[string length $daemon] > 0} {
    lappend pcmd "-daemon" $daemon
  }

  set pcmd [concat $pcmd $cmd]
 
  set retval [catch $pcmd]

  if {$retval == 1} {
    set result "Illegal Process Definition"
    eval [list uimpd showError 25 $result]
  }
  destroy .pDefn
}
