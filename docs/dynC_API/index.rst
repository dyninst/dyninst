DynC API
========

Motivation
----------

Dyninst is a powerful instrumentation tool, but specifying
instrumentation code (known as an Abstract Syntax Tree) in the
``BPatch_snippet`` language can be cumbersome. DynC API answers these
concerns, enabling a programmer to easily and quickly build
``BPatch_snippets`` using a simple C-like language. Other advantages to
specifying ``BPatch_snippets`` using dynC include cleaner, more readable
mutator code, automatic variable handling, and runtime-compiled
snippets.

As a motivating example, the following implements a function tracer that
notifies the user when entering and exiting functions, and keeps track
of the number of times each function is called.

Dyninst API
~~~~~~~~~~~

When creating a function tracer using the Dyninst API, the programmer
must perform many discrete lookups and create many
``BPatch_snippet objects``, which are then combined and inserted into
the mutatee.

Look up ``printf``:

.. code-block:: cpp

     std::vector<BPatch_function *> *printf_func;
     appImage->findFunction("printf", printf_func);
     BPatch_function *BPF_printf = printf_func[0];

Create each ``printf`` pattern:

.. code-block:: cpp

     BPatch_constExpr entryPattern("Entering %s, called %d times.\n");
     BPatch_constExpr exitPattern("Exiting %s.\n");

**For each function, do the following:**

Create snippet vectors:

.. code-block:: cpp

    std::vector<BPatch_snippet *> entrySnippetVect;
    std::vector<BPatch_snippet *> exitSnippetVect;

Create the ``intCounter`` global variable:

.. code-block:: cpp

    appProc->malloc(appImage->findType("int"), std::string("intCounter"));

Get the name of the function:

.. code-block:: cpp

    char fName[128];
    BPatch_constExpr funcName(functions[i]->getName(fName, 128));

Build the entry ``printf``:

.. code-block:: cpp
    
    std::vector<BPatch_snippet *> entryArgs;
    entryArgs.push_back(entryPattern);
    entryArgs.push_back(funcName);
    entryArgs.push_back(intCounter);

Build the exit ``printf``:

.. code-block:: cpp

    std::vector<BPatch_snippet *> exitArgs;
    exitArgs.push_back(exitPattern);
    exitArgs.push_back(funcName);

Add ``printf`` to the snippet:


.. code-block:: cpp

    entrySnippetVect.push_back(BPatch_functionCallExpr(*printf_func, entryArgs));
    exitSnippetVect.push_back(BPatch_functionCallExpr(*printf_func, exitArgs));

Increment the counter:

.. code-block:: cpp

    BPatch_arithExpr addOne(BPatch_assign, *intCounter, 
    BPatch_arithExpr(BPatch_plus, *intCounter, BPatch_constExpr(1)));

Add increment to the entry snippet:

.. code-block:: cpp

    entrySnippetVect.push_back(&addOne);

Insert the snippets:

.. code-block:: cpp

    appProc->insertSnippet(*entrySnippetVect, functions[i]->findPoint(BPatch_entry));
    appProc->insertSnippet(*exitSnippetVect, functions[i]->findPoint(BPatch_exit));


.. _dync-api-1:

The DynC API
~~~~~~~~~~~~

A function tracer is much easier to build in DynC API, especially if
reading dynC code from file. Storing dynC code in external files not
only cleans up mutator code, but also allows the programmer to modify
snippets without recompiling.

In this example, the files ``myEntryDynC.txt`` and ``myExitDynC.txt``
contain dynC code:

.. code-block:: cpp

    // myEntryDynC.txt
    static int intCounter;
    printf("Entering %s, called %d times.\n", dyninstfunction_name, intCounter++);

.. code-block:: cpp
    
    // myExitDynC.txt
    printf("Leaving %s.\n", dyninst function_name);

The code to read, build, and insert the snippets would look something
like the following:

First open files:

.. code-block:: cpp

    FILE *entryFile = fopen("myEntryDynC.txt", "r");
    FILE *exitFile = fopen("myExitDynC.txt", "r");

Next call DynC API with each function’s entry and exit points:

.. code-block:: cpp

    BPatch_snippet *entrySnippet = 
         dynC_API::createSnippet(entryFile, entryPoint, "entrySnippet");
    BPatch_snippet *exitSnippet = 
         dynC_API::createSnippet(exitFile, exitPoint, "exitSnippet");

Finally insert the snippets at each function’s entry and exit points:

