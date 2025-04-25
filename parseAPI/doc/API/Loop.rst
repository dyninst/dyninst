Class Loop
----------

The Loop class represents code that may execute repeatedly. We detect
both natural loops (loops that have a single entry block) and
irreducible loops (loops that have multiple entry blocks). A back edge
is defined as an edge that has its source in the loop and has its target
being an entry block of the loop. It represents the end of an iteration
of the loop. For all the loops detected in a function, we also build a
loop nesting tree to represent the nesting relations between the loops.
See class for more details.

Loop\* parent

bool containsAddress(Address addr)

bool containsAddressInclusive(Address addr)

int getLoopEntries(vector<Block*>& entries);

int getBackEdges(vector<Edge*> &edges)

bool getContainedLoops(vector<Loop*> &loops)

bool getOuterLoops(vector<Loop*> &loops)

bool getLoopBasicBlocks(vector<Block*> &blocks)

bool getLoopBasicBlocksExclusive(vector<Block*> &blocks)

bool hasBlock(Block \*b);

bool hasBlockExclusive(Block \*b);

bool hasAncestor(Loop \*loop)

Function \* getFunction();
