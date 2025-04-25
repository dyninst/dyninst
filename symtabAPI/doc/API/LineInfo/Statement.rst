\subsection{Class Statement}\label{Statement}

A \code{Statement} is the base representation of line information. 

\begin{tabular}{p{1.25in}p{1.25in}p{3in}}
\toprule
Method name & Return type & Method description \\
\midrule
startAddr & Offset & Starting address of this line in the file. \\
endAddr & Offset & Ending address of this line in the file. \\
getFile & std::string & File that contains the line.   \\
getLine & unsigned int & Line number. \\
getColumn & unsigned int & Starting column number. \\
\bottomrule
\end{tabular}

For backwards compatibility, this class may also be referred to as a \code{LineNoTuple}, and provides the following legacy member variables. They should not be used and will be removed in a future version of SymtabAPI. 

\begin{tabular}{p{1.25in}p{1.25in}p{3in}}
\toprule
Member & Return type & Method description \\
\midrule
first & const char * & Equivalent to getFile. \\
second & unsigned int & Equivalent to getLine. \\
column & unsigned int & Equivalent to getColumn. \\ 
\bottomrule
\end{tabular}
