#initSHG.tcl
# all tk setup for search history graph display, including initializing 
# some default styles for dag nodes and edges

# $Log: initSHG.tcl,v $
# Revision 1.1  1994/05/03 06:36:00  karavan
# Initial version.
#

proc initSHG {} {

global SHGname

set clrSHGQUITBUTTbg "#fb63e620d36b"
set clrSHGSTEPBUTTbg "#fb63e620d36b"
set clrSHGAUTOBUTTbg "#fb63e620d36b"
toplevel $SHGname
 
wm minsize $SHGname 200 200
dag $SHGname.d01 
frame $SHGname.buttons -bg  "#fb63e620d36b"
button $SHGname.buttons.b1 -text "QUIT" -bg $clrSHGQUITBUTTbg \
	-command {destroy .}
button $SHGname.buttons.b2 -text "STEP" -bg $clrSHGSTEPBUTTbg \
	-command {paradyn search true 1}
button $SHGname.buttons.b3 -text "AUTO SEARCH" -bg $clrSHGAUTOBUTTbg \
	-command {paradyn search true -1}
label $SHGname.title -text "Paradyn Search History" -fg black \
	-font "-Adobe-times-medium-r-normal--*-120*" \
	-bg "#fb63e620d36b" -relief raised
 
pack $SHGname.title -side top -fill both
pack $SHGname.d01 -side top -expand 1 -fill both
pack $SHGname.buttons -side bottom -expand 1 -fill both
pack $SHGname.buttons.b2 $SHGname.buttons.b3 $SHGname.buttons.b1  \
	-side left \
	-expand yes -fill x

wm title $SHGname "Performance Consultant"
tkwait visibility $SHGname

# style 1: not tested 
$SHGname.d01 addNstyle 1 -bg DarkSalmon \
		-font "-Adobe-times-medium-r-normal--*-120*" \
		-text "SlateGrey" -outline  "DarkSlateGrey" \
		-stipple "" -width 1

# style 2: not active
$SHGname.d01 addNstyle 2 -bg #a41bab855fe1 \
	 -font "-Adobe-times-medium-r-normal--*-120*" \
	-text black -outline DarkSlateGrey -stipple "" -width 1

#style 3: active and true
$SHGname.d01 addNstyle 3 -bg  #4cc6c43dc7ef \
	-font "-Adobe-times-medium-r-normal--*-120*" \
	-text black  -outline "SlateGrey" -stipple "" -width 1

#style 4: active and false
$SHGname.d01 addNstyle 4 -bg #8ba59f3b91f3 \
	 -font "-Adobe-times-medium-r-normal--*-120*" \
	-text white -outline DarkSlateGrey -stipple "" -width 1

$SHGname.d01 addEstyle 1 -arrow none -fill #f91612aedde6 -width 2
$SHGname.d01 addEstyle 2 -arrow none -fill #ffff8ada2b02 -width 2
$SHGname.d01 addEstyle 3 -arrow none -fill black -width 2

}

