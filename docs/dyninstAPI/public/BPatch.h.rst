.. _`sec:BPatch.h`:

BPatch.h
########

.. cpp:class:: BPatch
   
  **An entire Dyninst library**

  There can only be one instance of this class at a time. This class is used to
  perform functions and obtain information that is not specific to a
  particular thread or image.

  .. cpp:member:: static BPatch *bpatch
  .. cpp:member:: BPatch_builtInTypeCollection *builtInTypes
  .. cpp:member:: BPatch_typeCollection *stdTypes
  .. cpp:member:: BPatch_typeCollection *APITypes

      API/User defined types

  .. cpp:member:: BPatch_type *type_Error
  .. cpp:member:: BPatch_type *type_Untyped

  .. cpp:function:: static BPatch *getBPatch()

  .. cpp:function:: BPatch()
  .. cpp:function:: ~BPatch()
  .. cpp:function:: static const char *getEnglishErrorString(int number)

    Returns the descriptive error string for the error number ``number``.

  .. cpp:function:: static void formatErrorString(char *dst, int size, const char *fmt, const char * const *params)

    Takes a format string with an error message (obtained from getEnglishErrorString) and an array of
    parameters that were passed to an error callback function, and creates a string with the parameters
    substituted into it.

    - ``dst``: The address into which the formatted string should be copied.
    - ``size``: If the formatted string is equal to or longer than this number of characters, then it will be truncated to size-1 characters and terminated with a ``NULL``.
    - ``fmt``: The format string (returned by a function such as getEnglishErrorString).
    - ``params``: The array of parameters that were passed to an error callback function.



  .. cpp:function:: bool isTypeChecked()

    Returns ``true`` if type-checking of snippets is enabled.

  .. cpp:function:: bool parseDebugInfo()

   Returns ``true`` if debugger information parsing is enabled.

  .. cpp:function:: bool baseTrampDeletion()

    Returns ``true`` if base trampolines are set to be deleted.

  .. cpp:function:: void setPrelinkCommand(char *command)

    sets the fully qualified path name of the prelink command

  .. cpp:function:: char* getPrelinkCommand()

    gets the fully qualified path name of the prelink command

  .. cpp:function:: bool isTrampRecursive()

   Returns ``true`` if trampoline recursion is enabled.

  .. cpp:function:: bool isMergeTramp()

    Returns the current status of inlined trampolines.

    Returns ``true`` if trampolines are inlined.

  .. cpp:function:: bool isSaveFPROn()

    Returns ``true`` if floating point registers are saved during instrumentation.

  .. cpp:function:: bool isForceSaveFPROn()

   returns whether base tramp and mini-tramp is merged

  .. cpp:function:: bool hasForcedRelocation_NP()

   returns whether all instrumented functions will be relocated

  .. cpp:function:: bool autoRelocationOn()

   returns whether functions will be relocated when appropriate

  .. cpp:function:: bool delayedParsingOn()

   Returns ``true`` if delayed parsing is enabled.

  .. cpp:function:: bool livenessAnalysisOn()

    Returns ``true`` if liveness analysis is currently enabled.

  .. cpp:function:: int livenessAnalysisDepth()

  .. cpp:function:: BPatchErrorCallback registerErrorCallback(BPatchErrorCallback function)

    Registers a function that is to be called by the library when an error occurs or when there is
    status to report.

  .. cpp:function:: BPatchDynLibraryCallback registerDynLibraryCallback(BPatchDynLibraryCallback func)

    Register callback for new library events (eg. load)

  .. cpp:function:: BPatchForkCallback registerPostForkCallback(BPatchForkCallback func)

    Register callback to handle mutatee fork events (before fork)

  .. cpp:function:: BPatchForkCallback registerPreForkCallback(BPatchForkCallback func)

    Register callback to handle mutatee fork events (before fork)

  .. cpp:function:: BPatchExecCallback registerExecCallback(BPatchExecCallback func)

    Register callback to handle mutatee exec events

  .. cpp:function:: BPatchExitCallback registerExitCallback(BPatchExitCallback func)

    Register callback to handle mutatee exit events

  .. cpp:function:: BPatchOneTimeCodeCallback registerOneTimeCodeCallback(BPatchOneTimeCodeCallback func)

    Register callback to run at completion of oneTimeCode

  .. cpp:function:: bool registerThreadEventCallback(BPatch_asyncEventType type, BPatchAsyncThreadEventCallback cb)

    Registers a callback to run when a thread is created

  .. cpp:function:: bool removeThreadEventCallback(BPatch_asyncEventType type, BPatchAsyncThreadEventCallback cb)

    Registers a callback to run when a thread is destroyed

  .. cpp:function:: bool registerDynamicCallCallback(BPatchDynamicCallSiteCallback cb)

    Specifies a user-supplied function to be called when a dynamic call is  executed.

  .. cpp:function:: bool removeDynamicCallCallback(BPatchDynamicCallSiteCallback cb)

  .. cpp:function:: bool registerUserEventCallback(BPatchUserEventCallback cb)

    Specifies a user defined function to call when a "user event"
    occurs, user events are trigger by calls to the function
    ``DYNINSTuserMessage(void *, int)`` in the runtime library.

  .. cpp:function:: bool removeUserEventCallback(BPatchUserEventCallback cb)

  .. cpp:function:: bool registerSignalHandlerCallback(BPatchSignalHandlerCallback cb, std::set<long> &signal_numbers)

    If the mutator produces a signal matching an element of signal_numbers, the callback is invoked, returning the point
    that caused the exception, the signal number, and a Vector representing the address of signal handler(s) in the mutatee
    for the exception.  In Windows this is the handler stack, each function of which is invoked until one is found that agrees
    to handle the exception.  In Unix there will be at most one handler for the signal number, the handler registered with
    syscalls signal() or sigaction(), or the default system handler, in which case we return an empty vector.

  .. cpp:function:: bool registerSignalHandlerCallback(BPatchSignalHandlerCallback cb, BPatch_Set<long> *signal_numbers)

    If the mutator produces a signal matching an element of
    signal_numbers, the callback is invoked, returning the point
    that caused the exception, the signal number, and a Vector
    representing the address of signal handler(s) in the mutatee
    for the exception.  In Windows this is the handler stack, each
    function of which is invoked until one is found that agrees to
    handle the exception.  In Unix there will be at most one
    handler for the signal number, the handler registered with
    syscalls signal() or sigaction(), or the default system
    handler, in which case we return an empty vector.

  .. cpp:function:: bool removeSignalHandlerCallback(BPatchSignalHandlerCallback cb)
  .. cpp:function:: bool registerCodeDiscoveryCallback(BPatchCodeDiscoveryCallback cb)
  .. cpp:function:: bool removeCodeDiscoveryCallback(BPatchCodeDiscoveryCallback cb)
  .. cpp:function:: bool registerCodeOverwriteCallbacks(BPatchCodeOverwriteBeginCallback cbBegin, BPatchCodeOverwriteEndCallback cbEnd)

    Registers a callback at the beginning and end of overwrite events

  .. cpp:function:: BPatch_Vector<BPatch_process*> * getProcesses()

    Returns the list of processes that are currently defined.

    This list includes processes that were directly created by calling :cpp:func:`processCreate` or
    :cpp:func:`processAttach`, and indirectly via
    UNIX `fork <https://www.man7.org/linux/man-pages/man2/open.2.html>`_. It is
    up to the user to delete this vector when they are done with it.

  .. cpp:function:: void setDebugParsing(bool x)

    Turns on or off the parsing of debugger information.

    By default, compiler-generated debug information is parsed on
    those platforms that support it. For some applications, this
    information can be quite large. To disable parsing this information,
    pass ``state=false`` prior to creating a process.

  .. cpp:function:: void setBaseTrampDeletion(bool x)

    Turns on or off base tramp deletion.

    If ``true``, the base tramp is deleted when the last corresponding minitramp
    is deleted. If ``false``, the base tramp is untouched in. The default value is
    ``false``.

  .. cpp:function:: void setTypeChecking(bool x)

    Turns on or off type-checking of snippets.

    By default type-checking is
    turned on, and an attempt to create a snippet that contains type
    conflicts will fail. Any snippet expressions created with type-checking
    off have the type of their left operand. Turning type-checking off,
    creating a snippet, and then turning type-checking back on is similar to
    the type cast operation in the C programming language.


  .. cpp:function:: void setInstrStackFrames(bool b)

    Turns on and off stack frames in instrumentation.

    When enabled, Dyninst will
    create stack frames around instrumentation. A stack frame allows Dyninst
    or other tools to walk a call stack through instrumentation, but
    introduces overhead to instrumentation. The default is to not create
    stack frames.

  .. cpp:function:: bool getInstrStackFrames()

    Returns ``true`` if instrumentation will create stack frames.

  .. cpp:function:: void truncateLineInfoFilenames(bool)

  .. deprecated:: v12.3.0

  .. cpp:function:: void setTrampRecursive(bool x)

    Turns on or off trampoline recursion.

    By default, any snippets invoked while another snippet is active will not be
    executed. This is the safest
    behavior, since recursively calling snippets can cause a program to take
    up all available system resources and die. For example, adding
    instrumentation code to the start of printf, and then calling printf
    from that snippet will result in infinite recursion.

    This protection operates at the granularity of an instrumentation point.
    When snippets are first inserted at a point, this flag determines
    whether code will be created with recursion protection. Changing the
    flag is **not** retroactive, and inserting more snippets will not change
    the recursion protection of the point. Recursion protection increases
    the overhead of instrumentation points, so if there is no way for the
    snippets to call themselves, calling this method with the parameter true
    will result in a performance gain.

  .. cpp:function:: void setMergeTramp(bool x)

    Turns on or off inlined tramps.

    Setting this value to ``true`` will make each
    base trampoline have all of its mini-trampolines inlined within it.
    Using inlined mini-tramps may allow instrumentation to execute faster,
    but inserting and removing instrumentation may take more time. The
    default setting for this is ``true``.

  .. cpp:function:: void setSaveFPR(bool x)

    Turn on or off floating point saves.

    Setting this value to ``false`` means
    that floating point registers will never be saved, which can lead to
    large performance improvements. The default value is ``true``. Setting this
    flag may cause incorrect program behavior if the instrumentation does
    clobber floating point registers, so it should only be used when the
    user is positive this will never happen.

  .. cpp:function:: void forceSaveFPR(bool x)

    Force Turn on/off merged base & mini-tramps - ignores isConservative

  .. cpp:function:: void setForcedRelocation_NP(bool x)

    Turn on/off forced relocation of instrumted functions

  .. cpp:function:: void setAutoRelocation_NP(bool x)

    Turn on/off function relocations, performed when necessary

  .. cpp:function:: void setDelayedParsing(bool x)

    Turns on or off delayed parsing.

    When it is activated Dyninst will
    initially parse only the symbol table information in any new modules
    loaded by the program, and will postpone more thorough analysis
    (instrumentation point analysis, variable analysis, and discovery of new
    functions in stripped binaries). This analysis will automatically occur
    when the information is necessary.

    Users which require small run-time perturbation of a program should not
    delay parsing; the overhead for analysis may occur at unexpected times
    if it is triggered by internal Dyninst behavior. Users who desire
    instrumentation of a small number of functions will benefit from delayed
    parsing.

  .. cpp:function:: void setLivenessAnalysis(bool x)

    Turns on or off register liveness analysis.

    If ``true``, register liveness analysis is performed around an :cpp:class:`instPoint`
    before inserting instrumentation, and registers that are
    live at that point are saved. This can lead to faster run-time speeds at the
    expense of slower instrumentation time. The default value is ``true``.

  .. cpp:function:: void setLivenessAnalysisDepth(int x)

  .. cpp:function:: BPatch_process* processCreate(const char* path, const char* argv[], const char** envp = NULL,\
                                                  int stdin_fd=0, int stdout_fd=1, int stderr_fd=2,\
                                                  BPatch_hybridMode mode=BPatch_normalMode)

    Creates a new process and returns a new instance of :cpp:class:`BPatch_process` associated with it.

    ``path`` is the pathname of the executable file containing
    the process image. If it is ``NULL``, the executable image is derived from the process
    pid on Linux platforms. The new process is put into a stopped state before executing any code.
    ``stdin_fd``, ``stdout_fd``, and ``stderr_fd`` are used to set the
    standard input, output, and error streams of the child process. The default
    values are the same as those of the mutator process. To change these values,
    an open UNIX file descriptor (see `open <https://www.man7.org/linux/man-pages/man2/open.2.html>`_) can be passed.
    ``mode`` selects the desired level of code analysis.

  .. cpp:function:: BPatch_process *processAttach(const char *path, int pid, BPatch_hybridMode mode=BPatch_normalMode)

    Returns a new instance of :cpp:class:`BPatch_process` associated
    with an existing process.

    ``path`` is the pathname of the executable file containing
    the process image. If it is ``NULL``, the executable image is derived from the process
    pid on Linux platforms. Attaching to a process puts it into the stopped state.

  .. cpp:function:: BPatch_binaryEdit * openBinary(const char *path, bool openDependencies = false)

    Opens the executable or library file pointed to by ``path`` for binary rewriting.

    If ``openDependencies`` is ``true``, Dyninst will
    also open all shared libraries that path depends on. Upon success, this
    function returns a new instance of a :cpp:class:`BPatch_binaryEdit` that
    represents the opened file and any dependent shared libraries.

    Returns ``NULL`` on error.

  .. cpp:function:: BPatch_type *createEnum(const char * name, BPatch_Vector<char *> &elementNames, BPatch_Vector<int> &elementIds)

   Create a new enumerated type with name ``name`` having enumerators ``elementNames`` and corresponding values ``elementIds``.

   .. warning::
      There must be both a name and a value for each enumerator. Otherwise, the type will not be created.

  .. cpp:function:: BPatch_type *createEnum(const char * name, BPatch_Vector<char *> &elementNames)

    Create a new enumerated type with name ``name`` having enumerators ``elementNames`` with default sequential values.

  .. cpp:function:: BPatch_type *createStruct(const char * name, BPatch_Vector<char *> &fieldNames, BPatch_Vector<BPatch_type *> &fieldTypes)

    Create a new structure type. The name of the structure is specified in
    the name parameter. The fieldNames and fieldTypes vectors specify fields
    of the type. These two vectors must have the same number of elements or
    the function will fail (and return NULL). The standard rules of type
    compatibility, described in Section 4.28, are used with structures
    created using this function. The size of the structure is the sum of the
    size of the elements in the fieldTypes vector.

  .. cpp:function:: BPatch_type *createUnion(const char * name, BPatch_Vector<char *> &fieldNames, BPatch_Vector<BPatch_type *> &fieldTypes)

    Create a new union type. The name of the union is specified in the name
    parameter. The fieldNames and fieldTypes vectors specify fields of the
    type. These two vectors must have the same number of elements or the
    function will fail (and return NULL). The size of the union is the size
    of the largest element in the fieldTypes vector.

  .. cpp:function:: BPatch_type *createArray(const char * name, BPatch_type * ptr, unsigned int low, unsigned int hi)

    Creates a new array type.

    The name of the type is ``name``, and the type of
    each element is ``ptr``. The index of the first element of the array is ``low``,
    and the last is ``high``.

  .. cpp:function:: BPatch_type *createPointer(const char * name, BPatch_type * ptr, int size = sizeof(void *))

    Create a new type, named ``name``, which points to objects of type ``ptr``.

  .. cpp:function:: BPatch_type *createScalar(const char * name, int size)

    Create a new scalar type. The name field is used to specify the name of
    the type, and the size parameter is used to specify the size in bytes of
    each instance of the type. No additional information about this type is
    supplied. The type is compatible with other scalars with the same name
    and size.

  .. cpp:function:: BPatch_type *createTypedef(const char * name, BPatch_type * ptr)

    Create a new type called ``name`` and having the type ``ptr``.

  .. cpp:function:: bool pollForStatusChange()

    Checks if there has been a change in the status
    of one or more threads that has not yet been reported by either
    :cpp:func:`isStopped` or :cpp:func:`isTerminated`.

    This is useful for a mutator that needs to periodically check on the
    status of its managed threads and does not want to check each process
    individually. User programs are required to call this or :cpp:func:`waitForStatusChange` before user-level
    callback functions are executed (for example, fork, exit, or a library load). Non-blocking
    form returns immediately if no callback is ready, or executes callback(s) then returns.

  .. cpp:function:: bool waitForStatusChange()

    Waits until there is a status change to some thread that
    has not yet been reported by either isStopped or isTerminated.

    It is more efficient to call this function than to call
    :cpp:func:`pollForStatusChange` in a loop, because this blocks the
    mutator process while waiting.

    Returns ``false`` on error.

  .. cpp:function:: int getNotificationFD()

    Returns a file descriptor that is suitable for inclusion in a call to
    `select() <https://www.man7.org/linux/man-pages/man2/select.2.html>`_

    Dyninst will write data to this file descriptor when it to
    signal a state change in the process. :cpp:func:`pollForStatusChange` should
    then be called so that Dyninst can handle the state change. This is
    useful for applications where the user does not want to block in
    :cpp:func:`waitForStatusChange`. The file descriptor will reset when the
    user calls :cpp:func:`pollForStatusChange`.

    For user programs that block on other things as well, we provide a (simulated) file descriptor
    that can be added to a poll or select fdset. When a callback is prepared the BPatch layer writes
    to this fd, thus making pollselect return. The user program should then call pollForStatusChange.
    The BPatch layer will handle clearing the file descriptor all the program must do  is call
    pollForStatusChange or waitForStatusChange.

  .. cpp:function:: bool waitUntilStopped(BPatch_thread *appThread)

    BPatch:: waitUntilStopped:  Block until specified process has stopped.

  .. cpp:function:: BPatch_stats & getBPatchStatistics()

    Get Instrumentation statistics

  .. cpp:function:: bool isConnected()
  .. cpp:function:: bool remoteConnect(BPatch_remoteHost &remote)
  .. cpp:function:: bool getPidList(BPatch_remoteHost &remote, BPatch_Vector<unsigned int> &pidlist)
  .. cpp:function:: bool getPidInfo(BPatch_remoteHost &remote, unsigned int pid, std::string &pidStr)
  .. cpp:function:: bool remoteDisconnect(BPatch_remoteHost &remote)
  .. cpp:function:: void addNonReturningFunc(std::string name)

    Globally specify that any function with a given name will not return



