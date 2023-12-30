.. _`sec:vgannotations.h`:

vgannotations.h
###############

**Valgrind annotation features**

Use the CMake option ``-DADD_VALGRIND_ANNOTATIONS=ON`` to enable.

One or two places use function-scoped static variables for lazy singleton
initialization. Unfortunately its a pain to mark up properly, so the following
is a class with the same effect, but with the potential for marking.

Caveats:

  - The value will be default-constructed with all other global statics, so for
    efficiency use a type that has a small default constructor.
  - The value will be assigned to as part of the lazy initialization, so the
    logic for assignment and construction needs to be equivalent.
  - Same as call_once, the given functions should be identical, otherwise weird
    nondeterminism may occur.
  - Currently the function cannot take any arguments. Someone with more C++
    background can fix that later if they like.

.. cpp:class template<typename T> LazySingleton

  .. cpp:type:: T type

  .. cpp:function:: T& get(std::function<T()> f)
