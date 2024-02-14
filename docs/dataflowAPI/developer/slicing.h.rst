.. _`sec-dev:slicing.h`:

slicing.h
#########

An algorithm to generate a slice graph.

The slice graph is a directed graph that consists of nodes
corresponding to assignments from a set of inputs to an
output, where each input or output is an abstract region
(:cpp:class:`Dyninst::AbsRegion`) describing a register or a stack or heap
location. Edges in the slice graph indicate flow from the
output AbsRegion of the source node to an input AbsRegion of
the target node. Edges are typed with an AbsRegion
corresponding to the input region the flow describes; this
edge typing is necessary because two AbsRegions in different
assignments may be refer to equivalent locations without
being identical. Consider, for example, the transformation
of stack locations across function calls in interprocedural
slices.

Implementation details:

The slicing algorithm searches either forward or backward
from an initial assignment in the CFG, with the search
termination controlled in part by a set of user-provided
predicates (indicating, e.g., whether to follow call edges).
At each step, the slicer maintains an active set of
AbsRegions for which we are searching for related
assignments (uses, in the forward case; definitions, in the
backward case); this set is updated as the search
progresses. The graph is linked up on the way "down" the
slice recursion.

To avoid redundantly revisiting "down-slice" instructions
due to forks in the CFG, AbsRegion assignments are cached as
the search completes recursion down a particular branch.
Because CFGs are loopy directeg graphs, however, this does
not lead to an optimimal search ordering; it is possible to
construct pathological cases in which down-slice edges are
visited multiple times due to novel AbsRegions arising on
different paths. The caching of down-slice AbsRegions only
guarantees that a particular AbsRegion is only searched for
once along a given path---the loopy nature of the graph
prevents optimal search stragegies such as a topologically
sorted ordering that would be possible in a DAG.

