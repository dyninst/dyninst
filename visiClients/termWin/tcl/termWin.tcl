#
#  Copyright (c) 1996-2001 Barton P. Miller
#  
#  We provide the Paradyn Parallel Performance Tools (below
#  described as Paradyn") on an AS IS basis, and do not warrant its
#  validity or performance.  We reserve the right to update, modify,
#  or discontinue this software at any time.  We shall have no
#  obligation to supply such updates or modifications or any other
#  form of support to you.
#  
#  This license is for research uses.  For such uses, there is no
#  charge. We define "research use" to mean you may freely use it
#  inside your organization for whatever purposes you see fit. But you
#  may not re-distribute Paradyn or parts of Paradyn, in any form
#  source or binary (including derivatives), electronic or otherwise,
#  to any other organization or entity without our permission.
#  
#  (for other uses, please contact us at paradyn@cs.wisc.edu)
#  
#  All warranties, including without limitation, any warranty of
#  merchantability or fitness for a particular purpose, are hereby
#  excluded.
#  
#  By your use of Paradyn, you understand and agree that we (or any
#  other person or entity with proprietary rights in Paradyn) are
#  under no obligation to provide either maintenance services,
#  update services, notices of latent defects, or correction of
#  defects for Paradyn.
#  
#  Even if advised of the possibility of such damages, under no
#  circumstances shall we (or any other person or entity with
#  proprietary rights in the software licensed hereunder) be liable
#  to you or any third party for direct, indirect, or consequential
#  damages of any character regardless of type of action, including,
#  without limitation, loss of profits, loss of use, loss of good
#  will, or computer failure or malfunction.  You agree to indemnify
#  us (and any other person or entity with proprietary rights in the
#  software licensed hereunder) for any and all liability it may
#  incur to third parties resulting from your use of Paradyn.
#
#
# $Id: termWin.tcl,v 1.4 2003/01/17 19:00:04 willb Exp $
#

#
# default resources for TermWin header
#
option add *Pdtermwin*Header*headerBackground green4 widgetDefault
option add *Pdtermwin*Header*headerForeground white widgetDefault
option add *Pdtermwin*Header*Font "{New Century Schoolbook} 12 bold roman" widgetDefault

#
# default resources for TermWin text area
#
option add *Pdtermwin*TextArea*Font {Courier 10 roman} widgetDefault
option add *Pdtermwin*TextArea*daemonForeground	red	widgetDefault
option add *Pdtermwin*TextArea*appForeground black	widgetDefault




proc getWindowWidth {wName} {
   # warning!  This routine will return an old number if an important
   # event (i.e. resize) happened but idle routines haven't yet kicked in.
   # --> *** In such cases, be sure to grab the latest information directly
   #         from the event structure instead of calling this routine!!!!

   set result [winfo width $wName]
   if {$result == 1} {
      # hack for a window that hasn't yet been mapped
      set result [winfo reqwidth $wName]
   }

   return $result
}

proc getWindowHeight {wName} {
   # warning!  This routine will return an old number if an important
   # event (i.e. resize) happened but idle routines haven't yet kicked in.
   # --> *** In such cases, be sure to grab the latest information directly
   #         from the event structure instead of calling this routine!!!!

   set result [winfo height $wName]
   if {$result == 1} {
      # hack for a window that hasn't yet been mapped
      set result [winfo reqheight $wName]
   }

   return $result
}

#
#  Create the overall frame
#

frame .termwin -class Pdtermwin
pack .termwin -side top -fill both -expand true


#
#  Create the title bar, menu bar, and logo at the top
#
frame .termwin.top -class Header
pack .termwin.top -side top -fill x

frame .termwin.top.left
pack .termwin.top.left -side left -fill both -expand 1

label .termwin.top.left.title -relief raised -text "Application Output" \
      -foreground [option get .termwin.top headerForeground HeaderForeground] \
	  -background [option get .termwin.top headerBackground HeaderBackground]

pack .termwin.top.left.title -side top -fill both -expand true

#
#  Create the menubar as a frame with many menu buttons
#
frame .termwin.top.left.menubar -borderwidth 2 -relief raised
pack .termwin.top.left.menubar -side top -fill x

#
#  File menu
# 
menubutton .termwin.top.left.menubar.file -text "File" -menu .termwin.top.left.menubar.file.m
menu .termwin.top.left.menubar.file.m
.termwin.top.left.menubar.file.m add command -label "Save..." -command {SaveFile }
.termwin.top.left.menubar.file.m add command -label "Close" -command Shutdown

#
#  Option menu
#
menubutton .termwin.top.left.menubar.option -text "Option" -menu .termwin.top.left.menubar.option.m
menu .termwin.top.left.menubar.option.m
.termwin.top.left.menubar.option.m add cascade -label Close -menu .termwin.top.left.menubar.option.m.close
menu .termwin.top.left.menubar.option.m.close
.termwin.top.left.menubar.option.m.close add radio -label "On Paradyn Exit" -variable mode -value paradyn -command "close_mode 0"
.termwin.top.left.menubar.option.m.close add radio -label "Persistent" -variable mode -value persistent -command "close_mode 1"
set mode paradyn

#
#  Help menu
#
menubutton .termwin.top.left.menubar.help -text "Help" -menu .termwin.top.left.menubar.help.m -state disabled
menu .termwin.top.left.menubar.help.m
.termwin.top.left.menubar.help.m add command -label "General" -command "NotImpl"
.termwin.top.left.menubar.help.m add command -label "Context" -command "NotImpl"
#.termwin.top.left.menubar.help.m disable 0
#.termwin.top.left.menubar.help.m disable 1

#
#  Build the menu bar and add to display
#
pack .termwin.top.left.menubar.file -side left -padx 2
pack .termwin.top.left.menubar.option -side left -padx 2
pack .termwin.top.left.menubar.help -side right 

makeLogo .termwin.top.logo paradynLogo raised 2 HotPink4
pack .termwin.top.logo -side right

frame .termwin.textarea -class TextArea
pack  .termwin.textarea -side top -fill both -expand true

text .termwin.textarea.text -borderwidth 2 -width 80 -height 24 -relief sunken \
    -font [option get .termwin.textarea font Font] \
	-yscrollcommand ".termwin.textarea.sb set"

pack .termwin.textarea.text -side left -fill both -expand true
.termwin.textarea.text tag configure paradyn_tag \
	-foreground [option get .termwin.textarea daemonForeground DaemonForeground]
.termwin.textarea.text tag configure app_tag \
	-foreground [option get .termwin.textarea appForeground AppForeground]

scrollbar .termwin.textarea.sb -relief sunken \
	-command ".termwin.textarea.text yview"
pack .termwin.textarea.sb -side right -fill y 




#
# display everything
#
wm title . "Term Win"


#
#  Helper function for "Close" menu option
#
proc Shutdown {} {
  destroy .
}

proc SaveFile {} {
  set content [.termwin.textarea.text get 1.0 end]
  set filename [tk_getSaveFile -parent .termwin.textarea.text -title "Save to File"]
  if {$filename != ""} {
     set fileid [open $filename w+ 0644]
     puts $fileid $content
     close $fileid
  }
}
