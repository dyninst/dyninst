/* $Id: BPatch_memoryAccess_NP.h,v 1.9 2002/08/06 23:20:54 gaburici Exp $ */

#ifndef _MemoryAccess_h_
#define _MemoryAccess_h_

#include <stdlib.h>
#include <BPatch_point.h>

/* This is believed to be machine independent, modulo register numbers of course */
struct BPATCH_DLL_EXPORT BPatch_addrSpec_NP
{
  // the formula is regs[0] + 2 ^ scale * regs[1] + imm
  int imm;      // immediate
  unsigned int scale;
  int regs[2];  // registers: -1 means none, 0 is 1st, 1 is 2nd and so on

public:
  BPatch_addrSpec_NP(int _imm, int _ra = -1, int _rb = -1, int _scale = 0)
    :  imm(_imm), scale(_scale)
  {
    regs[0] = _ra;
    regs[1] = _rb;
  }

  BPatch_addrSpec_NP() : imm(0), scale(0)
  {
    regs[0] = 0;
    regs[1] = 0;
  }

  int getImm() const { return imm; }
  int getScale() const { return scale; }
  int getReg(unsigned i) const { return regs[i]; }

  bool equals(const BPatch_addrSpec_NP& ar) const
  {
    return
      (imm == ar.imm) &&
      (regs[0] == ar.regs[0]) &&
      (regs[1] == ar.regs[1]);
  }
};

#define BPatch_countSpec_NP BPatch_addrSpec_NP // right now it has the same form

class BPatch_memoryAccess;
extern void initOpCodeInfo();


class BPATCH_DLL_EXPORT BPatch_memoryAccess
{
  friend class BPatch_function;
  friend class AstNode;
  friend class InstrucIter;

 public:
  // maximum number of memory accesses per instruction; platform dependent
#if defined(i386_unknown_nt4_0)
  // Translation from C++ to VC++ 6.0
#define nmaxacc_NP 2
#else
#if defined(i386_unknown_linux2_0)
  static const unsigned int nmaxacc_NP = 2;
#else
  static const unsigned int nmaxacc_NP = 1;
#endif
#endif

  // Utility function to filter out the points that don't have a 2nd memory access on x86
  static BPatch_Vector<BPatch_point*>* filterPoints(const BPatch_Vector<BPatch_point*> &points,
                                                    unsigned int numMAs);
  
 private:
  unsigned int nacc;
  bool isLoad[nmaxacc_NP];  // can both be true on some arches like x86 (or even SPARC)
  bool isStore[nmaxacc_NP];
  bool isPrefetch[nmaxacc_NP];

  BPatch_addrSpec_NP start[nmaxacc_NP];
  BPatch_countSpec_NP count[nmaxacc_NP];
  short preFcn[nmaxacc_NP]; // prefetch function, currently the codes on SPARC (-1 = none)

 protected:
  bool hasALoad() const { return nacc == 1 ? isLoad[0] : (isLoad[0] || isLoad[1]); }
  bool hasAStore() const { return nacc == 1 ? isStore[0] : (isStore[0] || isStore[1]); }
  bool hasAPrefetch() const { return isPrefetch[0]; }
  short prefetchType(int which = 0) { return preFcn[which]; }

  BPatch_addrSpec_NP getStartAddr(int which = 0) const { return start[which]; }
  BPatch_countSpec_NP getByteCount(int which = 0) const { return count[which]; }

 public:
  static BPatch_memoryAccess* const none;

  // initializes only the first access; #bytes is a constant
  BPatch_memoryAccess(bool _isLoad, bool _isStore, unsigned int _bytes,
                      int _imm, int _ra, int _rb, unsigned int _scale = 0)
  {
    nacc = 1;
    isLoad[0] = _isLoad;
    isStore[0] = _isStore;
    isPrefetch[0] = false;
    start[0] = BPatch_addrSpec_NP(_imm, _ra, _rb, _scale);
    count[0] = BPatch_countSpec_NP(_bytes);
    preFcn[0] = -1;
  }

  static BPatch_memoryAccess* init_tables()
  {
    initOpCodeInfo();
    return NULL;
  }