The algorithm goes more or less like this::

    A_0 <- initial assignment
    F <- initialize frame from active AbsRegions in A_0
    // F contains `active' set of AbsRegions
  
    sliceInternalAux( F ) :
  
      // find assignments in the current instruction,
      // add them to the graph if appropriate, update
      // the active set:
      // active <- active \ killed U matches
      updateAndLink(F)
  
      // `successor' is direction-appropriate
      foreach successor NF of F
         if visited(F->NF) // edge visited
  
            // try to add assignments cached from down-slice
            updateAndLinkFromCache(NF)
  
            // remove AbsRegions that have been visited
            // along this edge from the active set
            removeBlocked(NF)
  
            // active is empty unless this instruction
            // introduced new active regions (in
            // updateAndLinkFromCache)
  
         visited(F->NF) <- true
         // recurse
         sliceInternalAux( NF )
         // merge cached definitions, except those generated
         // in F
         cache[F] <- cache[F] U (cache[NF] \ defs[F]

 Clearly the `find successors' bit is quite complicated
 and involves user-defined predicates and various CFG
 traversal rules, updating of AbsRegions, etc. Refer to
 comments in the code for more details.

.. cpp:namespace:: Dyninst::dev


.. cpp:class:: Slicer

  .. cpp:type:: std::deque<Dyninst::Slicer::ContextElement> Context

  .. cpp:function:: Slicer(AssignmentPtr a, ParseAPI::Block *block, ParseAPI::Function *func, \
                           bool cache = true, bool stackAnalysis = true)

      TODO: make this function-less interprocedural. That would throw the stack analysis for a loop, but is generally doable.

  .. cpp:function:: private bool getStackDepth(Dyninst::ParseAPI::Function *func, Dyninst::ParseAPI::Block *block,\
                                               Address callAddr, long &height)

    Our slicing is context-sensitive that is, if we enter a function foo from a caller bar, all return
    edges from foo must enter bar. This makes an assumption that the return address is not modified.
    We represent this as a list of call sites. This is redundant with the image_instPoint data structure,
    but hopefully that one will be going away.

  .. cpp:function:: private void pushContext(Context &context, Dyninst::ParseAPI::Function *callee,\
                                             Dyninst::ParseAPI::Block *callBlock, long stackDepth)

    Add the newly called function to the given Context.

  .. cpp:function:: private void popContext(Context &context)

    Remove the newly called function to the given Context.

  .. cpp:type:: private std::queue<Location> LocList
  .. cpp:function:: private bool ReachableFromBothBranches(Dyninst::ParseAPI::Edge *e, std::vector<Dyninst::Slicer::Element> &newE)

  .. cpp:function:: void shiftAbsRegion(AbsRegion const &callerReg, AbsRegion &calleeReg, long stack_depth, ParseAPI::Function *callee)

    Shift an abs region by a given stack offset

  .. cpp:function:: void shiftAllAbsRegions(SliceFrame &cur, long stack_depth, ParseAPI::Function *callee)

    Shift all of the abstract regions active in the current frame

  .. cpp:function:: GraphPtr sliceInternal(Direction dir, Predicates &predicates)

  .. cpp:function:: void sliceInternalAux(GraphPtr g, Direction dir, Predicates &p, SliceFrame &cand, bool skip, std::map<CacheEdge,\
                                          std::set<AbsRegion>> &visited, std::unordered_map<Address, DefCache> &single,\
                                          std::unordered_map<Address, DefCache> &cache)

      main slicing routine. creates any new edges if they are part of the
      slice, and recursively slices on the next instruction(s).

      skip - skip linking this frame; for bootstrapping

  .. cpp:function:: bool updateAndLink(GraphPtr g, Direction dir, SliceFrame &cand, DefCache &cache, Predicates &p)

      converts the current instruction into assignments and looks for matching
      elements in the active map. if any are found, graph nodes and edges are
      created. this function also updates the active map to be contain only the
      elements that are valid after the above linking (killed defs are removed).

  .. cpp:function:: void updateAndLinkFromCache(GraphPtr g, Direction dir, SliceFrame &f, DefCache &cache)

      similar to updateAndLink, but this version only looks at the unified cache. it then inserts edges for matching elements.

  .. cpp:function:: void removeBlocked(SliceFrame &f, std::set<AbsRegion> const &block)
  .. cpp:function:: bool stopSlicing(SliceFrame::ActiveMap &active, GraphPtr g, Address addr, Direction dir)
  .. cpp:function:: void markVisited(std::map<CacheEdge, std::set<AbsRegion>> &visited, CacheEdge const &e, SliceFrame::ActiveMap const &active)
  .. cpp:function:: void cachePotential(Direction dir, Assignment::Ptr assn, DefCache &cache)

  .. cpp:function:: bool findMatch(GraphPtr g, Direction dir, SliceFrame const &cand, AbsRegion const &cur, Assignment::Ptr assn,\
                                   std::vector<Dyninst::Slicer::Element> &matches, DefCache &cache)

      Compare the assignment ``assn`` to the abstract region ``cur``
      and see whether they match, for the direction-appropriate
      definition of "match". If so, generate new slice elements
      and return them in the `match' vector, after linking them
      to the elements associated with the region ``cur``.
      Return true if these exists at least a match.

  .. cpp:function:: bool getNextCandidates(Direction dir, Predicates &p, SliceFrame const &cand, std::vector<SliceFrame> &newCands)

  ......

  .. rubric::
    Forward Slicing

  .. cpp:function:: bool getSuccessors(Predicates &p, SliceFrame const &cand, std::vector<SliceFrame> &newCands)

      Given the location (instruction) in ``cand``, find zero or more
      control flow successors from this location and create new slicing
      frames for them. Certain types of control flow require mutation of
      the SliceFrame (modification of context, e.g.) AND mutate the
      abstract regions in the frame's active list (e.g. modifying
      stack locations).

  .. cpp:function:: bool handleCall(Predicates &p, SliceFrame &cur, bool &err)

      Process a call instruction, determining whether to follow the
      call edge (with the help of the predicates) or the fallthrough edge

  .. cpp:function:: bool followCall(Predicates &p, ParseAPI::Block *target, SliceFrame &cur)

      Builds up a call stack and callee function, and ask the predicate whether we should follow the call (or,
      implicitly, follow its fallthrough edge instead).

  .. cpp:function:: bool handleCallDetails(SliceFrame &cur, ParseAPI::Block *caller_block)

      Adjust the slice frame's context and translates the abstract regions in the active list from caller to callee

  .. cpp:function:: bool handleReturn(Predicates &p, SliceFrame &cur, bool &err)

      Properly adjusts the location & context of the slice frame and the AbsRegions of its active elements.

  .. cpp:function:: void handleReturnDetails(SliceFrame &cur)

      Do the actual context popping and active AbsRegion translation.

  .. cpp:function:: bool handleDefault(Direction dir, Predicates &p, ParseAPI::Edge *e, SliceFrame &cur, bool &err)

  ......

  .. rubric::
    Backward Slicing

  .. cpp:function:: bool getPredecessors(Predicates &p, SliceFrame const &cand, std::vector<SliceFrame> &newCands)

      Same as successors, only backwards

  .. cpp:function:: bool handleCallBackward(Predicates &p, SliceFrame const &cand, std::vector<SliceFrame> &newCands, ParseAPI::Edge *e, bool &err)
  .. cpp:function:: std::vector<ParseAPI::Function *> followCallBackward(Predicates &p, SliceFrame const &cand, AbsRegion const &reg, ParseAPI::Block *caller_block)

      FIXME: egregious copying

  .. cpp:function:: bool handleCallDetailsBackward(SliceFrame &cur)
  .. cpp:function:: bool handleReturnBackward(Predicates &p, SliceFrame const &cand, SliceFrame &newCand, ParseAPI::Edge *e, bool &err)
  .. cpp:function:: bool handleReturnDetailsBackward(SliceFrame &cur, ParseAPI::Block *caller_block)
  .. cpp:function:: bool followReturn(Predicates &p, SliceFrame const &cand, ParseAPI::Block *source)
  .. cpp:function:: void handlePredecessorEdge(ParseAPI::Edge *e, Predicates &p, SliceFrame const &cand, std::vector<SliceFrame> &newCands, bool &err, SliceFrame &nf)

  ......

  .. rubric::
    General slicing support

  .. cpp:function:: void constructInitialFrame(Direction dir, SliceFrame &initFrame)

      creates the initial slice frame and initializes instance variables.

  .. cpp:function:: void widenAll(GraphPtr graph, Direction dir, SliceFrame const &frame)
  .. cpp:function:: bool kills(AbsRegion const &reg, Assignment::Ptr &assign)
  .. cpp:function:: void widen(GraphPtr graph, Direction dir, Dyninst::Slicer::Element const &source)
  .. cpp:function:: void insertPair(GraphPtr graph, Direction dir, Dyninst::Slicer::Element const &source, Dyninst::Slicer::Element const &target, AbsRegion const &data)

      Iinserts an edge from source to target (forward) or target to source (backward) if the edge does not yet exist. this is done by converting source and target
      to graph nodes (creating them if they do not exist).

  .. cpp:function:: void insertPair(GraphPtr graph, Direction dir, SliceNode::Ptr &source, SliceNode::Ptr &target, AbsRegion const &data)

      inserts an edge from source to target (forward) or target to source (backward) if the edge does not yet exist.

  .. cpp:function:: void convertInstruction(const InstructionAPI::Instruction &, Address, ParseAPI::Function *, ParseAPI::Block *, std::vector<AssignmentPtr> &)

      Converts an instruction to a vector of assignments. if this slicer has already converted this instruction, this function returns the same assignments.

      Note that we CANNOT use a global cache based on the address of the instruction to convert because the block that contains the instructino may change during parsing.

  .. cpp:function:: void fastForward(Location &loc, Address addr)
  .. cpp:function:: void fastBackward(Location &loc, Address addr)
  .. cpp:function:: SliceNode::Ptr widenNode()
  .. cpp:function:: void markAsEndNode(GraphPtr ret, Direction dir, Dyninst::Slicer::Element &current)
  .. cpp:function:: void markAsExitNode(GraphPtr ret, Dyninst::Slicer::Element &current)
  .. cpp:function:: void markAsEntryNode(GraphPtr ret, Dyninst::Slicer::Element &current)
  .. cpp:function:: void getInsns(Location &loc)
  .. cpp:function:: void setAliases(Assignment::Ptr, Dyninst::Slicer::Element &)
  .. cpp:function:: SliceNode::Ptr createNode(Dyninst::Slicer::Element const &)

      Creates a new node from an element if that node does not yet exist. otherwise, it returns the pre-existing node.

  .. cpp:function:: void cleanGraph(GraphPtr g)

      removes unnecessary nodes from the slice graph. this is currently mostly ia32/amd64 flags that are written but never read.

  .. cpp:function:: void promotePlausibleNodes(GraphPtr g, Direction d)

      promotes nodes in the slice graph to termination nodes. essentially, the slicer maintains a set of nodes that
      may be entry/exit nodes for the backwards/fowards case. this function removes the nodes from the set, and
      marks them in the graph as true entry/exit nodes. in the forward case, the entry node is a single node,
      the assignment from which the slice began. in the backward case, this node is the single exit node. exit nodes in the
      forward case are definitions that are still live at function exit. entry nodes in the backward case are uses for which the
      definition lies outside the function (before entry) or move instructions where one operand is a literal.

  .. cpp:function:: ParseAPI::Block *getBlock(ParseAPI::Edge *e, Direction dir)
  .. cpp:function:: void insertInitialNode(GraphPtr ret, Direction dir, SliceNode::Ptr aP)
  .. cpp:function:: void mergeRecursiveCaches(std::unordered_map<Address, DefCache> &sc, std::unordered_map<Address, DefCache> &c, Address a)

      merges all single caches that have occured single addr in the recursion into the appropriate unified caches.

  .. cpp:member:: InsnCache *insnCache_
  .. cpp:member:: bool own_insnCache
  .. cpp:member:: AssignmentPtr a_
  .. cpp:member:: ParseAPI::Block *b_
  .. cpp:member:: ParseAPI::Function *f_
  .. cpp:member:: std::unordered_map<AssignmentPtr, SliceNode::Ptr, Assignment::AssignmentPtrHasher> created_

    Assignments map to unique slice nodes

  .. cpp:member:: std::unordered_map<EdgeTuple, int, EdgeTupleHasher> unique_edges_
  .. cpp:type:: std::map<AbsRegion, std::set<Dyninst::Slicer::Element>> PrevMap

    map of previous active maps. these are used to end recursion.

  .. cpp:member:: std::map<Address, PrevMap> prev_maps
  .. cpp:member:: std::set<SliceNode::Ptr> plausibleNodes

    set of plausible entryexit nodes.

  .. cpp:member:: std::deque<Address> addrStack

    a stack and set of addresses that mirror our recursion. these are used to detect loops and properly merge cache.

  .. cpp:member:: std::set<Address> addrSet
  .. cpp:member:: AssignmentConverter *converter
  .. cpp:member:: bool own_converter
  .. cpp:member:: SliceNode::Ptr widen_


