/*
 * Copyright (c) 1996 Barton P. Miller
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

// $Id: templates-nt.C,v 1.39 2003/04/04 16:36:18 pcroth Exp $

/* The VC++ v5.0 compiler (probably correctly) generates warning C4660's 
 * "template class specialization XXXX is already instantiated"
 * however when linking the executable, the linker is unfortunately not
 * able to resolve these external symbols and produces error LNK2001's,
 * therefore the compiler warning is being disabled for this template file.
 */
#pragma warning (disable: 4660)

#include "common/src/vectorSet.C"
#include "common/src/Dictionary.C"
#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/process.h"

#if defined(BPATCH_LIBRARY)
#include "dyninstAPI/src/LineInformation.h"
#endif // defined(BPATCH_LIBRARY)

template class vectorSet<inferiorRPCtoDo>;
template class vectorSet<inferiorRPCinProgress>;

template class dictionary_hash<unsigned int, unsigned int>;
template class dictionary_hash<unsigned int, resource *>;
template class dictionary_hash<unsigned int, heapItem *>;
template class dictionary_hash<unsigned int, pd_Function *>;

template class dictionary_hash<string, unsigned int>;
template class dictionary_hash<string, string>;
template class dictionary_hash<string, Symbol>;
template class dictionary_hash<string, resource *>;
template class dictionary_hash<string, pdmodule *>;
template class dictionary_hash<string, pd_Function *>;
template class dictionary_hash<string, internalSym *>;

template class dictionary_hash<string, pdvector<string> *>;
template class dictionary_hash<string, pdvector<pd_Function *> *>;

#include "dyninstAPI/src/rpcMgr.h"
template class  dictionary_hash<dyn_lwp *, rpcLwp *>;
template class  pdvector<dictionary_hash <dyn_lwp *, rpcLwp *>::entry>;

#include "common/src/List.C"
template class List<instInstance*>;
template class ListBase<instInstance*, void*>;
template class dictionary_hash<const instPoint *, installed_miniTramps_list*>;
template class dictionary_hash<instPoint const *, trampTemplate *>;
template class dictionary_hash<instInstance *, instInstance *>;

#ifdef BPATCH_LIBRARY
#include "common/h/String.h"
#include "dyninstAPI/h/BPatch_thread.h"
#include "dyninstAPI/h/BPatch_type.h"

template class dictionary_hash<string, BPatch_type *>;
template class dictionary_hash<int, BPatch_thread *>;
template class dictionary_hash<int, BPatch_type *>;
template class dictionary_hash<string, BPatch_localVar *>;
template class dictionary_hash<function_base*, BPatch_function*>;
template class  dictionary_hash <Address, BPatch_variableExpr*>;
template class dictionary_hash<Address, BPatch_point *>;

#endif

template class dictionary_hash<u_int, Address>;
template class dictionary_hash<Address, Address>;
template class dictionary_hash<Address, heapItem *>;
template class dictionary_hash<Address, pd_Function *>;
template class dictionary_hash<Address, unsigned>;

#include "dyn_lwp.h"
template class dictionary_hash<unsigned, dyn_lwp *>;

#if defined(BPATCH_LIBRARY)

class BPatch_basicBlock;

template class  dictionary_hash<Address,BPatch_basicBlock*>;
template class  pdvector<dictionary_hash<Address,BPatch_basicBlock*>::entry>;

template class BPatch_Vector<BPatch_frame>;
#endif


// Library callback templates
template class dictionary_hash<string, libraryCallback *>;
template class pdvector<dictionary_hash <string, libraryCallback *>::entry>;

#ifndef OLD_LINE_INFO
template class  dictionary_hash <string, FunctionInfo *>;
template class  pdvector< dictionary_hash <string, FunctionInfo * >::entry >;
template class  dictionary_hash_iter <string, FunctionInfo *>;

template class  dictionary_hash <string, FileLineInformation *>;
template class  pdvector< dictionary_hash <string, FileLineInformation * >::entry >;
template class  dictionary_hash_iter <string, FileLineInformation *>;

template class std::map<unsigned short, tuple *, std::less<unsigned short> >;
template class  std::map<Address, tuple *, std::less<Address> >;
template class std::map<unsigned short, std::map<Address, tuple *, std::less<Address> > *, std::less<unsigned short> >;
template class std::map<Address,std::map<unsigned short, tuple *, std::less<unsigned short> > *, std::less<Address> >;
#endif
