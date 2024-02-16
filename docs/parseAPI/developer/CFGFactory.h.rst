.. _`sec:CFGFactory.h`:

CFGFactory.h
############

.. cpp:namespace:: Dyninst::ParseAPI

An implementation of CFGFactory is responsible for allocation and
deallocation of CFG objects like Blocks, Edges, and Functions.
Overriding the default methods of this interface allows the parsing
routines to generate and work with extensions of the base types


.. cpp:class:: template <class T> fact_list

  .. cpp:type:: typename LockFreeQueue<T>::iterator iterator
  .. cpp:type:: std::forward_iterator_tag iterator_category
  .. cpp:type:: T elem
  .. cpp:type:: T& reference

  .. cpp:function:: protected void add(elem new_elem)
  .. cpp:function:: protected iterator begin()
  .. cpp:function:: protected iterator end()


.. cpp:enum:: EdgeState

  .. cpp:enumerator:: created
  .. cpp:enumerator:: destroyed_cb
  .. cpp:enumerator:: destroyed_noreturn
  .. cpp:enumerator:: destroyed_all

.. cpp:class:: CFGFactory
 
  These methods are called by ParseAPI, and perform bookkeeping
  around the user-overridden creation/destruction methods.

  .. cpp:function:: virtual Function* mkfunc(Address addr, FuncSource src, std::string name, \
                                              CodeObject* obj, CodeRegion* region, \
                                              Dyninst::InstructionSource* isrc)

      Returns an object derived from Function as though the provided
      parameters had been passed to the Function constructor. The ParseAPI
      parser will never invoke ``mkfunc()`` twice with identical ``addr``, and
      ``region`` parameters—that is, Functions are guaranteed to be unique by
      address within a region.

  .. cpp:function:: virtual Block* mkblock(Function* func, CodeRegion* region, Address addr)

      Returns an object derived from Block as though the provided parameters
      had been passed to the Block constructor. The parser will never invoke
      ``mkblock()`` with identical ``addr`` and ``region`` parameters.

  .. cpp:function:: virtual Edge* mkedge(Block* src, Block* trg, EdgeTypeEnum type)

      Returns an object derived from Edge as though the provided parameters
      had been passed to the Edge constructor. The parser *may* invoke
      ``mkedge()`` multiple times with identical parameters.

  .. cpp:function:: virtual Block* mksink(CodeObject *obj, CodeRegion *r)

      Returns a “sink” block derived from Block to which all unresolvable
      control flow instructions will be linked. Implementors may return a
      unique sink block per CodeObject or a single global sink.

      Implementors of extended CFG classes are required to override the
      default implementations of the ``mk*`` functions to allocate and return
      the appropriate derived types statically cast to the base type.

  .. cpp:function:: virtual void free_func(Function* f)
  .. cpp:function:: virtual void free_block(Block* b)
  .. cpp:function:: virtual void free_edge(Edge* e)
  .. cpp:function:: virtual void free_all()

      CFG objects should be freed using these functions, rather than delete,
      to avoid leaking memory.

  .. cpp:member:: protected fact_list<Edge *> edges_
  .. cpp:member:: protected fact_list<Block *> blocks_
  .. cpp:member:: protected fact_list<Function *> funcs_

      O(1) allocation lists for CFG types. See the CFG.h header file for list
      insertion and removal operations.

      Implementors *may* but are *not required to* override the deallocation
      following deallocation routines. The primary reason to override these
      routines is if additional action or cleanup is necessary upon CFG object
      release; the default routines simply remove the objects from the
      allocation list and invoke their destructors.

  .. cpp:function:: Block *_mkblock(CodeObject *co, CodeRegion *r, Address addr)
  .. cpp:function:: void destroy_func(Function *f)
  .. cpp:function:: void destroy_block(Block *b)
  .. cpp:function:: void destroy_edge(Edge *e, EdgeState reason)
