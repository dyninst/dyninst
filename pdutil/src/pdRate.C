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

#ifndef __PDRATE
#define __PDRATE

#include "pdutil/h/pdRate.h"
#include "common/h/int64iostream.h"

const pdRate *pdRate::_zero = NULL;
const pdRate *pdRate::_nan = NULL;

// header file for make_Nan is still makenan.h
// implemented now in pdRate.C, hopefully can go away soon with conversion of
// the visis to use new time and sample value types

float make_Nan()
{
  static float f_paradyn_nan = 0.0;
  static bool nan_created = false;

    if(!nan_created)
    {
        /* Are we little or big endian?  From Harbison&Steele.  */
        union
        {
            unsigned char c[sizeof(long)];
            long l;
        } u;

        u.l = 1;

        union
        {
            unsigned char nan_bytes[4];
            float nan_float;
        } nan_u;

        //
        // Bit pattern for IEEE 754 NaN 32-bit value is 0x7fc00000
        //
        if (u.c[sizeof (long) - 1] == 1)
        {
            // big endian
            nan_u.nan_bytes[0] = 0x7f;
            nan_u.nan_bytes[1] = 0xc0;
            nan_u.nan_bytes[2] = 0x00;
            nan_u.nan_bytes[3] = 0x00;
        }
        else
        {
            // little endian
            nan_u.nan_bytes[0] = 0x00;
            nan_u.nan_bytes[1] = 0x00;
            nan_u.nan_bytes[2] = 0xc0;
            nan_u.nan_bytes[3] = 0x7f;
        }

        f_paradyn_nan = nan_u.nan_float;
        bool aflag=(isnan(f_paradyn_nan)!=0);
        assert(aflag);
        nan_created = true;
    }
    return f_paradyn_nan;
}

const pdRate *pdRate::nanHelp() {
  double dnan = static_cast<double>(make_Nan());
  return new pdRate(dnan);
}

ostream& operator<<(ostream&s, const pdRate &sm) {
  s << "[";
  if(sm.isNaN()) {  s << "NaN"; }
  else           {  s << sm.getValue(); }
  s << "]";
  return s;
}


#endif 
