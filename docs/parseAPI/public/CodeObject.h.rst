.. _`sec:CodeObject.h`:

CodeObject.h
############

.. cpp:namespace:: Dyninst::ParseAPI

.. cpp:class:: CodeObject

  **An individual binary code object**

  A CodeObject defines a collection of binary code, for example a binary,
  dynamic library, archive, memory snapshot, etc. In the context of
  Dyninst, it maps to an :cpp:class:`image`.

  .. cpp:type:: std::set<Function*, Function::less> funclist

      Container for access to functions.

  .. cpp:function:: CodeObject(CodeSource* cs, CFGFactory* fact = NULL, \
                               ParseCallback* cb = NULL, bool defensiveMode = false, \
                               bool ignoreParse = false)

      Creates a CodeObject using the :cpp:class:`CodeSource` and optional object factory and callback handlers.

      Any parsing hints provided by the CodeSource are processed, but the binary is not parsed when this
      constructor returns. The passed CodeSource is **not** owned by this object. However, it must have
      the same lifetime as the CodeObject.

      ``defensiveMode`` optionally trades off coverage for safety; this mode is not recommended
      for most applications as it makes very conservative assumptions about control flow transfer instructions.
      This is only applicable on Windows.

      ``ignoreParse`` disables automatic parsing during construction.

  .. cpp:function:: void parse()

      Recursively parses the binary

      Includes all known function entry points (i.e., the hints provided by the
      CodeSource). This method and the following parsing methods may safely be
      invoked repeatedly if new information about function locations is
      provided through the CodeSource. Note that these parsing methods do not
      automatically perform speculative gap parsing. :cpp:func:`parseGaps` should be used
      for this purpose.

  .. cpp:function:: void parse(Address target, bool recursive)

      Parses the binary starting with the instruction starting at ``target``. ``recursive``
      enables recursive traversal parsing when enabled; otherwise only instructions
      reachable through intraprocedural control flow are visited.

  .. cpp:function:: void parse(CodeRegion* cr, Address target, bool recursive)

      Parses the region, ``cr``, of the binary starting at ``target``. ``recursive``
      enables recursive traversal parsing when enabled; otherwise only instructions
      reachable through intraprocedural control flow are visited.

  .. cpp:function:: bool parseNewEdges(vector<NewEdgeToParse>& worklist)

      Parses a set of newly created edges specified in ``worklist``
      that were not included when the function was originally parsed.

      ParseAPI is able to speculatively parse gaps (regions of binary that has
      not been identified as code or data yet) to identify function entry
      points and perform control flow traversal.

  .. cpp:function:: void parseGaps(CodeRegion* cr, GapParsingType type=IdiomMatching)

      Speculatively parses the region of the binary in ``cr`` using the
      specified technique, ``type``, to find likely function entry points.

  .. cpp:function:: Function* findFuncByEntry(CodeRegion* cr, Address entry)

      Find the function starting at address ``entry`` in the indicated
      CodeRegion. Returns null if no such function exists.

  .. cpp:function:: int findFuncs(CodeRegion* cr, Address addr, std::set<Function*> & funcs)

      Finds all functions spanning ``addr`` in the code region, ``cr``, adding each to
      ``funcs``. Returns the number of functions found.

  .. cpp:function:: int findFuncsByBlock(CodeRegion* cr, Block* b, std::set<Function*> &funcs)

      Finds all functions spanning the addresses covered by ``b`` in the code region, ``cr``,
      adding each to ``funcs``. Returns the number of functions found.

  .. cpp:function:: int findFuncs(CodeRegion* cr, Address start, Address end, std::set<Function*> & funcs)

      Finds all functions overlapping the range ``[start,end)`` in the code
      region, ``cr``, adding each to ``funcs``. Returns the number of functions found.

  .. cpp:function:: int findCurrentFuncs(CodeRegion* cr, Address addr, std::set<Function*> & funcs)

      Finds all functions spanning ``addr`` in the code region, ``cr``, adding each to
      ``funcs``, but does not implicitly invoke parsing. Returns the number of functions found.

  .. cpp:function:: const funclist & funcs()

      Returns all functions in the binary.

  .. cpp:function:: Block* findBlockByEntry(CodeRegion* cr, Address entry)

      Find the basic block starting at address ``entry`` in the code region ``cr``.

      Returns ``NULL`` if no block exists.

  .. cpp:function:: int findBlocks(CodeRegion* cr, Address addr, std::set<Block*> & blocks)

      Finds all blocks spanning ``addr`` in the code region, adding each to
      ``blocks``. Multiple blocks can be returned only on platforms with
      variable-length instruction sets (such as IA32) for which overlapping
      instructions are possible; at most one block will be returned on all
      other platforms.

  .. cpp:function:: Block* findNextBlock(CodeRegion* cr, Address addr)

      Find the next reachable basic block starting at address ``entry``.

      Returns ``NULL`` if not found.

  .. cpp:function:: CodeSource* cs()

      Returns the underlying CodeSource.

  .. cpp:function:: bool defensiveMode()

      Checks if defensive mode is enabled.

  .. cpp:function:: bool isIATcall(Address insn, std::string &calleeName)

      Checks if the address at ``addr`` is located at the function call ``calleeName``.

.. cpp:struct:: CodeObject::NewEdgeToParse

  **Parses new edges in already parsed functions**

  .. cpp:member:: Block*source
  .. cpp:member:: Address target
  .. cpp:member:: EdgeTypeEnum type
  .. cpp:member:: bool checked

      ``true`` if call_ft edges have already had their callees checked.

.. cpp:enum:: GapParsingType

  .. cpp:enumerator:: PreambleMatching

    Instruction patterns are matched at a function entry point

  .. cpp:enumerator:: IdiomMatching

    Based on a pre-trained model, this technique calculates the probability of an address to be a
    function entry point and predicts whether which addresses are function entry points.
