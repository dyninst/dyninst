/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

// tvCell.C
// Ariel Tamches

/*
 * $Log: tvCell.C,v $
 * Revision 1.4  2004/03/23 01:12:49  eli
 * Updated copyright string
 *
 * Revision 1.3  1996/08/16 21:37:03  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.2  1995/12/29 08:17:59  tamches
 * a cleanup commit: moved body of some member functions from tvCell.h to tvCell.C
 *
 * Revision 1.1  1995/11/04 00:47:22  tamches
 * First version of new table visi
 *
 */

#include "tvCell.h"

bool tvCell::operator<(const tvCell &src) const {
   // let's work out a sorting order for valid v. invalid data right now:
   // let's say that valid data is always "greater than" any invalid data,
   // and that all invalid data are equal.
   if (validData)
      if (src.validData)
         // yea, a sane comparison is possible
         return data < src.data;
      else
         // we are sane but "src" is nuts.  valid data is always > invalid data, so:
         return false;
   else
      if (src.validData)
         // we are nuts; "src" is sane.  valid data > invalid data, so:
         return true;
      else
         // everyone's nuts; all invalid data are equal, so:
         return false;
}

bool tvCell::operator==(const tvCell &src) const {
   if (validData)
      if (src.validData)
         return data == src.data;
      else
         // valid data > invalid data, so:
         return false;
   else
      if (src.validData)
         // valid data > invalid data, so:
         return false;
      else
         // invalid data are equal, so:
         return true;
}

bool tvCell::operator>(const tvCell &src) const {
   if (validData)
      if (src.validData)
         return data > src.data;
      else
         return true; // valid data > invalid data
   else
      if (src.validData)
         return false; // valid data > invalid data
      else
         return false; // invalid == invalid
}

tvCell &tvCell::operator=(const tvCell &src) {
   if (validData = src.validData) // yes, single = on purpose
      data = src.data;
   return *this;
}
