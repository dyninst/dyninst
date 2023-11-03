/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/* $Id: MemoryAccess.C,v 1.13 2006/10/16 20:17:14 bernat Exp $ */

#include <stdlib.h>
#include <string.h>

#include "BPatch_memoryAccess_NP.h"
#include "BPatch_Vector.h"
#include "BPatch_point.h"

BPatch_addrSpec_NP::BPatch_addrSpec_NP(long _imm, int _ra, int _rb, int _scale) :
   imm(_imm), 
   scale(_scale)
{
   regs[0] = _ra;
   regs[1] = _rb;
}

BPatch_addrSpec_NP::BPatch_addrSpec_NP() : 
   imm(0), 
   scale(0)
{
   regs[0] = 0;
   regs[1] = 0;
}

long BPatch_addrSpec_NP::getImm() const 
{ 
   return imm; 
}

int BPatch_addrSpec_NP::getScale() const 
{ 
   return scale; 
}

int BPatch_addrSpec_NP::getReg(unsigned i) const 
{ 
   return regs[i]; 
}

bool BPatch_addrSpec_NP::equals(const BPatch_addrSpec_NP& ar) const
{
   return ((imm == ar.imm) && (scale == ar.scale) &&
           (regs[0] == ar.regs[0]) && (regs[1] == ar.regs[1]));
}

BPatch_Vector<BPatch_point*>* BPatch_memoryAccess::filterPoints(
		const BPatch_Vector<BPatch_point *> &points,
                unsigned int numMAs)
{
  BPatch_Vector<BPatch_point*> *result = new BPatch_Vector<BPatch_point *>;

  for(unsigned int i = 0; i < points.size(); ++i) {
    const BPatch_memoryAccess *ma = points[i]->getMemoryAccess();
    if(ma)
      if(ma->getNumberOfAccesses() >= numMAs)
        result->push_back(points[i]);
  }
  return result;
}
 
const BPatch_addrSpec_NP *BPatch_memoryAccess::getStartAddr(int which) const
{ 
   return &(start[which]);
}

const BPatch_countSpec_NP *BPatch_memoryAccess::getByteCount(int which) const
{ 
   return &(count[which]);
}

// initializes only the first access - general case
void BPatch_memoryAccess::set1st(bool _isLoad, bool _isStore,
            long _imm_s, int _ra_s, int _rb_s, unsigned int _scale_s,
            long _imm_c, int _ra_c, int _rb_c, unsigned int _scale_c,
            int _preFcn, int _cond, bool _nt)
{
   nacc = 1;
   isLoad[0] = _isLoad;
   isStore[0] = _isStore;
   start[0] = BPatch_addrSpec_NP(_imm_s, _ra_s, _rb_s, _scale_s);
   count[0] = BPatch_countSpec_NP(_imm_c, _ra_c, _rb_c, _scale_c);
   preFcn[0] = _preFcn;
   condition[0] = _cond;
   nonTemporal[0] = _nt;
}

  // initializes only the first access - no scale for count
void BPatch_memoryAccess::set1st(bool _isLoad, bool _isStore,
              long _imm_s, int _ra_s, int _rb_s, long _imm_c, int _ra_c, 
              int _rb_c, unsigned int _scale_s, int _preFcn, int _cond, 
              bool _nt)
{
   nacc = 1;
   isLoad[0] = _isLoad;
   isStore[0] = _isStore;
   start[0] = BPatch_addrSpec_NP(_imm_s, _ra_s, _rb_s, _scale_s);
   count[0] = BPatch_countSpec_NP(_imm_c, _ra_c, _rb_c);
   preFcn[0] = _preFcn;
   condition[0] = _cond;
   nonTemporal[0] = _nt;
}

BPatch_memoryAccess* BPatch_memoryAccess::init_tables()
{
    return NULL;
}

