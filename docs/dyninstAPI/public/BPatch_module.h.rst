BPatch_module.h
===============

``BPatch_module``
-----------------
.. cpp:namespace:: BPatch_module

.. cpp:class:: BPatch_module
   
   An object of this class represents a program module, which is part of a
   program’s executable image. A BPatch_module represents a source file in
   either an executable or a shared library. Dyninst automatically creates
   a default module in each executable to hold any objects
   that it cannot match to a source file. BPatch_module objects are
   obtained by calling the BPatch_image member function getModules.
   
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
      
      [**NOTE**: If name is not found to match any demangled function names in
      the module, the search is repeated as if name is a mangled function
      name. If this second search succeeds, functions with mangled names
      matching name are returned instead.]
      
   .. cpp:function:: BPatch_Vector<BPatch_function *> *findFunctionByAddress( \
         void *addr, \
         BPatch_Vector<BPatch_function *> &funcs, \
         bool notify_on_failure = true, \
         bool incUninstrumentable = false)
      
      Return a vector of BPatch_functions that contains addr, or NULL if the
      function does not exist. [**NOTE**: The std::vector argument funcs must
      be declared fully by the user before calling this function. Passing in
      an uninitialized reference will result in undefined behavior.]
      
      If the incUninstrumentable flag is set, the returned table of procedures
      will include uninstrumentable functions. The default behavior is to omit
      these functions.
      
   .. cpp:function:: BPatch_function *findFunctionByEntry(Dyninst::Address addr)
      
      Returns the function that begins at the specified address addr.
      
   .. cpp:function:: BPatch_function *findFunctionByMangled( \
         const char *mangled_name, \
         bool incUninstrumentable = false)
      
      Return a BPatch_function for the mangled function name defined in the
      module corresponding to the invoking BPatch_module, or NULL if it does
      not define the function.
      
      If the incUninstrumentable flag is set, the functions searched will
      include uninstrumentable functions. The default behavior is to omit
      these functions.
      
   .. cpp:function:: bool getAddressRanges( char * fileName, unsigned int lineNo, \
         std::vector< std::pair< unsigned long, unsigned long > > & ranges )
      
      Given a filename and line number, fileName and lineNo, this function
      this function returns the ranges of mutatee addresses that implement the
      code range in the output parameter ranges. In many cases a source code
      line will only have one address range implementing it. However, compiler
      optimizations may turn this into multiple, disjoint address ranges. This
      information is only available if the mutatee was compiled with debug
      information.
      
      This function may be more efficient than the BPatch_process version of
      this function. Calling BPatch_process::getAddressRange will cause
      Dyninst to parse line information for all modules in a process. If
      BPatch_module::getAddressRange is called then only the debug information
      in this module will be parsed.
      
      This function returns true if it was able to find any line information,
      false otherwise.
      
   .. cpp:function:: size_t getAddressWidth()
      
      Return the size (in bytes) of a pointer in this module. On 32-bit
      systems this function will return 4, and on 64-bit systems this function
      will return 8.
      
   .. cpp:function:: void *getBaseAddr()
      
      Return the base address of the module. This address is defined as the
      start of the first function in the module.
      
   .. cpp:function:: std::vector<BPatch_function *>* getProcedures( bool incUninstrumentable = false )
      
      Return a vector containing the functions in the module.
      
   .. cpp:function:: char *getFullName(char *buffer, int length)
      
      Fills buffer with the full path name of a module, up to length
      characters when this information is available.
      
   .. cpp:function:: BPatch_hybridMode getHybridMode()
      
      Return the mutator’s analysis mode for the mutate; the default mode is
      the normal mode.
      
   .. cpp:function:: char *getName(char *buffer, int len)
      
      This function copies the filename of the module into buffer, up to len
      characters. It returns the value of the buffer parameter.
      
   .. cpp:function:: unsigned long getSize()
      
      Return the size of the module. The size is defined as the end of the
      last function minus the start of the first function.
      
   .. cpp:function:: bool getSourceLines( unsigned long addr, std::vector<BPatch_statement> &lines )
      
      This function returns the line information associated with the mutatee
      address addr. The vector lines contain pairs of filenames and line
      numbers that are associated with addr. In many cases only one filename
      and line number is associated with an address, but certain compiler
      optimizations may lead to multiple filenames and lines at an address.
      This information is only available if the mutatee was compiled with
      debug information.
      
      This function may be more efficient than the BPatch_process version of
      this function. Calling BPatch_process::getSourceLines will cause Dyninst
      to parse line information for all modules in a process. If
      BPatch_module::getSourceLines is called then only the debug information
      in this module will be parsed.
      
      This function returns true if it was able to find any line information
      at addr, or false otherwise.
      
   .. cpp:function:: char *getUniqueString(char *buffer, int length)
      
      Performs a lookup and returns a unique string for this image. Returns a
      string the can be compared (via strcmp) to indicate if two images refer
      to the same underlying object file (i.e., executable or library). The
      contents of the string are implementation specific and defined to have
      no semantic meaning.
      
   .. cpp:function:: bool getVariables(std::vector<BPatch_variableExpr *> &vars)
      
      Fill the vector vars with the global variables that are specified in
      this module. Returns false if no results are found and true otherwise.
      
   .. cpp:function:: BpatchSnippetHandle* insertInitCallback(Bpatch_snippet& callback)
      
      This function inserts the snippet callback at the entry point of this
      module’s init function (creating a new init function/section if
      necessary).
      
   .. cpp:function:: BpatchSnippetHandle* insertFiniCallback(Bpatch_snippet& callback)
      
      This function inserts the snippet callback at the exit point of this
      module’s fini function (creating a new fini function/section if
      necessary).
      
   .. cpp:function:: bool isExploratoryModeOn()
      
      This function returns true if the mutator’s analysis mode sets to the
      defensive mode or the exploratory mode.
      
   .. cpp:function:: bool isMutatee()
      
      This function returns true if the module is the mutatee.
      
   .. cpp:function:: bool isSharedLib()
      
      This function returns true if the module is part of a shared library.