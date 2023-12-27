.. _`sec:slicing.h`:

slicing.h
#########

.. cpp:namespace:: Dyninst

The slicing algorithm starts with a user provided Assignment
and generates a graph as the slicing results. A slice is represented as a
:cpp:class:`Dyninst::Graph` where the nodes are individual assignments that
affect the starting assignment (backward slicing) or are affected by the starting
assignment (forward slicing). The edges in the graph are directed and represent either
dataflow dependencies or controlflow dependencies.

Callback functions are provided and allow control over when to stop
slicing. In particular, :cpp:class:`Slicer::Predicates` contains a
collection of callback functions that can control the specific
behaviors of the slicer. Users can extend this class to provide customized
stopping criteria for the slicer.

.. cpp:type:: boost::shared_ptr<Assignment> AssignmentPtr
.. cpp:type:: boost::shared_ptr<Graph> GraphPtr
.. cpp:type:: boost::shared_ptr<InstructionAPI::Instruction> InstructionPtr

.. cpp:class:: Slicer

  **An interface for performing forward and backward slicing**

  .. cpp:type:: std::pair<InstructionAPI::Instruction, Address> InsnInstance

    An instruction and its associated address.

  .. cpp:type:: std::vector<InsnInstance> InsnVec

    A collection of :cpp:type:`InsnInstance`\ s encountered during slicing.

  .. cpp:type:: std::deque<ContextElement> Context

    A collection of :cpp:class:`ContextElement`\ s that creates the code region for slicing.

  .. cpp:type:: dyn_hash_map<Address, InsnVec> InsnCache

    An instruction cache to avoid redundant instruction decoding.
    A user can optionally provide a cache shared by multiple slicers.
    The cache key is the starting address of the basic block.

  .. cpp:member:: std::set<ParseAPI::Edge *> visitedEdges

    A set of edges that have been visited during slicing that can be used
    for external users to figure out which part of code has been analyzed.

  .. cpp:function:: Slicer(AssignmentPtr a, ParseAPI::Block *block, ParseAPI::Function *func, \
                           bool cache = true, bool stackAnalysis = true)

    Creates a Slicer to perform forward or backward slicing starting at the assignment ``a`` in the basic
    block ``block`` contained in ``func``. When ``cache`` is ``true``, conversions from instructions to
    assignments are cached for future use. When ``stackAnalysis`` is ``true``, stack analysis is used
    to distinguish stack variables.

  .. cpp:function:: Slicer(AssignmentPtr a, ParseAPI::Block *block, ParseAPI::Function *func, AssignmentConverter *ac)

    Creates a Slicer to perform forward or backward slicing starting at the assignment ``a`` in the basic
    block ``block`` contained in ``func`` using the converter ``ac``.

  .. cpp:function:: Slicer(AssignmentPtr a, ParseAPI::Block *block, ParseAPI::Function *func, AssignmentConverter *ac, InsnCache *c)

    Creates a Slicer to perform forward or backward slicing starting at the assignment ``a`` in the basic
    block ``block`` contained in ``func`` using the converter ``ac``. Instructions discovered during slicing
    are stored in ``c``.

  .. cpp:function:: static bool isWidenNode(Node::Ptr n)

    Checks if ``n`` is a :cpp:class:`SliceNode` that has been assigned to.

  .. cpp:function:: GraphPtr forwardSlice(Predicates &predicates)
  .. cpp:function:: GraphPtr backwardSlice(Predicates &predicates)

    Perform forward or backward slicing and use ``predicates`` to control
    the stopping criteria and return the slicing results as a graph

  .. cpp:function:: void getInsnsBackward(Location &loc)

    Retrieve the location of the previous instruction encountered during slicing.

.. cpp:struct:: Slicer::ContextElement

  **Description of an area of code under scrutiny for slicing**

  .. cpp:member:: ParseAPI::Function *func

    We can implicitly find the callsite given a block, since calls end blocks. It's easier to look up
    the successor this way than with an address.

  .. cpp:member:: ParseAPI::Block *block

    If non-NULL this must be an internal context element, since we have an active call site.

  .. cpp:member:: int stackDepth

    To enter or leave a function we must be able to map corresponding abstract regions.
    In particular, we need to know the depth of the stack in the caller.

  .. cpp:function:: ContextElement(ParseAPI::Function *f)

    Creates a context associated with ``f``.

  .. cpp:function:: ContextElement(ParseAPI::Function *f, long depth)

    Creates a context associated with ``f`` with a caller stack depth of ``depth``.

.. cpp:struct:: Slicer::Location

  **A description of the current location in the slicing search**

  .. cpp:function:: Location(ParseAPI::Function *f, ParseAPI::Block *b)

    Creates a location inside of the block ``b`` contained in the function ``f``.

  .. cpp:member:: ParseAPI::Function *func

    Function that contains :cpp:member:`block`.

  .. cpp:member:: ParseAPI::Block *block

    The current block.

  .. cpp:member:: InsnVec::iterator current
  .. cpp:member:: InsnVec::iterator end

    Current and last instructions in :cpp:member:`block`.

  .. cpp:member:: bool fwd

    Slicing direction. ``true`` indicates forward slicing.

  .. cpp:member:: InsnVec::reverse_iterator rcurrent
  .. cpp:member:: InsnVec::reverse_iterator rend

  Same as :cpp:member:`current` and :cpp:member:`end` but in the reverse direction.

  .. cpp:function:: Address addr() const

    Returns the current address of the instruction being examined.

