# dagSetup.tcl
# plain vanilla script to setup frames and scrollbars for the dag widget

# $Log: dagSetup.tcl,v $
# Revision 1.4  1994/08/01 20:26:28  karavan
# changes to accommodate new dag design.
#
# Revision 1.3  1994/06/29  21:47:34  hollings
# killed old background colors and switched to motif like greys.
# cleaned up option specification to use options data base.
#
# Revision 1.2  1994/05/18  00:37:28  karavan
# fixed scrollbars
#
# Revision 1.1  1994/05/03  06:35:55  karavan
# Initial version.
#

### handle error case -- not n*
proc updateCurrentSelection {whoCan dagID} {
    set gvarname currentSelection$dagID
    global $gvarname
    set nodeID [lindex [$whoCan gettags current] 0]
    if [string match n* $nodeID] { 
	set newID [string range $nodeID 1 end]
	uimpd unhighlightNode [expr $$gvarname] $dagID
	set currentSelection$dagID $newID
	uimpd highlightNode $newID $dagID
	return $newID
    }
}

proc setupDAG parentw {
    global $parentw

    scrollbar $parentw._vscroll_ -relief sunken \
	    -command "$parentw._c_ yview"
    scrollbar $parentw._hscroll_ -relief sunken \
	    -command "$parentw._c_ xview" -orient horiz  
    canvas $parentw._c_

    pack $parentw._hscroll_ -side bottom -fill x 
    pack $parentw._vscroll_ -side right -fill y 
    pack $parentw._c_ -side top -expand yes -fill both
	 
    $parentw._c_ config \
	    -xscroll "$parentw._hscroll_ set" \
	    -yscroll "$parentw._vscroll_ set"

    update
}

    


    
