/*
 * Copyright (c) 1996-2003 Barton P. Miller
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

// $Id: templatesPD-nt.C,v 

/* The VC++ v5.0 compiler (probably correctly) generates warning C4660's 
 * "template class specialization XXXX is already instantiated"
 * however when linking the executable, the linker is unfortunately not
 * able to resolve these external symbols and produces error LNK2001's,
 * therefore the compiler warning is being disabled for this template file.
 */
#pragma warning (disable: 4660)

#include "common/src/Dictionary.C"
#include "common/src/String.C"
#include "common/h/Vector.h"
#include "pdutil/h/mdl.h"
#include "paradynd/src/threadMetFocusNode.h"

class function_base;
class metricFocusNode;
struct _cpSample;
class metricFocusNode;
class instrCodeNode;
class instrCodeNode_Val;
class machineMetFocusNode;
class threadMetFocusNode_Val;
class processMetFocusNode;

template class dictionary_hash<unsigned, int>;
template class pdvector<dictionary_hash<unsigned, int>::entry>;

template class dictionary_hash<string, int>;
template class pdvector<dictionary_hash<string, int>::entry>;

template class dictionary_hash<function_base *, function_base *>;

template class dictionary_hash<unsigned int, _cpSample *>;
template class dictionary_hash<unsigned int, metricFocusNode *>;
template class dictionary_hash<unsigned int, pdvector<mdl_type_desc> >;

template class dictionary_hash<string, metricFocusNode *>;
template class dictionary_hash<string, instrCodeNode *>;

template class  dictionary_hash <unsigned, machineMetFocusNode*>;
template class  pdvector<dictionary_hash <unsigned, machineMetFocusNode*>::entry>;
template class  dictionary_hash <string, instrCodeNode_Val*>;
template class  pdvector<dictionary_hash <string, instrCodeNode_Val*>::entry>;
template class  dictionary_hash <string, instrCodeNode*>;
template class  pdvector<dictionary_hash <string, instrCodeNode*>::entry>;

template class  dictionary_hash <string, threadMetFocusNode_Val*>;
template class  pdvector<dictionary_hash <string, threadMetFocusNode_Val*>::entry>;

template class parentDataRec<processMetFocusNode>;
template class pdvector< parentDataRec<processMetFocusNode> >;

#include "paradynd/src/varTable.h" 
#include "paradynd/src/varTable.C"
#include "paradynd/src/varInstanceHKs.h"
template class varTable<intCounterHK>;
template class varTable<wallTimerHK>;
template class varTable<processTimerHK>;
#ifdef PAPI
template class varTable<hwTimerHK>;
template class varTable<hwCounterHK>;
#endif
template class pdvector<baseVarTable *>;

#include "paradynd/src/varInstance.h"
#include "paradynd/src/varInstance.C"
template class varInstance<intCounterHK>;
template class varInstance<wallTimerHK>;
template class varInstance<processTimerHK>;
#ifdef PAPI
template class varInstance<hwTimerHK>;
template class varInstance<hwCounterHK>;
#endif
template class pdvector<baseVarInstance *>;

template class dictionary_hash<string,supportedLanguages>;
template class pdvector<dictionary_hash<string,supportedLanguages>::entry>;

