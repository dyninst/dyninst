.. _`sec:BPatch_snippet.h`:

BPatch_snippet.h
################

.. cpp:class:: BPatch_snippet
   
  **An abstract representation of code to insert into a program**

  Snippets are defined by creating a new instance of the correct
  subclass of a snippet. For example, to create a snippet to call a
  function, create a new instance of the class BPatch_funcCallExpr.
  Creating a snippet does not result in code being inserted into an
  application. Code is generated when a request is made to insert a
  snippet at a specific point in a program. Sub-snippets may be shared by
  different snippets (i.e, a handle to a snippet may be passed as an
  argument to create two different snippets), but whether the generated
  code is shared (or replicated) between two snippets is implementation
  dependent.

  .. cpp:member:: AstNodePtr ast_wrapper

  .. cpp:function:: BPatch_snippet()

    Returns a :cpp:func:`nullNode()`.

  .. cpp:function:: BPatch_snippet(const AstNodePtr& ast)

  .. cpp:function:: virtual ~BPatch_snippet()

  .. cpp:function:: bool is_trivial()

    Checks if snippet operation failed.

  .. cpp:function:: bool checkTypesAtPoint(BPatch_point* p) const

    Type checking for inserting a particular snippet at a the point ``p``.

    Currently, check return exprs against the existence of return values.
    Called at insertion time, but can be used to check in advance.


.. cpp:class:: BPatch_arithExpr: public BPatch_snippet

  .. cpp:function:: BPatch_arithExpr(BPatch_binOp op, const BPatch_snippet &lOperand, const BPatch_snippet &rOperand)

    Creates a binary arithmetic operation with operation ``op`` and operands ``lOperand`` and ``rOperand``.

  .. cpp:function:: BPatch_arithExpr(BPatch_unOp op, const BPatch_snippet &lOperand)

    Creates a unary arithmetic operation with operation ``op`` and operand ``lOperand``.


.. cpp:class:: BPatch_boolExpr : public BPatch_snippet

  **A boolean operation**

  .. cpp:function:: BPatch_boolExpr(BPatch_relOp op, const BPatch_snippet &lOperand, const BPatch_snippet &rOperand)

    Constructs a snippet representing a boolean expression with operation ``op`` and operands ``lOperand`` and ``rOperand``.


.. cpp:class:: BPatch_constExpr : public BPatch_snippet

  .. cpp:function:: BPatch_constExpr(signed int value)

    A (signed int) value

  .. cpp:function:: BPatch_constExpr(unsigned int value)

    An (unsigned int) value

  .. cpp:function:: BPatch_constExpr(signed long value)

    A (signed long) value

  .. cpp:function:: BPatch_constExpr(unsigned long value)

    An (unsigned long) value

  .. cpp:function:: BPatch_constExpr(unsigned long long value)

    An (unsigned long long) value

  .. cpp:function:: BPatch_constExpr(const char *value)

    A C-style string

  .. cpp:function:: BPatch_constExpr(const void *value)

    An opaque pointer

  .. cpp:function:: BPatch_constExpr(long long value)

    A (long long) value

  .. cpp:function:: BPatch_constExpr()

    Creates an empty snippet.


.. cpp:class:: BPatch_whileExpr : public BPatch_snippet

  **A while loop**

  .. cpp:function:: BPatch_whileExpr(const BPatch_snippet &condition, const BPatch_snippet &body)

    This constructor creates a while statement. The first argument,
    condition, should be a Boolean expression that will be evaluated to
    decide whether body should be executed. The second argument, body, is
    the snippet to execute if the condition evaluates to true.


.. cpp:class:: BPatch_funcCallExpr : public BPatch_snippet

  **A function call**

  .. cpp:function:: BPatch_funcCallExpr(const BPatch_function& func, const BPatch_Vector<BPatch_snippet*> &args)

    Define a call to a function. The passed function must be valid for the
    current code region. Args is a list of arguments to pass to the
    function; the maximum number of arguments varies by platform and is
    summarized below. If type checking is enabled, the types of the passed
    arguments are checked against the function to be called. Availability of
    type checking depends on the source language of the application and
    program being compiled for debugging.

    .. note:: The number of arguments is limited to 8 on PowerPC and ARM.


