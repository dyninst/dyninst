\subsection{Class ExceptionBlock}\label{ExceptionBlock}
This class represents an exception block present in the object file. This class gives all the information pertaining to that exception block.

\begin{tabular}{p{1.25in}p{1in}p{3.25in}}
\toprule
Method name & Return type & Method description \\
\midrule
hasTry & bool & True if the exception block has a try block. \\
tryStart & Offset & Start of the try block if it exists, else 0. \\
tryEnd & Offset & End of the try block if it exists, else 0. \\
trySize & Offset & Size of the try block if it exists, else 0. \\
catchStart & Offset & Start of the catch block. \\
\bottomrule
\end{tabular} 


\begin{apient}
bool contains(Offset addr) const
\end{apient}
\apidesc{
This method returns \code{true} if the offset \code{addr} is contained with in the try block. If there is no try block associated with this exception block or the offset does not fall within the try block, it returns \code{false}.
}
