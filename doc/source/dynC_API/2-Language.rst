DynC Language Description
=========================

The DynC language is a subset of C with a **domain** specification for
selecting the location of a resource.

Domains
-------

Domains are special keywords that allow the programmer to precisely
indicate which resource to use. DynC domains follow the form of , with a
back-tick separating the domain and the identifier. The DynC domains are
as follows:

.. table:: DynC API Domains

   =========
   ===================================================================================================================================================
   Domain    Description
   =========
   ===================================================================================================================================================
   \         The inferior process (the program being instrumented). Allows access to functions of the mutatee and it’s loaded libraries.
   \         Dyninst utility functions. Allows access to context information as well as the function. See Appendix `[sec:dyninstdomain] <#sec:dyninstdomain>`__.
   \         A mutatee variable local to function in which the snippet is inserted.
   \         A global mutatee variable.
   \         A parameter of the mutatee function in which the snippet is inserted.
   *default* The default domain (domain not specified) is the domain of snippet variables.
   =========
   ===================================================================================================================================================

Example:

::

      inf`printf("n is equal to %d.\n", ++global`n);

This would increment and print the value of the mutatee global variable
n.

Control Flow
------------

Comments
~~~~~~~~

Block and line comments work as they do in C or C++.

Example:

::

      /*
       * This is a comment.
       */
      int i; // So is this.

Conditionals
~~~~~~~~~~~~

Use to conditionally execute code. Example:

::

      if(x == 0){
         inf`printf("x == 0.\n");
      }

The command can be used to specify code executed if a condition is not
true. Example:

::

      if(x == 0){
         inf`printf("x == 0.\n");
      }else if(x > 3){
         inf`printf("x > 3.\n");
      }else{
         inf`printf("x < 3 but x }= 0.\n");
      }

.. _sec:firstOnly:

First-Only Code Block
~~~~~~~~~~~~~~~~~~~~~

Code enclosed by a pair of is executed only once by a snippet.
First-only code blocks can be useful for declaring and initilizing
variables, or for any task that needs to be executed only once. Any
number of first-only code blocks can be used in a dynC code snippet.

A first-only code block is equivalent to the following:

::

      static int firstTime = 0;
      if(firstTime == 0){
        <code>
        firstTime = 1;
      }

DynC will only execute the code in a first-only section the first time a
snippet is executed. If is called multiple times and is passed the same
name, then the first-only code will be executed only once: the first
time that any of those snippets is executed. In contrast, if a snippet
is created by calling with a unique snippet name (or if a name is
unspecified), the first-only code will be executed only once upon
reaching the first point encountered in the execution of the mutatee
where the returned is inserted.

Example Touch:

::

      {%
         inf`printf("Function %s has been touched.\n", dyninst`function_name);
      %}

If is passed the code in Example Touch and the name and the returned is
inserted at the entry to function , the output would be:

::

      Function foo has been touched.
      (mutatee exit)

If the dynC code in Example Touch is passed to multiple times and each
snippet is given the same name, but is inserted at the entries of the
functions , , and respectively, the output would be:

::

      Function foo has been touched.
      (mutatee exit)

Creating the snippets with distinct names (e.g. is called with the dynC
code in Example Touch multiple times and the snippets are named , , )
would produce an output like:

::

      Function foo has been touched.
      Function bar has been touched.
      Function run has been touched.
      (mutatee exit)

A cautionary note: the use of first-only blocks can be expensive, as a
conditional must be evaluated each time the snippet is executed. If the
option is available, one may opt to insert a dynC snippet initializing
all global variables at the entry point of .

Variables
---------

DynC allows for the creation of *snippet local* variables. These
variables are in scope only within the snippet in which they are
created.

For example,

::

      int i;
      i = 5;

would create an uninitialized variable named of type integer. The value
of is then set to 5. This is equivalent to:

::

      int i = 5;

Static Variables
~~~~~~~~~~~~~~~~

Every time a snippet is executed, non-static variables are
reinitialized. To create a variable with value that persists across
executions of snippets, declare the variable as static.

Example:

::

      int i = 10;
      inf`printf("i is %d.\n", i++);

If the above is inserted at the entrance to a function that is called
four times, the output would be:

::

   i is 10.
   i is 10.
   i is 10.
   i is 10.

Adding to the variable declaration would make the value of persist
across executions:

::

      static int i = 10;
      inf`printf("i is %d.\n", i++);

Produces:

::

   i is 10.
   i is 11.
   i is 12.
   i is 13.

A variable declared in a first-only section will also behave statically,
as the initialization occurs only once.

::

      {%
         int i = 10;
      %}

.. _sec:varExplain:

An Explanation of the Internal Workings of DynC Variable Creation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

DynC uses the DyninstAPI function to allocate dynC declared variables
when is called. The variable name is mangled with the name of the
snippet passed to createSnippet. Thus, variables declared in dynC
snippets are accessible only to those snippets created by calling with
the same name.

If the variables are explicitly initialized, dynC sets the value of the
variable with a snippet. Because of this, each time the snippet is
executed, the value is reset to the initialized value. If, however the
variables are not explicitly initialized, they are automatically set to
a type-specific zero-value. Scalar variables are set to 0, and c-strings
are set to empty, null-terminated strings (i.e. ).

If a variable is declared with the keyword, then the initialization is
performed as if in a first-only block (see section
`1.2.3 <#sec:firstOnly>`__). Thus, a variable is initialized only the
first time that snippet is executed, and subsequent executions of the
variable initialization are ignored.

Creating Global Variables That Work With DynC
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

To declare a global variable that is accessible to all snippets inserted
into a mutatee, one must use the DyninstAPI method (see ). This code is
located in mutator code (*not* in dynC code).

**myMutator.C:**

::

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

::

      {%
         global`globalIntN = 0; // initialize global variable in first-only section
      %}
      inf`printf("Welcome to function %s. Global variable globalIntN = %d.\n", 
           dyninst`function_name, global`globalIntN++);

**file2:**

::

      inf`printf("Goodbye from function %s. Global variable globalIntN = %d.\n", 
           dyninst`function_name, global`globalIntN++);

When run, the output from the instrumentation would be:

::

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
  Dyninst: , , , and . Integer and c-string primitives are also
  recognized:
| Example:

::

      int i = 12;
      char *s = "hello";

Pointers
~~~~~~~~

Pointers are dereferenced with the prefix and the address of variable is
specified by . For example, in reference to the previous example from
section `1.3.4 <#dataTypes>`__, the statement would evaluate to the
character .

Arrays
~~~~~~

Arrays in DynC behave much the same way they do in C.

Example:

::

      int array[3] = {1, 2, 3};
      char *names[] = {"Mark", "Phil", "Deb", "Tracy"};
      names[2] = "Gwen" // change Deb to Gwen
      inf`printf("The seventh element of mutArray is %d.\n", global`mutArray[6]); //Mutatee array 
      if(inf`strcmp(*names, "Mark") == 0){} // This will evaluate to true. 

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
than statements, constant variables are recommended.

Functions
~~~~~~~~~

Specifying functions is beyond the scope of the DynC language.
DyninstAPI has methods for dynamically loading code into a mutatee, and
these loaded functions can be used in DynC snippets.
