.. _`example:dyninstapi-intercept-output`:

Intercept Output
################


The final example is a program called "re-tee." It takes three
arguments: the pathname of an executable program, the process id of a
running instance of the same program, and a file name. It adds code to
the running program that copies to the named file all output that the
program writes to its standard output file descriptor. In this way it
works like "tee," which passes output along to its own standard out
while also saving it in a file. The motivation for the example program
is that you run a program, and it starts to print copious lines of
output to your screen, and you wish to save that output in a file
without having to re-run the program.

.. rli:: https://raw.githubusercontent.com/dyninst/examples/master/interceptOutput/retee.cpp
   :language: cpp
   :linenos:
