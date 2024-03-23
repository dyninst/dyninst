.. _`sec:ParseContainers.h`:

ParseContainers.h
#################

An iterator and a predicate interface, and a ContainerWrapper that can provide a forward predicate
iterator for any container that exports a ``begin()`` and ``end()``.

.. cpp:namespace:: Dyninst::ParseAPI

**A predicate interface**

.. cpp:class:: template <typename VALUE, typename REFERENCE = VALUE&> iterator_predicate

  .. cpp:function:: bool operator()(const REFERENCE o) const
  .. cpp:function:: virtual bool pred_impl(const REFERENCE) const


.. cpp:struct:: template<typename ARG> true_predicate : iterator_predicate<ARG>
  
  **Container wrapper and iterators for predicate containers**

  .. cpp:function:: bool operator()(ARG)