.. cpp:class:: BPatch_ifExpr : public BPatch_snippet

  .. cpp:function:: BPatch_ifExpr(const BPatch_boolExpr &conditional, const BPatch_snippet &tClause)

    Creates a conditional expression "if <conditional> tClause"

    This constructor creates an if statement. The first argument,
    conditional, should be a Boolean expression that will be evaluated to
    decide which clause should be executed. The second argument, tClause, is
    the snippet to execute if the conditional evaluates to true. The third
    argument, fClause, is the snippet to execute if the conditional
    evaluates to false. This third argument is optional. Else-if statements,
    can be constructed by making the fClause of an if statement another if
    statement.

  .. cpp:function:: BPatch_ifExpr(const BPatch_boolExpr &conditional, const BPatch_snippet &tClause, const BPatch_snippet &fClause)

    Creates a conditional expression   "if <conditional> tClause else fClause"

    This constructor creates an if statement. The first argument,
    conditional, should be a Boolean expression that will be evaluated to
    decide which clause should be executed. The second argument, tClause, is
    the snippet to execute if the conditional evaluates to true. The third
    argument, fClause, is the snippet to execute if the conditional
    evaluates to false. This third argument is optional. Else-if statements,
    can be constructed by making the fClause of an if statement another if
    statement.


.. cpp:class:: BPatch_nullExpr : public BPatch_snippet

  .. cpp:function:: BPatch_nullExpr()

    Define a null snippet. This snippet contains no executable statements, and can be used as a placeholder.


.. cpp:class:: BPatch_paramExpr : public BPatch_snippet

  **A parameter of a function**

  .. cpp:function:: BPatch_paramExpr(int n, BPatch_ploc loc=BPatch_ploc_guess)

    ``n`` is the index of the parameter that should be retrieved. ``loc`` indicates whether
    the parameter lookup will be added at the call, at the function's entry point, or
    whether Dyninst should guess based on the instPoint type, which is error-prone and deprecated.

    Since the contents of parameters may change
    during subroutine execution, this snippet type is only valid at points
    that are entries to subroutines, or when inserted at a call point with
    the when parameter set to :cpp:enumerator:`BPatch_callBefore`.

.. cpp:class:: BPatch_retExpr : public BPatch_snippet

  .. cpp:function:: BPatch_retExpr()

    This snippet results in an expression that evaluates to the return value
    of a subroutine. This snippet type is only valid at BPatch_exit points,
    or at a call point with the when parameter set to BPatch_callAfter.


.. cpp:class:: BPatch_retAddrExpr : public BPatch_snippet

  .. cpp:function:: BPatch_retAddrExpr()

    Represents the return address from the function in which the snippet is inserted


.. cpp:class:: BPatch_registerExpr : public BPatch_snippet

  .. cpp:function:: BPatch_registerExpr(BPatch_register reg)

  .. cpp:function:: BPatch_registerExpr(Dyninst::MachRegister reg)

    This snippet results in an expression whose value is the value in the
    register at the point of instrumentation.


.. cpp:class:: BPatch_sequence : public BPatch_snippet

  .. cpp:function:: BPatch_sequence(const BPatch_Vector<BPatch_snippet *> &items)

    Define a sequence of snippets. The passed snippets will be executed in
    the order in which they appear in items.


