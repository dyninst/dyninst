.. _`sec-dev:Variable.h`:

Variable.h
##########

.. cpp:namespace:: Dyninst::SymtabAPI::dev

.. cpp:class:: Variable : public Aggregate, public AnnotatableSparse

  .. cpp:function:: bool removeSymbol(Symbol *sym)
  .. cpp:function:: void setType(boost::shared_ptr<Type> type)
  .. cpp:function:: void setType(Type* t)
  .. cpp:function:: bool operator==(const Variable &v)


.. cpp:class:: localVar : public AnnotatableSparse

  .. cpp:member:: private bool locsExpanded_

      We start with an abstract location that may include "the frame pointer" as a register.
      Once a user requests the location list we concretize it and set this flag.

  .. cpp:function:: bool addLocation(const VariableLocation &location)
  .. cpp:function:: void fixupUnknown(Module *)

  .. cpp:function:: bool setType(boost::shared_ptr<Type> newType)

  .. cpp:function:: bool setType(Type *type)

      Sets the type of the variable to ``type``.

  .. cpp:function:: bool operator==(const localVar &l)

  .. cpp:function:: localVar(std::string name,  boost::shared_ptr<Type> typ, std::string fileName, int lineNum, \
                             FunctionBase *f, std::vector<VariableLocation> *locs = NULL)

  .. cpp:function:: localVar(std::string n, Type* t, std::string fn, int l, FunctionBase *f, std::vector<VariableLocation> *ls = NULL)