.. code-block:: cpp

    appProc->insertSnippet(*entrySnippet, entryPoint);
    appProc->insertSnippet(*exitSnippet, exitPoint);

Calling DynC API
----------------

All DynC functions reside in the ``dynC_API`` namespace. The primary
DynC API function is:

.. code-block:: cpp

    BPatch_Snippet *createSnippet(<dynC code>, <location>, char * name);

| where ``<dynC code>`` can be either a constant c-style string or a
  file descriptor and ``<location>`` can take the form of a
  ``BPatch_point`` or a ``BPatch_addressSpace``. There is also an
  optional parameter to name a snippet. A snippet name makes code and
  error reporting much easier to read, and allows for the grouping of
  snippets (see section `2.3.2 <#sec:varExplain>`__). If a snippet name
  is not specified, the default name ``Snippet_[<#>]`` is used.

.. container:: centering

   .. table:: ``createSnippet(...)`` input options: dynC code

      +---------------------+-----------------------------------------------+
      | ``<dynC code>``     | Description                                   |
      +=====================+===============================================+
      | ``std::string str`` | A C++ string containing dynC code.            |
      +---------------------+-----------------------------------------------+
      | ``const char *s``   | A null terminated string containing dynC code |
      +---------------------+-----------------------------------------------+
      | ``FILE *f``         | A standard C file descriptor. Facilitates     |
      |                     | reading dynC code from file.                  |
      +---------------------+-----------------------------------------------+

   .. table:: ``createSnippet(...)`` input options: location

      +----------------------------------+----------------------------------+
      | ``<location>``                   | Description                      |
      +==================================+==================================+
      | ``BPatch_point &point``          | Creates a snippet specific to a  |
      |                                  | single point.                    |
      +----------------------------------+----------------------------------+
      | `                                | Creates a more flexible snippet  |
      | `BPatch_addressSpace &addSpace`` | specific to an address space.    |
      |                                  | See Section                      |
      |                                  | `1.3 <#sec:nopoint>`__.          |
      +----------------------------------+----------------------------------+

The location parameter is the point or address space in which the
snippet will be inserted. Inserting a snippet created for one location
into another can cause undefined behavior.

.. _`sec:nopoint`:

Creating Snippets Without Point Information
-------------------------------------------

Creating a snippet without point information (i.e., calling
``createSnippet(...)`` with a ``BPatch_addressSpace``) results in a far
more flexible snippet that may be inserted at any point in the specified
address space. There are, however, a few restrictions on the types of
operations that may be performed by a flexible snippet. No local
variables may be accessed, including parameters and return values.
Mutatee variables must be accessed through the ``global`` domain.

DynC Language Description
=========================

The DynC language is a subset of C with a **domain** specification for
selecting the location of a resource.

Domains
-------

Domains are special keywords that allow the programmer to precisely
indicate which resource to use. DynC domains follow the form of
``<domain>‘<identifier>``, with a back-tick separating the domain and
the identifier. The DynC domains are as follows:

.. table:: DynC API Domains

   +-------------+-------------------------------------------------------+
   | Domain      | Description                                           |
   +=============+=======================================================+
   | ``inf``     | The inferior process (the program being               |
   |             | instrumented). Allows access to functions of the      |
   |             | mutatee and it’s loaded libraries.                    |
   +-------------+-------------------------------------------------------+
   | ``dyninst`` | Dyninst utility functions. Allows access to context   |
   |             | information as well as the ``break()`` function. See  |
   |             | Appendix `3 <#sec:dyninstdomain>`__.                  |
   +-------------+-------------------------------------------------------+
   | ``local``   | A mutatee variable local to function in which the     |
   |             | snippet is inserted.                                  |
   +-------------+-------------------------------------------------------+
   | ``global``  | A global mutatee variable.                            |
   +-------------+-------------------------------------------------------+
   | ``param``   | A parameter of the mutatee function in which the      |
   |             | snippet is inserted.                                  |
   +-------------+-------------------------------------------------------+
   | *default*   | The default domain (domain not specified) is the      |
   |             | domain of snippet variables.                          |
   +-------------+-------------------------------------------------------+

Example:

.. code-block:: cpp

    printf("n is equal to %d.\n", ++global);

This would increment and print the value of the mutatee global variable
n.

Control Flow
------------

Comments
~~~~~~~~

Block and line comments work as they do in C or C++.

Example:

.. code-block:: cpp

    /*
     * This is a comment.
     */
    int i; // So is this.

Conditionals
~~~~~~~~~~~~

Use ``if`` to conditionally execute code. Example:

.. code-block:: cpp

    if(x == 0){
        printf("x == 0.\n");
     }

The ``else`` command can be used to specify code executed if a condition
is not true. Example:

.. code-block:: cpp

    if(x == 0){
        printf("x == 0.\n");
     }else if(x > 3){
        printf("x > 3.\n");
     }else{
        printf("x < 3 but x }= 0.\n");
     }

.. _`sec:firstOnly`:

First-Only Code Block
~~~~~~~~~~~~~~~~~~~~~

Code enclosed by a pair of ``% <code> %`` is executed only once by a
snippet. First-only code blocks can be useful for declaring and
initilizing variables, or for any task that needs to be executed only
once. Any number of first-only code blocks can be used in a dynC code
snippet.

A first-only code block is equivalent to the following:

.. code-block:: cpp

    static int firstTime = 0;
    if(firstTime == 0){
      <code>
      firstTime = 1;
    }

DynC will only execute the code in a first-only section the first time a
snippet is executed. If ``createSnippet(...)`` is called multiple times
and is passed the same name, then the first-only code will be executed
only once: the first time that any of those snippets *with the same
name* is executed. In contrast, if a snippet is created by calling
``createSnippet(...)`` with a unique snippet name (or if a name is
unspecified), the first-only code will be executed only once upon
reaching the first point encountered in the execution of the mutatee
where the returned ``BPatch_Snippet`` is inserted.

Example Touch:


.. code-block:: cpp

    {%
       printf("Function %s has been touched.\n", dyninst function_name);
    %}

If ``createSnippet(...)`` is passed the code in Example Touch and the
name ``"fooTouchSnip"`` and the returned ``BPatch_snippet`` is inserted
at the entry to function ``foo``, the output would be:

.. code-block:: cpp

    Function foo has been touched.
    (mutatee exit)

If the dynC code in Example Touch is passed to ``createSnippet(...)``
multiple times and each snippet is given the same name, but is inserted
at the entries of the functions ``foo``, ``bar``, and ``run``
respectively, the output would be:

.. code-block:: cpp

    Function foo has been touched.
    (mutatee exit)

Creating the snippets with distinct names (e.g. ``createSnippet(...)``
is called with the dynC code in Example Touch multiple times and the
snippets are named ``"fooTouchSnip"``, ``"barTouchSnip"``,
``"runTouchSnip"``) would produce an output like:

.. code-block:: cpp

    Function foo has been touched.
    Function bar has been touched.
    Function run has been touched.
    (mutatee exit)

A cautionary note: the use of first-only blocks can be expensive, as a
conditional must be evaluated each time the snippet is executed. If the
option is available, one may opt to insert a dynC snippet initializing
all global variables at the entry point of ``main``.

Variables
---------

DynC allows for the creation of *snippet local* variables. These
variables are in scope only within the snippet in which they are
created.

For example,

.. code-block:: cpp

    int i;
    i = 5;

would create an uninitialized variable named ``i`` of type integer. The
value of ``i`` is then set to 5. This is equivalent to:

.. code-block:: cpp
   
    int i = 5;

Static Variables
~~~~~~~~~~~~~~~~

Every time a snippet is executed, non-static variables are
reinitialized. To create a variable with value that persists across
executions of snippets, declare the variable as static.

Example:

.. code-block:: cpp

    int i = 10;
    printf("i is %d.\n", i++);

If the above is inserted at the entrance to a function that is called
four times, the output would be:

.. code-block:: cpp

    i is 10.
    i is 10.
    i is 10.
    i is 10.

Adding ``static`` to the variable declaration would make the value of
``i`` persist across executions:

.. code-block:: cpp

    static int i = 10;
    printf("i is %d.\n", i++);

Produces:

.. code-block:: cpp
    
   i is 10.
   i is 11.
   i is 12.
   i is 13.

A variable declared in a first-only section will also behave statically,
as the initialization occurs only once.

.. code-block:: cpp

    {%
       int i = 10;
    %}

.. _`sec:varExplain`:

An Explanation of the Internal Workings of DynC Variable Creation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

DynC uses the DyninstAPI function ``malloc(...)`` to allocate dynC
declared variables when ``createSnippet(...)`` is called. The variable
name is mangled with the name of the snippet passed to createSnippet.
Thus, variables declared in dynC snippets are accessible only to those
snippets created by calling ``createSnippet(...)`` with the same name.

If the variables are explicitly initialized, dynC sets the value of the
variable with a ``BPatch_arithExpr(BPatch_assign...)`` snippet. Because
of this, each time the snippet is executed, the value is reset to the
initialized value. If, however the variables are not explicitly
initialized, they are automatically set to a type-specific zero-value.
Scalar variables are set to 0, and c-strings are set to empty,
null-terminated strings (i.e. ``""``).

If a variable is declared with the ``static`` keyword, then the
initialization is performed as if in a first-only block (see section
`2.2.3 <#sec:firstOnly>`__). Thus, a variable is initialized only the
first time that snippet is executed, and subsequent executions of the
variable initialization are ignored.

Creating Global Variables That Work With DynC
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

To declare a global variable that is accessible to all snippets inserted
into a mutatee, one must use the DyninstAPI
``BPatch_addressSpace::malloc(...)`` method (see *Dyninst Programmer’s
Guide*). This code is located in mutator code (*not* in dynC code).

**myMutator.C:**

.. code-block:: cpp

    ...
    // Creates a global variable of type in named globalIntN
    myAddressSpace->malloc(myImage->getType("int"), "globalIntN"); 
      
    // file1 and file2 are FILE *, entryPoint and exitPoint are BPatch_point 
    BPatch_snippet *snippet1 = dynC::createSnippet(file1, &entryPoint, "mySnippet1"); 
    BPatch_snippet *snippet2 = dynC::createSnippet(file2, &exitPoint, "mySnippet2");
      
    assert(snippet1);
    assert(snippet2);
      
    myAdressSpace->insertSnippet(snippet1, &entryPoint);
    myAdressSpace->insertSnippet(snippet2, &exitPoint);
      
    // run the mutatee
    ((BPatch_process *)myAdressSpace)->continueExecution();
    ...

**file1:**

.. code-block:: cpp

    global globalIntN = 0; // initialize global variable in first-only section
    printf("Welcome to function %s. Global variable globalIntN = %d.\n", 
      dyninst function_name, global globalIntN++);


**file2:**

.. code-block:: cpp

    printf("Goodbye from function %s. Global variable globalIntN = %d.\n", 
      dyninst function_name, global globalIntN++);

When run, the output from the instrumentation would be:

.. code-block:: cpp

    Welcome to function foo. Global variable globalIntN = 0.
    Goodbye from function foo. Global variable globalIntN = 1.
    Welcome to function foo. Global variable globalIntN = 2.
    Goodbye from function foo. Global variable globalIntN = 3.
    Welcome to function foo. Global variable globalIntN = 4.
    Goodbye from function foo. Global variable globalIntN = 5.

.. _dataTypes:

Data Types
~~~~~~~~~~

| DynC supported data types are restricted by those supported by
  Dyninst: ``int``, ``long``, ``char *``, and ``void *``. Integer and
  c-string primitives are also recognized:
| Example:


.. code-block:: cpp

    int i = 12;
    char *s = "hello";

Pointers
~~~~~~~~

Pointers are dereferenced with the prefix ``*<variable>`` and the
address of variable is specified by ``&<variable>``. For example, in
reference to the previous example from section `2.3.4 <#dataTypes>`__,
the statement ``*s`` would evaluate to the character ``h``.

Arrays
~~~~~~

Arrays in DynC behave much the same way they do in C.

Example:

.. code-block:: cpp

    int array[3] = {1, 2, 3};
    char *names[] = {"Mark", "Phil", "Deb", "Tracy"};
    names[2] = "Gwen" // change Deb to Gwen
    printf("The seventh element of mutArray is %d.\n", global mutArray[6]); //Mutatee array 
    if(istrcmp(*names, "Mark") == 0){} // This will evaluate to true. 

DynC Limitations
----------------

The DynC, while quite expressive, is limited to those actions supported
by the DyninstAPI. As such, it lacks certain abilities that many
programmers have come to expect. These differences will be discussed in
an exploration of those C abilities that dynC lacks.

Loops
~~~~~

There are no looping structures in DynC.

Enums, Unions, Structures
~~~~~~~~~~~~~~~~~~~~~~~~~

These features present a unique implementation challenge and are in
development. Look to future revisions for full support for enums,
unions, and structures.

Preprocessing
~~~~~~~~~~~~~

DynC does not allow C-style preprocessing macros or importation. Rather
than ``#define`` statements, constant variables are recommended.

Functions
~~~~~~~~~

Specifying functions is beyond the scope of the DynC language.
DyninstAPI has methods for dynamically loading code into a mutatee, and
these loaded functions can be used in DynC snippets.

.. _`sec:dyninstdomain`:

The Dyninst Domain
==================

The ``dyninst`` domain has quite a few useful values and functions:

.. table:: Dyninst Domain Values

   +-----------------+------------+-----------------+-----------------+
   | Identifier      | Type       | Where Valid     | Description     |
   +=================+============+=================+=================+
   | ``              | ``char *`` | Within a        | Evaluates to    |
   | function_name`` |            | function        | the name of the |
   |                 |            |                 | current         |
   |                 |            |                 | function. Call  |
   |                 |            |                 | to              |
   |                 |            |                 | ``creat         |
   |                 |            |                 | eSnippet(...)`` |
   |                 |            |                 | must specify a  |
   |                 |            |                 | ``              |
   |                 |            |                 | BPatch_point``. |
   +-----------------+------------+-----------------+-----------------+
   | ``module_name`` | ``char *`` | Anywhere        | Evaluates to    |
   |                 |            |                 | the name of the |
   |                 |            |                 | current module. |
   |                 |            |                 | Call to         |
   |                 |            |                 | ``creat         |
   |                 |            |                 | eSnippet(...)`` |
   |                 |            |                 | must specify a  |
   |                 |            |                 | ``              |
   |                 |            |                 | BPatch_point``. |
   +-----------------+------------+-----------------+-----------------+
   | ``b             | int        | At a memory     | Evaluates to    |
   | ytes_accessed`` |            | operation       | the number of   |
   |                 |            |                 | bytes accessed  |
   |                 |            |                 | by a memory     |
   |                 |            |                 | operation.      |
   +-----------------+------------+-----------------+-----------------+
   | ``effe          | ``void *`` | At a memory     | Evaluates the   |
   | ctive_address`` |            | operation       | effective       |
   |                 |            |                 | address of a    |
   |                 |            |                 | memory          |
   |                 |            |                 | operation.      |
   +-----------------+------------+-----------------+-----------------+
   | ``ori           | ``void *`` | Anywhere        | Evaluates to    |
   | ginal_address`` |            |                 | the original    |
   |                 |            |                 | address where   |
   |                 |            |                 | the snippet was |
   |                 |            |                 | inserted.       |
   +-----------------+------------+-----------------+-----------------+
   | ``a             | ``void *`` | Anywhere        | Evaluates to    |
   | ctual_address`` |            |                 | the actual      |
   |                 |            |                 | address of the  |
   |                 |            |                 | i               |
   |                 |            |                 | nstrumentation. |
   +-----------------+------------+-----------------+-----------------+
   | `               | ``void *`` | Function exit   | Evaluates to    |
   | `return_value`` |            |                 | the return      |
   |                 |            |                 | value of a      |
   |                 |            |                 | function.       |
   +-----------------+------------+-----------------+-----------------+
   | `               | int        | Anywhere        | Returns the     |
   | `thread_index`` |            |                 | index of the    |
   |                 |            |                 | thread the      |
   |                 |            |                 | snippet is      |
   |                 |            |                 | executing on.   |
   +-----------------+------------+-----------------+-----------------+
   | ``tid``         | int        | Anywhere        | Returns the id  |
   |                 |            |                 | of the thread   |
   |                 |            |                 | the snippet is  |
   |                 |            |                 | executing on.   |
   +-----------------+------------+-----------------+-----------------+
   | ``d             | ``void *`` | At calls,       | Calculates the  |
   | ynamic_target`` |            | jumps, returns  | target of a     |
   |                 |            |                 | control flow    |
   |                 |            |                 | instruction.    |
   +-----------------+------------+-----------------+-----------------+
   | ``break()``     | void       | Anywhere        | Causes the      |
   |                 |            |                 | mutatee to      |
   |                 |            |                 | execute a       |
   |                 |            |                 | breakpoint.     |
   +-----------------+------------+-----------------+-----------------+
   | `               | void       | Anywhere        | Stops the       |
   | `stopthread()`` |            |                 | thread on which |
   |                 |            |                 | the snippet is  |
   |                 |            |                 | executing.      |
   +-----------------+------------+-----------------+-----------------+
