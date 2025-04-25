\subsubsection{Class Frame}
\label{subsec:frame}
\definedin{frame.h}
	
The \code{Walker} class returns a call stack as a vector of \code{Frame} objects. As described
in Section~\ref{subsec:definitions}, each Frame object represents a stack frame, and contains a
return address (RA), stack pointer (SP) and frame pointer (FP). For each of
these values, optionally, it stores the location where the values were found.
Each Frame object may also be augmented with symbol information giving a
function name (or a symbolic name, in the case of non-functions) for the object
that created the stack frame.

The Frame class provides a set of functions (getRALocation, getSPLocation and
getFPLocation) that return the location in the target process' memory or
registers where the RA, SP, or FP were found. These functions may be used to
modify the stack. For example, the DyninstAPI uses these functions to change
return addresses on the stack when it relocates code. The RA, SP, and FP may be
found in a register or in a memory address on a call stack. 

\begin{apient}
static Frame *newFrame(Dyninst::MachRegisterVal ra,
                       Dyninst::MachRegisterVal sp, 
                       Dyninst::MachRegisterVal fp, 
                       Walker *walker)
\end{apient}
\apidesc{
    This method creates a new \code{Frame} object and sets the mandatory data
    members: RA, SP and FP. The new \code{Frame} object is associated with
    \code{walker}.
	
    The optional location fields can be set by the methods below.
	
    The new \code{Frame} object is created with the \code{new} operator, and the
    user should be deallocate it with the \code{delete} operator when it is no longer needed.
}

\begin{apient}
bool operator==(const Frame &)
\end{apient}
\apidesc{\code{Frame} objects have a defined equality operator.}


\begin{apient}
Dyninst::MachRegisterVal getRA() const
\end{apient}
\apidesc{
	This method returns this \code{Frame} object's return address.
}

\begin{apient}
void setRA(Dyninst::MachRegisterVal val)
\end{apient}
\apidesc{
    This method sets this \code{Frame} object's return address to \code{val}.
}

\begin{apient}
Dyninst::MachRegisterVal getSP() const
\end{apient}
\apidesc{
	This method returns this \code{Frame} object's stack pointer.
}

\begin{apient}
void setSP(Dyninst::MachRegisterVal val)
\end{apient}
\apidesc{
    This method sets this \code{Frame} object's stack pointer to \code{val}.
}

\begin{apient}
Dyninst::MachRegisterVal getFP() const
\end{apient}
\apidesc{
	This method returns this \code{Frame} object's frame pointer.
}

\begin{apient}
void setFP(Dyninst::MachRegisterVal val)
\end{apient}
\apidesc{
    This method sets this \code{Frame} object's frame pointer to \code{val}.
}

\begin{apient}
bool isTopFrame() const;                                                      
bool isBottomFrame() const;                                                   
\end{apient}
\apidesc{
  These methods return whether a \code{Frame} object is the top (e.g.,
  most recently executing) or bottom of the stack walk. }

\begin{apient}
typedef enum { 
    loc_address, 
    loc_register, 
    loc_unknown 
} storage_t;
\end{apient}

\begin{apient}
typedef struct {
    union {
        Dyninst::Address addr;
        Dyninst::MachRegister reg;
    } val;
    storage_t location;
} location_t; 
\end{apient}
\apidesc{
    The \code{location\_t} structure is used by the \code{getRALocation},
    \code{getSPLocation}, and \code{getFPLocation} methods to describe where in
    the process a \code{Frame} object's RA, SP, or FP were found. When walking a
    call stack these values may be found in registers or memory. If they were
    found in memory, the \code{location} field of \code{location\_t} will contain
    \code{loc\_address} and the \code{addr} field will contain the address where it was found.
    If they were found in a register the \code{location} field of \code{location\_t}
    will contain \code{loc\_register} and the \code{reg} field will refer to the register where
    it was found. If this \code{Frame} object was not created by a stackwalk
    (using the \code{newframe} factory method, for example), and has not had a set
    location method called, then location will contain \code{loc\_unknown}.
}

\begin{apient}
location_t getRALocation() const
\end{apient}
\apidesc{
	This method returns a \code{location\_t} describing where the RA was found.
}

\begin{apient}
void setRALocation(location_t newval)
\end{apient}
\apidesc{
	This method sets the location of where the RA was found to newval.
}

\begin{apient}
location_t getSPLocation() const
\end{apient}
\apidesc{
	This method returns a \code{location\_t} describing where the SP was found.
}

\begin{apient}
void setSPLocation(location_t newval)
\end{apient}
\apidesc{
    This method sets the location of where the SP was found to \code{newval}.
}

\begin{apient}
location_t getFPLocation() const
\end{apient}
\apidesc{
	This method returns a \code{location\_t} describing where the FP was found.
}

\begin{apient}
void setFPLocation(location_t newval)
\end{apient}
\apidesc{
    This method sets the location of where the FP was found to \code{newval}.
}

\begin{apient}
bool getName(std::string &str) const
\end{apient}
\apidesc{
This method returns a stack frame's symbolic name. Most stack frames are created
by functions, or function-like objects such as signal handlers or system calls.
This method returns the name of the object that created this stack frame. For
stack frames created by functions, this symbolic name will be the function name.
A symbolic name may not always be available for all \code{Frame} objects, such
as in cases of stripped binaries or special stack frames types.
	
The function name is obtained by using this \code{Frame} object's RA to call the
\code{SymbolLookup} callback. By default StackwalkerAPI will attempt to use the
\code{SymtabAPI} package to look up symbol names in binaries. If
\code{SymtabAPI} is not found, and no alternative \code{SymbolLookup} object is
present, then this method will return an error.
	
This method returns \code{true} on success and \code{false} on error.  
}

\begin{apient}
bool getObject(void* &obj) const
\end{apient}
\apidesc{
    In addition to returning a symbolic name (see \code{getName}) the \code{SymbolLookup}
    interface allows for an opaque object, a \code{void*}, to be associated with a
    \code{Frame} object. The contents of this \code{void*} is determined by the
\code{SymbolLookup} implementation. Under the default implementation that uses
SymtabAPI, the \code{void*} points to a Symbol object or NULL if no symbol is found.

This method returns \code{true} on success and \code{false} on error.  
}

\begin{apient}
  Walker *getWalker() const; 
\end{apient}

\apidesc{This method returns the \code{Walker} object that constructed
  this stack frame. }

\begin{apient}
  THR_ID getThread() const;                                                     
\end{apient}
\apidesc{This method returns the execution thread that the current
  \code{Frame} represents. }

\begin{apient}
FrameStepper* getStepper() const
\end{apient}
\apidesc{
    This method returns the \code{FrameStepper} object that was used to
    construct this \code{Frame} object in the \code{stepper} output parameter. 

    This method returns \code{true} on success and \code{false} on error.  
}

\begin{apient}
bool getLibOffset(std::string &lib, Dyninst::Offset &offset, void* &symtab) const
\end{apient}
\apidesc{
This method returns the DSO (a library or executable) and an offset into that
DSO that points to the location within that DSO where this frame was created.
\code{lib} is the path to the library that was loaded, and \code{offset} is the offset into
that library. The return value of the \code{symtab} parameter is dependent on the
SymbolLookup implementation-by default it will contain a pointer to a
Dyninst::Symtab object for this DSO. See the SymtabAPI Programmer's Guide for
more information on using Dyninst::Symtab objects.
}

\begin{apient}
bool nonCall() const
\end{apient}
\apidesc{
    This method returns whether a \code{Frame} object represents a function
    call; if \code{false}, the \code{Frame} may represent instrumentation, a
    signal handler, or something else.
}
