#ifndef Rose_ConcreteSemantics2_H
#define Rose_ConcreteSemantics2_H

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

#include <iosfwd>
#include <stddef.h>
#include <inttypes.h>

#include "../integerOps.h"
#include "BaseSemantics2.h"
#include "MemoryMap.h"
#include "RegisterStateGeneric.h"
#include "../util/BitVector.h"

namespace rose {
    namespace BinaryAnalysis {              // documented elsewhere
        namespace InstructionSemantics2 {       // documented elsewhere

/** A concrete semantic domain.
 *
 *  Semantics in a concrete domain, where all values are actual known bits.  Since the symbolic domain does constant folding,
 *  it also can be used as a concrete domain, although using ConcreteSemantics is probably somewhat faster. */
            namespace ConcreteSemantics {

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Value type
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** Smart-ownership pointer to a concrete semantic value. See @ref heap_object_shared_ownership. */
                typedef Sawyer::SharedPointer<class SValue> SValuePtr;

/** Formatter for symbolic values. */
                typedef BaseSemantics::Formatter Formatter;             // we might extend this in the future

/** Type of values manipulated by the concrete domain.
 *
 *  Values of type type are used whenever a value needs to be stored, such as memory addresses, the values stored at those
 *  addresses, the values stored in registers, the operands for RISC operations, and the results of those operations.  Each
 *  value has a known size and known bits.  All RISC operations on these values will produce a new known value. This type is
 *  not capable of storing an undefined value, and any attempt to create an undefined value will create a value with all bits
 *  cleared instead. */
                class SValue : public BaseSemantics::SValue {
                protected:
                    Sawyer::Container::BitVector bits_;

                    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                    // Real constructors
                protected:
                    explicit SValue(size_t nbits) : BaseSemantics::SValue(nbits), bits_(nbits) { }

                    SValue(size_t nbits, uint64_t number) : BaseSemantics::SValue(nbits) {
                        bits_ = Sawyer::Container::BitVector(nbits);
                        bits_.fromInteger(number);
                    }

                    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                    // Static allocating constructors
                public:
                    /** Instantiate a new prototypical value. Prototypical values are only used for their virtual constructors. */
                    static SValuePtr instance() {
                        return SValuePtr(new SValue(1));
                    }

                    /** Instantiate a new undefined value of specified width.
                     *
                     *  This semantic domain has no representation for an undefined values--all values are defined. Therefore any attempt to
                     *  create an undefined value will result in all bits being set to zero instead. */
                    static SValuePtr instance(size_t nbits) {
                        return SValuePtr(new SValue(nbits));
                    }

                    /** Instantiate a new concrete value. */
                    static SValuePtr instance(size_t nbits, uint64_t value) {
                        return SValuePtr(new SValue(nbits, value));
                    }

                    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                    // Virtual allocating constructors
                public:
                    virtual BaseSemantics::SValuePtr undefined_(size_t nbits) const {
                        return instance(nbits);
                    }

                    virtual BaseSemantics::SValuePtr unspecified_(size_t nbits) const {
                        return instance(nbits);
                    }

                    virtual BaseSemantics::SValuePtr bottom_(size_t nbits) const {
                        return instance(nbits);
                    }

                    virtual BaseSemantics::SValuePtr number_(size_t nbits, uint64_t value) const {
                        return instance(nbits, value);
                    }

                    virtual BaseSemantics::SValuePtr boolean_(bool value) const {
                        return instance(1, value ? 1 : 0);
                    }

                    virtual BaseSemantics::SValuePtr copy(size_t new_width = 0) const {
                        SValuePtr retval(new SValue(*this));
                        if (new_width != 0 && new_width != retval->get_width())
                            retval->set_width(new_width);
                        return retval;
                    }

                    virtual Sawyer::Optional <BaseSemantics::SValuePtr>
                            createOptionalMerge(const BaseSemantics::SValuePtr &other, const BaseSemantics::MergerPtr &,
                                                SMTSolver *) const;

                    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                    // Dynamic pointer casts
                public:
                    /** Promote a base value to a SymbolicSemantics value.  The value @p v must have a SymbolicSemantics::SValue dynamic type. */
                    static SValuePtr promote(const BaseSemantics::SValuePtr &v) { // hot
                        SValuePtr retval = v.dynamicCast<SValue>();
                        ASSERT_not_null(retval);
                        return retval;
                    }

                    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                    // Override virtual methods...
                public:
                    virtual bool may_equal(const BaseSemantics::SValuePtr &other, SMTSolver *solver = NULL) const;

