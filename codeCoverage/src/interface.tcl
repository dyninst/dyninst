# global variables used

set globalDataStructure(0) 1
set globalExecutionMap(0) 1
set globalFrequencyMap(1) [list 0 0]

set lastFileSelection -1
set lastFuncSelection -1
set isTerminated 0
set alreadyStarted 0

# utility related code

proc DisplayAxis { canvasPanel } {
	$canvasPanel.line create line 10 10 10 192 -fill black \
		-tag permanent -width 2
	$canvasPanel.line create line 8 190 190 190 -fill black \
		-tag permanent -width 2
	$canvasPanel.code create line 10 10 10 192 -fill black \
		-tag permanent -width 2
	$canvasPanel.code create line 8 190 190 190 -fill black \
		-tag permanent -width 2
}
proc DisplayCoverage { canvasPanel globalFrequency } {
	upvar $globalFrequency localgf
	set gfsize [array size localgf]
	set gridsize [expr [expr 180 * 1.0] / $gfsize]
	set linemax -1
	set codemax -1
	for {set i 1} { $i <= $gfsize } { incr i } {
		set x [expr 10 + [expr int([expr $gridsize * $i ])]]
		$canvasPanel.line create line $x 187 $x 190 \
				-fill black -tag temporary
		$canvasPanel.code create line $x 187 $x 190 \
				-fill black -tag temporary

		set value [lindex $localgf($i) 0]
		if { $linemax < $value } {
			set linemax $value
		}

		set value [lindex $localgf($i) 1]
		if { $codemax < $value } {
			set codemax $value 
		}
	}
	set xp 10
	set yp 190
	if { $linemax > 0  && $codemax > 0 } {
		set codetotal 0
		for {set i 1} { $i <= $gfsize } { incr i } {
			set x [expr 10 + [expr int([expr $gridsize * $i ])]]

			set liney [expr int([expr [expr [lindex $localgf($i) 0] \
							* 180.0] / $linemax])]
			$canvasPanel.line create line $xp $yp $x \
				[expr 190 - $liney] -fill purple \
				-tag temporary -width 2
			set xp $x
			set yp [expr 190 - $liney]

			set codetotal [expr $codetotal + [lindex $localgf($i) 1]]
			set codey [expr int([expr [expr [lindex $localgf($i) 1] \
							* 180.0] / $codemax])]
			$canvasPanel.code create line $x 190 $x \
				[expr 190 - $codey] -fill purple \
				-tag temporary -width 2
		}

		set str "total: "
		append str $linemax
		set strid [$canvasPanel.line create text 110 198 \
				-font comic8 -tag temporary] 
		$canvasPanel.line insert $strid insert $str
		
		set str "max: "
		append str $codemax " total: " $codetotal
		set strid [$canvasPanel.code create text 110 198 \
				-font comic8 -tag temporary]
		$canvasPanel.code insert $strid insert $str
	}
}

proc CalculatePercentage { fileIndex funcIndex globalExecution } {
	upvar $globalExecution localge
	set retValue 0.0
	if { $funcIndex == -1 } {
		set retValue [lindex $localge($fileIndex) 1]
	} else {
		set funcList [lindex [lindex $localge($fileIndex) 0] $funcIndex]
		set retValue [lindex $funcList 0]
	}
	return $retValue
}
 

# widget related code

proc Scroll_Set { scrollbar geoCmd offset size } {
	if {$offset != 0.0 || $size != 1.0} {
		eval $geoCmd
		$scrollbar set $offset $size
	} else {
		set manager [lindex $geoCmd 0]
		$manager forget $scrollbar
	}
}