.. cpp:class:: BPatch_variableExpr : public BPatch_snippet
   
  **A variable or area of memory in a process’s address space**

  A BPatch_variableExpr can be obtained from a
  BPatch_process using the malloc member function, or from a BPatch_image
  using the findVariable member function.
  Some BPatch_variableExpr have an associated BPatch_type, which can be
  accessed by functions inherited from BPatch_snippet. BPatch_variableExpr
  objects will have an associated BPatch_type if they originate from
  binaries with sufficient debug information that describes types, or if
  they were provided with a BPatch_type when created by Dyninst.

  .. cpp:function:: static BPatch_variableExpr* makeVariableExpr(BPatch_addressSpace* in_addSpace,\
                                                                 AddressSpace* in_llAddSpace,\
                                                                 std::string name, void* offset,\
                                                                 BPatch_type* type)

  .. cpp:function:: unsigned int getSize() const

    Returns the size (in bytes) of this variable

  .. cpp:function:: const BPatch_type * getType()

    Returns the type of this variable

  .. cpp:function:: void readValue(void *dst)

    Read the value of the variable in an application’s address space that is
    represented by this BPatch_variableExpr. The dst parameter is assumed to
    point to a buffer large enough to hold a value of the variable’s type.
    If the size of the
    variable is unknown (i.e., no type information), no data is copied and
    the method returns false.

  .. cpp:function:: void readValue(void *dst, int size)

    Read the value of the variable in an application’s address space that is
    represented by this BPatch_variableExpr. The dst parameter is assumed to
    point to a buffer large enough to hold a value of the variable’s type.
    If the size parameter is supplied, then the number of bytes it specifies
    will be read.

  .. cpp:function:: void writeValue(void *src)

    Change the value of the variable in an application’s address space that
    is represented by this BPatch_variableExpr. The src parameter should
    point to a value of the variable’s type. If the size of the variable is unknown
    (i.e., no type information), no data is copied and the method returns
    false.

  .. cpp:function:: void writeValue(void *src, int size)

    Change the value of the variable in an application’s address space that
    is represented by this BPatch_variableExpr. The src parameter should
    point to a value of the variable’s type. If the size parameter is
    supplied, then the number of bytes it specifies will be written.

  .. cpp:function:: const char* getName()

    Returns the symbol table name for this variable

  .. cpp:function:: void *getBaseAddr()

    Return the base address of the variable. This is designed to let users
    who wish to access elements of arrays or fields in structures do so. It
    can also be used to obtain the address of a variable to pass a point to
    that variable as a parameter to a procedure call. It is similar to the
    ampersand (&) operator in C.

  .. cpp:function:: BPatch_Vector<BPatch_variableExpr*>* getComponents()

    Return a pointer to a vector containing the components of a struct or
    union. Each element of the vector is one field of the composite type,
    and contains a variable expression for accessing it.


.. cpp:class:: BPatch_breakPointExpr : public BPatch_snippet

  **A break point in the target process**

  .. cpp:function:: BPatch_breakPointExpr()

    Define a snippet that stops a process when executed by it. The stop can
    be detected using the isStopped member function of BPatch_process, and
    the program’s execution can be resumed by calling the continueExecution
    member function of BPatch_process.


.. cpp:class:: BPatch_effectiveAddressExpr : public BPatch_snippet

  **An effective address**

  .. cpp:function:: BPatch_effectiveAddressExpr(int _which = 0, int size = 8)

    Define an expression that contains the effective address of a memory
    operation. For a multi-word memory operation (i.e. more than the
    "natural" operation size of the machine), the effective address is the
    base address of the operation.


.. cpp:class:: BPatch_bytesAccessedExpr : public BPatch_snippet

  .. cpp:function:: BPatch_bytesAccessedExpr(int _which = 0)

    This expression returns the number of bytes accessed by a memory
    operation. For most load/store architecture machines it is a constant
    expression returning the number of bytes for the particular style of
    load or store. This snippet is only valid at a memory operation
    instrumentation point.


.. cpp:class:: BPatch_ifMachineConditionExpr : public BPatch_snippet

  .. cpp:function:: BPatch_ifMachineConditionExpr(const BPatch_snippet &tClause)


.. cpp:class:: BPatch_threadIndexExpr : public BPatch_snippet

  .. cpp:function:: BPatch_threadIndexExpr()

    This snippet returns an integer expression that contains the thread
    index of the thread that is executing this snippet. The thread index is
    the same value that is returned on the mutator side by
    BPatch_thread::getBPatchID.


.. cpp:class:: BPatch_tidExpr : public BPatch_snippet

  .. cpp:function:: BPatch_tidExpr(BPatch_process *proc)

    This snippet results in an integer expression that contains the tid of
    the thread that is *executing* this snippet. This can be used to
    record the threadId, or to filter instrumentation so that it only
    executes for a specific thread.


.. cpp:class:: BPatch_shadowExpr : public BPatch_snippet

  .. cpp:function:: BPatch_shadowExpr(bool entry, const BPatchStopThreadCallback &cb,\
                                      const BPatch_snippet &calculation, bool useCache = false,\
                                      BPatch_stInterpret interp = BPatch_noInterp)

    This snippet type stops the thread that executes it.

    It evaluates a calculation snippet and triggers a callback to the  user program with the
    result of the calculation and a pointer to  the BPatch_point at which the snippet was inserted.



