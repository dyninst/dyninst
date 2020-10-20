\section{API Reference}
\label{sec:api}

This section describes the StackwalkerAPI interface. It is divided into three
sub-sections: a description of the definitions and basic types used by this API,
a description of the interface for collecting stackwalks, and a description of
the callback interface.

\subsection{Definitions and Basic Types}
The following definitions and basic types are referenced throughout the rest of
this manual.

\subsubsection{Definitions}
\label{subsec:definitions}
\begin{description}

\item [Stack Frame] A stack frame is a record of a function (or function-like
    object) invocation. When a function is executed, it may create a frame on
    the call stack. StackwalkerAPI finds stack frames and returns a description
    of them when it walks a call stack.  The following three definitions deal
    with stack frames.

\item [Bottom of the Stack] The bottom of the stack is the earliest stack frame
    in a call stack, usually a thread's initial function. The stack grows from
    bottom to the top.

\item [Top of the Stack] The top of the stack is the most recent stack frame in
    a call stack. The stack frame at the top of the stack is for the currently
    executing function.

\item [Frame Object] A Frame object is StackwalkerAPI's representation of a
    stack frame. A Frame object is a snapshot of a stack frame at a specific
    point in time. Even if a stack frame changes as a process executes, a Frame
    object will remain the same. Each Frame object is represented by an instance
    of the Frame class.

\end{description}

The following three definitions deal with fields in a Frame object. 
\begin{description}

\item [SP (Stack Pointer)] A Frame object's SP member points to the top of its
    stack frame (a stack frame grows from bottom to top, similar to a call
    stack). The Frame object for the top of the stack has a SP that is equal to
    the value in the stack pointer register at the time the Frame object was
    created. The Frame object for any other stack frame has a SP that is equal
    to the top address in the stack frame. 

\item [FP (Frame Pointer)] A Frame object's FP member points to the beginning
    (or bottom) of its stack frame. The Frame object for the top of the stack
    has a FP that is equal to the value in the frame pointer register at the
    time the Frame object was created. The Frame object for any other stack
    frame has a FP that is equal to the beginning of the stack frame.
    
\item [RA (Return Address)] A Frame object's RA member points to the location in
    the code space where control will resume when the function that created the
    stack frame resumes. The Frame object for the top of the stack has a RA that
    is equal to the value in the program counter register at the time the Frame
    object was created.  The Frame object for any other stack frame has a RA
    that is found when walking a call stack.

\end{description}

\input{fig/layout}
\input{fig/layout-armv8}

Figure~\ref{fig:layout} shows the relationship between application code, stack
frames, and Frame objects. In the figure, the source code on the left has run
through the main and foo functions, and into the bar function. It has created
the call stack in the center, which is shown as a sequence of words growing
down. The current values of the processor registers, while executing in bar, are
shown below the call stack. When StackwalkerAPI walks the call stack, it creates
the Frame objects shown on the right. Each Frame object corresponds to one of
the stack frames found in the call stack or application registers.  

The call stack in Figure~\ref{fig:layout} is similar to one that would be found
on the x86 architecture. Details about how the call stack is laid out may be
different on other architectures, but the meanings of the FP, SP, and RA fields
in the Frame objects will remain the same. The layout of the ARM64 stack may be found in Figure~\ref{fig:layout-armv8} as an example of the scope of architectural variations.


The following four definitions deal with processes involved in StackwalkerAPI.
\begin{description}

\item [Target Process] The process from which StackwalkerAPI is collecting
    stackwalks.

\item [Host Process] The process in which StackwalkerAPI code is currently
    running.

\item [First Party Stackwalk] StackwalkerAPI collects first party stackwalk when
    it walks a call stack in the same address space it is running in, i.e. the
    target process is the same as the host process.

\item [Third Party Stackwalk] StackwalkerAPI collects third party stackwalk when
    it walks the call stack in a different address space from the one it is
    running in, i.e. the target process is different from the host process. A
    third party stackwalk is usually done through a debugger interface.

\end{description}

\subsubsection{Basic Types}

\begin{apient}
typedef unsigned long Address
\end{apient}
\apidesc{
    An integer value capable of holding an address in the target process.
    Address variables should not, and in many cases cannot, be used directly as
    a pointer. It may refer to an address in a different process, and it may not
    directly match the target process' pointer representation. Address is
    guaranteed to be at least large enough to hold an address in a target
    process, but may be larger.
}
\begin{apient}
typedef ... Dyninst::PID
\end{apient}
\apidesc{
	A handle for identifying a process. On UNIX systems this will be an integer representing a PID. On Windows this will be a HANDLE object.
}
\begin{apient}
typedef ... Dyninst::THR_ID
\end{apient}
\apidesc{
	A handle for identifying a thread. On Linux platforms this is an integer referring to a TID (Thread Identifier). On Windows it is a HANDLE object.
}

\begin{apient}
class Dyninst::MachRegister
\end{apient}
\apidesc{
	A value that names a machine register.
}

