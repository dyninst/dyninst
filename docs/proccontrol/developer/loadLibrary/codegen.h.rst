.. _`sec:codegen.h`:

codegen.h
#########

.. cpp:namespace:: Dyninst::ProcControlAPI

.. cpp:class:: Codegen

  .. cpp:function:: Codegen(ProcControlAPI::Process *proc, std::string libname)
  .. cpp:function:: bool generate()
  .. cpp:function:: const Buffer &buffer()
  .. cpp:function:: unsigned startOffset() const

  .. cpp:function:: private Address buildLinuxArgStruct(Address libbase, unsigned mode)

      Only available when ``os_linux`` is defined.

  .. cpp:function:: private bool generateStackUnprotect(Address var, Address mprotect)

      Only available when ``os_linux`` is defined.

  .. cpp:member:: private std::map<Address, Address> toc_

      PPC64 only, but it's handy to stash it here
