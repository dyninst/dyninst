.. _`sec-dev:BPatch_memoryAccess_NP.h`:

BPatch_memoryAccess_NP.h
########################

.. cpp:namespace:: dev

.. code:: cpp

  /* Pseudoregisters definitions */
  #define POWER_XER2531 9999

  #define IA32_EMULATE  1000
  #define IA32_ESCAS  1000
  #define IA32_NESCAS 1001
  #define IA32_ECMPS  1002
  #define IA32_NECMPS 1003

  #define IA32prefetchNTA  0
  #define IA32prefetchT0   1
  #define IA32prefetchT1   2
  #define IA32prefetchT2   3
  #define IA32AMDprefetch  100
  #define IA32AMDprefetchw 101


.. cpp:class:: BPatch_memoryAccess

  .. cpp:function:: BPatch_memoryAccess(internal_instruction *, Dyninst::Address _addr, bool _isLoad, bool _isStore,\
                                        unsigned int _bytes, long _imm, int _ra, int _rb, unsigned int _scale = 0,\
                                        int _cond = -1, bool _nt = false)

    initializes only the first access #bytes is a constant

  .. cpp:function:: BPatch_memoryAccess(internal_instruction *insn, Dyninst::Address _addr, bool _isinternal_Load, bool _isStore,\
                                        long _imm_s, int _ra_s, int _rb_s, unsigned int _scale_s, long _imm_c, int _ra_c,\
                                        int _rb_c, unsigned int _scale_c, int _cond, bool _nt, int _preFcn = -1)

    initializes only the first access #bytes is an expression wscale

  .. cpp:function:: BPatch_memoryAccess(internal_instruction *insn, Dyninst::Address _addr, bool _isLoad, bool _isStore,\
                                        bool _isPrefetch, long _imm_s, int _ra_s, int _rb_s, long _imm_c, int _ra_c, int _rb_c,\
                                        unsigned short _preFcn)

    initializes only the first access #bytes is an expression

  .. cpp:function:: BPatch_memoryAccess(internal_instruction *insn, Dyninst::Address _addr, bool _isLoad, bool _isStore,\
                                        long _imm_s, int _ra_s, int _rb_s, long _imm_c, int _ra_c, int _rb_c)

    initializes only the first access #bytes is an expression & not a prefetch

  .. cpp:function:: void set2nd(bool _isLoad, bool _isStore, unsigned int _bytes, long _imm, int _ra, int _rb,\
                                unsigned int _scale = 0)

    sets 2nd access #bytes is constant

  .. cpp:function:: void set2nd(bool _isLoad, bool _isStore, long _imm_s, int _ra_s, int _rb_s, unsigned int _scale_s,\
                                long _imm_c, int _ra_c, int _rb_c, unsigned int _scale_c, int _cond, bool _nt)

    sets 2nd access #bytes is an expression wscale

  .. cpp:function:: BPatch_memoryAccess(internal_instruction *insn, Dyninst::Address _addr, bool _isLoad, bool _isStore,\
                                        unsigned int _bytes, long _imm, int _ra, int _rb, unsigned int _scale, bool _isLoad2,\
                                        bool _isStore2, unsigned int _bytes2, long _imm2, int _ra2, int _rb2, unsigned int _scale2)

    initializes both accesses #bytes is a constant

  .. cpp:function:: BPatch_memoryAccess(internal_instruction *insn, Dyninst::Address _addr, bool _isLoad, bool _isStore,\
                                        long _imm_s, int _ra_s, int _rb_s, unsigned int _scale_s, long _imm_c, int _ra_c,\
                                        int _rb_c, unsigned int _scale_c, bool _isLoad2, bool _isStore2, long _imm2_s,\
                                        int _ra2_s, int _rb2_s, unsigned int _scale2_s, long _imm2_c, int _ra2_c, int _rb2_c,\
                                        unsigned int _scale2_c)

    initializes both accesses #bytes is an expression & not a prefetch

  .. cpp:function:: protected void set1st(bool _isLoad, bool _isStore, long _imm_s, int _ra_s, int _rb_s, unsigned int _scale_s,\
                                          long _imm_c, int _ra_c, int _rb_c, unsigned int _scale_c, int _preFcn, int _cond,\
                                          bool _nt)

    initializes only the first access - general case

  .. cpp:function:: protected void set1st(bool _isLoad, bool _isStore, long _imm_s, int _ra_s, int _rb_s, long _imm_c,\
                                          int _ra_c = -1, int _rb_c = -1, unsigned int _scale_s = 0, int _preFcn = -1,\
                                          int _cond = -1, bool _nt = false)

    initializes only the first access - no scale for count


.. cpp:class:: BPatch_addrSpec_NP
   
  **Information required to determine an effective address at runtime**

  This is believed to be machine independent, modulo register numbers of course.

  The general representation for an address is a sum of two registers and a constant; this may
  change in future releases. Some architectures use only certain bits of a register (e.g. bits 25:31
  of XER register on the Power chip family); these are represented as pseudo-registers. The
  numbering scheme for registers and pseudo-registers is implementation dependent and should not
  be relied upon; it may change in future releases.

  The formula is ``regs[0] + 2^scale * regs[1] + imm``.

  .. cpp:member:: private long imm

      immediate

  .. cpp:member:: private unsigned int scale
  .. cpp:member:: private int regs[2]

      registers: -1 means none, 0 is 1st, 1 is 2nd and so on

  .. cpp:function:: BPatch_addrSpec_NP(long _imm, int _ra = -1, int _rb = -1, int _scale = 0)

    some pseudoregisters may be used

  .. cpp:function:: BPatch_addrSpec_NP()
  .. cpp:function:: long getImm() const
  .. cpp:function:: int getScale() const
  .. cpp:function:: int getReg(unsigned i) const
  .. cpp:function:: bool equals(const BPatch_addrSpec_NP& ar) const

  .. cpp:function:: int getImm()

    Return the constant offset. This may be positive or negative.

  .. cpp:function:: int getReg(unsigned i)

    Return the register number for the ``ith`` register in the sum,
    where ``0 <= i <= 2``. Register numbers are positive; a value of -1 means no
    register.

  .. cpp:function:: int getScale()

    Returns any scaling factor used in the memory address computation.
