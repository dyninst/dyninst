#if !defined(ROSE_TYPEDEFS_H)
#define ROSE_TYPEDEFS_H

#if !defined(_MSC_VER)
#include <stdint.h>
#endif

#include <vector>

typedef std::vector<unsigned char> SgUnsignedCharList;
typedef SgUnsignedCharList* SgUnsignedCharListPtr;

class SgAsmExpression;
typedef std::vector<SgAsmExpression*> SgAsmExpressionPtrList;

typedef uint64_t rose_addr_t;
#endif
