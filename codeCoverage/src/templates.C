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

/** beginning of the pragma */

/** the initiation of the template objects for
  * the code coverage different than the dynsint library.
  */

#pragma implementation "Vector.h"
#include "common/h/Vector.h"

#pragma implementation "Dictionary.h"
#include "common/src/Dictionary.C"

#pragma implementation "BPatch_Set.h"
#include "BPatch_Set.h"
/** end of the pragma */

#include "common/h/Types.h"
#include "common/h/String.h"

class BPatch_function;
class FunctionCoverage;
class BPFunctionList;
class FileLineCoverage;

/** beginning of the template class initialization */
template struct comparison<BPatch_function*>;
template class BPatch_Set<BPatch_function*>;

template struct comparison<FunctionCoverage*>;
template class BPatch_Set<FunctionCoverage*>;

template class pdvector<BPFunctionList*>;
template class dictionary_hash<pdstring,BPFunctionList*>;
template class pdvector<dictionary_hash<pdstring,BPFunctionList*>::entry>;

template class pdvector<FunctionCoverage*>;
template class dictionary_hash<pdstring,FunctionCoverage*>;
template class pdvector<dictionary_hash<pdstring,FunctionCoverage*>::entry>;

/** end of the template class initialization */
