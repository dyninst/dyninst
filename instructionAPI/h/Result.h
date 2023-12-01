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

#if !defined(RESULT_H)
#define RESULT_H

#include <string.h> // memcmp
#include <cstdint>
#include <cassert>
#include <string>
#include "util.h"


namespace Dyninst
{
    namespace InstructionAPI
    {
        union Result_Value
        {
            unsigned char bitval : 1;
            unsigned char u8val;
	    /* char can be signed or unsigned, must be signed for s8val */
            signed char s8val;
            uint16_t u16val;
            int16_t s16val;
            uint32_t u24val:24;
            uint32_t u32val;
            int32_t s32val;
            uint64_t u48val : 48;
            int64_t s48val : 48;
            uint64_t u64val;
            int64_t s64val;
            float floatval;
            double dblval;
            void * dbl128val;
            void * m14val;
            void * m32val;
            void * m64val;
            void * m80val;
            void * m96val;
            void * m128val;
            void * m160val;
            void * m192val;
            void * m224val;
            void * m256val;
            void * m288val;
            void * m320val;
            void * m352val;
            void * m384val;
            void * m416val;
            void * m448val;
            void * m480val;
            void * m512val;
        };

        // The order of these enumerations is important.
        // See 'operator==' and arithmetic operators.
        enum Result_Type
        {
            bit_flag = 0,
            s8,
            u8,
            s16,
            u16,
            u24,
            s32,
            u32,
            s48,
            u48,
            s64,
            u64,
            sp_float,
            dp_float,
            m14,  // For historical reason m14 means 14 bytes. All other mX means X bits
            dbl128,
            m32,
            m64,
            m80,
            m96,
            m128,
            m160,
            m192,
            m224,
            m256,
            m288,
            m320,
            m352,
            m384,
            m416,
            m448,
            m480,
            m512,
            invalid_type
        };

        template < Result_Type t > struct Result_type2type
        {
            typedef void* type;
        };
        template < > struct Result_type2type<s8>
        {
            typedef signed char type;
        };
        template < > struct Result_type2type<u8>
        {
            typedef unsigned char type;
        };
        template < > struct Result_type2type <s16>
        {
            typedef int16_t type;
        };
        template < > struct Result_type2type <u16>
        {
            typedef uint16_t type;
        };
        template < > struct Result_type2type <u24>
        {
            typedef uint32_t type;
        };
        template <  > struct Result_type2type <s32>
        {
            typedef int32_t type;
        };
        template < > struct Result_type2type <u32>
        {
            typedef uint32_t type;
        };
        template <  > struct Result_type2type <s48>
        {
            typedef int64_t type;
        };
        template < > struct Result_type2type <u48>
        {
            typedef uint64_t type;
        };
        template <  > struct Result_type2type <s64>
        {
            typedef int64_t type;
        };
        template < > struct Result_type2type <u64>
        {
            typedef uint64_t type;
        };
        template < > struct Result_type2type <sp_float>
        {
            typedef float type;
        };
        template < > struct Result_type2type <dp_float>
        {
            typedef double type;
        };
        template < > struct Result_type2type <bit_flag>
        {
            typedef unsigned char type;
        };

