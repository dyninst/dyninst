
option add *Visi*font *-New*Century*Schoolbook-Bold-R-*-18-*
option add *Data*font *-Helvetica-*-r-*-12-* 
option add *MyMenu*font *-New*Century*Schoolbook-Bold-R-*-14-*
option add *title*font *-New*Century*Schoolbook-Bold-R-*-18-*
option add *phaseName*font *-New*Century*Schoolbook-Bold-R-*-14-*
option add *phaseStart*font *-New*Century*Schoolbook-Bold-R-*-14-*
option add *phaseEnd*font *-New*Century*Schoolbook-Bold-R-*-14-*

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

#
#  Create the title bar, menu bar, and logo at the top
#
frame .top
pack .top -side top -fill x

frame .top.left 
pack .top.left -side left -fill both -expand 1

label .top.left.title -relief raised -text "Application Output" \
      -foreground white -background Green4

pack .top.left.title -side top -fill both -expand true

#
#  Create the menubar as a frame with many menu buttons
#
frame .top.left.menubar -class MyMenu -borderwidth 2 -relief raised
pack .top.left.menubar -side top -fill x

#
#  File menu
# 
menubutton .top.left.menubar.file -text "File" -menu .top.left.menubar.file.m
menu .top.left.menubar.file.m
.top.left.menubar.file.m add command -label "Save" -command {SaveFile }
.top.left.menubar.file.m add command -label "Close" -command Shutdown

#
#  Option menu
#
menubutton .top.left.menubar.option -text "Option" -menu .top.left.menubar.option.m
menu .top.left.menubar.option.m
.top.left.menubar.option.m add cascade -label Close -menu .top.left.menubar.option.m.close
menu .top.left.menubar.option.m.close
.top.left.menubar.option.m.close add radio -label "On Paradyn Exit" -variable mode -value paradyn -command "close_mode 0"
.top.left.menubar.option.m.close add radio -label "Persistant" -variable mode -value persistant -command "close_mode 1"
set mode paradyn

#
#  Help menu
#
menubutton .top.left.menubar.help -text "Help" -menu .top.left.menubar.help.m -state disabled
menu .top.left.menubar.help.m
.top.left.menubar.help.m add command -label "General" -command "NotImpl"
.top.left.menubar.help.m add command -label "Context" -command "NotImpl"
#.top.left.menubar.help.m disable 0
#.top.left.menubar.help.m disable 1

#
#  Build the menu bar and add to display
#
pack .top.left.menubar.file -side left -padx 2
pack .top.left.menubar.option -side left -padx 2
pack .top.left.menubar.help -side right 

makeLogo .top.logo paradynLogo raised 2 HotPink4
pack .top.logo -side right

frame .textarea
pack  .textarea -side top -fill both -expand true

text .textarea.text -borderwidth 2 -width 40 -height 15 -relief sunken \
	-font { Helvetica 12 } \
	-yscrollcommand ".textarea.sb set" 

pack .textarea.text -side left -fill both -expand true
.textarea.text tag configure paradyn_tag -foreground Red
.textarea.text tag configure app_tag -foreground Black

scrollbar .textarea.sb -relief sunken \
	-command ".textarea.text yview"
pack .textarea.sb -side right -fill y 

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
  set content [.textarea.text get 1.0 end]
  set filename [tk_getSaveFile -parent .textarea.text -title "Save to File"]
  if {$filename != ""} {
     set fileid [open $filename w+ 0644]
     puts $fileid $content
     close $fileid
  }
}
