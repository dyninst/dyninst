\subsubsection{Class SymbolLookup}
\definedin{symlookup.h}

The \code{SymbolLookup} virtual class is an interface for associating a symbolic
name with a stack frame. Each \code{Frame} object contains an address (the RA)
pointing into the function (or function-like object) that created its stack
frame. However, users do not always want to deal with addresses when symbolic
names are more convenient. This class is an interface for mapping a \code{Frame} object's RA into a name.

In addition to getting a name, this class can also associate an opaque object
(via a \code{void*}) with a Frame object. It is up to the \code{SymbolLookup}
implementation what to return in this opaque object.

The default implementation of \code{SymbolLookup} provided by StackwalkerAPI
uses the \code{SymLite} tool to lookup symbol names. It returns a Symbol
object in the anonymous \code{void*}.

\begin{apient}
SymbolLookup(std::string exec_path = "");
\end{apient}
\apidesc{Constructor for a \code{SymbolLookup} object.}

\begin{apient}
virtual bool lookupAtAddr(Address addr, 
                          string &out_name, 
                          void* &out_value) = 0
\end{apient}
\apidesc{
    This method takes an address, \code{addr}, as input and returns the function name,
    \code{out\_name}, and an opaque value, \code{out\_value}, at that address. Output parameter
    \code{out\_name} should be the name of the function that contains
    \code{addr}. Output
    parameter \code{out\_value} can be any opaque value determined by the
    \code{SymbolLookup}
    implementation. The values returned are used by the \code{Frame::getName} and
    \code{Frame::getObject} functions.

    This method returns \code{true} on success and \code{false} on error.
}

\begin{apient}
virtual Walker *getWalker()
\end{apient}
\apidesc{
    This method returns the \code{Walker} object associated with this
    \code{SymbolLookup}.
}

\begin{apient}
virtual ProcessState *getProcessSate()
\end{apient}
\apidesc{
    This method returns the \code{ProcessState} object associated with this
    \code{SymbolLookup}.
}