\begin{apient}
typedef unsigned long Dyninst::MachRegisterVal
\end{apient}
\apidesc{
	A value that holds the contents of a register. A Dyninst::MachRegister names a specific register, while a Dyninst::MachRegisterVal represents the value that may be in that register.
}    
    
\subsection{Namespace StackwalkerAPI}
The classes in Section~\ref{sec:stackwalking-interface} and
Section~\ref{sec:callback-interface} fall under the C++ namespace
Dyninst::Stackwalker. To access them, a user should refer to them using the
Dyninst::Stackwalker:: prefix, e.g. Dyninst::Stackwalker::Walker. Alternatively,
a user can add the C++ using keyword above any references to StackwalkerAPI
objects, e.g, using namespace Dyninst and using namespace Stackwalker.
    
\subsection{Stackwalking Interface}
\label{sec:stackwalking-interface}

This section describes StackwalkerAPI's interface for walking a call stack. This
interface is sufficient for walking call stacks on all the systems and
variations covered by our default callbacks. 

To collect a stackwalk, first create new Walker object associated with the target process via

\begin{lstlisting}
    Walker::newWalker()
\end{lstlisting}
	or 
\begin{lstlisting}
    Walker::newWalker(Dyninst::PID pid)
\end{lstlisting}

Once a Walker object has been created, a call stack can be walked with the
\begin{lstlisting}
Walker::walkStack
\end{lstlisting}
method. The new stack walk is returned as a vector of Frame objects.
    
\input{API/Walker}
\input{API/Frame}

\subsection{Mapping Addresses to Libraries}
\definedin{procstate.h}

StackwalkerAPI provides an interface to access the addresses where libraries are mapped in the
target process.

\begin{apient}
typedef std::pair<std::string, Address> LibAddrPair;
\end{apient}
\apidesc{
A pair consisting of a library filename and its base address in the target process.
}

\begin{apient}
class LibraryState
\end{apient}
\apidesc{
Class providing interfaces for library tracking. Only the public query interfaces below are user-facing; the other public
methods are callbacks that allow StackwalkerAPI to update its internal state.
}

\begin{apient}
   virtual bool getLibraryAtAddr(Address addr, LibAddrPair &lib) = 0;
\end{apient}
\apidesc{
Given an address \code{addr} in the target process, returns \code{true} and sets \code{lib} to the name and base address of the library containing
addr. Given an address outside the target process, returns \code{false}.
}
\begin{apient}
   virtual bool getLibraries(std::vector<LibAddrPair> &libs, bool allow\_refresh = true) = 0;
\end{apient}
\apidesc{
Fills \code{libs} with the libraries loaded in the target process. If \code{allow\_refresh} is true, this method will attempt to ensure
that this list is freshly updated via inspection of the process; if it is false, it will return a cached list.
}
\begin{apient}
   virtual bool getLibc(LibAddrPair &lc);
\end{apient}
\apidesc{
Convenience function to find the name and base address of the standard C runtime, if present.
}
\begin{apient}
   virtual bool getLibthread(LibAddrPair &lt);
\end{apient}
\apidesc{
Convenience function to find the name and base address of the standard thread library, if present (e.g. pthreads).
}
\begin{apient}
   virtual bool getAOut(LibAddrPair &ao) = 0;
\end{apient}
\apidesc{
Convenience function to find the name and base address of the executable.
}
\subsection{Accessing Local Variables}
\definedin{local\_var.h}
	
StackwalkerAPI can be used to access local variables found in the frames of a
call stack. The StackwalkerAPI interface for accessing the values of local
variables is closely tied to the SymtabAPI interface for collecting information
about local variables--SymtabAPI handles for functions, local variables, and
types are part of this interface. 

Given an initial handle to a SymtabAPI Function object, SymtabAPI can look up
local variables contained in that function and the types of those local
variables. See the SymtabAPI Programmer's Guide for more information.

\begin{apient}
static Dyninst::SymtabAPI::Function *getFunctionForFrame(Frame f)
\end{apient}
\apidesc{
This method returns a SymtabAPI function handle for the function that created the call stack frame, f. 
}

\begin{apient}
static int glvv_Success = 0;
static int glvv_EParam = -1;
static int glvv_EOutOfScope = -2;
static int glvv_EBufferSize = -3;
static int glvv_EUnknown = -4;
\end{apient}

\begin{apient}
static int getLocalVariableValue(Dyninst::SymtabAPI::localVar *var,
                                 std::vector<Frame> &swalk,
                                 unsigned frame,
                                 void *out_buffer,
                                 unsigned out_buffer_size)
