.. _`sec:BPatch_object.h`:

BPatch_object.h
###############

.. cpp:class:: BPatch_object
   
  An object of this class represents the original executable or a library.
  It serves as a container of BPatch_module objects.

  .. cpp:function:: std::string name()

  .. cpp:function:: std::string pathName()

    Return the name of this file; either just the file name or the fully
    path-qualified name.

  .. cpp:function:: Dyninst::Address fileOffsetToAddr(Dyninst::Offset offset)

    Convert the provided offset into the file into a full address in memory.

  .. cpp:struct:: Region

  .. cpp:enum:: @typet

    .. cpp:enumerator:: UNKNOWN

    .. cpp:enumerator:: CODE

    .. cpp:enumerator:: DATA

  .. cpp:type:: @typet type_t

    .. cpp:member:: Dyninst::Address base

    .. cpp:member:: unsigned long size

    .. cpp:member:: type_t type

  .. cpp:function:: void regions(std::vector<Region> &regions)

    Returns information about the address ranges occupied by this object in
    memory.

  .. cpp:function:: void modules(std::vector<BPatch_module *> &modules)

    Returns the modules contained in this object.

  .. cpp:function:: std::vector<BPatch_function*> *findFunction( \
       const char *name,\
       std::vector<BPatch_function*> &funcs,\
       bool dont_use_regex, \
       bool showError = true,\
       bool regex_case_sensitive = true,\
       bool incUninstrumentable = false)

    Return a vector of BPatch_functions corresponding to name, or NULL if
    the function does not exist. If name contains a POSIX-extended regular
    expression, and dont_use_regex is false, a regular expression search
    will be performed on function names and matching BPatch_functions
    returned. If showError is true, then Dyninst will report an error via
    the BPatch::registerErrorCallback if no function is found.

    If the incUninstrumentable flag is set, the returned table of procedures
    will include uninstrumentable functions. The default behavior is to omit
    these functions.

    .. note::
    
       If name is not found to match any demangled function names in
       the module, the search is repeated as if name is a mangled function
       name. If this second search succeeds, functions with mangled names
       matching name are returned instead.
    
  .. cpp:function:: bool findPoints(Dyninst::Address addr, std::vector<BPatch_point *> &points);

    Return a vector of BPatch_points that correspond with the provided
    address, one per function that includes an instruction at that address.
    There will be one element if there is not overlapping code.

  .. cpp:function:: std::vector<BPatch_function*> *findFunction( \
       const char *name, \
       std::vector<BPatch_function*> &funcs, \
       bool notify_on_failure = true, \
       bool regex_case_sensitive = true, \
       bool incUninstrumentable = false)

    Return a vector of BPatch_functions matching name, or NULL if the
    function does not exist. If name contains a POSIX-extended regular
    expression, a regex search will be performed on function names, and
    matching BPatch_functions returned. [**NOTE**: The std::vector argument
    funcs must be declared fully by the user before calling this function.
    Passing in an uninitialized reference will result in undefined
    behavior.]

    If the incUninstrumentable flag is set, the returned table of procedures
    will include uninstrumentable functions. The default behavior is to omit
    these functions.

    .. note::
       If name is not found to match any demangled function names in
       the BPatch_object, the search is repeated as if name is a mangled
       function name. If this second search succeeds, functions with mangled
       names matching name are returned instead.
       
  .. cpp:function:: BpatchSnippetHandle* insertInitCallback(Bpatch_snippet& callback)

    This function inserts the snippet callback at the entry point of this
    module’s init function (creating a new init function/section if
    necessary).

  .. cpp:function:: BpatchSnippetHandle* insertFiniCallback(Bpatch_snippet& callback)

    This function inserts the snippet callback at the exit point of this
    module’s fini function (creating a new fini function/section if
    necessary).
