\subsection{Result Class}
\label{sec:result}

A \code{Result} object represents a value computed by an \code{Expression} AST.

The \code{Result} class is a tagged-\/union representation of the results that Expressions can produce. It includes 8, 16, 32, 48, and 64 bit integers (signed and unsigned), bit values, and single and double precision floating point values. For each of these types, the value of a Result may be undefined, or it may be a value within the range of the type.

The \code{type} field is an enum that may contain any of the following values:

\begin{itemize}
\item \code{u8:} an unsigned 8-\/bit integer
\item \code{s8:} a signed 8-\/bit integer
\item \code{u16:} an unsigned 16-\/bit integer
\item \code{s16:} a signed 16-\/bit integer
\item \code{u32:} an unsigned 32-\/bit integer
\item \code{s32:} a signed 32-\/bit integer
\item \code{u48:} an unsigned 48-\/bit integer (IA32 pointers)
\item \code{s48:} a signed 48-\/bit integer (IA32 pointers)
\item \code{u64:} an unsigned 64-\/bit integer
\item \code{s64:} a signed 64-\/bit integer
\item \code{sp\_\-float:} a single-\/precision float
\item \code{dp\_\-float:} a double-\/precision float
\item \code{bit\_\-flag:} a single bit (individual flags)
\item \code{m512:} a 512-\/bit memory value
\item \code{dbl128:} a 128-\/bit integer, which often contains packed floating point values -\/ \code{m14:} a 14 byte memory value 
\end{itemize}

\begin{apient}
Result (Result_Type t)
\end{apient}
\apidesc{
A \code{Result} may be constructed from a type without providing a value. This constructor creates a \code{Result} of type \code{t} with undefined contents.
}

\begin{apient}
Result (Result_Type t, T v)
\end{apient}
\apidesc{
A \code{Result} may be constructed from a type and any value convertible to the type that the tag represents. This constructor creates a \code{Result} of type \code{t} and contents \code{v} for any \code{v} that is implicitly convertible to type \code{t}. Attempting to construct a \code{Result} with a value that is incompatible with its type will result in a compile-time error.
}

\begin{apient}
bool operator== (const Result & o) const
\end{apient}
\apidesc{
Two \code{Result}s are equal if any of the following hold:
\begin{itemize}
\item Both \code{Result}s are of the same type and undefined
\item Both \code{Result}s are of the same type, defined, and have the same value
\end{itemize}
Otherwise, they are unequal (due to having different types, an undefiend \code{Result} compared to a defined \code{Result}, or different values).
}

\begin{apient}
std::string format () const
\end{apient}
\apidesc{
\code{Result}s are formatted as strings containing their contents, represented as hexadecimal. The type of the \code{Result} is not included in the output.
}

\begin{apient}
template <typename to_type>
to_type convert() const
\end{apient}
\apidesc{Converts the \code{Result} to the desired datatype. For
  example, to convert a \code{Result} \code{res} to a signed char, use
  \code{res.convert<char>()}; to convert it to an unsigned long, use
  \code{res.convert<unsigned long>()}. }

\begin{apient}
int size () const
\end{apient}
\apidesc{
Returns the size of the contained type, in bytes.
}