proc Scrolled_Text { frameWidget args } {
	frame $frameWidget
	text $frameWidget.text \
		-xscrollcommand [list Scroll_Set $frameWidget.xscroll \
				 [list grid $frameWidget.xscroll -row 1 \
				 -column 0 -sticky we]] \
		-yscrollcommand [list Scroll_Set $frameWidget.yscroll \
				 [list grid $frameWidget.yscroll -row 0 \
				 -column 1 -sticky ns]]

	eval { $frameWidget.text configure } $args

	scrollbar $frameWidget.xscroll -orient horizontal \
		-command [list $frameWidget.text xview] -bg darkgray
	scrollbar $frameWidget.yscroll -orient vertical \
		-command [list $frameWidget.text yview] -bg darkgray

	grid $frameWidget.text $frameWidget.yscroll -sticky news
	grid $frameWidget.xscroll -sticky news
	grid rowconfigure $frameWidget 0 -weight 1
	grid columnconfigure $frameWidget 0 -weight 1
	return $frameWidget.text
}

proc ListSelectSingle { index listWidget } {
	$listWidget selection clear 0 end
	$listWidget selection anchor $index
	$listWidget selection set anchor
	$listWidget activate anchor
}

proc ListSelectTogether { y args } {
	foreach w $args {
		$w selection clear 0 end
		$w selection anchor [$w nearest $y]
		$w selection set anchor
		$w activate anchor
	}
}

proc ListViewBoth { how listWidgets args } {
	foreach w $listWidgets {
		eval {$w $how} $args
	}
}

proc ListDragToBoth { x y args } {
	foreach w $args {
		$w scan dragto $x $y
	}
}

proc ListMarkBoth { x y args } {
	foreach w $args {
		$w scan mark $x $y
	}
}

proc ListPageUpDown { howMany args } {
	foreach w $args {
		$w yview scroll $howMany pages
	}
}

proc ListUpDown { howMany args } {
	foreach w $args {
		$w yview scroll $howMany units
		set index [expr [$w index active] + $howMany ]
		ListSelectSingle $index $w
	}
}

proc Scrolled_Listbox { frameWidget argsFirst argsSecond args } {
	frame $frameWidget

	listbox $frameWidget.list \
		-xscrollcommand [list Scroll_Set $frameWidget.xscroll \
				 [list grid $frameWidget.xscroll -row 1 \
				  -column 0 -sticky we]] \
		-yscrollcommand [list Scroll_Set $frameWidget.yscroll \
				 [list grid $frameWidget.yscroll -row 0 \
				  -column 2 -sticky ns]]

	listbox $frameWidget.perc  \
		-yscrollcommand [list Scroll_Set $frameWidget.yscroll \
		 		 [list grid $frameWidget.yscroll -row 0 \
				  -column 2 -sticky ns]]
			
	eval { $frameWidget.list configure } $args
	eval { $frameWidget.list configure } $argsFirst
	eval { $frameWidget.perc configure } $args
	eval { $frameWidget.perc configure } $argsSecond

	foreach l [list $frameWidget.list $frameWidget.perc] {
		bind $l <Button-1> \
			[list ListSelectTogether \
				%y $frameWidget.list $frameWidget.perc]
		bind $l <Button-2> \
			[list ListMarkBoth \
				%x %y $frameWidget.list $frameWidget.perc]
		bind $l <B2-Motion> \
			[list ListDragToBoth \
				%x %y $frameWidget.list $frameWidget.perc]
		bind $l <Prior> \
			[list ListPageUpDown \
				-1 $frameWidget.list $frameWidget.perc]
		bind $l <Next> \
			[list ListPageUpDown \
				1 $frameWidget.list $frameWidget.perc]
		bind $l <Key-Up> \
			[list ListUpDown -1 $frameWidget.list $frameWidget.perc]
		bind $l <Key-Down> \
			[list ListUpDown 1 $frameWidget.list $frameWidget.perc]
	}

	scrollbar $frameWidget.xscroll -orient horizontal \
		-command [list $frameWidget.list xview] -bg darkgray
	scrollbar $frameWidget.yscroll -orient vertical \
		-command [list ListViewBoth yview [list $frameWidget.list \
			$frameWidget.perc]] -bg darkgray

	grid $frameWidget.list $frameWidget.perc $frameWidget.yscroll \
		-sticky news
	grid $frameWidget.xscroll -sticky news
	grid rowconfigure $frameWidget 0 -weight 1
	grid columnconfigure $frameWidget 0 -weight 1
	return $frameWidget.list
}

# menu operation related code

