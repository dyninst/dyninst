# dagSetup.tcl
# plain vanilla script to setup frames and scrollbars for the dag widget

# $Log: dagSetup.tcl,v $
# Revision 1.3  1994/06/29 21:47:34  hollings
# killed old background colors and switched to motif like greys.
# cleaned up option specification to use options data base.
#
# Revision 1.2  1994/05/18  00:37:28  karavan
# fixed scrollbars
#
# Revision 1.1  1994/05/03  06:35:55  karavan
# Initial version.
#

proc setupDAG parentw {
	set vs $parentw._vscroll_
	set hs $parentw._hscroll_
	set c $parentw._c_

	scrollbar $vs -relief sunken -command "$c yview"
 	scrollbar $hs -relief sunken -orient horiz -command "$c xview" 
 
	pack $hs -side bottom -fill x 
	pack $vs -side right -fill y 
	pack $c -side top -expand yes -fill both
	 
    $c config -xscroll "$hs set" -yscroll "$vs set"
#	-background "#fb63e620d36b"	

}

    


    
