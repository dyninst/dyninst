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

#ifndef _STACKLOCATION_H_
#define _STACKLOCATION_H_

#include <string>
#include <functional>

#include "registers/MachRegister.h"
#include "common/src/IntervalTree.h"
#include "stackanalysis.h"
#include "dyntypes.h"
#include "StackAccess.h"

using namespace Dyninst;

typedef IntervalTree<Dyninst::Address, int> ValidPCRange;

class StackLocation {
    public:
        StackLocation(StackAnalysis::Height o, int s, StackAccess::StackAccessType t, bool h, ValidPCRange* v = NULL) :
            _type(t),
            _size(s),
            _isStackMemory(true),
            _off(o),
            _isRegisterHeight(h),
            _isRegister(false),
            _isNull(false),
            _valid(v)
    {}

        StackLocation(StackAnalysis::Height o, StackAccess::StackAccessType t, MachRegister r, ValidPCRange* v = NULL) :
            _type(t),
            _size(1),
            _isStackMemory(true),
            _off(o),
            _isRegisterHeight(true),
            _isRegister(false),
            _reg(r),
            _isNull(false),
            _valid(v)
    {}

        StackLocation(MachRegister r, int s) :
            _type(StackAccess::StackAccessType::UNKNOWN),
            _size(s),
            _isStackMemory(false),
            _isRegister(true),
            _reg(r),
            _isNull(false),
            _valid(NULL)
    {}

        StackLocation() :
            _type(StackAccess::StackAccessType::UNKNOWN),
            _size{},
            _isStackMemory(false),
            _isRegister(false),
			_reg{},
            _isNull(true),
            _valid(NULL)
    {}


        StackAccess::StackAccessType type() const { return _type; }
        void setType(StackAccess::StackAccessType t) { _type = t; }

        int size() const { return _size; }
        void setSize(int s) { _size = s; }

        bool isNull() const { return _isNull; }

        bool isStackMemory() const { return _isStackMemory; }
        StackAnalysis::Height off() const { return _off; }
        void setOff(StackAnalysis::Height o) { _off = o; }
        bool isRegisterHeight() const { return _isRegisterHeight; }

        bool isRegister() const { return _isRegister; }
        MachRegister reg() const { return _reg; }
        void setReg(MachRegister r) { _reg = r; }

        ValidPCRange* valid() const { return _valid; }

        std::string format() const;

        bool operator<(const StackLocation& rhs) const {
            if (isStackMemory()) {
                return this->_off < rhs._off;
            } else {
                return this->_reg < rhs._reg;
            }
        }

    private:
        StackAccess::StackAccessType _type;

        int _size;

        bool _isStackMemory;
        StackAnalysis::Height _off;
        bool _isRegisterHeight{};

        bool _isRegister;
        MachRegister _reg;

        bool _isNull;

        ValidPCRange* _valid;
};

struct less_StackLocation
{
    bool operator()(StackLocation* a, StackLocation* b) const {
        if (a->isStackMemory() && b->isStackMemory()) {
            if (a->off().height() == b->off().height()) {
                if (a->isRegisterHeight() && b->isRegisterHeight()) {
                    if (a->off().height() == b->off().height()) {
                        return a->reg() < b->reg();
                    } else {
                        return a->off().height() < b->off().height();
                    }
                } else if (a->isRegisterHeight()) {
                    return true;
                } else if (b->isRegisterHeight()) {
                    return false;
                } else {
                    return true;
                }
            }
            else {
                return a->off().height() < b->off().height();
            }
        } else if (a->isRegister() && b->isRegister()){
            return a->reg() < b->reg();
        }
        return true;
    }
};

class tmpObject
{
    public:
        tmpObject(long o, int s, StackAccess::StackAccessType t, ValidPCRange* v = NULL) :
            _offset(o), _size(s), _type(t), _valid(v) {}

        long offset() const { return _offset; }
        int size() const { return _size; }
        StackAccess::StackAccessType type() const { return _type; }
        ValidPCRange* valid() const { return _valid; }

    private:
        long _offset;
        int _size;
        StackAccess::StackAccessType _type;
        ValidPCRange* _valid;
};

struct less_tmpObject
{
    bool operator()(tmpObject a, tmpObject b) const {
        if (a.offset() < b.offset()) {
            return true;
        } else if (a.offset() == b.offset()) {
            if (a.type() == b.type()) {
                return a.size() <= b.size();
            } else {
                return a.type() < b.type();
            }
        } else {
            return false;
        }
    }
};

#endif
