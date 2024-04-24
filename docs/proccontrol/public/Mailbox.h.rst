.. _`sec:Mailbox.h`:

Mailbox.h
=========

.. cpp:namespace:: Dyninst::ProcControlAPI

.. cpp:class:: Mailbox

  .. cpp:function:: Mailbox()
  .. cpp:function:: virtual ~Mailbox()
  .. cpp:function:: virtual void enqueue(Event::ptr ev, bool priority = false) = 0
  .. cpp:function:: virtual void enqueue_user(Event::ptr ev) = 0
  .. cpp:function:: virtual bool hasPriorityEvent() = 0
  .. cpp:function:: virtual Event::ptr dequeue(bool block) = 0
  .. cpp:function:: virtual Event::ptr peek() = 0
  .. cpp:function:: virtual unsigned int size() = 0

.. cpp:enum:: Mailbox::priority_t

  .. cpp:enumerator:: low
  .. cpp:enumerator:: med
  .. cpp:enumerator:: high
