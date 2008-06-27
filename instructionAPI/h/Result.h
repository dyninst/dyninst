
#if !defined(RESULT_H)
#define RESULT_H

#include <sstream>
#include "common/h/Types.h"

namespace Dyninst
{
  namespace Instruction
  {
    union Result_Value
    {
      unsigned char bitval : 1;
      unsigned char u8val;
      char s8val;
      uint16_t u16val;
      int16_t s16val;
      uint32_t u32val;
      int32_t s32val;
      uint64_t u64val;
      int64_t s64val;
      float floatval;
      double dblval;
      uint64_t u48val : 48;
      int64_t s48val : 48;
    };
    enum Result_Type
    {
      u8 = 0,
      s8,
      u16,
      s16,
      u32,
      s32,
      u64,
      s64,
      sp_float,
      dp_float,
      bit_flag,
      // 48-bit pointers...yay Intel
      s48,
      u48
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
    ///
    // The %Instruction API's model of %Results is a simple one, and may seem overly aggressive about
    // making an %Expression's %Result undefined.  It follows the same basic rule as the rest of the API:
    // a decoded %Instruction object represents only the information that may be obtained from the machine
    // instruction that was decoded.  As discussed in the Expression section, the \c setValue
    // and \c eval interface allows you to determine the possible %Results of an %Expression when evaluated over various
    // machine states.  From this, you may construct abstractions to represent the set of possible results.
    // Alternately, you may use instrumentation to determine the exact machine state at the time an
    // instruction executes, which will allow you to evaluate the %Result of an %Expression in its actual context.
    class Result
    {
    public:
      Result_Value val;
      Result_Type type;
      bool defined;
      
      /// A %Result may be constructed from a type without providing a value.
      /// This constructor creates a %Result of type \c t with undefined contents.
      Result(Result_Type t)
      {
	type = t;
	defined = false;
	val.dblval = 0.0;
	
      }
      /// A %Result may be constructed from a type and any value convertible to the type that the
      /// tag represents.
      /// This constructor creates a %Result of type \c t and contents \c v for any \c v that is implicitly
      /// convertible to type \c t.  Attempting to construct a %Result with a value that is incompatible with
      /// its type will result in a compile-time error.
      template<typename T>
      Result(Result_Type t, T v)
      {
	type = t,
	defined = true;
	val.dblval = 0.0;
	switch(type)
	{
	case u8:
	  val.u8val = v;
	  break;
	case s8:
	  val.s8val = v;
	  break;
	case u16:
	  val.u16val = v;
	  break;
	case s16:
	  val.s16val = v;
	  break;
	case u32:
	  val.u32val = v;
	  break;
	case s32:
	  val.s32val = v;
	  break;
	case u64:
	  val.u64val = v;
	  break;
	case s64:
	  val.s64val = v;
	  break;
	case sp_float:
	  val.floatval = v;
	  break;
	case dp_float:
	  val.dblval = v;
	  break;
	case bit_flag:
	  val.bitval = v;
	  break;
	case u48:
	  val.u48val = v;
	  break;
	case s48:
	  val.s48val = v;
	  break;
	default:
	  assert(!"Invalid type!");
	  break;
	}
      }
      ~Result()
      {
      }
      /// Two %Results are equal if any of the following hold:
      /// - Both %Results are of the same type and undefined
      /// - Both %Results are of the same type, defined, and have the same value
      ///
      /// Otherwise, they are unequal (due to having different types, an undefined %Result compared to a defined %Result,
      /// or different values).
      bool operator==(const Result& o) const
      {

	if(type != o.type) return false;
	if(defined != o.defined) return false;
	if(!defined) return true;
	switch(type)
	{
	case u8:
	  return val.u8val == o.val.u8val;
	  break;
	case s8:
	  return val.s8val == o.val.s8val;
	  break;
	case u16:
	  return val.u16val == o.val.u16val;
	  break;
	case s16:
	  return val.s16val == o.val.s16val;
	  break;
	case u32:
	  return val.u32val == o.val.u32val;
	  break;
	case s32:
	  return val.s32val == o.val.s32val;
	  break;
	case u64:
	  return val.u64val == o.val.u64val;
	  break;
	case s64:
	  return val.s64val == o.val.s64val;
	  break;
	case sp_float:
	  return val.floatval == o.val.floatval;
	  break;
	case dp_float:
	  return val.dblval == o.val.dblval;
	  break;
	case bit_flag:
	  return val.bitval == o.val.bitval;
	  break;
	case u48:
	  return val.u48val == o.val.u48val;
	  break;
	case s48:
	  return val.s48val == o.val.s48val;
	  break;
	default:
	  assert(!"Invalid type!");
	  break;
	}
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
	  std::stringstream ret;
	  ret << std::hex << "0x";
	  switch(type)
	  {
	  case u8:
	    // Type promote the characters so that they're treated as integral, not as strings
	    ret << (unsigned long)(val.u8val);
	    break;
	  case s8:
	    ret << (long)(val.s8val);
	    break;
	  case u16:
	    ret << val.u16val;
	    break;
	  case s16:
	    ret << val.s16val;
	    break;
	  case u32:
	    ret << val.u32val;
	    break;
	  case s32:
	    ret << val.s32val;
	    break;
	  case u64:
	    ret << val.u64val;
	    break;
	  case s64:
	    ret << val.s64val;
	    break;
	  case sp_float:
	    ret << val.floatval;
	    break;
	  case dp_float:
	    ret << val.dblval;
	    break;
	  case bit_flag:
	    ret << val.bitval;
	    break;
	  case u48:
	    ret << val.u48val;
	    break;
	  case s48:
	    ret << val.s48val;
	    break;
	  default:
	    ret << "[ERROR: invalid type value!]";
	    break;
	  };
	  return ret.str();
	}
      }
      
    };
    Result operator+(const Result& arg1, const Result& arg2);
    
  };
};


#endif // !defined(RESULT_H)