                    virtual bool must_equal(const BaseSemantics::SValuePtr &other, SMTSolver *solver = NULL) const;

                    virtual void set_width(size_t nbits);

                    virtual bool isBottom() const {
                        return false;
                    }

                    virtual bool is_number() const {
                        return true;
                    }

                    virtual uint64_t get_number() const;

                    virtual void print(std::ostream &, BaseSemantics::Formatter &) const;

                    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                    // Additional methods first declared in this class...
                public:
                    /** Returns the bit vector storing the concrete value.
                     *
                     * @{ */
                    virtual const Sawyer::Container::BitVector &bits() const { return bits_; }

                    virtual void bits(const Sawyer::Container::BitVector &);
                    /** @} */
                };


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Register State
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

                typedef BaseSemantics::RegisterStateGeneric RegisterState;
                typedef BaseSemantics::RegisterStateGenericPtr RegisterStatePtr;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Memory State
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** Shared-ownership pointer to a concrete memory state. See @ref heap_object_shared_ownership. */
                typedef boost::shared_ptr<class MemoryState> MemoryStatePtr;

/** Byte-addressable memory.
 *
 *  This class represents an entire state of memory via MemoryMap, allocating new memory in units of pages (the size of a page
 *  is configurable. */
                class MemoryState : public BaseSemantics::MemoryState {
                    MemoryMap map_;
                    rose_addr_t pageSize_;

                    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                    // Real constructors
                protected:
                    explicit MemoryState(const BaseSemantics::SValuePtr &addrProtoval,
                                         const BaseSemantics::SValuePtr &valProtoval)
                            : BaseSemantics::MemoryState(addrProtoval, valProtoval), pageSize_(4096) {
                        (void) SValue::promote(addrProtoval);           // for its checking side effects
                        (void) SValue::promote(valProtoval);
                    }

                    MemoryState(const MemoryState &other)
                            : BaseSemantics::MemoryState(other), map_(other.map_), pageSize_(other.pageSize_) {
                        BOOST_FOREACH(MemoryMap::Segment & segment, map_.values())
                        segment.buffer()->copyOnWrite(true);
                    }

                    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                    // Static allocating constructors
                public:
                    /** Instantiates a new memory state having specified prototypical value.
                     *
                     *  The @p addrProtoval and @p valProtoval must both be of ConcreteSemantics::SValue type or derived classes. */
                    static MemoryStatePtr instance(const BaseSemantics::SValuePtr &addrProtoval,
                                                   const BaseSemantics::SValuePtr &valProtoval) {
                        return MemoryStatePtr(new MemoryState(addrProtoval, valProtoval));
                    }

                    /** Instantiates a new deep copy of an existing state.
                     *
                     *  For efficiency purposes, the data buffers are not copied immediately but rather marked as copy-on-write. However, the
                     *  newly constructed memory map will have its own segments, which hold the segment names, access permissions, etc. */
                    static MemoryStatePtr instance(const MemoryStatePtr &other) {
                        return MemoryStatePtr(new MemoryState(*other));
                    }

                    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                    // Virtual constructors
                public:
                    /** Virtual constructor.
                     *
                     *  Creates a memory state having specified prototypical value, which should be of type ConcreteSemantics::SValue or
                     *  subclasses. */
                    virtual BaseSemantics::MemoryStatePtr create(const BaseSemantics::SValuePtr &addrProtoval,
                                                                 const BaseSemantics::SValuePtr &valProtoval) const

                    {
                        return instance(addrProtoval, valProtoval);
                    }

                    /** Virtual copy constructor.
                     *
                     *  Creates a new deep copy of this memory state. For efficiency purposes, the data buffers are not copied immediately but
                     *  rather marked as copy-on-write.  However, the newly constructed memory map will have its own segments, which hold the
                     *  segment names, access permissions, etc. */
                    virtual BaseSemantics::MemoryStatePtr clone() const {
                        return MemoryStatePtr(new MemoryState(*this));
                    }

                    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                    // Dynamic pointer casts
                public:
                    /** Recasts a base pointer to a concrete memory state. This is a checked cast that will fail if the specified pointer does
                     *  not have a run-time type that is a ConcreteSemantics::MemoryState or subclass thereof. */
                    static MemoryStatePtr promote(const BaseSemantics::MemoryStatePtr &x) {
                        MemoryStatePtr retval = boost::dynamic_pointer_cast<MemoryState>(x);
                        ASSERT_not_null(retval);
                        return retval;
                    }

                    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                    // Methods we inherited
                public:
                    virtual void clear() {
                        map_.clear();
                    }

