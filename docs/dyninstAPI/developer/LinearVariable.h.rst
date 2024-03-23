.. _`sec:LinearVariable.h`:

LinearVariable.h
################

A representation of a variable ``int x = x + var1 + var2 + var3 + ...``
where ``int`` is an integer and ``var1...varN`` are unknown variables.

.. cpp:struct:: template <typename T> Var

  .. cpp:type:: typename std::map<T, int> Unknowns
  .. cpp:function:: Var<T> &operator+=(const Var<T> &rhs)
  .. cpp:function:: Var<T> &operator+=(const int &rhs)
  .. cpp:function:: Var<T> operator+(const Var<T> &rhs) const
  .. cpp:function:: Var<T> operator+(const int &rhs) const
  .. cpp:function:: Var<T> operator*=(const int &rhs)
  .. cpp:function:: Var<T> operator*(const int &rhs) const
  .. cpp:function:: bool operator==(const Var<T> &rhs) const
  .. cpp:function:: bool operator!=(const Var<T> &rhs) const
  .. cpp:function:: bool operator!=(const int &rhs) const
  .. cpp:function:: Var()
  .. cpp:function:: Var(int a)
  .. cpp:function:: Var(T a)
  .. cpp:member:: int x
  .. cpp:member:: Unknowns unknowns


.. cpp:struct:: template <typename T> linVar

  .. cpp:function:: linVar<T> &operator+=(const linVar<T> &rhs)
  .. cpp:function:: const linVar<T> operator+(const linVar<T> &rhs) const
  .. cpp:function:: const linVar<T> operator*=(const int &rhs)
  .. cpp:function:: const linVar<T> operator*=(const linVar<T> &rhs)
  .. cpp:function:: const linVar<T> operator*(const linVar<T> &rhs) const
  .. cpp:function:: linVar()
  .. cpp:function:: linVar(T x, T y)
  .. cpp:function:: linVar(int x, int y)
  .. cpp:function:: linVar(T x, int y)
  .. cpp:function:: linVar(Var<T> x, Var<T> y)
  .. cpp:member:: bool bottom
  .. cpp:member:: Var<T> a
  .. cpp:member:: Var<T> b
