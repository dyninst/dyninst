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

//
// Igen and MDL templates
//

#include "common/h/String.h"

#pragma implementation "Vector.h"
#include "common/h/Vector.h"

#pragma implementation "Dictionary.h"
#include "common/src/Dictionary.C"

#pragma implementation "dyninstRPC.xdr.h"
#include "dyninstRPC.xdr.h"
#pragma implementation "visi.xdr.h"
#include "visi.xdr.h"

#if !defined(i386_unknown_nt4_0)
#pragma implementation "termWin.xdr.h"
#include "termWin.xdr.h"
#endif // !defined(i386_unknown_nt4_0)

template class pdvector< pdvector<string> >;
template class refCounter<string_ll>;

/* ***********************************
 * met stuff
 */
#include "pdutil/h/mdlParse.h" 
#include "paradyn/src/met/mdl.h" 

class stringList;
class daemonMet;
class processMet;
class visiMet;
class tunableMet;

template class pdvector<functionName*>;
template class pdvector<processMet *>;
template class pdvector<daemonMet*>;
template class pdvector<visiMet*>;
template class pdvector<tunableMet*>;
template class pdvector<string_list*>;
template class pdvector<function_base*>;
template class pdvector<module*>;
template class pdvector<instPoint*>;

// MDL stuff 

template class pdvector<pd_process *>;
template class pdvector< pdvector<dyn_thread *> >;
template class pdvector<const instrDataNode *>;
template class pdvector<mdl_var>;
template class pdvector<mdl_focus_element>;
template class pdvector<mdl_env::Frame>;

template class pdvector<mdl_type_desc>;
template class pdvector< pdvector< mdl_type_desc > >;

template class dictionary_hash<unsigned, pdvector<mdl_type_desc> >;
template class pdvector<dictionary_hash<unsigned, pdvector<mdl_type_desc> >::entry>;

// Igen - dyninstRPC stuff
// no longer need to instantiate here, now done in dyninstRPC.xdr.temp.C
// except for these which are needed only for nt builds

#if defined(i386_unknown_nt4_0)
template bool writerfn_noMethod<int>(struct XDR *, const int);
template bool writerfn_noMethod<unsigned>(struct XDR *, const unsigned);
template bool writerfn_noMethod<string>(struct XDR *, const string);
template bool writerfn_noMethod<T_dyninstRPC::mdl_expr *>(struct XDR *, T_dyninstRPC::mdl_expr * const);
template bool writerfn_noMethod<T_dyninstRPC::mdl_stmt *>(struct XDR *, T_dyninstRPC::mdl_stmt * const);
template bool writerfn_noMethod<T_dyninstRPC::mdl_icode *>(struct XDR *, T_dyninstRPC::mdl_icode * const);
template bool writerfn_noMethod<T_dyninstRPC::mdl_constraint *>(struct XDR *, T_dyninstRPC::mdl_constraint * const);
template bool writerfn_noMethod<T_dyninstRPC::mdl_metric *>(struct XDR *, T_dyninstRPC::mdl_metric * const);
template bool writerfn_noMethod<T_dyninstRPC::batch_buffer_entry>(struct XDR *, const T_dyninstRPC::batch_buffer_entry);
template bool writerfn_noMethod<T_dyninstRPC::trace_batch_buffer_entry>(struct XDR *, const T_dyninstRPC::trace_batch_buffer_entry);
template bool writerfn_noMethod<T_dyninstRPC::focusStruct>(struct XDR *, const T_dyninstRPC::focusStruct);
template bool writerfn_noMethod<T_dyninstRPC::indivInstResponse>(struct XDR *, const T_dyninstRPC::indivInstResponse);
#endif

// Igen - visi stuff
// no longer need to instantiate here, now done in visi.xdr.temp.C
// except for these which are needed only for nt builds

#if defined(i386_unknown_nt4_0)
template bool writerfn_noMethod<float>(struct XDR *, const float);
template bool writerfn_noMethod<T_visi::dataValue>(struct XDR *, const T_visi::dataValue);
template bool writerfn_noMethod<T_visi::traceDataValue>(struct XDR *, const T_visi::traceDataValue);
template bool writerfn_noMethod<T_visi::visi_matrix>(struct XDR *, const T_visi::visi_matrix);
template bool writerfn_noMethod<T_visi::phase_info>(struct XDR *, const T_visi::phase_info);
#endif

#if !defined(i386_unknown_nt4_0)
// termWin igen interface template instantiations
template class pdvector<T_termWin::buf_struct*>;
#endif // !defined(i386_unknown_nt4_0)

