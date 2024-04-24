.. _`sec:DynPointMaker.h`:

DynPointMaker.h
###############


.. cpp:class:: DynPointMaker : public Dyninst::PatchAPI::PointMaker

  .. cpp:function:: DynPointMaker()
  .. cpp:function:: virtual ~DynPointMaker()
  .. cpp:function:: protected virtual Dyninst::PatchAPI::Point *mkFuncPoint(Dyninst::PatchAPI::Point::Type t,\
                                                                            Dyninst::PatchAPI::PatchMgrPtr m,\
                                                                            Dyninst::PatchAPI::PatchFunction *)

  .. cpp:function:: protected virtual Dyninst::PatchAPI::Point *mkFuncSitePoint(Dyninst::PatchAPI::Point::Type t,\
                                                                                Dyninst::PatchAPI::PatchMgrPtr m,\
                                                                                Dyninst::PatchAPI::PatchFunction*,\
                                                                                Dyninst::PatchAPI::PatchBlock*)


  .. cpp:function:: protected virtual Dyninst::PatchAPI::Point *mkBlockPoint(Dyninst::PatchAPI::Point::Type t,\
                                                                             Dyninst::PatchAPI::PatchMgrPtr m,\
                                                                             Dyninst::PatchAPI::PatchBlock*,\
                                                                             Dyninst::PatchAPI::PatchFunction* context)


  .. cpp:function:: protected virtual Dyninst::PatchAPI::Point *mkInsnPoint(Dyninst::PatchAPI::Point::Type t,\
                                                                            Dyninst::PatchAPI::PatchMgrPtr m,\
                                                                            Dyninst::PatchAPI::PatchBlock*,\
                                                                            Dyninst::Address,\
                                                                            Dyninst::InstructionAPI::Instruction,\
                                                                            Dyninst::PatchAPI::PatchFunction* context)


  .. cpp:function:: protected virtual Dyninst::PatchAPI::Point *mkEdgePoint(Dyninst::PatchAPI::Point::Type t,\
                                                                            Dyninst::PatchAPI::PatchMgrPtr m,\
                                                                            Dyninst::PatchAPI::PatchEdge*,\
                                                                            Dyninst::PatchAPI::PatchFunction* f)