  // initializes only the first access; #bytes is an expression
  BPatch_memoryAccess(bool _isLoad, bool _isStore, bool _isPrefetch,
                      int _imm_s, int _ra_s, int _rb_s,
                      int _imm_c, int _ra_c, int _rb_c,
                      unsigned short _preFcn)
  {
    nacc = 1;
    isLoad[0] = _isLoad;
    isStore[0] = _isStore;
    isPrefetch[0] = _isPrefetch;
    start[0] = BPatch_addrSpec_NP(_imm_s, _ra_s, _rb_s);
    count[0] = BPatch_countSpec_NP(_imm_c, _ra_c, _rb_c);
    preFcn[0] = _preFcn;
  }

  // initializes only the first access; #bytes is an expression & not a prefetch
  BPatch_memoryAccess(bool _isLoad, bool _isStore,
	       int _imm_s, int _ra_s, int _rb_s,
	       int _imm_c, int _ra_c, int _rb_c)
  {
    nacc = 1;
    isLoad[0] = _isLoad;
    isStore[0] = _isStore;
    isPrefetch[0] = false;
    start[0] = BPatch_addrSpec_NP(_imm_s, _ra_s, _rb_s);
    count[0] = BPatch_countSpec_NP(_imm_c, _ra_c, _rb_c);
    preFcn[0] = -1;
  }

  // sets 2nd access; #bytes is constant
  void set2nd(bool _isLoad, bool _isStore, unsigned int _bytes,
              int _imm, int _ra, int _rb, unsigned int _scale = 0)
  {
    if(nacc >= 2)
      return;
    nacc = 2;
    isLoad[1] = _isLoad;
    isStore[1] = _isStore;
    isPrefetch[1] = false;
    start[1] = BPatch_addrSpec_NP(_imm, _ra, _rb, _scale);
    count[1] = BPatch_countSpec_NP(_bytes);
    preFcn[1] = -1;
  }

  // initializes both accesses; #bytes is a constant
  BPatch_memoryAccess(bool _isLoad, bool _isStore, unsigned int _bytes,
                      int _imm, int _ra, int _rb, unsigned int _scale,
                      bool _isLoad2, bool _isStore2, unsigned int _bytes2,
                      int _imm2, int _ra2, int _rb2, unsigned int _scale2)
  {
    nacc = 1;
    isLoad[0] = _isLoad;
    isStore[0] = _isStore;
    isPrefetch[0] = false;
    start[0] = BPatch_addrSpec_NP(_imm, _ra, _rb, _scale);
    count[0] = BPatch_countSpec_NP(_bytes);
    preFcn[0] = -1;

    set2nd(_isLoad2, _isStore2, _bytes2, _imm2, _ra2, _rb2, _scale2);
  }

  bool equals(const BPatch_memoryAccess* mp) const { return mp ? equals(*mp) : false; }

  bool equals(const BPatch_memoryAccess& rp) const
  {
    bool res = nacc == rp.nacc;

    if(!res)
      return res;

    for(unsigned int i=0; i<nacc; ++i) {
      res = res &&
        (isLoad[i] == rp.isLoad[i]) &&
        (isStore[i] == rp.isStore[i]) &&
        (isPrefetch[i] == rp.isPrefetch[i]) &&
        (start[i].equals(rp.start[i])) &&
        (count[i].equals(rp.count[i])) &&
        (preFcn[i] == rp.preFcn[i]);
      if(!res)
        break;
    }

    return res;
  }

  unsigned int getNumberOfAccesses() const { return nacc; }
  BPatch_addrSpec_NP getStartAddr_NP(int which = 0) const { return start[which]; }
  BPatch_countSpec_NP getByteCount_NP(int which = 0) const { return count[which]; }
  bool isALoad_NP(int which = 0) const { return isLoad[which]; }
  bool isAStore_NP(int which = 0) const { return isStore[which]; }
  bool isAPrefetch_NP(int which = 0) const { return isPrefetch[which]; }
  short prefetchType_NP(int which = 0) const { return preFcn[which]; }

};

#endif
