/*
 * Copyright (c) 1996 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
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

// $Id: headers.h,v 1.16 2001/08/01 15:39:50 chadd Exp $

#ifndef KLUDGES_H
#define KLUDGES_H

/*
 * Kludges to handle broken system includes and such...
 */

#ifndef mips_unknown_ce2_11 //ccw 8 apr 2001

extern "C" {
typedef int (*xdr_rd_func)(void *, char *, int);
typedef int (*xdr_wr_func)(void *, char *, int);
typedef int (*const_xdr_rd_func)(const void *, char *, const int);
typedef int (*const_xdr_wr_func)(const void *, const char *, const int);
}
#endif

#include "common/h/Types.h"

#if defined(sparc_sun_solaris2_4)
#include "common/h/solarisHeaders.h"

#elif defined(i386_unknown_solaris2_5)
#include "common/h/solarisHeaders.h"

#elif defined(i386_unknown_linux2_0)
#include "common/h/linuxHeaders.h"

#elif defined(rs6000_ibm_aix4_1)
#include "common/h/aixv41Headers.h"

#elif defined(alpha_dec_osf4_0)
#include "common/h/osfHeaders.h"

#elif defined(i386_unknown_nt4_0) || defined (mips_unknown_ce2_11) //ccw 20 july 2000 : 29 mar 2001
#include "common/h/ntHeaders.h"

#elif defined(mips_sgi_irix6_4)
#include "common/h/irixHeaders.h"

#endif  /* architecture specific */

#endif /* KLUDGES_H */

