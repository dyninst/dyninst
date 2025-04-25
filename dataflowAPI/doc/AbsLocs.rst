\subsection{Class Absloc}
\label{sec:abslocs}

\definedin{Absloc.h}

Class Absloc represents an abstract location. Abstract locations can have the
following types

\begin{center}
\begin{tabular}{ll}
\toprule
Type & Meaning \\
\midrule
Register &  The abstract location represents a register \\
Stack & The abstract location represents a stack variable \\
Heap  & The abstract location represents a heap variable \\
Unknown & The default type of abstract location \\
\bottomrule
\end{tabular}
\end{center}


\begin{apient}
static Absloc makePC(Dyninst::Architecture arch);
static Absloc makeSP(Dyninst::Architecture arch);
static Absloc makeFP(Dyninst::Architecture arch);
\end{apient}
\apidesc{Shortcut interfaces for creating abstract locations representing PC,
SP, and FP}
  
\begin{apient}
bool isPC() const;
bool isSP() const;
bool isFP() const;
\end{apient}
\apidesc{Check whether this abstract location represents a PC, SP, or FP.}

\begin{apient}
Absloc();
\end{apient}
\apidesc{Create an Unknown type abstract location.}

\begin{apient}
Absloc(MachRegister reg);
\end{apient}
\apidesc{Create a Register type abstract location, representing
register \code{reg}.}

\begin{apient}
Absloc(Address addr):
\end{apient}
\apidesc{Create a Heap type abstract location, representing a heap variable at
address \code{addr}.}


\begin{apient}
Absloc(int o,
       int r,
       ParseAPI::Function *f);
\end{apient}		       
\apidesc{Create a Stack type abstract location, representing a stack variable in
the frame of function \code{f}, within abstract region \code{r}, and at offset \code{o} within the frame.}
    
\begin{apient}    
std::string format() const;
\end{apient}
\apidesc{Return the string representation of this abstract location.}

\begin{apient}
const Type& type() const;
\end{apient}
\apidesc{Return the type of this abstract location.}

\begin{apient}
bool isValid() const;
\end{apient}
\apidesc{Check whether this abstract location is valid or not. Return
\code{true} when the type is not Unknown.}

\begin{apient}
const MachRegister &reg() const;
\end{apient}
\apidesc{Return the register represented by this abstract location. This method
should only be called when this abstract location truly represents a register.}

\begin{apient}
int off() const;
\end{apient}
\apidesc{Return the offset of the stack variable represented by this abstract
location. This method should only be called when this abstract location truly
represents a stack variable.}

\begin{apient}
int region() const;
\end{apient}
\apidesc{Return the region of the stack variable represented by this abstract
location. This method should only be called when this abstract location truly
represents a stack variable.}

\begin{apient}
ParseAPI::Function *func() const;
\end{apient}
\apidesc{
Return the function of the stack variable represented by this abstract
location. This method should only be called when this abstract location truly
represents a stack variable.
}

\begin{apient}
Address addr() const;
\end{apient}
\apidesc{
Return the address of the heap variable represented by this abstract
location. This method should only be called when this abstract location truly
represents a heap variable.
}
  

\begin{apient}
bool operator<(const Absloc &rhs) const;
bool operator==(const Absloc &rhs) const;
bool operator!=(const Absloc &rhs) const;
\end{apient}
\apidesc{Comparison operators}


\subsection{Class AbsRegion}
\label{sec:absregion}

\definedin{Absloc.h}

Class AbsRegion represents a set of abstract locations of the same type.

\begin{apient}
AbsRegion();
\end{apient}
\apidesc{Create a default abstract region.}

\begin{apient}
AbsRegion(Absloc::Type t);
\end{apient}
\apidesc{Create an abstract region representing all abstract locations with
type \code{t}.}

\begin{apient}
AbsRegion(Absloc a);
\end{apient}
\apidesc{Create an abstract region representing a single abstract location
\code{a}.}

