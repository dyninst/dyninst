#initWHERE.tcl
# all tk setup for WHERE axis display, including initializing 
# some default styles for nodes and edges

# $Log: initWHERE.tcl,v $
# Revision 1.2  1994/05/26 21:23:11  karavan
# changed parent window name.
#
# Revision 1.1  1994/05/23  01:56:23  karavan
# initial version.
#

proc initWHERE {wwindow} {

global WHEREname PdMainBgColor
set WHEREname $wwindow

dag $WHEREname.d01 
frame $WHEREname.buttons -bg  "#fb63e620d36b"
#button $WHEREname.buttons.b1 -text "QUIT WHERE" -bg $PdMainBgColor \
#	-command {destroy $WHEREname}
label $WHEREname.title -text "Where Axis" -fg black \
	-font "-Adobe-times-bold-r-normal--*-120*" \
	-bg $PdMainBgColor -relief raised
#label $WHEREname.explain -text " " -fg black \
#	-bg $PdMainBgColor -relief raised -width 60

pack $WHEREname.title \
	-side top -fill both
pack $WHEREname.d01 -side top -expand 1 -fill both
#pack $WHEREname.buttons -side bottom -expand 0 -fill x
#pack $WHEREname.buttons.b1 -side left -expand yes -fill x


#$WHEREname.d01 bind all <2> {shgFullName $WHEREname.d01._c_}

# style 1: WHERE axis node 
$WHEREname.d01 addNstyle 1 -bg #c99e5f54dcab \
		-font "-adobe-times-bold-r-normal--*-100*" \
		-text black  \
		-stipple "" -width 1

$WHEREname.d01 addEstyle 1 -arrow none -fill #c99e5f54dcab -width 2
}

