\section{Callback Interface Default Implementations}

StackwalkerAPI provides one or more default implementations of each of the
callback classes described in Section 3.5. These implementations are used by a
default configuration of StackwalkerAPI. 

\subsection{Debugger Interface}
\label{subsec:debugger}

This section describes how to use StackwalkerAPI for collecting 3rd party stack
walks. In 3rd party mode StackwalkerAPI uses the OS's debugger interface to
connect to another process and walk its call stacks. As part of being a debugger
StackwalkerAPI receives and needs to handle debug events. When a debugger event
occurs, StackwalkerAPI must get control of the host process in order to receive
the debugger event and continue the target process. 

To illustrate the complexities with running in 3rd party mode, consider the
follow code snippet that uses StackwalkerAPI to collect a stack walk every five
seconds. 

\begin{lstlisting}
Walker *walker = Walker::newWalker(pid);
std::vector<Frame> swalk;
for (;;) {
		walker->walkStack(swalk);
		sleep(5);
}
\end{lstlisting}

StackwalkerAPI is running in 3rd party mode, since it attached to the target
process, \code{pid}. As the target process runs it may be generating debug events such
a thread creation and destruction, library loads and unloads, signals,
forking/execing, etc. When one of these debugger events is generated the OS will
pause the target process and send a notice to the host process. The target
process will remain paused until the host process handles the debug event and
resumes the target process.

In the above example the host process is spending almost all of its time in the
sleep call. If a debugger event happens during the sleep, then StackwalkerAPI
will not be able to get control of the host process and handle the event for up
to five seconds. This will cause long pauses in the target process and lead to a
potentially very large slowdown.

To work around this problem StackwalkerAPI provides a notification file
descriptor. This file descriptor represents a connection between the
StackwalkerAPI library and user code. StackwalkerAPI will write a single byte to
this file descriptor when a debug event occurs, thus notifying the user code
that it needs to let StackwalkerAPI receive and handle debug events. The user
code can use system calls such as select to watch for events on the notification
file descriptor.

The following example illustrates how to properly use StackwalkerAPI to collect
a stack walk from another process at a five second interval. Details on the
\code{ProcDebug} class, \code{getNotificationFD} method, and
\code{handleDebugEvent} method can be
found in Section~\ref{subsubsec:procdebug}. See the UNIX man pages for more information on the
\code{select} system call. Note that this example does not include all of the proper
error handling and includes that should be present when using \code{select}.

\begin{lstlisting}
Walker *walker = Walker::newWalker(pid);
ProcDebug *debugger = (ProcDebug *) walker->getProcessState();
std::vector<Frame> swalk;
for (;;) {
    walker->walkStack(swalk);		
    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    int max = 1;
    fd_set readfds, writefds, exceptfds;
    FD_ZERO(&readfds); FD_ZERO(&writefds); FD_ZERO(&exceptfds);
    FD_SET(ProcDebug::getNotificationFD(), &readfds);
    for (;;) {
        int result = select(max, &readfds, &writefds, &exceptfds, &timeout);
        if (FD_ISSET(ProcDebug::getNotificationFD(), readfds)) {
            //Debug event
            ProcDebug::handleDebugEvent();
        }
        if (result == 0) {
            //Timeout
            break;
        }
    }
}
\end{lstlisting}


\subsubsection{Class ProcDebug}
\label{subsubsec:procdebug}
\definedin{procstate.h}

Access to StackwalkerAPI's debugger is through the \code{ProcDebug} class, which
inherits from the \code{ProcessState} interface. The easiest way to get at a
\code{ProcDebug} object is to cast the return value of
\code{Walker::getProcessState} into a \code{ProcDebug}. C++'s
\code{dynamic\_cast} operation can be used to test if a \code{Walker} uses the
\code{ProcDebug} interface:

\begin{lstlisting}
ProcDebug *debugger;
debugger = dynamic_cast<ProcDebug*>(walker->getProcessState());
if (debugger != NULL) {
    //3rd party
    ...
} else {
    //1st party
    ...
}
\end{lstlisting}

In addition to the handling of debug events, described in
Section~\ref{subsec:debugger}, the \code{ProcDebug} class provides a process
control interface; users can pause and resume process or threads, detach from a
process, and test for events such as process death. As an implementation of the
\code{ProcessState} class, \code{ProcDebug} also provides all of the
functionality described in Section~\ref{subsec:processstate}.

\begin{apient}
virtual bool pause(Dyninst::THR_ID tid = NULL_THR_ID)
\end{apient}
\apidesc{
    This method pauses a process or thread. The paused object will not resume
    execution until \code{ProcDebug::resume} is called. If the \code{tid} parameter is not
    \code{NULL\_THR\_ID} then StackwalkerAPI will pause the thread specified by
    \code{tid}. If
    \code{tid} is \code{NULL\_THR\_ID} then StackwalkerAPI will pause every thread in the
    process.

    When StackwalkerAPI collects a call stack from a running thread it first
    pauses the thread, collects the stack walk, and then resumes the thread.
    When collecting a call stack from a paused thread StackwalkerAPI will
    collect the stack walk and leave the thread paused. This method is thus
    useful for pausing threads before stack walks if the user needs to keep the
    returned stack walk synchronized with the current state of the thread.

    This method returns \code{true} if successful and \code{false} on error.
}

