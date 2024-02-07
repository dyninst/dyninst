.. _`sec:BPatch_memoryAccess_NP.h`:

BPatch_memoryAccess_NP.h
########################

.. cpp:class:: BPatch_memoryAccess : public BPatch_instruction

  **A memory access abstraction**

  It contains information that describes the memory access type: read, write,
  read/write, or prefetch. It also contains information that allows the
  effective address and the number of bytes transferred to be determined.

  .. cpp:member:: static BPatch_memoryAccess* const none

  .. cpp:function:: static BPatch_Vector<BPatch_point*>* filterPoints(const BPatch_Vector<BPatch_point*> &points, unsigned int numMAs)

    Utility function to filter out the points that don't have a 2nd memory   access on x86

  .. cpp:function:: const BPatch_addrSpec_NP *getStartAddr(int which = 0) const
  .. cpp:function:: const BPatch_countSpec_NP *getByteCount(int which = 0) const

  .. cpp:function:: static BPatch_memoryAccess* init_tables()
  .. cpp:function:: BPatch_memoryAccess(internal_instruction *, Dyninst::Address _addr, bool _isLoad, bool _isStore, unsigned int _bytes, long _imm, int _ra, int _rb, unsigned int _scale = 0, int _cond = -1, bool _nt = false)

    initializes only the first access #bytes is a constant

  .. cpp:function:: BPatch_memoryAccess(internal_instruction *insn, Dyninst::Address _addr, bool _isinternal_Load, bool _isStore, long _imm_s, int _ra_s, int _rb_s, unsigned int _scale_s, long _imm_c, int _ra_c, int _rb_c, unsigned int _scale_c, int _cond, bool _nt, int _preFcn = -1)

    initializes only the first access #bytes is an expression wscale

  .. cpp:function:: BPatch_memoryAccess(internal_instruction *insn, Dyninst::Address _addr, bool _isLoad, bool _isStore, bool _isPrefetch, long _imm_s, int _ra_s, int _rb_s, long _imm_c, int _ra_c, int _rb_c, unsigned short _preFcn)

    initializes only the first access #bytes is an expression

  .. cpp:function:: BPatch_memoryAccess(internal_instruction *insn, Dyninst::Address _addr, bool _isLoad, bool _isStore, long _imm_s, int _ra_s, int _rb_s, long _imm_c, int _ra_c, int _rb_c)

    initializes only the first access #bytes is an expression & not a prefetch

  .. cpp:function:: void set2nd(bool _isLoad, bool _isStore, unsigned int _bytes, long _imm, int _ra, int _rb, unsigned int _scale = 0)

    sets 2nd access #bytes is constant

  .. cpp:function:: void set2nd(bool _isLoad, bool _isStore, long _imm_s, int _ra_s, int _rb_s, unsigned int _scale_s, long _imm_c, int _ra_c, int _rb_c, unsigned int _scale_c, int _cond, bool _nt)

    sets 2nd access #bytes is an expression wscale

  .. cpp:function:: BPatch_memoryAccess(internal_instruction *insn, Dyninst::Address _addr, bool _isLoad, bool _isStore, unsigned int _bytes, long _imm, int _ra, int _rb, unsigned int _scale, bool _isLoad2, bool _isStore2, unsigned int _bytes2, long _imm2, int _ra2, int _rb2, unsigned int _scale2)

    initializes both accesses #bytes is a constant

  .. cpp:function:: BPatch_memoryAccess(internal_instruction *insn, Dyninst::Address _addr, bool _isLoad, bool _isStore, long _imm_s, int _ra_s, int _rb_s, unsigned int _scale_s, long _imm_c, int _ra_c, int _rb_c, unsigned int _scale_c, bool _isLoad2, bool _isStore2, long _imm2_s, int _ra2_s, int _rb2_s, unsigned int _scale2_s, long _imm2_c, int _ra2_c, int _rb2_c, unsigned int _scale2_c)

    initializes both accesses #bytes is an expression & not a prefetch

  .. cpp:function:: virtual ~BPatch_memoryAccess()
  .. cpp:function:: bool equals(const BPatch_memoryAccess* mp) const
  .. cpp:function:: bool equals(const BPatch_memoryAccess& rp) const

  .. cpp:function:: BPatch_addrSpec_NP getStartAddr_NP(int which = 0) const

    Return an address specification that allows the effective address of a
    memory reference to be computed. For example, on the x86 platform a
    memory access instruction operand may contain a base register, an index
    register, a scaling value, and a constant base. The BPatch_addrSpec_NP
    describes each of these values.

  .. cpp:function:: BPatch_countSpec_NP getByteCount_NP(int which = 0) const

    Return a specification that describes the number of bytes transferred by
    the memory access.
