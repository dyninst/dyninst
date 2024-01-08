.. _`sec:PlatFeatures.h`:

PlatFeatures.h
##############

.. cpp:namespace:: Dyninst::ProcControlAPI

The classes described in this section are all used to configure
platform-specific features for :cpp:class:`Process` objects.
:cpp:class:`LibraryTracking`, :cpp:class:`ThreadTracking`,
:cpp:class:`LWPTracking` all contain
member functions to set either interrupt-driven or polling-driven
handling for different events associated with ``Process`` objects. When
interrupt-driven handling is enabled, the associated process may be
modified to accommodate timely handling (e.g., inserting breakpoints).
When polling-driven handling is enabled, the associated process is not
modified and events are handled on demand by calling the appropriate
“refresh” member function.


.. cpp:class:: LibraryTracking

  The LibraryTracking class is used to configure the handling of library
  events for its associated Process.

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

  .. cpp:function:: protected LibraryTracking(Process::ptr proc_)
  .. cpp:function:: protected ~LibraryTracking()
  .. cpp:member:: protected Process::weak_ptr proc

.. cpp:class:: LibraryTrackingSet

  .. cpp:function:: bool setTrackLibraries(bool b) const
  .. cpp:function:: bool refreshLibraries() const
  .. cpp:function:: protected LibraryTrackingSet(ProcessSet::ptr ps_)
  .. cpp:function:: protected ~LibraryTrackingSet()
  .. cpp:member:: protected ProcessSet::weak_ptr wps

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

  .. cpp:function:: protected ThreadTracking(Process::ptr proc_)
  .. cpp:function:: protected ~ThreadTracking()
  .. cpp:member:: protected Process::weak_ptr proc
  .. cpp:member:: protected static bool default_track_threads


.. cpp:class:: ThreadTrackingSet

  .. cpp:function:: bool setTrackThreads(bool b) const
  .. cpp:function:: bool refreshThreads() const
  .. cpp:function:: protected ThreadTrackingSet(ProcessSet::ptr ps_)
  .. cpp:function:: protected ~ThreadTrackingSet()
  .. cpp:member:: protected ProcessSet::weak_ptr wps


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



.. cpp:class:: LWPTrackingSet

  .. cpp:function:: bool setTrackLWPs(bool b) const
  .. cpp:function:: bool refreshLWPs() const
  .. cpp:function:: protected LWPTrackingSet(ProcessSet::ptr ps_)
  .. cpp:function:: protected ~LWPTrackingSet()
  .. cpp:member:: protected ProcessSet::weak_ptr wps


.. cpp:class:: FollowFork

  The FollowFork class is used to configure ProcControlAPI’s behavior when
  the associated :cpp:class:`Process` forks.

  .. cpp:function:: static void setDefaultFollowFork(follow_t f)

    Sets the default forking behavior across all :cpp:class:`Process` objects.

  .. cpp:function:: static follow_t getDefaultFollowFork()

    Returns the default forking behavior across all :cpp:class:`Process` objects.

  .. cpp:function:: bool setFollowFork(follow_t f) const

    Sets the forking behavior for the associated :cpp:class:`Process` object.

    Returns ``true`` on success.

  .. cpp:function:: follow_t getFollowFork() const

    This function returns the current forking behavior for the associated :cpp:class:`Process`.

  .. cpp:function:: protected FollowFork(Process::ptr proc_)
  .. cpp:function:: protected ~FollowFork()
  .. cpp:member:: protected Process::weak_ptr proc
  .. cpp:member:: protected static follow_t default_should_follow_fork


.. cpp:enum:: FollowFork::follow_t
  
  .. cpp:enumerator:: None

    Fork tracking is not available for the current platform.

  .. cpp:enumerator:: ImmediateDetach

    Forked children are never attached to.

  .. cpp:enumerator:: DisableBreakpointsDetach

    Inherited breakpoints are removed from forked children, and then the children are detached.

  .. cpp:enumerator:: Follow

    Forked children are attached to and remain under full control of ProcControlAPI. This is the default behavior.


