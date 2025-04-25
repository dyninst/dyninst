.. _sec:abslocs:

Class Absloc
------------

Class Absloc represents an abstract location. Abstract locations can
have the following types

======== =================================================
Type     Meaning
======== =================================================
Register The abstract location represents a register
Stack    The abstract location represents a stack variable
Heap     The abstract location represents a heap variable
Unknown  The default type of abstract location
======== =================================================

static Absloc makePC(Dyninst::Architecture arch); static Absloc
makeSP(Dyninst::Architecture arch); static Absloc
makeFP(Dyninst::Architecture arch);

bool isPC() const; bool isSP() const; bool isFP() const;

Absloc();

Absloc(MachRegister reg);

Absloc(Address addr):

Absloc(int o, int r, ParseAPI::Function \*f);

std::string format() const;

const Type& type() const;

bool isValid() const;

const MachRegister &reg() const;

int off() const;

int region() const;

ParseAPI::Function \*func() const;

Address addr() const;

bool operator<(const Absloc &rhs) const; bool operator==(const Absloc
&rhs) const; bool operator!=(const Absloc &rhs) const;

.. _sec:absregion:

Class AbsRegion
---------------

Class AbsRegion represents a set of abstract locations of the same type.

AbsRegion();

AbsRegion(Absloc::Type t);

AbsRegion(Absloc a);

bool contains(const Absloc::Type t) const; bool contains(const Absloc
&abs) const; bool contains(const AbsRegion &rhs) const;

bool containsOfType(Absloc::Type t) const;

bool operator==(const AbsRegion &rhs) const; bool operator!=(const
AbsRegion &rhs) const; bool operator<(const AbsRegion &rhs) const;

const std::string format() const;

Absloc absloc() const;

Absloc::Type type() const;

AST::Ptr generator() const;

bool isImprecise() const;

Class AbsRegionConverter
------------------------

Class AbsRegionConverter converts instructions to abstract regions.

AbsRegionConverter(bool cache, bool stack = true);

void convertAll(InstructionAPI::Expression::Ptr expr, Address addr,
ParseAPI::Function \*func, ParseAPI::Block \*block,
std::vector<AbsRegion> &regions);

void convertAll(InstructionAPI::Instruction::Ptr insn, Address addr,
ParseAPI::Function \*func, ParseAPI::Block \*block,
std::vector<AbsRegion> &used, std::vector<AbsRegion> &defined);

AbsRegion convert(InstructionAPI::RegisterAST::Ptr reg);

AbsRegion convert(InstructionAPI::Expression::Ptr expr, Address addr,
ParseAPI::Function \*func, ParseAPI::Block \*block);