.. cpp:class:: BPatch_stats

  **Instrumentation statistics**

  Introduced to export this information to paradyn, which
  produces a summary of these numbers upon application exit.
  It probably makes more sense to maintain such numbers on a
  per-process basis.  But is set up globally due to historical
  precendent.

  .. cpp:member:: unsigned int pointsUsed
  .. cpp:member:: unsigned int totalMiniTramps
  .. cpp:member:: unsigned int trampBytes
  .. cpp:member:: unsigned int ptraceOtherOps
  .. cpp:member:: unsigned int ptraceOps
  .. cpp:member:: unsigned int ptraceBytes
  .. cpp:member:: unsigned int insnGenerated


Notes
*****

Activating hybrid code analysis causes Dyninst to augment its static
analysis of the code with run-time code discovery techniques. There are
three modes: :cpp:enumerator:`BPatch_normalMode`, :cpp:enumerator:`BPatch_exploratoryMode`, and
:cpp:enumerator:`BPatch_defensiveMode`. Normal mode enables the regular static analysis
features of Dyninst. Exploratory mode and defensive mode enable
addtional dynamic features to correctly analyze programs that contain
uncommon code patterns, such as malware. Exploratory mode is primarily
oriented towards analyzing dynamic control transfers, while defensive
mode additionally aims to tackle code obfuscation and self-modifying
code. Both of these modes are still experimental and should be used with
caution. Defensive mode is only supported on Windows.

Defensive mode has been tested on normal binaries (binaries that run
correctly under normal mode), as well as some simple, packed executables
(self-decrypting or decompressing). More advanced forms of code
obfuscation, such as self-modifying code, have not been tested recently.
The traditional Dyninst interface may be used for instrumentation of
binaries in defensive mode, but in the case of highly obfuscated code,
this interface may prove to be ineffective due to the lack of a complete
view of control flow at any given point. Therefore, defensive mode also
includes a set of callbacks that enables instrumentation to be performed
as new code is discovered. Due to the fact that recent efforts have
focused on simpler forms of obfuscation, these callbacks have not been
tested in detail. The next release of Dyninst will target more advanced
uses of defensive mode.
