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

// templatesPCV.C
// templates for perf consultant and visis

#pragma implementation "list.h"
#include "common/h/list.h"
#include "common/h/String.h"

#pragma implementation "Vector.h"
#include "common/h/Vector.h"

#pragma implementation "BufferPool.h"
#include "paradyn/src/DMthread/BufferPool.h"

#pragma implementation "Dictionary.C"
#include "common/src/Dictionary.C"

#pragma implementation "dyninstRPC.xdr.h"
#include "dyninstRPC.xdr.h"
#pragma implementation "visi.xdr.h"
#include "visi.xdr.h"

/* ********************************
 * PCthread stuff
 */

#include "paradyn/src/PCthread/PCintern.h"

#include "paradyn/src/PCthread/PCsearch.h"
#include "paradyn/src/PCthread/PCfilter.h"
#include "paradyn/src/PCthread/PCmetric.h"
#include "paradyn/src/PCthread/PCmetricInst.h"
#include "paradyn/src/PCthread/PCwhy.h"
#include "paradyn/src/PCthread/PCshg.h"
#include "paradyn/src/PCthread/PCdata.h"
#include "paradyn/src/PCthread/PCcostServer.h"
#include "UI.thread.h"

template class vector<string*>;
template class vector<costServerRec>;
template class vector<uiSHGrequest>;
template class vector<dataSubscriber*>;
template class vector<PCsearch*>;
template class vector<filter*>;
template class vector<PCMetInfo*>;
template class vector<PCmetric*>;
template class vector<PCMRec>;
template class vector<inPort>;
template class vector<searchHistoryNode*>;
template class vector<hypothesis*>;
template class vector<PCmetricInst*>;

template class dictionary_hash<unsigned, PCsearch*>;
template class vector<dictionary_hash<unsigned, PCsearch*>::entry>;

#include "pdutil/src/PriorityQueue.C"
template class PriorityQueue<unsigned, searchHistoryNode*>; 
template ostream &operator<<(ostream &, PriorityQueue<unsigned, searchHistoryNode*> &);
template class vector<PriorityQueue<unsigned, searchHistoryNode*>::pair>;

template class dictionary_hash<unsigned,filter*>;
template class vector<dictionary_hash<unsigned,filter*>::entry>;
 
template class dictionary_hash<unsigned, searchHistoryNode*>;
template class vector<dictionary_hash<unsigned, searchHistoryNode*>::entry>;

template class dictionary_hash<string,PCmetric*>;
template class vector<dictionary_hash<string,PCmetric*>::entry>;

template class dictionary_hash<unsigned, PCmetricInst*>;
template class vector<dictionary_hash<unsigned, PCmetricInst*>::entry>;

template class dictionary_hash<string, hypothesis*>;
template class vector<dictionary_hash<string, hypothesis*>::entry>;

template class dictionary_hash<focus, vector<searchHistoryNode*>*>;
template class vector<dictionary_hash<focus, vector<searchHistoryNode*>*>::entry>;
template class vector< vector<searchHistoryNode*> *>;

#include "pdutil/src/CircularBuffer.C"
#include "paradyn/src/PCthread/PCintern.h"
template class circularBuffer<PCInterval, PCdataQSize>;

/* ************************************
 * VMthread stuff
 */
#include "paradyn/src/VMthread/VMtypes.h"
#include "VM.thread.h"

template class vector<VMactiveStruct *>;
template class vector<VMvisisStruct *>;
template class vector<VM_visiInfo>;

/* ******************************
 * VISIthread stuff
  */
template class vector<metricInstInfo *>;
