#initSHG.tcl
# all tk setup for search history graph display, including initializing 
# some default styles for dag nodes and edges

# $Log: initSHG.tcl,v $
# Revision 1.3  1994/05/07 23:25:21  karavan
# added short explanation feature
#
# Revision 1.2  1994/05/06  06:41:04  karavan
# changed buttons to AUTO SEARCH/REFINE/PAUSE-RUN
#
# Revision 1.1  1994/05/03  06:36:00  karavan
# Initial version.
#

proc SHGpause {butt} {
global PCsearchState
if {$PCsearchState == 1} {
 set PCsearchState 0
 $butt configure -text "RESUME SEARCH"
 paradyn search pause
} else {
 set PCsearchState 1
 $butt configure -text "PAUSE SEARCH"
 paradyn search true -1
}
}

#proc shgShortExplain {num} {
#	puts $num
#}

proc shgFullName {whoCan} {
	global shgExplainStr
	set nodeID [lindex [$whoCan gettags current] 0]
	if [string match n* $nodeID] {
		uimpd shgShortExplain [string range $nodeID 1 end]
	}  
}

proc initSHG {} {

global SHGname PCsearchState shgExplainStr

set PCsearchState 1
set shgExplainStr ""
set clrSHGQUITBUTTbg "#fb63e620d36b"
set clrSHGSTEPBUTTbg "#fb63e620d36b"
set clrSHGAUTOBUTTbg "#fb63e620d36b"
set clrSHGPAUSEBUTTbg "#fb63e620d36b"
toplevel $SHGname
 
wm minsize $SHGname 400 200
dag $SHGname.d01 
frame $SHGname.buttons -bg  "#fb63e620d36b"
button $SHGname.buttons.b1 -text "QUIT PC" -bg $clrSHGQUITBUTTbg \
	-command {destroy $SHGname}
button $SHGname.buttons.b2 -text "REFINE" -bg $clrSHGSTEPBUTTbg \
	-command {paradyn search true 1}
button $SHGname.buttons.b3 -text "AUTO SEARCH" -bg $clrSHGAUTOBUTTbg \
	-command {paradyn search true -1}
button $SHGname.buttons.b4 -text "PAUSE SEARCH" -bg $clrSHGPAUSEBUTTbg \
	-command {SHGpause $SHGname.buttons.b4}   
label $SHGname.title -text "Paradyn Search History" -fg black \
	-font "-Adobe-times-bold-r-normal--*-120*" \
	-bg "#fb63e620d36b" -relief raised
label $SHGname.explain -textvariable shgExplainStr -fg black \
	-bg "#fb63e620d36b" -relief raised -width 80

pack $SHGname.title $SHGname.explain -side top -fill both
pack $SHGname.d01 -side top -expand 1 -fill both
pack $SHGname.buttons -side bottom -expand 0 -fill x
pack $SHGname.buttons.b2 $SHGname.buttons.b3 $SHGname.buttons.b4 \
	$SHGname.buttons.b1 -side left -expand yes -fill x

wm title $SHGname "Performance Consultant"
$SHGname.d01 bind all <2> {shgFullName $SHGname.d01._c_}

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

