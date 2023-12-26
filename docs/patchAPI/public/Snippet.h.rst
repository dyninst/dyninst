Snippet.h
=========

.. cpp:namespace:: Dyninst::patchAPI

Snippet
=======

**Declared in**: Snippet.h

The Snippet class allows programmers to customize their own snippet
representation and the corresponding mini-compiler to translate the
representation into the binary code.

.. code-block:: cpp
    
    static Ptr create(Snippet* a);

Creates an object of the Snippet.

.. code-block:: cpp
    
    virtual bool generate(Point *pt, Buffer &buf);

Users should implement this virtual function for generating binary code
for the snippet.

Returns false if code generation failed catastrophically. Point *pt* is
an in-param that identifies where the snippet is being generated. Buffer
*buf* is an out-param that holds the generated code.