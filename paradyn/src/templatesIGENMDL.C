//
// igen and MDL templates
//


#include "util/h/String.h"
#pragma implementation "Vector.h"
#include "util/h/Vector.h"
#pragma implementation "Queue.h"
#include "util/h/Queue.h"
#pragma implementation "Dictionary.h"
#include "util/h/Dictionary.h"
#pragma implementation "dyninstRPC.xdr.h"
#include "dyninstRPC.xdr.h"
#pragma implementation "visi.xdr.h"
#include "visi.xdr.h"

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

template class vector<processMet *>;
template class vector<daemonMet*>;
template class vector<visiMet*>;
template class vector<tunableMet*>;
template class vector<string_list*>;
template class vector<pdFunction*>;
template class vector<module*>;
template class vector<instPoint*>;

// Igen - dyninstRPC stuff

template class vector<T_dyninstRPC::buf_struct*>;
template class queue<T_dyninstRPC::buf_struct*>;
template class vector<string>;
template class vector<T_dyninstRPC::mdl_expr*>;
template class vector<T_dyninstRPC::mdl_stmt*>;
template class vector<T_dyninstRPC::mdl_icode*>;
template class vector<T_dyninstRPC::mdl_constraint*>;
template class vector<T_dyninstRPC::metricInfo>;
template class vector<T_dyninstRPC::mdl_metric*>;
template bool_t T_dyninstRPC_P_xdr_stl(XDR*, vector<string>*, 
	bool_t (*)(XDR*, string*), string*);
template bool_t T_dyninstRPC_P_xdr_stl(XDR*, vector<T_dyninstRPC::mdl_expr*>*,
	bool_t (*)(XDR*, T_dyninstRPC::mdl_expr**), T_dyninstRPC::mdl_expr**);
template bool_t T_dyninstRPC_P_xdr_stl(XDR*, vector<u_int>*, 
	bool_t (*)(XDR*, u_int*), u_int*);
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
	vector<T_dyninstRPC::mdl_metric*>**, 
	bool_t (*)(XDR*, T_dyninstRPC::mdl_metric**), 
	T_dyninstRPC::mdl_metric**);


// MDL stuff 

template class vector<dataReqNode *>;
template class vector<mdl_var>;
template class vector<mdl_focus_element>;
template class vector<mdl_type_desc>;
template class pair< unsigned, vector<mdl_type_desc> >;
template class vector< pair< unsigned, vector<mdl_type_desc> > >;
template class dictionary<unsigned, vector<mdl_type_desc> >;
template class dictionary_hash<unsigned, vector<mdl_type_desc> >;
template class vector< vector< mdl_type_desc > >;
template class vector< dictionary_hash< unsigned, vector<mdl_type_desc> > :: hash_pair >;
template class vector< vector< dictionary_hash<unsigned, vector<mdl_type_desc> > :: hash_pair > >;

template bool_t T_dyninstRPC_P_xdr_stl(XDR*, vector<u_int>*,
			       bool_t (*)(XDR*, u_int*), u_int*);

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

// Igen - visi stuff

template class vector<T_visi::buf_struct*>;
template class queue<T_visi::buf_struct*>;
template class vector<T_visi::dataValue>;
template class vector<T_visi::visi_matrix>;
template class vector<T_visi::phase_info>;
template class vector<float>;
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

