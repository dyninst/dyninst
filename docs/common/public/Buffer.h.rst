.. _`sec:Buffer.h`:

Buffer.h
########

.. cpp:namespace:: Dyninst

.. cpp:class:: Buffer

  A wrapper around a sequence of in-memory bytes to support multiple forms of code generation

  .. cpp:function:: Buffer(Address addr, unsigned initial_size)
  .. cpp:function:: void initialize(Address addr, unsigned initial_size)

  .. cpp:var:: static const int ALLOCATION_UNIT

  .. cpp:function:: template <class InputIterator> void copy(InputIterator begin, InputIterator end)

  .. cpp:function:: void copy(void *buffer, unsigned size)

  .. cpp:function:: unsigned size() const
  .. cpp:function:: unsigned max_size() const
  .. cpp:function:: bool empty() const

  .. cpp:function:: template <class Input> void push_back(const Input &)

  .. cpp:type:: iterator<unsigned char> byte_iterator
  .. cpp:type:: iterator<unsigned int> word_iterator
  .. cpp:type:: iterator<unsigned long> long_iterator

  .. cpp:function:: byte_iterator begin() const

    Returns an iterator pointing to the beginning of the buffer, interpreted as byte.

  .. cpp:function:: byte_iterator end() const

    Returns an iterator marking the end of the buffers.

  .. cpp:function:: word_iterator w_begin() const

    Returns an iterator pointing to the beginning of the buffer, interpreted as words.

  .. cpp:function:: word_iterator w_end() const

    Returns an iterator marking the end of the buffer.

  .. cpp:function:: long_iterator l_begin() const

    Returns an iterator pointing to the beginning of the buffer.

  .. cpp:function:: long_iterator l_end() const

    Returns an iterator marking the end of the buffer.

  .. cpp:function:: unsigned char *start_ptr() const

    Returns a pointer to the beginning of the buffer, interpreted as bytes.

  .. cpp:function:: Address startAddr() const
  .. cpp:function:: Address curAddr() const

.. cpp:class:: template <class storage> Buffer::iterator

  Helper class modeling the C++ `LegacyForwardIterator <https://en.cppreference.com/w/cpp/named_req/ForwardIterator>`_ concept.
