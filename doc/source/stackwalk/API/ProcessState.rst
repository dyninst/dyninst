\subsubsection{Class ProcessState}
\label{subsec:processstate}
\definedin{procstate.h}

The ProcessState class is a virtual class that defines an interface through
which StackwalkerAPI can access the target process. It allows access to
registers and memory, and provides basic information about the threads in the
target process. StackwalkerAPI provides two default types of \code{ProcessState}
objects: \code{ProcSelf} does a first party stackwalk, and \code{ProcDebug} does a third party
stackwalk.

A new \code{ProcessState} class can be created by inheriting from this class and
implementing the necessary methods. 

\begin{apient}
static ProcessState *getProcessStateByPid(Dyninst::PID pid)
\end{apient}
\apidesc{Given a \code{PID}, return the corresponding
  \code{ProcessState} object.}


\begin{apient}
virtual unsigned getAddressWidth() = 0;
\end{apient}
\apidesc{Return the number of bytes in a pointer for the target
  process. This value is 4 for 32-bit platforms (x86, PowerPC-32) and
  8 for 64-bit platforms (x86-64, PowerPC-64).}

\begin{apient}
 typedef enum { Arch_x86, Arch_x86_64, Arch_ppc32, Arch_ppc64 }
 Architecture;
 virtual Dyninst::Architecture getArchitecture() = 0;
\end{apient}
\apidesc{Return the appropriate architecture for the target
process.}

\begin{apient}
virtual bool getRegValue(Dyninst::MachRegister reg, 
                         Dyninst::THR_ID thread, 
                         Dyninst::MachRegisterVal &val) = 0
\end{apient}
\apidesc{
    This method takes a register name as input, \code{reg}, and returns the
    value in that register in \code{val} in the thread thread.
	
    This method returns \code{true} on success and \code{false} on error.  
}

\begin{apient}
virtual bool readMem(void *dest, Address source, size_t size) = 0
\end{apient}
\apidesc{
    This method reads memory from the target process. Parameter \code{dest} should
    point to an allocated buffer of memory at least \code{size} bytes in the host
    process. Parameter \code{source} should contain an address in the target process to
    be read from. If this method succeeds, \code{size} bytes of memory is copied from
    \code{source}, stored in \code{dest}, and \code{true} is returned. This
    method returns \code{false}
    otherwise.
}

\begin{apient}
virtual bool getThreadIds(std::vector<Dyninst::THR_ID> &threads) = 0
\end{apient}
\apidesc{
    This method returns a list of threads whose call stacks can be walked in the
    target process. Thread are returned in the \code{threads} vector. In some cases,
    such as with the default \code{ProcDebug}, this method returns all of the threads
    in the target process. In other cases, such as with \code{ProcSelf}, this method
    returns only the calling thread. 

    The first thread in the \code{threads} vector (index 0) will be used as the default
    thread if the user requests a stackwalk without specifying an thread (see
    \code{Walker::WalkStack}).
    
    This method returns \code{true} on success and \code{false} on error.  }

\begin{apient}
virtual bool getDefaultThread(Dyninst::THR_ID &default_tid) = 0
\end{apient}
\apidesc{
	This method returns the thread representing the initial process in the
    \code{default\_tid} output parameter.
    
    This method returns \code{true} on success and \code{false} on error.  
}

\begin{apient}
virtual Dyninst::PID getProcessId()
\end{apient}
\apidesc{
    This method returns a process ID for the target process. The default
    \code{ProcessState} implementations (\code{ProcDebug} and \code{ProcSelf}) will return a PID on
    UNIX systems and a HANDLE object on Windows.
}

\begin{apient}
Walker *getWalker() const;
\end{apient}
\apidesc{Return the \code{Walker} associated with the current process
  state. }
   
\begin{apient}
std::string getExecutablePath();
\end{apient}
\apidesc{
Returns the name of the executable associated with the current process state.
}

\paragraph{Class LibraryState}

\definedin{procstate.h}

\code{LibraryState} is a helper class for \code{ProcessState} that provides information about
the current DSOs (libraries and executables) that are loaded into a process'
address space. FrameSteppers frequently use the LibraryState to get the DSO
through which they are attempting to stack walk.

Each \code{Library} is represented using a \code{LibAddrPair} object, which is defined as
follows:

\begin{apient}
typedef std::pair<std::string, Dyninst::Address> LibAddrPair
\end{apient}
\apidesc{
    \code{LibAddrPair.first} refers to the file path of the library that was
    loaded, and \code{LibAddrPair.second} is the load address of that library in
    the process' address space. The load address of a library can be added to a
    symbol offset from the file in order to get the absolute address of a
    symbol.
}

\begin{apient}
virtual bool getLibraryAtAddr(Address addr, LibAddrPair &lib) = 0
\end{apient}
\apidesc{
    This method returns a DSO, using the \code{lib} output parameter, that is
    loaded over address \code{addr} in the current process.

    This method returns \code{false} if no library is loaded over \code{addr} or
    an error occurs, and \code{true} if it successfully found a library.
}

\begin{apient}
virtual bool getLibraries(std::vector<LibAddrPair> &libs) = 0
\end{apient}
\apidesc{
	This method returns all DSOs that are loaded into the process' address space
    in the output vector parameter, \code{libs}. 
    
    This method returns \code{true} on success and \code{false} on error.  
}

\begin{apient}
virtual void notifyOfUpdate() = 0
\end{apient}
\apidesc{
    This method is called by the \code{ProcessState} when it detects a change in
    the process' list of loaded libraries. Implementations of
    \code{LibraryStates} should use this method to refresh their lists of loaded libraries.
}

\begin{apient}
virtual Address getLibTrapAddress() = 0
\end{apient}
\apidesc{
    Some platforms that implement the System/V standard (Linux)
    use a trap event to determine when a process loads a library. A
    trap instruction is inserted into a certain address, and that trap will
    execute whenever the list of loaded libraries change. 
	
    On System/V platforms this method should return the address where a trap
    should be inserted to watch for libraries loading and unloading. The
    ProcessState object will insert a trap at this address and then call
    notifyOfUpdate when that trap triggers.
	
    On non-System/V platforms this method should return 0.  
}