proc SetExecutedTextTag { textWidget globalStructure } {
	upvar $globalStructure localgs
	for {set i 0} {$i < [array size localgs]} {incr i} {
		set fileName [lindex $localgs($i) 0]
		$textWidget tag configure executed.$fileName \
			-foreground yellow
	}
}

proc DisplayFilePercentage { listPanel index globalExecution } {
	upvar $globalExecution localge
	$listPanel.perc delete 0 end 
	for {set i 0} {$i < [array size localge]} {incr i} {
		set perc [lindex $localge($i) 1]
		set executed [lindex $localge($i) 2]
		set total [lindex $localge($i) 3]
		set str ""
		append str $perc "% " $executed "/" $total
		$listPanel.perc insert end $str
	}
	if { $index != -1 } {
		ListSelectSingle $index $listPanel.perc
	}
}

proc DisplayFileList \
{ listPanel textWidget index globalStructure globalExecution } \
{
	upvar $globalStructure localgs
	upvar $globalExecution localge 
	$listPanel.list delete 0 end
	for {set i 0} {$i < [array size localgs]} {incr i} {
		set fileName [lindex $localgs($i) 0]
		$listPanel.list insert end $fileName
	}
	if { $index != -1 } {
		ListSelectSingle $index $listPanel.list
	}
	DisplayFilePercentage $listPanel $index localge
}

proc DisplayFuncPercentage { listPanel fileIndex index globalExecution } {
	upvar $globalExecution localge
	$listPanel.perc delete 0 end
	set funcList [lindex $localge($fileIndex) 0]
	foreach l $funcList {
		set perc [lindex $l 0]
		set executed [lindex $l 1]
		set total [lindex $l 2]
		set str ""
		append str $perc "% " $executed "/" $total
		$listPanel.perc insert end $str
	}
	if { $index != -1 } {
		ListSelectSingle $index $listPanel.perc
	}
}

proc DisplayFunctionList \
{ listPanel fileIndex index globalStructure globalExecution } {
	upvar $globalStructure localgs
	upvar $globalExecution localge
	$listPanel.list delete 0 end
	set nameList [lindex $localgs($fileIndex) 1]
	foreach x $nameList {
		$listPanel.list insert end $x
	}
	if { $index != -1 } {
		ListSelectSingle $index $listPanel.list
	}
	DisplayFuncPercentage $listPanel $fileIndex $index localge
}

proc InitializeInterface \
{ listPanel textWidget index globalStructure globalExecution } \
{
	upvar $globalStructure localgs
	upvar $globalExecution localge

	DisplayFileList $listPanel $textWidget $index localgs localge
	SetExecutedTextTag $textWidget localgs
}

proc HighlightExecutedLines { textWidget fileIndex fileName globalExecution } {
	upvar $globalExecution localge
	foreach funcInfo [lindex $localge($fileIndex) 0] {
		set howMany [lindex $funcInfo 1]
		if {$howMany > 0} {
			foreach line [lindex $funcInfo 3] {
				$textWidget tag add executed.$fileName \
					$line.0 $line.end
			}
		}
	}
}

proc UpdateAfterRefresh \
{textWidget messageWidget statusWidget fileFrame funcFrame canvasFrame \
 globalStructure globalExecution globalFrequency } \
{
	upvar $globalStructure localgs
	upvar $globalExecution localge
	upvar $globalFrequency localgf

	global lastFileSelection
	global lastFuncSelection
	global deletionInterval
	global isTerminated
	global alreadyStarted

	if { $alreadyStarted != 1 } {
		return
	}
	if { $deletionInterval > 0 } {
		$canvasFrame.line delete temporary
		$canvasFrame.code delete temporary
		DisplayCoverage $canvasFrame localgf 
	}
	if { $deletionInterval > 0 || $isTerminated == 1 } {

		set currentIndex [lindex [$fileFrame.list curselection] 0]
		DisplayFilePercentage $fileFrame -1 localge
		set firstShowed [$fileFrame.list index @0,0]
		$fileFrame.perc yview $firstShowed 
		ListSelectSingle $currentIndex $fileFrame.perc

		if { $lastFileSelection > -1 } {

			DisplayFuncPercentage \
				$funcFrame $lastFileSelection -1 localge
			set currentIndex [lindex [$funcFrame.list curselection] 0]
			set firstShowed [$funcFrame.list index @0,0]
			$funcFrame.perc yview $firstShowed 
			ListSelectSingle $currentIndex $funcFrame.perc

			set fileName [lindex $localgs($lastFileSelection) 0]
			HighlightExecutedLines \
				$textWidget $lastFileSelection $fileName localge

			set percentage [CalculatePercentage \
						$lastFileSelection -1 localge]
			set str ""
			append str $fileName " \[ " $percentage "% \]"
                	$messageWidget configure -text $str
		}

	}
}