\end{apient}
\apidesc{
    Given a local variable and a stack frame from a call stack, this function
    returns the value of the variable in that frame. The local variable is
    specified by the SymtabAPI variable object, \code{var}. \code{swalk} is a
    call stack that was collected via StackwalkerAPI, and \code{frame} specifies
    an index into that call stack that contains the local variable. The value of
    the variable is stored in \code{out\_buffer} and the size of
    \code{out\_buffer} should be specified in \code{out\_buffer\_size}.
	
    A local variable only has a limited scope with-in a target process'
    execution. StackwalkerAPI cannot guarantee that it can collect the correct
    return value of a local variable from a call stack if the target process is
    continued after the call stack is collected.
	
    Finding and collecting the values of local variables is dependent on
    debugging information being present in a target process' binary. Not all
    binaries contain debugging information, and in some cases, such as for
    binaries built with high compiler optimization levels, that debugging
    information may be incorrect.

    \code{getLocalVariableValue} will return on of the following values:
    \begin{description}
    
        \item [glvv\_Success] getLocalVariableValue was able to correctly read
            the value of the given variable.
   
        \item [glvv\_EParam] An error occurred, an incorrect parameter was
            specified (frame was larger than \code{swalk.size()}, or var was not a
            variable in the function specified by frame).
    
        \item [glvv\_EOutOfScope] An error occurred, the specified variable
            exists in the function but isn't live at the current execution
            point.
    
        \item [glvv\_EBufferSize] An error occurred, the variable's value does
            not fit inside \code{out\_buffer}. 
    
        \item [glvv\_EUnknown] An unknown error occurred. It is most likely that
            the local variable was optimized away or debugging information about
            the variable was incorrect.
    
    \end{description} 
}

\subsection{Callback Interface}
\label{sec:callback-interface}
This subsection describes the Callback Interface for StackwalkerAPI. The
Callback Interface is primarily used to port StackwalkerAPI to new platforms,
extend support for new types of stack frames, or integrate StackwalkerAPI into
existing tools.

The classes in this subsection are interfaces, they cannot be instantiated.  To
create a new implementation of one of these interfaces, create a new class that
inherits from the callback class and implement the necessary methods. To use a
new ProcessState, StepperGroup, or SymbolLookup class with StackwalkerAPI,
create a new instance of the class and register it with a new Walker object
using the
\begin{lstlisting}
Walker::newWalker(ProcessState *, StepperGroup *, SymbolLookup *)
\end{lstlisting}	
factory method (see Section~\ref{subsec:walker}). To use a new FrameStepper class with
StackwalkerAPI, create a new instance of the class and register it with a
StepperGroup using the
\begin{lstlisting}
StepperGroup::addStepper(FrameStepper *)
\end{lstlisting}
method (see Section~\ref{subsec:steppergroup}).

Some of the classes in the Callback Interface have methods with default
implementations. A new class that inherits from a Callback Interface can
optionally implement these methods, but it is not required. If a method requires
implementation, it is written as a C++ pure virtual method (\code{virtual funcName() =
0}). A method with a default implementation is written as a C++ virtual method
(\code{virtual funcName()}).

\subsubsection{Default Implementations}
\label{subsec:defaults}

The classes described in the Callback Interface are C++ abstract classes, or
interfaces. They cannot be instantiated. For each of these classes
StackwalkerAPI provides one or more default implementations on each platform.
These default implementations are classes that inherit from the abstract classes
described in the Callback Interface. If a user creates a Walker object without
providing their own \code{FrameStepper}, \code{ProcessState}, and
\code{SymbolLookup} objects, then StackwalkerAPI will use the default
implementations listed in Table~\ref{table:defaults}. These
implementations are described in Section \ref{sec:framesteppers}.

\begin{table}
\begin{tabular}{| l | l | l | l | l |}
    \hline
                    &   StepperGroup    & ProcessState      &   SymbolLookup    &   FrameStepper\\
    \hline
    Linux/x86       &   1. AddrRange    &   1. ProcSelf     &   1. SwkSymtab    &   1. FrameFuncStepper\\
    Linux/x86-64    &                   &   2. ProcDebug    &                   &   2. SigHandlerStepper\\
                    &                   &                   &                   &   3. DebugStepper\\
                    &                   &                   &                   &   4. AnalysisStepper\\ 
                    &                   &                   &                   &   5. StepperWanderer\\
                    &                   &                   &                   &   6. BottomOfStackStepper\\
   \hline
    Linux/PPC       &   1. AddrRange    &   1. ProcSelf     &   1. SwkSymtab    & 1. FrameFuncStepper\\
    Linux/PPC-64    &                   &   2. ProcDebug    &                   & 2. SigHandlerStepper\\
                    &                   &                   &                   & 3. AnalysisStepper\\
    \hline
    Windows/x86     &   1. AddrRange    &   1. ProcSelf     &   1. SwkSymtab    & 1. FrameFuncStepper\\
                    &                   &   2. ProcDebug    &                   & 2. AnalysisStepper \\
                    &                   &                   &                   & 3. StepperWanderer\\
                    &                   &                   &                   & 4. BottomOfStackStepper\\
    \hline
\end{tabular}
\caption{Callback Interface Defaults}
\label{table:defaults}
\end{table}

\input{API/FrameStepper}
\input{API/StepperGroup}
\input{API/ProcessState}
\input{API/SymbolLookup}