\begin{apient}
virtual bool resume(Dyninst::THR_ID tid = NULL_THR_ID)
\end{apient}
\apidesc{
    This method resumes execution on a paused process or thread. This method
    only resumes threads that were paused by the \code{ProcDebug::pause} call, using it
    on other threads is an error. If the \code{tid} parameter is not
    \code{NULL\_THR\_ID} then
    StackwalkerAPI will resume the thread specified by \code{tid}. If \code{tid} is
    \code{NULL\_THR\_ID} then StackwalkerAPI will resume all paused threads in the
    process.

    This method returns \code{true} if successful and \code{false} on error.
}

\begin{apient}
virtual bool detach(bool leave_stopped = false)
\end{apient}
\apidesc{
    This method detaches StackwalkerAPI from the target process. StackwalkerAPI
    will no longer receive debug events on this target process and will no
    longer be able to collect call stacks from it. This method invalidates the
    associated \code{Walker} and \code{ProcState} objects, they should be cleaned using C++'s
    \code{delete} operator after making this call. It is an error to attempt to do
    operations on these objects after a detach, and undefined behavior may
    result.

    If the \code{leave\_stopped} parameter is \code{true} StackwalkerAPI will detach from the
    process but leave it in a paused state so that it does resume progress. This
    is useful for attaching another debugger back to the process for further
    analysis. The \code{leave\_stopped} parameter is not supported on the Linux platform
    and its value will have no affect on the detach call.

    This method returns \code{true} if successful and \code{false} on error.
}

\begin{apient}
virtual bool isTerminated()
\end{apient}
\apidesc{
    This method returns \code{true} if the associated target process has
    terminated and \code{false} otherwise. A target process may terminate itself by
    calling exit, returning from main, or receiving an unhandled signal.
    Attempting to collect stack walks or perform other operations on a
    terminated process is illegal an will lead to undefined behavior.
	
    A process termination will also be signaled through the notification FD.
Users should check processes for the isTerminated state after returning from
handleDebugEvent.  
}

\begin{apient}
static int getNotificationFD()
\end{apient}
\apidesc{
    This method returns StackwalkerAPI's notification FD. The notification FD is
    a file descriptor that StackwalkerAPI will write a byte to whenever a debug
    event occurs that need. If the user code sees a byte on this file descriptor
    it should call \code{handleDebugEvent} to let StackwalkerAPI handle the debug
    event. Example code using \code{getNotificationFD} can be found in
    Section~\ref{subsec:debugger}. 

	StackwalkerAPI will only create one notification FD, even if it is attached to multiple 3rd party target processes.
}

\begin{apient}
static bool handleDebugEvent(bool block = false)
\end{apient}
\apidesc{
    When this method is called StackwalkerAPI will receive and handle all
    pending debug events from each 3rd party target process to which it is
    attached. After handling debug events each target process will be continued
    (unless it was explicitly stopped by the ProcDebug::pause method) and any
    bytes on the notification FD will be cleared. It is generally expected that
    users will call this method when a event is sent to the notification FD,
    although it can be legally called at any time.

    If the \code{block} parameter is \code{true}, then \code{handleDebugEvents}
    will block until it has handled at least one debug event. If the block
    parameter is \code{false}, then handleDebugEvents will handle any currently pending
    debug events or immediately return if none are available. 

    StackwalkerAPI may receive process exit events for target processes while
    handling debug events. The user should check for any exited processes by
    calling \code{ProcDebug::isTerminated} after handling debug events.

    This method returns \code{true} if successful and \code{false} on error.
}
    
\subsection{FrameSteppers}\label{sec:framesteppers} 
\definedin{framestepper.h}

StackwalkerAPI ships with numerous default implementations of the
\code{FrameStepper}
class. Each of these \code{FrameStepper} implementations allow StackwalkerAPI to walk a
type of call frames. Section~\ref{subsec:defaults} describes which
\code{FrameStepper} implementations
are available on which platforms. This sections gives a brief description of
what each \code{FrameStepper} implementation does. Each of the following classes
implements the \code{FrameStepper} interface described in
Section~\ref{subsec:framestepper}, so we do not
repeat the API description for the classes here.

Several of the \code{FrameStepper}s use helper classes (see
\code{FrameFuncStepper} as an
example). Users can further customize the behavior of a \code{FrameStepper} by
providing their own implementation of these helper classes.

\subsubsection{Class FrameFuncStepper}
This class implements stack walking through a call frame that is setup with the
architectures standard stack frame. For example, on x86 this \code{FrameStepper} will
be used to walk through stack frames that are setup with a \code{push \%ebp/mov
\%esp,\%ebp} prologue. 

