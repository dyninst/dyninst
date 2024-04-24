.. _`sec:Snippet.h`:

Snippet.h
#########

.. cpp:namespace:: Dyninst::PatchAPI

.. cpp:class:: Snippet

  **Custom snippet representation and corresponding mini-compiler for translation into binary code**

  .. cpp:type:: boost::shared_ptr<Snippet> Ptr

  .. cpp:function:: static Ptr create(Snippet* a)

      Creates an object of the Snippet.

  .. cpp:function:: virtual bool generate(Point* p, Buffer& b) = 0

      Generates binary code from this snippet at point ``p``, and writes it into ``b``.

      Returns false if code generation fails.
