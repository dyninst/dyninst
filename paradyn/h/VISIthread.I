/* $Log: VISIthread.I,v $
/* Revision 1.6  1995/02/26 02:11:51  newhall
/* added some phase interface routines
/*
# Revision 1.5  1994/09/22  00:41:35  markc
# Incremented version number because igen is new
#
# Revision 1.4  1994/08/17  17:55:23  markc
# Incremented version numbers because a new version of igen has been
# committed.
#
# Revision 1.3  1994/08/13  20:50:16  newhall
# *** empty log message ***
#
# Revision 1.2  1994/04/09  21:26:06  newhall
# change VISIKillVisi to async
#
# Revision 1.1  1994/03/30  05:41:55  newhall
# exported services of visualization threads of paradyn
# */

$remote VISIthread{

  $base 9000;
  $version 4;

  // called by VM thread when a visualization process is stopped by
  // another thread in the system
  void VISIKillVisi();
};
