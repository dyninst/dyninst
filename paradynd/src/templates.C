/*
 * Generate all the templates in one file.
 *
 */

/* 
 * $Log: templates.C,v $
 * Revision 1.18  1995/11/29 00:28:48  tamches
 * will now compile with g++ 2.7.1 on sunos; some templates had been missing.
 *
 * Revision 1.17  1995/08/24 15:04:40  hollings
 * AIX/SP-2 port (including option for split instruction/data heaps)
 * Tracing of rexec (correctly spawns a paradynd if needed)
 * Added rtinst function to read getrusage stats (can now be used in metrics)
 * Critical Path
 * Improved Error reporting in MDL sematic checks
 * Fixed MDL Function call statement
 * Fixed bugs in TK usage (strings passed where UID expected)
 *
 * Revision 1.16  1995/08/05  17:17:12  krisna
 * added lots of missing templates
 *
 * Revision 1.15  1995/05/18  10:42:54  markc
 * removed unused templates
 *
 * Revision 1.14  1995/02/16  08:54:27  markc
 * Corrected error in comments -- I put a "star slash" in the comment.
 *
 * Revision 1.13  1995/02/16  08:35:01  markc
 * Changed igen interfaces to use strings/vectors rather than char igen-arrays
 * Changed igen interfaces to use bool, not Boolean.
 * Cleaned up symbol table parsing - favor properly labeled symbol table objects
 * Updated binary search for modules
 * Moved machine dependnent ptrace code to architecture specific files.
 * Moved machine dependent code out of class process.
 * Removed almost all compiler warnings.
 * Use "posix" like library to remove compiler warnings
 *
 * Revision 1.12  1994/11/10  20:49:32  jcargill
 * Removed references to kludgeInternalMetric
 *
 * Revision 1.11  1994/11/10  18:58:20  jcargill
 * The "Don't Blame Me Either" commit
 *
 * Revision 1.10  1994/11/09  18:40:42  rbi
 * the "Don't Blame Me" commit
 *
 * Revision 1.9  1994/11/02  11:18:03  markc
 * Instantiated new classes here.
 *
 * Revision 1.8  1994/09/22  02:27:18  markc
 * Gave type names to typedefs
 *
 * Revision 1.7  1994/09/20  18:18:33  hollings
 * added code to use actual clock speed for cost model numbers.
 *
 * Revision 1.6  1994/08/08  20:13:48  hollings
 * Added suppress instrumentation command.
 *
 * Revision 1.5  1994/08/02  18:25:09  hollings
 * fixed modules to use list template for lists of functions.
 *
 * Revision 1.4  1994/07/26  20:02:09  hollings
 * fixed heap allocation to use hash tables.
 *
 * Revision 1.3  1994/07/20  23:23:44  hollings
 * added insn generated metric.
 *
 * Revision 1.2  1994/06/27  18:57:17  hollings
 * removed printfs.  Now use logLine so it works in the remote case.
 * added internalMetric class.
 * added extra paramter to metric info for aggregation.
 *
 * Revision 1.1  1994/05/30  19:51:17  hollings
 * added code to support external templates.
 *
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

#pragma implementation "Queue.h"
#include "util/h/Queue.h"

#pragma implementation "dyninstRPC.xdr.h"
#include "dyninstRPC.xdr.h"

#pragma implementation "Symbol.h"
#include "util/h/Symbol.h"

#include "util/h/String.h"

template bool_t T_dyninstRPC_P_xdr_stl(XDR*, vector<u_int>*,
				       bool_t (*)(XDR*, u_int*), u_int*);

template bool_t T_dyninstRPC_P_xdr_stl(XDR*, vector<string>*,
				       bool_t (*)(XDR*, string*), string*);
template bool_t T_dyninstRPC_P_xdr_stl_PTR(XDR*, vector<string>**,
					   bool_t (*)(XDR*, string*), string*);

template bool_t T_dyninstRPC_P_xdr_stl(XDR*, vector<T_dyninstRPC::metricInfo>*,
				       bool_t (*)(XDR*, T_dyninstRPC::metricInfo*),
				       T_dyninstRPC::metricInfo*);

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

#include "rtinst/h/rtinst.h"
#include "rtinst/h/trace.h"
#include "util/h/aggregateSample.h"
#include "symtab.h"
#include "process.h"
#include "inst.h"
#include "instP.h"
#include "dyninstP.h"
#include "metric.h"
#include "ast.h"
#include "util.h"
#include "internalMetrics.h"
#include "util/h/Object.h"

template class  List<sampleInfo*>;

template class  queue<T_dyninstRPC::buf_struct*>;
template class vector<T_dyninstRPC::buf_struct*>;

template class vector<unsigned>;
template class vector<float>;
template class vector<int>;
template class  vector<Symbol*>;
template class  vector<Symbol>;
template class  vector<T_dyninstRPC::mdl_constraint *>;
template class  vector<T_dyninstRPC::mdl_expr *>;
template class  vector<T_dyninstRPC::mdl_icode *>;
template class  vector<T_dyninstRPC::mdl_metric *>;
template class  vector<T_dyninstRPC::mdl_stmt *>;
template class  vector<T_dyninstRPC::metricInfo>;
template class  vector<dataReqNode*>;
template class  vector<heapItem*>;
template class  vector<image*>;
template class  vector<instMapping*>;
template class  vector<instPoint *>;
template class  vector<instReqNode*>;
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
template class  vector<vector<string> >;
template class  vector<watch_data>;

template class  dictionary_hash <Address, Symbol*>;
template class  dictionary_hash <instPoint*, point*>;
template class  dictionary_hash <instPoint*, unsigned>;
template class  dictionary_hash <string, Symbol>;
template class  dictionary_hash <string, internalSym*>;
template class  dictionary_hash <string, module *>;
template class  dictionary_hash <string, pdFunction*>;
template class  dictionary_hash <string, resource*>;
template class  dictionary_hash <string, unsigned>;
template class  dictionary_hash <string, vector<pdFunction*>*>;
template class  dictionary_hash <unsigned, Line>;
template class  dictionary_hash <unsigned, heapItem*>;
template class  dictionary_hash <unsigned, cpSample*>;

template class  dictionary_hash <unsigned, metricDefinitionNode*>;
template class  dictionary_hash <unsigned, pdFunction*>;
template class  dictionary_hash <unsigned, resource *>;
template class  dictionary_hash <unsigned, unsigned>;
template class  dictionary_hash <unsigned, vector<mdl_type_desc> >;

template class  dictionary_hash_iter <Address, Symbol*>;
template class  dictionary_hash_iter <instPoint*, point*>;
template class  dictionary_hash_iter <instPoint*, unsigned>;
template class  dictionary_hash_iter <string, Symbol>;
template class  dictionary_hash_iter <string, internalSym*>;
template class  dictionary_hash_iter <string, module *>;
template class  dictionary_hash_iter <string, pdFunction*>;
template class  dictionary_hash_iter <string, resource*>;
template class  dictionary_hash_iter <string, unsigned>;
template class  dictionary_hash_iter <string, vector<pdFunction*>*>;
template class  dictionary_hash_iter <unsigned, heapItem*>;
template class  dictionary_hash_iter <unsigned, metricDefinitionNode*>;
template class  dictionary_hash_iter <unsigned, pdFunction*>;