proc DisplayFile \
{ textWidget messageWidget fileName fileIndex globalExecution } {
	upvar $globalExecution localge

	$textWidget configure -state normal
	$textWidget delete 1.0 end
	if [catch {open $fileName r } fileDesc] {
		$messageWidget configure \
			-text "Error : Can not open file $fileName"
	} else {
		set i 1
		while {[gets $fileDesc line] >= 0} {
			$textWidget insert end $i 
			$textWidget insert end "\t"
			$textWidget insert end $line
			$textWidget insert end "\n"
			incr i
		}
		close $fileDesc
		HighlightExecutedLines $textWidget $fileIndex $fileName localge
	}
	$textWidget configure -state disabled
}

proc FileListHandler \
{ filePanel funcPanel textWidget messageWidget globalStructure globalExecution } \
{
	global lastFileSelection
	global lastFuncSelection

	upvar $globalStructure localgs
	upvar $globalExecution localge

	foreach fileIndex [$filePanel.list curselection] {
		if { $fileIndex == $lastFileSelection } {
			break
		}
		set lastFileSelection $fileIndex
		set lastFuncSelection -1

		DisplayFunctionList $funcPanel $fileIndex 0 localgs localge

		DisplayFile $textWidget $messageWidget \
			[$filePanel.list get $fileIndex] $fileIndex localge

		set percentage [CalculatePercentage $fileIndex -1 localge]

		set str ""
		append str [$filePanel.list get $fileIndex] \
			" \[ " $percentage "% \]"
		$messageWidget configure -text $str
	}
}

proc FunctionListHandler \
{ listWidget textWidget messageWidget globalStructure globalExecution} {

	global lastFileSelection
	global lastFuncSelection

	upvar $globalStructure localgs
	upvar $globalExecution localge

	if { $lastFileSelection  == -1 } {
		return;
	}

	set fileEntry $localgs($lastFileSelection)
	set fileName [lindex $fileEntry 0]
	set funcLine [lindex $fileEntry 2]

	foreach i [$listWidget curselection] {
		if { $i == $lastFuncSelection } {
			break
		}

		set lastFuncSelection $i

		set funcName [$listWidget get $i]
		set minLine [lindex $funcLine $i]

		set searchBegin [expr $minLine - 5]
		set searchEnd [expr $minLine + 1]

		if { $searchBegin < 1 } {
			set searchBegin 1
		}

		set matches 0
		set location [$textWidget search -backward -count matches \
			    $funcName $searchEnd.end $searchBegin.0]
		set percentage \
			[CalculatePercentage $lastFileSelection $i localge]

		set str ""
		append str $funcName " \[ " $percentage "% \]\t" \
			   $fileName ":" $minLine
		$messageWidget configure -text $str

		if { $matches > 0 } {
			$textWidget yview $location

			set markBegin [$textWidget bbox $location] 
			tkTextButton1 $textWidget [lindex $markBegin 0] \
				      [lindex $markBegin 1]
			set markEnd [$textWidget bbox "$location wordend"]
			tkTextSelectTo $textWidget [lindex $markEnd 0] \
				       [lindex $markEnd 1]
		} else {
			$textWidget yview $searchBegin.0
		}
	}
}

wm 	title . CodeCoverage
wm	geometry . 1200x850
wm	maxsize . 1250 950
wm	minsize . 1100 825

