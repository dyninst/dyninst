Class LoopTreeNode
------------------

The LoopTreeNode class provides a tree interface to a collection of
instances of class Loop contained in a function. The structure of the
tree follows the nesting relationship of the loops in a function. Each
LoopTreeNode contains a pointer to a loop (represented by Loop), and a
set of sub-loops (represented by other LoopTreeNode objects). The field
at the root node is always since a function may contain multiple outer
loops. The field is never at any other node since it always corresponds
to a real loop. Therefore, the outer most loops in the function are
contained in the vector of of the root.

Each instance of LoopTreeNode is given a name that indicates its
position in the hierarchy of loops. The name of each outermost loop
takes the form of , where is an integer from 1 to n, where n is the
number of outer loops in the function. Each sub-loop has the name of its
parent, followed by a , where is 1 to m, where m is the number of
sub-loops under the outer loop. For example, consider the following C
function:

::


   void foo() {
     int x, y, z, i;
     for (x=0; x<10; x++) {
       for (y = 0; y<10; y++)
         ...
       for (z = 0; z<10; z++)
         ...
     }
     for (i = 0; i<10; i++) {
        ...
     }
   }

The function will have a root LoopTreeNode, containing a NULL loop entry
and two LoopTreeNode children representing the functions outermost
loops. These children would have names and , respectively representing
the and loops. has no children. has two child LoopTreeNode objects,
named and , respectively representing the and loops.

Loop \*loop;

std::vector<LoopTreeNode \*> children;

const char \* name();

const char \* getCalleeName(unsigned int i)

unsigned int numCallees()

bool getCallees(vector<Function \*> &v);

Loop \* findLoop(const char \*name);
