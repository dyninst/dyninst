# dagSetup.tcl
# plain vanilla script to setup frames and scrollbars for the dag widget

# $Log: dagSetup.tcl,v $
# Revision 1.2  1994/05/18 00:37:28  karavan
# fixed scrollbars
#
# Revision 1.1  1994/05/03  06:35:55  karavan
# Initial version.
#

proc setupDAG parentw {
	set vs $parentw._vscroll_
	set hs $parentw._hscroll_
	set c $parentw._c_

	scrollbar $vs -relief sunken -command "$c yview" \
		-foreground "#fb63e620d36b" \
		-activeforeground "#d8107000421d"
 	scrollbar $hs -relief sunken -orient horiz -command "$c xview" \
		-foreground "#fb63e620d36b" \
		-activeforeground "#d8107000421d"
 
	pack $hs -side bottom -fill x 
	pack $vs -side right -fill y 
	pack $c -side top -expand yes -fill both
	 
    $c config -xscroll "$hs set" -yscroll "$vs set" \
	-background "#fb63e620d36b"	

}

    


    
