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

// $Id: arch-power.h,v 1.45 2008/03/25 19:24:23 bernat Exp $

#ifndef _ARCH_AMDGPU_H
#define _ARCH_AMDGPU_H

// Code generation

#include "dyn_regs.h"
#include <vector>
#include "dyntypes.h"
typedef unsigned long Address;
class AddressSpace;

namespace NS_amdgpu {
/*
 * Define amdgpu instruction information.
 *
 */

#define INSN_SET(I, s, e, v)    ((I).setBits(s, e - s + 1, (v)))


typedef const unsigned int insn_mask;
typedef void codeBuf_t;
typedef unsigned codeBufIndex_t;


class COMMON_EXPORT instruction {
	public:

    char * raw_;
    instruction() : raw_(NULL), allocated_(false) , size_(0) { }
    instruction(uint32_t size) : raw_(NULL), allocated_(false) , size_(0) {
        raw_ = (char * ) malloc(size);
        assert(raw_);
        allocated_ = true;
        size_ = size;
    }
    void setSize(uint32_t newSize){
        if(size_ > newSize)
            return;
        if(!allocated_){
            raw_ = (char *) malloc(newSize);
            assert(raw_);
            allocated_ = true;
            size_ = newSize;
        }else{
            raw_ = (char *) ::realloc(raw_,newSize);
            assert(raw_);
            allocated_ = true;
            size_ = newSize;

        }

    }
    ~instruction(){
        if(allocated_ && raw_){
            free(raw_);
            raw_ =0 ;
            allocated_ = false;
            size_= 0;
        }
    }
/*
    instruction(const void *ptr) {
      insn_ = *((const instructUnion *)ptr);
    }
    instruction(const void *ptr, bool) {
      insn_ = *((const instructUnion *)ptr);
    }

    instruction(const instruction &insn) :        insn_(insn.insn_) {}
    instruction(instructUnion &insn) :
        insn_(insn) {}*/

    instruction *copy() const;

    //void clear() { insn_.raw = 0; }
    void setInstruction(codeBuf_t *ptr, Address = 0);
    void setBits(unsigned int/* pos*/, unsigned int /*len*/, unsigned int/* value*/) {
        /*unsigned int mask;

        mask = ~((unsigned int)(~0) << len);
        value = value & mask;

        mask = ~(mask << pos);
        value = value << pos;

        insn_.raw = insn_.raw & mask;
        insn_.raw = insn_.raw | value;*/
    }
    //unsigned int asInt() const { return insn_.raw; }
    void setInstruction(unsigned char *ptr, Address = 0);


    unsigned size() const{ return size_; }
	private:
    bool allocated_;
    uint32_t size_;

};

}
//end of NS_amdgpu

#endif
