/*
 * Generate all the templates in one file.
 *
 */

#pragma implementation "Pair.h"
#include "util/h/Pair.h"

#pragma implementation "Vector.h"
#include "util/h/Vector.h"

#pragma implementation "Dictionary.h"
#include "util/h/Dictionary.h"

#pragma implementation "list.h"
#include "util/h/list.h"

#pragma implementation "dyninstRPC.xdr.h"
#include "dyninstRPC.xdr.h"

#pragma implementation "Symbol.h"
#include "util/h/Symbol.h"

#include "util/h/String.h"

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
template bool_t T_dyninstRPC_P_xdr_stl(XDR*, vector<T_dyninstRPC::mdl_rand*>*,
			      bool_t (*)(XDR*, T_dyninstRPC::mdl_rand**),
			      T_dyninstRPC::mdl_rand**);
template bool_t T_dyninstRPC_P_xdr_stl(XDR*, vector<T_dyninstRPC::mdl_instr_rand*>*,
			       bool_t (*)(XDR*, T_dyninstRPC::mdl_instr_rand**),
			       T_dyninstRPC::mdl_instr_rand**);

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

template bool_t T_dyninstRPC_P_xdr_stl_PTR(XDR*, vector<T_dyninstRPC::mdl_rand*>**,
				      bool_t (*)(XDR*, T_dyninstRPC::mdl_rand**),
				      T_dyninstRPC::mdl_rand**);
template bool_t T_dyninstRPC_P_xdr_stl_PTR(XDR*, vector<T_dyninstRPC::mdl_instr_rand*>**,
                        	      bool_t (*)(XDR*, T_dyninstRPC::mdl_instr_rand**),
				      T_dyninstRPC::mdl_instr_rand**);



// added for batchSampleDataCallbackFunc
template bool T_dyninstRPC_P_xdr_stl(XDR *, vector<T_dyninstRPC::batch_buffer_entry> *, int (*)(XDR *, T_dyninstRPC::batch_buffer_entry *), T_dyninstRPC::batch_buffer_entry *);
T_dyninstRPC_P_xdr_stl(XDR *, vector<T_dyninstRPC::batch_buffer_entry> *, int (*)(XDR *, T_dyninstRPC::batch_buffer_entry *), T_dyninstRPC::batch_buffer_entry *);
template class vector<T_dyninstRPC::batch_buffer_entry>;

