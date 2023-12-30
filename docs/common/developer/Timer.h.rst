.. _`sec:Timer.h`:

Timer.h
#######

.. cpp:class:: timer

  .. cpp:function:: timer()
  .. cpp:function:: timer(const timer &)
  .. cpp:function:: ~timer()
  .. cpp:function:: timer& operator=(const timer &)
  .. cpp:function:: timer& operator+=(const timer &)
  .. cpp:function:: timer operator+(const timer &) const
  .. cpp:function:: void clear()
  .. cpp:function:: void start()
  .. cpp:function:: void stop()
  .. cpp:function:: double usecs() const
  .. cpp:function:: double ssecs() const
  .. cpp:function:: double wsecs() const
  .. cpp:function:: bool is_running() const
  .. cpp:function:: double CYCLES_PER_SEC() const
  .. cpp:function:: double NANOSECS_PER_SEC() const
  .. cpp:function:: double MICROSECS_PER_SEC() const
