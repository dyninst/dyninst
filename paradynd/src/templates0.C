/*
 * Generate all the templates in one file.
 *
 */

/* 
 * $Log: templates0.C,v $
 * Revision 1.3  1996/05/08 23:55:11  mjrg
 * added support for handling fork and exec by an application
 * use /proc instead of ptrace on solaris
 * removed warnings
 *
 * Revision 1.2  1996/04/29 03:43:00  tamches
 * added vector<internalMetric::eachInstance>
 *
 * Revision 1.1  1996/04/08 21:42:12  lzheng
 * split templates.C up into templates0.C and templates1.C; needed for HP.
 *
 * Revision 1.24  1995/12/29 01:35:34  zhichen
 * Added an instantiation related to the new paradynd-->paradyn buffering
 *
 * Revision 1.23  1995/12/28 23:44:39  zhichen
 * added 2 new instantiations related to the new paradynd->>paradyn
 * batching code.
 *
 * Revision 1.22  1995/12/18  23:31:21  tamches
 * wrapped blizzard-specific templates in an ifdef
 *
 * Revision 1.21  1995/12/16 00:21:26  tamches
 * added a template needed by blizzard
 *
 * Revision 1.20  1995/12/05 01:52:38  zhichen
 * Added some instanciation of templates for paradyn+blizzard
 *
 * Revision 1.19  1995/11/29  18:45:27  krisna
 * added inlines for compiler. added templates
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


#include "util/h/aggregateSample.h"

template class vector<sampleInfo*>;
template class vector<bool>;


/* ************************************************ */


#include "rtinst/h/rtinst.h"
#include "rtinst/h/trace.h"
#include "symtab.h"
#include "process.h"
#include "inst.h"
#include "instP.h"
#include "dyninstP.h"
#include "metric.h"
#include "costmetrics.h"
#include "ast.h"
#include "util.h"
#include "internalMetrics.h"
#include "util/h/Object.h"

template class  vector<AstNode>;
template class  vector<Symbol*>;
template class  vector<Symbol>;
template class  vector<T_dyninstRPC::mdl_rand *>;
template class  vector<T_dyninstRPC::mdl_instr_rand *>;
template class  vector<T_dyninstRPC::buf_struct*>;
template class  vector<T_dyninstRPC::mdl_constraint *>;
template class  vector<T_dyninstRPC::mdl_expr *>;
template class  vector<T_dyninstRPC::mdl_icode *>;
template class  vector<T_dyninstRPC::mdl_metric *>;
template class  vector<T_dyninstRPC::mdl_stmt *>;
template class  vector<T_dyninstRPC::metricInfo>;
template class  vector<T_dyninstRPC::focusStruct>;
template class  vector<dataReqNode*>;
template class  vector<float>;
template class  vector<heapItem*>;
template class  vector<image*>;
template class  vector<instMapping*>;
template class  vector<instPoint *>;
template class  vector<instReqNode>;
template class  vector<int>;
template class  vector<internalMetric*>;
template class  vector<mdl_focus_element>;
template class  vector<mdl_type_desc>;
template class  vector<mdl_var>;
template class  vector<metricDefinitionNode *>;
template class  vector<module *>;
template class  vector<pdFunction*>;
template class  vector<process*>;
template class  vector<string>;
template class  vector<sym_data>;
template class  vector<unsigned>;
template class  vector<disabledItem>;
template class  vector<unsigVecType>;
template class  vector<vector<string> >;
template class  vector<watch_data>;
template class  vector<costMetric *>;
template class  vector<sampleValue>;
template class  vector<double>;
template class  vector<point *>;
template class  vector<instInstance *>;
template class  vector<internalMetric::eachInstance>;

template class  dictionary<unsigned int, vector<mdl_type_desc> >;
template class  dictionary<unsigned int, _cpSample *>;
template class  dictionary<string, pdFunction *>;
template class  dictionary<instPoint *, point *>;
template class  dictionary<unsigned int, Symbol *>;
template class  dictionary<unsigned int, metricDefinitionNode *>;
template class  dictionary<string, unsigned int>;
template class  dictionary<instPoint *, unsigned int>;
template class  dictionary<unsigned int, heapItem *>;
template class  dictionary<string, vector<pdFunction *> *>;
template class  dictionary<string, internalSym *>;
template class  dictionary<string, module *>;
template class  dictionary<unsigned int, pdFunction *>;
template class  dictionary<unsigned int, unsigned int>;
template class  dictionary<unsigned int, resource *>;
template class  dictionary<string, resource *>;
template class  dictionary_iter<string, Symbol>;
template class  dictionary<string, Symbol>;
