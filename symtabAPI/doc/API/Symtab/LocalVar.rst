\subsection{Class localVar}\label{localVar}
This represents a local variable or parameter of a function.


\begin{tabular}{p{1.25in}p{1in}p{3.25in}}
\toprule
Method name & Return type & Method description \\
\midrule
getName & string \& & Name of the local variable or parameter. \\
getType & Type * & Type associated with the variable. \\
getFileName & string \& & File where the variable was declared, if known. \\
getLineNum & int & Line number where the variable was declared, if known. \\
\bottomrule
\end{tabular}

\begin{apient}
vector<VariableLocation> &getLocationLists()
\end{apient}
\apidesc{
A local variable can be in scope at different positions and based on that it is accessible in different ways. Location lists provide a way to encode that information. 
The method retrieves the location list, specified in terms of \code{VariableLocation} structures (section \ref{VariableLocation}) where the variable is in scope. 
}
