#this file is sourced by UIMain to perform any necessary tcl initialization
# for paradyn
# $Log: pdStartup.tcl,v $
# Revision 1.1  1994/05/05 19:55:49  karavan
# initial version.
#

#this tells tcl where to look for tcl procedures referenced in paradyn code

set auto_path [linsert $auto_path 0 \
	       /usr/home/paradyn/development/karavan/core/paradyn/tcl]
 
