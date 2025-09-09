
\subsection{Containers}
\label{sec:containers}

Several of the ParseAPI data structures export containers of CFG objects; the
CodeObject provides a list of functions in the binary, for example, while
functions provide lists of blocks and so on. To avoid tying the internal
storage for these structures to any particular container type, ParseAPI objects
export a ContainerWrapper that provides an iterator interface to the internal
containers. These wrappers and predicate interfaces are designed to add minimal
overhead while protecting ParseAPI users from exposure to internal container
storage details. Users \emph{must not} rely on properties of the underlying
container type (e.g. storage order) unless that property is explicity stated in
this manual.

\noindent
ContainerWrapper containers export the following interface (\code{iterator} types vary depending on the template parameters of the ContainerWrapper, but are always instantiations of the PredicateIterator described below):

\begin{apient}
iterator begin()
iterator begin(predicate *)
\end{apient}
\apidesc{Return an iterator pointing to the beginning of the container, with or without a filtering predicate implementation (see Section \ref{sec:pred} for details on filter predicates).}

\begin{apient}
iterator const& end()
\end{apient}
\apidesc{Return the iterator pointing to the end of the container (past the last element).}

\begin{apient}
size_t size()
\end{apient}
\apidesc{Returns the number of elements in the container. Execution cost may vary depending on the underlying container type.}

\begin{apient}
bool empty()
\end{apient}
\apidesc{Indicates whether the container is empty or not.}

\noindent
The elements in ParseAPI containers can be accessed by iteration using an instantiation of the PredicateIterator. These iterators can optionally act as filters, evaluating a boolean predicate for each element and only returning those elements for which the predicate returns {\scshape true}. \emph{Iterators with non-{\scshape null} predicates may return fewer elements during iteration than their \code{size()} method indicates.} Currently PredicateIterators only support forward iteration. The operators \code{++} (prefix and postfix), \code{==}, \code{!=}, and \code{*} (dereference) are supported.
