# shg.tcl
# Ariel Tamches

#
# $Log: shg.tcl,v $
# Revision 1.1  1995/10/17 22:25:14  tamches
# First version of new search history graph
#
#

proc resize1Scrollbar {sbname newTotal newVisible} {
   # This is a nice n' generic routine  --ari
   # However, it is (currently) only called from C++ code.  If this
   # situation doesn't change, then we might want to just
   # zap this and turn it into C++ code...

   # 'newTotal' and 'newVisible' are tentative values;
   # We use them to calculate 'newFirst' and 'newLast'.
   # We make an effort to keep 'newFirst' as close as possible to 'oldFirst'.

   set oldConfig [$sbname get]
   set oldFirst  [lindex $oldConfig 0]
   set oldLast   [lindex $oldConfig 1]
   #   puts stderr "oldFirst=$oldFirst; oldLast=$oldLast"

   if {$newVisible < $newTotal} {
      # The usual case: not everything fits
      set fracVisible [expr 1.0 * $newVisible / $newTotal]
#      puts stderr "newVisible=$newVisible; newTotal=$newTotal; fracVisible=$fracVisible"

      set newFirst $oldFirst
      set newLast [expr $newFirst + $fracVisible]

#      puts stderr "tentative newFirst=$newFirst; newLast=$newLast"

      if {$newLast > 1.0} {
         set theOverflow [expr $newLast - 1.0]
#         puts stderr "resize1Scrollbar: would overflow by fraction of $theOverflow; moving newFirst back"
         set newFirst [expr $oldFirst - $theOverflow]
         set newLast  [expr $newFirst + $fracVisible]
      } else {
#         puts stderr "resize1Scrollbar: yea, we were able to keep newFirst unchanged at $newFirst"
      }
   } else {
      # the unusual case: everything fits (visible >= total)
      set newFirst 0.0
      set newLast  1.0
   }

   if {$newFirst < 0} {
      # This is an assertion failure
      puts stderr "resize1Scrollbar warning: newFirst is $newFirst"
   }
   if {$newLast > 1} {
      # This is an assertion failure
      puts stderr "resize1Scrollbar warning: newLast is $newLast"
   }

   $sbname set $newFirst $newLast
}

proc shgInitialize {} {
   global shgHack
   
   if {[winfo exists .shg]} {
#      puts stderr "(shg window already exists; not creating)"
      wm deiconify .shg
      return
   }

   toplevel .shg -class "Shg"
   option add *shg*Background grey
   wm protocol .shg WM_DELETE_WINDOW {wm iconify .shg}

   # area for title, menubar, logo
   frame .shg.titlearea
   pack  .shg.titlearea -side top -fill x -expand false -anchor n

   frame .shg.titlearea.right
   pack  .shg.titlearea.right -side right -fill y -expand false

   global PdBitmapDir
   label .shg.titlearea.right.logo -bitmap @$PdBitmapDir/logo.xbm -foreground #b3331e1b53c7 -relief raised
   pack  .shg.titlearea.right.logo -side top

#   mkLogo .shg.titlearea.right.logo top

   frame .shg.titlearea.left
   pack  .shg.titlearea.left -side left -fill both -expand true

   label .shg.titlearea.left.title -text "The Performance Consultant" \
	   -foreground white \
           -font *-New*Century*Schoolbook-Bold-R-*-14-* \
	   -relief raised \
	   -background limegreen
   pack  .shg.titlearea.left.title -side top -fill both -expand true

   # area for menubar:
   frame .shg.titlearea.left.menu
   pack  .shg.titlearea.left.menu -side top -fill x -expand false -anchor n

   frame .shg.titlearea.left.menu.mbar -borderwidth 2 -relief raised
   pack  .shg.titlearea.left.menu.mbar -side top -fill both -expand false

   menubutton .shg.titlearea.left.menu.mbar.file -text File -menu .shg.titlearea.left.menu.mbar.file.m
   menu .shg.titlearea.left.menu.mbar.file.m -selectcolor cornflowerblue
   .shg.titlearea.left.menu.mbar.file.m add command -label "Iconify" -command "wm iconify .shg"

   menubutton .shg.titlearea.left.menu.mbar.phase -text Searches -menu .shg.titlearea.left.menu.mbar.phase.m
   menu .shg.titlearea.left.menu.mbar.phase.m -selectcolor cornflowerblue

   pack .shg.titlearea.left.menu.mbar.file .shg.titlearea.left.menu.mbar.phase -side left -padx 4

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

#   label .shg.nontop.labelarea.current -relief sunken -height 1 \
#	   -font "*-Helvetica-*-r-*-12-*"
#   pack  .shg.nontop.labelarea.current -side left -fill both -expand false

   text .shg.nontop.labelarea.current -relief sunken -height 1 \
	   -font "*-Helvetica-*-r-*-12-*" \
	   -wrap none
   pack .shg.nontop.labelarea.current -side left -fill both -expand true

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
	   -text "Uninstrumented" \
	   -font "*-Helvetica-*-r-*-12-*" \
	   -background "#e9fbb57aa3c9"
              # yuck --ari
#	   -background Gray
   pack   .shg.nontop.tip0 -side top -fill both -expand false

   label .shg.nontop.tip1 -relief groove \
	   -text "Instrumented; no decision yet" \
	   -font "*-Helvetica-*-r-*-12-*" \
	   -background "#ffffbba5bba5"
                # yuck --ari
#	   -background Tan
   pack   .shg.nontop.tip1 -side top -fill both -expand false

   label .shg.nontop.tip2 -relief groove \
	   -text "Instrumented; believed true" \
	   -font "*-Helvetica-*-r-*-12-*" \
	   -background "#acbff48ff6c8"
                # yuck --ari
#-text "True (tentatively)"

#	   -background LightBlue
   pack   .shg.nontop.tip2 -side top -fill both -expand false

   label .shg.nontop.tip3 -relief groove \
	   -text "Uninstrumented; believed false" \
	   -font "*-Helvetica-*-r-*-12-*" \
	   -background "#cc85d5c2777d" 
                # yuck --ari
#	   -background pink
#False (instrumentation removed)

   pack   .shg.nontop.tip3 -side top -fill both -expand false

   label .shg.nontop.tip4 -relief sunken \
           -text "Hold down Alt and move the mouse to scroll freely" \
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