        /// A %Result object represents a value computed by a %Expression AST.
        ///
        /// The %Result class is a tagged-union representation of the results that
        /// %Expressions can produce.  It includes 8, 16, 32, 48, and 64 bit integers
        /// (signed and unsigned), bit values, and single and double precision floating point values.
        /// For each of these types, the value of a %Result may be undefined, or it may be a value within
        /// the range of the type.
        ///
        /// The \c type field is an enum that may contain any of the following values:
        /// - \c bit_flag: a single bit (individual flags)
        /// - \c u8: an unsigned 8-bit integer
        /// - \c s8: a signed 8-bit integer
        /// - \c u16: an unsigned 16-bit integer
        /// - \c s16: a signed 16-bit integer
        /// - \c u24: an unsigned 24-bit integer
        /// - \c u32: an unsigned 32-bit integer
        /// - \c s32: a signed 32-bit integer
        /// - \c u48: an unsigned 48-bit integer
        /// - \c s48: a signed 48-bit integer
        /// - \c u64: an unsigned 64-bit integer
        /// - \c s64: a signed 64-bit integer
        /// - \c sp_float: a single-precision float
        /// - \c dp_float: a double-precision float
        /// - \c dbl128: a 128-bit integer, which often contains packed floating point values
        /// - \c m14: a 14 byte memory value
        /// - \c m32: a 32-bit memory value
        /// - \c m64: a 64-bit memory value
        /// - \c m80: an 80-bit memory value
        /// - \c m96: a 96-bit memory value
        /// - \c m128: a 128-bit memory value
        /// - \c m160: a 160-bit memory value
        /// - \c m192: a 192-bit memory value
        /// - \c m224: a 224-bit memory value
        /// - \c m256: a 256-bit memory value
        /// - \c m288: a 288-bit memory value
        /// - \c m320: a 320-bit memory value
        /// - \c m352: a 352-bit memory value
        /// - \c m384: a 384-bit memory value
        /// - \c m416: a 416-bit memory value
        /// - \c m448: a 448-bit memory value
        /// - \c m480: a 480-bit memory value
        /// - \c m512: a 512-bit memory value

        // The %Instruction API's model of %Results is a simple one, and may seem overly aggressive about
        // making an %Expression's %Result undefined.  It follows the same basic rule as the rest of the API:
        // a decoded %Instruction object represents only the information that may be obtained from the machine
        // instruction that was decoded.  As discussed in the Expression section, the \c setValue
        // and \c eval interface allows you to determine the possible %Results of an %Expression when evaluated over various
        // machine states.  From this, you may construct abstractions to represent the set of possible results.
        // Alternately, you may use instrumentation to determine the exact machine state at the time an
        // instruction executes, which will allow you to evaluate the %Result of an %Expression in its actual context.
        class INSTRUCTION_EXPORT Result
        {
            public:
                Result_Value val;
                Result_Type type;
                bool defined;

                Result() :
                    type(u32), defined(false)
            {
                val.u32val = 0;
            }
                Result(const Result& o) :
                    val(o.val), type(o.type), defined(o.defined)
            {
            }

                const Result& operator=(const Result& rhs)
                {
                    val = rhs.val;
                    type = rhs.type;
                    defined = rhs.defined;
                    return *this;
                }

                /// A %Result may be constructed from a type without providing a value.
                /// This constructor creates a %Result of type \c t with undefined contents.
                Result(Result_Type t) :
                    type(t), defined(false)
            {
                val.u32val = 0;
            }

