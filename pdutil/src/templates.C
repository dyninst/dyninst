/*
 * Copyright (c) 1996-2003 Barton P. Miller
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
 * Generate code for template classes used by libpdutil
 * $Id: templates.C,v 1.22 2003/10/24 18:25:19 pcroth Exp $
 */

#include "common/src/Dictionary.C"
#include "common/h/Vector.h"
#include "pdutil/src/PriorityQueue.C"
#include "common/h/Time.h"
#include "pdutil/h/pdSample.h"
#include "pdutil/h/rpcUtil.h"

#if defined( i386_unknown_linux2_0 )
#if ( __GNUC__ == 3 ) && ( __GNUC_MINOR__ == 1 )
template void std::__pad<char, std::char_traits<char> >(std::ios_base&, char, char*, char const*, int, int, bool);
#endif
#endif
template class PriorityQueue<timeStamp, pdSample>;
template class pdvector<RPCSockCallbackFunc>;

// MDL template support
#include "common/h/Vector.h"
#include "pdutil/h/mdlParse.h"
#include "dyninstRPC.xdr.h"

class daemonMet;
class processMet;
class visiMet;
class tunableMet;
class string_list;

template class pdvector<functionName*>;
template class pdvector<processMet *>;
template class pdvector<daemonMet*>;
template class pdvector<visiMet*>;
template class pdvector<tunableMet*>;
template class pdvector<string_list*>;
template class pdvector<T_dyninstRPC::mdl_metric*>;


#if defined(rs6000_ibm_aix4_1)
// AIX seems to need explicit instantiations of these to link
// progmams like the visis and termWin that use pdutil but don't
// use the MDL parser
template class pdvector<mdl_type_desc>;
template class dictionary_hash<unsigned int, pdvector<mdl_type_desc> >;
template class pdvector<dictionary_hash<unsigned int, pdvector<mdl_type_desc> >::entry>;
#endif // defined(rs6000_ibm_aix4_1)

