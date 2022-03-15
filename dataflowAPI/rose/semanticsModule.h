#ifndef ROSE_SEMANTICSMODULE_H
#define ROSE_SEMANTICSMODULE_H

#include "SgAsmType.h"

//#include "rose.h"
#include <inttypes.h>

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif
#include <stdlib.h>

#include <iostream>

static inline int numBytesInAsmType(SgAsmType* ty) {
  switch (ty->variantT()) {
    case V_SgAsmTypeByte: return 1;
    case V_SgAsmTypeWord: return 2;
    case V_SgAsmTypeDoubleWord: return 4;
    case V_SgAsmTypeQuadWord: return 8;
    default: {
        std::cerr << "Unhandled type " << ty->class_name() << " in numBytesInAsmType" << std::endl; 
        abort();
        return 0;
    }
  }
}

#endif // ROSE_SEMANTICSMODULE_H
