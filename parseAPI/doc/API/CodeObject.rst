Class CodeObject
----------------

The CodeObject class describes an individual binary code object, such as
an executable or library. It is the top-level container for parsing the
object as well as accessing that parse data. The following API routines
and data types are provided to support parsing and retrieving parsing
products.

typedef std::set<Function \*, Function::less> funclist

CodeObject(CodeSource \* cs, CFGFactory \* fact = NULL, ParseCallback \*
cb = NULL, bool defensiveMode = false)

void parse()

void parse(Address target, bool recursive)

void parse(CodeRegion \* cr, Address target, bool recursive)

struct NewEdgeToParse Block \*source; Address target; EdgeTypeEnum type;
bool parseNewEdges( vector<NewEdgeToParse> & worklist )

ParseAPI is able to speculatively parse gaps (regions of binary that has
not been identified as code or data yet) to identify function entry
points and perform control flow traversal.

================
=================================================================================================================================================================================
GapParsingType   Technique description
================
=================================================================================================================================================================================
PreambleMatching If instruction patterns are matched at an adderss, the address is a function entry point
IdiomMatching    Based on a pre-trained model, this technique calculates the probability of an address to be a function entry point and predicts whether which addresses are function entry points
================
=================================================================================================================================================================================

void parseGaps(CodeRegion \*cr, GapParsingType type=IdiomMatching)

Function \* findFuncByEntry(CodeRegion \* cr, Address entry)

int findFuncs(CodeRegion \* cr, Address addr, std::set<Function*> &
funcs)

int findFuncs(CodeRegion \* cr, Address start, Address end,
std::set<Function*> & funcs)

const funclist & funcs()

Block \* findBlockByEntry(CodeRegion \* cr, Address entry)

int findBlocks(CodeRegion \* cr, Address addr, std::set<Block*> &
blocks)

Block \* findNextBlock(CodeRegion \* cr, Address addr)

CodeSource \* cs()

CFGFactory \* fact()

bool defensiveMode()

bool isIATcall(Address insn, std::string &calleeName)

void startCallbackBatch()

void finishCallbackBatch()

void registerCallback(ParseCallback \*cb);

void unregisterCallback(ParseCallback \*cb);

void finalize()

void destroy(Edge \*)

void destroy(Block \*)

void destroy(Function \*)
