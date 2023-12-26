BPatch_addressSpace.h
=====================

.. cpp:class:: BPatch_addressSpace
   
   The **BPatch_addressSpace** class is a superclass of the BPatch_process
   and BPatch_binaryEdit classes. It contains functionality that is common
   between the two sub classes.
   
   .. cpp:function:: BPatch_image *getImage()
      
      Return a handle to the executable file associated with this
      BPatch_process object.
      
   .. cpp:function:: bool getSourceLines(unsigned long addr, std::vector< BPatch_statement >& lines)
      
      This function returns the line information associated with the mutatee
      address, addr. The vector lines contain pairs of filenames and line
      numbers that are associated with addr. In many cases only one filename
      and line number is associated with an address, but certain compiler
      optimizations may lead to multiple filenames and lines at an address.
      This information is only available if the mutatee was compiled with
      debug information.
      
      This function returns true if it was able to find any line information
      at addr, or false otherwise.
      
   .. cpp:function:: bool getAddressRanges( const char * fileName, unsigned int lineNo, \
                        std::vector< std::pair< unsigned long, unsigned long > > & ranges )
      
      Given a filename and line number, fileName and lineNo, this function
      this function returns the ranges of mutatee addresses that implement the
      code range in the output parameter ranges. In many cases a source code
      line will only have one address range implementing it. However, compiler
      optimizations may transform this into multiple disjoint address ranges.
      This information is only available if the mutatee was compiled with
      debug information.
      
      This function returns true if it was able to find any line information,
      false otherwise.
      
   .. cpp:function:: BPatch_variableExpr *malloc(int n, std::string name = std::string(""))
      
   .. cpp:function:: BPatch_variableExpr *malloc(const BPatch_type &type, std::string name = std::string(""))
      
      These two functions allocate memory. Memory allocation is from a heap.
      The heap is not necessarily the same heap used by the application. The
      available space in the heap may be limited depending on the
      implementation. The first function, malloc(int n), allocates n bytes of
      memory from the heap. The second function, malloc(const BPatch_type& t),
      allocates enough memory to hold an object of the specified type. Using
      the second version is strongly encouraged because it provides additional
      information to permit better type checking of the passed code. If a name
      is specified, Dyninst will assign var_name to the variable; otherwise,
      it will assign an internal name. The returned memory is persistent and
      will not be released until BPatch_process::free is called or the
      application terminates.
      
   .. cpp:function:: BPatch_variableExpr *createVariable(Dyninst::Address addr, \
                        BPatch_type *type, \
                        std::string var_name = std::string(""), \
                        BPatch_module *in_module = NULL)
      
      This method creates a new variable at the given address addr in the
      module in_module. If a name is specified, Dyninst will assign var_name
      to the variable; otherwise, it will assign an internal name. The type
      parameter will become the type for the new variable.
      
      When operating in binary rewriting mode, it is an error for the
      in_module parameter to be NULL; it is necessary to specify the module in
      which the variable will be created. Dyninst will then write the variable
      back out in the file specified by in_module.
      
   .. cpp:function:: bool free(BPatch_variableExpr &ptr)
      
      Free the memory in the passed variable ptr. The programmer is
      responsible for verifying that all code that could reference this memory
      will not execute again (either by removing all snippets that refer to
      it, or by analysis of the program). Return true if the free succeeded.
      
   .. cpp:function:: bool getRegisters(std::vector<BPatch_register> &regs)
      
      This function returns a vector of BPatch_register objects that represent
      registers available to snippet code.
      
   .. cpp:function:: BPatchSnippetHandle *insertSnippet(const BPatch_snippet &expr, \
         BPatch_point &point, \
         BPatch_callWhen when=BPatch_callBefore| BPatch_callAfter, \
         BPatch_snippetOrder order = BPatch_firstSnippet)
      
   .. cpp:function:: BPatchSnippetHandle *insertSnippet(const BPatch_snippet &expr, \
         const std::vector<BPatch_point *> &points, \
         BPatch_callWhen when=BPatch_callBefore| BPatch_callAfter, \
         BPatch_snippetOrder order = BPatch_firstSnippet)
      
      Insert a snippet of code at the specified point. If a list of points is
      supplied, insert the code snippet at each point in the list. The
      optional when argument specifies when the snippet is to be called; a
      value of BPatch_callBefore indicates that the snippet should be inserted
      just before the specified point or points in the code, and a value of
      BPatch_callAfter indicates that it should be inserted just after them.
      
      The order argument specifies where the snippet is to be inserted
      relative to any other snippets previously inserted at the same point.
      The values BPatch_firstSnippet and BPatch_lastSnippet indicate that the
      snippet should be inserted before or after all snippets, respectively.
      
      It is illegal to use BPatch_callAfter with a BPatch_entry point. Use
      BPatch_callBefore when instrumenting entry points, which inserts
      instrumentation before the first instruction in a subroutine. Likewise,
      it is illegal to use BPatch_callBefore with a BPatch_exit point. Use
      BPatch_callAfter with exit points. BPatch_callAfter inserts
      instrumentation at the last instruction in the subroutine.
      insert­Snippet will return NULL when used with an illegal pair of
      points.
      
   .. cpp:function:: bool deleteSnippet(BPatchSnippetHandle *handle)
      
      Remove the snippet associated with the passed handle. If the handle is
      not defined for the process, then deleteSnippet will return false.
      
   .. cpp:function:: void beginInsertionSet()
      
      Normally, a call to insertSnippet immediately injects instrumentation
      into the mutatee. However, users may wish to insert a set of snippets as
      a single batch operation. This provides two benefits: First, Dyninst may
      insert instrumentation in a more efficient manner. Second, multiple
      snippets may be inserted at multiple points as a single operation, with
      either all snippets being inserted successfully or none. This batch
      insertion mode is begun with a call to beginInsertionSet; after this
      call, no snippets are actually inserted until a corresponding call to
      finalizeInsertionSet. Dyninst accumulates all calls to insertSnippet
      during batch mode internally, and the returned BPatchSnippetHandles are
      filled in when finalizeInsertionSet is called.
      
      Insertion sets are un­necessary when doing static binary
      instrumentation. Dyninst uses an implicit insertion set around all
      instrumentation to a static binary.
      
   .. cpp:function:: bool finalizeInsertionSet(bool atomic)
      
      Inserts all snippets accumulated since a call to beginInsertionSet. If
      the atomic parameter is true, then a failure to insert any snippet
      results in all snippets being removed; effectively, the insertion is
      all-or-nothing. If the atomic parameter is false, then snippets are
      inserted individually. This function also fills in the
      BPatchSnippetHandle structures returned by the insertSnippet calls
      comprising this insertion set. It returns true on success and false if
      there was an error inserting any snippets.
      
      Insertion sets are unnecessary when doing static binary instrumentation.
      Dyninst uses an implicit insertion set around all instrumentation to a
      static binary.
      
   .. cpp:function:: bool removeFunctionCall(BPatch_point &point)
      
      Disable the mutatee function call at the specified location. The point
      specified must be a valid call point in the image of the mutatee. The
      purpose of this routine is to permit tools to alter the semantics of a
      program by eliminating procedure calls. The mechanism to achieve the
      removal is platform dependent, but might include branching over the call
      or replacing it with NOPs. This function only removes a function call;
      any parameters to the function will still be evaluated.
      
   .. cpp:function:: bool replaceFunction (BPatch_function &old, BPatch_function &new)
      
   .. cpp:function:: bool revertReplaceFunction (BPatch_function &old)
      
      Replace all calls to user function old with calls to new. This is done
      by inserting instrumentation (specifically a BPatch_funcJumpExpr) into
      the beginning of function old such that a non-returning jump is made to
      function new. Returns true upon success, false otherwise.
      
   .. cpp:function:: bool replaceFunctionCall(BPatch_point &point, BPatch_function &newFunc)
      
      Change the function call at the specified point to the function
      indicated by newFunc. The purpose of this routine is to permit runtime
      steering tools to change the behavior of programs by replacing a call to
      one procedure by a call to another. Point must be a function call point.
      If the change was successful, the return value is true, otherwise false
      will be returned.
      
      **WARNING**\ *: Care must be used when replacing functions. In
      particular if the compiler has performed inter-procedural register
      allocation between the original caller/callee pair, the replacement may
      not be safe since the replaced function may clobber registers the
      compiler thought the callee left untouched. Also the signatures of the
      both the function being replaced and the new function must be
      compatible.*
      
   .. cpp:function:: bool wrapFunction(BPatch_function *old, BPatch_function *new, \
         Dyninst::SymtabAPI::Symbol *sym)
      
   .. cpp:function:: bool revertWrapFunction(BPatch_function *old)
      
      Replaces all calls to function old with calls to function new. Unlike
      replaceFunction above, the old function can still be reached via the
      name specified by the provided symbol sym. Function wrapping allows
      existing code to be extended by new code. Consider the following code
      that implements a fast memory allocator for a particular size of memory
      allocation, but falls back to the original memory allocator (referenced
      by origMalloc) for all others.
      
      .. code-block:: cpp
      
         void *origMalloc(unsigned long size);
         
         void *fastMalloc(unsigned long size) {
            if (size == 1024) {
               unsigned long ret = fastPool;
               fastPool += 1024;
               return ret;
            } else {
               return origMalloc(size);
            }
         }
      
      The symbol sym is provided by the user and must exist in the program;
      the easiest way to ensure it is created is to use an undefined function
      as shown above with the definition of origMalloc.
      
      The following code wraps malloc with fastMalloc, while allowing
      functions to still access the original malloc function by calling
      origMalloc. It makes use of the new convert interface described in
      Section 5..
      
      .. code-block:: cpp
      
         using namespace Dyninst;
         
         using namespace SymtabAPI;
         
         BPatch_function *malloc = appImage->findFunction(...);
         
         BPatch_function *fastMalloc = appImage->findFunction(...);
         
         Symtab *symtab = SymtabAPI::convert(fastMalloc->getModule());
         
         std::vector<Symbol *> syms;
         
         symtab->findSymbol(syms, "origMalloc",
         
         Symbol::ST_UNKNOWN, // Don’t specify type
         
         mangledName, // Look for raw symbol name
         
         false, // Not regular expression
         
         false, // Don’t check case
         
         true); // Include undefined symbols
         
         app->wrapFunction(malloc, fastMalloc, syms[0]);
      
      For a full, executable example, see Appendix A - Complete Examples.
      
   .. cpp:function:: bool replaceCode(BPatch_point *point, BPatch_snippet *snippet)
      
      This function has been removed; users interested in replacing code
      should instead use the PatchAPI code modification interface described in
      the PatchAPI manual. For information on accessing PatchAPI abstractions
      from DyninstAPI abstractions, see Section 5..
      
   .. cpp:function:: BPatch_module * loadLibrary(const char *libname, bool reload=false)
      
      For dynamic rewriting, this function loads a dynamically linked library
      into the process’s address space. For static rewriting, this function
      adds a library as a library dependency in the rewritten file. In both
      cases Dyninst creates a new BPatch_module to represent this library.
      
      The libname parameter identifies the file name of the library to be
      loaded, in the standard way that dynamically linked libraries are
      specified on the operating system on which the API is running. This
      function returns a handle to the loaded library. The reload parameter is
      ignored and only remains for backwards compatibility.
      
   .. cpp:function:: bool isStaticExecutable()
      
      This function returns true if the original file opened with this
      BPatch_addressSpace is a statically linked executable, or false
      otherwise.
      
   .. cpp:function:: processType getType()
      
      This function returns a processType that reflects whether this address
      space is a BPatch_process or a BPatch_binaryEdit.