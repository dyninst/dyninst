\subsection{Class Edge}

\definedin{CFG.h}

Typed Edges join two blocks in the CFG, indicating the type of control flow
transfer instruction that joins the blocks to each other. Edges may not correspond
to a control flow transfer instruction at all, as in the case of the {\scshape
fallthrough} edge that indicates where straight-line control flow is split by
incoming transfers from another location, such as a branch. While not all
blocks end in a control transfer instruction, all control transfer instructions
end basic blocks and have outgoing edges; in the case of unresolvable control
flow, the edge will target a special ``sink'' block (see \code{sinkEdge()},
below).

\begin{center}
\begin{tabular}{ll}
\toprule
EdgeTypeEnum & Meaning \\
\midrule
CALL & call edge \\
COND\_TAKEN & conditional branch--taken \\
COND\_NOT\_TAKEN & conditional branch--not taken \\
INDIRECT & branch indirect \\
DIRECT & branch direct \\
FALLTHROUGH & direct fallthrough (no branch) \\
CATCH & exception handler \\
CALL\_FT & post-call fallthrough \\
RET & return \\
\bottomrule
\end{tabular}
\end{center}

\begin{tabular}{p{1.25in}p{1.125in}p{3.125in}}
\toprule
Method name & Return type & Method description \\
\midrule
src & Block * & Source of the edge. \\
trg & Block * & Target of the edge. \\
type & EdgeTypeEnum & Type of the edge. \\
sinkEdge & bool & True if the target is the sink block. \\
interproc & bool & True if the edge should be interpreted as interprocedural
(e.g. calls, returns, unconditional or conditional tail calls). \\
\bottomrule
\end{tabular}