.. cpp:class:: FollowForkSet

  .. cpp:function:: protected FollowForkSet(ProcessSet::ptr ps_)
  .. cpp:function:: protected ~FollowForkSet()
  .. cpp:member:: protected ProcessSet::weak_ptr wps
  .. cpp:function:: bool setFollowFork(FollowFork::follow_t f) const


.. cpp:class:: CallStackCallback

  .. cpp:member:: bool top_first
  .. cpp:function:: CallStackCallback()
  .. cpp:function:: virtual bool beginStackWalk(Thread::ptr thr) = 0
  .. cpp:function:: virtual bool addStackFrame(Thread::ptr thr, Dyninst::Address ra, Dyninst::Address sp, Dyninst::Address fp) = 0
  .. cpp:function:: virtual void endStackWalk(Thread::ptr thr) = 0
  .. cpp:function:: virtual ~CallStackCallback()


.. cpp:class:: CallStackUnwinding

  .. cpp:function:: CallStackUnwinding(Thread::ptr t)
  .. cpp:function:: virtual ~CallStackUnwinding()
  .. cpp:function:: bool walkStack(CallStackCallback *stk_cb) const


.. cpp:class:: MemoryUsage

  .. cpp:function:: bool sharedUsed(unsigned long &sused) const
  .. cpp:function:: bool heapUsed(unsigned long &hused) const
  .. cpp:function:: bool stackUsed(unsigned long &sused) const
  .. cpp:function:: bool resident(unsigned long &resident) const


.. cpp:class:: MemoryUsageSet

  .. cpp:function:: protected MemoryUsageSet(ProcessSet::ptr ps_)
  .. cpp:function:: protected ~MemoryUsageSet()
  .. cpp:member:: protected ProcessSet::weak_ptr wps
  .. cpp:function:: protected bool usedX(std::map<Process::const_ptr, unsigned long> &used, mem_usage_t mu) const
  .. cpp:function:: bool sharedUsed(std::map<Process::const_ptr, unsigned long> &used) const
  .. cpp:function:: bool heapUsed(std::map<Process::const_ptr, unsigned long> &used) const
  .. cpp:function:: bool stackUsed(std::map<Process::const_ptr, unsigned long> &used) const
  .. cpp:function:: bool resident(std::map<Process::const_ptr, unsigned long> &res) const


.. cpp:enum:: memoryUsageSet::mem_usage_t

  .. cpp:enumerator:: mem_usage_t
  .. cpp:enumerator:: mus_shared
  .. cpp:enumerator:: mus_heap
  .. cpp:enumerator:: mus_stack
  .. cpp:enumerator:: mus_resident


.. cpp:class:: CallStackUnwindingSet

  .. cpp:function:: CallStackUnwindingSet(ThreadSet::ptr ts)
  .. cpp:function:: ~CallStackUnwindingSet()
  .. cpp:function:: bool walkStack(CallStackCallback *stk_cb)


.. cpp:class:: MultiToolControl

  .. cpp:type:: unsigned int priority_t
  .. cpp:function:: protected MultiToolControl(Process::ptr p)
  .. cpp:function:: protected ~MultiToolControl()
  .. cpp:member:: protected static std::string default_tool_name
  .. cpp:member:: protected static priority_t default_tool_priority
  .. cpp:function:: static void setDefaultToolName(std::string name)
  .. cpp:function:: static void setDefaultToolPriority(priority_t p)
  .. cpp:function:: static std::string getDefaultToolName()
  .. cpp:function:: static priority_t getDefaultToolPriority()
  .. cpp:function:: std::string getToolName() const
  .. cpp:function:: priority_t getToolPriority() const


.. cpp:type:: sigset_t dyn_sigset_t


