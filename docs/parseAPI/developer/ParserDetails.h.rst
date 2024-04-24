.. _`sec:ParserDetails.h`:

ParserDetails.h
###############

.. cpp:namespace:: Dyninst::ParseAPI

.. cpp:namespace-push:: @parser_details_anon

.. cpp:function:: static inline bool is_code(Function* f, Address addr)

  Queries CodeSource objects with disjoint regions involve expensive range lookups.
  Because it's most often called on addresses within the current function's region, this code
  short circuits the expensive case.

  NB for overlapping region CodeSources, the two cases in this function are identical.
  We'll pay extra in this (uncommon) case.

.. cpp:namespace-pop::

.. cpp:class:: ParseWorkElem

  .. cpp:function:: ParseWorkElem(ParseWorkBundle *b, parse_work_order o, Edge *e, Address source, Address target, bool resolvable, bool tailcall)
  .. cpp:function:: ParseWorkElem(ParseWorkBundle *b, Edge *e, Address source, Address target, bool resolvable, bool tailcall)

    We also the source address of the edge because the source block may be split

  .. cpp:function:: ParseWorkElem()

  .. cpp:function:: ParseWorkElem(ParseWorkBundle *bundle, Block *b, const InsnAdapter::IA_IAPI* ah)

      This work element is a continuation of parsing jump tables

  .. cpp:function:: ParseWorkElem(ParseWorkBundle *bundle, Function *f)
  .. cpp:function:: ParseWorkBundle* bundle() const
  .. cpp:function:: Edge* edge() const
  .. cpp:function:: Addresssource() const
  .. cpp:function:: Addresstarget() const
  .. cpp:function:: boolresolvable() const
  .. cpp:function:: parse_work_order order() const
  .. cpp:function:: voidsetTarget(Address t)
  .. cpp:function:: booltailcall() const
  .. cpp:function:: boolcallproc() const
  .. cpp:function:: voidmark_call()
  .. cpp:function:: Block* cur() const
  .. cpp:function:: InsnAdapter::IA_IAPI* ah() const
  .. cpp:function:: Function* shared_func() const
  .. cpp:member:: private ParseWorkBundle * _bundle
  .. cpp:member:: private Edge * _edge
  .. cpp:member:: private Address _src
  .. cpp:member:: private Address _targ
  .. cpp:member:: private bool _can_resolve
  .. cpp:member:: private bool _tailcall
  .. cpp:member:: private parse_work_order _order
  .. cpp:member:: private bool _call_processed
  .. cpp:member:: private Block* _cur

    Data for continuing parsing jump tables

  .. cpp:member:: private InsnAdapter::IA_IAPI* _ah
  .. cpp:member:: private Function * _shared_func


.. cpp:class:: ParseWorkElem::compare

  .. cpp:function:: bool operator()(const ParseWorkElem * e1, const ParseWorkElem * e2) const

      Note that compare treats the parse_work_order as lowest is highest priority.
      Sorts by parse_work_order, then bundle, then address.

.. cpp:class:: ParseWorkBundle

  **ParseWorkElem container**

  .. cpp:function:: ParseWorkBundle()
  .. cpp:function:: ParseWorkElem* add(ParseWorkElem* e)
  .. cpp:function:: vector<ParseWorkElem*> const& elems()

.. cpp:enum:: ParseWorkElem::parse_work_order

  .. cpp:enumerator:: seed_addr
  .. cpp:enumerator:: ret_fallthrough

    conditional returns

  .. cpp:enumerator:: call
  .. cpp:enumerator:: call_fallthrough
  .. cpp:enumerator:: cond_not_taken
  .. cpp:enumerator:: cond_taken
  .. cpp:enumerator:: br_direct
  .. cpp:enumerator:: br_indirect
  .. cpp:enumerator:: catch_block
  .. cpp:enumerator:: checked_call_ft
  .. cpp:enumerator:: resolve_jump_table

    We want to finish all possible parsing work before parsing jump tables.

  .. cpp:enumerator:: func_shared_code

    For shared code we only parse once. The return statuses of the functions
    that share code depend on the function that performs the real parsing.

  .. cpp:enumerator:: __parse_work_end__


Notes
=====

.. Attention:: The order of elements in :cpp:enum:`ParseWorkElem::parse_work_order` is critical to parsing order.

The earier an element appear in the enum, the sooner the corresponding edges are going to be parsed.

1. Our current implementation of non-returning function analysis  WORK IFF call is prioritized over call_fallthrough.

2.  We have a tail call heuristics that a jump to its own block is not a tail call.
    For this heuristics to be more effective, we want to traverse
    certain intraprocedural edges such as call_fallthrough and cond_not_taken
    over potential tail call edges such as cond_taken, br_direct, and br_indirect.

3. Jump table analysis would like to have as much intraprocedural control flow
   as possible to resolve an indirect jump. So resolve_jump_table is delayed.

4. Parsing cond_not_taken edges over cond_taken edges. This is because cond_taken
   edges may split a block. In special cases, the source block of the edge is split.
   The cond_not_taken edge work element would still have the unsplit block, which is
   now the upper portion after splitting.