.. cpp:class:: BPatch_stopThreadExpr : public BPatch_snippet

  .. cpp:function:: BPatch_stopThreadExpr(const BPatchStopThreadCallback &cb,\
                                          const BPatch_snippet &calculation, bool useCache = false,\
                                          BPatch_stInterpret interp = BPatch_noInterp)

    This snippet stops the thread that executes it. It evaluates a
    calculation snippet and triggers a callback to the user program with the
    result of the calculation and a pointer to the BPatch_point at which the
    snippet was inserted.


.. cpp:class:: BPatch_originalAddressExpr : public BPatch_snippet

  .. cpp:function:: BPatch_originalAddressExpr()

    This snippet results in an expression that evaluates to the original
    address of the point where the snippet was inserted. To access the
    actual address where instrumentation is executed, use
    BPatch_actualAddressExpr.


.. cpp:class:: BPatch_actualAddressExpr : public BPatch_snippet

  .. cpp:function:: BPatch_actualAddressExpr()

    This snippet results in an expression that evaluates to the actual
    address of the instrumentation. To access the original address where
    instrumentation was inserted, use BPatch_originalAddressExpr. Note that
    this actual address is highly dependent on a number of internal
    variables and has no relation to the original address.


.. cpp:class:: BPatch_dynamicTargetExpr : public BPatch_snippet

  .. cpp:function:: BPatch_dynamicTargetExpr()

    This snippet calculates the target of a control flow instruction with a
    dynamically determined target. It can handle dynamic calls, jumps, and
    return statements.


.. cpp:class:: BPatch_scrambleRegistersExpr : public BPatch_snippet

  .. cpp:function:: BPatch_scrambleRegistersExpr()

    BPatch_scrambleRegistersExpr Set all GPR to flag value.

.. cpp:enum:: BPatch_relOp

  .. cpp:enumerator:: BPatch_lt

    Return lOperand < rOperand

  .. cpp:enumerator:: BPatch_eq

    Return lOperand == rOperand

  .. cpp:enumerator:: BPatch_gt

    Return lOperand > rOperand

  .. cpp:enumerator:: BPatch_le

    Return lOperand <= rOperand

  .. cpp:enumerator:: BPatch_ne

    Return lOperand != rOperand

  .. cpp:enumerator:: BPatch_ge

    Return lOperand >= rOperand

  .. cpp:enumerator:: BPatch_and

    Return lOperand && rOperand (Boolean and)

  .. cpp:enumerator:: BPatch_or

    Return lOperand || rOperand (Boolean or)


.. cpp:enum:: BPatch_binOp

  .. cpp:enumerator:: BPatch_assign

    assign the value of rOperand to lOperand

  .. cpp:enumerator:: BPatch_plus

    add lOperand and rOperand

  .. cpp:enumerator:: BPatch_minus

    subtract rOperand from lOperand

  .. cpp:enumerator:: BPatch_divide

    divide rOperand by lOperand

  .. cpp:enumerator:: BPatch_times

    multiply rOperand by lOperand

  .. cpp:enumerator:: BPatch_mod

  .. cpp:enumerator:: BPatch_ref

    Array reference of the form lOperand[rOperand]

  .. cpp:enumerator:: BPatch_fieldref

  .. cpp:enumerator:: BPatch_seq

    Define a sequence of two expressions (similar to comma in C)

  .. cpp:enumerator:: BPatch_xor
  .. cpp:enumerator:: BPatch_bit_and
  .. cpp:enumerator:: BPatch_bit_or
  .. cpp:enumerator:: BPatch_bit_xor
  .. cpp:enumerator:: BPatch_left_shift
  .. cpp:enumerator:: BPatch_right_shift


.. cpp:enum:: BPatch_unOP

  .. cpp:enumerator:: BPatch_negate

    Returns the negation of an integer

  .. cpp:enumerator:: BPatch_addr

    Returns a pointer to a BPatch_variableExpr

  .. cpp:enumerator:: BPatch_deref

    Dereferences a pointer


.. cpp:enum:: BPatch_stInterpret

  .. cpp:enumerator:: BPatch_noInterp
  .. cpp:enumerator:: BPatch_interpAsTarget
  .. cpp:enumerator:: BPatch_interpAsReturnAddr


Notes
*****

The following aliases are provided for backwards compatibility. Do not use.

.. code:: cpp

  #define BPatch_addr BPatch_address
