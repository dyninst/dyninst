/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
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

// $Id: templatesPD.C,v 1.41 2004/07/28 07:24:47 jaw Exp $

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
#include "pdutil/h/mdl.h"
#include "paradynd/src/pd_image.h"
#include "paradynd/src/pd_module.h"
#include "paradynd/src/shmMgr.h"
#include "paradynd/src/shmSegment.h"

template class dictionary_hash <unsigned, pdvector<mdl_type_desc> >;
template class pdvector<dictionary_hash <unsigned, pdvector<mdl_type_desc> >::entry>;

template class pdvector<functionName*>;

template class pdvector<aggComponent*>;

template class pdvector<internalMetric*>;

template class pdvector<mdl_focus_element>;
template class pdvector<mdl_type_desc>;
template class pdvector<mdl_var>;
template class pdvector<mdl_env::Frame>;

template class pdvector<costMetric *>;

template class pdvector<instReqNode *>;
template bool  find(const pdvector<instReqNode *> &, 
		    instReqNode * const &, 
		    unsigned &);
template class pdvector<internalMetric::eachInstance>;

template class  dictionary_hash <unsigned, machineMetFocusNode*>;
template class  pdvector<dictionary_hash <unsigned, machineMetFocusNode*>::entry>;

template class  dictionary_hash <pdstring, instrCodeNode_Val*>;
template class  pdvector<dictionary_hash <pdstring, instrCodeNode_Val*>::entry>;
template class  dictionary_hash <pdstring, instrCodeNode*>;
template class  pdvector<dictionary_hash <pdstring, instrCodeNode*>::entry>;

template class  dictionary_hash <pdstring, threadMetFocusNode_Val*>;
template class  pdvector<dictionary_hash <pdstring, threadMetFocusNode_Val*>::entry>;

template class  pdvector<instrDataNode*>;
template class  pdvector<const instrDataNode*>;

template class dictionary_hash <unsigned, cpSample*>;
template class pdvector<dictionary_hash <unsigned, cpSample*>::entry>;

template class dictionary_hash<pdstring, Symbol>;
template class pdvector<dictionary_hash<pdstring, Symbol>::entry>;

template class dictionary_hash<pdstring, pdstring>;
template class pdvector<dictionary_hash<pdstring, pdstring>::entry>;

template class dictionary_hash <unsigned, resource *>;
template class pdvector<dictionary_hash<unsigned, resource *>::entry>;

template class dictionary_hash<pdstring, resource*>;
template class pdvector<dictionary_hash <pdstring, resource*>::entry>;

template class pdvector<BPatch_module *>;
template class pdvector<BPatch_point *>;

template class pdvector<timeStamp>;
template class pdvector<pdSample>;

template class timeMgrBase<NoClass, NoArgs>;
template class timeMgrBase<process, int>;
template class timeMgr<pd_process, int>;

template class pdvector<pd_image *>;
template class pdvector<pd_module *>;

template class parentDataRec<processMetFocusNode>;
template class pdvector< parentDataRec<processMetFocusNode> >;

template class pdvector<shmMgrPreallocInternal *>;
template class pdvector<ShmSegment *>;

template class dictionary_hash<pdstring, int>;
template class pdvector<dictionary_hash<pdstring, int>::entry>;

template class dictionary_hash<unsigned, int>;
template class pdvector<dictionary_hash<unsigned, int>::entry>;

#include "paradynd/src/focus.h"

template class pdvector<const Hierarchy *>;

#include "paradynd/src/pd_process.h"
template class pdvector<pd_process*>;

#include "paradynd/src/pd_thread.h"
template class pdvector<pd_thread*>;

#include "varInstanceHKs.h"
template class pdvector<intCounterHK*>;
template class pdvector<wallTimerHK*>;
template class pdvector<processTimerHK*>;
#ifdef PAPI
template class pdvector<hwTimerHK*>;
template class pdvector<hwCounterHK*>;
#endif

#ifndef USES_NATIVE_CC
#include "varInstance.C"
#endif
template class varInstance<intCounterHK>;
template class varInstance<wallTimerHK>;
template class varInstance<processTimerHK>;
#ifdef PAPI
template class varInstance<hwTimerHK>;
template class varInstance<hwCounterHK>;
#endif
template class pdvector<baseVarInstance *>;

#ifndef USES_NATIVE_CC
#include "varTable.C"
#endif
template class varTable<intCounterHK>;
template class varTable<wallTimerHK>;
template class varTable<processTimerHK>;
#ifdef PAPI
template class varTable<hwTimerHK>;
template class varTable<hwCounterHK>;
#endif
template class pdvector<baseVarTable *>;
