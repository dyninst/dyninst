# $Id: focusUtils.tcl,v 1.2 1998/03/03 23:09:46 wylie Exp $
# these commands are used to translate between different formats for a focus
#

proc convToPath {lis} {
  return [join /$lis /]
}

proc convToFocus {inp} {

  foreach a $inp {
    lappend lis [convToPath $a]
  }

  set retval <[join $lis ,]>
  return $retval
}

proc convFromPath {path} {
  return [lrange [split $path /] 1 end]  
}

proc convFromFocus {focus} {
  set meat [string range $focus 1 [expr [string length $focus] - 2]]
  set lis [split $meat ,]
  foreach p $lis {
    lappend retval [convFromPath $p]
  }   
  return $retval
}

