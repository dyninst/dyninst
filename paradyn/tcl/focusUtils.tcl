#these commands are used to translate between different formats for 
# a focus

# $Log: focusUtils.tcl,v $
# Revision 1.1  1994/05/03 06:35:58  karavan
# Initial version.
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

