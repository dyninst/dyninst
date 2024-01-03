.. _`sec:Command.h`:

Command.h
#########

.. cpp:namespace:: Dyninst::PatchAPI

.. cpp:class:: Command

  **An instrumentation request**

  Snippet insertion or removal, or an internal logical step in the code patching
  (e.g., install instrumentation).

  .. cpp:function:: virtual bool run() = 0

      Executes the normal operation of this Command.

      Returns ``false`` on error.

  .. cpp:function:: virtual bool undo() = 0

      Undoes the operation of this Command.

      Returns ``false`` on error.

  .. cpp:function:: virtual bool commit()

      Implements the transactional semantics: all succeed, or all fail.

      Returns ``false`` on error.


.. cpp:class:: BatchCommand : public Command

  **A sequence of commands**

  .. cpp:type:: std::list<CommandPtr> CommandList

  .. cpp:member:: protected CommandList to_do_

      The list of Commands to execute.

  .. cpp:member:: protected CommandList done_

      The list of Commands that have been executed.

  .. cpp:function:: virtual bool run()

      Runs all commands.

  .. cpp:function:: virtual bool undo()

      Undoes all commands in the order they were executed.

  .. cpp:function:: void add(CommandPtr command)

      Adds ``command`` to the list of commands to be executed.

  .. cpp:function:: void remove(CommandList::iterator iter)

      Removes the command pointed to by ``iter`` from the list of
      commands to be executed.


.. cpp:class:: Patcher : public BatchCommand

  **Special BatchCommand that implicitly executes the instrumentation**

  Accepts instrumentation :cpp:class:`Command` requests from users and
  implicitly adds an instance of :cpp:class:`Instrumenter` to the end of the
  command list to generate binary code and install the instrumentation.

  .. cpp:type:: Ptr = boost::shared_ptr<Patcher>

  .. cpp:function:: Patcher(PatchMgrPtr mgr)

      Creates a new patcher managed by ``mgr``.

  .. cpp:function:: static Ptr create(PatchMgrPtr mgr)

      Helper for creating a ``Patcher``.

  .. cpp:function:: virtual bool run()

    Runs all commands.

    It also implicitly adds an :cpp:class:`Instrumenter` to the end of
    the list of commands to execute.

.. cpp:class:: PushFrontCommand : public Command

  **Adds a snippet to the front of the list of commands to be executed**

  .. cpp:function:: PushFrontCommand(Point* pt, SnippetPtr snip)

      Creates a command to insert ``snip`` at the point ``pt``.

      The point maintains a list of snippet instances.

  .. cpp:function:: static PushFrontCommand* create(Point* pt, SnippetPtr snip)

      Helper for creating a ``PushFrontCommand``.

  .. cpp:function:: virtual bool run()

      The same as :cpp:func:`Command::run`.

  .. cpp:function:: virtual bool undo()

      The same as :cpp:func:`Command::undo`.

  .. cpp:function:: InstancePtr instance()

      Returns the snippet instance that is inserted at the point.

.. cpp:class:: PushBackCommand : public Command

  **Adds a snippet to the end of the list of commands to be executed**

  .. cpp:function:: PushBackCommand(Point* pt, SnippetPtr snip)

      Creates a command to insert ``snip`` at the point ``pt``.

      The point maintains a list of snippet instances.

  .. cpp:function:: static PushBackCommand* create(Point* pt, SnippetPtr snip)

      Helper for creating a ``PushBackCommand``.

  .. cpp:function:: virtual bool run()

      The same as :cpp:func:`Command::run`.

  .. cpp:function:: virtual bool undo()

      The same as :cpp:func:`Command::undo`.

.. cpp:class:: RemoveSnippetCommand : public Command

  **Removes a snippet from the list of commands to be executed**

  .. cpp:function:: RemoveSnippetCommand(InstancePtr instance)

      Creates a command to remove the snippet ``instance``.

      The point maintains a list of snippet instances.

  .. cpp:function:: static RemoveSnippetCommand* create(InstancePtr instance)

      Helper for creating a ``RemoveSnippetCommand``.

  .. cpp:function:: virtual bool run()

      The same as :cpp:func:`Command::run`.

  .. cpp:function:: virtual bool undo()

      The same as :cpp:func:`Command::undo`.

.. cpp:class:: RemoveCallCommand : public Command

  **Remove a function call**

  .. cpp:function:: RemoveCallCommand(PatchMgrPtr mgr, PatchBlock* call_block, PatchFunction* context)

      Creates a command to remove the function in ``context`` from the block at ``call_block`` owned by ``mgr``.

      There may be multiple functions containing the same ``call_block``. If the ``context`` is
      ``NULL``, then the ``call_block`` is deleted from all ``PatchFunctions`` that contain it.
      Otherwise, it is only deleted ``context``.

  .. cpp:function:: static RemoveCallCommand* create(PatchMgrPtr mgr, PatchBlock* call_block, PatchFunction* context = NULL)

      A helper for creating a ``RemoveCallCommand``.

  .. cpp:function:: virtual bool run()

      The same as :cpp:func:`Command::run`.

  .. cpp:function:: virtual bool undo()

      The same as :cpp:func:`Command::undo`.

.. cpp:class:: ReplaceCallCommand : public Command

  **Replace a function call with another one**

  .. cpp:function:: ReplaceCallCommand(PatchMgrPtr mgr, PatchBlock* call_block, PatchFunction* new_callee, PatchFunction* context)

      Creates a command to replace the function in ``context`` from the block at ``call_block`` owned by ``mgr``
      with the function ``new_callee``.

      There may be multiple functions containing the same ``call_block``. If the ``context`` is
      ``NULL``, then the ``call_block`` is deleted from all ``PatchFunctions`` that contain it.
      Otherwise, it is only deleted ``context``.

  .. cpp:function:: static ReplaceCallCommand* create(PatchMgrPtr mgr, PatchBlock* call_block, PatchFunction* new_callee, PatchFunction* context)

        Helper for creating a ``ReplaceCallCommand``.

  .. cpp:function:: virtual bool run()

      The same as :cpp:func:`Command::run`.

  .. cpp:function:: virtual bool undo()

      The same as :cpp:func:`Command::undo`.

.. cpp:class:: ReplaceFuncCommand : public Command

  **Replace an old function with the new one**

  .. cpp:function:: ReplaceFuncCommandcreate(PatchMgrPtr mgr, PatchFunction* old_func, PatchFunction* new_func)

      Creates a command to replace the ``old_func`` with ``new_func``, owned by ``mgr``.

  .. cpp:function:: static ReplaceFuncCommand* create(PatchMgrPtr mgr, PatchFunction* old_func, PatchFunction* new_func)

      A helper for created a ``ReplaceFuncCommandcreate``.

  .. cpp:function:: virtual bool run()

      The same as :cpp:func:`Command::run`.

  .. cpp:function:: virtual bool undo()

      The same as :cpp:func:`Command::undo`.
