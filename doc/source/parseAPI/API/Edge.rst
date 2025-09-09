Class Edge
----------

Typed Edges join two blocks in the CFG, indicating the type of control
flow transfer instruction that joins the blocks to each other. Edges may
not correspond to a control flow transfer instruction at all, as in the
case of the fallthrough edge that indicates where straight-line control
flow is split by incoming transfers from another location, such as a
branch. While not all blocks end in a control transfer instruction, all
control transfer instructions end basic blocks and have outgoing edges;
in the case of unresolvable control flow, the edge will target a special
“sink” block (see , below).

============== ==============================
EdgeTypeEnum   Meaning
============== ==============================
CALL           call edge
COND_TAKEN     conditional branch–taken
COND_NOT_TAKEN conditional branch–not taken
INDIRECT       branch indirect
DIRECT         branch direct
FALLTHROUGH    direct fallthrough (no branch)
CATCH          exception handler
CALL_FT        post-call fallthrough
RET            return
============== ==============================

=========== ============
=========================================================================================================================
Method name Return type  Method description
=========== ============
=========================================================================================================================
src         Block \*     Source of the edge.
trg         Block \*     Target of the edge.
type        EdgeTypeEnum Type of the edge.
sinkEdge    bool         True if the target is the sink block.
interproc   bool         True if the edge should be interpreted as interprocedural (e.g. calls, returns, unconditional or conditional tail calls).
=========== ============
=========================================================================================================================
