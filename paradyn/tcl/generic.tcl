#
# Copyright (c) 1996-2004 Barton P. Miller
# 
# We provide the Paradyn Parallel Performance Tools (below
# described as "Paradyn") on an AS IS basis, and do not warrant its
# validity or performance.  We reserve the right to update, modify,
# or discontinue this software at any time.  We shall have no
# obligation to supply such updates or modifications or any other
# form of support to you.
# 
# This license is for research uses.  For such uses, there is no
# charge. We define "research use" to mean you may freely use it
# inside your organization for whatever purposes you see fit. But you
# may not re-distribute Paradyn or parts of Paradyn, in any form
# source or binary (including derivatives), electronic or otherwise,
# to any other organization or entity without our permission.
# 
# (for other uses, please contact us at paradyn@cs.wisc.edu)
# 
# All warranties, including without limitation, any warranty of
# merchantability or fitness for a particular purpose, are hereby
# excluded.
# 
# By your use of Paradyn, you understand and agree that we (or any
# other person or entity with proprietary rights in Paradyn) are
# under no obligation to provide either maintenance services,
# update services, notices of latent defects, or correction of
# defects for Paradyn.
# 
# Even if advised of the possibility of such damages, under no
# circumstances shall we (or any other person or entity with
# proprietary rights in the software licensed hereunder) be liable
# to you or any third party for direct, indirect, or consequential
# damages of any character regardless of type of action, including,
# without limitation, loss of profits, loss of use, loss of good
# will, or computer failure or malfunction.  You agree to indemnify
# us (and any other person or entity with proprietary rights in the
# software licensed hereunder) for any and all liability it may
# incur to third parties resulting from your use of Paradyn.
#

# $Id: generic.tcl,v 1.4 2004/04/13 19:56:35 legendre Exp $
# Routines found useful across all tk4.0 programs

proc resize1Scrollbar {sbname newTotal newVisible} {
   # This is a nice n' generic routine  --ari
   # However, it is (currently) only called from C++ code.  If this
   # situation doesn't change, then we might want to just
   # zap this and turn it into C++ code...

   # 'newTotal' and 'newVisible' are tentative values;
   # We use them to calculate 'newFirst' and 'newLast'.
   # We make an effort to keep 'newFirst' as close as possible to 'oldFirst'.

   set oldConfig [$sbname get]
   set oldFirst  [lindex $oldConfig 0]
   set oldLast   [lindex $oldConfig 1]
#   puts stderr "newTotal=$newTotal; newVisible=$newVisible; oldFirst=$oldFirst; oldLast=$oldLast"

   if {$newVisible < $newTotal} {
      # The usual case: not everything fits
      set fracVisible [expr 1.0 * $newVisible / $newTotal]
#      puts stderr "newVisible=$newVisible; newTotal=$newTotal; fracVisible=$fracVisible"

      set newFirst $oldFirst
      set newLast [expr $newFirst + $fracVisible]

#      puts stderr "tentative newFirst=$newFirst; newLast=$newLast"

      if {$newLast > 1.0} {
         set theOverflow [expr $newLast - 1.0]
#         puts stderr "resize1Scrollbar: would overflow by fraction of $theOverflow; moving newFirst back"

         set newFirst [expr $oldFirst - $theOverflow]
         set newLast  [expr $newFirst + $fracVisible]
      } else {
#         puts stderr "resize1Scrollbar: yea, we were able to keep newFirst unchanged at $newFirst"
      }
   } else {
      # the unusual case: everything fits (visible >= total)
#      puts stderr "everything fits"
      set newFirst 0.0
      set newLast  1.0
   }

   if {$newFirst < 0} {
      # This is an assertion failure
      puts stderr "resize1Scrollbar warning: newFirst is $newFirst"
   }
   if {$newLast > 1} {
      # This is an assertion failure
      # puts stderr "resize1Scrollbar warning: newLast is $newLast"
   }

   $sbname set $newFirst $newLast
   return $newFirst
}
