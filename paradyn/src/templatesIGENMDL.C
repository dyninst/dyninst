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
// igen and MDL templates
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

/* ***********************************
 * met stuff
 */
#include "paradyn/src/met/metParse.h" 
#include "paradyn/src/met/mdl.h" 

class stringList;
class daemonMet;
class processMet;
class visiMet;
class tunableMet;

template class vector<functionName*>;
template class vector<processMet *>;
template class vector<daemonMet*>;
template class vector<visiMet*>;
template class vector<tunableMet*>;
template class vector<string_list*>;
template class vector<function_base*>;
template class vector<module*>;
template class vector<instPoint*>;

// Igen - dyninstRPC stuff

template class vector<T_dyninstRPC::buf_struct*>;
template class vector<string>;
template class vector<T_dyninstRPC::mdl_expr*>;
template class vector<T_dyninstRPC::mdl_stmt*>;
template class vector<T_dyninstRPC::mdl_icode*>;
template class vector<T_dyninstRPC::mdl_constraint*>;
template class vector<T_dyninstRPC::metricInfo>;
template class vector<T_dyninstRPC::focusStruct>;
template class vector<T_dyninstRPC::mdl_metric*>;
template bool_t T_dyninstRPC_P_xdr_stl(XDR*, vector<string>*, 
	bool_t (*)(XDR*, string*), string*);
template bool_t T_dyninstRPC_P_xdr_stl(XDR*, vector<T_dyninstRPC::mdl_expr*>*,
	bool_t (*)(XDR*, T_dyninstRPC::mdl_expr**), T_dyninstRPC::mdl_expr**);
template bool_t T_dyninstRPC_P_xdr_stl(XDR*, vector<u_int>*, 
	bool_t (*)(XDR*, u_int*), u_int*);
template bool_t T_dyninstRPC_P_xdr_stl(XDR*, vector<int>*, 
	bool_t (*)(XDR*, int*), int*);
template bool_t T_dyninstRPC_P_xdr_stl(XDR*, vector<T_dyninstRPC::mdl_stmt*>*,
	bool_t (*)(XDR*, T_dyninstRPC::mdl_stmt**), T_dyninstRPC::mdl_stmt**);
template bool_t T_dyninstRPC_P_xdr_stl(XDR*, vector<T_dyninstRPC::mdl_icode*>*,
	bool_t (*)(XDR*, T_dyninstRPC::mdl_icode**), T_dyninstRPC::mdl_icode**);
template bool_t T_dyninstRPC_P_xdr_stl(XDR*, 
	vector<T_dyninstRPC::mdl_constraint*>*, 
	bool_t (*)(XDR*, T_dyninstRPC::mdl_constraint**), 
	T_dyninstRPC::mdl_constraint**);
template bool_t T_dyninstRPC_P_xdr_stl(XDR*, vector<T_dyninstRPC::metricInfo>*,
	bool_t (*)(XDR*, T_dyninstRPC::metricInfo*), T_dyninstRPC::metricInfo*);

template bool_t T_dyninstRPC_P_xdr_stl(XDR*, vector<T_dyninstRPC::focusStruct>*,
	bool_t (*)(XDR*, T_dyninstRPC::focusStruct*), T_dyninstRPC::focusStruct*);

template bool_t T_dyninstRPC_P_xdr_stl(XDR*, vector<T_dyninstRPC::mdl_metric*>*,
 	bool_t (*)(XDR*, T_dyninstRPC::mdl_metric**), 
	T_dyninstRPC::mdl_metric**);
template bool_t T_dyninstRPC_P_xdr_stl_PTR(XDR*, vector<string>**, 
	bool_t (*)(XDR*, string*), string*);
template bool_t T_dyninstRPC_P_xdr_stl_PTR(XDR*, 
	vector<T_dyninstRPC::mdl_expr*>**, 
	bool_t (*)(XDR*, T_dyninstRPC::mdl_expr**), T_dyninstRPC::mdl_expr**);
template bool_t T_dyninstRPC_P_xdr_stl_PTR(XDR*, 
	vector<u_int>**, bool_t (*)(XDR*, u_int*), u_int*);
template bool_t T_dyninstRPC_P_xdr_stl_PTR(XDR*, 
	vector<int>**, bool_t (*)(XDR*, int*), int*);
template bool_t T_dyninstRPC_P_xdr_stl_PTR(XDR*, 
	vector<T_dyninstRPC::mdl_stmt*>**, 
	bool_t (*)(XDR*, T_dyninstRPC::mdl_stmt**), 
	T_dyninstRPC::mdl_stmt**);
template bool_t T_dyninstRPC_P_xdr_stl_PTR(XDR*, 
	vector<T_dyninstRPC::mdl_icode*>**, 
	bool_t (*)(XDR*, T_dyninstRPC::mdl_icode**), 
	T_dyninstRPC::mdl_icode**);
