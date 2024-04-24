.. _`sec:stats.h`:

stats.h
#######

.. cpp:class:: Statistic

  .. cpp:function:: virtual bool is_count()
  .. cpp:function:: virtual bool is_timer()
  .. cpp:function:: virtual long int value()
  .. cpp:function:: virtual double usecs()
  .. cpp:function:: virtual double ssecs()
  .. cpp:function:: virtual double wsecs()

  .. cpp:function:: protected Statistic(StatContainer *c)
  .. cpp:function:: protected virtual ~Statistic() = default
  .. cpp:function:: protected Statistic(const Statistic&) = default
  .. cpp:member:: protected StatContainer *container_


.. cpp:class:: CntStatistic : public Statistic

  .. cpp:function:: protected CntStatistic(StatContainer *c)

  .. cpp:function:: CntStatistic()
  .. cpp:function:: bool is_count()
  .. cpp:function:: CntStatistic operator++( int )
  .. cpp:function:: CntStatistic& operator++()
  .. cpp:function:: CntStatistic operator--( int )
  .. cpp:function:: CntStatistic& operator--()
  .. cpp:function:: CntStatistic& operator=( long int )
  .. cpp:function:: CntStatistic& operator+=( long int )
  .. cpp:function:: CntStatistic& operator+=( const CntStatistic &)
  .. cpp:function:: CntStatistic& operator-=( long int )
  .. cpp:function:: CntStatistic& operator-=(  const CntStatistic &)
  .. cpp:function:: long int operator*()
  .. cpp:function:: long int value()

.. cpp:class:: TimeStatistic : public Statistic

  **Wraps the timer class**

  .. cpp:function:: protected TimeStatistic(StatContainer *c)

  .. cpp:function:: TimeStatistic()
  .. cpp:function:: bool is_timer()
  .. cpp:function:: TimeStatistic& operator=( TimeStatistic &)
  .. cpp:function:: TimeStatistic& operator+=( TimeStatistic &)
  .. cpp:function:: TimeStatistic& operator+( TimeStatistic &)
  .. cpp:function:: void clear()
  .. cpp:function:: void start()
  .. cpp:function:: void stop()
  .. cpp:function:: double usecs()
  .. cpp:function:: double ssecs()
  .. cpp:function:: double wsecs()
  .. cpp:function:: bool is_running()

.. cpp:enum:: StatType

  .. cpp:enumerator:: CountStat
  .. cpp:enumerator:: TimerStat

.. cpp:class:: StatContainer

  **A container for a group of (one expects) mutually related statistics**

  .. cpp:function:: StatContainer()
  .. cpp:function:: Statistic * operator[](const std::string &)

    Access or create a statistic indexed by the provided name.

    This operator may return null if the named statistic doe not exist.

  .. cpp:function:: Statistic * operator[](const char *s)

    Access or create a statistic indexed by the provided name.

    This operator may return null if the named statistic doe not exist.

  .. cpp:function:: void add(const std::string& name, StatType type)

    Create a new statistic of the given type indexed by name.

    .. attention:: This will replace any existing stat with the same index within this container.

  .. cpp:function:: dyn_hash_map<std::string, Statistic *> &allStats()
  .. cpp:function:: void startTimer(const std::string&)
  .. cpp:function:: void stopTimer(const std::string&)
  .. cpp:function:: void incrementCounter(const std::string&)
  .. cpp:function:: void decrementCounter(const std::string&)
  .. cpp:function:: void addCounter(const std::string&, int)
