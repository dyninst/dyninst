\subsubsection{Class Walker}
\label{subsec:walker}
\definedin{walker.h}

The \code{Walker} class allows users to walk call stacks and query basic information
about threads in a target process. The user should create a \code{Walker} object for
each process from which they are walking call stacks. Each \code{Walker} object is
associated with one process, but may walk call stacks on multiple threads within
that process. The \code{Walker} class allows users to query for the threads available
for walking, and it allows you to specify a particular thread whose call stack
should be walked. Stackwalks are returned as a vector of Frame objects. 

Each Walker object contains three objects: 
\begin{itemize}
    \item ProcessState
    \item StepperGroup
    \item SymbolLookup
\end{itemize}

These objects are part of the Callback Interface and can be used to customize
StackwalkerAPI. The \code{ProcessState} object tells \code{Walker} how to access data in the
target process, and it determines whether this \code{Walker} collects first party or
third party stackwalks. \code{Walker} will pick an appropriate default \code{ProcessState}
object based on which factory method the users calls. The \code{StepperGroup} object is
used to customize how the \code{Walker} steps through stack frames. The
\code{SymbolLookup}
object is used to customize how StackwalkerAPI looks up symbolic names of the
function or object that created a stack frame.

\begin{apient}
static Walker *newWalker()
static Walker *newWalker(Dyninst::PID pid)
static Walker *newWalker(Dyninst::PID pid, std::string executable)
static Walker *newWalker(Dyninst::ProcControlAPI::Process::ptr proc);
static Walker *newWalker(std::string executable,
                         const std::vector<std::string> &argv)
static Walker *newWalker(ProcessState *proc,
                         StepperGroup *steppergroup = NULL ,
                         SymbolLookup *lookup = NULL)
\end{apient}
\apidesc{
These factory methods return new Walker objects:
\begin{itemize}
\item The first takes no arguments and returns a first-party stackwalker.
\item The second takes a PID representing a running process and returns a third-party stackwalker on that
process.
\item The third takes the name of the executing binary in addition to the PID and also returns a third-party
stackwalker on that process.
\item The fourth takes a ProcControlAPI process object and returns a third-party stackwalker.
\item The fifth takes the name of an executable and its arguments, creates the process, and returns a
third-party stackwalker.
\item The sixth takes a ProcessState pointer representing a running process as well as user-defined
StepperGroup and SymbolLookup pointers. It can return both first-party and third-party Walkers,
depending on the ProcessState parameter.
\end{itemize}

Unless overriden with the sixth variant, the new Walker object uses the default StepperGroup and
SymbolLookup callbacks for the current platform. First-party walkers use the ProcSelf callback
for its ProcessState object. Third-party walkers use ProcDebug instead. See Section 3.5.1 for
more information about defaults in the Callback Interface.

This method returns NULL if it was unable to create a new Walker object. The new Walker object
was created with the new operator, and should be deallocated with the delete operator when it
is no longer needed.
}
 
\begin{apient}
static bool newWalker(const std::vector<Dyninst::PID> &pids,
                      std::vector<Walker *> &walkers_out)
static bool newWalker(const std::vector<Dyninst::PID> &pids,
                      std::vector<Walker *> &walkers_out,
                      std::string executable)
\end{apient}
\apidesc{
 This method attaches to a group of processes and returns a vector of Walker objects that perform
third-party stackwalks. As above, the first variant takes a list of PIDs and attaches to those
processes; the second variant also specifies the executable binary.}

\begin{apient}
bool walkStack(std::vector<Frame> &stackwalk,	 
               Dyninst::THR_ID thread = NULL_THR_ID)
