.. _`sec:BPatch_memoryAccess_NP.h`:

BPatch_memoryAccess_NP.h
########################

.. cpp:class:: BPatch_memoryAccess : public BPatch_instruction

  **A memory access abstraction**

  It contains information that describes the memory access type: read, write,
  read/write, or prefetch. It also contains information that allows the
  effective address and the number of bytes transferred to be determined.

  .. cpp:member:: static BPatch_memoryAccess* const none

  .. cpp:function:: static BPatch_Vector<BPatch_point*>* filterPoints(const BPatch_Vector<BPatch_point*> &points,\
                                                                      unsigned int numMAs)

    Utility function to filter out the points that don't have a 2nd memory   access on x86

  .. cpp:function:: const BPatch_addrSpec_NP *getStartAddr(int which = 0) const
  .. cpp:function:: const BPatch_countSpec_NP *getByteCount(int which = 0) const

  .. cpp:function:: static BPatch_memoryAccess* init_tables()

  .. cpp:function:: virtual ~BPatch_memoryAccess()
  .. cpp:function:: bool equals(const BPatch_memoryAccess* mp) const
  .. cpp:function:: bool equals(const BPatch_memoryAccess& rp) const

  .. cpp:function:: BPatch_addrSpec_NP getStartAddr_NP(int which = 0) const

    Return an address specification that allows the effective address of a
    memory reference to be computed. For example, on the x86 platform a
    memory access instruction operand may contain a base register, an index
    register, a scaling value, and a constant base. The BPatch_addrSpec_NP
    describes each of these values.

  .. cpp:function:: BPatch_countSpec_NP getByteCount_NP(int which = 0) const

    Return a specification that describes the number of bytes transferred by
    the memory access.
