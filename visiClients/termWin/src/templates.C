/*
 * Copyright (c) 1996-2001 Barton P. Miller
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
 * $Id: templates.C,v 1.5 2003/07/15 22:48:10 schendel Exp $
 */

#pragma implementation "Vector.h"
#include "common/h/Vector.h"
//#include "visi/h/visiTypes.h"
//

#pragma implementation "termWin.xdr.h"
#include "termWin.xdr.h"

class clientConn;

template class pdvector<clientConn *>;

// logo stuff:
#include "paradyn/src/UIthread/minmax.C"
template float max(const float, const float);

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

// termWin igen interface template instantiations
template class pdvector<T_termWin::buf_struct*>;