                    virtual void print(std::ostream &, Formatter &) const;

                    virtual BaseSemantics::SValuePtr readMemory(const BaseSemantics::SValuePtr &addr,
                                                                const BaseSemantics::SValuePtr &dflt,
                                                                BaseSemantics::RiscOperators *addrOps,
                                                                BaseSemantics::RiscOperators *valOps);

                    virtual void writeMemory(const BaseSemantics::SValuePtr &addr,
                                             const BaseSemantics::SValuePtr &value,
                                             BaseSemantics::RiscOperators *addrOps,
                                             BaseSemantics::RiscOperators *valOps);

                    virtual bool merge(const BaseSemantics::MemoryStatePtr &other,
                                       BaseSemantics::RiscOperators *addrOps,
                                       BaseSemantics::RiscOperators *valOps);

                    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                    // Methods first declared in this class
                public:
                    /** Returns the memory map. */
                    const MemoryMap &memoryMap() const { return map_; }

                    /** Set memory map.
                     *
                     *  If the specified map's areas not not in units of pages then padding segments will be added to this memory state. The
                     *  padding segments will either have the accessibility specified by @p padAccess, or will have the same accessibility as
                     *  the memory region being padded.  All padding segments will be named "padding". */
                    void memoryMap(const MemoryMap &, Sawyer::Optional<unsigned> padAccess = Sawyer::Nothing());

                    /** Size of each page of memory.
                     *
                     *  Memory is allocated in units of the page size and aligned on page-size boundaries.  The page size cannot be changed
                     *  once the map contains data.
                     *
                     * @{ */
                    rose_addr_t pageSize() const { return pageSize_; }

                    void pageSize(rose_addr_t nBytes);
                    /** @} */

                    /** Allocate a page of memory.
                     *
                     *  The specified address will be contained in the page, which is aligned on a page boundary. Do not call this if the page
                     *  is already allocated unless: it will replace the allocated page with a new one containing all zeros. */
                    void allocatePage(rose_addr_t va);

                };


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Complete semantic state
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

                typedef BaseSemantics::State State;
                typedef BaseSemantics::StatePtr StatePtr;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      RISC operators
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** Shared-ownership pointer to concrete RISC operations. See @ref heap_object_shared_ownership. */
                typedef boost::shared_ptr<class RiscOperators> RiscOperatorsPtr;

/** Defines RISC operators for the ConcreteSemantics domain.
 *
 *  These RISC operators depend on functionality introduced into the SValue class hierarchy at the ConcreteSemantics::SValue
 *  level. Therefore, the prototypical value supplied to the constructor or present in the supplied state object must have a
 *  dynamic type which is a ConcreteSemantics::SValue (or subclass).
 *
 *  Each RISC operator should return a newly allocated semantic value rather than trying to re-use an existing one. This will
 *  allow the caller to change the value stored there without affecting any of the input arguments. For example, a no-op that
 *  returns its argument should be implemented like this:
 *
 * @code
 *  BaseSemantics::SValuePtr noop(const BaseSemantics::SValuePtr &arg) {
 *      return arg->copy();     //correct
 *      return arg;             //incorrect
 *  }
 * @endcode
 */
                class RiscOperators : public BaseSemantics::RiscOperators {
                    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                    // Real constructors
                protected:
                    RiscOperators(const BaseSemantics::SValuePtr &protoval, SMTSolver *solver)
                            : BaseSemantics::RiscOperators(protoval, solver) {
                        name("Concrete");
                        (void) SValue::promote(protoval); // make sure its dynamic type is a ConcreteSemantics::SValue
                    }

                    RiscOperators(const BaseSemantics::StatePtr &state, SMTSolver *solver)
                            : BaseSemantics::RiscOperators(state, solver) {
                        name("Concrete");
                        (void) SValue::promote(
                                state->protoval());      // values must have ConcreteSemantics::SValue dynamic type
                    }

                    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                    // Static allocating constructors
                public:
                    /** Instantiates a new RiscOperators object and configures it to use semantic values and states that are defaults for
                     * ConcreteSemantics. */
                    static RiscOperatorsPtr instance(const RegisterDictionary *regdict, SMTSolver *solver = NULL) {
                        BaseSemantics::SValuePtr protoval = SValue::instance();
                        BaseSemantics::RegisterStatePtr registers = RegisterState::instance(protoval, regdict);
                        BaseSemantics::MemoryStatePtr memory = MemoryState::instance(protoval, protoval);
                        BaseSemantics::StatePtr state = State::instance(registers, memory);
                        return RiscOperatorsPtr(new RiscOperators(state, solver));
                    }