                /// A %Result may be constructed from a type and any value convertible to the type that the
                /// tag represents.
                /// This constructor creates a %Result of type \c t and contents \c v for any \c v that is implicitly
                /// convertible to type \c t.  Attempting to construct a %Result with a value that is incompatible with
                /// its type will result in a compile-time error.
                template<typename T>
                    Result(Result_Type t, T v) :
                        type(t), defined(true)
            {
                switch(type)
                {
                    case bit_flag:
                        val.bitval = (v != 0) ? 1 : 0;
                        break;
                    case u8:
                        val.u8val = (unsigned char)(v);
                        break;
                    case s8:
                        val.s8val = (signed char)(v);
                        break;
                    case u16:
                        val.u16val = (uint16_t)(v);
                        break;
                    case s16:
                        val.s16val = (int16_t)(v);
                        break;
                    case u24:
                        val.u24val = (uint32_t)(v & 0xFFFFFF);
                        break;
                    case u32:
                        val.u32val = (uint32_t)(v );
                        break;
                    case s32:
                        val.s32val = (int32_t)(v);
                        break;
                    case u48:
                        val.u48val = (uint64_t)(v & 0x0000FFFFFFFFFFFFLL);
                        break;
                    case s48:
                        val.s48val = (int64_t)(v & 0x0000FFFFFFFFFFFFLL);
                        break;
                    case u64:
                        val.u64val = (uint64_t)(v);
                        break;
                    case s64:
                        val.s64val = (int64_t)(v);
                        break;
                    case dbl128:
                        val.dbl128val = (void*)(intptr_t) v;
                        break;
                    case m14:
                        val.m14val = (void*)(intptr_t) v;
                        break;
                    case m32:
                        val.m32val = (void *)(intptr_t) v;
                        break;
                    case m64:
                        val.m64val = (void *)(intptr_t) v;
                        break;
                    case m80:
                        val.m80val = (void *)(intptr_t) v;
                        break;
                    case m96:
                        val.m96val = (void *)(intptr_t) v;
                        break;
                    case m128:
                        val.m128val = (void *)(intptr_t) v;
                        break;
                    case m160:
                        val.m160val = (void *)(intptr_t) v;
                        break;
                    case m192:
                        val.m192val = (void *)(intptr_t) v;
                        break;
                    case m224:
                        val.m224val = (void *)(intptr_t) v;
                        break;
                    case m256:
                        val.m256val = (void *)(intptr_t) v;
                        break;
                    case m288:
                        val.m288val = (void *)(intptr_t) v;
                        break;
                    case m320:
                        val.m320val = (void *)(intptr_t) v;
                        break;
                    case m352:
                        val.m352val = (void *)(intptr_t) v;
                        break;
                    case m384:
                        val.m384val = (void *)(intptr_t) v;
                        break;
                    case m416:
                        val.m416val = (void *)(intptr_t) v;
                        break;
                    case m448:
                        val.m448val = (void *)(intptr_t) v;
                        break;
                    case m480:
                        val.m480val = (void *)(intptr_t) v;
                        break;
                    case m512:
                        val.m512val = (void *)(intptr_t) v;
                        break;
                    default:
                    // Floats should be constructed with float types
                    case sp_float:
                    case dp_float:
                        assert(!"Invalid type!");
                        break;
                }
            }
                Result(Result_Type t, float v) :
                    type(t), defined(true)
            {
                assert(t == sp_float || t == dp_float);
                val.dblval = (double)v;
            }
                Result(Result_Type t, double v) :
                    type(t), defined(true)
            {
                assert(t == sp_float || t == dp_float);
                val.dblval = v;
            }
                ~Result()
                {
                }

                bool operator<(const Result& o) const
                {
                    if(type < o.type) return true;
                    if(!defined) return false;
                    if(!o.defined) return true;

                    switch(type)
                    {
                        case bit_flag:
                            return val.bitval < o.val.bitval;
                            break;
                        case u8:
                            return val.u8val < o.val.u8val;
                            break;
                        case s8:
                            return val.s8val < o.val.s8val;
                            break;
                        case u16:
                            return val.u16val < o.val.u16val;
                            break;
                        case s16:
                            return val.s16val < o.val.s16val;
                            break;
                        case u24:
                            return val.u24val < o.val.u24val;
                            break;
                        case u32:
                            return val.u32val < o.val.u32val;
                            break;
                        case s32:
                            return val.s32val < o.val.s32val;
                            break;
                        case u48:
                            return val.u48val < o.val.u48val;
                            break;
                        case s48:
                            return val.s48val < o.val.s48val;
                            break;
                        case u64:
                            return val.u64val < o.val.u64val;
                            break;
                        case s64:
                            return val.s64val < o.val.s64val;
                            break;
                        case sp_float:
                            return val.floatval < o.val.floatval;
                            break;
                        case dp_float:
                            return val.dblval < o.val.dblval;
                            break;
                        case dbl128:
                            return memcmp(val.dbl128val, o.val.dbl128val, 16) < 0;
                            break;
                        case m14:
                            return memcmp(val.m14val, o.val.m14val, 14) < 0;
                            break;
                        case m32:
                            return memcmp(val.m32val, o.val.m32val, 4) < 0;
                            break;
                        case m64:
                            return memcmp(val.m64val, o.val.m64val, 8) < 0;
                            break;
                        case m80:
                            return memcmp(val.m80val, o.val.m80val, 10) < 0;
                            break;
                        case m96:
                            return memcmp(val.m96val, o.val.m96val, 12) < 0;
                            break;
                        case m128:
                            return memcmp(val.m128val, o.val.m128val, 16) < 0;
                            break;
                        case m160:
                            return memcmp(val.m160val, o.val.m160val, 20) < 0;
                            break;
                        case m192:
                            return memcmp(val.m192val, o.val.m192val, 24) < 0;
                            break;
                        case m224:
                            return memcmp(val.m224val, o.val.m224val, 28) < 0;
                            break;
                        case m256:
                            return memcmp(val.m256val, o.val.m256val, 32) < 0;
                            break;
                        case m288:
                            return memcmp(val.m288val, o.val.m288val, 36) < 0;
                            break;
                        case m320:
                            return memcmp(val.m320val, o.val.m320val, 40) < 0;
                            break;
                        case m352:
                            return memcmp(val.m352val, o.val.m352val, 44) < 0;
                            break;
                        case m384:
                            return memcmp(val.m384val, o.val.m384val, 48) < 0;
                            break;
                        case m416:
                            return memcmp(val.m416val, o.val.m416val, 52) < 0;
                            break;
                        case m448:
                            return memcmp(val.m448val, o.val.m448val, 56) < 0;
                            break;
                        case m480:
                            return memcmp(val.m480val, o.val.m480val, 60) < 0;
                            break;
                        case m512:
                            return memcmp(val.m512val, o.val.m512val, 64) < 0;
                            break;
                        default:
                            assert(!"Invalid type!");
                            break;
                    }
                    return false;
                }

