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

// Old Dyninst
#include "dyn_detail/boost/shared_ptr.hpp"
#include "dyn_detail/boost/enable_shared_from_this.hpp"
#include "dyntypes.h"
#include "common/h/IntervalTree.h"
#include "common/h/Types.h"

// For debug
extern PATCHAPI_EXPORT bool debug_patchapi_flag;
#define patch_cerr if (debug_patchapi_flag) std::cerr

// For formating debug information
#define  ws2 "  "
#define  ws4 "    "
#define  ws6 "      "
#define  ws8 "        "
#define ws10 "          "
#define ws12 "            "
#define ws14 "              "
#define ws16 "                "
#define ws18 "                  "
#define ws20 "                    "

namespace Dyninst {
namespace PatchAPI {

class AddrSpace;
typedef dyn_detail::boost::shared_ptr<AddrSpace> AddrSpacePtr;

class CFGMaker;
typedef dyn_detail::boost::shared_ptr<CFGMaker> CFGMakerPtr;

class PatchObject;

class PointMaker;
typedef dyn_detail::boost::shared_ptr<PointMaker> PointMakerPtr;

class Point;
typedef std::set<Point*> PointSet;
typedef PointSet::iterator PointIter;

class Instance;
typedef dyn_detail::boost::shared_ptr<Instance> InstancePtr;
typedef std::set<InstancePtr> InstanceSet;
typedef std::list<InstancePtr> InstanceList;

class Instrumenter;
typedef dyn_detail::boost::shared_ptr<Instrumenter> InstrumenterPtr;

class Linker;
typedef dyn_detail::boost::shared_ptr<Linker> LinkerPtr;

class PatchMgr;
typedef dyn_detail::boost::shared_ptr<PatchMgr> PatchMgrPtr;

class PatchFunction;
class PatchBlock;
class PatchEdge;

class Relocator;
class Modificator;
class SnippetGenerator;

class Command;
typedef dyn_detail::boost::shared_ptr<Command> CommandPtr;
typedef std::list<CommandPtr> CommandList;

class BatchCommand;
typedef dyn_detail::boost::shared_ptr<BatchCommand> BatchCommandPtr;

class Patcher;
typedef dyn_detail::boost::shared_ptr<Patcher> PatcherPtr;

typedef dyn_detail::boost::shared_ptr<void> VoidPtr;
typedef VoidPtr SnippetPtr;

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

using std::map;
using std::list;
using std::set;
using std::vector;
using std::cerr;

#endif  // PATCHAPI_H_COMMON_H_
