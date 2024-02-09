.. _`sec:codeRange.h`:

codeRange.h
###########

.. cpp:class:: codeRange : public patchTarget

  .. cpp:function:: virtual void *getPtrToInstruction(Dyninst::Address) const

  .. cpp:function:: virtual void *get_local_ptr() const

    This returns a local pointer to the "beginning" of the code range - as opposed to get_address,
    which returns the "remote" address.

  .. cpp:function:: func_instance *is_function()

    returns NULL if not of type so some people who don't like dynamic_cast don't have to be troubled
    by it's use This is actually a fake we don't have func_instances as code ranges. However, there are
    many times we want to know if we're in a function, and this suffices. We actually do a basic block
    lookup, then transform that into a function.

  .. cpp:function:: block_instance *is_basicBlock()
  .. cpp:function:: block_instance *is_basicBlockInstance()
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
