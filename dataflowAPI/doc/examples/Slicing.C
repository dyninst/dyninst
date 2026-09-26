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
void AnalyzeJumpTarget(Function* f, Block* b) {
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
	const AbsRegion& out = (*ait)->out();
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
