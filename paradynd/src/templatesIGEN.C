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

/*
 * $Id: templatesIGEN.C,v 1.9 2000/07/27 15:24:47 hollings Exp $
 * Generate all the templates in one file.
 */

#pragma implementation "Pair.h"
#include "common/h/Pair.h"

#pragma implementation "Vector.h"
#include "common/h/Vector.h"

#pragma implementation "Dictionary.h"
#include "common/src/Dictionary.C"

#pragma implementation "list.h"
#include "common/h/list.h"

#pragma implementation "dyninstRPC.xdr.h"
#include "dyninstRPC.xdr.h"

#pragma implementation "Symbol.h"
#include "common/h/Symbol.h"

#include "common/h/String.h"

template bool_t T_dyninstRPC_P_xdr_stl(XDR*, vector<u_int>*,
                                       bool_t (*)(XDR*, u_int*), u_int*);

template bool_t T_dyninstRPC_P_xdr_stl(XDR*, vector<int>*,
				       bool_t (*)(XDR*, int*), int*);

template bool_t T_dyninstRPC_P_xdr_stl(XDR*, vector<string>*,
				       bool_t (*)(XDR*, string*), string*);
template bool_t T_dyninstRPC_P_xdr_stl_PTR(XDR*, vector<string>**,
					   bool_t (*)(XDR*, string*), string*);

template bool_t T_dyninstRPC_P_xdr_stl(XDR*, vector<T_dyninstRPC::metricInfo>*,
				       bool_t (*)(XDR*, T_dyninstRPC::metricInfo*),
				       T_dyninstRPC::metricInfo*);

template bool_t T_dyninstRPC_P_xdr_stl(XDR*,
				       vector<T_dyninstRPC::focusStruct>*,
				       bool_t (*)(XDR*, T_dyninstRPC::focusStruct*),
				       T_dyninstRPC::focusStruct*);

template bool_t T_dyninstRPC_P_xdr_stl(XDR*, vector<T_dyninstRPC::mdl_expr*>*,
				       bool_t (*)(XDR*, T_dyninstRPC::mdl_expr**),
				       T_dyninstRPC::mdl_expr**);
template bool_t T_dyninstRPC_P_xdr_stl(XDR*, vector<T_dyninstRPC::mdl_stmt*>*,
				       bool_t (*)(XDR*, T_dyninstRPC::mdl_stmt**),
				       T_dyninstRPC::mdl_stmt**);
template bool_t T_dyninstRPC_P_xdr_stl(XDR*, vector<T_dyninstRPC::mdl_icode*>*,
				       bool_t (*)(XDR*, T_dyninstRPC::mdl_icode**),
				       T_dyninstRPC::mdl_icode**);
template bool_t T_dyninstRPC_P_xdr_stl(XDR*, vector<T_dyninstRPC::mdl_constraint*>*,
				       bool_t (*)(XDR*, T_dyninstRPC::mdl_constraint**),
				       T_dyninstRPC::mdl_constraint**);
template bool_t T_dyninstRPC_P_xdr_stl(XDR*, vector<T_dyninstRPC::mdl_metric*>*,
				       bool_t (*)(XDR*, T_dyninstRPC::mdl_metric**),
				       T_dyninstRPC::mdl_metric**);

template bool_t T_dyninstRPC_P_xdr_stl_PTR(XDR*, vector<T_dyninstRPC::mdl_expr*>**,
					   bool_t (*)(XDR*, T_dyninstRPC::mdl_expr**),
					   T_dyninstRPC::mdl_expr**);
template bool_t T_dyninstRPC_P_xdr_stl_PTR(XDR*, vector<T_dyninstRPC::mdl_stmt*>**,
					   bool_t (*)(XDR*, T_dyninstRPC::mdl_stmt**),
					   T_dyninstRPC::mdl_stmt**);
template bool_t T_dyninstRPC_P_xdr_stl_PTR(XDR*, vector<T_dyninstRPC::mdl_icode*>**,
					   bool_t (*)(XDR*, T_dyninstRPC::mdl_icode**),
					   T_dyninstRPC::mdl_icode**);
template bool_t T_dyninstRPC_P_xdr_stl_PTR(XDR*, vector<T_dyninstRPC::mdl_constraint*>**,
					   bool_t (*)(XDR*, T_dyninstRPC::mdl_constraint**),
					   T_dyninstRPC::mdl_constraint**);
template bool_t T_dyninstRPC_P_xdr_stl_PTR(XDR*, vector<T_dyninstRPC::mdl_metric*>**,
					   bool_t (*)(XDR*, T_dyninstRPC::mdl_metric**),
					   T_dyninstRPC::mdl_metric**);

template bool_t T_dyninstRPC_P_xdr_stl(XDR *, vector<T_dyninstRPC::resourceInfoCallbackStruct> *, int (*)(XDR *, T_dyninstRPC::resourceInfoCallbackStruct *), T_dyninstRPC::resourceInfoCallbackStruct *);

template class vector<T_dyninstRPC::resourceInfoCallbackStruct>;


// added for batchSampleDataCallbackFunc
template bool_t T_dyninstRPC_P_xdr_stl(XDR *, vector<T_dyninstRPC::batch_buffer_entry> *, int (*)(XDR *, T_dyninstRPC::batch_buffer_entry *), T_dyninstRPC::batch_buffer_entry *);
int T_dyninstRPC_P_xdr_stl(XDR *, vector<T_dyninstRPC::batch_buffer_entry> *, int (*)(XDR *, T_dyninstRPC::batch_buffer_entry *), T_dyninstRPC::batch_buffer_entry *);
template class vector<T_dyninstRPC::batch_buffer_entry>;

// added for batchTraceDataCallbackFunc
template bool_t T_dyninstRPC_P_xdr_stl(XDR *, vector<T_dyninstRPC::trace_batch_buffer_entry> *, int (*)(XDR *, T_dyninstRPC::trace_batch_buffer_entry *), T_dyninstRPC::trace_batch_buffer_entry *);
int T_dyninstRPC_P_xdr_stl(XDR *, vector<T_dyninstRPC::trace_batch_buffer_entry> *, int (*)(XDR *, T_dyninstRPC::trace_batch_buffer_entry *), T_dyninstRPC::trace_batch_buffer_entry *);
template class vector<T_dyninstRPC::trace_batch_buffer_entry>;
