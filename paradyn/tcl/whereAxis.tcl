# whereAxis.tcl
# Ariel Tamches

#
# $Log: whereAxis.tcl,v $
# Revision 1.5  1995/10/17 22:25:57  tamches
# A few command names have "whereAxis" prepended to them, to differentiate
# them from shg tcl code.  Added tip4.
#
# Revision 1.4  1995/09/20 01:37:11  tamches
# Stupid hack to ensure middle-mouse-button can move scrollbars
# within the where axis.
#
# Revision 1.3  1995/07/24  21:38:07  tamches
# Implemented alt-freescroll feature
#
# Revision 1.2  1995/07/18  03:38:08  tamches
# Added ctrl-double-click to select/unselect an entire subtree (nonrecursive).
# Added "clear" menu item to clear all selections.
#
# Revision 1.1  1995/07/17  05:00:34  tamches
# First version of new where axis
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

# ##################################################################

proc whereAxisCatchDeleteWindow {} {
   # change the delete into an iconify
   wm iconify .whereAxis
}

# ##################################################################

proc whereAxisShowSelections {} {
}

# ##################################################################

proc whereAxisInitialize {} {
   toplevel .whereAxis -class "WhereAxis"
   option add *whereAxis*Background grey
   wm protocol .whereAxis WM_DELETE_WINDOW whereAxisCatchDeleteWindow
   
   frame .whereAxis.top
   pack  .whereAxis.top -side top -fill x -expand false -anchor n
      # area for menubar
   
   frame .whereAxis.top.mbar -borderwidth 2 -relief raised
   pack  .whereAxis.top.mbar -side top -fill both -expand false
   
   menubutton .whereAxis.top.mbar.file -text Window -menu .whereAxis.top.mbar.file.m
   menu .whereAxis.top.mbar.file.m -selectcolor cornflowerblue
   .whereAxis.top.mbar.file.m add command -label "Iconify" -command "wm iconify .whereAxis"
   
   menubutton .whereAxis.top.mbar.sel -text Selections -menu .whereAxis.top.mbar.sel.m
   menu .whereAxis.top.mbar.sel.m -selectcolor cornflowerblue
   .whereAxis.top.mbar.sel.m add command -label "Clear" -command whereAxisClearSelections
#   .whereAxis.top.mbar.sel.m add command -label "Show" -command whereAxisShowSelections
   
   menubutton .whereAxis.top.mbar.nav -text Navigate -menu .whereAxis.top.mbar.nav.m
   menu .whereAxis.top.mbar.nav.m -selectcolor cornflowerblue
   
   menubutton .whereAxis.top.mbar.abs -text Abstraction -menu .whereAxis.top.mbar.abs.m
   menu .whereAxis.top.mbar.abs.m -selectcolor cornflowerblue
   
   pack .whereAxis.top.mbar.file .whereAxis.top.mbar.sel .whereAxis.top.mbar.nav .whereAxis.top.mbar.abs -side left -padx 4
   tk_menuBar .whereAxis.top.mbar .whereAxis.top.mbar.file .whereAxis.top.mbar.sel .whereAxis.top.mbar.nav .whereAxis.top.mbar.abs
   
   # -----------------------------------------------------------
   
   frame .whereAxis.nontop
   pack  .whereAxis.nontop -side bottom -fill both -expand true
   
   # -----------------------------------------------------------
   
   frame .whereAxis.nontop.main -width 3i -height 2.5i
   pack  .whereAxis.nontop.main -side top -fill both -expand true
   
   scrollbar .whereAxis.nontop.main.leftsb -orient vertical -width 16 -background gray \
   	-activebackground gray \
   	-command "whereAxisNewVertScrollPosition"
   
   pack .whereAxis.nontop.main.leftsb -side left -fill y -expand false
   
   scrollbar .whereAxis.nontop.main.bottsb -orient horizontal -width 16 \
   	-activebackground gray \
   	-command "whereAxisNewHorizScrollPosition"
   
   pack .whereAxis.nontop.main.bottsb -side bottom -fill x -expand false
   
   #canvas .whereAxis.nontop.main.all -relief flat -width 3i -height 2i \
   #	-yscrollcommand myYScrollCommand \
   #	-xscrollcommand myXScrollCommand \
   #	-scrollincrement 1
   frame .whereAxis.nontop.main.all -relief flat -width 3i -height 2i
   pack .whereAxis.nontop.main.all -side left -fill both -expand true
   
   # -----------------------------------------------------------
   
   label .whereAxis.nontop.tip1 -relief sunken \
	   -text "Click to select; double-click to expand/un-expand" \
	   -font "*-Helvetica-*-r-*-12-*"
   pack  .whereAxis.nontop.tip1 -side top -fill both -expand false
      # fill both (instead of just x) seems needed to prevent from shrinking
      # when window made shorter
   
   label .whereAxis.nontop.tip2 -relief sunken \
	   -text "Shift-double-click to expand/un-expand all subtrees of a node" \
	   -font "*-Helvetica-*-r-*-12-*"
   pack  .whereAxis.nontop.tip2 -side top -fill both -expand false
      # fill both (instead of just x) seems needed to prevent from shrinking
      # when window made shorter
   
   label .whereAxis.nontop.tip3 -relief sunken \
	   -text "Ctrl-double-click to select/un-select all subtrees of a node" \
	   -font "*-Helvetica-*-r-*-12-*"
   pack  .whereAxis.nontop.tip3 -side top -fill both -expand false
      # fill both (instead of just x) seems needed to prevent from shrinking
      # when window made shorter

   label .whereAxis.nontop.tip4 -relief sunken \
	   -text "Hold down Alt and move the mouse to scroll freely" \
	   -font "*-Helvetica-*-r-*-12-*"
   pack  .whereAxis.nontop.tip4 -side top -fill both -expand false

   
   # -----------------------------------------------------------
   
   frame .whereAxis.nontop.find
   pack  .whereAxis.nontop.find -side top -fill both -expand false
   
   label .whereAxis.nontop.find.label -relief sunken -font "*-Helvetica-*-r-*-12-*" -text "Search:"
   pack  .whereAxis.nontop.find.label -side left -fill y -expand false
   
   entry .whereAxis.nontop.find.entry -relief sunken -font "*-Helvetica-*-r-*-12-*"
   pack  .whereAxis.nontop.find.entry -side left -fill x -expand true
   
   bind  .whereAxis.nontop.find.entry <Return> {whereAxisFindHook [.whereAxis.nontop.find.entry get]}
   
   # -----------------------------------------------------------
   
   # install resize, expose, and button event hooks for .whereAxis.nontop.main.all
   bind .whereAxis.nontop.main.all <Configure> {whereAxisConfigureHook}
   bind .whereAxis.nontop.main.all <Expose>    {whereAxisExposeHook %c}
   bind .whereAxis.nontop.main.all <Button-1>  {whereAxisSingleClickHook %x %y}
   bind .whereAxis.nontop.main.all <Button-2>  {whereAxisSingleClickHook %x %y}
   bind .whereAxis.nontop.main.all <Double-Button-1> {whereAxisDoubleClickHook %x %y}
   bind .whereAxis.nontop.main.all <Shift-Double-Button-1> {whereAxisShiftDoubleClickHook %x %y}
   bind .whereAxis.nontop.main.all <Control-Double-Button-1> {whereAxisCtrlDoubleClickHook %x %y}
   bind .whereAxis.nontop.main.all <Alt-Motion> {whereAxisAltPressHook %x %y}
   bind .whereAxis.nontop.main.all <Motion> {whereAxisAltReleaseHook}
   
   set currMenuAbstraction 1
}
