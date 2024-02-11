.. _`sec:RegisterConversion.h`:

RegisterConversion.h
####################

.. cpp:var:: extern std::multimap<Dyninst::Register, Dyninst::MachRegister> regToMachReg32
.. cpp:var:: extern std::multimap<Dyninst::Register, Dyninst::MachRegister> regToMachReg64
.. cpp:function:: Dyninst::Register convertRegID(Dyninst::InstructionAPI::RegisterAST::Ptr toBeConverted, bool& wasUpcast)
.. cpp:function:: Dyninst::Register convertRegID(Dyninst::InstructionAPI::RegisterAST* toBeConverted, bool& wasUpcast)
.. cpp:function:: Dyninst::Register convertRegID(Dyninst::MachRegister reg, bool &wasUpcast)
.. cpp:function:: Dyninst::MachRegister convertRegID(Dyninst::Register r, Dyninst::Architecture arch)
