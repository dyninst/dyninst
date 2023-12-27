.. _`sec:liveness.h`:

liveness.h
##########

.. cpp:struct:: livenessData

  .. cpp:member:: bitArray in
  .. cpp:member:: bitArray out
  .. cpp:member:: bitArray use
  .. cpp:member:: bitArray def

.. cpp:class:: LivenessAnalyzer

  **Register liveness detection**

  .. cpp:function:: LivenessAnalyzer(int w)

     Creates an analyzer with an assumed address width ``w``.

  .. cpp:enum:: Type

    .. cpp:enumerator:: Before

       Analyze liveness for registers before the current location.

    .. cpp:enumerator:: After

       Analyze liveness for registers after the current location.

  .. cpp:function:: void analyze(ParseAPI::Function *func)

     Calculate basic block summaries of liveness information for all basic blocks in ``func``.

  .. cpp:function:: bool query(ParseAPI::Location loc, Type type, const MachRegister &machReg, bool& live)

     Calculates liveness for the register ``machReg`` at ``loc`` in the direction ``type`` and stores
     the result in ``live``.

     Returns ``false`` if no result could be found.

  .. cpp:function:: bool query(ParseAPI::Location loc, Type type, bitArray &bitarray)

     Calculates liveness for all registers represented by ``bitarray`` starting at ``loc`` in the direction ``type``.

     This does two things. First, it does a backwards iteration over instructions in its
     blocks to calculate liveness. At the same time, liveness in any covered instPoints is
     cached. Since an IP only exists if the user asked for it, its existence is assumed to
     indicate that they'll also be instrumenting.

     Returns ``false`` if ``loc`` is not valid.

  .. cpp:function:: void clean(ParseAPI::Function *func)

     Resets cached liveness information associated with ``func``.

  .. cpp:function:: void clean()

     Resets all cached liveness information.

  .. cpp:function:: int getIndex(MachRegister machReg)

     Returns the cache index of the register ``machReg``.

  .. cpp:function:: Dyninst::ABI* getABI()

     Returns the :cpp:class:`Dyninst::ABI` associated with the current platform.
