DynC API
========

Motivation
----------

Dyninst is a powerful instrumentation tool, but specifying
instrumentation code (known as an Abstract Syntax Tree) in the language
can be cumbersome. DynC API answers these concerns, enabling a
programmer to easily and quickly build using a simple C-like language.
Other advantages to specifying using dynC include cleaner, more readable
mutator code, automatic variable handling, and runtime-compiled
snippets.

As a motivating example, the following implements a function tracer that
notifies the user when entering and exiting functions, and keeps track
of the number of times each function is called.

Dyninst API
~~~~~~~~~~~

When creating a function tracer using the Dyninst API, the programmer
must perform many discrete lookups and create many , which are then
combined and inserted into the mutatee.

Look up :

::

     std::vector<BPatch_function *> *printf_func;
     appImage->findFunction("printf", printf_func);
     BPatch_function *BPF_printf = printf_func[0];

Create each pattern:

::

     BPatch_constExpr entryPattern("Entering %s, called %d times.\n");
     BPatch_constExpr exitPattern("Exiting %s.\n");

**For each function, do the following:**

Create snippet vectors:

::

        std::vector<BPatch_snippet *> entrySnippetVect;
        std::vector<BPatch_snippet *> exitSnippetVect;

Create the global variable:

::

        appProc->malloc(appImage->findType("int"), std::string("intCounter"));

Get the name of the function:

::

        char fName[128];
        BPatch_constExpr funcName(functions[i]->getName(fName, 128));

Build the entry :

::

        std::vector<BPatch_snippet *> entryArgs;
        entryArgs.push_back(entryPattern);
        entryArgs.push_back(funcName);
        entryArgs.push_back(intCounter);

Build the exit :

::

        std::vector<BPatch_snippet *> exitArgs;
        exitArgs.push_back(exitPattern);
        exitArgs.push_back(funcName);

Add to the snippet:

::

        entrySnippetVect.push_back(BPatch_functionCallExpr(*printf_func, entryArgs));
        exitSnippetVect.push_back(BPatch_functionCallExpr(*printf_func, exitArgs));

Increment the counter:

::

        BPatch_arithExpr addOne(BPatch_assign, *intCounter, 
               BPatch_arithExpr(BPatch_plus, *intCounter, BPatch_constExpr(1)));

Add increment to the entry snippet:

::

        entrySnippetVect.push_back(&addOne);

Insert the snippets:

::

        appProc->insertSnippet(*entrySnippetVect, functions[i]->findPoint(BPatch_entry));
        appProc->insertSnippet(*exitSnippetVect, functions[i]->findPoint(BPatch_exit));

.. _dync-api-1:

DynC API
~~~~~~~~

A function tracer is much easier to build in DynC API, especially if
reading dynC code from file. Storing dynC code in external files not
only cleans up mutator code, but also allows the programmer to modify
snippets without recompiling.

In this example, the files and contain dynC code:

::

     // myEntryDynC.txt
     static int intCounter;
     printf("Entering %s, called %d times.\n", dyninst`function_name, intCounter++);

::

     // myExitDynC.txt
     printf("Leaving %s.\n", dyninst`function_name);

The code to read, build, and insert the snippets would look something
like the following:

First open files:

::

     FILE *entryFile = fopen("myEntryDynC.txt", "r");
     FILE *exitFile = fopen("myExitDynC.txt", "r");

Next call DynC API with each function’s entry and exit points:

::

     BPatch_snippet *entrySnippet = 
          dynC_API::createSnippet(entryFile, entryPoint, "entrySnippet");
     BPatch_snippet *exitSnippet = 
          dynC_API::createSnippet(exitFile, exitPoint, "exitSnippet");

Finally insert the snippets at each function’s entry and exit points:

::

     appProc->insertSnippet(*entrySnippet, entryPoint);
     appProc->insertSnippet(*exitSnippet, exitPoint);

Calling DynC API
----------------

All DynC functions reside in the namespace. The primary DynC API
function is:

::

      BPatch_Snippet *createSnippet(<dynC code>, <location>, char * name);

| where can be either a constant c-style string or a file descriptor and
  can take the form of a or a . There is also an optional parameter to
  name a snippet. A snippet name makes code and error reporting much
  easier to read, and allows for the grouping of snippets (see section
  `[sec:varExplain] <#sec:varExplain>`__). If a snippet name is not
  specified, the default name is used.

.. table::  input options: dynC code

   ==
   ======================================================================
   \  Description
   ==
   ======================================================================
   \  A C++ string containing dynC code.
   \  A null terminated string containing dynC code
   \  A standard C file descriptor. Facilitates reading dynC code from file.
   ==
   ======================================================================

.. table::  input options: location

   ==
   =================================================================================================
   \  Description
   ==
   =================================================================================================
   \  Creates a snippet specific to a single point.
   \  Creates a more flexible snippet specific to an address space. See Section `1.3 <#sec:nopoint>`__.
   ==
   =================================================================================================

The location parameter is the point or address space in which the
snippet will be inserted. Inserting a snippet created for one location
into another can cause undefined behavior.

.. _sec:nopoint:

Creating Snippets Without Point Information
-------------------------------------------

Creating a snippet without point information (i.e., calling with a )
results in a far more flexible snippet that may be inserted at any point
in the specified address space. There are, however, a few restrictions
on the types of operations that may be performed by a flexible snippet.
No local variables may be accessed, including parameters and return
values. Mutatee variables must be accessed through the domain.
