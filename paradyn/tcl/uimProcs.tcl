#
# Copyright (c) 1996-2004 Barton P. Miller
# 
# We provide the Paradyn Parallel Performance Tools (below
# described as "Paradyn") on an AS IS basis, and do not warrant its
# validity or performance.  We reserve the right to update, modify,
# or discontinue this software at any time.  We shall have no
# obligation to supply such updates or modifications or any other
# form of support to you.
# 
# This license is for research uses.  For such uses, there is no
# charge. We define "research use" to mean you may freely use it
# inside your organization for whatever purposes you see fit. But you
# may not re-distribute Paradyn or parts of Paradyn, in any form
# source or binary (including derivatives), electronic or otherwise,
# to any other organization or entity without our permission.
# 
# (for other uses, please contact us at paradyn@cs.wisc.edu)
# 
# All warranties, including without limitation, any warranty of
# merchantability or fitness for a particular purpose, are hereby
# excluded.
# 
# By your use of Paradyn, you understand and agree that we (or any
# other person or entity with proprietary rights in Paradyn) are
# under no obligation to provide either maintenance services,
# update services, notices of latent defects, or correction of
# defects for Paradyn.
# 
# Even if advised of the possibility of such damages, under no
# circumstances shall we (or any other person or entity with
# proprietary rights in the software licensed hereunder) be liable
# to you or any third party for direct, indirect, or consequential
# damages of any character regardless of type of action, including,
# without limitation, loss of profits, loss of use, loss of good
# will, or computer failure or malfunction.  You agree to indemnify
# us (and any other person or entity with proprietary rights in the
# software licensed hereunder) for any and all liability it may
# incur to third parties resulting from your use of Paradyn.
#

# $Id: uimProcs.tcl,v 1.23 2004/03/23 19:12:18 eli Exp $
# utilities for UIM tcl functions
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
            button $w.$i -text [lindex $arg 0]  -height 1 \
                    -command "$every; [lindex $arg 1]"
            pack append $w $w.$i {left expand padx 15 pady 4}
            set i [expr $i+1]
        }
}


#
#  Make a new dialog toplevel window
#
proc mkDialogWindow {w} {
    catch {destroy $w}
    toplevel $w -class Paradyn -bd 0
    wm title $w "Dialog box"
    wm iconname $w "Dialog"
    wm geometry $w +425+300
# Under 7.5/4.1, the tkwait causes the window to "flicker"
# noticably.
#    tkwait visibility $w
    catch {grab $w}
    focus $w
    return $w
}

proc mkDialogWindowTitle {w theTitle} {
    catch {destroy $w}
    toplevel $w -class Paradyn -bd 0 
    wm title $w $theTitle
    wm iconname $w $theTitle
    label $w.la -text $theTitle \
	    -foreground white -anchor c \
	    -relief raised \
	    -background red \
	    -width 40
    pack $w.la -side top -fill x 
    catch {grab $w}
    focus $w
    return $w
}

# (re-)set the release identifier in the main window title banner

proc setTitleVersion {release_id} {
    set w .parent.menub.left.top.title.versionFrame.version 
    $w configure -text "$release_id"
}

# present a dialog with Paradyn information
# (based on explError dialog)

