.. _`sec:Handler.h`:

Handler.h
=========

.. cpp:namespace:: Dyninst::ProcControlAPI

.. cpp:class:: Handler

  .. cpp:function:: Handler(std::string name_ = std::string(""));
  .. cpp:function:: virtual ~Handler();

  .. cpp:enum:: handler_ret_t

     .. cpp:enumerator:: handler_ret_t::ret_success
     .. cpp:enumerator:: handler_ret_t::ret_async
     .. cpp:enumerator:: handler_ret_t::ret_cbdelay
     .. cpp:enumerator:: handler_ret_t::ret_again
     .. cpp:enumerator:: handler_ret_t::ret_error

  .. cpp:function:: virtual handler_ret_t handleEvent(Event::ptr ev) = 0;
  .. cpp:function:: virtual void getEventTypesHandled(std::vector<EventType> &etypes) = 0;
  .. cpp:function:: virtual int getPriority() const;
  .. cpp:function:: virtual Event::ptr convertEventForCB(Event::ptr orig);

  .. cpp:function:: std::string getName() const;

  .. csv-table:: Priority values
    :header: "Name", "Value"
    :widths: 25 4

    "CallbackPriority","0x1018"
    "DefaultPriority","0x1008"
    "PostCallbackPriority","0x1020"
    "PostPlatformPriority","0x1010"
    "PrePlatformPriority","0x1000"
