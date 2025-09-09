\section{Extending ParseAPI}
\label{sec:extend}

The ParseAPI is design to be a low level toolkit for binary analysis tools.
Users can extend the ParseAPI in two ways: by extending the control flow
structures (Functions, Blocks, and Edges) to incorporate additional data to
support various analysis applications, and by adding additional binary code
sources that are unsupported by the default SymtabAPI-based code source. For
example, a code source that represents a program image in memory could be
implemented by fulfilling the CodeSource and InstructionSource interfaces
described in Section \ref{sec:codesource} and below. Implementations that
extend the CFG structures need only provide a custom allocation factory in
order for these objects to be allocated during parsing.

\subsection{Instruction and Code Sources}

A CodeSource, as described above, exports its own and the InstructionSource interface for access to binary code and other details. In addition to implementing the virtual methods in the CodeSource base class (Section \ref{sec:codesource}), the methods in the pure-virtual InstructionSource class must be implemented:

\begin{apient}
virtual bool isValidAddress(const Address) 
\end{apient}
\apidesc{Returns {\scshape true} if the address is a valid code location.}

\begin{apient}
virtual void* getPtrToInstruction(const Address)
\end{apient}
\apidesc{Returns pointer to raw memory in the binary at the provided address.}

\begin{apient}
virtual void* getPtrToData(const Address)
\end{apient}
\apidesc{Returns pointer to raw memory in the binary at the provided address. The address need not correspond to an executable code region.}

\begin{apient}
virtual unsigned int getAddressWidth()
\end{apient}
\apidesc{Returns the address width (e.g. four or eight bytes) for the represented binary.}

\begin{apient}
virtual bool isCode(const Address)
\end{apient}
\apidesc{Indicates whether the location is in a code region.}

\begin{apient}
virtual bool isData(const Address)
\end{apient}
\apidesc{Indicates whether the location is in a data region.}

\begin{apient}
virtual Address offset()
\end{apient}
\apidesc{The start of the region covered by this instruction source.}

\begin{apient}
virtual Address length()
\end{apient}
\apidesc{The size of the region.}

\begin{apient}
virtual Architecture getArch()
\end{apient}
\apidesc{The architecture of the instruction source. See the Dyninst manual for details on architecture differences.}

\begin{apient}
virtual bool isAligned(const Address)
\end{apient}
\apidesc{For fixed-width instruction architectures, must return {\scshape true} if the address is a valid instruction boundary and {\scshape false} otherwise; otherwise returns {\scshape true}. This method has a default implementation that should be sufficient.}

CodeSource implementors need to fill in several data structures in the base CodeSource class:

\begin{apient}
std::map<Address, std::string> _linkage
\end{apient}
\apidesc{Entries in the linkage map represent external linkage, e.g. the PLT in ELF binaries. Filling in this map is optional.}

\begin{apient}
Address _table_of_contents
\end{apient}
\apidesc{Many binary format have ``table of contents'' structures for position
independant references. If such a structure exists, its address should be filled in.}

\begin{apient}
std::vector<CodeRegion *> _regions
Dyninst::IBSTree<CodeRegion> _region_tree
\end{apient}
\apidesc{One or more contiguous regions of code or data in the binary object must be registered with the base class. Keeping these structures in sync is the responsibility of the implementing class.}

\begin{apient}
std::vector<Hint> _hints
\end{apient}
\apidesc{CodeSource implementors can supply a set of Hint objects describing where functions are known to start in the binary. These hints are used to seed the parsing algorithm. Refer to the CodeSource header file for implementation details.}

\subsection{CFG Object Factories}
\label{sec:factories}

Users who which to incorporate the ParseAPI into large projects may need to store additional information about CFG objects like Functions, Blocks, and Edges. The simplest way to associate the ParseAPI-level CFG representation with higher-level implementation is to extend the CFG classes provided as part of the ParseAPI. Because the parser itself does not know how to construct such extended types, implementors must provide an implementation of the CFGFactory that is specialized for their CFG classes. The CFGFactory exports the following simple interface:

\begin{apient}
virtual Function * mkfunc(Address addr, 
                          FuncSource src,
                          std::string name, 
                          CodeObject * obj, 
                          CodeRegion * region,
                          Dyninst::InstructionSource * isrc)
\end{apient}
\apidesc{Returns an object derived from Function as though the provided parameters had been passed to the Function constructor. The ParseAPI parser will never invoke \code{mkfunc()} twice with identical \code{addr}, and \code{region} parameters---that is, Functions are guaranteed to be unique by address within a region.}

\begin{apient}
virtual Block * mkblock(Function * func,
                        CodeRegion * region,
                        Address addr)
\end{apient}
\apidesc{Returns an object derived from Block as though the provided parameters had been passed to the Block constructor. The parser will never invoke \code{mkblock()} with identical \code{addr} and \code{region} parameters.}

\begin{apient}
virtual Edge * mkedge(Block * src,
                      Block * trg,
                      EdgeTypeEnum type)
\end{apient}
\apidesc{Returns an object derived from Edge as though the provided parameters had been passed to the Edge constructor. The parser \emph{may} invoke \code{mkedge()} multiple times with identical parameters.}

\begin{apient}
virtual Block * mksink(CodeObject *obj,
                       CodeRegion *r)
\end{apient}
\apidesc{Returns a ``sink'' block derived from Block to which all unresolvable control flow instructions will be linked. Implementors may return a unique sink block per CodeObject or a single global sink.}

Implementors of extended CFG classes are required to override the default implementations of the \emph{mk*} functions to allocate and return the appropriate derived types statically cast to the base type. Implementors must also add all allocated objects to the following internal lists:

\begin{apient}
fact_list<Edge> edges_
fact_list<Block> blocks_
fact_list<Function> funcs_
\end{apient}
\apidesc{O(1) allocation lists for CFG types. See the CFG.h header file for list insertion and removal operations.}

Implementors \emph{may} but are \emph{not required to} override the
deallocation following deallocation routines. The primary reason to override
these routines is if additional action or cleanup is necessary upon CFG object
release; the default routines simply remove the objects from the allocation
list and invoke their destructors.

\begin{apient}
virtual void free_func(Function * f)
virtual void free_block(Block * b)
virtual void free_edge(Edge * e)
virtual void free_all()
\end{apient}
\apidesc{CFG objects should be freed using these functions, rather than delete, to avoid leaking memory.}
