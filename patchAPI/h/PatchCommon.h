#ifndef PATCHAPI_H_COMMON_H_
#define PATCHAPI_H_COMMON_H_

// C++
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
#define patchapi_debug(...) do { \
  if (getenv("DYNINST_DEBUG_PATCHAPI")) {   \
  const char* nodir = basename(__FILE__);              \
  fprintf(stderr, "%s [%d]: ", nodir, __LINE__); \
  fprintf(stderr, __VA_ARGS__); \
  fprintf(stderr, "\n");  \
  fflush(stderr); \
  } \
  else {};                                      \
} while(0)
#endif

using std::map;
using std::list;
using std::set;
using std::vector;
using std::cerr;

#endif  // PATCHAPI_H_COMMON_H_
