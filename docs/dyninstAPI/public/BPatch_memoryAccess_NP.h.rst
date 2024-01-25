.. _`sec:BPatch_memoryAccess_NP.h`:

BPatch_memoryAccess_NP.h
########################


Instrumentation points created through findPoint(const
std::set<BPatch_opCode>& ops) get memory access information attached to
them. This information is used by the memory access snippets, but is
also available to the API user. The classes that encapsulate memory
access information are contained in the BPatch_memoryAccess_NP.h header.

.. cpp:class:: BPatch_memoryAccess
   
  This class encapsulates a memory access abstraction. It contains
  information that describes the memory access type: read, write,
  read/write, or prefetch. It also contains information that allows the
  effective address and the number of bytes transferred to be determined.

  .. cpp:function:: bool isALoad()

    Return true if the memory access is a load (memory is read into a
    register).

  .. cpp:function:: bool isAStore()

    Return true if the memory access is write. Some machine instructions may
    both load and store.

  .. cpp:function:: bool isAPrefetch_NP()

    Return true if memory access is a prefetch (i.e, it has no observable
    effect on user registers). It this returns true, the instruction is
    considered neither load nor store. Prefetches are detected only on IA32.

  .. cpp:function:: short prefetchType_NP()

    If the memory access is a prefetch, this method returns a platform
    specific prefetch type.

  .. cpp:function:: BPatch_addrSpec_NP getStartAddr_NP()

    Return an address specification that allows the effective address of a
    memory reference to be computed. For example, on the x86 platform a
    memory access instruction operand may contain a base register, an index
    register, a scaling value, and a constant base. The BPatch_addrSpec_NP
    describes each of these values.

  .. cpp:function:: BPatch_countSpec_NP getByteCount_NP()

    Return a specification that describes the number of bytes transferred by
    the memory access.


.. cpp:class:: BPatch_addrSpec_NP
   
  This class encapsulates the information required to determine an
  effective address at runtime. The general representation for an address
  is a sum of two registers and a constant; this may change in future
  releases. Some architectures use only certain bits of a register (e.g.
  bits 25:31 of XER register on the Power chip family); these are
  represented as pseudo-registers. The numbering scheme for registers and
  pseudo-registers is implementation dependent and should not be relied
  upon; it may change in future releases.

  .. cpp:function:: int getImm()

    Return the constant offset. This may be positive or negative.

  .. cpp:function:: int getReg(unsigned i)

    Return the register number for the i\ :sup:`th` register in the sum,
    where 0 ≤ i ≤ 2. Register numbers are positive; a value of -1 means no
    register.

  .. cpp:function:: int getScale()

    Returns any scaling factor used in the memory address computation.


.. cpp:class:: BPatch_countSpec_NP
   
  This class encapsulates the information required to determine the number
  of bytes transferred by a memory access.
