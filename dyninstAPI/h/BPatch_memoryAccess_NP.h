#ifndef _MemoryAccess_h_
#define _MemoryAccess_h_

/* This is believed to be machine independent, modulo register numbers of course */
struct AddrSpec
{
  int imm;      // immediate
  int regs[2];  // registers: -1 means none, 0 is 1st, 1 is 2nd and so on
public:
  AddrSpec(int _imm, int _ra = -1, int _rb = -1)
    :  imm(_imm)
  {
    regs[0] = _ra;
    regs[1] = _rb;
  }

  int getImm() { return imm; }
  int getReg(unsigned i) { return regs[i]; }

  bool equals(const AddrSpec& ar)
  {
    return
      (imm == ar.imm) &&
      (regs[0] == ar.regs[0]) &&
      (regs[1] == ar.regs[1]);
  }
};

#define CountSpec AddrSpec // right now it has the same form

class MemoryAccess;
extern void initOpCodeInfo();

/* This is believed to be machine independent, modulo register numbers of course */
class MemoryAccess
{
  friend class BPatch_function;

  bool isLoad;  // can both be true on some arches like x86 (or even SPARC)
  bool isStore;
  bool isPrefetch;

  AddrSpec start;
  CountSpec count;
  short preFcn; // prefetch function, currently the codes on SPARC (-1 = none)

  bool isALoad_NP() { return isLoad; }
  bool isAStore_NP() { return isStore; }
  bool isAPrefetch_NP() { return isPrefetch; }
  short prefetchType_NP() { return preFcn; }

  AddrSpec getStartAddr_NP() { return start; }
  CountSpec getByteCount_NP() { return count; }

protected:
  bool isALoad() { return isLoad; }
  bool isAStore() { return isStore; }
  bool isAPrefetch() { return isPrefetch; }
  short prefetchType() { return preFcn; }

  AddrSpec getStartAddr() { return start; }
  CountSpec getByteCount() { return count; }

public:
  static const MemoryAccess none;

  MemoryAccess(bool _isLoad, bool _isStore, unsigned int _bytes,
	       int _imm, int _ra, int _rb)
    : isLoad(_isLoad), isStore(_isStore), isPrefetch(false),
      start(_imm, _ra, _rb), count(_bytes), preFcn(-1)
  {} // this one exists for historical reasons (i.e. I'm lazy to rewrite)

  MemoryAccess(bool _isLoad, bool _isStore)
    : isLoad(_isLoad), isStore(_isStore), start(0), count(0)
  {
    // initialize the op table here (if any exists on a given platform).
    initOpCodeInfo();
  }

  MemoryAccess(bool _isLoad, bool _isStore,
	       int _imm_s, int _ra_s, int _rb_s,
	       int _imm_c, int _ra_c, int _rb_c)
    : isLoad(_isLoad), isStore(_isStore), isPrefetch(false),
      start(_imm_s, _ra_s, _rb_s), count(_imm_c, _ra_c, _rb_c),
      preFcn(-1)
  {}

  MemoryAccess(bool _isLoad, bool _isStore, bool _isPrefetch,
	       int _imm_s, int _ra_s, int _rb_s,
	       int _imm_c, int _ra_c, int _rb_c,
               unsigned short _preFcn)
    : isLoad(_isLoad), isStore(_isStore), isPrefetch(_isPrefetch),
      start(_imm_s, _ra_s, _rb_s), count(_imm_c, _ra_c, _rb_c),
      preFcn(_preFcn)
    {}

  bool equals(const MemoryAccess* mp) { return equals(*mp); }

  bool equals(const MemoryAccess& rp)
  {
    return
      (isLoad == rp.isLoad) &&
      (isStore == rp.isStore) &&
      (isPrefetch == rp.isPrefetch) &&
      (start.equals(rp.start)) &&
      (count.equals(rp.count)) &&
      (preFcn == rp.preFcn);
  }
};
#endif
