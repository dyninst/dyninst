\subsection{Class CodeSource}
\label{sec:codesource}

\definedin{CodeSource.h}

The CodeSource interface is used by the ParseAPI to retrieve binary code from
an executable, library, or other binary code object; it also can provide hints
of function entry points (such as those derived from debugging symbols) to seed
the parser. The ParseAPI provides a default implementation based on the
SymtabAPI that supports many common binary formats. For details on implementing
a custom CodeSource, see Appendix \ref{sec:extend}.

\begin{apient}
virtual bool nonReturning(Address func_entry)
virtual bool nonReturning(std::string func_name)
\end{apient}
\apidesc{Looks up whether a function returns (by name or location). This information may be statically known for some code sources, and can lead to better parsing accuracy.}

\begin{apient}
virtual bool nonReturningSyscall(int /*number*/)
\end{apient}
\apidesc{Looks up whether a system call returns (by system call number). This information may be statically known for some code sources, and can lead to better parsing accuracy.}


\begin{apient}
virtual Address baseAddress()
virtual Address loadAddress()
\end{apient}
\apidesc{If the binary file type supplies non-zero base or load addresses (e.g. Windows PE), implementations should override these functions.}

\begin{apient}
std::map< Address, std::string > & linkage()
\end{apient}
\apidesc{Returns a reference to the external linkage map, which may or may not be filled in for a particular CodeSource implementation.}

\begin{apient}
struct Hint {
    Address _addr;
    CodeRegion *_region;
    std::string _name;
    Hint(Addr, CodeRegion *, std::string);
}
std::vector< Hint > const& hints()
\end{apient}
\apidesc{Returns a vector of the currently defined function entry hints.}

\begin{apient}
std::vector<CodeRegion *> const& regions()
\end{apient}
\apidesc{Returns a read-only vector of code regions within the binary represented by this code source.}

\begin{apient}
int findRegions(Address addr,
                set<CodeRegion *> & ret)
\end{apient}
\apidesc{Finds all CodeRegion objects that overlap the provided address. Some code sources (e.g. archive files) may have several regions with overlapping address ranges; others (e.g. ELF binaries) do not.}

\begin{apient}
bool regionsOverlap() 
\end{apient}
\apidesc{Indicates whether the CodeSource contains overlapping regions.}