// initializes only the first access; #bytes is a constant
BPatch_memoryAccess::BPatch_memoryAccess(internal_instruction *insn, Dyninst::Address _addr,
                                         bool _isLoad, bool _isStore, unsigned int _bytes,
                                         long _imm, int _ra, int _rb, unsigned int _scale,
                                         int _cond, bool _nt) : 
    BPatch_instruction(insn, _addr)
{
    start = new BPatch_addrSpec_NP[nmaxacc_NP];
    count = new BPatch_countSpec_NP[nmaxacc_NP];
    set1st(_isLoad, _isStore, _imm, _ra, _rb, _bytes, -1, -1, _scale, -1, _cond,
           _nt);
}

// initializes only the first access; #bytes is an expression w/scale
BPatch_memoryAccess::BPatch_memoryAccess(internal_instruction *insn, Dyninst::Address _addr,
                    bool _isLoad, bool _isStore, long _imm_s, int _ra_s, int _rb_s,
                    unsigned int _scale_s, long _imm_c, int _ra_c, int _rb_c, 
                    unsigned int _scale_c, int _cond, bool _nt, int _preFcn) : 
   BPatch_instruction(insn, _addr)
{
   start = new BPatch_addrSpec_NP[nmaxacc_NP];
   count = new BPatch_countSpec_NP[nmaxacc_NP];
   set1st(_isLoad, _isStore, _imm_s, _ra_s, _rb_s, _scale_s, _imm_c, _ra_c, 
          _rb_c, _scale_c, _preFcn, _cond, _nt);
}

// initializes only the first access; #bytes is an expression
BPatch_memoryAccess::BPatch_memoryAccess(internal_instruction *insn, Dyninst::Address _addr,
                    bool _isLoad, bool _isStore, bool _isPrefetch, long _imm_s, 
                    int _ra_s, int _rb_s, long _imm_c, int _ra_c, int _rb_c,
                    unsigned short _preFcn) : 
   BPatch_instruction(insn, _addr)
{
   start = new BPatch_addrSpec_NP[nmaxacc_NP];
   count = new BPatch_countSpec_NP[nmaxacc_NP];
   assert(_isPrefetch);
   set1st(_isLoad, _isStore, _imm_s, _ra_s, _rb_s, _imm_c, _ra_c, _rb_c, 
          0, _preFcn);
}

  // initializes only the first access; #bytes is an expression & not a prefetch
BPatch_memoryAccess::BPatch_memoryAccess(internal_instruction *insn, Dyninst::Address _addr,
	       bool _isLoad, bool _isStore, long _imm_s, int _ra_s, int _rb_s,
	       long _imm_c, int _ra_c, int _rb_c) : 
   BPatch_instruction(insn, _addr)
{
   start = new BPatch_addrSpec_NP[nmaxacc_NP];
   count = new BPatch_countSpec_NP[nmaxacc_NP];
   set1st(_isLoad, _isStore, _imm_s, _ra_s, _rb_s, _imm_c, _ra_c, _rb_c);
}

  // sets 2nd access; #bytes is constant
void BPatch_memoryAccess::set2nd(bool _isLoad, bool _isStore, 
            unsigned int _bytes, long _imm, int _ra, int _rb, 
            unsigned int _scale)
{
   if(nacc >= 2)
      return;
   nacc = 2;
   isLoad[1] = _isLoad;
   isStore[1] = _isStore;
   start[1] = BPatch_addrSpec_NP(_imm, _ra, _rb, _scale);
   count[1] = BPatch_countSpec_NP(_bytes);
   preFcn[1] = -1;
   condition[1] = -1;
   nonTemporal[1] = false;
}

  // sets 2nd access; #bytes is an expression w/scale
