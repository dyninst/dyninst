/*
 * Copyright (c) 1996-1999 Barton P. Miller
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

/*
 * $Log: templates.C,v $
 * Revision 1.12  2003/07/15 22:48:00  schendel
 * rename string to pdstring
 *
 * Revision 1.11  2002/12/20 07:50:09  jaw
 * This commit fully changes the class name of "vector" to "pdvector".
 *
 * A nice upshot is the removal of a bunch of code previously under the flag
 * USE_STL_VECTOR, which is no longer necessary in many cases where a
 * functional difference between common/h/Vector.h and stl::vector was
 * causing a crash.
 *
 * Generally speaking, Dyninst and Paradyn now use pdvector exclusively.
 * This commit DOES NOT cover the USE_STL_VECTOR flag, which will now
 * substitute stl::vector for BPatch_Vector only.  This is currently, to
 * the best of my knowledge, only used by DPCL.  This will be updated and
 * tested in a future commit.
 *
 * The purpose of this, again, is to create a further semantic difference
 * between two functionally different classes (which both have the same
 * [nearly] interface).
 *
 * Revision 1.10  2002/04/09 18:06:42  mjbrim
 * Updates to allow sharing of common/pdutil/igen between Paradyn
 * & Kerninst, which in turn allows them to share binary visis  - - - - - -
 * added vector and pair instantiations
 *
 * Revision 1.9  2000/07/27 17:42:43  pcroth
 * Updated #includes to reflect new locations of util headers
 *
 * Revision 1.8  1999/03/13 15:23:59  pcroth
 * Added support for building under Windows NT
 *
 * Revision 1.7  1997/10/29 03:38:21  tamches
 * fix to previous commit
 *
 * Revision 1.6  1997/10/28 20:45:22  tamches
 * update for new dictionary class
 *
 * Revision 1.5  1997/09/05 22:20:33  naim
 * Changes to template files to be able to compile paradyn with -O3 - naim
 *
 * Revision 1.4  1997/04/30 15:37:16  mjrg
 * added template needed for new implementation of string class
 *
 * Revision 1.3  1996/08/16 21:35:49  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.2  1996/01/17 18:32:00  newhall
 * changes due to new visiLib
 *
 * Revision 1.1  1995/12/15 22:01:55  tamches
 * first version of phaseTable
 *
 */

// logo stuff:
#include "paradyn/src/UIthread/minmax.C"
template float max(const float, const float);

#if !defined(i386_unknown_nt4_0)

#pragma implementation "Vector.h"
#include "common/h/Vector.h"
#include "visi/h/visiTypes.h"

class PhaseInfo;
template class pdvector<PhaseInfo *>;

#include "common/h/String.h"
#include "common/src/Dictionary.C"
#include "pdLogo.h"

template class pdpair<pdstring, pdLogo *>;
template class pdpair<pdstring, pdLogo::logoStruct>;
template class pdvector<pdpair<pdstring, pdLogo *> >;
template class pdvector<pdpair<pdstring, pdLogo::logoStruct> >;

template class dictionary_hash<pdstring, pdLogo *>;
template class pdvector<dictionary_hash<pdstring, pdLogo *>::entry>;
template class pdvector<pdLogo*>;

template class dictionary_hash<pdstring, pdLogo::logoStruct>;
template class pdvector<dictionary_hash<pdstring, pdLogo::logoStruct>::entry>;
template class pdvector<pdLogo::logoStruct>;

template class pdvector<unsigned int>;
template class refCounter<string_ll>;

#endif //!defined(i386_unknown_nt4_0)
