.. _`sec:EventType.h`:

EventType.h
###########

.. cpp:namespace:: Dyninst::ProcControlAPI

.. cpp:class:: EventType

  **A type of event**

  .. cpp:member:: static const int Error               = -1

      Error-handling only. Do not use for callback functions or associate with events.

  .. cpp:member:: static const int Unset               = 0
  .. cpp:member:: static const int Exit                = 1
  .. cpp:member:: static const int Crash               = 2
  .. cpp:member:: static const int Fork                = 3
  .. cpp:member:: static const int Exec                = 4
  .. cpp:member:: static const int UserThreadCreate    = 5
  .. cpp:member:: static const int LWPCreate           = 6
  .. cpp:member:: static const int UserThreadDestroy   = 7
  .. cpp:member:: static const int LWPDestroy          = 8
  .. cpp:member:: static const int Stop                = 9
  .. cpp:member:: static const int Signal              = 10
  .. cpp:member:: static const int LibraryLoad         = 11
  .. cpp:member:: static const int LibraryUnload       = 12
  .. cpp:member:: static const int Bootstrap           = 13
  .. cpp:member:: static const int Breakpoint          = 14
  .. cpp:member:: static const int RPC                 = 15
  .. cpp:member:: static const int SingleStep          = 16
  .. cpp:member:: static const int Library             = 17
  .. cpp:member:: static const int ForceTerminate      = 18
  .. cpp:member:: static const int ControlAuthority    = 20
  .. cpp:member:: static const int AsyncRead           = 21
  .. cpp:member:: static const int AsyncWrite          = 22
  .. cpp:member:: static const int AsyncReadAllRegs    = 23
  .. cpp:member:: static const int AsyncSetAllRegs     = 24
  .. cpp:member:: static const int AsyncFileRead       = 25
  .. cpp:member:: static const int PreSyscall          = 26
  .. cpp:member:: static const int PostSyscall         = 27
  .. cpp:member:: static const int MaxProcCtrlEvent    = 1000

      Users should define their own events value ``MaxProcCtrlEvent`` or higher.

  .. cpp:type:: int Code

      Describes the code value of an EventType.

  .. cpp:function:: EventType()

      Creates a type with code :cpp:member::`Unset` and time :cpp:enumerator:`Time::None`.

  .. cpp:function:: EventType(Code e)

      Creates a type with code ``e`` and time :cpp:enumerator:`Time::Any`.

  .. cpp:function:: EventType(Time t, Code e)

      Creates a type with code ``e`` and time ``t``.

  .. cpp:function:: Code code() const

      Returns the code value of this type.

  .. cpp:function:: Time time() const

      Returns the time value of this type.

  .. cpp:function:: std::string name() const

      Returns string representation of this type.

.. cpp:enum:: EventType::Time

  **The time of an event**

  .. cpp:enumerator:: Pre
  .. cpp:enumerator:: Post
  .. cpp:enumerator:: None
  .. cpp:enumerator:: Any


Notes
=====

Each instance of an :cpp:class:`Event` happening has one associated EventType, and callback functions can
be registered against EventTypes. All EventTypes have an associate code - an integral value that identifies
the EventType. Some EventTypes also have a time associated with them (Pre, Post, or None) - describing
when an Event may occur relative to the Code. For example, an EventType with a code of Exit and a time of Pre
(written as pre-exit) would be associated with an Event that occurs just before a process exits and its
address space is cleaned. An EventType with code Exit and a time of Post would be associated with an Event
that occurs after the process exits and the address space is cleaned.

When using EventTypes to register for callback functions a special time value of Any can be used. This signifies
that the callback function should trigger for both Pre and Post time events. ProcControlAPI will never deliver
an Event that has an EventType with time code Any.
