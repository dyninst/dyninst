/*
 * Copyright (c) 1996-2011 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
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

class Statistic;
#include <string>
template class dictionary_hash<std::string, Statistic *>;
/** end of the template class initialization */
