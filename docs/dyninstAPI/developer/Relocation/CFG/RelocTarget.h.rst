.. _`sec:RelocTarget.h`:

RelocTarget.h
#############

.. cpp:namespace:: Dyninst::Relocation

Wraps an object that can serve as a  control flow target. This
may include existing code objects (a block or function)
or something that has been relocated. We wrap them with this
template class (which will then be specialized as appropriate)
so we don't pollute the base class with extraneous code (I'm
looking at _you_, get_address_cr....

Preliminary requirement: T must be persistent during the existence
of this class so we can use a reference to it.

predictedAddr takes into account things moving during code generation

.. cpp:class:: TargetInt

  .. cpp:function:: TargetInt()
  .. cpp:function:: virtual ~TargetInt()
  .. cpp:function:: virtual std::string format() const
  .. cpp:function:: virtual Address origAddr() const = 0
  .. cpp:function:: virtual bool necessary() const

    It would be nice to eventually move these into the code generator loop,
    but for now it's okay to keep them here.

  .. cpp:function:: virtual void setNecessary(bool a)
  .. cpp:function:: virtual type_t type() const
  .. cpp:function:: virtual bool matches(RelocBlock *) const
  .. cpp:function:: virtual int label(CodeBuffer *) const
  .. cpp:function:: virtual void addTargetEdge(RelocEdge *)
  .. cpp:function:: virtual void addSourceEdge(RelocEdge *)
  .. cpp:function:: virtual void removeTargetEdge(RelocEdge *)
  .. cpp:function:: virtual void removeSourceEdge(RelocEdge *)
  .. cpp:function:: virtual block_instance *block()
  .. cpp:function:: virtual TargetInt *copy() const
  .. cpp:member:: protected bool necessary_


.. cpp:enum:: TargetInt::type_t

  .. cpp:enumerator:: Illegal
  .. cpp:enumerator:: RelocBlockTarget
  .. cpp:enumerator:: BlockTarget
  .. cpp:enumerator:: AddrTarget


.. cpp:class:: template <typename T> Target : public TargetInt

  .. cpp:function:: Target(const T t)
  .. cpp:function:: ~Target()
  .. cpp:function:: const T t()
  .. cpp:member:: private const T &t_


.. cpp:class:: template <> Target<RelocBlock*> : public TargetInt

  .. cpp:function:: Target(RelocBlock * t)
  .. cpp:function:: ~Target()
  .. cpp:function:: RelocBlock * t() const
  .. cpp:function:: Address origAddr() const
  .. cpp:function:: virtual type_t type() const
  .. cpp:function:: virtual string format() const
  .. cpp:function:: virtual bool matches(RelocBlock *t) const
  .. cpp:function:: int label(CodeBuffer *) const
  .. cpp:function:: virtual void addTargetEdge(RelocEdge *e)
  .. cpp:function:: virtual void addSourceEdge(RelocEdge *e)
  .. cpp:function:: virtual void removeTargetEdge(RelocEdge *e)
  .. cpp:function:: virtual void removeSourceEdge(RelocEdge *e)
  .. cpp:function:: virtual block_instance *block()
  .. cpp:function:: virtual TargetInt *copy() const
  .. cpp:member:: private RelocBlock * t_


.. cpp:class::template <> Target<block_instance *> : public TargetInt

  .. cpp:function:: Target(block_instance *t)
  .. cpp:function:: ~Target()
  .. cpp:function:: block_instance *t() const
  .. cpp:function:: virtual type_t type() const
  .. cpp:function:: Address origAddr() const
  .. cpp:function:: virtual string format() const
  .. cpp:function:: int label(CodeBuffer *) const
  .. cpp:function:: virtual block_instance *block()
  .. cpp:function:: virtual TargetInt *copy() const
  .. cpp:member:: private block_instance *t_


.. cpp:class:: template <> Target<Address> : public TargetInt

  .. cpp:function:: Target(Address t)
  .. cpp:function:: ~Target()
  .. cpp:function:: const Address &t() const
  .. cpp:function:: virtual type_t type() const
  .. cpp:function:: Address origAddr() const
  .. cpp:function:: virtual string format() const
  .. cpp:function:: int label(CodeBuffer *) const
  .. cpp:function:: virtual TargetInt *copy() const
  .. cpp:member:: private const Address t_