                /// Two %Results are equal if any of the following hold:
                /// - Both %Results are of the same type and undefined
                /// - Both %Results are of the same type, defined, and have the same value
                ///
                /// Otherwise, they are unequal (due to having different types, an undefined %Result compared to a defined %Result,
                /// or different values).
                bool operator==(const Result& o) const
                {
                    return !((*this < o) || (o < *this));
                }

                /// %Results are formatted as strings containing their contents, represented as hexadecimal.
                /// The type of the %Result is not included in the output.
                std::string format() const
                {
                    if(!defined)
                    {
                        return "[empty]";
                    }
                    else
                    {
                        char hex[20]; 
                        switch(type)
                        {
                            case bit_flag:
                                snprintf(hex, 20, "%x", (unsigned int)val.bitval);
                                break;
                            case u8:
                                snprintf(hex, 20, "%x", val.u8val);
                                break;
                            case s8:
                                snprintf(hex, 20, "%x", (unsigned int)val.s8val);
                                break;
                            case u16:
                                snprintf(hex, 20, "%x", val.u16val);
                                break;
                            case s16:
                                snprintf(hex, 20, "%x", (unsigned int)val.s16val);
                                break;
                            case u24:
                                snprintf(hex, 20, "%x", (unsigned int)val.u24val);
                                break;
                            case u32:
                                snprintf(hex, 20, "%x", val.u32val);
                                break;
                            case s32:
                                snprintf(hex, 20, "%x", (unsigned int)val.s32val);
                                break;
                            case u48:
                                snprintf(hex, 20, "%lx", val.u48val);
                                break;
                            case s48:
                                snprintf(hex, 20, "%lx", (uint64_t ) val.s48val);
                                break;
                            case u64:
                                snprintf(hex, 20, "%lx", val.u64val);
                                break;
                            case s64:
                                snprintf(hex, 20, "%lx", (uint64_t) val.s64val);
                                break;
                            case sp_float:
                                snprintf(hex, 20, "%f", (double)val.floatval);
                                break;
                            case dp_float:
                                snprintf(hex, 20, "%lf", val.dblval);
                                break;
                            case dbl128:
                                snprintf(hex, 20, "%p", val.dbl128val);
                                break;
                            case m14:
                                snprintf(hex, 20, "%p", val.m14val);
                                break;
                            case m32:
                                snprintf(hex, 20, "%p", val.m32val);
                                break;
                            case m64:
                                snprintf(hex, 20, "%p", val.m64val);
                                break;
                            case m80:
                                snprintf(hex, 20, "%p", val.m80val);
                                break;
                            case m96:
                                snprintf(hex, 20, "%p", val.m96val);
                                break;
                            case m128:
                                snprintf(hex, 20, "%p", val.m128val);
                                break;
                            case m160:
                                snprintf(hex, 20, "%p", val.m160val);
                                break;
                            case m192:
                                snprintf(hex, 20, "%p", val.m192val);
                                break;
                            case m224:
                                snprintf(hex, 20, "%p", val.m224val);
                                break;
                            case m256:
                                snprintf(hex, 20, "%p", val.m256val);
                                break;
                            case m288:
                                snprintf(hex, 20, "%p", val.m288val);
                                break;
                            case m320:
                                snprintf(hex, 20, "%p", val.m320val);
                                break;
                            case m352:
                                snprintf(hex, 20, "%p", val.m352val);
                                break;
                            case m384:
                                snprintf(hex, 20, "%p", val.m384val);
                                break;
                            case m416:
                                snprintf(hex, 20, "%p", val.m416val);
                                break;
                            case m448:
                                snprintf(hex, 20, "%p", val.m448val);
                                break;
                            case m480:
                                snprintf(hex, 20, "%p", val.m480val);
                                break;
                            case m512:
                                snprintf(hex, 20, "%p", val.m512val);
                                break;
                            default:
                                snprintf(hex, 20, "[invalid type]");
                                break;
                        };
                        return hex;
                    }
                }