.. cpp:class:: SignalMask

  The SignalMask class is used to configure the signal mask for its associated :cpp:class:`Process`.

  On POSIX systems, this type is equivalent to `sigset_t <https://www.man7.org/linux/man-pages/man0/signal.h.0p.html>`_.

  .. cpp:function:: static dyn_sigset_t getDefaultSigMask()

      Returns the current default signal mask.

  .. cpp:function:: static void setDefaultSigMask(dyn_sigset_t s)

      This function sets the default signal mask across all :cpp:class:`Process`.

  .. cpp:function:: bool setSigMask(dyn_sigset_t s)

      This function sets the signal mask for the associated :cpp:class:`Process`.

      Returns ``true`` on success.

  .. cpp:function:: dyn_sigset_t getSigMask() const

      This function returns the current signal mask for the associated :cpp:class:`Process`.

  .. cpp:member:: protected static dyn_sigset_t default_sigset
  .. cpp:member:: protected static bool sigset_initialized


.. cpp:struct:: stat64_ret_t

  This struct is copied from the GLIBC sources for 'struct stat64'.  It is
  recreated here because this header is supposed to compile without ifdefs
  across platforms that may not have 'struct stat64'

  .. cpp:member:: unsigned long long st_dev
  .. cpp:member:: unsigned long long st_ino
  .. cpp:member:: unsigned int st_mode
  .. cpp:member:: unsigned int st_nlink
  .. cpp:member:: unsigned int st_uid
  .. cpp:member:: unsigned int st_gid
  .. cpp:member:: unsigned long long st_rdev
  .. cpp:member:: unsigned short __pad2
  .. cpp:member:: long long st_size
  .. cpp:member:: int st_blksize
  .. cpp:member:: long long st_blocks
  .. cpp:member:: int st_atime_
  .. cpp:member:: unsigned int st_atime_nsec
  .. cpp:member:: int st_mtime_
  .. cpp:member:: unsigned int st_mtime_nsec
  .. cpp:member:: int st_ctime_
  .. cpp:member:: unsigned int st_ctime_nsec
  .. cpp:member:: unsigned int __unused4
  .. cpp:member:: unsigned int __unused5

.. cpp:type:: stat64_ret_t *stat64_ptr
.. cpp:type:: boost::shared_ptr<int_fileInfo> int_fileInfo_ptr


.. cpp:class:: FileInfo

  .. cpp:function:: FileInfo(std::string fname)
  .. cpp:function:: FileInfo()
  .. cpp:function:: FileInfo(const FileInfo &fi)
  .. cpp:function:: ~FileInfo()
  .. cpp:function:: std::string getFilename() const
  .. cpp:function:: stat64_ptr getStatResults() const

.. cpp:type:: std::multimap<Process::const_ptr, FileInfo> FileSet


.. cpp:class:: RemoteIO

  .. cpp:member:: protected Process::weak_ptr proc
  .. cpp:function:: RemoteIO(Process::ptr proc)
  .. cpp:function:: virtual ~RemoteIO()
  .. cpp:function:: FileSet *getFileSet(std::string filename) const
  .. cpp:function:: FileSet *getFileSet(const std::set<std::string> &filenames) const
  .. cpp:function:: bool addToFileSet(std::string filename, FileSet *fs) const
  .. cpp:function:: bool getFileNames(FileSet *result) const
  .. cpp:function:: bool getFileStatData(FileSet *fset) const
  .. cpp:function:: bool readFileContents(const FileSet *fset)


.. cpp:class:: RemoteIOSet

  .. cpp:member:: protected ProcessSet::weak_ptr pset
  .. cpp:function:: RemoteIOSet(ProcessSet::ptr procs_)
  .. cpp:function:: virtual ~RemoteIOSet()
  .. cpp:function:: FileSet *getFileSet(std::string filename)
  .. cpp:function:: FileSet *getFileSet(const std::set<std::string> &filenames)
  .. cpp:function:: bool addToFileSet(std::string filename, FileSet *fs)
  .. cpp:function:: bool getFileNames(FileSet *result)
  .. cpp:function:: bool getFileStatData(FileSet *fset)
  .. cpp:function:: bool readFileContents(const FileSet *fset)
