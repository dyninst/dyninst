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
 * Generate all the templates in one file.
 *
 */

/* 
 * $Log: templates0.C,v $
 * Revision 1.12  1996/11/14 14:59:09  naim
 * Minor fix to my previous commit - naim
 *
 * Revision 1.11  1996/11/14 14:28:06  naim
 * Changing AstNodes back to pointers to improve performance - naim
 *
 * Revision 1.10  1996/10/31 08:53:12  tamches
 * split; moved half to templates05.C
 *
 * Revision 1.9  1996/10/18 23:54:16  mjrg
 * Solaris/X86 port
 *
 * Revision 1.8  1996/10/09 14:03:17  mjrg
 * added template for class functionName
 *
 * Revision 1.7  1996/09/26 18:59:24  newhall
 * added support for instrumenting dynamic executables on sparc-solaris
 * platform
 *
 * Revision 1.6  1996/08/20 18:56:53  lzheng
 * Implementation of moving multiple instructions sequence and
 * Splitting the instrumentation into two phases
 *
 * Revision 1.5  1996/08/16 21:20:06  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.4  1996/07/25 23:21:55  mjrg
 * Divided the templates files, to avoid problems compiling on HP.
 * Added new templates
 *
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
 */

#pragma implementation "Pair.h"
#include "util/h/Pair.h"

#pragma implementation "Vector.h"
#include "util/h/Vector.h"

#pragma implementation "Symbol.h"
#include "util/h/Symbol.h"

#include "util/h/String.h"

#include "util/h/aggregateSample.h"
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

template class  vector<reg>;
template class  vector<functionName*>;
template class  vector<sampleInfo*>;
template class  vector<bool>;
template class  vector<AstNode>;
template class  vector<AstNode *>;
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
template class  vector<instruction>;
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
//template class  vector<watch_data>;
template class  vector<costMetric *>;
template class  vector<sampleValue>;
template class  vector<double>;
template class  vector<point *>;
template class  vector<instInstance *>;
template class  vector<internalMetric::eachInstance>;
template class  vector<returnInstance *>;             //XXX

