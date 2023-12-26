PatchMgr.h
==========

.. cpp:namespace:: Dyninst::patchAPI

PatchMgr
========

**Declared in**: PatchMgr.h

The PatchMgr class is the top-level class for finding instrumentation
**Points**, inserting or deleting **Snippets**, and registering
user-provided plugins.

.. code-block:: cpp
    
    static PatchMgrPtr create(AddrSpace* as, Instrumenter* inst = NULL, PointMaker* pm = NULL);

This factory method creates a new PatchMgr object that performs binary
code patching. It takes input three plugins, including AddrSpace *as*,
Instrumenter *inst*, and PointMaker *pm*. PatchAPI uses default plugins
for PointMaker and Instrumenter, if *pm* and *inst* are not specified
(NULL by default).

This method returns PatchMgrPtr() if it was unable to create a new
PatchMgr object.

.. code-block:: cpp
    
    Point *findPoint(Location loc, Point::Type type, bool create = true);

This method returns a unique Point according to a Location *loc* and a
Type *type*. The Location structure is to specify a physical location of
a Point (e.g., at function entry, at block entry, etc.), details of
Location will be covered in Section `4.2.2 <#sec-3.1.2>`__. PatchAPI
creates Points on demand, so if a Point is not yet created, the *create*
parameter is to indicate whether to create this Point. If the Point we
want to find is already created, this method simply returns a pointer to
this Point from a buffer, no matter whether *create* is true or false.
If the Point we want to find is not yet created, and *create* is true,
then this method constructs this Point and put it in a buffer, and
finally returns a Pointer to this Point. If the Point creation fails,
this method also returns false. If the Point we want to find is not yet
created, and *create* is false, this method returns NULL. The basic
logic of finding a point can be found in the
Listing `[findpt] <#findpt>`__.

.. code-block:: cpp
    
   if (point is in the buffer) {
     return point;
   } else {
     if (create == true) {
       create point
       if (point creation fails) return NULL;
       put the point in the buffer
     } else {
       return NULL;
     }
   }

.. code-block:: cpp
    

    template <class OutputIterator> bool findPoint(Location loc, Point::Type type, OutputIterator outputIter, bool create = true);

This method finds a Point at a physical Location *loc* with a *type*. It
adds the found Point to *outputIter* that is a STL inserter. The point
is created on demand. If the Point is already created, then this method
outputs a pointer to this Point from a buffer. Otherwise, the *create*
parameter indicates whether to create this Point.

This method returns true if a point is found, or the *create* parameter
is false; otherwise, it returns false.

.. code-block:: cpp
    
    template <class OutputIterator> bool findPoints(Location loc,
    Point::Type types, OutputIterator outputIter, bool create = true);

This method finds Points at a physical Location *loc* with composite
*types* that are combined using the overloaded operator “\|”. This
function outputs Points to the STL inserter *outputIter*. The point is
created on demand. If the Point is already created, then this method
outputs a pointer to this Point from a buffer. Otherwise, the *create*
parameter indicates whether to create this Point.

This method returns true if a point is found, or the *create* parameter
is false; otherwise, it returns false.

.. code-block:: cpp
    
    template <class FilterFunc, class FilterArgument, class OutputIterator>
    bool findPoints(Location loc, Point::Type types, FilterFunc filter_func,
    FilterArgument filter_arg, OutputIterator outputIter, bool create = true);

This method finds Points at a physical Location *loc* with composite
*types* that are combined using the overloaded operator “\|”. Then, this
method applies a filter functor *filter_func* with an argument
*filter_arg* on each found Point. The method outputs Points to the
inserter *outputIter*. The point is created on demand. If the Point is
already created, then this method returns a pointer to this Point from a
buffer. Otherwise, the *create* parameter indicates whether to create
this Point.

If no any Point is created, then this method returns false; otherwise,
true is returned. The code below shows the prototype of an example
functor.

.. code-block:: cpp
    
   template <class T>
   class FilterFunc {
     public:
       bool operator()(Point::Type type, Location loc, T arg) {
         // The logic to check whether this point is what we need
         return true;
       }
   };

In the functor FilterFunc above, programmers check each candidate Point
by looking at the Point::Type, Location, and the user-specified
parameter *arg*. If the return value is true, then the Point being
checked will be put in the STL inserter *outputIter*; otherwise, this
Point will be discarded.

.. code-block:: cpp
    
    struct Scope Scope(PatchBlock *b); Scope(PatchFunction *f, PatchBlock *b); Scope(PatchFunction *f);;

The Scope structure specifies the scope to find points, where a scope
could be a function, or a basic block. This is quite useful if
programmers don’t know the exact Location, then they can use Scope as a
wildcard. A basic block can be contained in multiple functions. The
second constructor only specifies the block *b* in a particular function
*f*.

.. code-block:: cpp
    
    template <class FilterFunc, class FilterArgument, class OutputIterator>
    bool findPoints(Scope scope, Point::Type types, FilterFunc filter_func,
    FilterArgument filter_arg, OutputIterator output_iter, bool create = true);

This method finds points in a *scope* with certain *types* that are
combined together by using the overloaded operator “\|”. Then, this
method applies the filter functor *filter_func* on each found Point. It
outputs Points where *filter_func* returns true to the STL inserter
*output_iter*. Points are created on demand. If some points are already
created, then this method outputs pointers to them from a buffer.
Otherwise, the *create* parameter indicates whether to create Points.

If no any Point is created, then this function returns false; otherwise,
true is returned.

.. code-block:: cpp
    
    template <class OutputIterator> bool findPoints(Scope scope, Point::Type types, OutputIterator output_iter, bool create = true);

This method finds points in a *scope* with certain *types* that are
combined together by using the overloaded operator “\|”. It outputs the
found points to the STL inserter *output_iter*. If some points are
already created, then this method outputs pointers to them from a
buffer. Otherwise, the *create* parameter indicates whether to create
Points.

If no any Point is created, then this method returns false; otherwise,
true is returned.

.. code-block:: cpp
    
    bool removeSnippet(InstancePtr);

This method removes a snippet Instance.

It returns false if the point associated with this Instance cannot be
found; otherwise, true is returned.

.. code-block:: cpp
    
    template <class FilterFunc, class FilterArgument> bool
    removeSnippets(Scope scope, Point::Type types, FilterFunc filter_func,
    FilterArgument filter_arg);

This method deletes ALL snippet instances at certain points in certain
*scope* with certain *types*, and those points pass the test of
*filter_func*.

If no any point can be found, this method returns false; otherwise, true
is returned.

.. code-block:: cpp
    
    bool removeSnippets(Scope scope, Point::Type types);

This method deletes ALL snippet instances at certain points in certain
*scope* with certain *types*.

If no any point can be found, this method returns false; otherwise, true
is returned.

.. code-block:: cpp
    
    void destroy(Point *point);

This method is to destroy the specified *Point*.

.. code-block:: cpp
    
    AddrSpace* as() const; PointMaker* pointMaker() const; Instrumenter* instrumenter() const;

The above three functions return the corresponding plugin: AddrSpace,
PointMaker, Instrumenter.