                    /** Instantiates a new RiscOperators object with specified prototypical values.  An SMT solver may be specified as the
                     *  second argument because the base class expects one, but it is not used for concrete semantics. See @ref solver for
                     *  details. */
                    static RiscOperatorsPtr instance(const BaseSemantics::SValuePtr &protoval,
                                                     SMTSolver *solver = NULL) {
                        return RiscOperatorsPtr(new RiscOperators(protoval, solver));
                    }

                    /** Instantiates a new RiscOperators object with specified state.  An SMT solver may be specified as the second argument
                     *  because the base class expects one, but it is not used for concrete semantics. See @ref solver for details. */
                    static RiscOperatorsPtr instance(const BaseSemantics::StatePtr &state, SMTSolver *solver = NULL) {
                        return RiscOperatorsPtr(new RiscOperators(state, solver));
                    }

                    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                    // Virtual constructors
                public:
                    virtual BaseSemantics::RiscOperatorsPtr create(const BaseSemantics::SValuePtr &protoval,
                                                                   SMTSolver *solver = NULL) const {
                        return instance(protoval, solver);
                    }

                    virtual BaseSemantics::RiscOperatorsPtr create(const BaseSemantics::StatePtr &state,
                                                                   SMTSolver *solver = NULL) const {
                        return instance(state, solver);
                    }

                    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                    // Dynamic pointer casts
                public:
                    /** Run-time promotion of a base RiscOperators pointer to concrete operators. This is a checked conversion--it
                     *  will fail if @p x does not point to a ConcreteSemantics::RiscOperators object. */
                    static RiscOperatorsPtr promote(const BaseSemantics::RiscOperatorsPtr &x) {
                        RiscOperatorsPtr retval = boost::dynamic_pointer_cast<RiscOperators>(x);
                        ASSERT_not_null(retval);
                        return retval;
                    }

                    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                    // New methods for constructing values, so we don't have to write so many SValue::promote calls in the RiscOperators
                    // implementations.
                protected:
                    SValuePtr svalue_number(size_t nbits, uint64_t value) {
                        return SValue::promote(number_(nbits, value));
                    }

                    SValuePtr svalue_number(const Sawyer::Container::BitVector &);

                    SValuePtr svalue_boolean(bool b) {
                        return SValue::promote(boolean_(b));
                    }

                    SValuePtr svalue_zero(size_t nbits) {
                        return SValue::promote(number_(nbits, 0));
                    }

                    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                    // Override methods from base class.  These are the RISC operators that are invoked by a Dispatcher.
                public:
                    virtual void interrupt(int majr, int minr);

                    virtual BaseSemantics::SValuePtr and_(const BaseSemantics::SValuePtr &a_,
                                                          const BaseSemantics::SValuePtr &b_);

                    virtual BaseSemantics::SValuePtr or_(const BaseSemantics::SValuePtr &a_,
                                                         const BaseSemantics::SValuePtr &b_);

                    virtual BaseSemantics::SValuePtr xor_(const BaseSemantics::SValuePtr &a_,
                                                          const BaseSemantics::SValuePtr &b_);

                    virtual BaseSemantics::SValuePtr invert(const BaseSemantics::SValuePtr &a_);

                    virtual BaseSemantics::SValuePtr extract(const BaseSemantics::SValuePtr &a_,
                                                             size_t begin_bit, size_t end_bit);

                    virtual BaseSemantics::SValuePtr concat(const BaseSemantics::SValuePtr &a_,
                                                            const BaseSemantics::SValuePtr &b_);

                    virtual BaseSemantics::SValuePtr leastSignificantSetBit(const BaseSemantics::SValuePtr &a_);

                    virtual BaseSemantics::SValuePtr mostSignificantSetBit(const BaseSemantics::SValuePtr &a_);

                    virtual BaseSemantics::SValuePtr rotateLeft(const BaseSemantics::SValuePtr &a_,
                                                                const BaseSemantics::SValuePtr &sa_);

                    virtual BaseSemantics::SValuePtr rotateRight(const BaseSemantics::SValuePtr &a_,
                                                                 const BaseSemantics::SValuePtr &sa_);

                    virtual BaseSemantics::SValuePtr shiftLeft(const BaseSemantics::SValuePtr &a_,
                                                               const BaseSemantics::SValuePtr &sa_);

                    virtual BaseSemantics::SValuePtr shiftRight(const BaseSemantics::SValuePtr &a_,
                                                                const BaseSemantics::SValuePtr &sa_);

