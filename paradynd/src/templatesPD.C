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

// $Id: templatesPD.C,v 1.18 2000/07/27 15:24:47 hollings Exp $

#pragma implementation "Dictionary.h"
#include "common/src/Dictionary.C"

#pragma implementation "list.h"
#include "common/h/list.h"

#include "common/h/String.h"

#include "pdutil/h/aggregateSample.h"
#include "rtinst/h/rtinst.h"
#include "rtinst/h/trace.h"
#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/instP.h"
#include "dyninstAPI/src/dyninstP.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/util.h"
#include "dyninstAPI/src/Object.h"

#include "paradynd/src/metric.h"
#include "paradynd/src/costmetrics.h"
#include "paradynd/src/internalMetrics.h"

template class dictionary_hash <unsigned, vector<mdl_type_desc> >;
template class vector<dictionary_hash <unsigned, vector<mdl_type_desc> >::entry>;

template class vector<functionName*>;

template class vector<T_dyninstRPC::buf_struct*>;
template class vector<T_dyninstRPC::mdl_constraint *>;
template class vector<T_dyninstRPC::mdl_expr *>;
template class vector<T_dyninstRPC::mdl_icode *>;
template class vector<T_dyninstRPC::mdl_metric *>;
template class vector<T_dyninstRPC::mdl_stmt *>;
template class vector<T_dyninstRPC::metricInfo>;
template class vector<T_dyninstRPC::focusStruct>;
template class vector<dataReqNode*>;
template class vector<sampleInfo*>;

template class vector<instReqNode>;

template class vector<internalMetric*>;

template class vector<mdl_focus_element>;
template class vector<mdl_type_desc>;
template class vector<mdl_var>;

template class vector<costMetric *>;

template class vector<instReqNode *>;
template bool  find(const vector<instReqNode *> &, 
		    instReqNode * const &, 
		    unsigned &);
template class vector<internalMetric::eachInstance>;

template class dictionary_hash <unsigned, cpSample*>;
template class vector<dictionary_hash <unsigned, cpSample*>::entry>;

template class dictionary_hash<string, Symbol>;
template class vector<dictionary_hash<string, Symbol>::entry>;

template class dictionary_hash<string, string>;
template class vector<dictionary_hash<string, string>::entry>;

//template class vector<dictionary_hash<unsigned int, unsigned int>::entry>;
//template class vector<dictionary_hash<unsigned int, unsigned int>::entry>;
template class vector<dictionary_hash<unsigned int, resource *>::entry>;
