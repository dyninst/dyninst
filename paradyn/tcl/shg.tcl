# shg.tcl
# Ariel Tamches

#
# $Log: shg.tcl,v $
# Revision 1.5  1996/01/11 00:53:59  tamches
# removed resize1ScrollBar (now in generic.tcl)
# removed iconify menu
#
# Revision 1.4  1996/01/09 01:09:00  tamches
# the label area at the bottom of the shg window can now be 1 or
# 4 lines in height, depending on the status of the devel mode tc
#
# Revision 1.3  1995/11/29 00:21:56  tamches
# removed refs to PdBitmapDir; we now call makeLogo (pdLogo.C)
#
# Revision 1.2  1995/11/20 04:06:02  tamches
# fixed activeBackground/activeForeground colors that were making for ugly
# menu highlighting.
#
# Revision 1.1  1995/10/17 22:25:14  tamches
# First version of new search history graph
#
#

proc shgChangeCurrLabelHeight {numlines} {
   if {[winfo exists .shg.nontop.labelarea.current]} {
      .shg.nontop.labelarea.current config -height $numlines
      pack .shg.nontop.labelarea.current -side left -fill both -expand true
   }
}

proc shgInitialize {} {
   global shgHack
   
   if {[winfo exists .shg]} {
#      puts stderr "(shg window already exists; not creating)"
      wm deiconify .shg
      raise .shg
      return
   }

   toplevel .shg -class "Shg"
   option add *shg*Background grey
   option add *shg*activeBackground LightGrey
   option add *shg*activeForeground black
   wm protocol .shg WM_DELETE_WINDOW {wm iconify .shg}

   # area for title, menubar, logo
   frame .shg.titlearea
   pack  .shg.titlearea -side top -fill x -expand false -anchor n

   frame .shg.titlearea.right
   pack  .shg.titlearea.right -side right -fill y -expand false

   makeLogo .shg.titlearea.right.logo paradynLogo raised 2 mediumseagreen
   pack  .shg.titlearea.right.logo -side top

   frame .shg.titlearea.left
   pack  .shg.titlearea.left -side left -fill both -expand true

   label .shg.titlearea.left.title -text "The Performance Consultant" \
	   -foreground white -anchor c \
           -font *-New*Century*Schoolbook-Bold-R-*-14-* \
	   -relief raised \
	   -background mediumseagreen
   pack  .shg.titlearea.left.title -side top -fill both -expand true

   # area for menubar:
   frame .shg.titlearea.left.menu
   pack  .shg.titlearea.left.menu -side top -fill x -expand false -anchor n

   frame .shg.titlearea.left.menu.mbar -borderwidth 2 -relief raised
   pack  .shg.titlearea.left.menu.mbar -side top -fill both -expand false

   menubutton .shg.titlearea.left.menu.mbar.phase -text Searches -menu .shg.titlearea.left.menu.mbar.phase.m
   menu .shg.titlearea.left.menu.mbar.phase.m -selectcolor cornflowerblue

   pack .shg.titlearea.left.menu.mbar.phase -side left -padx 4

   # -----------------------------------------------------------

   frame .shg.nontop
   pack  .shg.nontop -side bottom -fill both -expand true

   # -----------------------------------------------------------

   frame .shg.nontop.currphasearea
   pack  .shg.nontop.currphasearea -side top -fill x -expand false

   label .shg.nontop.currphasearea.label1 -text "Current Phase: " \
	   -font "*-Helvetica-*-r-*-12-*" -anchor e
   pack  .shg.nontop.currphasearea.label1 -side left -fill both -expand true

   label .shg.nontop.currphasearea.label2 -text "" \
	   -font "*-Helvetica-*-r-*-12-*" -anchor w
   pack  .shg.nontop.currphasearea.label2 -side left -fill both -expand true

   # -----------------------------------------------------------

   frame .shg.nontop.textarea
   pack  .shg.nontop.textarea -side top -fill x -expand false

   text .shg.nontop.textarea.text -borderwidth 2 -width 40 -height 5 -relief sunken \
	   -font "*-Helvetica-*-r-*-12-*" \
	   -yscrollcommand ".shg.nontop.textarea.sb set"
   pack .shg.nontop.textarea.text -side left -fill both -expand true

   scrollbar .shg.nontop.textarea.sb -relief sunken \
	   -command ".shg.nontop.textarea.text yview"
   pack .shg.nontop.textarea.sb -side right -fill y -expand false

   # -----------------------------------------------------------

   frame .shg.nontop.main -width 4i -height 3.5i
   pack  .shg.nontop.main -side top -fill both -expand true

   scrollbar .shg.nontop.main.leftsb -orient vertical -width 16 \
	   -background gray \
	   -activebackground gray \
	   -command "shgNewVertScrollPosition"

   pack .shg.nontop.main.leftsb -side left -fill y -expand false

   scrollbar .shg.nontop.main.bottsb -orient horizontal -width 16 \
        -activebackground gray \
        -command "shgNewHorizScrollPosition"

   pack .shg.nontop.main.bottsb -side bottom -fill x -expand false

   frame .shg.nontop.main.all -relief flat -width 3i -height 2i
   pack .shg.nontop.main.all -side left -fill both -expand true

   # -----------------------------------------------------------

   frame .shg.nontop.labelarea
   pack  .shg.nontop.labelarea -side top -fill x -expand false

   text .shg.nontop.labelarea.current -relief sunken -height 1 \
	   -font "*-Helvetica-*-r-*-12-*" \
	   -wrap none
   if {[uimpd tclTunable getvaluebyname developerMode]} {
      set numlines 4
   } else {
      set numlines 1
   }
   shgChangeCurrLabelHeight $numlines

   # -----------------------------------------------------------

   frame .shg.nontop.buttonarea
   pack  .shg.nontop.buttonarea -side top -fill x -expand false

   frame .shg.nontop.buttonarea.left
   pack  .shg.nontop.buttonarea.left -side left -fill y -expand true

   button .shg.nontop.buttonarea.left.search -text "Search" -anchor c \
	   -command {.shg.nontop.buttonarea.left.search config -state normal; \
	             .shg.nontop.buttonarea.middle.pause config -state normal; \
		     shgSearchCommand}
   pack   .shg.nontop.buttonarea.left.search -side left -ipadx 10 -fill y -expand false

   frame .shg.nontop.buttonarea.middle
   pack  .shg.nontop.buttonarea.middle -side left -fill y -expand true

   button .shg.nontop.buttonarea.middle.pause -text "Pause" -state disabled -anchor c \
	   -command {.shg.nontop.buttonarea.left.search config -state normal; \
	             .shg.nontop.buttonarea.middle.pause config -state disabled; \
		     shgPauseCommand}
   pack   .shg.nontop.buttonarea.middle.pause -side left -fill y -expand false

#   frame .shg.nontop.buttonarea.right
#   pack  .shg.nontop.buttonarea.right -side left -fill y -expand true
#
#   button .shg.nontop.buttonarea.right.resume -text "Resume" -state disabled -anchor c \
#	   -command {.shg.nontop.buttonarea.left.search config -state normal; \
#	             .shg.nontop.buttonarea.middle.pause config -state normal; \
#	             .shg.nontop.buttonarea.right.resume config -state disabled; \
#		     shgResumeCommand}
#   pack   .shg.nontop.buttonarea.right.resume -side right -fill y -expand false

   # -----------------------------------------------------------

   label .shg.nontop.tip0 -relief groove \
	   -text "Uninstrumented" -anchor c \
	   -font "*-Helvetica-*-r-*-12-*" \
	   -background "#e9fbb57aa3c9"
              # yuck --ari
#	   -background Gray
   pack   .shg.nontop.tip0 -side top -fill both -expand false

   label .shg.nontop.tip1 -relief groove \
	   -text "Instrumented; no decision yet" -anchor c \
	   -font "*-Helvetica-*-r-*-12-*" \
	   -background "#ffffbba5bba5"
                # yuck --ari
#	   -background Tan
   pack   .shg.nontop.tip1 -side top -fill both -expand false

   label .shg.nontop.tip2 -relief groove \
	   -text "Instrumented; believed true" -anchor c \
	   -font "*-Helvetica-*-r-*-12-*" \
	   -background "#acbff48ff6c8"
                # yuck --ari
#-text "True (tentatively)"

#	   -background LightBlue
   pack   .shg.nontop.tip2 -side top -fill both -expand false

   label .shg.nontop.tip3 -relief groove \
	   -text "Uninstrumented; believed false" -anchor c \
	   -font "*-Helvetica-*-r-*-12-*" \
	   -background "#cc85d5c2777d" 
                # yuck --ari
#	   -background pink
#False (instrumentation removed)

   pack   .shg.nontop.tip3 -side top -fill both -expand false

   label .shg.nontop.tip4 -relief sunken \
           -text "Hold down Alt and move the mouse to scroll freely" -anchor c \
           -font "*-Helvetica-*-r-*-12-*"
   pack  .shg.nontop.tip4 -side top -fill both -expand false

   # -----------------------------------------------------------

   bind .shg.nontop.main.all <Configure> {shgConfigureHook}
   bind .shg.nontop.main.all <Expose>    {shgExposeHook %c}
   bind .shg.nontop.main.all <Button-1>  {shgSingleClickHook %x %y}
   bind .shg.nontop.main.all <Button-2>  {shgSingleClickHook %x %y}
   bind .shg.nontop.main.all <Double-Button-1> {shgDoubleClickHook %x %y}
#   bind .shg.nontop.main.all <Shift-Double-Button-1> {shgShiftDoubleClickHook %x %y}
#   bind .shg.nontop.main.all <Control-Double-Button-1> {shgCtrlDoubleClickHook %x %y}
   bind .shg.nontop.main.all <Alt-Motion> {shgAltPressHook %x %y}
   bind .shg.nontop.main.all <Motion> {shgAltReleaseHook %x %y}

   if {$shgHack} {
#      puts stderr "shgHack"
      paradyn shg start global
   }
}
