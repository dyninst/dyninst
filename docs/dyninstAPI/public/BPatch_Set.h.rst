.. _`sec:BPatch_Set.h`:

BPatch_Set.h
############

.. cpp:class:: BPatch_Set
   
  **BPatch_Set** is another container class, similar to the set class in
  the STL. THIS CLASS HAS BEEN DEPRECATED AND WILL BE REMOVED IN THE NEXT
  RELEASE. In addition the methods provided by std::set, it provides the
  following compatibility methods:

  .. cpp:function:: BPatch_Set::BPatch_Set()

    A constructor that creates an empty set with the default comparison
    function.

  .. cpp:function:: BPatch_Set::BPatch_Set(const BPatch_Set<T,Compare>& newBPatch_Set)

    Copy constructor.

  .. cpp:function:: void remove(const T&)

    Remove the given element from the set.

  .. cpp:function:: bool contains(const T&)

    Return true if the argument is a member of the set, otherwise returns
    false.

  .. cpp:function:: T* elements(T*)

  .. cpp:function:: void elements(std::vector<T> &)

    Fill an array (or vector) with a list of the elements in the set that
    are sorted in ascending order according to the comparison function. The
    input argument should point to an array large enough to hold the
    elements. This function returns its input argument, unless the set is
    empty, in which case it returns NULL.

  .. cpp:function:: T minimum()

    Return the minimum element in the set, as determined by the comparison
    function. For an empty set, the result is undefined.

  .. cpp:function:: T maximum()

    Return the maximum element in the set, as determined by the comparison
    function. For an empty set, the result is undefined.

  .. cpp:function:: BPatch_Set<T,Compare>& operator+= (const T&)

    Add the given object to the set.

  .. cpp:function:: BPatch_Set<T,Compare>& operator|= (const BPatch_Set<T,Compare>&)

    Set union operator. Assign the result of the union to the set on the
    left hand side.

  .. cpp:function:: BPatch_Set<T,Compare>& operator&= (const BPatch_Set<T,Compare>&)

    Set intersection operator. Assign the result of the intersection to the
    set on the left hand side.

  .. cpp:function:: BPatch_Set<T,Compare>& operator-= (const BPatch_Set<T,Compare>&)

    Set difference operator. Assign the difference of the sets to the set on
    the left hand side.

  .. cpp:function:: BPatch_Set<T,Compare> operator| (const BPatch_Set<T,Compare>&)

    Set union operator.

  .. cpp:function:: BPatch_Set<T,Compare> operator& (const BPatch_Set<T,Compare>&)

    Set intersection operator.

  .. cpp:function:: BPatch_Set<T,Compare> operator- (const BPatch_Set<T,Compare>&)

    Set difference operator.