font 	create comic8 -family comic -size 8
font 	create comic10 -family comic -size 10
font 	create comic12 -family comic -size 12
font 	create comic12bold -family comic -size 12 -weight bold
font 	create comic10bold -family comic -size 10 -weight bold

frame	.fileFrame -borderwidth 0
frame	.menuFrame -borderwidth 4
pack	.fileFrame -side left -fill both -expand true
pack	.menuFrame -side right -fill both

frame	.menuFrame.buttonFrame -borderwidth 0
frame	.menuFrame.listFrame -borderwidth 0
pack	.menuFrame.buttonFrame -side top
pack	.menuFrame.listFrame -side top -expand true

button	.menuFrame.buttonFrame.start -text "Start" -bg darkgreen -fg white \
	-font comic12bold -activebackground green -activeforeground black \
	-relief solid 
button	.menuFrame.buttonFrame.refresh -text "Refresh" -bg darkgreen -fg white \
	-font comic12bold -activebackground green -activeforeground black \
	-relief solid 
button	.menuFrame.buttonFrame.quit -text "Quit" -bg darkgreen -fg white \
	-font comic12bold -activebackground green -activeforeground black \
	-relief solid -command exit
pack	.menuFrame.buttonFrame.start -side left -padx 0 -pady 0
pack	.menuFrame.buttonFrame.refresh -side left -padx 0 -pady 0
pack	.menuFrame.buttonFrame.quit -side left -padx 0 -pady 0

label	.fileFrame.label -text "Source Code Coverage Information" \
	-padx 2 -pady 2 -bg red -fg white -borderwidth 2 \
	-font comic12bold -relief raised
message .fileFrame.message -padx 1 -pady 1 -borderwidth 2 \
	-bg pink -fg darkblue -justify center -relief sunken \
	-aspect 10000 -font comic12
message .fileFrame.status -padx 1 -pady 1 -borderwidth 2 \
	-bg pink -fg darkblue -justify center -relief flat \
	-aspect 10000 -font comic10 -relief sunken \
	-text "Ready to execute mutatee program..."
pack	.fileFrame.label -side top -fill x
pack	.fileFrame.message -side top -fill x

Scrolled_Text \
	.fileFrame.displayPanel -width 80 -height 40 \
	-bg black -fg white -font comic12 -wrap char \
	-exportselection false -relief sunken -borderwidth 2 
pack	.fileFrame.displayPanel -side top -fill both -expand true -padx 2 -pady 5

pack    .fileFrame.status -side top -fill x

.fileFrame.displayPanel.text \
	tag configure sel -background midnightblue -foreground white \
	-relief flat

Scrolled_Listbox .menuFrame.listFrame.fileListFrame \
	[list -width 30] [list -width 10] \
	-height 15 -bg pink -fg darkblue -selectbackground purple \
	-selectforeground white -font comic10 -exportselection false
label	.menuFrame.listFrame.fileLabel -text "Source File List" \
	-padx 0 -bg purple -fg white -font comic12bold

Scrolled_Listbox .menuFrame.listFrame.funcListFrame \
	[list -width 30] [list -width 10] \
	-height 15 -bg pink -fg darkblue -selectbackground blue \
	-selectforeground white -font comic10 -exportselection false
label	.menuFrame.listFrame.funcLabel -text "Function List" \
	-padx 0 -bg blue -fg white -font comic12bold

frame	.menuFrame.listFrame.graphPanel -borderwidth 4 

if { $deletionInterval >  0 } {
	pack .menuFrame.listFrame.graphPanel \
		-padx 0 -fill both -padx 0 -pady 0
} 


# here the canvas related ccode comes which never occurs
# when ther will be no update

label	.menuFrame.listFrame.graphPanel.lineLabel \
	-text "Line Coverage" -pady 2 -bg orange \
	-fg black -font comic10bold -relief ridge
label	.menuFrame.listFrame.graphPanel.codeLabel \
	-text "Intrumentation Coverage" -pady 2 -bg orange \
	-fg black -font comic10bold -relief ridge

canvas	.menuFrame.listFrame.graphPanel.line \
	-highlightthickness 0 -borderwidth 2 -relief sunken \
	-width 200 -height 200 -bg lightyellow
