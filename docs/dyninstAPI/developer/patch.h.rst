.. _`sec:patch.h`:

patch.h
#######

.. cpp:class:: patchTarget

  .. cpp:function:: virtual Dyninst::Address get_address() const = 0
  .. cpp:function:: virtual unsigned get_size() const = 0
  .. cpp:function:: virtual std::string get_name() const
  .. cpp:function:: patchTarget() = default
  .. cpp:function:: patchTarget(const patchTarget&) = default
  .. cpp:function:: virtual ~patchTarget() = default

.. cpp:class:: toAddressPatch : public patchTarget

  .. cpp:member:: private Dyninst::Address addr
  .. cpp:function:: toAddressPatch(Dyninst::Address a)
  .. cpp:function:: virtual ~toAddressPatch()
  .. cpp:function:: virtual Dyninst::Address get_address() const
  .. cpp:function:: virtual unsigned get_size() const
  .. cpp:function:: void set_address(Dyninst::Address a)

.. cpp:class:: relocPatch

  .. cpp:function:: relocPatch(unsigned d, patchTarget *s, relocPatch::patch_type_t ptype, codeGen *gen, Dyninst::Offset off, unsigned size)
  .. cpp:function:: void applyPatch()
  .. cpp:function:: bool isApplied()
  .. cpp:function:: void setTarget(patchTarget *s)
  .. cpp:member:: private unsigned dest_
  .. cpp:member:: private patchTarget *source_
  .. cpp:member:: private unsigned size_
  .. cpp:member:: private patch_type_t ptype_
  .. cpp:member:: private codeGen *gen_
  .. cpp:member:: private Dyninst::Offset offset_
  .. cpp:member:: private bool applied_

.. cpp:class:: ifTargetPatch : public patchTarget

  .. cpp:member:: private signed int targetOffset
  .. cpp:function:: ifTargetPatch(signed int o)
  .. cpp:function:: virtual Dyninst::Address get_address() const
  .. cpp:function:: virtual unsigned get_size() const
  .. cpp:function:: virtual std::string get_name() const
  .. cpp:function:: virtual ~ifTargetPatch()


.. cpp:enum:: relocPatch::patch_type_t

  .. cpp:enumerator:: abs

    Patch the absolute address of the source into dest

  .. cpp:enumerator:: pcrel

    Patch a PC relative address from codeGen start + offset

  .. cpp:enumerator:: abs_lo

    Patch lower half of source's bytes into dest

  .. cpp:enumerator:: abs_hi

    Patch upper half of source's bytes into dest

  .. cpp:enumerator:: abs_quad1

    Patch the first quarter of source's bytes into dest

  .. cpp:enumerator:: abs_quad2

    Patch the second quarter of source's bytes into dest

  .. cpp:enumerator:: abs_quad3

    Patch the third quarter of source's bytes into dest

  .. cpp:enumerator:: abs_quad4

    Patch the forth quarter of source's bytes into dest


.. code:: cpp

  #define SIZE_16BIT 2
  #define SIZE_32BIT 4
  #define SIZE_64BIT 8

