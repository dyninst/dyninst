.. _`sec:BPatch_object.h`:

BPatch_object.h
###############

.. cpp:class:: BPatch_object

  **The original executable or library**

  It serves as a container of :cpp:class:`BPatch_module` objects.

  .. cpp:member:: static const Dyninst::Address E_OUT_OF_BOUNDS

  .. cpp:function:: std::string name()

    Returns the file name of the object

  .. cpp:function:: std::string pathName()

    Returns the full pathname of the object

  .. cpp:function:: Dyninst::Address fileOffsetToAddr(const Dyninst::Offset offset)

    Converts a file offset into an absolute address suitable for use in looking up functions or points.

    For dynamic instrumentation, this is an address in memory. For binary rewriting, this is an offset
    that can be treated as an address.

    Returns :cpp:var::`E_OUT_OF_BOUNDS` on failure.

  .. cpp:function:: void regions(std::vector<Region> &regions)

    Returns a vector of address ranges occupied by this object May be multiple if there are
    multiple disjoint ranges, such as separate code and data or multiple code regions Ranges
    are returned as (base, size, type) tuples.

  .. cpp:function:: void modules(std::vector<BPatch_module *> &modules)

    Returns a vector of BPatch_modules logically contained in this object.

    By design, shared libraries contain a single module executable files contain one or more.

  .. cpp:function:: std::vector<BPatch_function*>* findFunction(std::string name, std::vector<BPatch_function*> &funcs,\
                                                                bool notify_on_failure =true, bool regex_case_sensitive =true,\
                                                                bool incUninstrumentable =false, bool dont_use_regex = false)

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

  .. cpp:function:: bool findPoints(Dyninst::Address addr, std::vector<BPatch_point *> &points)

    Returns a vector of BPatch_points that correspond with the provided address, one per function that
    includes an instruction at that address. Will have one element if there is not overlapping code.

  .. cpp:function:: void addModsAllFuncs(const std::set<StackMod*> &mods, bool interproc,\
                                         std::vector<std::pair<BPatch_function*, bool>> &modResults,\
                                         unsigned depthLimit = 0)

    Apply stack modifications in mods to all functions in the current object.

    Perform error checking, handle stack alignment requirements, and generate any modifications required for cleanup
    at function exit. Atomically adds all modifications in mods if any mod is found to be unsafe, none of the
    modifications are applied.  If interproc is true, interprocedural analysis is used for more precise evaluation
    of modification safety (i.e. modifications that are actually safe are more likely to be correctly identified as
    safe, but analysis will take longer).  depthLimit specifies the maximum depth allowed for interprocedural analysis,
    and is only used if interproc is true.  Note that depthLimit 0 will still analyze interprocedural edges within the
    current object it just won't analyze edges between this object and another object. Returns in modResults a vector of
    (function, instrumented) pairs where instrumented is true if stack modifications were successfully added.

  .. cpp:function:: BPatchSnippetHandle* insertInitCallback(BPatch_snippet& callback)

    This function inserts the snippet callback at the entry point of this
    module’s init function (creating a new init function/section if
    necessary).

  .. cpp:function:: BpatchSnippetHandle* insertFiniCallback(Bpatch_snippet& callback)

    This function inserts the snippet callback at the exit point of this
    module’s fini function (creating a new fini function/section if
    necessary).



.. cpp:struct:: BPatch_object::Region

  .. cpp:member:: Dyninst::Address base
  .. cpp:member:: unsigned long size
  .. cpp:member:: type_t type

.. cpp:enum:: BPatch_object::Region::type_t

  .. cpp:enumerator:: UNKNOWN
  .. cpp:enumerator:: CODE
  .. cpp:enumerator:: DATA


.. cpp:namespace:: Dyninst::ParseAPI

.. cpp::function:: CodeObject *convert(const BPatch_object *)

.. cpp:namespace:: Dyninst::PatchAPI

.. cpp::function:: PatchObject *convert(const BPatch_object *)

.. cpp:namespace:: Dyninst::SymtabAPI

.. cpp:function:: Symtab *convert(const BPatch_object *)