proc showMsg {infoCode infoStr} {
    global pdError
    set w .infoDisp$infoCode

    #lookup infoCode, get explanation
    set ehead [lindex $pdError($infoCode) 0]
    set etext [lindex $pdError($infoCode) 3]

    mkDialogWindowTitle $w "Paradyn Information"
    grab release $w                     ;# don't want this dialog to hold focus
    $w configure -bg orange
    $w.la configure -bg orange
    frame $w.out 
    pack $w.out -padx 5 -pady 5 -expand true -fill both

    # title
    ## **** don't forget to use class for this font!!!!
    label $w.out.top -text "Paradyn Information \#$infoCode: $ehead" \
        -fg orange 
    pack $w.out.top -pady 5 -padx 5

    frame $w.out.explain
    pack $w.out.explain -expand yes -fill both -padx 2

    scrollbar $w.out.explain.msgsb -orient vertical \
            -command "$w.out.explain.msg yview" \
   	    -background lightgray -activebackground lightgray
    pack $w.out.explain.msgsb -side right -fill y -expand false

    # explanation text 
    # message $w.out.explain -width 300 -text $etext -relief groove
    text $w.out.explain.msg -wrap word -width 65 -height 6 \
   	    -yscrollcommand "$w.out.explain.msgsb set"
    if {$infoStr != ""} {
        $w.out.explain.msg insert end $infoStr
        $w.out.explain.msg insert end "\n"
    }
    $w.out.explain.msg insert end $etext
    pack $w.out.explain.msg -expand true -fill both

    # single button option
    button $w.out.b0 -text "OK" -command "destroy $w" -width 10 
    pack $w.out.b0 -pady 5
}

proc explError {errorCode} {
    global pdError
    set w .error2$errorCode

    #lookup errorCode, get explanation
    set etext [lindex $pdError($errorCode) 3]

    mkDialogWindowTitle $w "Paradyn Error Explanation"
    $w configure -bg red
    frame $w.out 
    pack $w.out -padx 5 -pady 5 -expand true -fill both

    # title
    ## **** don't forget to use class for this font!!!!
    label $w.out.top -text "Paradyn Message \#\ $errorCode Explanation" \
        -fg red 
    pack $w.out.top -pady 5 -padx 5

    frame $w.out.explain
    pack $w.out.explain -expand yes -fill both -padx 2

    scrollbar $w.out.explain.msgsb -orient vertical \
            -command "$w.out.explain.msg yview" \
   	    -background lightgray -activebackground lightgray
    pack $w.out.explain.msgsb -side right -fill y -expand false

    # explanation text 
    # message $w.out.explain -width 300 -text $etext -relief groove
    text $w.out.explain.msg -wrap word -width 50 -height 4 \
   	    -yscrollcommand "$w.out.explain.msgsb set"
    $w.out.explain.msg insert end $etext
    pack $w.out.explain.msg -expand true -fill both

    # single button option
    button $w.out.b0 -text "OK" -command "destroy $w" -width 10 
    pack $w.out.b0 -pady 5
}

proc showErrorHistory {} {
    global pdErrorHistory
    frame .errorHist
    set w .errorHist
    mkDialogWindow $w
    label $w.title -text "Paradyn Error History"
    frame $w.list
    listbox $w.list.hlist -relief groove 
    scrollbar $w.list.s -orient vert -command "$w.list.hlist yview"
    $w.list.hlist configure -yscrollcommand "$w.list.s set" 
    pack $w.title -side top
    pack $w.list.hlist $w.list.s -side left 
    pack $w.list -side top
    button $w.butt -text "OKAY" -command "destroy $w"
    pack $w.butt -side top
}