                template< typename to_type >
                    to_type convert() const
                    {
                        switch(type)
                        {
                            case s8:
                                return to_type(val.s8val);
                            case u8:
                                return to_type(val.u8val);
                            case s16:
                                return to_type(val.s16val);
                            case u16:
                                return to_type(val.u16val);
                            case u24:
                                return to_type(val.u24val);
                            case s32:
                                return to_type(val.s32val);
                            case u32:
                                return to_type(val.u32val);
                            case s48:
                                return to_type(val.s48val);
                            case u48:
                                return to_type(val.u48val);
                            case s64:
                                return to_type(val.s64val);
                            case u64:
                                return to_type(val.u64val);
                            case sp_float:
                                return to_type(val.floatval);
                            case dp_float:
                                return to_type(val.dblval);
                            case bit_flag:
                                return to_type(val.bitval);
                            case dbl128:
                            case m14:
                            case m32:
                            case m64:
                            case m80:
                            case m96:
                            case m128:
                            case m160:
                            case m192:
                            case m224:
                            case m256:
                            case m288:
                            case m320:
                            case m352:
                            case m384:
                            case m416:
                            case m448:
                            case m480:
                            case m512:
                                assert(!"Memory types cannot be converted yet");
                                return to_type(0);
                            default:
                                assert(!"Invalid type in result!");
                                return to_type(0);
                        }
                    }


                /// Returns the size of the contained type, in bytes
                int size() const
                {
                    switch(type)
                    {
                        case bit_flag:
                            return 1;
                        case u8:
                        case s8:
                            return 1;
                        case u16:
                        case s16:
                            return 2;
                        case u24:
                            return 3;
                        case u32:
                        case s32:
                            return 4;
                        case u48:
                        case s48:
                            return 6;
                        case u64:
                        case s64:
                            return 8;
                        case sp_float:
                            return sizeof(float);
                        case dp_float:
                            return sizeof(double);
                        case dbl128:
                            return 16;
                        case m14:
                            return 14;
                        case m32:
                            return 4;
                        case m64:
                            return 8;
                        case m80:
                            return 10;
                        case m96:
                            return 12;
                        case m128:
                            return 16;
                        case m160:
                            return 20;
                        case m192:
                            return 24;
                        case m224:
                            return 28;
                        case m256:
                            return 32;
                        case m288:
                            return 36;
                        case m320:
                            return 40;
                        case m352:
                            return 44;
                        case m384:
                            return 48;
                        case m416:
                            return 52;
                        case m448:
                            return 56;
                        case m480:
                            return 60;
                        case m512:
                            return 64;
                        default:
                            // In probabilistic gap parsing,
                            // we could start to decode at any byte and reach here.
                            // It is a sign for junk bytes
                            return 0;
                    };
                }
        };

        INSTRUCTION_EXPORT Result operator+(const Result& arg1, const Result& arg2);
        INSTRUCTION_EXPORT Result operator*(const Result& arg1, const Result& arg2);
        INSTRUCTION_EXPORT Result operator<<(const Result& arg1, const Result& arg2);
        INSTRUCTION_EXPORT Result operator>>(const Result& arg1, const Result& arg2);
        INSTRUCTION_EXPORT Result operator&(const Result& arg1, const Result& arg2);
        INSTRUCTION_EXPORT Result operator|(const Result& arg1, const Result& arg2);

    }
}


#endif // !defined(RESULT_H)

