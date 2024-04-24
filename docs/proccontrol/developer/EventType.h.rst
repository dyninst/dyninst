.. _`sec-dev:EventType.h`:

EventType.h
===========

.. cpp:namespace:: Dyninst::ProcControlAPI::dev

.. cpp:class:: EventType

  These aren't completely real events.  They can have callbacks registered, but won't be delivered.
  Instead, a real event will be delivered to their callback.  E.g, a callback registered for
  Terminate will actually get Exit or Crash events.

  .. cpp:member:: static const int Terminate            = 400
  .. cpp:member:: static const int ThreadCreate         = 401
  .. cpp:member:: static const int ThreadDestroy        = 402
  .. cpp:member:: static const int AsyncIO              = 403

  Users do not recieve CBs for the below event types

  .. cpp:member:: static const int InternalEvents       = 500
  .. cpp:member:: static const int BreakpointClear      = 500
  .. cpp:member:: static const int BreakpointRestore    = 501
  .. cpp:member:: static const int Async                = 502
  .. cpp:member:: static const int ChangePCStop         = 503

      Used for bug_freebsd_change_pc

  .. cpp:member:: static const int Detach               = 504
  .. cpp:member:: static const int Detached             = 505
  .. cpp:member:: static const int IntBootstrap         = 506
  .. cpp:member:: static const int Nop                  = 507
  .. cpp:member:: static const int ThreadDB             = 508
  .. cpp:member:: static const int RPCLaunch            = 509
  .. cpp:member:: static const int ThreadInfo           = 510
  .. cpp:member:: static const int WinStopThreadDestroy = 511
  .. cpp:member:: static const int PreBootstrap         = 512
  .. cpp:member:: static const int Continue             = 513
  .. cpp:member:: static const int PostponedSyscall     = 514