.. cpp:struct:: Slicer::EdgeTupleHasher
  
  Cache to prevent edge duplication

  .. cpp:function:: size_t operator()(const EdgeTuple &et) const


.. cpp:enum:: Slicer::Direction

  .. cpp:enumerator:: forward
  .. cpp:enumerator:: backward

.. cpp:struct:: Slicer::Def

  An element that is a slicing ``def`` (where def means "definition" in the backward case and "use"
  in the forward case, along with the associated AbsRegion that labels the slicing edge. These two
  pieces of information, along with an  element describing the other end of the dependency, are what
  you need to create a slice edge.

  .. cpp:function:: Def(Dyninst::Slicer::Element const& e, AbsRegion const& r)
  .. cpp:member:: Dyninst::Slicer::Element ele
  .. cpp:member:: AbsRegion data
  .. cpp:function:: bool operator<(Def const& o) const

    only the Assignment::Ptr of an Dyninst::Slicer::Element matters for comparison

  .. cpp:function:: bool operator==(Def const &o) const


.. cpp:struct:: Slicer::Def::DefHasher

  .. cpp:function:: size_t operator()(const Def &o) const

.. cpp:class:: Slicer::DefCache

  A cache from AbsRegions -> Defs.

  Each node that has been visited in the search has a DefCache that reflects the resolution of any
  AbsRegions down-slice. If the node is visited again through a different search path (if the graph
  has fork-join structure), this caching prevents expensive recursion

  .. cpp:function:: DefCache()
  .. cpp:function:: ~DefCache()
  .. cpp:function:: void merge(DefCache const& o)

    add the values from another defcache

  .. cpp:function:: void replace(DefCache const& o)

    replace mappings in this cache with those from another

  .. cpp:function:: std::set<Def> & get(AbsRegion const& r)
  .. cpp:function:: bool defines(AbsRegion const& r) const
  .. cpp:function:: void print() const
  .. cpp:member:: private std::map< AbsRegion, std::set<Def> > defmap


.. cpp:struct:: Slicer::EdgeTuple

  For preventing insertion of duplicate edges into the slice graph

  .. cpp:function:: private EdgeTuple(SliceNode::Ptr src, SliceNode::Ptr dst, AbsRegion const& reg)
  .. cpp:function:: private bool operator<(EdgeTuple const& o) const
  .. cpp:function:: private bool operator ==(EdgeTuple const &o) const
  .. cpp:member:: private SliceNode::Ptr s
  .. cpp:member:: private SliceNode::Ptr d
  .. cpp:member:: private AbsRegion r


.. cpp:struct:: CacheEdge

  Used for keeping track of visited edges in the slicing search

  .. cpp:function:: CacheEdge(Address src, Address trg)
  .. cpp:member:: Address s
  .. cpp:member:: Address t
  .. cpp:function:: bool operator<(CacheEdge const& o) const


.. cpp:class:: SliceNode : public Node

  Used in temp slicer should probably replace OperationNodes when we fix up the DDG code.

  .. cpp:type:: boost::shared_ptr<SliceNode> Ptr
  .. cpp:function:: static SliceNode::Ptr create(AssignmentPtr ptr, Dyninst::ParseAPI::Block *block, Dyninst::ParseAPI::Function *func)
  .. cpp:function:: Dyninst::ParseAPI::Block *block() const
  .. cpp:function:: Dyninst::ParseAPI::Function *func() const
  .. cpp:function:: Address addr() const
  .. cpp:function:: AssignmentPtr assign() const
  .. cpp:function:: Node::Ptr copy()
  .. cpp:function:: bool isVirtual() const
  .. cpp:function:: std::string format() const
  .. cpp:function:: virtual ~SliceNode()
  .. cpp:function:: private SliceNode(AssignmentPtr ptr, Dyninst::ParseAPI::Block *block, Dyninst::ParseAPI::Function *func)
  .. cpp:member:: private AssignmentPtr a_
  .. cpp:member:: private Dyninst::ParseAPI::Block *b_
  .. cpp:member:: private Dyninst::ParseAPI::Function *f_


.. cpp:class:: SliceEdge : public Edge

  .. cpp:type:: boost::shared_ptr<SliceEdge> Ptr
  .. cpp:function:: static SliceEdge::Ptr create(SliceNode::Ptr source, SliceNode::Ptr target, AbsRegion const &data)
  .. cpp:function:: const AbsRegion &data() const
  .. cpp:function:: private SliceEdge(const SliceNode::Ptr source, const SliceNode::Ptr target, AbsRegion const &data)
  .. cpp:member:: private AbsRegion data_


.. cpp:class:: Slicer::Predicates

  .. cpp:function:: bool performCacheClear()
  .. cpp:function:: void setClearCache(bool b)

  .. cpp:function:: virtual int slicingSizeLimitFactor()

    A negative number means that we do not bound slicing size.

  .. cpp:function:: virtual bool allowImprecision()

  .. cpp:function:: virtual bool widenAtAssignment(const AbsRegion&* in, const AbsRegion& out)

