.. _`sec:BoundFactData.h`:

BoundFactData.h
###############

.. cpp:struct:: StridedInterval

  .. cpp:member:: static const int64_t minValue = LLONG_MIN
  .. cpp:member:: static const int64_t maxValue = LLONG_MAX
  .. cpp:member:: static const StridedInterval top
  .. cpp:member:: static const StridedInterval bottom

  .. cpp:member:: int64_t stride

      | stride < 0: bottom (empty set)
      | stride = 0: represents a constant
      | stride > 0: represents an interval

  .. cpp:member:: int64_t low
  .. cpp:member:: int64_t high

  .. cpp:function:: StridedInterval()

      Bottom: empty set

  .. cpp:function:: StridedInterval(int64_t x)

      Construct a constant

  .. cpp:function:: StridedInterval(unsigned s, int64_t l, int64_t h)

      Construct an interval

  .. cpp:function:: StridedInterval(const StridedInterval &si)
  .. cpp:function:: void Join(const StridedInterval &rhs)
  .. cpp:function:: void Neg()
  .. cpp:function:: void Not()
  .. cpp:function:: void Add(const StridedInterval &rhs)
  .. cpp:function:: void Sub(const StridedInterval &rhs)
  .. cpp:function:: void And(const StridedInterval &rhs)
  .. cpp:function:: void Or(const StridedInterval &rhs)
  .. cpp:function:: void Xor(const StridedInterval &rhs)
  .. cpp:function:: void Mul(const StridedInterval &rhs)
  .. cpp:function:: void Div(const StridedInterval &rhs)
  .. cpp:function:: void ShiftLeft(const StridedInterval &rhs)
  .. cpp:function:: void ShiftRight(const StridedInterval &rhs)
  .. cpp:function:: std::string format()
  .. cpp:function:: void Print()
  .. cpp:function:: StridedInterval & operator = (const StridedInterval &rhs)
  .. cpp:function:: bool operator == (const StridedInterval &rhs) const
  .. cpp:function:: bool operator != (const StridedInterval &rhs) const
  .. cpp:function:: bool operator < (const StridedInterval &rhs) const
  .. cpp:function:: void Intersect(StridedInterval &rhs)
  .. cpp:function:: void DeleteElement(int64_t val)
  .. cpp:function:: uint64_t size() const
  .. cpp:function:: bool IsConst(int64_t v) const
  .. cpp:function:: bool IsConst() const


.. cpp:struct:: BoundFact

  .. cpp:type:: map<AST::Ptr, StridedInterval*> FactType

  .. cpp:type:: std::map<AST::Ptr, AST::Ptr> AliasMap

      The left hand side represents an abstract location at the current address
      and the right hand side represents an AST of input absloc locations.
      eax at the current location can be different from eax at the input absloc location.
      Register abstract location with address 0 represents an absloc at the current address.
      Register abstract location with address 1 represents an input absloc.

  .. cpp:member:: vector<Relation*> relation
  .. cpp:member:: AliasMap aliasMap
  .. cpp:member:: FactType fact
  .. cpp:member:: FlagPredicate pred
  .. cpp:member:: StackTop stackTop

  .. cpp:function:: BoundFact()
  .. cpp:function:: BoundFact(const BoundFact& bf)
  .. cpp:function:: BoundFact& operator = (const BoundFact &bf)
  .. cpp:function:: StridedInterval* GetBound(const AST::Ptr ast)
  .. cpp:function:: StridedInterval* GetBound(const AST* ast)
  .. cpp:function:: AST::Ptr GetAlias(const AST::Ptr ast)
  .. cpp:function:: void Meet(BoundFact &bf)
  .. cpp:function:: bool ConditionalJumpBound(InstructionAPI::Instruction insn, EdgeTypeEnum type)
  .. cpp:function:: void SetPredicate(Assignment::Ptr assign, std::pair<AST::Ptr, bool> expand)
  .. cpp:function:: void GenFact(const AST::Ptr ast, StridedInterval* bv, bool isConditionalJump)
  .. cpp:function:: void KillFact(const AST::Ptr ast, bool isConditionalJump)
  .. cpp:function:: void SetToBottom()
  .. cpp:function:: void Print()
  .. cpp:function:: void AdjustPredicate(AST::Ptr out, AST::Ptr in)
  .. cpp:function:: void IntersectInterval(const AST::Ptr ast, StridedInterval si)
  .. cpp:function:: void DeleteElementFromInterval(const AST::Ptr ast, int64_t val)
  .. cpp:function:: void InsertRelation(AST::Ptr left, AST::Ptr right, RelationType)
  .. cpp:function:: void TrackAlias(AST::Ptr expr, AST::Ptr outAST, bool findBound)
  .. cpp:function:: StridedInterval *ApplyRelations(AST::Ptr outAST)
  .. cpp:function:: StridedInterval *ApplyRelations2(AST::Ptr outAST)
  .. cpp:function:: void PushAConst(int64_t value)
  .. cpp:function:: bool PopAConst(AST::Ptr ast)
  .. cpp:function:: void SwapFact(AST::Ptr a, AST::Ptr b)


.. cpp:struct:: StackTop

  .. cpp:member:: int64_t value
  .. cpp:member:: bool valid

  .. cpp:function:: StackTop()
  .. cpp:function:: StackTop(int64_t v)
  .. cpp:function:: bool operator != (const StackTop &st) const
  .. cpp:function:: StackTop& operator = (const StackTop &st)

.. cpp:struct:: BoundFact::FlagPredicate

  .. cpp:member:: bool valid
  .. cpp:member:: entryID id
  .. cpp:member:: AST::Ptr e1
  .. cpp:member:: AST::Ptr e2

  .. cpp:function:: FlagPredicate()
  .. cpp:function:: bool operator!= (const FlagPredicate& fp) const
  .. cpp:function:: FlagPredicate& operator= (const FlagPredicate &fp)


.. cpp:enum:: BoundFact::RelationType

    Sometimes the bound of a jump table index are derived from
    the difference between two values. In this case, it is useful
    to know that whether there is a certain relation between the two values.

  .. cpp:enumerator:: Equal
  .. cpp:enumerator:: NotEqual
  .. cpp:enumerator:: UnsignedLessThan
  .. cpp:enumerator:: UnsignedLargerThan
  .. cpp:enumerator:: UnsignedLessThanOrEqual
  .. cpp:enumerator:: UnsignedLargerThanOrEqual
  .. cpp:enumerator:: SignedLessThan
  .. cpp:enumerator:: SignedLargerThan
  .. cpp:enumerator:: SignedLessThanOrEqual
  .. cpp:enumerator:: SignedLargerThanOrEqual

.. cpp:struct:: BoundFact::Relation

  .. cpp:member:: AST::Ptr left
  .. cpp:member:: AST::Ptr right
  .. cpp:member:: RelationType type

  .. cpp:function:: Relation(AST::Ptr l, AST::Ptr r, RelationType t)
  .. cpp:function:: bool operator != (const Relation &rhs) const
  .. cpp:function:: Relation& operator = (const Relation &rhs)
  .. cpp:function:: Relation(const Relation &r)

.. cpp:type:: map<Node::Ptr, BoundFact*> BoundFactsType
