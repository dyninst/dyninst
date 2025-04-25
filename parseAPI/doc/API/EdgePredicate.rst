\subsection{Edge Predicates}
\label{sec:pred}

\definedin{CFG.h}

Edge predicates control iteration over edges. For example, the provided
\code{Intraproc} edge predicate can be used with filter iterators and standard
algorithms, ensuring that only intraprocedural edges are visited during
iteration. Two other examples of edge predicates are provided: 
\code{SingleContext} only visits edges that stay in a single
function context, and \code{NoSinkPredicate} does not visit edges to 
the \emph{sink} block.  The following code traverses 
all of the basic blocks within a
function:

\lstset{language=[GNU]C++,basicstyle=\fontfamily{fvm}\selectfont\small}
\lstset{numbers=left, numberstyle=\tiny, stepnumber=5, numbersep=5pt}
\begin{lstlisting}
    #include <boost/filter_iterator.hpp>
    using boost::make_filter_iterator;
    struct target_block
    {
      Block* operator()(Edge* e) { return e->trg(); }
    };


    vector<Block*> work;
    Intraproc epred; // ignore calls, returns
   
    work.push_back(func->entry()); // assuming `func' is a Function*

    // do_stuff is a functor taking a Block* as its argument
    while(!work.empty()) {
        Block * b = work.back();
        work.pop_back();

        Block::edgelist & targets = block->targets();
        // Do stuff for each out edge
        std::for_each(make_filter_iterator(targets.begin(), epred), 
                      make_filter_iterator(targets.end(), epred),
                      do_stuff());
        std::transform(make_filter_iterator(targets.begin(), epred),
                       make_filter_iterator(targets.end(), epred), 
                       std::back_inserter(work), 
                       std::mem_fun(Edge::trg));
        Block::edgelist::const_iterator found_interproc =
                std::find_if(targets.begin(), targets.end(), Interproc());
        if(interproc != targets.end()) {
                // do something with the interprocedural edge you found
        }
    }

\end{lstlisting}

Anything that can be treated as a function from \code{Edge*} to a \code{bool} can be used in this manner. This replaces the beta interface where all
\code{EdgePredicate}s needed to descend from a common parent class. Code that previously constructed iterators from an edge predicate should be replaced
with equivalent code using filter iterators as follows:

\begin{lstlisting}
  // OLD
  for(Block::edgelist::iterator i = targets.begin(epred); 
      i != targets.end(epred); 
      i++)
  {
    // ...
  }
  // NEW
  for_each(make_filter_iterator(epred, targets.begin(), targets.end()),
           make_filter_iterator(epred, targets.end(), targets,end()),
           loop_body_as_function);
  // NEW (C++11)
  for(auto i = make_filter_iterator(epred, targets.begin(), targets.end()); 
      i != make_filter_iterator(epred, targets.end(), targets.end()); 
      i++)
  {
    // ...
  }
  

\end{lstlisting}
