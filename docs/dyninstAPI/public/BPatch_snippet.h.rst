BPatch_snippet.h
================

``BPatch_snippet``
------------------
.. cpp:namespace:: BPatch_snippet

.. cpp:class:: BPatch_snippet
   
   A snippet is an abstract representation of code to insert into a
   program. Snippets are defined by creating a new instance of the correct
   subclass of a snippet. For example, to create a snippet to call a
   function, create a new instance of the class BPatch_funcCallExpr.
   Creating a snippet does not result in code being inserted into an
   application. Code is generated when a request is made to insert a
   snippet at a specific point in a program. Sub-snippets may be shared by
   different snippets (i.e, a handle to a snippet may be passed as an
   argument to create two different snippets), but whether the generated
   code is shared (or replicated) between two snippets is implementation
   dependent.
   
   .. cpp:function:: BPatch_type *getType()
      
      Return the type of the snippet. The BPatch_type system is described in
      section 4.14.
      
   .. cpp:function:: float getCost()
      
      Returns an estimate of the number of seconds it would take to execute
      the snippet. The problems with accurately estimating the cost of
      executing code are numerous and out of the scope of this document[2]. It
      is important to realize that the returned cost value is, at best, an
      estimate.
      
      The rest of the classes are derived classes of the class BPatch_snippet.
      
   .. cpp:function:: BPatch_actualAddressExpr()
      
      This snippet results in an expression that evaluates to the actual
      address of the instrumentation. To access the original address where
      instrumentation was inserted, use BPatch_originalAddressExpr. Note that
      this actual address is highly dependent on a number of internal
      variables and has no relation to the original address.
      
   .. cpp:function:: BPatch_arithExpr(BPatch_binOp op, const BPatch_snippet &lOperand, const BPatch_snippet &rOperand)
      
      Perform the required binary operation. The available binary operators
      are:
      
      +---------------+--------------------------------------------------+
      | **Operator**  | **Description**                                  |
      +---------------+--------------------------------------------------+
      | BPatch_assign | assign the value of rOperand to lOperand         |
      +---------------+--------------------------------------------------+
      | BPatch_plus   | add lOperand and rOperand                        |
      +---------------+--------------------------------------------------+
      | BPatch_minus  | subtract rOperand from lOperand                  |
      +---------------+--------------------------------------------------+
      | BPatch_divide | divide rOperand by lOperand                      |
      +---------------+--------------------------------------------------+
      | BPatch_times  | multiply rOperand by lOperand                    |
      +---------------+--------------------------------------------------+
      | BPatch_ref    | Array reference of the form lOperand[rOperand]   |
      +---------------+--------------------------------------------------+
      | BPatch_seq    | Define a sequence of two expressions (similar to |
      |               | comma in C)                                      |
      +---------------+--------------------------------------------------+
      
      BPatch_arithExpr(BPatch_unOp, const BPatch_snippet &operand)
      
      Define a snippet consisting of a unary operator. The unary operators
      are:
      
      ============= ==========================================
      **Operator**  **Description**
      BPatch_negate Returns the negation of an integer
      BPatch_addr   Returns a pointer to a BPatch_variableExpr
      BPatch_deref  Dereferences a pointer
      ============= ==========================================
      
   .. cpp:function:: BPatch_boolExpr(BPatch_relOp op, const BPatch_snippet &lOperand, const BPatch_snippet &rOperand)
      
      Define a relational snippet. The available operators are:
      
      ============ ==========================================
      **Operator** **Function**
      BPatch_lt    Return lOperand < rOperand
      BPatch_eq    Return lOperand == rOperand
      BPatch_gt    Return lOperand > rOperand
      BPatch_le    Return lOperand <= rOperand
      BPatch_ne    Return lOperand != rOperand
      BPatch_ge    Return lOperand >= rOperand
      BPatch_and   Return lOperand && rOperand (Boolean and)
      BPatch_or    Return lOperand \|\| rOperand (Boolean or)
      ============ ==========================================
      
      The type of the returned snippet is boolean, and the operands are type
      checked.
      
   .. cpp:function:: BPatch_breakPointExpr()
      
      Define a snippet that stops a process when executed by it. The stop can
      be detected using the isStopped member function of BPatch_process, and
      the program’s execution can be resumed by calling the continueExecution
      member function of BPatch_process.
      
   .. cpp:function:: BPatch_bytesAccessedExpr()
      
      This expression returns the number of bytes accessed by a memory
      operation. For most load/store architecture machines it is a constant
      expression returning the number of bytes for the particular style of
      load or store. This snippet is only valid at a memory operation
      instrumentation point.
      
   .. cpp:function:: BPatch_constExpr(signed int value)
      
   .. cpp:function:: BPatch_constExpr(unsigned int value)
      
   .. cpp:function:: BPatch_constExpr(signed long value)
      
   .. cpp:function:: BPatch_constExpr(unsigned long value)
      
   .. cpp:function:: BPatch_constExpr(const char *value)
      
   .. cpp:function:: BPatch_constExpr(const void *value)
      
   .. cpp:function:: BPatch_constExpr(long long value)
      
      Define a constant snippet of the appropriate type. The char* form of
      the constructor creates a constant string; the null-terminated string
      beginning at the location pointed to by the parameter is copied into the
      application’s address space, and the BPatch_constExpr that is created
      refers to the location to which the string was copied.
      
   .. cpp:function:: BPatch_dynamicTargetExpr()
      
      This snippet calculates the target of a control flow instruction with a
      dynamically determined target. It can handle dynamic calls, jumps, and
      return statements.
      
   .. cpp:function:: BPatch_effectiveAddressExpr()
      
      Define an expression that contains the effective address of a memory
      operation. For a multi-word memory operation (i.e. more than the
      "natural" operation size of the machine), the effective address is the
      base address of the operation.
      
   .. cpp:function:: BPatch_funcCallExpr(const BPatch_function& func, const std::vector<BPatch_snippet*> &args)
      
      Define a call to a function. The passed function must be valid for the
      current code region. Args is a list of arguments to pass to the
      function; the maximum number of arguments varies by platform and is
      summarized below. If type checking is enabled, the types of the passed
      arguments are checked against the function to be called. Availability of
      type checking depends on the source language of the application and
      program being compiled for debugging.
      
      ============ ===============================
      **Platform** **Maximum number of arguments**
      AMD64/EMT-64 No limit
      IA-32        No limit
      POWER        8 arguments
      ============ ===============================
      
      BPatch_funcJumpExpr (const BPatch_function &func)
      
      This snippet has been removed; use BPatch_addressSpace::wrapFunction
      instead.
      
   .. cpp:function:: BPatch_ifExpr(const BPatch_boolExpr &conditional, const BPatch_snippet &tClause, const BPatch_snippet &fClause)
      
   .. cpp:function:: BPatch_ifExpr(const BPatch_boolExpr &conditional, const BPatch_snippet &tClause)
      
      This constructor creates an if statement. The first argument,
      conditional, should be a Boolean expression that will be evaluated to
      decide which clause should be executed. The second argument, tClause, is
      the snippet to execute if the conditional evaluates to true. The third
      argument, fClause, is the snippet to execute if the conditional
      evaluates to false. This third argument is optional. Else-if statements,
      can be constructed by making the fClause of an if statement another if
      statement.
      
   .. cpp:function:: BPatch_insnExpr(BPatch_instruction *insn)
      
      implemented on x86-64
      
      This constructor creates a snippet that allows the user to mimic the
      effect of an existing instruction. In effect, the snippet "wraps" the
      instruction and provides a handle to particular components of
      instruction behavior. This is currently implemented for memory
      operations, and provides two override methods: overrideLoadAddress and
      overrideStoreAddress. Both methods take a BPatch_snippet as an argument.
      Unlike other snippets, this snippet should be installed via a call to
      BPatch_process­::replaceCode (to replace the original instruction). For
      example:
      
      .. code-block:: cpp
      
         // Assume that access is of type BPatch_memoryAccess, as
         // provided by a call to BPatch_point->getMemoryAccess. A
         // BPatch_memoryAccess is a child of BPatch_instruction, and
         // is a valid source of a BPatch_insnExpr.
         BPatch_insnExpr insn(access);
      
         // This example will modify a store by increasing the target
         // address by 16.
         BPatch_arithExpr newStoreAddr(BPatch_plus,
         BPatch_effectiveAddressExpr(),
         BPatch_constExpr(16));
      
         // now override the original store address
         insn.overrideStoreAddress(newStoreAddr)
      
         // now replace the original instruction with the new one.
         // Point is a BPatch_point corresponding to the desired location, and
         // process is a BPatch_process.
         process.replaceCode(point, insn);
      
   .. cpp:function:: BPatch_nullExpr()
      
      Define a null snippet. This snippet contains no executable statements.
      
   .. cpp:function:: BPatch_originalAddressExpr()
      
      This snippet results in an expression that evaluates to the original
      address of the point where the snippet was inserted. To access the
      actual address where instrumentation is executed, use
      BPatch_actualAddressExpr.
      
   .. cpp:function:: BPatch_paramExpr(int paramNum)
      
      This constructor creates an expression whose value is a parameter being
      passed to a function. ParamNum specifies the number of the parameter to
      return, starting at 0. Since the contents of parameters may change
      during subroutine execution, this snippet type is only valid at points
      that are entries to subroutines, or when inserted at a call point with
      the when parameter set to BPatch_callBefore.
      
   .. cpp:function:: BPatch_registerExpr(BPatch_register reg)
      
   .. cpp:function:: BPatch_registerExpr(Dyninst::MachRegister reg)
      
      This snippet results in an expression whose value is the value in the
      register at the point of instrumentation.
      
   .. cpp:function:: BPatch_retExpr()
      
      This snippet results in an expression that evaluates to the return value
      of a subroutine. This snippet type is only valid at BPatch_exit points,
      or at a call point with the when parameter set to BPatch_callAfter.
      
   .. cpp:function:: BPatch_scrambleRegistersExpr()
      
      This snippet sets all General Purpose Registers to the flag value.
      
   .. cpp:function:: BPatch_sequence(const std::vector<BPatch_snippet*> &items)
      
      Define a sequence of snippets. The passed snippets will be executed in
      the order in which they appear in items.
      
   .. cpp:function:: BPatch_shadowExpr(bool entry, \
         const BPatchStopThreadCallback &cb, \
         const BPatch_snippet &calculation, \
         bool useCache = false, \
         BPatch_stInterpret interp = BPatch_noInterp)
      
      This snippet creates a shadow copy of the snippet BPatch_stopThreadExpr.
      
   .. cpp:function:: BPatch_stopThreadExpr(const BPatchStopThreadCallback &cb, \
         const BPatch_snippet &calculation, \
         bool useCache = false, \
         BPatch_stInterpret interp = BPatch_noInterp)
      
      This snippet stops the thread that executes it. It evaluates a
      calculation snippet and triggers a callback to the user program with the
      result of the calculation and a pointer to the BPatch_point at which the
      snippet was inserted.
      
   .. cpp:function:: BPatch_threadIndexExpr()
      
      This snippet returns an integer expression that contains the thread
      index of the thread that is executing this snippet. The thread index is
      the same value that is returned on the mutator side by
      BPatch_thread::getBPatchID.
      
   .. cpp:function:: BPatch_tidExpr(BPatch_process *proc)
      
      This snippet results in an integer expression that contains the tid of
      the thread that is **executing** this snippet. This can be used to
      record the threadId, or to filter instrumentation so that it only
      executes for a specific thread.
      
   .. cpp:function:: BPatch_variableExpr(char *in_name, \
         BPatch_addressSpace *in_addSpace, \
         AddressSpace *as, \
         AstNodePtr ast_wrapper_, \
         BPatch_type *type, void* in_address)
      
   .. cpp:function:: BPatch_variableExpr(BPatch_addressSpace *in_addSpace, \
         AddressSpace *as,\
         void *in_address,\
         int in_register,\
         BPatch_type *type,\
         BPatch_storageClass storage = BPatch_storageAddr, \
         BPatch_point *scp = NULL)
      
   .. cpp:function:: BPatch_variableExpr(BPatch_addressSpace *in_addSpace, \
         AddressSpace *as, \
         BPatch_localVar *lv, \
         BPatch_type *type, \
         BPatch_point *scp)
      
   .. cpp:function:: BPatch_variableExpr(BPatch_addressSpace *in_addSpace, \
         AddressSpace *ll_addSpace, \
         int_variable *iv, \
         BPatch_type *type)
      
      Define a variable snippet of the appropriate type. The first constructor
      is used to get function pointers; the second is used to get forked
      copies of variable expression, used by malloc; the third is used for
      local variables; and the last is used by
      BPatch_addressSpace::findOrCreateVariable().
      
   .. cpp:function:: BPatch_whileExpr(const BPatch_snippet &condition, const BPatch_snippet &body)
      
      This constructor creates a while statement. The first argument,
      condition, should be a Boolean expression that will be evaluated to
      decide whether body should be executed. The second argument, body, is
      the snippet to execute if the condition evaluates to true.

