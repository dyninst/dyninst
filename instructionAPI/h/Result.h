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

#include <sstream>
#include <string.h> // memcmp
#if !defined(_MSC_VER)
#include <inttypes.h>
#else
   typedef __int64 int64_t;
   typedef __int32 int32_t;
   typedef __int16 int16_t;
   typedef unsigned __int64 uint64_t;
   typedef unsigned __int32 uint32_t;
   typedef unsigned __int16 uint16_t;
#endif
#include <assert.h>
#include <string>



namespace Dyninst
{
  namespace InstructionAPI
  {
    union Result_Value
    {
      unsigned char bitval : 1;
      unsigned char u8val;
      char s8val;
      uint16_t u16val;
      int16_t s16val;
      uint32_t u24val:24;
      uint32_t u32val;
      int32_t s32val;
      uint64_t u64val;
      int64_t s64val;
      float floatval;
      double dblval;
      uint64_t u48val : 48;
      int64_t s48val : 48;
      void * m14val;
      void * m96val;
      void * dbl128val;
      void * m192val;
      void * m256val;
      void * m384val;
      void * m512val;
    };
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
      // 48-bit pointers...yay Intel
      m14,
      m96,
      dbl128,
      m192,
      m256,
      m384,
      m512,
      invalid_type
    };

    template < Result_Type t > struct Result_type2type
    {
      typedef void* type;
    };
    template < > struct Result_type2type<s8>
    {
      typedef char type;
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
    /// - \c u8: an unsigned 8-bit integer
    /// - \c s8: a signed 8-bit integer
    /// - \c u16: an unsigned 16-bit integer
    /// - \c s16: a signed 16-bit integer
    /// - \c u32: an unsigned 32-bit integer
    /// - \c s32: a signed 32-bit integer
    /// - \c u48: an unsigned 48-bit integer (IA32 pointers)
    /// - \c s48: a signed 48-bit integer (IA32 pointers)
    /// - \c u64: an unsigned 64-bit integer
    /// - \c s64: a signed 64-bit integer
    /// - \c sp_float: a single-precision float
    /// - \c dp_float: a double-precision float
    /// - \c bit_flag: a single bit (individual flags)
    /// - \c m512: a 512-bit memory value
    /// - \c dbl128: a 128-bit integer, which often contains packed floating point values
   /// - \c m14: a 14 byte memory value
    ///
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
	case u8:
	  val.u8val = (unsigned char)(v);
	  break;
	case s8:
	  val.s8val = (char)(v);
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
	case u64:
	  val.u64val = (uint64_t)(v);
	  break;
	case s64:
	  val.s64val = (int64_t)(v);
	  break;
	case bit_flag:
	  val.bitval = (v != 0) ? 1 : 0;
	  break;
	case u48:
	  val.u48val = (uint64_t)(v & 0x0000FFFFFFFFFFFFLL);
	  break;
	case s48:
	  val.s48val = (int64_t)(v & 0x0000FFFFFFFFFFFFLL);
	  break;
	case m512:
          val.m512val = (void *)(intptr_t) v;
	  break;
    case dbl128:
	  val.dbl128val = (void*)(intptr_t) v;
	  break;
	case m14:
	  val.m14val = (void*)(intptr_t) v;
	  break;
	case m96:
          val.m96val = (void *)(intptr_t) v;
	  break;
	case m192:
          val.m192val = (void *)(intptr_t) v;
	  break;
	case m256:
          val.m256val = (void *)(intptr_t) v;
	  break;
	case m384:
          val.m384val = (void *)(intptr_t) v;
	  break;
	  // Floats should be constructed with float types
	default:
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
	case bit_flag:
	  return val.bitval < o.val.bitval;
	  break;
	case u48:
	  return val.u48val < o.val.u48val;
	  break;
	case s48:
	  return val.s48val < o.val.s48val;
	  break;
	case m512:
	  return memcmp(val.m512val, o.val.m512val, 512) < 0;
	  break;
	case dbl128:
	  return memcmp(val.dbl128val, o.val.dbl128val, 128 / 8) < 0;
	  break;
	case m14:
	  return memcmp(val.m14val, o.val.m14val, 14) < 0;
	  break;
	case m96:
	  return memcmp(val.m96val, o.val.m96val, 96) < 0;
	  break;
	case m192:
	  return memcmp(val.m192val, o.val.m192val, 192) < 0;
	  break;
	case m256:
	  return memcmp(val.m256val, o.val.m256val, 256) < 0;
	  break;
	case m384:
	  return memcmp(val.m384val, o.val.m384val, 384) < 0;
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
	  case u64:
	    return std::to_string(val.u64val);
	  case s64:
	    return std::to_string(val.s64val);
	    break;
	  case sp_float:
	    snprintf(hex, 20, "%f", (double)val.floatval);
	    break;
	  case dp_float:
	    snprintf(hex, 20, "%lf", val.dblval);
	    break;
	  case bit_flag:
	    snprintf(hex, 20, "%x", (unsigned int)val.bitval);
	    break;
	  case u48:
	    return std::to_string(val.s48val);
	    break;
	  case s48:
	    return std::to_string(val.s48val);
	    break;
     case m512:
	    snprintf(hex, 20, "%p", val.m512val);
        break;
     case m14:
	    snprintf(hex, 20, "%p", val.m14val);
        break;
     case m96:
	    snprintf(hex, 20, "%p", val.m96val);
        break;
     case dbl128:
	    snprintf(hex, 20, "%p", val.dbl128val);
         break;
     case m192:
	    snprintf(hex, 20, "%p", val.m192val);
        break;
     case m256:
	    snprintf(hex, 20, "%p", val.m256val);
        break;
     case m384:
	    snprintf(hex, 20, "%p", val.m384val);
        break;
	  default:
	    snprintf(hex, 20, "[invalid type]");
	    break;
	  };
	  return std::string(hex);
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
	case m512:
	case dbl128:
    case m192:
    case m256:
    case m384:
    case m96:
	  assert(!"M512 and DBL128 types cannot be converted yet");
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
	case u64:
	case s64:
	  return 8;
	case u48:
	case s48:
	  return 6;
	case sp_float:
	  return sizeof(float);
	case dp_float:
             return sizeof(double);
	case bit_flag:
	  return 1;
   case m512:
      return 64;
   case dbl128:
      return 16;
   case m192:
      return 24;
   case m256:
      return 32;
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

