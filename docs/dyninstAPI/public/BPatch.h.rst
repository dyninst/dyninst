.. _`sec:BPatch.h`:

BPatch.h
########

.. cpp:class:: BPatch
   
  The ``BPatch`` class represents the entire Dyninst library. There can
  only be one instance of this class at a time. This class is used to
  perform functions and obtain information that is not specific to a
  particular thread or image.

  .. cpp:function:: std::vector<BPatch_process*> *getProcesses()

    Returns the list of processes that are currently defined.

    This list
    includes processes that were directly created by calling :cpp:func:`processCreate` or
    :cpp:func:`processAttach`, and indirectly via
    UNIX `fork <https://www.man7.org/linux/man-pages/man2/open.2.html>`_. It is
    up to the user to delete this vector when they are done with it.

  .. cpp:function:: BPatch_process *processAttach(const char *path, int pid,\
                    BPatch_hybridMode mode=BPatch_normalMode)

    Returns a new instance of :cpp:class:`BPatch_process` associated
    with an existing process.

    ``path`` is the pathname of the executable file containing
    the process image. If it is ``NULL``, the executable image is derived from the process
    pid on Linux platforms. Attaching to a process puts it into the stopped state.

  .. cpp:function:: BPatch_process *processCreate(const char *path, const char *argv[], \
                      const char **envp = NULL, int stdin_fd=0, int stdout_fd=1, int \
                      stderr_fd=2, BPatch_hybridMode mode=BPatch_normalMode)

    Creates a new process and returns a new instance of
    :cpp:class:`BPatch_process` associated with it.

    ``path`` is the pathname of the executable file containing
    the process image. If it is ``NULL``, the executable image is derived from the process
    pid on Linux platforms. The new process is put into a stopped state before executing any code.
    ``stdin_fd``, ``stdout_fd``, and ``stderr_fd`` are used to set the
    standard input, output, and error streams of the child process. The default
    values are the same as those of the mutator process. To change these values,
    an open UNIX file descriptor (see `open <https://www.man7.org/linux/man-pages/man2/open.2.html>`_) can be passed.
    ``mode`` selects the desired level of code analysis.

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

  .. cpp:function:: BPatch_binaryEdit *openBinary(const char *path, bool openDependencies = false)

    Opens the executable or library file pointed to by ``path`` for binary rewriting.

    If ``openDependencies`` is ``true``, Dyninst will
    also open all shared libraries that path depends on. Upon success, this
    function returns a new instance of a :cpp:class:`BPatch_binaryEdit` that
    represents the opened file and any dependent shared libraries.

    Returns ``NULL`` on error.

  .. cpp:function:: bool pollForStatusChange()

    Checks if there has been a change in the status
    of one or more threads that has not yet been reported by either
    :cpp:func:`isStopped` or :cpp:func:`isTerminated`.

    This is useful for a mutator that needs to periodically check on the
    status of its managed threads and does not want to check each process
    individually.

  .. cpp:function:: void setDebugParsing (bool state)

    Turns on or off the parsing of debugger information.

    By default, compiler-generated debug information is parsed on
    those platforms that support it. For some applications, this
    information can be quite large. To disable parsing this information,
    pass ``state=false`` prior to creating a process.

  .. cpp:function:: bool parseDebugInfo()

    Returns ``true`` if debugger information parsing is enabled.

  .. cpp:function:: void setTrampRecursive (bool state)

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

  .. cpp:function:: bool isTrampRecursive ()

    Returns ``true`` if trampoline recursion is enabled.

  .. cpp:function:: void setTypeChecking(bool state)

    Turns on or off type-checking of snippets.

    By default type-checking is
    turned on, and an attempt to create a snippet that contains type
    conflicts will fail. Any snippet expressions created with type-checking
    off have the type of their left operand. Turning type-checking off,
    creating a snippet, and then turning type-checking back on is similar to
    the type cast operation in the C programming language.

  .. cpp:function:: bool isTypeChecked()

    Returns ``true`` if type-checking of snippets is enabled.

  .. cpp:function:: bool waitForStatusChange()

    Waits until there is a status change to some thread that
    has not yet been reported by either isStopped or isTerminated.

    It is more efficient to call this function than to call
    :cpp:func:`pollForStatusChange` in a loop, because this blocks the
    mutator process while waiting.

    Returns ``false`` on error.

  .. cpp:function:: void setDelayedParsing(bool)

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

  .. cpp:function:: bool delayedParsingOn()

    Returns ``true`` if delayed parsing is enabled.

  .. cpp:function:: void setInstrStackFrames(bool)

    Turns on and off stack frames in instrumentation.

    When enabled, Dyninst will
    create stack frames around instrumentation. A stack frame allows Dyninst
    or other tools to walk a call stack through instrumentation, but
    introduces overhead to instrumentation. The default is to not create
    stack frames.

  .. cpp:function:: bool getInstrStackFrames()

    Returns ``true`` if instrumentation will create stack frames.

  .. cpp:function:: void setMergeTramp (bool)

    Turns on or off inlined tramps.

    Setting this value to ``true`` will make each
    base trampoline have all of its mini-trampolines inlined within it.
    Using inlined mini-tramps may allow instrumentation to execute faster,
    but inserting and removing instrumentation may take more time. The
    default setting for this is ``true``.

  .. cpp:function:: bool isMergeTramp ()

    Returns the current status of inlined trampolines.

    Returns ``true`` if trampolines are inlined.

  .. cpp:function:: void setSaveFPR (bool)

    Turn on or off floating point saves.

    Setting this value to ``false`` means
    that floating point registers will never be saved, which can lead to
    large performance improvements. The default value is ``true``. Setting this
    flag may cause incorrect program behavior if the instrumentation does
    clobber floating point registers, so it should only be used when the
    user is positive this will never happen.

  .. cpp:function:: bool isSaveFPROn ()

    Returns ``true`` if floating point registers are saved during instrumentation.

  .. cpp:function:: void setBaseTrampDeletion(bool)

    Turns on or off base tramp deletion.

    If ``true``, the base tramp is deleted when the last corresponding minitramp
    is deleted. If ``false``, the base tramp is untouched in. The default value is
    ``false``.

  .. cpp:function:: bool baseTrampDeletion()

    Returns ``true`` if base trampolines are set to be deleted.

  .. cpp:function:: void setLivenessAnalysis(bool)

    Turns on or off register liveness analysis.

    If ``true``, register liveness analysis is performed around an :cpp:class:`instPoint`
    before inserting instrumentation, and registers that are
    live at that point are saved. This can lead to faster run-time speeds at the
    expense of slower instrumentation time. The default value is ``true``.

  .. cpp:function:: bool livenessAnalysisOn()

    Returns ``true`` if liveness analysis is currently enabled.

  .. cpp:function:: void getBPatchVersion(int &major, int &minor, int &subminor)

    Returns the version number for Dyninst.

    The major version number will be stored
    in ``major``, the minor version number in ``minor``, and the subminor version in
    ``subminor``. For example, under Dyninst 5.1.0, this function will return 5
    in ``major``, 1 in ``minor``, and 0 in ``subminor``.

  .. cpp:function:: int getNotificationFD()

    Returns a file descriptor that is suitable for inclusion in a call to
    `select() <https://www.man7.org/linux/man-pages/man2/select.2.html>`_

    Dyninst will write data to this file descriptor when it to
    signal a state change in the process. :cpp:func:`pollForStatusChange` should
    then be called so that Dyninst can handle the state change. This is
    useful for applications where the user does not want to block in
    :cpp:func:`waitForStatusChange`. The file descriptor will reset when the
    user calls :cpp:func:`pollForStatusChange`.

  .. cpp:function:: BPatch_type *createArray(const char *name, BPatch_type *ptr, unsigned int low, unsigned int hi)

    Creates a new array type.

    The name of the type is ``name``, and the type of
    each element is ``ptr``. The index of the first element of the array is ``low``,
    and the last is ``high``.

  .. cpp:function:: BPatch_type *createEnum(const char *name, std::vector<char *> &elementNames, \
                      std::vector<int> &elementIds)

  .. cpp:function:: BPatch_type *createEnum(const char *name, std::vector<char *> &elementNames)

    Create a new enumerated type. There are two variations of this function.
    The first one is used to create an enumerated type where the user
    specifies the identifier (int) for each element. In the second form, the
    system specifies the identifiers for each element. In both cases, a
    vector of character arrays is passed to supply the names of the elements
    of the enumerated type. In the first form of the function, the number of
    element in the elementNames and elementIds vectors must be the same, or
    the type will not be created and this function will return NULL. The
    standard rules of type compatibility, described in Section 4.28, are
    used with enums created using this function.

  .. cpp:function:: BPatch_type *createScalar(const char *name, int size)

    Create a new scalar type. The name field is used to specify the name of
    the type, and the size parameter is used to specify the size in bytes of
    each instance of the type. No additional information about this type is
    supplied. The type is compatible with other scalars with the same name
    and size.

  .. cpp:function:: BPatch_type *createStruct(const char *name, std::vector<char *> &fieldNames, \
                      std::vector<BPatch_type *> &fieldTypes)

    Create a new structure type. The name of the structure is specified in
    the name parameter. The fieldNames and fieldTypes vectors specify fields
    of the type. These two vectors must have the same number of elements or
    the function will fail (and return NULL). The standard rules of type
    compatibility, described in Section 4.28, are used with structures
    created using this function. The size of the structure is the sum of the
    size of the elements in the fieldTypes vector.

  .. cpp:function:: BPatch_type *createTypedef(const char *name, BPatch_type *ptr)

    Create a new type called name and having the type ptr.

  .. cpp:function:: BPatch_type *createPointer(const char *name, BPatch_type *ptr)

  .. cpp:function:: BPatch_type *createPointer(const char *name, BPatch_type *ptr, int size)

    Create a new type, named name, which points to objects of type ptr. The
    first form creates a pointer whose size is equal to sizeof(void*)on the
    target platform where the muta­tee is running. In the second form, the
    size of the pointer is the value passed in the size parameter.

  .. cpp:function:: BPatch_type *createUnion(const char *name, std::vector<char *>&fieldNames, \
                      std::vector<BPatch_type *> &fieldTypes)

    Create a new union type. The name of the union is specified in the name
    parameter. The fieldNames and fieldTypes vectors specify fields of the
    type. These two vectors must have the same number of elements or the
    function will fail (and return NULL). The size of the union is the size
    of the largest element in the fieldTypes vector.
