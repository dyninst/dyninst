\subsection{Class ParseCallback}

\definedin{ParseCallback.h}

The ParseCallback class allows ParseAPI users to be notified of various events
during parsing. For most users this notification is unnecessary, and an
instantiation of the default ParseCallback can be passed to the CodeObject during initialization. Users who wish to be notified must implement a class that inherits from ParseCallback, and implement one or more of the methods described below to receive notification of those events.

\begin{apient}
struct default_details {
    default_details(unsigned char * b,size_t s, bool ib);
    unsigned char * ibuf;
    size_t isize;
    bool isbranch;
}
\end{apient}
\apidesc{Details used in the \code{unresolved\_cf} and \code{abruptEnd\_cf}
callbacks.}

\begin{apient}
virtual void instruction_cb(Function *,
                            Block *,
                            Address,
                            insn_details *)
\end{apient}
\apidesc{Invoked for each instruction decoded during parsing. Implementing this callback may incur significant overhead.}

\begin{apient}
struct insn_details {
    InsnAdapter::InstructionAdapter * insn;
}
\end{apient}

\begin{apient}
void interproc_cf(Function *,
                  Address,
                  interproc_details *)
\end{apient}
\apidesc{Invoked for each interprocedural control flow instruction.}

\begin{apient}  
struct interproc_details {
    typedef enum {
        ret,
        call,
        branch_interproc, // tail calls, branches to plts
        syscall
    } type_t;
    unsigned char * ibuf;
    size_t isize;
    type_t type;
    union {
        struct {
            Address target;
            bool absolute_address;
            bool dynamic_call;
        } call;
    } data;
}
\end{apient}
\apidesc{Details used in the \code{interproc\_cf} callback.}

\begin{apient}
void overlapping_blocks(Block *,
                        Block *)
\end{apient}
\apidesc{Noification of inconsistent parse data (overlapping blocks).}