\end{apient}
\apidesc{
    This method walks a call stack in the process associated with this \code{Walker}.
    The call stack is returned as a vector of \code{Frame} objects in stackwalk. The
    top of the stack is returned in index 0 of stackwalk, and the bottom of the
    stack is returned in index \code{stackwalk.size()-1}.

    A stackwalk can be taken on a specific thread by passing a value
    in the thread parameter. If \code{thread} has the value
    \code{NULL\_THR\_ID}, then a default thread will be chosen. When
    doing a third party stackwalk, the default thread will be the
    process' initial thread. When doing a first party stackwalk, the
    default thread will be the thread that called
    \code{walkStack}. The default StepperGroup provided to a Walker
    will support collecting call stacks from almost all types of
    functions, including signal handlers and optimized, frameless
    functions.

    This method returns \code{true} on success and \code{false} on failure.
}
 
\begin{apient}
bool walkStackFromFrame(std::vector<Frame> &stackwalk, const Frame &frame)
\end{apient}
\apidesc{
    This method walks a call stack starting from the given stack frame,
    \code{frame}.
    The call stack will be output in the \code{stackwalk} vector, with frame stored in
    index 0 of \code{stackwalk} and the bottom of the stack stored in index
    \code{stackwalk.size()-1}.

    This method returns \code{true} on success and \code{false} on failure.
}
 
\begin{apient}
bool walkSingleFrame(const Frame &in, Frame &out)
\end{apient}
\apidesc{
    This methods walks through single frame, \code{in}. Parameter \code{out}
    will be set to \code{in}'s caller frame.

    This method returns \code{true} on success and \code{false} on failure.
}
 
\begin{apient}
bool getInitialFrame(Frame &frame, Dyninst::THR_ID thread = NULL_THR_ID)
\end{apient}
\apidesc{
    This method returns the \code{Frame} object on the top of the stack in parameter
    frame. Under \code{walkStack}, \code{frame} would be the one returned in index 0 of the
    \code{stackwalk} vector.  A stack frame can be found on a specific thread by
    passing a value in the thread parameter. If \code{thread} has the value
    \code{NULL\_THR\_ID}, then a default thread will be chosen. When doing a third party
    stackwalk, the default thread will be the process' initial thread. When
    doing a first party stackwalk, the default thread will be the thread that
    called \code{getInitialFrame}. 

    This method returns \code{true} on success and \code{false} on failure.
}
 
\begin{apient}
bool getAvailableThreads(std::vector<Dyninst::THR_ID> &threads)
\end{apient}
\apidesc{
    This method returns a vector of threads in the target process upon which
    StackwalkerAPI can walk call stacks. The threads are returned in output
    parameter \code{threads}. Note that this method may return a subset of the actual
    threads in the process. For example, when walking call stacks on the current
    process, it is only legal to walk the call stack on the currently running
    thread. In this case, \code{getAvailableThreads} returns a vector containing only
    the current thread.

    This method returns \code{true} on success and \code{false} on failure.
}
 
\begin{apient}
ProcessState *getProcessState() const
\end{apient}
\apidesc{
    This method returns the \code{ProcessState} object associated with this \code{Walker}.
}
 
\begin{apient}
StepperGroup *getStepperGroup() const
\end{apient}
\apidesc{
    This method returns the \code{StepperGroup} object associated with this \code{Walker}.
}
 
\begin{apient}
SymbolLookup *getSymbolLookup() const
\end{apient}
\apidesc{
    This method returns the \code{SymbolLookup} object associated with this \code{Walker}.
}

\begin{apient}
bool addStepper(FrameStepper *stepper)
\end{apient}
\apidesc{
This method adds a provided FrameStepper to those used by the Walker.
}

\begin{apient}
static SymbolReaderFactory *getSymbolReader()
\end{apient}
\apidesc{This method returns a factory for creating process-specific symbol readers. Unlike the above
methods it is global across all Walkers and is thus defined static.}

\begin{apient}
static void setSymbolReader(SymbolReaderFactory *);                       
\end{apient}
\apidesc{Set the symbol reader factory used when creating
  \code{Walker} objects.}

\begin{apient}
static void version(int &major, int &minor, int &maintenance)
\end{apient}

\apidesc{This method returns version information (e.g., 8, 0, 0 for
  the 8.0 release).}
