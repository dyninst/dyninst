# utilities for UIM tcl functions
# $Log: uimProcs.tcl,v $
# Revision 1.2  1994/05/23 01:55:46  karavan
# its a whole new look for paradyn!
#
# Revision 1.1  1994/05/03  06:36:03  karavan
# Initial version.
#

proc mkEntry {w {pack {top expand fillx}} args} {
        eval entry $w $args
        pack append [winfo parent $w] $w $pack
        return $w
}

proc mkFrame {w {pack {top expand fill}} args} {
        eval frame $w $args
        pack append [winfo parent $w] $w $pack
        return $w
}

proc mkMessage {w {text ""} {pack {top fillx}} args} {
        eval message $w -text \"$text\" $args
        pack append [winfo parent $w] $w $pack
        return $w
}
#----------------------------------------------------------------------------
#  Make a bar of buttons and pack into parent.  Embed the left button in an
#  additional sunken frame to indicaute that it is the default button, and
#  arrange for that button to be invoked as the default action for clicks 
#  and returns inthe dialog.
# **Changed order of every and other command klk**
#----------------------------------------------------------------------------
proc mkButtonBar {w every retval blist} {	
	upvar $retval retv
        set arg [lindex $blist 0]
        focus $w

        set i 1
        foreach arg [lrange $blist 0 end] {
            button $w.$i -text [lindex $arg 0] -width 7 -height 2\
                    -command "$every; [lindex $arg 1]"
            pack append $w $w.$i {left expand padx 20 pady 4}
            set i [expr $i+1]
        }
}


#
#  Make a new dialog toplevel window
#
proc mkDialogWindow {w} {
    catch {destroy $w}
    toplevel $w -class Dialog -bd 0
    wm title $w "Dialog box"
    wm iconname $w "Dialog"
    wm geometry $w +425+300
    grab $w
    focus $w
    return $w
}

#
# a simple help error screen for paradyn
#  the single parameter is the text for error message
#
proc showError {emsg} {

set buttonfg red
set buttonbg white
set w .error
 
mkDialogWindow $w

frame $w.top  -bg red -width 300
pack $w.top -expand yes -fill both
label $w.top.exclaim -bitmap @dont.xbm -bg white -fg black -height 40
pack $w.top.exclaim -pady 5

frame $w.mid -bg red
pack $w.mid -expand yes -fill both
message $w.mid.msg -fg $buttonfg -bg $buttonbg -width 300 -text $emsg
pack $w.mid.msg -expand yes -fill both -padx 5 -pady 5

frame $w.bot -bg red
pack $w.bot -expand yes -fill both
button $w.bot.b0 -text "OK" -bg $buttonbg -fg $buttonfg \
	-activeforeground $buttonfg \
	-activebackground $buttonbg -command {destroy $w} -width 10 
pack $w.bot.b0 -pady 5

}
