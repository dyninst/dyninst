.. _`sec:codeRange.h`:

codeRange.h
###########

.. cpp:class:: codeRange : public patchTarget

  .. cpp:function:: virtual void *getPtrToInstruction(Dyninst::Address) const

  .. cpp:function:: virtual void *get_local_ptr() const

    .. warning:: not implemented

  .. cpp:function:: func_instance *is_function()

    returns NULL if not of type so some people who don't like dynamic_cast don't have to be troubled
    by it's use This is actually a fake we don't have func_instances as code ranges. However, there are
    many times we want to know if we're in a function, and this suffices. We actually do a basic block
    lookup, then transform that into a function.

  .. cpp:function:: block_instance *is_basicBlock()
  .. cpp:function:: block_instance *is_basicBlockInstance()

    This is a special case. The multitramp is the thing in the codeRange tree, but people think of
    baseTramps. So this is dangerous to use, actually.

  .. cpp:function:: image *is_image()
  .. cpp:function:: mapped_object *is_mapped_object()
  .. cpp:function:: parse_func *is_parse_func()
  .. cpp:function:: parse_block *is_parse_block()
  .. cpp:function:: signal_handler_location *is_signal_handler_location()
  .. cpp:function:: inferiorRPCinProgress *is_inferior_rpc()
  .. cpp:function:: void print_range(Dyninst::Address addr = 0)

    Prints codeRange info to stderr.

  .. cpp:function:: codeRange() = default
  .. cpp:function:: codeRange(const codeRange&) = default
  .. cpp:function:: virtual ~codeRange() = default


.. cpp:struct:: entry

  Red/black tree node

  .. cpp:member:: Dyninst::Address key
  .. cpp:member:: codeRange *value
  .. cpp:member:: color_t color

    color of the node

  .. cpp:member:: struct entry* left

    left child

  .. cpp:member:: struct entry* right

    right child

  .. cpp:member:: struct entry* parent

    parent of the node

  .. cpp:function:: entry()

    constructor for structure

  .. cpp:function:: entry(entry* e)

    constructor used for non-nil elements

  .. cpp:function:: entry(Dyninst::Address key_, codeRange* value_, entry* e)

  .. cpp:function:: entry(const entry& e)


.. cpp:class:: codeRangeTree

  .. cpp:member:: private entry* nil

    pointer to define the nil element of the tree NULL is not used since some operations need
    sentinel nil which may have non-nil parent.

  .. cpp:member:: private int setSize

    size of the tree

  .. cpp:member:: private entry* setData

    pointer to the tree structure

  .. cpp:function:: private void leftRotate(entry*)

    Left rotation used by RB tree for balanced tree construction and keeps the RBtree properties.

  .. cpp:function:: private void rightRotate(entry*)

    Right rotattion used by RB tree for balanced tree construction and keeps the RBtree properties.

  .. cpp:function:: private void deleteFixup(entry*)

    Modifies the tree structure after deletion for keeping the RBtree properties.

  .. cpp:function:: private entry* treeInsert(Dyninst::Address, codeRange *)

    insertion to a binary search tree.

    It returns the new element pointer that is inserted. If element is already there it returns NULL

  .. cpp:function:: private entry* treeSuccessor(entry* ) const

    finds the elemnts in the tree that will be replaced with the element being deleted in the deletion.

    That is the element with the largest/smallest value than the element being deleted.

  .. cpp:function:: private entry* find_internal(Dyninst::Address) const

    method that returns the entry pointer for the element that is searchedfor. If the entry is not found then it retuns NULL

  .. cpp:function:: private void traverse(codeRange**, entry*, int&) const

    infix traverse of the RB tree. It traverses the tree in ascending order

  .. cpp:function:: private void traverse(std::vector<codeRange*>& all, entry*) const

    Vector version of above infix traverse of the RB tree. It traverses the tree in ascending order

  .. cpp:function:: private void destroy(entry*)

    deletes the tree structure for deconstructor.

  .. cpp:function:: private codeRangeTree(const codeRangeTree &)

  .. cpp:function:: codeRangeTree()

    constructor. The default comparison structure is used

  .. cpp:function:: ~codeRangeTree()

    destructor which deletes all tree structure and allocated entries

  .. cpp:function:: int size() const

    returns the cardinality of the tree , number of elements

  .. cpp:function:: bool empty() const

    returns true if tree is empty

  .. cpp:function:: void insert(codeRange *r)

    Inserts ``r`` into tree.

  .. cpp:function:: void remove(Dyninst::Address addr)

    Removes the range starting at ``addr``.

  .. cpp:function:: bool find(Dyninst::Address addr, codeRange*& r) const

    Returns in ``r`` the range starting at address ``r``.

    Returns ``true`` if a match was found.

  .. cpp:function:: bool precessor(Dyninst::Address, codeRange *&) const

    Returns the largest value less than or equal to the key given

  .. cpp:function:: bool successor(Dyninst::Address, codeRange *&) const

    Returns the smallest value greater than or equal to the key given

  .. cpp:function:: codeRange** elements(codeRange**) const

    Fill an buffer array with the sorted elements of the codeRangeTree in ascending order according
    to comparison function.

    Returns ``NULL`` if the tree is empty. Otherwise returns the input argument.

  .. cpp:function:: bool elements(std::vector<codeRange *> &) const

    And vector-style

  .. cpp:function:: entry* replicateTree(entry*,entry*,entry*,entry*)

    method that replicates the tree structure of this tree

  .. cpp:function:: void clear()

    Remove all entries in the tree


.. cpp:enum:: codeRangeTree::color_t

  .. cpp:enumerator:: TREE_BLACK
  .. cpp:enumerator:: TREE_RED
