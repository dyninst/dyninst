/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

// $Id: BPatch_templates.C,v 1.42 2008/02/23 02:09:04 jaw Exp $

#include <sys/types.h>

#define BPATCH_FILE

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
#include <string>
#include "common/src/refCounter.h"
#include "common/src/Types.h"
#include "common/src/Pair.h"
#include "function.h"

// VG(09/19/01): I don't think one can forward a typedefed enum (BPatch_opCode)
//               so this must be included...
#include <BPatch_point.h>

#include "BPatch_basicBlock.h"

//class BPatch_point;
class BPatch_field;
class BPatch_thread;
class BPatch_module;
class BPatch_snippet;
class BPatchSnippetHandle;
class BPatch_function;
class BPatch_type;
class BPatch_variableExpr;
class BPatch_localVar;
class BPatch_sourceObj;
class BPatch_loopTreeNode;
class func_instance;
class miniTramp;
class instPoint;
class BPatch_basicBlock;

/* only define the ones not defined with a vector inside  */
template class BPatch_Vector<BPatch_point *>;
template class BPatch_Vector<BPatch_thread *>;
template class BPatch_Vector<BPatch_process *>;
template class BPatch_Vector<BPatch_snippet *>;
template class BPatch_Vector<BPatchSnippetHandle *>;
template class BPatch_Vector<BPatch_function *>;
template class BPatch_Vector<BPatch_variableExpr *>;
template class BPatch_Vector<BPatch_type *>;
template class BPatch_Vector<BPatch_module *>;
template class BPatch_Vector<char *>;
template class BPatch_Vector<BPatch_sourceObj *>;
template class BPatch_Vector<BPatch_loopTreeNode *>;
template class BPatch_Vector<func_instance *>;
template class BPatch_Vector<miniTramp *>;

template class BPatch_Vector<const char *>;


template class BPatch_Vector<BPatch_localVar *>;
template class BPatch_Vector<BPatch_field *>;
template class BPatch_Vector<int>;
template class BPatch_Vector<unsigned long>;

template class BPatch_Vector<unsigned short>;
template class BPatch_Vector<BPatch_opCode>;

class BPatch_basicBlock;
class BPatch_basicBlockLoop;

template class BPatch_Vector<BPatch_basicBlock*>;
template class BPatch_Vector<BPatch_basicBlockLoop*>;

class BPatch_cblock;

template class BPatch_Vector<BPatch_cblock*>;


class BPatch_sourceBlock;

template class BPatch_Vector<BPatch_sourceBlock*>;

class BPatch_instruction;
template class BPatch_Vector<BPatch_instruction*>;

template struct pdpair<unsigned short, unsigned short>;

template pdpair<unsigned short, unsigned short> min_max_pdpair<unsigned short>(const BPatch_Vector<unsigned short>&);


class BPatch_edge;
template class BPatch_Vector<BPatch_edge*>;

struct batchInsertionRecord;
template class BPatch_Vector<batchInsertionRecord *>;


