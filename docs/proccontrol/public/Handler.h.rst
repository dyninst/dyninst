.. _`sec:Handler.h`:

Handler.h
=========

.. cpp:namespace:: Dyninst::ProcControlAPI

.. cpp:class:: Handler

  .. cpp:function:: Handler(std::string name_ = std::string(""))

  .. cpp:enum:: handler_ret_t

     .. cpp:enumerator:: ret_success
     .. cpp:enumerator:: ret_async
     .. cpp:enumerator:: ret_cbdelay
     .. cpp:enumerator:: ret_again
     .. cpp:enumerator:: ret_error

  .. cpp:member:: static const int PrePlatformPriority = 0x1000
  .. cpp:member:: static const int DefaultPriority = 0x1008
  .. cpp:member:: static const int PostPlatformPriority = 0x1010
  .. cpp:member:: static const int CallbackPriority = 0x1018
  .. cpp:member:: static const int PostCallbackPriority = 0x1020

  .. cpp:function:: virtual handler_ret_t handleEvent(Event::ptr ev) = 0
  .. cpp:function:: virtual void getEventTypesHandled(std::vector<EventType> &etypes) = 0
  .. cpp:function:: virtual int getPriority() const
  .. cpp:function:: virtual Event::ptr convertEventForCB(Event::ptr orig)

  .. cpp:function:: std::string getName() const