\paragraph{Class FrameFuncHelper}

\code{FrameFuncStepper} uses a helper class, \code{FrameFuncHelper}, to get information on
what kind of stack frame it's walking through. The \code{FrameFuncHelper} will
generally use techniques such as binary analysis to determine what type of stack
frame the \code{FrameFuncStepper} is walking through. Users can have StackwalkerAPI use
their own binary analysis mechanisms by providing an implementation of this
\code{FrameFuncHelper}.

There are two important types used by \code{FrameFuncHelper} and one important function:
\begin{apient}
typedef enum {
    unknown_t=0,
    no_frame,
    standard_frame,
    savefp_only_frame,
} frame_type;
\end{apient}
\apidesc{
    The \code{frame\_type} describes what kind of stack frame a function uses. If it
    does not set up a stack frame then \code{frame\_type} should be
    \code{no\_frame}. If it sets
    up a standard frame then \code{frame\_type} should be \code{standard\_frame}. The
    \code{savefp\_only\_frame} value currently only has meaning on the x86 family of
    systems, and means that a function saves the old frame pointer, but does not
    setup a new frame pointer (it has a \code{push \%ebp} instruction, but no \code{mov
    \%esp,\%ebp}). If the \code{FrameFuncHelper} cannot determine the
    \code{frame\_type}, then it should be assigned the value \code{unknown\_t}. 
}

\begin{apient}
typedef enum {
    unknown_s=0,
    unset_frame,
    halfset_frame,
    set_frame
} frame_state;
\end{apient}
\apidesc{
    The \code{frame\_state} type determines the current state of function with a stack
    frame at some point of execution. For example, a function may set up a
    standard stack frame and have a \code{frame\_type} of \code{standard\_frame}, but execution
    may be at the first instruction in the function and the frame is not yet
    setup, in which case the \code{frame\_state} will be \code{unset\_frame}. 

    If the function sets up a standard stack frame and the execution point is
    someplace where the frame is completely setup, then the \code{frame\_state} should be
    \code{set\_frame}. If the function sets up a standard frame and the execution point is
at a point where the frame does not yet exist or has been torn down, then
\code{frame\_state} should be \code{unset\_frame}. The \code{halfset\_frame}
value of \code{frame\_state} is
currently only meaningful on the x86 family of architecture, and should if the
function has saved the old frame pointer, but not yet set up a new frame
pointer.	
}

\begin{apient}
typedef std::pair<frame_type, frame_state> alloc_frame_t;		
virtual alloc_frame_t allocatesFrame(Address addr) = 0;
\end{apient}
\apidesc{
    The \code{allocatesFrame} function of \code{FrameFuncHelper} returns a
    \code{alloc\_frame\_t} that
    describes the frame\_type of the function at \code{addr} and the
    \code{frame\_state} of the
    function when execution reached \code{addr}.
	
    If \code{addr} is invalid or an error occurs, allocatedFrame should return
    \code{alloc\_frame\_t(unknown\_t, unknown\_s)}.  
}

\subsubsection{Class SigHandlerStepper}

The \code{SigHandlerStepper} is used to walk through UNIX signal handlers as found on
the call stack. On some systems a signal handler generates a special kind of
stack frame that cannot be walked through using normal stack walking techniques.


\subsubsection{Class DebugStepper}

This class uses debug information found in a binary to walk through a stack
frame. It depends on SymtabAPI to read debug information from a binary, then
uses that debug information to walk through a call frame. 

Most binaries must be built with debug information (\code{-g} with \code{gcc}) in order to
include debug information that this \code{FrameStepper} uses. Some languages, such as
C++, automatically include stackwalking debug information for use by exceptions.
The \code{DebugStepper} class will also make use of this kind of exception
information if it is available.

\subsubsection{Class AnalysisStepper}

This class uses dataflow analysis to determine possible stack sizes at
all locations in a function as well as the location of the frame
pointer. It is able to handle optimized code with omitted frame
pointers and overlapping code sequences. 

\subsubsection{Class StepperWanderer}

This class uses a heuristic approach to find possible return addresses
in the stack frame. If a return address is found that matches a valid
caller of the current function, we conclude it is the actual return
address and construct a matching stack frame. Since this approach is
heuristic it can make mistakes leading to incorrect stack
information. It has primarily been replaced by the
\code{AnalysisStepper} described above. 

\subsubsection{Class BottomOfStackStepper}

The \code{BottomOfStackStepper} doesn't actually walk through any type of call frame.
Instead it attempts to detect whether the bottom of the call stack has been
reached. If so, \code{BottomOfStackStepper} will report \code{gcf\_stackbottom} from its
\code{getCallerFrame} method. Otherwise it will report \code{gcf\_not\_me}.
\code{BottomOfStackStepper}
runs with a higher priority than any other \code{FrameStepper} class.

