/*
 * Generate all the templates in one file.
 *
 */

/* 
 * $Log: templates.C,v $
 * Revision 1.33  1996/04/06 21:25:36  hollings
 * Fixed inst free to work on AIX (really any platform with split I/D heaps).
 * Removed the Line class.
 * Removed a debugging printf for multiple function returns.
 *
 * Revision 1.32  1996/04/03  14:28:00  naim
 * Implementation of deallocation of instrumentation for solaris and sunos - naim
 *
 * Revision 1.31  1996/03/25  20:25:58  tamches
 * the reduce-mem-leaks-in-paradynd commit
 *
 * Revision 1.30  1996/03/20 17:02:53  mjrg
 * Added multiple arguments to calls.
 * Instrument pvm_send instead of pvm_recv to get tags.
 *
 * Revision 1.29  1996/03/14 14:23:27  naim
 * Batching enable data requests for better performance - naim
 *
 * Revision 1.28  1996/03/12  20:48:42  mjrg
 * Improved handling of process termination
 * New version of aggregateSample to support adding and removing components
 * dynamically
 * Added error messages
 *
 * Revision 1.27  1996/02/13 06:17:37  newhall
 * changes to how cost metrics are computed. added a new costMetric class.
 *
 * Revision 1.26  1996/02/09  22:13:57  mjrg
 * metric inheritance now works in all cases
 * paradynd now always reports to paradyn when a process is ready to run
 * fixed aggregation to handle first samples and addition of new components
 *
 * Revision 1.25  1996/01/18 16:32:01  hollings
 * Added a bunch of definitions for Aix.
 *
 * Revision 1.24  1995/12/29  01:35:34  zhichen
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

//#pragma implementation "Queue.h"
//#include "util/h/Queue.h"

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

#include "rtinst/h/rtinst.h"
#include "rtinst/h/trace.h"
#include "util/h/aggregateSample.h"
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

//template class  List<sampleInfo*>;
template class vector<sampleInfo*>;
template class vector<bool>;

//template class  queue<T_dyninstRPC::buf_struct*>;

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
template class  dictionary_hash <unsigned, cpSample*>;
template class  dictionary_hash <unsigned, heapItem*>;
template class  dictionary_hash <unsigned, metricDefinitionNode*>;
template class  dictionary_hash <unsigned, pdFunction*>;
template class  dictionary_hash <unsigned, resource *>;
template class  dictionary_hash <unsigned, unsigned>;
template class  dictionary_hash <unsigned, vector<mdl_type_desc> >;

template class  dictionary_iter<unsigned int, pdFunction *>;
template class  dictionary_iter<unsigned int, metricDefinitionNode *>;
template class  dictionary_iter<unsigned int, heapItem *>;
template class  dictionary_iter<string, vector<pdFunction *> *>;
template class  dictionary_iter<string, unsigned int>;
template class  dictionary_iter<string, resource *>;
template class  dictionary_iter<string, pdFunction *>;
template class  dictionary_iter<string, module *>;
template class  dictionary_iter<string, internalSym *>;
template class  dictionary_iter<instPoint *, unsigned int>;
template class  dictionary_iter<instPoint *, point *>;
template class  dictionary_iter<unsigned int, Symbol *>;

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

#ifdef paradyndCM5_blizzard
//=============================added by zxu for sampleNodes
#include "util/h/sys.h"
#include "rtinst/h/trace.h"
#include "../../paradyndCM5_blizzard/src/sample_nodes.h"

//template class vector<unsigned> ;
template class vector<time64> ;
//template class vector<sampleValue> ;
template class vector<stamped_sample> ;
template class vector<per_node_buffer> ;
template class dictionary_hash <unsigned, sampleVec *>  ;
template class dictionary_hash <unsigned, traceHeaderVec *>  ;
template class dictionary_hash_iter<unsigned, sampleVec *> ;
template class dictionary_hash_iter<unsigned, per_mid_buffer *> ;
template class dictionary_hash<unsigned, per_mid_buffer *>;
#endif
