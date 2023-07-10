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

#ifndef _StackMod_h_
#define _StackMod_h_

#include <string>
#include "BPatch_dll.h"
#include "stackanalysis.h"

class BPatch_function;
class StackModChecker;
class StackAccess;

struct stackmod_exception : public std::runtime_error {
    stackmod_exception(std::string what) : std::runtime_error(std::string("StackMod failure: ") + what) {}
};

#define STACKMOD_ASSERT(X) \
if(!(X)) { \
    throw(stackmod_exception(#X));\
}

class BPATCH_DLL_EXPORT StackMod
{
    public:
        enum MType
        {
            INSERT,
            REMOVE,
            MOVE,
            CANARY,
            RANDOMIZE
        };

        enum MOrder
        {
            NEW,
            CLEANUP
        };

        // Constructor
        StackMod() {}

        // Functions
        MType type() const;
        MOrder order() const { return _order; }

        // Virtual Functions
	virtual ~StackMod() = default;
        virtual std::string format() const { return ""; }

    protected:
        MOrder _order{};
        MType _type{};
};

/* Modification to insert stack space at [low, high) */
class BPATCH_DLL_EXPORT Insert : public StackMod
{
    friend class StackModChecker; 
    public:
        // Constructor
        Insert(int, int);

        // Functions
        int low() const { return _low; }
        int high() const { return _high; }

        unsigned int size() const { return _high - _low; }
        std::string format() const;

    private:
        Insert(MOrder, int, int);

        int _low{};
        int _high{};
};

/* Modification to remove stack space at [low, high) */
class BPATCH_DLL_EXPORT Remove : public StackMod
{
    friend class StackModChecker; 
    public:
        // Constructor
        Remove(int, int);

        // Functions
        int low() const { return _low; }
        int high() const { return _high; }

        unsigned int size() const { return _high - _low; }
        std::string format() const;

    private:
        Remove(MOrder, int, int);

        int _low{};
        int _high{};
};

/* Modification to move stack space from [srcLow, srcHigh)
 * to [destLow, destLow+(srcHigh-srcLow)) */
class BPATCH_DLL_EXPORT Move : public StackMod
{
    public:
        // Constructor
        Move(int, int, int);

        // Functions
        int srcLow() const { return _srcLow; }
        int srcHigh() const { return _srcHigh; }
        int destLow() const { return _destLow; }
        int destHigh() const { return _destHigh; }

        int size() const { return _srcHigh - _srcLow; }

        std::string format() const;

    private:
        int _srcLow{};
        int _srcHigh{};
        int _destLow{};
        int _destHigh{};
};

/* Modification to insert a stack canary at function entry
 * (and canary verification at function exit(s)) */
class BPATCH_DLL_EXPORT Canary : public StackMod
{
    public:
        // Constructors

        // The default constructor uses a canary check failure function
        // provided by libc (__stack_chk_fail); this constructor is only
        // valid on Linux, and libc must be present in the address space.
        Canary();

        // This constructor will use failFunc as the canary check failure
        // function. failFunc cannot take any arguments
        Canary(BPatch_function* failFunc);

        // Functions
        int low() const { return _low; }
        int high() const { return _high; }
        
        void init(int l, int h) {
            _low = l;
            _high = h;
        }
        
        BPatch_function* failFunc() const { return _failFunc; }

        std::string format() const;

    private:
        int _low{};
        int _high{};
        BPatch_function* _failFunc{};
};

/* Modification to randomize the locations of the DWARF-specified
 * local variables. This modification has no effect on functions
 * without DWARF information. */
class BPATCH_DLL_EXPORT Randomize : public StackMod
{
    public:
        // Constructor
        Randomize();
        Randomize(int);

        bool isSeeded() const { return _isSeeded; }
        int seed() const { return _seed; }
        
        std::string format() const;

    private:
        bool _isSeeded{};
        int _seed{};
};

#endif
