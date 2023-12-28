.. _`sec:MachSyscall.h`:

MachSyscall.h
#############

.. cpp:namespace:: Dyninst

.. cpp:class:: Platform
 
  .. cpp:function:: Platform(Architecture a, OSType o)
  .. cpp:function:: Architecture arch() const
  .. cpp:function:: OSType os() const

.. cpp:class:: MachSyscall 

  .. cpp:type:: unsigned long SyscallIDPlatform
  .. cpp:type:: unsigned long SyscallIDIndependent
  .. cpp:type:: const char * SyscallName

  .. cpp:function:: static MachSyscall makeFromPlatform(Platform, SyscallIDIndependent)

    Constructs a MachSyscall from a Platform and a platform-independent ID (e.g. ``dyn_getpid``).
    The platform-independent syscall IDs may be found in :ref:`sec-dev:dyn_syscalls.h`.

  .. cpp:function:: SyscallIDPlatform num() const

    The platform-specific number for this system call

  .. cpp:function:: SyscallName name() const
