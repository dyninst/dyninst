\section{API Reference - Dynamic Components}

Unlike the static components discussed in Section \ref{sec:symtabAPI}, which operate on files,
SymtabAPI's dynamic components operate on a process. The dynamic components currently consist of the Dynamic Address Translation system, which translates between absolute addresses in a running process and static SymtabAPI objects. 

\subsection{Class AddressLookup}

The \code{AddressLookup} class provides a mapping interface for determining the address in a process where a SymtabAPI object is loaded. A single dynamic library may load at different addresses in different processes. The `address' fields in a dynamic library's symbol tables will contain offsets rather than absolute addresses. These offsets can be added to the library's load address, which is computed at runtime, to determine the absolute address where a symbol is loaded. 

The \code{AddressLookup} class examines a process and finds its dynamic libraries and executables and each one's load address. This information can be used to map between SymtabAPI objects and absolute addresses. Each \code{AddressLookup} instance is associated with one process. An \code{AddressLookup} object can be created to work with the currently running process or a different process on the same system.

On the Linux platform the \code{AddressLookup} class needs to read from the process' address space to determine its shared objects and load addresses. By default, \code{AddressLookup} will attach to another process using a debugger interface to read the necessary information, or simply use \code{memcpy} if reading from the current process. The default behavior can be changed by implementing a new ProcessReader class and passing an instance of it to the
create\code{AddressLookup} factor constructors. The ProcessReader class is discussed in more detail in Section \ref{subsec:ProcessReader}.

When an \code{AddressLookup} object is created for a running process it takes a snapshot of the process' currently loaded libraries and their load addresses. This snapshot is used to answer queries into the \code{AddressLookup} object, and is not automatically updated when the process loads or unloads libraries. The refresh function can be used to updated an \code{AddressLookup} object's view of its process.

\begin{apient}
static AddressLookup *createAddressLookup(ProcessReader *reader = NULL)
\end{apient}
\apidesc{
This factory constructor creates a new \code{AddressLookup} object associated with the process that called this function. The returned \code{AddressLookup} object should be cleaned with the delete operator when it is no longer needed.
If the reader parameter is non-NULL on Linux then the new \code{AddressLookup} object will use reader to read from the target process.
This function returns the new \code{AddressLookup} object on success and \code{NULL} on error.
}

\begin{apient}
static AddressLookup *createAddressLookup(PID pid, 
                                          ProcessReader *reader = NULL)
\end{apient}
\apidesc{
This factory constructor creates a new \code{AddressLookup} object associated with the process referred to by \code{pid}. The returned \code{AddressLookup} object should be cleaned with the delete operator when it is no longer needed. If the \code{reader} parameter is non-NULL on Linux then the new \code{AddressLookup} object will use it to read from the target process. This function returns the new \code{AddressLookup} object on success and \code{NULL} on error.
}

\begin{apient}
typedef struct {
    std::string name;
    Address codeAddr;
    Address dataAddr;
} LoadedLibrary;
\end{apient}

\begin{apient}
static AddressLookup *createAddressLookup(const std::vector<LoadedLibrary> &ll)
\end{apient}
\apidesc{
This factory constructor creates a new \code{AddressLookup} associated with a previously collected list of libraries from a process. The list of libraries can initially be collected with the \code{getLoadAddresses} function. The list can then be used with this function to re-create the \code{AddressLookup} object, even if the original process no longer exists. This can be useful for off-line address lookups, where only the load addresses are collected while the process exists and then all address translation is done after the process has terminated.
This function returns the new \code{AddressLookup} object on success and \code{NULL} on error.
}

\begin{apient}
bool getLoadAddresses(std::vector<LoadedLibrary> &ll)
\end{apient}
\apidesc{
This function returns a vector of LoadedLibrary objects that can be used by the
\texttt{createAddressLookup(const std::vector<LoadedLibrary> \&ll)} function to create a new \code{AddressLookup} object. This function is usually used as part of an off-line address lookup mechanism. 
This function returns \code{true} on success and \code{false} on error.
}

\begin{apient}
bool refresh()
\end{apient}
\apidesc{
When a \code{AddressLookup} object is initially created it takes a snapshot of the libraries currently loaded in a process, which is then used to answer queries into this API. As the process runs more libraries may be loaded and unloaded, and this snapshot may become out of date. 
An \code{AddressLookup}'s view of a process can be updated by calling this function, which causes it to examine the process for loaded and unloaded objects and update its data structures accordingly.
This function returns \code{true} on success and \code{false} on error.
}

\begin{apient}
bool getAddress(Symtab *tab,
                Symbol *sym,
                Address &addr)
\end{apient}
\apidesc{
Given a \code{Symtab} object, \code{tab}, and a symbol, \code{sym}, this function returns the address, \code{addr}, where the symbol can be found in the process associated with this \code{AddressLookup}. 
This function returns \code{true} if it was able to successfully lookup the address of \code{sym} and \code{false} otherwise.
}

\begin{apient}
bool getAddress(Symtab *tab,
                Offset off,
                Address &addr)
\end{apient}
\apidesc{
Given a \code{Symtab} object, \code{tab}, and an offset into that object, \code{off}, this function returns the address, \code{addr}, of that location in the process associated with this \code{AddressLookup}. 
This function returns \code{true} if it was able to successfully lookup the address of sym and \code{false} otherwise.
}

\begin{apient}
bool getSymbol(Address addr,
               Symbol * &sym,
               Symtab* &tab,
               bool close = false)
\end{apient}
\apidesc{
Given an address, \code{addr}, this function returns the \code{Symtab} object, \code{tab}, and \code{Symbol}, \code{sym}, that reside at that address. If the close parameter is \code{true} then \code{getSymbol} will return the nearest symbol that comes before \code{addr}; this can be useful when looking up the function that resides at an address.
This function returns \code{true} if it was able to find a symbol and \code{false} otherwise.
}

\begin{apient}
bool getOffset(Address addr,
               Symtab* &tab,
               Offset &off)
\end{apient}
\apidesc{
Given an address, \code{addr}, this function returns the \code{Symtab} object, \code{tab}, and an offset into \code{tab}, \code{off}, that reside at that address. 
This function returns \code{true} on success and \code{false} otherwise.
}

\begin{apient}
bool getOffset(Address addr,
               LoadedLibrary &lib,
               Offset &off)
\end{apient}
\apidesc{
As above, but returns a \code{LoadedLibrary} data structure instead of a Symtab. 
}


\begin{apient}
bool getAllSymtabs(std::vector<Symtab *> &tabs)
\end{apient}
\apidesc{
This function returns all \code{Symtab} objects that are contained in the process represented by this \code{AddressLookup} object. This will include the process's executable and all shared objects loaded by this process. 
This function returns \code{true} on success and \code{false} otherwise.
}

\begin{apient}
bool getLoadAddress(Symtab *sym,
                    Address &load_address)
\end{apient}
\apidesc{
Given a \code{Symtab} object, \code{sym}, that resides in the process associated with this \code{AddressLookup}, this function returns \code{sym}'s load address.
On systems where an object can have one load address for its code and one for its data, this function will return the code's load address. Use \code{getDataLoadAddress} to get the data load address.
This function returns \code{true} on success and \code{false} otherwise.
}

\begin{apient}
bool getDataLoadAddress(Symtab *sym,
                        Address &load_addr)
\end{apient}
\apidesc{
Given a Symtab object, \code{sym}, this function returns the load address of its data section. This function returns \code{true} on success and \code{false} otherwise.}


\subsection{Class ProcessReader}
\label{subsec:ProcessReader}

The implementation of the \code{AddressLookup} on Linux requires it to be able to read from the target process's address space. By default, reading from another process on the same system this is done through the operating system debugger interface. A user can provide their own process reading mechanism by implementing a child of the \code{ProcessReader} class and passing it to the \code{AddressLookup} constructors. 
The API described in this section is an interface that a user can implement. With the exception of the \code{ProcessReader} constructor, these functions should not be called by user code.

The \code{ProcessReader} is defined, but not used, on non-Linux systems.

\begin{apient}
ProcessReader()
\end{apient}
\apidesc{
This constructor for a \code{ProcessReader} should be called by any child class constructor.
}

\begin{apient}
virtual bool ReadMem(Address traced,
                     void *inSelf,
                     unsigned size) = 0
\end{apient}
\apidesc{
This function should read \code{size} bytes from the address at \code{traced} into the buffer pointed to by \code{inSelf}. 
This function must return \code{true} on success and \code{false} on error.
}

\begin{apient}
virtual bool GetReg(MachRegister reg,
                    MachRegisterVal &val) = 0
\end{apient}
\apidesc{
This function reads from the register specified by \code{reg} and places the result in \code{val}. It must return \code{true} on success and \code{false} on failure. 
}

