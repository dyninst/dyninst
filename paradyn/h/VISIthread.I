/* $Id: VISIthread.I,v 1.7 1998/03/03 23:28:59 wylie Exp $
 */

$remote VISIthread{

  $base 9000;
  $version 4;

  // called by VM thread when a visualization process is stopped by
  // another thread in the system
  void VISIKillVisi();
};