.. cpp:struct:: Slicer::Element

  **The features of a slice**

  .. cpp:function:: Element(ParseAPI::Block *b, ParseAPI::Function *f, AbsRegion const &r, Assignment::Ptr p)

    Creates an element describing the abstract region ``r`` with a minimal context (the block ``b`` contained in the
    function ``f``). The assignment ``p`` relates to that region (uses or defines it, depending on slice direction).

.. cpp:struct:: SliceFrame

  **State for recursive slicing**

  It is a context/location pair and a list of AbsRegions that are being searched for. SliceFrames keep a list of
  the currently active elements that are at the 'leading edge' of the under-construction slice.

  .. cpp:function:: SliceFrame()

    Creates an empty, but valid, SliceFrame.

  .. cpp:function:: SliceFrame(Location const &l, Context const &c)

    Creates a SliceFrame starting at ``l`` in the context ``c``.

  .. cpp:function:: SliceFrame(bool v)

    Creates an empty SliceFrame with validity ``v``.

  .. cpp:member:: std::map<AbsRegion, std::vector<Element>> active

    Active slice nodes -- describe regions that are currently under scrutiny

  .. cpp:function:: Address addr() const

    Returns the address of the current location.

.. cpp:class:: Slicer::Predicates

  **Stopping criteria of slicing**

  Users can extend this class to control slicing in various situations such as
  performing inter-procedural slicing, searching for controlflow dependencies,
  stopping slicing after discovering certain assignments. A set of callback
  functions are provided to allow dynamic control over the behavior of the
  :cpp:class:`Slicer`.

  .. cpp:function:: Predicates()

    Constructs a default predicate that only searches for intraprocedural dataflow dependencies.

  .. cpp:type:: std::pair<ParseAPI::Function *, int> StackDepth_t

    Stack depth of a function.

  .. cpp:type:: std::stack<StackDepth_t> CallStack_t

    A collection of :cpp:type:`StackDepth_t` representing a complete call stack.

  .. cpp:function:: bool searchForControlFlowDep()

    Checks if this predicate searches for controlflow dependencies.

  .. cpp:function:: void setSearchForControlFlowDep(bool cfd)

    Enables or disables searching for controlflow dependencies.

  .. cpp:function:: virtual bool widenAtPoint(AssignmentPtr)

    The default behavior is to not widen.

  .. cpp:function:: virtual bool endAtPoint(AssignmentPtr)

    Returns ``true`` if searching for this assignment should stop.

    In backward slicing, this function is invoked for every matched assignment.

    The default behavior is to stop.

  .. cpp:function:: virtual bool followCall(ParseAPI::Function * callee, CallStack_t & cs, AbsRegion argument)

    Checks if the slicer will follow the *direct* call to ``callee``.

    The callstack leading to the current callsite is stored in ``cs``. The slice in the callee is carried out
    with respect to the variable in ``argument``.

    .. Note:: Dyninst does not currently try to resolve indirect calls, so the slicer will NOT call this when examining an indirect callsite.

  .. cpp:function:: virtual std::vector<ParseAPI::Function *> followCallBackward(ParseAPI::Block * caller, CallStack_t & cs, AbsRegion argument)

    Returns the callers to follow during slicing starting at the block ``caller``. The callstack leading to the current location
    is provided in ``cs``. The slice with the caller function is carried out with respect to the variable ``argument``.

    This predicate is invoked when the slicer reaches the entry of a
    function in the case of backward slicing or reaches a return instruction
    in the case of forward slicing.

    By default, there are no callers to follow.

  .. cpp:function:: virtual bool addPredecessor(AbsRegion reg)

    Checks if searching for dependencies for the region ``reg`` should continue.

    .. Note:: This callback is invoked for every matching assignment during backward slicing.

    The default is to continue searching.

  .. cpp:function:: virtual bool addNodeCallback(AssignmentPtr assign, std::set<ParseAPI::Edge*> &visited)

    Checks if slicing should continue when a node is added to the slice. The newly-added assignment, ``assign``,
    and the set of visited controlflow edges, ``visited``, are provided.

    .. Note:: This callback is only invoked during backward slicing.

    The default is to continue.

  .. cpp:function:: virtual bool modifyCurrentFrame(SliceFrame &f, GraphPtr g, Slicer *s)

    Allows inspection of the current slice graph, ``p``, being inspected by the slicer ``s`` to determine
    which :ref:`Abstract Locations <sec:dataflow-abstractions>` need further slicing by modifying the current
    SliceFrame, ``f``, after adding a new node and corresponding new edges to the slice.

    The default is to continue slicing.

  .. cpp:function:: virtual bool ignoreEdge(ParseAPI::Edge *e)

    Checks if the edge ``e`` should be ignored during slicing.

    .. Note:: This callback is only invoked during backward slicing.

    The default is to not ignore the edge.