#
# a simple help error screen for paradyn
#  errorStr: text for custom error message
#  errorCode: error ID from paradyn error database
#
proc showError {errorCode errorStr} {
    global pdError pdErrorHistory
    global numErrorsShown
    global whichDefaultErrorsShown

    set w .paradynErrorWindow
    set windowOpened [winfo exists $w]
    if {!$windowOpened} {
       if {[array exists whichDefaultErrorsShown]} {
	  unset whichDefaultErrorsShown
       }
    }

    # If "errorStr" is empty and whichDefaultErrorsShown() says that a
    # default msg for this error code is already up, then we do nothing
    if {$errorStr == ""} {
       if {[array exists whichDefaultErrorsShown]} {
          if {[llength [array get whichDefaultErrorsShown $errorCode]]!=0} {
             return
	  }
       }
    }
    set whichDefaultErrorsShown($errorCode) true
   
    set retval [catch {set errRec $pdError($errorCode)}]

    if {$retval == 1} {
	set errorStr "No entry in error database for this error code."
	set etype serious
    } else {
	set etype [lindex $errRec 2]
        if {$etype == "information"} {
            # Consider informational messages separately from errors
            showMsg $errorCode $errorStr
            return
        }
	if {$errorStr == ""} {
	    # No error string was passed in to this routine, so use
            # the default one located in the database.
	    set errorStr [lindex $errRec 0]
	}
    }

    
    # If the main error window isn't already opened, then open it.
    set theText $w.out.mid.msg

    if {!$windowOpened} {
       set numErrorsShown 0
       
       mkDialogWindowTitle $w "Paradyn Error Window"
       $w configure -bg red
       frame $w.out
       pack $w.out -padx 5 -pady 5 -fill both -expand true

       # Error screen header: bitmap, title and Error Number
       frame $w.out.top
       pack $w.out.top -padx 5 -pady 5 -fill both -expand false

       # specific error message text
       frame $w.out.mid
       pack $w.out.mid -expand yes -fill both  -padx 5

       scrollbar $w.out.mid.msgsb -orient vertical -command "$w.out.mid.msg yview" \
   	    -background lightgray -activebackground lightgray
       pack $w.out.mid.msgsb -side right -fill y -expand false

       text $theText -wrap word \
   	    -yscrollcommand "$w.out.mid.msgsb set" \
   	    -height 8 -width 50
       pack $theText -fill both -expand true

       # option buttons 
       frame $w.out.buttons 
       mkButtonBar $w.out.buttons {} retval {{Continue ""} \
	    {Exit PARADYN "destroy ."} }

       #$w.out.buttons.2 configure -command "errorExit $w"
       $w.out.buttons.2 configure -command "procExit"
       $w.out.buttons.1 configure -command "destroy $w"
       pack $w.out.buttons -fill both -padx 5 -expand false

       $theText tag configure categoryTag 
       $theText tag configure errorPrefixTag -foreground red
                
    } else {
       #puts stderr "window already up"
       #flush stderr

       # Since the window is already up, at least one error is already
       # being shown.  Hence, we want to insert a newline now to put some vertical
       # space between us and the error above us
       $theText insert end "\n"
    }

    incr numErrorsShown

    # Now insert the information for this specific error code:
    makeLogo $w.logo$numErrorsShown dont flat 0 red
    $theText window create end -padx 5 -pady 5 -window $w.logo$numErrorsShown

    $theText insert end "Paradyn message #$errorCode" errorPrefixTag
    $theText insert end "  (category: $etype) " categoryTag

    button $w.explain$numErrorsShown -text "Explain..." \
	    -command "explError $errorCode" \
	    -relief groove
    $theText window create end -padx 5 -pady 5 -window $w.explain$numErrorsShown
    $theText insert end "\n"

    $theText insert end "$errorStr\n"
    
    # add this error to error history list
    lappend pdErrorHistory [list $errorCode $errorStr]
}

# Exit Paradyn, with or without core file per the user selection
# This is only called from an error condition.
#
proc errorExit {oldwin} {
    set w .exerror

    mkDialogWindowTitle $w "Exit Paradyn"
    label $w.l -text "Generate Core File (Y/N)?"
    frame $w.buttons
    mkButtonBar $w.buttons {} retval {{YES ""} {NO ""}}
    $w.buttons.1 configure -command "paradyn core -1; destroy ."
    $w.buttons.2 configure -command "destroy ."
    destroy $oldwin
    pack $w.l $w.buttons -side top -padx 10 -pady 10 
    focus $w
}

# Makes sure that the user wants to exit paradyn
#
proc procExit {} {
    set w .exitWindow
    mkDialogWindowTitle $w "Exit Paradyn"
    frame $w.fr -borderwidth 2
    pack $w.fr -side top
    label $w.fr.l -text "Are you sure?"
    pack $w.fr.l -side top -pady 10

    frame $w.fr.buttons
    mkButtonBar $w.fr.buttons {} retval {{Yes ""} {No ""}}
    $w.fr.buttons.1 configure -command "destroy ."
    $w.fr.buttons.2 configure -command "destroy $w"
    pack $w.fr.buttons -side top -fill both

    focus $w
}