                    virtual BaseSemantics::SValuePtr shiftRightArithmetic(const BaseSemantics::SValuePtr &a_,
                                                                          const BaseSemantics::SValuePtr &sa_);

                    virtual BaseSemantics::SValuePtr equalToZero(const BaseSemantics::SValuePtr &a_);

                    virtual BaseSemantics::SValuePtr ite(const BaseSemantics::SValuePtr &sel_,
                                                         const BaseSemantics::SValuePtr &a_,
                                                         const BaseSemantics::SValuePtr &b_);

                    virtual BaseSemantics::SValuePtr unsignedExtend(const BaseSemantics::SValuePtr &a_,
                                                                    size_t new_width);

                    virtual BaseSemantics::SValuePtr signExtend(const BaseSemantics::SValuePtr &a_, size_t new_width);

                    virtual BaseSemantics::SValuePtr add(const BaseSemantics::SValuePtr &a_,
                                                         const BaseSemantics::SValuePtr &b_);

                    virtual BaseSemantics::SValuePtr addWithCarries(const BaseSemantics::SValuePtr &a_,
                                                                    const BaseSemantics::SValuePtr &b_,
                                                                    const BaseSemantics::SValuePtr &c_,
                                                                    BaseSemantics::SValuePtr &carry_out/*out*/);

                    virtual BaseSemantics::SValuePtr negate(const BaseSemantics::SValuePtr &a_);

                    virtual BaseSemantics::SValuePtr signedDivide(const BaseSemantics::SValuePtr &a_,
                                                                  const BaseSemantics::SValuePtr &b_);

                    virtual BaseSemantics::SValuePtr signedModulo(const BaseSemantics::SValuePtr &a_,
                                                                  const BaseSemantics::SValuePtr &b_);

                    virtual BaseSemantics::SValuePtr signedMultiply(const BaseSemantics::SValuePtr &a_,
                                                                    const BaseSemantics::SValuePtr &b_);

                    virtual BaseSemantics::SValuePtr unsignedDivide(const BaseSemantics::SValuePtr &a_,
                                                                    const BaseSemantics::SValuePtr &b_);

                    virtual BaseSemantics::SValuePtr unsignedModulo(const BaseSemantics::SValuePtr &a_,
                                                                    const BaseSemantics::SValuePtr &b_);

                    virtual BaseSemantics::SValuePtr unsignedMultiply(const BaseSemantics::SValuePtr &a_,
                                                                      const BaseSemantics::SValuePtr &b_);

                    virtual BaseSemantics::SValuePtr fpFromInteger(const BaseSemantics::SValuePtr &intValue,
                                                                   SgAsmFloatType *);

                    virtual BaseSemantics::SValuePtr fpToInteger(const BaseSemantics::SValuePtr &fpValue,
                                                                 SgAsmFloatType *fpType,
                                                                 const BaseSemantics::SValuePtr &dflt);

                    virtual BaseSemantics::SValuePtr fpAdd(const BaseSemantics::SValuePtr &a,
                                                           const BaseSemantics::SValuePtr &b,
                                                           SgAsmFloatType *);

                    virtual BaseSemantics::SValuePtr fpSubtract(const BaseSemantics::SValuePtr &a,
                                                                const BaseSemantics::SValuePtr &b,
                                                                SgAsmFloatType *);

                    virtual BaseSemantics::SValuePtr fpMultiply(const BaseSemantics::SValuePtr &a,
                                                                const BaseSemantics::SValuePtr &b,
                                                                SgAsmFloatType *);

                    virtual BaseSemantics::SValuePtr fpRoundTowardZero(const BaseSemantics::SValuePtr &a,
                                                                       SgAsmFloatType *);

                    virtual BaseSemantics::SValuePtr readMemory(const RegisterDescriptor &segreg,
                                                                const BaseSemantics::SValuePtr &addr,
                                                                const BaseSemantics::SValuePtr &dflt,
                                                                const BaseSemantics::SValuePtr &cond);

                    virtual void writeMemory(const RegisterDescriptor &segreg,
                                             const BaseSemantics::SValuePtr &addr,
                                             const BaseSemantics::SValuePtr &data,
                                             const BaseSemantics::SValuePtr &cond);

                protected:
                    // Convert expression to double
                    double exprToDouble(const BaseSemantics::SValuePtr &expr, SgAsmFloatType *);

                    // Convert double to expression
                    BaseSemantics::SValuePtr doubleToExpr(double d, SgAsmFloatType *);
                };

            } // namespace
        } // namespace
    } // namespace
} // namespace

#endif
