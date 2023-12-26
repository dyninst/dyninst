CodeObject.h
============

.. cpp:namespace:: Dyninst::parseAPI

Class CodeObject
----------------

**Defined in:** ``CodeObject.h``

The CodeObject class describes an individual binary code object, such as
an executable or library. It is the top-level container for parsing the
object as well as accessing that parse data. The following API routines
and data types are provided to support parsing and retrieving parsing
products.

.. code-block:: cpp
    
    typedef std::set<Function *, Function::less> funclist

Container for access to functions. Refer to Section
`4.12 <#sec:containers>`__ for details. Library users *must not* rely on
the underlying container type of std::set, as it is subject to change.

.. code-block:: cpp

    CodeObject(CodeSource * cs, CFGFactory * fact = NULL, ParseCallback *
    cb = NULL, bool defensiveMode = false)

Constructs a new CodeObject from the provided CodeSource and optional
object factory and callback handlers. Any parsing hints provided by the
CodeSource are processed, but the binary is not parsed when this
constructor returns. The passed CodeSource is **not** owned by this
object. However, it must have the same lifetime as the CodeObject.

The ``defensiveMode`` parameter optionally trades off coverage for
safety; this mode is not recommended for most applications as it makes
very conservative assumptions about control flow transfer instructions
(see Section `6 <#sec:defmode>`__).

.. code-block:: cpp
    
    void parse()

Recursively parses the binary represented by this CodeObject from all
known function entry points (i.e., the hints provided by the
CodeSource). This method and the following parsing methods may safely be
invoked repeatedly if new information about function locations is
provided through the CodeSource. Note that these parsing methods do not
automatically perform speculative gap parsing. parseGaps should be used
for this purpose.

.. code-block:: cpp

    void parse(Address target, bool recursive)

Parses the binary starting with the instruction at the provided target
address. If ``recursive`` is true, recursive traversal parsing is used
as in the default ``parse()`` method; otherwise only instructions
reachable through intraprocedural control flow are visited.

.. code-block:: cpp

    void parse(CodeRegion * cr, Address target, bool recursive)

Parses the specified core region of the binary starting with the
instruction at the provided target address. If ``recursive`` is true,
recursive traversal parsing is used as in the default ``parse()``
method; otherwise only instructions reachable through intraprocedural
control flow are visited.

.. code-block:: cpp

    struct NewEdgeToParse Block *source; Address target; EdgeTypeEnum type;
    bool parseNewEdges( vector<NewEdgeToParse> & worklist )

Parses a set of newly created edges specified in the worklist supplied
that were not included when the function was originally parsed.

ParseAPI is able to speculatively parse gaps (regions of binary that has
not been identified as code or data yet) to identify function entry
points and perform control flow traversal.

.. container:: center

   +------------------+--------------------------------------------------+
   | GapParsingType   | Technique description                            |
   +==================+==================================================+
   | PreambleMatching | If instruction patterns are matched at an        |
   |                  | adderss, the address is a function entry point   |
   +------------------+--------------------------------------------------+
   | IdiomMatching    | Based on a pre-trained model, this technique     |
   |                  | calculates the probability of an address to be a |
   |                  | function entry point and predicts whether which  |
   |                  | addresses are function entry points              |
   +------------------+--------------------------------------------------+


.. code-block:: cpp
   
    void parseGaps(CodeRegion *cr, GapParsingType type=IdiomMatching)

Speculatively parse the indicated region of the binary using the
specified technique to find likely function entry points, enabled on the
x86 and x86-64 platforms.

.. code-block:: cpp
    
    Function * findFuncByEntry(CodeRegion * cr, Address entry)

Find the function starting at address ``entry`` in the indicated
CodeRegion. Returns null if no such function exists.

.. code-block:: cpp

    int findFuncs(CodeRegion * cr, Address addr, std::set<Function*> & funcs)

Finds all functions spanning ``addr`` in the code region, adding each to
``funcs``. The number of results of this stabbing query are returned.

.. code-block:: cpp 

    int findFuncs(CodeRegion * cr, Address start, Address end,
    std::set<Function*> & funcs)

Finds all functions overlapping the range ``[start,end)`` in the code
region, adding each to ``funcs``. The number of results of this stabbing
query are returned.

.. code-block:: cpp

    const funclist & funcs()

Returns a const reference to a container of all functions in the binary.
Refer to Section `4.12 <#sec:containers>`__ for container access
details.

.. code-block:: cpp
    
    Block * findBlockByEntry(CodeRegion * cr, Address entry)

Find the basic block starting at address ``entry``. Returns null if no
such block exists.

.. code-block:: cpp

    int findBlocks(CodeRegion * cr, Address addr, std::set<Block*> & blocks)

Finds all blocks spanning ``addr`` in the code region, adding each to
``blocks``. Multiple blocks can be returned only on platforms with
variable-length instruction sets (such as IA32) for which overlapping
instructions are possible; at most one block will be returned on all
other platforms.

.. code-block:: cpp

    Block * findNextBlock(CodeRegion * cr, Address addr)

Find the next reachable basic block starting at address ``entry``.
Returns null if no such block exists.

.. code-block:: cpp
    
    CodeSource * cs()

Return a reference to the underlying CodeSource.

.. code-block:: cpp
    
    CFGFactory * fact()

Return a reference to the CFG object factory.

.. code-block:: cpp
    
    bool defensiveMode()

Return a boolean specifying whether or not defensive mode is enabled.

.. code-block:: cpp
    
    bool isIATcall(Address insn, std::string &calleeName)

Returns a boolean specifying if the address at ``addr`` is located at
the call named in ``calleeName``.

.. code-block:: cpp
    
    void startCallbackBatch()

Starts a batch of callbacks that have been registered.

.. code-block:: cpp
    
    void finishCallbackBatch()

Completes all callbacks in the current batch.

.. code-block:: cpp
    
    void registerCallback(ParseCallback *cb);

Register a callback ``cb``

.. code-block:: cpp
    
    void unregisterCallback(ParseCallback *cb);

Unregister an existing callback ``cb``

.. code-block:: cpp
    
    void finalize()

Force complete parsing of the CodeObject; parsing operations are
otherwise completed only as needed to answer queries.

.. code-block:: cpp
    
    void destroy(Edge *)

Destroy the edge listed.

.. code-block:: cpp
    
    void destroy(Block *)

Destroy the code block listed.

.. code-block:: cpp
    
    void destroy(Function *)

Destroy the function listed.