void BPatch_memoryAccess::set2nd(bool _isLoad, bool _isStore,
            long _imm_s, int _ra_s, int _rb_s, unsigned int _scale_s,
              long _imm_c, int _ra_c, int _rb_c, unsigned int _scale_c,
              int _cond, bool _nt)
{
   if(nacc >= 2)
      return;
   nacc = 2;
   isLoad[1] = _isLoad;
   isStore[1] = _isStore;
   start[1] = BPatch_addrSpec_NP(_imm_s, _ra_s, _rb_s, _scale_s);
   count[1] = BPatch_countSpec_NP(_imm_c, _ra_c, _rb_c, _scale_c);
   preFcn[1] = -1;
   condition[1] = _cond;
   nonTemporal[1] = _nt;
}

// initializes both accesses; #bytes is a constant
BPatch_memoryAccess::BPatch_memoryAccess(internal_instruction *insn, Dyninst::Address _addr,
		      bool _isLoad, bool _isStore, unsigned int _bytes,
            long _imm, int _ra, int _rb, unsigned int _scale,
            bool _isLoad2, bool _isStore2, unsigned int _bytes2,
            long _imm2, int _ra2, int _rb2, unsigned int _scale2) : 
   BPatch_instruction(insn, _addr)
{
   start = new BPatch_addrSpec_NP[nmaxacc_NP];
   count = new BPatch_countSpec_NP[nmaxacc_NP];
   set1st(_isLoad, _isStore, _imm, _ra, _rb, _bytes, -1, -1, _scale);
   set2nd(_isLoad2, _isStore2, _bytes2, _imm2, _ra2, _rb2, _scale2);
}

  // initializes both accesses; #bytes is an expression & not a prefetch
BPatch_memoryAccess::BPatch_memoryAccess(internal_instruction *insn, Dyninst::Address _addr,
         bool _isLoad, bool _isStore, long _imm_s, int _ra_s, int _rb_s, 
         unsigned int _scale_s, long _imm_c, int _ra_c, int _rb_c, 
         unsigned int _scale_c, bool _isLoad2, bool _isStore2, long _imm2_s, 
         int _ra2_s, int _rb2_s, unsigned int _scale2_s, long _imm2_c, 
         int _ra2_c, int _rb2_c, unsigned int _scale2_c) : 
   BPatch_instruction(insn, _addr)
{
   start = new BPatch_addrSpec_NP[nmaxacc_NP];
   count = new BPatch_countSpec_NP[nmaxacc_NP];
   set1st(_isLoad, _isStore, 
          _imm_s, _ra_s, _rb_s, _scale_s,
          _imm_c, _ra_c, _rb_c, _scale_c,
          -1, -1, false);
   set2nd(_isLoad2, _isStore2,
          _imm2_s, _ra2_s, _rb2_s, _scale2_s,
          _imm2_c, _ra2_c, _rb2_c, _scale2_c,
          -1, false);
}


bool BPatch_memoryAccess::equals(const BPatch_memoryAccess* mp) const 
{ 
   return mp ? equals(*mp) : false; 
}

bool BPatch_memoryAccess::equals(const BPatch_memoryAccess& rp) const
{
   bool res = nacc == rp.nacc;
   
   if(!res)
      return res;
   
   for(unsigned int i=0; i<nacc; ++i) {
      res = res &&
         (isLoad[i] == rp.isLoad[i]) &&
         (isStore[i] == rp.isStore[i]) &&
         (start[i].equals(rp.start[i])) &&
         (count[i].equals(rp.count[i])) &&
         (preFcn[i] == rp.preFcn[i]) &&
         (condition[i] == rp.condition[i]) && 
         (nonTemporal[i] == rp.nonTemporal[i]);
      if(!res)
         break;
   }

   return res;
}

BPatch_addrSpec_NP BPatch_memoryAccess::getStartAddr_NP(int which) const 
{ 
   return start[which]; 
}

BPatch_countSpec_NP BPatch_memoryAccess::getByteCount_NP(int which) const 
{ 
   return count[which]; 
}

BPatch_memoryAccess::~BPatch_memoryAccess() {
   delete[] start;
   delete[] count;
}

BPatch_memoryAccess* const BPatch_memoryAccess::none = init_tables();
