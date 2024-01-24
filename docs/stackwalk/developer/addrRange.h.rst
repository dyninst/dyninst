.. _`sec:addrRange.h`:

addrRange.h
###########

.. cpp:enum:: color_t

  .. cpp:enumerator:: TREE_RED
  .. cpp:enumerator:: TREE_BLACK

.. cpp:class:: addrRange

  .. cpp:function:: virtual Dyninst::Address get_address() const = 0
  .. cpp:function:: virtual unsigned long get_size() const = 0
  .. cpp:function:: virtual std::string get_name() const
  .. cpp:function:: virtual ~addrRange()

.. cpp:class:: template <class T> addrRangeTree

  A red/black tree

  .. note:: ``T`` should be-an :cpp:class:`addrRange`

  .. cpp:member:: entry* nil

      Pointer to define the nil element of the tree.

      ``NULL`` is not used since some operations need sentinel nil which may have non-nil parent.

  .. cpp:member:: int setSize

      Returns the cardinality of the tree , number of elements.

  .. cpp:member:: entry* setData

      Pointer to the tree structure

  .. cpp:function:: void leftRotate(entry* pivot)

      Implements left rotation used by RB tree for balanced tree construction and keeps the RBtree properties.

  .. cpp:function:: void rightRotate(entry *pivot)

      Implements right rotation used by RB tree for balanced tree construction and keeps the RBtree properties.

  .. cpp:function:: void deleteFixup(entry *x)

      Modifies the tree structure after deletion for keeping the RBtree properties.

  .. cpp:function:: entry* treeInsert(Dyninst::Address key, T *value)

      Insertion into a binary search tree.

      Returns the new element pointer that is inserted. If element is already there, returns ``NULL``.

  .. cpp:function:: entry* treeSuccessor(entry *x) const

      Finds the elements in the tree that will be replaced with the element
      being deleted in the  deletion. That is the element with the largest
      smallest value than the element being deleted.

  .. cpp:function:: entry* find_internal(Dyninst::Address element) const

      Returns the entry pointer for the element that is searched for.

      Returns ``NULL`` if no entry is found.

  .. cpp:function:: void traverse(T **all, entry *node, int &n) const

      Infix traverse of the RB tree. It traverses the tree in ascending order

  .. cpp:function:: void traverse(std::vector<T *> &all, entry*node) const

      Infix traverse of the RB tree. It traverses the tree in ascending order

  .. cpp:function:: void destroy(entry *node)

      Deletes the tree structure for deconstructor.

  .. cpp:function:: addrRangeTree(const addrRangeTree& y)

      Intentionally incomplete.

  .. cpp:function:: bool precessor_internal(Dyninst::Address key, entry * &value) const

      Similar to precessor, but returns an entry

  .. cpp:function:: bool successor_internal(Dyninst::Address key, entry * &value) const

      Similar to successor, but returns an entry

  .. cpp:function:: addrRangeTree()
  .. cpp:function:: virtual ~addrRangeTree()
  .. cpp:function:: int size() const
  .. cpp:function:: bool empty() const

      Checks if tree is empty

  .. cpp:function:: void insert(T *value)

      Inserts the element ``value`` in the tree.

  .. cpp:function:: void remove(Dyninst::Address key)

      Removes ``key`` from the tree.

  .. cpp:function:: virtual bool find(Dyninst::Address key, T *& value) const

      Checks if ``key`` is in the tree.

  .. cpp:function:: virtual bool find(Dyninst::Address start, Dyninst::Address end, std::vector<T *> &ranges) const

      Fills in the vector with all address ranges that overlap with the address range defined by (start, end].

  .. cpp:function:: virtual bool precessor(Dyninst::Address key, T *& value) const

      Returns the largest value less than or equal to the key given.

  .. cpp:function:: virtual bool successor(Dyninst::Address key, T *& value) const

      Returns the smallest value greater than or equal to the key given.

  .. cpp:function:: T ** elements(T ** buffer) const

      Fill a buffer array with the sorted elements of the addrRangeTree in ascending order according to comparison function
      if the addrRangeTree is empty it retuns NULL, other wise it returns the input argument.

  .. cpp:function:: bool elements(std::vector<T *> &buffer) const

      Fill a buffer array with the sorted elements of the addrRangeTree in ascending order according to comparison function
      if the addrRangeTree is empty it retuns NULL, other wise it returns the input argument.

  .. cpp:function:: void clear()

      Remove all entries in the tree


.. cpp:struct:: addrRangeTree::entry

  tree implementation structure. Used to implement the RB tree

  .. cpp:member:: Dyninst::Address key
  .. cpp:member:: T *value

  .. cpp:member:: color_t color

      color of the node

  .. cpp:member:: struct entry* left

      left child

  .. cpp:member:: struct entry* right

      right child

  .. cpp:member:: struct entry* parent

      parent of the node

  .. cpp:function:: entry()

  .. cpp:function:: entry(entry* e)

      constructor used for non-nil elements

  .. cpp:function:: entry(Dyninst::Address key_, T *value_, entry* e)

  .. cpp:function:: entry(const entry& e)
