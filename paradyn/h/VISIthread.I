/* $Log: VISIthread.I,v $
/* Revision 1.4  1994/08/17 17:55:23  markc
/* Incremented version numbers because a new version of igen has been
/* committed.
/*
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
  $version 3;

  void VISIKillVisi();
};