canvas	.menuFrame.listFrame.graphPanel.code \
	-highlightthickness 0 -borderwidth 2 -relief sunken \
	-width 200 -height 200 -bg lightyellow
grid	.menuFrame.listFrame.graphPanel.lineLabel \
	.menuFrame.listFrame.graphPanel.codeLabel -sticky news
grid	.menuFrame.listFrame.graphPanel.line \
	.menuFrame.listFrame.graphPanel.code -sticky news
grid	rowconfigure .menuFrame 0 -weight 1
grid	columnconfigure .menuFrame 0 -weight 1

#end of the canvas code

grid	.menuFrame.listFrame.fileLabel -sticky news
grid	.menuFrame.listFrame.fileListFrame -sticky news
grid	.menuFrame.listFrame.funcLabel -sticky news
grid	.menuFrame.listFrame.funcListFrame -stick news
if { $deletionInterval > 0 } {
	grid .menuFrame.listFrame.graphPanel -stick news
}
grid	rowconfigure .menuFrame 0 -weight 1
grid	columnconfigure .menuFrame 0 -weight 1



if { $deletionInterval > 0 } {
	DisplayAxis .menuFrame.listFrame.graphPanel 
#	DisplayCoverage .menuFrame.listFrame.graphPanel globalFrequencyMap
} else {
	.menuFrame.listFrame.fileListFrame.list configure -height 22
	.menuFrame.listFrame.fileListFrame.perc configure -height 22
	.menuFrame.listFrame.funcListFrame.list configure -height 22
	.menuFrame.listFrame.funcListFrame.perc configure -height 22
}

foreach l [list .menuFrame.listFrame.fileListFrame.list \
		.menuFrame.listFrame.fileListFrame.perc] \
{
	foreach e [list <ButtonRelease-1> <space> ] {
	    bind $l $e { \
		FileListHandler \
			.menuFrame.listFrame.fileListFrame \
			.menuFrame.listFrame.funcListFrame \
			.fileFrame.displayPanel.text \
			.fileFrame.message \
			globalDataStructure \
			globalExecutionMap \
	    }
	}
}

bind	.menuFrame.buttonFrame.refresh <ButtonRelease-1> { \
	UpdateAfterRefresh .fileFrame.displayPanel.text \
		.fileFrame.message \
		.fileFrame.status \
		.menuFrame.listFrame.fileListFrame \
		.menuFrame.listFrame.funcListFrame \
		.menuFrame.listFrame.graphPanel \
		globalDataStructure \
		globalExecutionMap \
		globalFrequencyMap \
}

foreach l [list .menuFrame.listFrame.funcListFrame.list \
		.menuFrame.listFrame.funcListFrame.perc] \
{
	foreach e [list <ButtonRelease-1> <space> ] \
	{
	    bind $l $e { \
		FunctionListHandler .menuFrame.listFrame.funcListFrame.list \
			.fileFrame.displayPanel.text \
			.fileFrame.message \
			globalDataStructure \
			globalExecutionMap \
	    }
	}
}

bind	Listbox <Enter> {focus %W}
bind	Listbox <Key-Up> {break}
bind	Listbox <Key-Down> {break}
bind	Listbox	<Control-Home> {break}
bind	Listbox <Control-End> {break}
bind	Listbox	<Prior> {break}
bind	Listbox <Next> {break}
bind	Listbox <B1-Motion> {break}
bind	Listbox <ButtonRelease-1> {break}

bind	Text <Enter> {focus %W}
bind	Text <Key-Up> {%W yview scroll -1 units; break}
bind	Text <Key-Down> {%W yview scroll 1 units; break}
bind	Text <Key-Left> {%W xview scroll -1 units; break}
bind	Text <Key-Right> {%W xview scroll 1 units; break}
bind	Text <Control-Up> {%W yview scroll -1 pages; break}
bind	Text <Control-Down> {%W yview scroll 1 pages; break}
bind	Text <Button-1> {break}
bind	Text <Double-Button-1> {break}
bind	Text <Triple-Button-1> {break}
bind	Text <B1-Motion> {break;}