template bool_t T_dyninstRPC_P_xdr_stl_PTR(XDR*, 
	vector<T_dyninstRPC::mdl_constraint*>**, 
	bool_t (*)(XDR*, T_dyninstRPC::mdl_constraint**), 
	T_dyninstRPC::mdl_constraint**);
template bool_t T_dyninstRPC_P_xdr_stl_PTR(XDR*, 
	vector<T_dyninstRPC::metricInfo>**, 
	bool_t (*)(XDR*, T_dyninstRPC::metricInfo*), 
	T_dyninstRPC::metricInfo*);

template bool_t T_dyninstRPC_P_xdr_stl_PTR(XDR*, 
	vector<T_dyninstRPC::focusStruct>**, 
	bool_t (*)(XDR*, T_dyninstRPC::focusStruct*), 
	T_dyninstRPC::focusStruct*);

template bool_t T_dyninstRPC_P_xdr_stl_PTR(XDR*, 
	vector<T_dyninstRPC::resourceInfoCallbackStruct>**, 
	bool_t (*)(XDR*, T_dyninstRPC::resourceInfoCallbackStruct*), 
	T_dyninstRPC::resourceInfoCallbackStruct*);

template bool_t T_dyninstRPC_P_xdr_stl_PTR(XDR*, 
	vector<T_dyninstRPC::mdl_metric*>**, 
	bool_t (*)(XDR*, T_dyninstRPC::mdl_metric**), 
	T_dyninstRPC::mdl_metric**);


template bool_t T_dyninstRPC_P_xdr_stl(XDR *, vector<T_dyninstRPC::resourceInfoCallbackStruct> *, int (*)(XDR *, T_dyninstRPC::resourceInfoCallbackStruct *), T_dyninstRPC::resourceInfoCallbackStruct *);

// MDL stuff 

template class vector<process *>;
template class vector<pdThread *>;
template class vector< vector<pdThread *> >;
template class vector<const dataReqNode *>;
template class vector<mdl_var>;
template class vector<mdl_focus_element>;
template class vector<mdl_type_desc>;

template class dictionary_hash<unsigned, vector<mdl_type_desc> >;
template class vector<dictionary_hash<unsigned, vector<mdl_type_desc> >::entry>;
template class vector< vector< mdl_type_desc > >;

// Igen - visi stuff

template class vector<T_visi::buf_struct*>;
template class vector<T_visi::dataValue>;
template class vector<T_visi::visi_matrix>;
template class vector<T_visi::phase_info>;
template class vector<float>;
// trace data streams
template class vector<T_visi::traceDataValue>;
template bool_t T_visi_P_xdr_stl(XDR*, vector<string>*, 
	bool_t (*)(XDR*, string*), string*);
template bool_t T_visi_P_xdr_stl(XDR*, vector<T_visi::dataValue>*, 
	bool_t (*)(XDR*, T_visi::dataValue*), T_visi::dataValue*);
template bool_t T_visi_P_xdr_stl(XDR*, vector<T_visi::visi_matrix>*,
	bool_t (*)(XDR*, T_visi::visi_matrix*), T_visi::visi_matrix*);
template bool_t T_visi_P_xdr_stl(XDR*, vector<T_visi::phase_info>*, 
	bool_t (*)(XDR*, T_visi::phase_info*), T_visi::phase_info*);
template bool_t T_visi_P_xdr_stl(XDR*, vector<float>*, 
	bool_t (*)(XDR*, float*), float*);
template bool_t T_visi_P_xdr_stl(XDR*, vector<T_visi::traceDataValue>*,
        bool_t (*)(XDR*, T_visi::traceDataValue*), T_visi::traceDataValue*);
template bool_t T_visi_P_xdr_stl_PTR(XDR*, vector<string>**, 
	bool_t (*)(XDR*, string*), string*);
template bool_t T_visi_P_xdr_stl_PTR(XDR*, vector<T_visi::dataValue>**, 
	bool_t (*)(XDR*, T_visi::dataValue*), T_visi::dataValue*);
template bool_t T_visi_P_xdr_stl_PTR(XDR*, vector<T_visi::visi_matrix>**,
	bool_t (*)(XDR*, T_visi::visi_matrix*), T_visi::visi_matrix*);
template bool_t T_visi_P_xdr_stl_PTR(XDR*, vector<T_visi::phase_info>**, 
	bool_t (*)(XDR*, T_visi::phase_info*), T_visi::phase_info*);
template bool_t T_visi_P_xdr_stl_PTR(XDR*, vector<float>**, 
	bool_t (*)(XDR*, float*), float*);
template bool_t T_visi_P_xdr_stl_PTR(XDR*, vector<T_visi::traceDataValue>**,
        bool_t (*)(XDR*, T_visi::traceDataValue*), T_visi::traceDataValue*);


#if !defined(i386_unknown_nt4_0)
// termWin igen interface template instantiations
template class vector<T_termWin::buf_struct*>;
#endif // !defined(i386_unknown_nt4_0)

template class refCounter<string_ll>;
