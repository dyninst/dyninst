.. _sec:stackanalysis:

Class StackAnalysis
-------------------

The StackAnalysis interface is used to determine the possible stack
heights of abstract locations at any instruction in a function. Due to
there often being many paths through the CFG to reach a given
instruction, abstract locations may have different stack heights
depending on the path taken to reach that instruction. In other cases,
StackAnalysis is unable to adequately determine what is contained in an
abstract location. In both situations, StackAnalysis is conservative in
its reported stack heights. The table below explains what the reported
stack heights mean.

=====================
==============================================================================================================================================================================================================
Reported stack height Meaning
=====================
==============================================================================================================================================================================================================
TOP                   On all paths to this instruction, the specified abstract location contains a value that does not point to the stack.
\                    
*x* (some number)     On at least one path to this instruction, the specified abstract location has a stack height of *x*. On all other paths, the abstract location either has a stack height of *x* or doesn’t point to the stack.
\                    
BOTTOM                There are three possible meanings:
                     
                      #. On at least one path to this instruction, StackAnalysis was unable to determine whether or not the specified abstract location points to the stack.
                     
                      #. On at least one path to this instruction, StackAnalysis determined that the specified abstract location points to the stack but could not determine the exact stack height.
                     
                      #. On at least two paths to this instruction, the specified abstract location pointed to different parts of the stack.
=====================
==============================================================================================================================================================================================================

StackAnalysis(ParseAPI::Function \*f)

StackAnalysis(ParseAPI::Function \*f, const std::map<Address, Address>
&crm, const std::map<Address, TransferSet> &fs)

StackAnalysis::Height find(ParseAPI::Block \*b, Address addr, Absloc
loc)

StackAnalysis::Height findSP(ParseAPI::Block \*b, Address addr)
StackAnalysis::Height findFP(ParseAPI::Block \*b, Address addr)

void findDefinedHeights(ParseAPI::Block \*b, Address addr,
std::vector<std::pair<Absloc, StackAnalysis::Height>> &heights)

bool canGetFunctionSummary()

bool getFunctionSummary(TransferSet &summary)

Class StackAnalysis::Height
---------------------------

The Height class is used to represent the abstract notion of stack
heights. Every Height object represents a stack height of either TOP,
BOTTOM, or *x*, where *x* is some integral number. The Height class also
defines methods for comparing, combining, and modifying stack heights in
various ways.

typedef signed long Height_t

=========== =========== =======================================
Method name Return type Method description
=========== =========== =======================================
height      Height_t    This stack height as an integral value.
format      std::string This stack height as a string.
isTop       bool        True if this stack height is TOP.
isBottom    bool        True if this stack height is BOTTOM.
=========== =========== =======================================

Height(const Height_t h)

Height()

bool operator<(const Height &rhs) const bool operator>(const Height
&rhs) const bool operator<=(const Height &rhs) const bool
operator>=(const Height &rhs) const bool operator==(const Height &rhs)
const bool operator!=(const Height &rhs) const

Height &operator+=(const Height &rhs) Height &operator+=(const signed
long &rhs) const Height operator+(const Height &rhs) const const Height
operator+(const signed long &rhs) const const Height operator-(const
Height &rhs) const
