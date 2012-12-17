\subsubsection{Class StepperGroup}
\label{subsec:steppergroup}
\definedin{steppergroup.h}

The \code{StepperGroup} class contains a collection of \code{FrameStepper} objects. The
\code{StepperGroup}'s primary job is to decide which \code{FrameStepper} should be used to
walk through a stack frame given a return address. The default \code{StepperGroup}
keeps a set of address ranges for each \code{FrameStepper}. If multiple \code{FrameStepper}
objects overlap an address, then the default \code{StepperGroup} will use a priority
system to decide.

\code{StepperGroup} provides both an interface and a default implementation of that
interface. Users who want to customize the \code{StepperGroup} should inherit from this
class and re-implement any of the below virtual functions.

\begin{apient}
StepperGroup(Walker *walker)
\end{apient}
\apidesc{
    This factory constructor creates a new \code{StepperGroup} object associated
with \code{walker}. 
}

\begin{apient}
virtual bool addStepper(FrameStepper *stepper)
\end{apient}
\apidesc{
    This method adds a new \code{FrameStepper} to this \code{StepperGroup}. The
    newly added stepper will be tracked by this \code{StepperGroup}, and it will
    be considered for use when walking through stack frames. 

    This method returns \code{\code{true}} if it successfully added the
    \code{FrameStepper}, and \code{\code{false}} on error.
}

\begin{apient}
virtual bool addStepper(FrameStepper *stepper, Address start, Address
end) = 0;
\end{apient}
\apidesc{Add the specified \code{FrameStepper} to the list of known
  steppers, and register it to handle frames in the range
  [\code{start}, \code{end}).}

\begin{apient}
virtual void registerStepper(FrameStepper *stepper);
\end{apient}

\apidesc{Add the specified \code{FrameStepper} to the list of known
  steppers and use it over the entire address space.}

\begin{apient}
virtual bool findStepperForAddr(Address addr, FrameStepper* &out, 
                                const FrameStepper *last_tried = NULL) = 0
\end{apient}
\apidesc{
    Given an address that points into a function (or function-like object),
    addr, this method decides which \code{FrameStepper} should be used to walk through
    the stack frame created by the function at that address. A pointer to the
    \code{FrameStepper} will be returned in parameter \code{out}. 

    It may be possible that the \code{FrameStepper} this method decides on is unable to
    walk through the stack frame (it returns \code{gcf\_not\_me} from
    \code{FrameStepper::getCallerFrame}). In this case StackwalkerAPI will call
    findStepperForAddr again with the last\_tried parameter set to the failed
    \code{FrameStepper}. findStepperForAddr should then find another \code{FrameStepper} to
    use. Parameter \code{last\_tried} will be set to NULL the first time getStepperToUse
    is called for a stack frame.

    The default version of this method uses address ranges to decide which
    \code{FrameStepper} to use. The address ranges are contained within the process'
    code space, and map a piece of the code space to a \code{FrameStepper} that can
    walk through stack frames created in that code range. If multiple
    \code{FrameStepper} objects share the same range, then the one with the highest
    priority will be tried first.
	
    This method returns \code{true} on success and \code{false} on failure.  
}

\begin{apient}
typedef std::pair<std::string, Address> LibAddrPair;
typedef enum { library_load, library_unload } lib_change_t;    
virtual void newLibraryNotification(LibAddrPair *libaddr, lib_change_t
change);
\end{apient}
\apidesc{Called by the StackwalkerAPI when a new library is loaded.}

\begin{apient}
Walker *getWalker() const
\end{apient}
\apidesc{
	This method returns the Walker object that associated with this StepperGroup.
}

\begin{apient}
void getSteppers(std::set<FrameStepper *> &);
\end{apient}
\apidesc{Fill in the provided set with all \code{FrameSteppers}
  registered in the \code{StepperGroup}. }