\begin{apient}
bool contains(const Absloc::Type t) const;
bool contains(const Absloc &abs) const;
bool contains(const AbsRegion &rhs) const;
\end{apient}
\apidesc{Return \code{true} if this abstract region contains abstract locations
of type \code{t}, contains abstract location \code{abs}, or contains abstract
region \code{rhs}.}

\begin{apient}
bool containsOfType(Absloc::Type t) const;
\end{apient}
\apidesc{Return \code{true} if this abstract region contains abstract locations
in type \code{t}.}

\begin{apient}
bool operator==(const AbsRegion &rhs) const;
bool operator!=(const AbsRegion &rhs) const;
bool operator<(const AbsRegion &rhs) const;
\end{apient}
\apidesc{Comparison operators}

\begin{apient}
const std::string format() const;
\end{apient}
\apidesc{Return the string representation of the abstract region.}

\begin{apient}
Absloc absloc() const;
\end{apient}
\apidesc{Return the abstract location in this abstract region.}

\begin{apient}
Absloc::Type type() const;
\end{apient}
\apidesc{Return the type of this abstract region.}

\begin{apient}
AST::Ptr generator() const;
\end{apient}
\apidesc{If this abstract region represents memory locations, this method
returns address calculation of the memory access.}

\begin{apient}
bool isImprecise() const;
\end{apient}
\apidesc{Return \code{true} if this abstract region represents more than one
abstract locations.}

\subsection{Class AbsRegionConverter}
\definedin{AbslocInterface.h}

Class AbsRegionConverter converts instructions to abstract regions.

\begin{apient}
AbsRegionConverter(bool cache, bool stack = true);
\end{apient}
\apidesc{Create an AbsRegionConverter. When \code{cache} is \code{true}, this object
will cache the conversion results for converted instructions. When \code{stack}
is \code{true}, stack analysis is used to distinguish stack variables at
different offsets. When \code{stack} is \code{false}, the stack is treated as a
single memory region.}

\begin{apient}
void convertAll(InstructionAPI::Expression::Ptr expr,
                Address addr,
                ParseAPI::Function *func,
                ParseAPI::Block *block,
                std::vector<AbsRegion> &regions);
\end{apient}
\apidesc{Create all abstract regions used in \code{expr} and return them in
\code{regions}. All registers appear in \code{expr} will have a separate
abstract region. If the expression represents a memory access, we will also
create a heap or stack abstract region depending on where it accesses.
\code{addr}, \code{func}, and \code{blocks} specify the contexts of the
expression.
If PC appears in this expression, we assume the expression is
at address \code{addr} and replace PC with a constant value \code{addr}.}

\begin{apient}
void convertAll(InstructionAPI::Instruction::Ptr insn,
                Address addr,		
                ParseAPI::Function *func,
                ParseAPI::Block *block,
                std::vector<AbsRegion> &used,
                std::vector<AbsRegion> &defined);
\end{apient}
\apidesc{Create abstract regions appearing in instruction \code{insn}. Input
abstract regions of this instructions are returned in \code{used} and output
abstract regions are returned in \code{defined}.  If the expression represents a memory access, we will also
create a heap or stack abstract region depending on where it accesses.
\code{addr}, \code{func}, and \code{blocks} specify the contexts of the
expression.
If PC appears in this expression, we assume the expression is
at address \code{addr} and replace PC with a constant value \code{addr}.}

\begin{apient} 
AbsRegion convert(InstructionAPI::RegisterAST::Ptr reg);
\end{apient}
\apidesc{Create an abstract region representing the register \code{reg}.}

\begin{apient}
AbsRegion convert(InstructionAPI::Expression::Ptr expr,
                  Address addr,
                  ParseAPI::Function *func,
                  ParseAPI::Block *block);
\end{apient}
\apidesc{Create and return the single abstract region represented by
\code{expr}.}
