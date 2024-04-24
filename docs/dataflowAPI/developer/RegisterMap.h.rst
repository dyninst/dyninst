RegisterMap.h
#############

.. cpp:namespace:: Dyninst::dev::dataflowAPI

.. cpp:type:: std::map<MachRegister, int> RegisterMap

.. cpp:function:: RegisterMap &machRegIndex_x86()
.. cpp:function:: RegisterMap &machRegIndex_x86_64()
.. cpp:function:: RegisterMap &machRegIndex_ppc()
.. cpp:function:: RegisterMap &machRegIndex_ppc_64()
.. cpp:function:: RegisterMap &machRegIndex_aarch64()

Notes
*****

We use the singleton approach, rather than static construction, to ensure the
register maps are created correctly. In at least one case (Ubuntu 12.04) they weren't.
