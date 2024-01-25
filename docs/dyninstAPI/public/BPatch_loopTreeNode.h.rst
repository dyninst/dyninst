.. _`sec:BPatch_loopTreeNode.h`:

BPatch_loopTreeNode.h
#####################

.. cpp:class:: BPatch_loopTreeNode
   
  The **BPatch_loopTreeNode** class provides a tree interface to a
  collection of instances of class BPatch_basicBlockLoop contained in a
  BPatch_flowGraph. The structure of the tree follows the nesting
  relationship of the loops in a function’s flow graph. Each
  BPatch_­loopTreeNode contains a pointer to a loop (represented by
  BPatch_basicBlockLoop), and a set of sub-loops (represented by other
  BPatch_loopTreeNode objects). The root BPatch_­loopTreeNode instance has
  a null loop member since a function may contain multiple outer loops.
  The outer loops are contained in the root instance’s vector of children.

  Each instance of BPatch_loopTreeNode is given a name that indicates its
  position in the hierarchy of loops. The name of each root loop takes the
  form of loop_x, where x is an integer from 1 to n, where n is the number
  of outer loops in the function. Each sub-loop has the name of its
  parent, followed by a .y, where y is 1 to m, where m is the number of
  sub-loops under the outer loop. For example, consider the following C
  function:

  .. code-block:: cpp

    void foo() {
       int x, y, z, i;

       for (x=0; x<10; x++) {
          for (y = 0; y<10; y++)
          ...
          for (z = 0; z<10; z++)
          ...
       }
       for (i = 0; i<10; i++) {
       ...
       }
    }

  The foo function will have a root BPatch_loopTreeNode, containing a NULL
  loop entry and two BPatch_loopTreeNode children representing the
  functions outer loops. These children would have names loop_1 and
  loop_2, respectively representing the x and i loops. loop_2 has no
  children. loop_1 has two child BPatch_loopTreeNode objects, named
  loop_1.1 and loop_1.2, respectively representing the y and z loops.

  .. cpp:var:: BPatch_basicBlockLoop *loop

  A node in the tree that represents a single BPatch_basicBlockLoop
  instance.

  .. cpp:var:: std::vector<BPatch_loopTreeNode *> children

  The tree nodes for the loops nested under this loop.

  .. cpp:function:: const char *name()

    Return a name for this loop that indicates its position in the hierarchy
    of loops.

  .. cpp:function:: bool getCallees(std::vector<BPatch_function *> &v, BPatch_addressSpace*p)

    This function fills the vector v with the list of functions that are
    called by this loop.

  .. cpp:function:: const char *getCalleeName(unsigned int i)

    This function return the name of the i\ :sup:`th` function called in the
    loop’s body.

  .. cpp:function:: unsigned int numCallees()

    Returns the number of callees contained in this loop’s body.

  .. cpp:function:: BPatch_basicBlockLoop *findLoop(const char *name)

    Finds the loop object for the given canonical loop name.
