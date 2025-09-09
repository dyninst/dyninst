\subsection{Class VariableLocation}\label{VariableLocation}

The \code{VariableLocation} class is an encoding of the location of a variable in memory or registers. 

\begin{apient}
typedef enum {
   storageUnset,
   storageAddr,
   storageReg,
   storageRegOffset
} storageClass;
	
typedef enum {
   storageRefUnset,
   storageRef,
   storageNoRef
} storageRefClass;

struct VariableLocation  {
    storageClass stClass;
    storageRefClass refClass;
    MachRegister mr_reg;
    long frameOffset;
    Address lowPC;
    Address hiPC;
}
\end{apient}

A \code{VariableLocation} is valid within the address range represented by \code{lowPC} and \code{hiPC}. If these are 0 and (Address) -1, respectively, the \code{VariableLocation} is always valid. 

The location represented by the \code{VariableLocation} can be determined by the user as follows:

\begin{itemize}
\item stClass == storageAddr
\begin{description}
\item[refClass == storageRef] The frameOffset member contains the address of a pointer to the variable. 
\item[refClass == storageNoRef] The frameOffset member contains the address of the variable. 
\end{description}
\item stClass == storageReg
\begin{description}
\item[refClass == storageRef] The register named by mr\_reg  contains the address of the variable. 
\item[refClass == storageNoRef] The register named by mr\_reg member contains the variable. 
\end{description}
\item stClass == storageRegOffset
\begin{description}
\item[refClass == storageRef] The address computed by adding frameOffset to the contents of mr\_reg contains a pointer to the variable. 
\item[refClass == storageNoRef] The address computed by adding frameOffset to the contents of mr\_reg contains the variable.  
\end{description}
\end{itemize}
