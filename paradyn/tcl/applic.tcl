# $Id: applic.tcl,v 1.23 2004/03/20 20:44:49 pcroth Exp $
# window to get application choices from user

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

  toplevel $W -class Paradyn
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
            -anchor center -relief raised
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
  label $D.user.lbl -text "User: " -anchor e -width 14
  pack $D.user.lbl -side left -expand false
  entry $D.user.ent -width 50 -textvariable applicUser -relief sunken
  pack  $D.user.ent -side right -fill x -expand true
  bind $D.user.ent <Return> "$B.1 invoke"

  frame $D.machine -border 2
  pack $D.machine -side top -expand yes -fill x
  label $D.machine.lbl -text "Host: " -anchor e -width 14
  pack $D.machine.lbl -side left -expand false
  entry $D.machine.ent -width 50 -textvariable applicMachine -relief sunken
  pack $D.machine.ent -side right -fill x -expand true
  bind $D.machine.ent <Return> "$B.1 invoke"

  # Does a directory entry make any sense for attach???  Probably not...it's only
  # useful in the non-attach case to remove the need to use full-path-names for
  # arguments, input files, etc.  Here, we don't use that.

#  frame $D.directory -border 2
#  pack  $D.directory -side top -expand yes -fill x
#  label $D.directory.lbl -text "Directory: " -anchor e -width 14
#  pack  $D.directory.lbl -side left -expand false
#  entry $D.directory.ent -width 50 -textvariable applicDir -relief sunken
#  pack  $D.directory.ent -side right -fill x -expand true
#  bind  $D.directory.ent <Return> "$B.1 invoke"

  frame $D.command -border 2
  pack  $D.command -side top -fill x -expand false
  entry $D.command.entry -textvariable applicCommand -relief sunken
  pack  $D.command.entry -side right -fill x -expand true
  bind  $D.command.entry <Return> "$B.1 invoke"

  label $D.command.label -text "Executable file: " -anchor e -width 14
  pack  $D.command.label -side left -expand false
  

  frame $D.pid -border 2
  pack  $D.pid -side top -fill x -expand false
  entry $D.pid.entry -textvariable applicPid -relief sunken
  pack  $D.pid.entry -side right -fill x -expand true
  bind  $D.pid.entry <Return> "$B.1 invoke"

  label $D.pid.label -text "Pid: " -anchor e -width 14
  pack  $D.pid.label -side left -expand false


  set daemons [paradyn daemons]
  frame $D.daemon -border 2
  label $D.daemon.lbl -text "Daemon: " -anchor e -width 14
  pack $D.daemon -side top -expand yes -fill x
  pack $D.daemon.lbl -side left -expand no -fill x
  foreach d $daemons {
    radiobutton $D.daemon.$d -text $d -variable applicDaemon -value $d \
	-relief flat
    pack $D.daemon.$d -side left -expand yes -fill x
  }
  $D.daemon.$applicDaemon invoke


  # user interface tips:
  frame $D.tips
  pack  $D.tips -side top -fill x

  frame $D.tips.1
  pack  $D.tips.1 -side top -fill x

  label $D.tips.1.label -text "Entering a pid is mandatory." \
        -justify left
  pack  $D.tips.1.label -side left

  frame $D.tips.2
  pack  $D.tips.2 -side top -fill x
  
  label $D.tips.2.label -justify left \
	  -text "Enter the full path to the executable in 'Executable file'.\
                It will be used just to parse the symbol table.\n\
                Paradyn tries to determine this information automatically,\
                so you can usually leave 'Executable file' blank."
  pack  $D.tips.2.label -side left -fill x



  frame $D.run -border 2
  pack  $D.run -side top -fill x

  label $D.run.label -text "After attaching: " -justify left 
  pack  $D.run.label -side left

  global afterAttaching
  set afterAttaching 0

  frame $D.run.fr
  pack  $D.run.fr -ipady 2 -pady 4 -side left -fill x -expand true

  radiobutton $D.run.fr.1 -text "Pause application" -variable afterAttaching \
	  -value 1 -justify left -relief groove -highlightthickness 0
  pack $D.run.fr.1 -side left -fill x -expand true

  radiobutton $D.run.fr.2 -text "Run application" -variable afterAttaching \
	  -value 2 -justify left -relief groove -highlightthickness 0
  pack $D.run.fr.2 -side left -fill x -expand true

  radiobutton $D.run.fr.3 -text "Leave as is" -variable afterAttaching \
	  -value 0 -justify left -relief groove -highlightthickness 0
  pack $D.run.fr.3 -side left -fill x -expand true



  mkButtonBar $B {} retVal \
  {{"Attach" {AcceptAttachDefn $applicUser $applicMachine \
	  $applicCommand $applicPid $applicDaemon $afterAttaching}} \
  {"Cancel" {destroy .attachDefn}}}

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

  toplevel $W -class Paradyn
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
            -anchor center -relief raised
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
  {{"Accept" {AcceptNewApplicDefn $applicUser $applicMachine \
	  $applicDaemon $applicDir $applicCmd}} \
  {"Cancel" {destroy .pDefn}}}

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

proc AcceptAttachDefn {user machine cmd pid daemon afterAttach} {
  set W .attachDefn

  if {[string length $cmd] == 0 && [string length $pid] == 0} {
      # must enter at least one of (cmd, pid); ring bell
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

  if {[string length $cmd] > 0} {
     lappend pcmd "-command" $cmd
  }

  if {[string length $pid] > 0} {
     lappend pcmd "-pid" $pid
  }

  if {[string length $daemon] > 0} {
    lappend pcmd "-daemon" $daemon
  }

  if {[string length $afterAttach]} {
    lappend pcmd "-afterattach" $afterAttach
    # 0 --> leave as is; 1 --> pause; 2 --> run
  }

  destroy $W

#puts stderr $pcmd

  # Now execute it!
  set retval [catch $pcmd result]

  if {$retval == 1} {
    eval [list uimpd showError 26 $result]
  }
}
