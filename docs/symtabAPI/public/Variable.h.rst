.. _`sec:Variable.h`:

Variable.h
##########

.. cpp:namespace:: Dyninst::SymtabAPI

.. cpp:class:: Variable : public Aggregate, public AnnotatableSparse

  **A collection of symbols that have the same address and represent data**


  .. cpp:function:: Variable()
  .. cpp:function:: boost::shared_ptr<Type> getType(Type::do_share_t)

      Returns the type of this variable.

  .. cpp:function:: Type* getType()

      Returns the type of this variable.

  .. cpp:function:: std::ostream &operator<<(std::ostream &os, Variable const& v)

      Writes a string representation of ``v`` into the stream ``os``.


.. cpp:class:: localVar : public AnnotatableSparse

  **A local variable or parameter of a function**

  .. cpp:function:: localVar()
  .. cpp:function:: std::string &getName()

      Returns the name of the local variable or parameter.

  .. cpp:function:: boost::shared_ptr<Type> getType(Type::do_share_t)

      Returns the type of this variable.

  .. cpp:function:: Type* getType()

      Returns the type of this variable.

  .. cpp:function:: int  getLineNum()

      Returns the line number where the variable was declared, if known.

  .. cpp:function:: std::string &getFileName()

      Returns the file where the variable was declared, if known.

  .. cpp:function:: std::vector<VariableLocation> &getLocationLists()

      Returns the locations where this variable is referenced.

      A local variable can be in scope at different positions and based on that it is accessible
      in different ways. Location lists provide a way to encode that information.
