# tcl procedures for display of visualization menu
# $Log: visiMenu.tcl,v $
# Revision 1.1  1994/05/03 06:36:05  karavan
# Initial version.
#

#
# fillMenu
#   menuname = valid name of tk menu to which choices will be added 
#   menulabels = list of menu choices (strings)
#   menucoms = list of menu commands (strings)
#   numchoices = number of menu choices (integer)
#
proc fillMenu {menuname menulabels menucoms numchoices} {

  for {set i 0} {$i < $numchoices} {incr i} {
   $menuname add command -label [lindex $menulabels $i] \
	 -command [lindex $menucoms $i] -underline 0
  }

}

proc buildVisiCreateLst {menuIDs} {
 set newCmdLst {}
 foreach menuID $menuIDs {
	lappend newCmdLst "paradyn visi create [expr $menuID]"
 }
 return $newCmdLst
}

proc drawVisiMenu {title menunames menuIDs numchoices} {
	puts $menunames
	puts $menuIDs
	puts $numchoices
  set menucoms [buildVisiCreateLst $menuIDs]
  fillMenu $title $menunames $menucoms $numchoices
}





