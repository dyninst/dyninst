.. _`sec:singleton_object_pool.h`:

singleton_object_pool.h
#######################

.. warning:: This is only safe for objects with nothrow constructors.

.. cpp:class:: template <typename T, typename Alloc = std::allocator<T>> singleton_object_pool : public Alloc

  .. cpp:function:: static pointer allocate( size_type n )
  .. cpp:function:: static void deallocate( pointer p )
  .. cpp:function:: template<typename... Args> static T* construct(Args&&... args)
  .. cpp:function:: static void destroy(pointer p)

.. cpp:function:: template <typename T, typename Alloc> typename singleton_object_pool<T, Alloc>::pointer singleton_object_pool<T, Alloc>::allocate( size_type n )

.. cpp:struct template <typename T> PoolDestructor

  .. cpp:function:: void operator()(T* e)

.. cpp:function:: template <typename T> boost::shared_ptr<T> make_shared(T* t)
