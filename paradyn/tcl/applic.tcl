#applic.tcl
# window to get application choices from user
# $Log: applic.tcl,v $
# Revision 1.19  1997/01/15 00:14:58  tamches
# added attach
#
# Revision 1.18  1995/10/30 23:28:21  naim
# Chaning "Machine" by "Host" - naim
#
# Revision 1.17  1995/10/05  04:16:46  karavan
# added error handling for empty command name
#
# Revision 1.16  1995/09/26  20:31:08  naim
# Eliminating error message "eval [list uimpd showError 25 $result]". The idea
# here is to display a more precise error message if there is an error during
# process creation
#
# Revision 1.15  1995/09/18  22:39:49  mjrg
# added directory command.
#
# Revision 1.14  1995/07/19  23:01:16  tamches
# Commented out TAB-key bindings to move between entries on the
# start process dialog, because these bindings are provided
# automagically in tk4.0
#
# Revision 1.13  1995/07/03  03:26:53  karavan
# Changed default for user to blank, workaround for nonstandard rsh in use
# in the CS department.
#
#

#
#  Process definitions depend on several global variables.
#
#  applicDaemon  -- name of the paradyn daemon
#  applicUser    -- user name
#  applicMachine -- machine to use
#  applicCmd     -- command to run (including arguments)
#  applicDir     -- directory to chdir
#
#  It would be nice if there were a better way to specify defaults
#  such as these.
#

#
# this is a strange way to say "if default daemon name
# is not set then set it to defd"
#
if {[catch {set applicDaemon}]} {
  set applicDaemon defd
}

#
#  display a dialog box that prompts for daemon, 
#  user name, machine name, command and arguments
#
#  the user finishes selection, the dialog disappears, and 
#  the new process is started
#
#  TODO -- use the dialog creation routine in uimpd file
#
proc AttachProcess {} {
  global env applicDaemon applicUser applicMachine applicCmd applicDir 

  set W .attachDefn

  # If the window already exists, the following line brings it to the fore.  In other
  # words, if "Attach" is chosen twice from the menu, only one window appears.
  if {[winfo exists $W]} {
     puts stderr "attach: the window already exists...bringing it to fore"

     wm deiconify $W
     raise $W
     return
  }

  if {[winfo exists .pDefn]} {
     puts stderr "attach: the process definition window is already up...killing it"
     destroy .pDefn
  }

  toplevel $W
  wm title $W "Attach"
  wm iconname $W "Attach"

# force the window to a happy location
  set baseGeom [wm geometry .]
  set Xbase 0
  set Ybase 0
  set Xoffset 30
  set Yoffset 30
  scan $baseGeom "%*dx%*d+%d+%d" Xbase Ybase
  wm geometry $W [format "+%d+%d" [expr $Xbase + $Xoffset] \
                                      [expr $Ybase + $Yoffset]]
 
# define all of the main frames
  set T $W.title
  label $T -text "Attach to a Process" \
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
  pack $D.user -side top -expand yes -fill x
  label $D.user.lbl -text "User: " -anchor e -width 12
  pack $D.user.lbl -side left -expand false
  entry $D.user.ent -width 50 -textvariable applicUser -relief sunken
  pack  $D.user.ent -side right -fill x -expand true
  bind $D.user.ent <Return> "$B.1 invoke"

  frame $D.machine -border 2
  pack $D.machine -side top -expand yes -fill x
  label $D.machine.lbl -text "Host: " -anchor e -width 12
  pack $D.machine.lbl -side left -expand false
  entry $D.machine.ent -width 50 -textvariable applicMachine -relief sunken
  pack $D.machine.ent -side right -fill x -expand true
  bind $D.machine.ent <Return> "$B.1 invoke"

  # Does a directory entry make any sense for attach???  Might have some use...
#  frame $D.directory -border 2
#  pack $D.directory -side top -expand yes -fill x
#  label $D.directory.lbl -text "Directory: " -anchor e -width 12
#  pack $D.directory.lbl -side left -expand false
#  entry $D.directory.ent -width 50 -textvariable applicDir -relief sunken
#  pack $D.directory.ent -side right -fill x -expand true
#  bind $D.directory.ent <Return> "$B.1 invoke"
  
  set daemons [paradyn daemons]
  frame $D.daemon -border 2
  label $D.daemon.lbl -text "Daemon: " -anchor e -width 12
  pack $D.daemon -side top -expand yes -fill x
  pack $D.daemon.lbl -side left -expand no -fill x
  foreach d $daemons {
    radiobutton $D.daemon.$d -text $d -variable applicDaemon -value $d \
	-relief flat
    pack $D.daemon.$d -side left -expand yes -fill x
  }
  $D.daemon.$applicDaemon invoke

  frame $D.pid -border 2
  pack  $D.pid -side top -fill x -expand false
  entry $D.pid.entry -textvariable applicPid -relief sunken
  pack  $D.pid.entry -side right -fill x -expand true
  bind  $D.pid.entry <Return> "$B.1 invoke"

  label $D.pid.label -text "pid: " -anchor e -width 12
  pack  $D.pid.label -side left -expand false
  

  mkButtonBar $B {} retVal \
  {{"ATTACH" {AcceptAttachDefn $applicUser $applicMachine \
	  $applicDaemon $applicPid}} \
  {"CANCEL" {destroy .attachDefn}}}

  focus $D.machine.ent
}

