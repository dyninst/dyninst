.. _`sec:SymEval.h`:

SymEval.h
#########

.. cpp:namespace:: Dyninst::DataflowAPI

.. cpp:class:: SymEval

  **Symbolic evaluation of instructions**

  .. cpp:type:: std::map<Assignment::Ptr, AST::Ptr, AssignmentPtrValueComp> Result_t

    This data type represents the results of symbolic expansion of a slice.
    Each assignment in the slice has a corresponding AST.

  .. cpp:function:: static std::pair<AST::Ptr, bool> expand(const Assignment::Ptr &assignment, bool applyVisitors = true)

    Expands a single assignment given by ``assignment``.

    Returns a tuple where first element is the AST after expansion and the second element indicates if expansion
    succeeded or not. ``applyVisitors`` specifies whether or not to perform stack analysis to precisely track
    stack variables.

  .. cpp:function:: static bool expand(Result_t &res, std::set<InstructionPtr> &failedInsns, bool applyVisitors = true)

    Expands a set of assignment prepared in ``res``.

    The corresponding ASTs are written back into ``res`` and all instructions
    that failed during expansion are inserted into ``failedInsns``.
    ``applyVisitors`` specifies whether or not to perform stack analysis to
    precisely track stack variables.

    The assignments are assumed to be prepped in the input; whatever they point to is discarded.

    Returns ``true`` if all assignments in ``res`` are successfully expanded.

  .. cpp:enum:: Retval_t

     .. cpp:enumerator:: FAILED
     .. cpp:enumerator:: WIDEN_NODE
     .. cpp:enumerator:: FAILED_TRANSLATION
     .. cpp:enumerator:: SKIPPED_INPUT
     .. cpp:enumerator:: SUCCESS

  .. cpp:function:: static Retval_t expand(Dyninst::Graph::Ptr slice, DataflowAPI::Result_t &res)

    Expands a slice and returns an AST for each assignment in it.

    .. Note:: This function performs substitution of ASTs.

    An AST is used to represent the symbolic expressions of an assignment. A
    symbolic expression AST contains internal node type ``RoseAST``, which
    abstracts the operations performed with its child nodes, and two leaf
    node types: ``VariableAST`` and ``ConstantAST``.
