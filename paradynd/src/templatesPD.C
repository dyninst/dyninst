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

// $Id: templatesPD.C,v 1.26 2002/05/09 21:42:56 schendel Exp $

#pragma implementation "Dictionary.h"
#include "common/src/Dictionary.C"

#include "common/h/String.h"

#include "pdutil/h/sampleAggregator.h"
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
#include "paradynd/src/machineMetFocusNode.h"
#include "paradynd/src/processMetFocusNode.h"
#include "paradynd/src/instrCodeNode.h"
#include "paradynd/src/instrDataNode.h"
#include "paradynd/src/threadMetFocusNode.h"
#include "paradynd/src/costmetrics.h"
#include "paradynd/src/internalMetrics.h"
#include "common/h/Time.h"
#include "pdutil/h/pdSample.h"
#include "paradynd/src/timeMgr.h"

template class dictionary_hash <unsigned, vector<mdl_type_desc> >;
template class vector<dictionary_hash <unsigned, vector<mdl_type_desc> >::entry>;

template class vector<functionName*>;

template class vector<aggComponent*>;

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

template class  dictionary_hash <unsigned, machineMetFocusNode*>;
template class  vector<dictionary_hash <unsigned, machineMetFocusNode*>::entry>;

template class  dictionary_hash <unsigned, metricDefinitionNode*>;
template class  vector<dictionary_hash <unsigned, metricDefinitionNode*>::entry>;
template class  vector<dictionary_hash <unsigned long, metricDefinitionNode*>::entry>;
template class  dictionary_hash <string, metricDefinitionNode*>;
template class  vector<dictionary_hash <string, metricDefinitionNode*>::entry>;
template class  dictionary_hash <string, instrCodeNode_Val*>;
template class  vector<dictionary_hash <string, instrCodeNode_Val*>::entry>;
template class  dictionary_hash <string, instrCodeNode*>;
template class  vector<dictionary_hash <string, instrCodeNode*>::entry>;

template class  dictionary_hash <string, threadMetFocusNode_Val*>;
template class  vector<dictionary_hash <string, threadMetFocusNode_Val*>::entry>;

template class  dictionary_hash_iter <string, metricDefinitionNode*>;
template class  vector<instrDataNode*>;
template class  vector<const instrDataNode*>;

template class dictionary_hash <unsigned, cpSample*>;
template class vector<dictionary_hash <unsigned, cpSample*>::entry>;

template class dictionary_hash<string, Symbol>;
template class vector<dictionary_hash<string, Symbol>::entry>;

template class dictionary_hash<string, string>;
template class vector<dictionary_hash<string, string>::entry>;

template class vector<dictionary_hash<unsigned int, resource *>::entry>;

template class vector<timeStamp>;
template class vector<pdSample>;

template class timeMgrBase<NoClass, NoArgs>;
template class timeMgrBase<process, int>;
template class timeMgr<process, int>;

template class parentDataRec<processMetFocusNode>;
template class vector< parentDataRec<processMetFocusNode> >;

#include "paradynd/src/focus.h"

template class vector<const Hierarchy *>;