proc DefineProcess {} {
  global env applicDaemon applicUser applicMachine applicCmd applicDir 

  set W .pDefn

  # If the window already exists, the following line brings it to the fore.  In other
  # words, if "Attach" is chosen twice from the menu, only one window appears.
  if {[winfo exists $W]} {
     puts stderr "DefineProcess: the window already exists...bringing it to fore"

     wm deiconify $W
     raise $W
     return
  }

  if {[winfo exists .attachDefn]} {
     puts stderr "DefineProcess: the attach definition window is already up...killing it"
     destroy .attachDefn
  }

  toplevel $W
  wm title $W "Process Defn"
  wm iconname $W "Process Defn"

# force the window to a happy location
  set baseGeom [wm geometry .]
  set Xbase 0
  set Ybase 0
  set Xoffset 30
  set Yoffset 30
  scan $baseGeom "%*dx%*d+%d+%d" Xbase Ybase
  wm geometry .pDefn [format "+%d+%d" [expr $Xbase + $Xoffset] \
                                      [expr $Ybase + $Yoffset]]
 
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
  bind $D.user.ent <Return> "$B.1 invoke"
  pack $D.user -side top -expand yes -fill x
  pack $D.user.lbl $D.user.ent -side left -expand yes -fill x

  frame $D.machine -border 2
  label $D.machine.lbl -text "Host: " -anchor e -width 12
  entry $D.machine.ent -width 50 -textvariable applicMachine -relief sunken
  bind $D.machine.ent <Return> "$B.1 invoke"
  pack $D.machine -side top -expand yes -fill x
  pack $D.machine.lbl $D.machine.ent -side left -expand yes -fill x

  frame $D.directory -border 2
  label $D.directory.lbl -text "Directory: " -anchor e -width 12
  entry $D.directory.ent -width 50 -textvariable applicDir -relief sunken
  bind $D.directory.ent <Return> "$B.1 invoke"
  pack $D.directory -side top -expand yes -fill x
  pack $D.directory.lbl $D.directory.ent -side left -expand yes -fill x
  
  set daemons [paradyn daemons]
  frame $D.daemon -border 2
  label $D.daemon.lbl -text "Daemon: " -anchor e -width 12
  pack $D.daemon -side top -expand yes -fill x
  pack $D.daemon.lbl -side left -expand no -fill x
  foreach d $daemons {
    radiobutton $D.daemon.$d -text $d -variable applicDaemon -value $d \
	-relief flat
    pack $D.daemon.$d -side left -expand yes -fill x
  }
  $D.daemon.$applicDaemon invoke

  frame $D.cmd -border 2
  label $D.cmd.lbl -text "Command: " -anchor e -width 12
  entry $D.cmd.ent -width 50 -textvariable applicCmd -relief sunken
  bind $D.cmd.ent <Return> "$B.1 invoke"
  pack $D.cmd -side top -expand yes -fill x
  pack $D.cmd.lbl $D.cmd.ent -side left -expand yes -fill x

  mkButtonBar $B {} retVal \
  {{"ACCEPT" {AcceptNewApplicDefn $applicUser $applicMachine \
	  $applicDaemon $applicDir $applicCmd}} \
  {"CANCEL" {destroy .pDefn}}}

  focus $D.machine.ent
}

#
#  when the user has accepted a new application definition, 
#  we invoke this command to start the process.
#
#  <cmd> is passed directly to execv() with no
#  further parsing or substition
#
#  any errors in process startup are reported through error dialogs
#  (i.e. there is no useful return value from this proc)
#
#  TODO: we must have very specific reporting of process startup
#        failures.  the current error message is useless.
#
proc AcceptNewApplicDefn {user machine daemon directory cmd} {
  set W .pDefn
  set D $W.data

  if {[string length $cmd] == 0} {
      # user forgot to enter a command (program name + args); ring bell
      puts "\a"
      return
  }
  set pcmd [list paradyn process]

  if {[string length $user] > 0} {
    lappend pcmd "-user" $user
  }

  if {[string length $machine] > 0} {
    lappend pcmd "-machine" $machine
  }

  if {[string length $directory] > 0} {
    lappend pcmd "-dir" $directory
  }

  if {[string length $daemon] > 0} {
    lappend pcmd "-daemon" $daemon
  }

  set pcmd [concat $pcmd $cmd]
 
  destroy $W

  # Now execute it!
  set retval [catch $pcmd result]

  if {$retval == 1} {
#    set result "Illegal Process Definition"
    eval [list uimpd showError 25 $result]
  }
}

proc AcceptAttachDefn {user machine daemon pid} {
  set W .attachDefn
  set D $W.data

  if {[string length $pid] == 0} {
      # user forgot to enter a command (program name + args); ring bell
      puts "\a"
      return
  }
  set pcmd [list paradyn attach]

  if {[string length $user] > 0} {
    lappend pcmd "-user" $user
  }

  if {[string length $machine] > 0} {
    lappend pcmd "-machine" $machine
  }

  if {[string length $daemon] > 0} {
    lappend pcmd "-daemon" $daemon
  }

  lappend pcmd "-pid" $pid
 
  destroy $W

puts stderr $pcmd

  # Now execute it!
  set retval [catch $pcmd result]

  if {$retval == 1} {
    eval [list uimpd showError 26 $result]
  }
}
