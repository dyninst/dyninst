# $Id: whereAxis.tcl,v 1.15 2004/03/20 20:44:50 pcroth Exp $

# ##################################################################

proc whereAxisInitialize {} {
   toplevel .whereAxis -class Paradyn
   option add *whereAxis*Background grey widgetDefault
   option add *whereAxis*activeBackground LightGrey widgetDefault
   option add *activeForeground black widgetDefault
   wm protocol .whereAxis WM_DELETE_WINDOW {wm iconify .whereAxis}
   
   frame .whereAxis.top
   pack  .whereAxis.top -side top -fill x -expand false -anchor n
      # area for menubar
   
   # we need notification when the whereAxis is to be
   # destroyed so we can release the fonts it uses
   bind .whereAxis.top <Destroy> +{whereAxisDestroyHook}

   frame .whereAxis.top.mbar -borderwidth 2 -relief raised
   pack  .whereAxis.top.mbar -side top -fill both -expand false
   
   menubutton .whereAxis.top.mbar.sel -text Selections -menu .whereAxis.top.mbar.sel.m
   menu .whereAxis.top.mbar.sel.m -selectcolor #6495ED
   .whereAxis.top.mbar.sel.m add command -label "Clear" -command whereAxisClearSelections
   
   menubutton .whereAxis.top.mbar.nav -text Navigate -menu .whereAxis.top.mbar.nav.m
   menu .whereAxis.top.mbar.nav.m -selectcolor #6495ED
   
   pack .whereAxis.top.mbar.sel .whereAxis.top.mbar.nav  -side left -padx 4
   
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
   
   frame .whereAxis.nontop.main.all -relief flat -width 3i -height 2i
   pack .whereAxis.nontop.main.all -side left -fill both -expand true
   
   # -----------------------------------------------------------

   frame .whereAxis.nontop.find
   pack  .whereAxis.nontop.find -side top -fill both -expand false
   
   label .whereAxis.nontop.find.label -relief sunken -text "Search:"
   pack  .whereAxis.nontop.find.label -side left -fill y -expand false
   
   entry .whereAxis.nontop.find.entry -relief sunken
   pack  .whereAxis.nontop.find.entry -side left -fill x -expand true
   
   bind  .whereAxis.nontop.find.entry <Return> {whereAxisFindHook [.whereAxis.nontop.find.entry get]}
   
   # -----------------------------------------------------------
   
   whereAxisDrawTipsBase
   whereAxisDrawTips

   # -----------------------------------------------------------
   
   # install resize, expose, and button event hooks for .whereAxis.nontop.main.all
   bind .whereAxis.nontop.main.all <Configure> {whereAxisConfigureHook}
   bind .whereAxis.nontop.main.all <Expose>    {whereAxisExposeHook %c}
   bind .whereAxis.nontop.main.all <Visibility> {whereAxisVisibilityHook %s}
   bind .whereAxis.nontop.main.all <Button-1>  {whereAxisSingleClickHook %x %y}
   bind .whereAxis.nontop.main.all <Button-3>  {whereAxisSingleClickHook %x %y}
   bind .whereAxis.nontop.main.all <Double-Button-1> {whereAxisDoubleClickHook %x %y}
   bind .whereAxis.nontop.main.all <Control-Button-3> {whereAxisCtrlClickHook %x %y}
   bind .whereAxis.nontop.main.all <Shift-Double-Button-1> {whereAxisShiftDoubleClickHook %x %y}
   bind .whereAxis.nontop.main.all <Control-Double-Button-1> {whereAxisCtrlDoubleClickHook %x %y}
   bind .whereAxis.nontop.main.all <Alt-Motion> {whereAxisAltPressHook %x %y}
   bind .whereAxis.nontop.main.all <Motion> {whereAxisAltReleaseHook}
   
   set currMenuAbstraction 1
}

proc whereAxisDrawTipsBase {} {
   frame .whereAxis.nontop.tips
   pack .whereAxis.nontop.tips -side top -fill x -expand false
}

proc whereAxisDrawTips {} {
   if { [winfo exists .whereAxis.nontop.tips.tip1] } {
      return
   }

   label .whereAxis.nontop.tips.tip1 -relief groove \
	   -text "Click to select; double-click to expand/un-expand"
   pack  .whereAxis.nontop.tips.tip1 -side top -fill both -expand false
      # fill both (instead of just x) seems needed to prevent from shrinking
      # when window made shorter
   
   label .whereAxis.nontop.tips.tip2 -relief groove \
	   -text "Shift-double-click to expand/un-expand all subtrees of a node"
   pack  .whereAxis.nontop.tips.tip2 -side top -fill both -expand false
      # fill both (instead of just x) seems needed to prevent from shrinking
      # when window made shorter
   
   label .whereAxis.nontop.tips.tip3 -relief groove \
	   -text "Ctrl-double-click to select/un-select all subtrees of a node"
   pack  .whereAxis.nontop.tips.tip3 -side top -fill both -expand false
      # fill both (instead of just x) seems needed to prevent from shrinking
      # when window made shorter

   label .whereAxis.nontop.tips.tip4 -relief groove \
	   -text "Hold down Alt and move the mouse to scroll freely"
   pack  .whereAxis.nontop.tips.tip4 -side top -fill both -expand false

   label .whereAxis.nontop.tips.tip5 -relief groove \
	   -text "Ctrl-Single-Click right button to show Visi table"
   pack  .whereAxis.nontop.tips.tip5 -side top -fill both -expand false
}

proc whereAxisEraseTips {} {
   if { ![winfo exists .whereAxis.nontop.tips.tip1] } {
      return
   }

   destroy .whereAxis.nontop.tips
   whereAxisDrawTipsBase
}
