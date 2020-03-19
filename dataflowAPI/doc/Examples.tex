\section{Examples}
\label{sec:examples}
We show several examples of how to use DataflowAPI. In these examples, we assume
that the mutatee has been parsed and we have function and block objects to analyze.
Users may refer to the ParseAPI manual for how to obtain these function and block objects.

\subsection{Slicing}
The following example uses DataflowAPI to perform a backward slice on an
indirect jump instruction to determine the instructions that affect the
calculation of the jump target. The goal of this example is to show (1) how to
convert an instruction to assignments; (2) how to perform slicing on a given
assignment; (3) how to extend the default \texttt{Slicer::Predicates} and write
call back functions to control the behavior of slicing.

\lstset{language=[GNU]C++,basicstyle=\fontfamily{fvm}\selectfont\small}
\lstset{numbers=left, numberstyle=\tiny, stepnumber=5, numbersep=5pt}
\lstset{showstringspaces=false}
\begin{lstlisting}
#include "Instruction.h"
#include "CFG.h"
#include "slicing.h"

using namespace Dyninst;
using namespace ParseAPI;
using namespace InstructionAPI;
using namespace DataflowAPI;

// We extend the default predicates to control when to stop slicing
class ConstantPred : public Slicer::Predicates {
  public:
    // We do not want to track through memory writes
    virtual bool endAtPoint(Assignment::Ptr ap) {
        return ap->insn().writesMemory();
    }

    // We can treat PC as a constant as its value is the address of the instruction
    virtual bool addPredecessor(AbsRegion reg) {
        if (reg.absloc().type() == Absloc::Register) {
	    MachRegister r = reg.absloc().reg();
	    return !r.isPC();
	} 
	return true;
    }
};

// Assume that block b in function f ends with an indirect jump.
void AnalyzeJumpTarget(Function *f, Block *b) {
    // Get the last instruction in this block, which should be a jump
    Instruction insn = b->getInsn(b->last());
   
    // Convert the instruction to assignments
    // The first parameter means to cache the conversion results.
    // The second parameter means whether to use stack analysis to anlyze stack accesses.
    AssignmentConverter ac(true, false);
    vector<Assignment::Ptr> assignments;
    ac.convert(insn, b->last(), f, b, assignments);

    // An instruction can corresponds to multiple assignment.
    // Here we look for the assignment that changes the PC.
    Assignment::Ptr pcAssign;
    for (auto ait = assignments.begin(); ait != assignments.end(); ++ait) {
	const AbsRegion &out = (*ait)->out();
	if (out.absloc().type() == Absloc::Register && out.absloc().reg().isPC()) {
	    pcAssign = *ait;
	    break;
	}
    }

    // Create a Slicer that will start from the given assignment
    Slicer s(pcAssign, b, f);

    // We use the customized predicates to control slicing
    ConstantPred mp;
    GraphPtr slice = s.backwardSlice(mp);
}
\end{lstlisting}


\subsection{Symbolic Evaluation}
The following example shows how to expand a slice to ASTs and analyze an AST.
Suppose we have a slice representing the instructions that affect the jump
target of an indirect jump instruction.
We can get the expression of the jump targets and visit the expression to see if it
is a constant. 

\lstset{language=[GNU]C++,basicstyle=\fontfamily{fvm}\selectfont\small}
\lstset{numbers=left, numberstyle=\tiny, stepnumber=5, numbersep=5pt}
\lstset{showstringspaces=false}
\begin{lstlisting}
#include "SymEval.h"
#include "slicing.h"
using namespace Dyninst;
using namespace DataflowAPI;

// We extend the default ASTVisitor to check whether the AST is a constant
class ConstVisitor: public ASTVisitor {
  public:
    bool resolved;
    Address target;
    ConstVisitor() : resolved(true), target(0){}

    // We reach a constant node and record its value
    virtual AST::Ptr visit(DataflowAPI::ConstantAST * ast) {
        target = ast->val().val;
        return AST::Ptr();
    };

    // If the AST contains a variable 
    // or an operation, then the control flow target cannot
    // be resolved through constant propagation
    virtual AST::Ptr visit(DataflowAPI::VariableAST *) {
        resolved = false;
	return AST::Ptr();
    };
    virtual AST::Ptr visit(DataflowAPI::RoseAST * ast) {
        resolved = false;

	// Recursively visit all children
        unsigned totalChildren = ast->numChildren();
	for (unsigned i = 0 ; i < totalChildren; ++i) {
	    ast->child(i)->accept(this);
	}
        return AST::Ptr();
    };
};

Address ExpandSlice(GraphPtr slice, Assignment::Ptr pcAssign) {
    Result_t symRet;
    SymEval::expand(slice, symRet);

    // We get AST representing the jump target
    AST::Ptr pcExp = symRet[pcAssign];

    // We analyze the AST to see if it can actually be resolved by constant propagation
    ConstVisitor cv;
    pcExp->accept(&cv);
    if (cv.resolved) return cv.target;
    return 0;
}
\end{lstlisting}

\subsection{Liveness Analysis}
The following example shows how to query for live registers.

\lstset{language=[GNU]C++,basicstyle=\fontfamily{fvm}\selectfont\small}
\lstset{numbers=left, numberstyle=\tiny, stepnumber=5, numbersep=5pt}
\lstset{showstringspaces=false}
\begin{lstlisting}
#include "Location.h"
#include "liveness.h"
#include "bitArray.h"
using namespace std;
using namespace Dyninst;
using namespace Dyninst::ParseAPI;

void LivenessAnalysis(Function *f, Block *b) {   
    // Construct a liveness analyzer based on the address width of the mutatee.
    // 32-bit code and 64-bit code have different ABI.
    LivenessAnalyzer la(f->obj()->cs()->getAddressWidth());
   
    // Construct a liveness query location
    Location loc(f, b);
   
    // Query live registers at the block entry
    bitArray liveEntry;
    if (!la.query(loc, LivenessAnalyzer::Before, liveEntry)) {
        printf("Cannot look up live registers at block entry\n");
    }

    printf("There are %d registers live at the block entry\n", liveEntry.count());

    // Query live register at the block exit
    bitArray liveExit;
    if (!la.query(loc, LivenessAnalyzer::After, liveExit)) {
       printf("Cannot look up live registers at block exit\n");
    }

    printf("rbx is live or not at the block exit: %d\n", liveExit.test(la.getIndex(x86_64::rbx)));
}
\end{lstlisting}

\subsection{Stack Analysis}
The following example shows how to use stack analysis to print out all defined stack heights at the first instruction in a block.

\lstset{language=[GNU]C++,basicstyle=\fontfamily{fvm}\selectfont\small}
\lstset{numbers=left, numberstyle=\tiny, stepnumber=5, numbersep=5pt}
\lstset{showstringspaces=false}
\begin{lstlisting}
#include "CFG.h"
#include "Absloc.h"
#include "stackanalysis.h"
using namespace Dyninst;
using namespace ParseAPI;

void StackHeight(Function *func, Block *block) {
    // Get the address of the first instruction of the block
    Address addr = block->start();

    // Get the stack heights at that address
    StackAnalysis sa(func);
    std::vector<std::pair<Absloc, StackAnalysis::Height>> heights;
    sa.findDefinedHeights(block, addr, heights);

    // Print out the stack heights
    for (auto iter = heights.begin(); iter != heights.end(); iter++) {
        const Absloc &loc = iter->first;
        const StackAnalysis::Height &height = iter->second;
        printf("%s := %s\n", loc.format().c_str(), height.format().c_str());
    }
}
\end{lstlisting}

