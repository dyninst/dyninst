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
#ifndef PATCHAPI_H_COMMON_H_
#define PATCHAPI_H_COMMON_H_

// C++
#include <string>
#include <string.h>
#include <utility>
#include <vector>
#include <list>
#include <set>
#include <map>
#include <iterator>
#include <iostream>

// ParseAPI
#include "CFG.h"
#include "CodeObject.h"

// InstructionAPI
#include "Instruction.h"
#include "InstructionDecoder.h"

namespace Dyninst {
namespace PatchAPI {

class AddrSpace;
class CFGMaker;
class PatchObject;
class PointMaker;

class Point;
typedef std::set<Point*> PointSet;
typedef PointSet::iterator PointIter;

class Instance;
typedef boost::shared_ptr<Instance> InstancePtr;
typedef std::set<InstancePtr> InstanceSet;
typedef std::list<InstancePtr> InstanceList;

class Instrumenter;
class PatchMgr;
typedef boost::shared_ptr<PatchMgr> PatchMgrPtr;

class PatchFunction;
class PatchBlock;
class PatchEdge;

class Relocator;
class Modificator;
class SnippetGenerator;
class Command;
class BatchCommand;
class Patcher;

class Snippet;
typedef boost::shared_ptr<Snippet> SnippetPtr;

typedef std::map<PatchFunction*, PatchFunction*> FuncModMap;
typedef std::map<PatchFunction*, std::pair<PatchFunction*, std::string> > FuncWrapMap;

// This is a little complex, so let me explain my logic
// Map from B -> F_c -> F
// B identifies a call site
// F_c identifies an (optional) function context for the replacement
//   ... if F_c is not specified, we use NULL
// F specifies the replacement callee; if we want to remove the call entirely,
// also use NULL
typedef std::map<PatchBlock*, std::map<PatchFunction*, PatchFunction*> > CallModMap;

typedef std::set<ParseAPI::CodeObject*> CodeObjectSet;
typedef std::set<ParseAPI::CodeSource*> CodeSourceSet;
}
}

#if defined(_MSC_VER)
#define patchapi_debug(...)
#else
// Get basename
#include <libgen.h> 
#define patchapi_debug(...) do {        \
  if (getenv("DYNINST_DEBUG_PATCHAPI")) {   \
  const char* nodir = strrchr(__FILE__, '/'); \
  nodir = nodir ? nodir+1 : __FILE__; \
  fprintf(stderr, "%s [%d]: ", nodir, __LINE__); \
  fprintf(stderr, __VA_ARGS__); \
  fprintf(stderr, "\n"); \
  fflush(stderr); \
  } \
} while(0)
#endif

using std::map;
using std::list;
using std::set;
using std::vector;
using std::cerr;

#endif  // PATCHAPI_H_COMMON_H_
