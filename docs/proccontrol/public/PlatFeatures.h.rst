.. _`sec:PlatFeatures.h`:

PlatFeatures.h
==============

.. cpp:namespace:: Dyninst::ProcControlAPI

The classes described in this section are all used to configure
platform-specific features for :cpp:class:`Process` objects. The three tracking
classes (LibraryTracking, ThreadTracking, LWPTracking) all contain
member functions to set either interrupt-driven or polling-driven
handling for different events associated with ``Process`` objects. When
interrupt-driven handling is enabled, the associated process may be
modified to accommodate timely handling (e.g., inserting breakpoints).
When polling-driven handling is enabled, the associated process is not
modified and events are handled on demand by calling the appropriate
“refresh” member function. All of these classes are defined in
PlatFeatures.h.

.. cpp:class:: LibraryTracking

  The LibraryTracking class is used to configure the handling of library
  events for its associated Process.

  LibraryTracking Static Member Functions:

  .. cpp:function:: static void setDefaultTrackLibraries(bool b)

    Sets the default handling mechanism for library events across all
    Process objects to interrupt-driven (b = ``true``) or polling-driven (b =
    ``false``).

  .. cpp:function:: static bool getDefaultTrackLibraries()

    Returns the current default handling mechanism for library events across all :cpp:class:`Process` objects.

    Returns ``true`` if interrupt-driven, ``false`` if polling-driven.

  .. cpp:function:: bool setTrackLibraries(bool b) const

    Sets the library event handling mechanism for the associated :cpp:class:`Process`
    object to interrupt-driven (b = ``true``) or polling-driven (b = ``false``).

    Returns ``true`` on success.

  .. cpp:function:: bool getTrackLibraries() const

    Returns the current library event handling mechanism for the associated
    :cpp:class:`Process` object.

    Returns ``true`` if interrupt-driven, ``false`` if polling-driven.

  .. cpp:function:: bool refreshLibraries()

    Manually polls for queued library events to handle.

    Returns ``true`` on success.

.. cpp:class:: ThreadTracking

  The ThreadTracking class is used to configure the handling of thread
  events for its associated :cpp:class:`Process`.

  .. cpp:function:: static void setDefaultTrackThreads(bool b)

    Sets the default handling mechanism for thread events across all :cpp:class:`Process`
    objects to interrupt-driven (b = ``true``) or polling-driven (b = ``false``).

  .. cpp:function:: static bool getDefaultTrackThreads()

    Returns the current default handling mechanism for thread events across
    all :cpp:class:`Process` objects.

    Returns ``true`` if interrupt-driven, ``false`` if polling-driven.

  .. cpp:function:: bool setTrackThreads(bool b) const

    Sets the thread event handling mechanism for the associated :cpp:class:`Process`
    object to interrupt-driven (b = ``true``) or polling-driven (b = ``false``).

    Returns ``true`` on success.

  .. cpp:function:: bool getTrackThreads() const

    Returns the current thread event handling mechanism for the associated :cpp:class:`Process`.

    Returns ``true`` if interrupt-driven, ``false`` if polling-driven.

  .. cpp:function:: bool refreshThreads()

    Manually polls for queued thread events to handle.

    Returns ``true`` on success.

.. cpp:class:: LWPTracking

  The LWPTracking class is used to configure the handling of LWP events
  for its associated :cpp:class:`Process`.

  .. cpp:function:: static void setDefaultTrackLWPs(bool b)

    Sets the default handling mechanism for LWP events across all :cpp:class:`Process`
    objects to interrupt-driven (b = ``true``) or polling-driven (b = ``false``).

  .. cpp:function:: static bool getDefaultTrackLWPs()

    Returns the current default handling mechanism for LWP events across all :cpp:class:`Process` objects.

    Returns ``true`` if interrupt-driven, ``false`` if polling-driven.

  .. cpp:function:: bool setTrackLWPs(bool b) const

    Sets the LWP event handling mechanism for the associated :cpp:class:`Process` object
    to interrupt-driven (b = ``true``) or polling-driven (b = ``false``).

    Returns ``true`` on success.

  .. cpp:function:: bool getTrackLWPs() const

    Returns the current LWP event handling mechanism for the associated :cpp:class:`Process` object.

    Returns ``true`` if interrupt-driven, ``false`` if polling-driven.

  .. cpp:function:: bool refreshLWPs()

    Manually polls for queued LWP events to handle.

    Returns ``true`` on success.

.. cpp:class:: FollowFork

  The FollowFork class is used to configure ProcControlAPI’s behavior when
  the associated :cpp:class:`Process` forks.

  .. cpp:enum:: follow_t

    .. cpp:enumerator:: follow_t::None
    
      Fork tracking is not available for the current platform.
      
    .. cpp:enumerator:: follow_t::ImmediateDetach
    
      Forked children are never attached to.
      
    .. cpp:enumerator:: follow_t::DisableBreakpointsDetach
    
      Inherited breakpoints are removed from forked children, and then the children are detached.
    
    .. cpp:enumerator:: follow_t::Follow
    
      Forked children are attached to and remain under full control of ProcControlAPI. This is the default behavior.

  .. cpp:function:: static void setDefaultFollowFork(follow_t f)

    Sets the default forking behavior across all :cpp:class:`Process` objects.

  .. cpp:function:: static follow_t getDefaultFollowFork()

    Returns the default forking behavior across all :cpp:class:`Process` objects.

  .. cpp:function:: bool setFollowFork(follow_t f) const

    Sets the forking behavior for the associated :cpp:class:`Process` object.

    Returns ``true`` on success.

  .. cpp:function:: follow_t getFollowFork() const

    This function returns the current forking behavior for the associated :cpp:class:`Process`.

.. cpp:class:: SignalMask

  The SignalMask class is used to configure the signal mask for its
  associated :cpp:class:`Process`.

  .. cpp:class:: dyn_sigset_t

    On POSIX systems, this type is equivalent to `sigset_t <https://www.man7.org/linux/man-pages/man0/signal.h.0p.html>`_.

  .. cpp:function:: static void setDefaultSigMask(dyn_sigset_t s)

    This function sets the default signal mask across all :cpp:class:`Process`.

  .. cpp:function:: static dyn_sigset_t getDefaultSigMask()

    Returns the current default signal mask.

  .. cpp:function:: bool setSigMask(dyn_sigset_t s)

    This function sets the signal mask for the associated :cpp:class:`Process`.

    Returns ``true`` on success.

  .. cpp:function:: dyn_sigset_t getSigMask() const

    This function returns the current signal mask for the associated :cpp:class:`Process`.
