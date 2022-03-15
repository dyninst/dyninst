#if !defined(ROSE_TYPEDEFS_H)
#define ROSE_TYPEDEFS_H

#include <vector>
#include <stdint.h>


typedef std::vector<unsigned char> SgUnsignedCharList;
typedef SgUnsignedCharList* SgUnsignedCharListPtr;

class SgAsmExpression;
typedef std::vector<SgAsmExpression*> SgAsmExpressionPtrList;

typedef uint64_t rose_addr_t;
#endif
