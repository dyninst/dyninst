.. _`sec:analysis_stepper.h`:

analysis_stepper.h
##################

.. cpp:namespace:: Dyninst::Stackwalker

.. cpp:class:: AnalysisStepperImpl : public FrameStepper

  .. cpp:type:: std::pair<StackAnalysis::Height, StackAnalysis::Height> height_pair_t
  .. cpp:type:: std::pair<Absloc, StackAnalysis::Height> registerState_t

  .. cpp:member:: private AnalysisStepper *parent
  .. cpp:member:: private CallChecker * callchecker
  .. cpp:member:: protected static std::map<std::string, ParseAPI::CodeObject *> objs
  .. cpp:member:: protected static std::map<std::string, ParseAPI::CodeSource*> srcs
  .. cpp:member:: protected static std::map<std::string, SymReader*> readers
  .. cpp:member:: static const height_pair_t err_height_pair

  .. cpp:function:: AnalysisStepperImpl(Walker *w, AnalysisStepper *p)
  .. cpp:function:: virtual ~AnalysisStepperImpl()
  .. cpp:function:: virtual gcframe_ret_t getCallerFrame(const Frame &in, Frame &out)
  .. cpp:function:: virtual unsigned getPriority() const
  .. cpp:function:: virtual const char *getName() const

  .. cpp:function:: protected static ParseAPI::CodeObject *getCodeObject(std::string name)
  .. cpp:function:: protected static ParseAPI::CodeSource *getCodeSource(std::string name)
  .. cpp:function:: protected std::set<height_pair_t> analyzeFunction(std::string name, Offset off)
  .. cpp:function:: protected std::vector<registerState_t> fullAnalyzeFunction(std::string name, Offset off)
  .. cpp:function:: protected virtual bool isPrevInstrACall(Address addr, Address & target)
  .. cpp:function:: protected virtual gcframe_ret_t getCallerFrameArch(std::set<height_pair_t> height, const Frame &in, Frame &out)
  .. cpp:function:: protected gcframe_ret_t getFirstCallerFrameArch(const std::vector<registerState_t>& heights, const Frame& in, Frame& out)
  .. cpp:function:: protected gcframe_ret_t checkResult(bool result)
  .. cpp:function:: protected bool validateRA(Address candidateRA)
  .. cpp:function:: protected ParseAPI::CodeRegion* getCodeRegion(std::string name, Offset off)
  .. cpp:function:: protected bool getOutRA(Address out_sp, Address& out_ra, location_t& out_ra_loc, ProcessState* proc)
