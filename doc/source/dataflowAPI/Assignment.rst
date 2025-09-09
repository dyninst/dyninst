.. _sec:assign:

Class Assignment
----------------

An assignment represents data dependencies between an output abstract
region that is modified by this instruction and several input abstract
regions that are used by this instruction. An instruction may modify
several abstract regions, so an instruction can correspond to multiple
assignments.

typedef boost::shared_ptr<Assignment> Ptr;

const std::vector<AbsRegion> &inputs() const; std::vector<AbsRegion>
&inputs();

const AbsRegion &out() const; AbsRegion &out();

InstructionAPI::Instruction::Ptr insn() const;

Address addr() const;

ParseAPI::Function \*func() const;

ParseAPI::Block \*block() const;

const std::string format() const;

.. _sec:assignmentcovnert:

Class AssignmentConverter
-------------------------

This class should be used to convert instructions to assignments.

AssignmentConverter(bool cache, bool stack = true);

void convert(InstructionAPI::Instruction::Ptr insn, const Address &addr,
ParseAPI::Function \*func, ParseAPI::Block \*blk,
std::vector<Assignment::Ptr> &assign);