``BPatch_variableExpr``
-----------------------
.. cpp:namespace:: BPatch_variableExpr

.. cpp:class:: BPatch_variableExpr
   
   The **BPatch_variableExpr** class is another class derived from
   BPatch_snippet. It represents a variable or area of memory in a
   process’s address space. A BPatch_variableExpr can be obtained from a
   BPatch_process using the malloc member function, or from a BPatch_image
   using the findVariable member function.
   
   Some BPatch_variableExpr have an associated BPatch_type, which can be
   accessed by functions inherited from BPatch_snippet. BPatch_variableExpr
   objects will have an associated BPatch_type if they originate from
   binaries with sufficient debug information that describes types, or if
   they were provided with a BPatch_type when created by Dyninst.
   
   **BPatch_variableExpr** provides several member functions not provided
   by other types of snippets:
   
   .. cpp:function:: void readValue(void *dst)
      
   .. cpp:function:: void readValue(void *dst, int size)
      
      Read the value of the variable in an application’s address space that is
      represented by this BPatch_variableExpr. The dst parameter is assumed to
      point to a buffer large enough to hold a value of the variable’s type.
      If the size parameter is supplied, then the number of bytes it specifies
      will be read. For the first version of this method, if the size of the
      variable is unknown (i.e., no type information), no data is copied and
      the method returns false.
      
   .. cpp:function:: void writeValue(void *src)
      
   .. cpp:function:: void writeValue(void *src, int size)
      
      Change the value of the variable in an application’s address space that
      is represented by this BPatch_variableExpr. The src parameter should
      point to a value of the variable’s type. If the size parameter is
      supplied, then the number of bytes it specifies will be written. For the
      first version of this method, if the size of the variable is unknown
      (i.e., no type information), no data is copied and the method returns
      false.
      
   .. cpp:function:: void *getBaseAddr()
      
      Return the base address of the variable. This is designed to let users
      who wish to access elements of arrays or fields in structures do so. It
      can also be used to obtain the address of a variable to pass a point to
      that variable as a parameter to a procedure call. It is similar to the
      ampersand (&) operator in C.
      
   .. cpp:function:: std::vector<BPatch_variableExpr *> *getComponents()
      
      Return a pointer to a vector containing the components of a struct or
      union. Each element of the vector is one field of the composite type,
      and contains a variable expression for accessing it.