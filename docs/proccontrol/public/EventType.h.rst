.. _`sec:EventType.h`:

EventType.h
===========

.. cpp:namespace:: Dyninst::ProcControlAPI

.. cpp:class:: EventType

  The EventType class represents a type of event. Each instance of an
  Event happening has one associated EventType, and callback functions can
  be registered against EventTypes. All EventTypes have an associate
  code—an integral value that identifies the EventType. Some EventTypes
  also have a time associated with them (Pre, Post, or None)—describing
  when an Event may occur relative to the Code. For example, an EventType
  with a code of Exit and a time of Pre (written as pre-exit) would be
  associated with an Event that occurs just before a process exits and its
  address space is cleaned. An EventType with code Exit and a time of Post
  would be associated with an Event that occurs after the process exits
  and the address space is cleaned.

  When using EventTypes to register for callback functions a special time
  value of Any can be used. This signifies that the callback function
  should trigger for both Pre and Post time events. ProcControlAPI will
  never deliver an Event that has an EventType with time code Any.

  .. cpp:enum:: Time

     .. cpp:enumerator:: Time::Pre=0
     .. cpp:enumerator:: Time::Post
     .. cpp:enumerator:: Time::None
     .. cpp:enumerator:: Time::Any

  .. cpp:type:: int Code

    The Time and Code types are respectively used to describe the time and
    code values of an EventType.

    These constants describe possible values for an EventType’s code. The
    Error and Unset codes are for handling error cases and should not be
    used for callback functions or be associated with Events.

    The EventType codes were implemented as an integer (rather than an enum)
    to allow users to create custom EventTypes. Any custom EventType should
    begin at the MaxProcCtrlEvent value, all smaller values are reserved by
    ProcControlAPI.

    This type defines a less-than comparison function for EventTypes. While
    a comparison of EventTypes does not have a semantic meaning, this can be
    useful for inserting EventTypes into maps or other STL data structures.

  .. cpp:function:: EventType(Code e)

    Constructs an EventType with the given code and a time of Any.

  .. cpp:function:: EventType(Time t, Code e)

    Constructs an EventType with the given time and code values.

  .. cpp:function:: EventType()

    Constructs an EventType with an Unset code and None time value.

  .. cpp:function:: Code code() const

    Returns the code value of the EventType.

  .. cpp:function:: Time time() const

    Returns the time value of the EventType.

  .. cpp:function:: std::string name() const

    Returns a human readable name for this EventType.

.. _eventtype-values:

.. csv-table:: EventType values
   :header: "Name", "Value", "", "Name", "Value"
   :widths: 25 4 10 25 4

    "AsyncFileRead","25","","AsyncIO","403"
    "AsyncRead","21","","AsyncReadAllRegs","23"
    "AsyncSetAllRegs","24","","AsyncWrite","22"
    "Bootstrap","13","","Breakpoint","14"
    "ControlAuthority","20","","Crash","2"
    "Error","-1","","Exec","4"
    "Exit","1","","ForceTerminate","18"
    "Fork","3","","LWPCreate","6"
    "LWPDestroy","8","","Library","17"
    "LibraryLoad","11","","LibraryUnload","12"
    "PostSyscall","27","","PreSyscall","26"
    "RPC","15","","Signal","10"
    "SingleStep","16","","Stop","9"
    "Terminate","400","","ThreadCreate","401"
    "ThreadDestroy","402","","Unset","0"
    "UserThreadCreate","5","","UserThreadDestroy","7"

Users should define their own events value ``MaxProcCtrlEvent`` or higher.
