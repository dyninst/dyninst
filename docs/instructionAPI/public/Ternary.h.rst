.. _`sec:Ternary.h`:

Ternary.h
#########

.. cpp:namespace:: Dyninst::InstructionAPI

.. cpp:class:: TernaryAST : public Expression

  **The value of a ternary assignment**

  .. cpp:type:: boost::shared_ptr<TernaryAST> Ptr

  .. cpp:member:: Expression::Ptr cond

      The condition part of the ternary expression.

  .. cpp:member:: Expression::Ptr first

      The true part of the ternary expression.

  .. cpp:member:: Expression::Ptr second

      The false part of the ternary expression.

  .. cpp:member:: Result_Type result_type

      The type of the result of the ternary expression.

  .. cpp:function:: TernaryAST(Expression::Ptr cond , Expression::Ptr first , Expression::Ptr second, Result_Type result_type)

  .. cpp:function:: virtual void getChildren(vector<InstructionAST::Ptr>& children) const

      Appends this node's children to ``children`` as an :cpp:class::`InstructionAST`.

      By definition, a ``TernaryAST`` has three children: the condition, the first child, and the second child.

  .. cpp:function:: virtual void getChildren(vector<Expression::Ptr>& children) const

      Appends this node's children to ``children`` as an :cpp:class::`Expression`.

      By definition, a ``TernaryAST`` has three children: the condition, the first child, and the second child.

  .. cpp:function:: virtual void getUses(set<InstructionAST::Ptr>& uses)

      Appends the use set to ``uses``.

      The set of used registers is the union of the sets used by the children.

  .. cpp:function:: virtual bool isUsed(InstructionAST::Ptr i) const

      Does nothing.

  .. cpp:function:: virtual std::string format(Architecture, formatStyle how = defaultStyle) const

      Forwards to :cpp:func:`format`.

  .. cpp:function:: virtual std::string format(formatStyle how = defaultStyle) const

      Returns a string representation of this expression using the style ``formatStyle``.

  .. cpp:function:: bool operator<(const TernaryAST& rhs) const

      Returns ``false``.

  .. cpp:function:: virtual void apply(Visitor* v)

      Does nothing.

  .. cpp:function:: virtual bool bind(Expression* e, const Result& val)

      Does nothing.

  .. cpp:function:: virtual bool isStrictEqual(const InstructionAST& rhs) const

      Returns ``false``.
