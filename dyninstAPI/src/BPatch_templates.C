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

// $Id: BPatch_templates.C,v 1.15 2001/02/27 20:44:52 buck Exp $

#include <sys/types.h>

#if !defined(i386_unknown_nt4_0)
#pragma implementation "BPatch_Vector.h"
#endif
#include "BPatch_Vector.h"

#if !defined(i386_unknown_nt4_0)
#pragma implementation "BPatch_Set.h"
#endif
#include "BPatch_Set.h"

#if !defined(i386_unknown_nt4_0)
#pragma implementation "refCounter.h"
#endif
#include "common/h/refCounter.h"
#include "common/h/String.h"
#include "common/h/Types.h"
#if defined(rs6000_ibm_aix4_1)
#include "LineInformation.h"
#endif

class BPatch_point;
class BPatch_field;
class BPatch_thread;
class BPatch_module;
class BPatch_snippet;
class BPatch_function;
class BPatch_type;
class BPatch_variableExpr;
class BPatch_localVar;
class BPatch_sourceObj;
class instInstance;

template class BPatch_Vector<BPatch_function *>;
template class BPatch_Vector<BPatch_point *>;
template class BPatch_Vector<BPatch_localVar *>;
template class BPatch_Vector<BPatch_snippet *>;
template class BPatch_Vector<BPatch_thread *>;
template class BPatch_Vector<BPatch_module *>;
template class BPatch_Vector<BPatch_field *>;
template class BPatch_Vector<BPatch_type *>;
template class BPatch_Vector<BPatch_variableExpr *>;
template class BPatch_Vector<BPatch_sourceObj *>;
template class BPatch_Vector<instInstance *>;
template class BPatch_Vector<int>;
template class BPatch_Vector<char *>;
template class BPatch_Vector<unsigned long>;
#if defined(rs6000_ibm_aix4_1)
template class BPatch_Vector<IncludeFileInfo>;
#endif

template struct comparison<unsigned short>;
template class BPatch_Set<unsigned short>;
template struct comparison<Address>;
template class BPatch_Set<Address>;
template class BPatch_Vector<unsigned short>;
template struct comparison<int>;
template class BPatch_Set<int>;

class BPatch_basicBlock;
class BPatch_basicBlockLoop;

template class BPatch_Vector<BPatch_basicBlock*>;
template class BPatch_Vector<BPatch_basicBlockLoop*>;

class BPatch_cblock;

template class BPatch_Vector<BPatch_cblock*>;

template struct comparison<BPatch_basicBlock*>;
template class BPatch_Set<BPatch_basicBlock*>;
template struct comparison<BPatch_basicBlockLoop*>;
template class BPatch_Set<BPatch_basicBlockLoop*>;
