/* $Log: VISIthread.I,v $
/* Revision 1.2  1994/04/09 21:26:06  newhall
/* change VISIKillVisi to async
/*
# Revision 1.1  1994/03/30  05:41:55  newhall
# exported services of visualization threads of paradyn
# */

$remote VISIthread{

  $base 9000;
  $version 1;

  void VISIKillVisi();
};
