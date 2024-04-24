.. _`sec-dev:abslocinterface.h`:

AbslocInterface.h
#################

.. cpp:namespace:: Dyninst::dev

.. cpp:class:: AbsRegionConverter

  .. cpp:function:: private bool getCurrentStackHeight(ParseAPI::Function *func, ParseAPI::Block *block, Address addr, long &height)

    Returns false if the current height is unknown.

  .. cpp:function:: private bool getCurrentFrameHeight(ParseAPI::Function *func, ParseAPI::Block *block, Address addr, long &height)
  .. cpp:function:: private bool convertResultToAddr(const InstructionAPI::Result &res, Address &addr)
  .. cpp:function:: private bool convertResultToSlot(const InstructionAPI::Result &res, int &slot)
  .. cpp:function:: private bool usedCache(Address, ParseAPI::Function *, std::vector<AbsRegion> &used)
  .. cpp:function:: private bool definedCache(Address, ParseAPI::Function *, std::vector<AbsRegion> &defined)
  .. cpp:type:: private std::vector<AbsRegion> RegionVec

    Caching mechanism...

  .. cpp:type:: private std::map<Address, RegionVec> AddrCache
  .. cpp:type:: private std::map<ParseAPI::Function *, AddrCache> FuncCache
  .. cpp:member:: private FuncCache used_cache_
  .. cpp:member:: private FuncCache defined_cache_
  .. cpp:member:: private bool cacheEnabled_
  .. cpp:member:: private bool stackAnalysisEnabled_
