/* $Id: VISIthread.I,v 1.8 1998/04/01 01:27:57 wylie Exp $ */

$remote VISIthread{

  $base 9000;
  $version 5;

  // called by VM thread when a visualization process is stopped by
  // another thread in the system
  void VISIKillVisi();
};
