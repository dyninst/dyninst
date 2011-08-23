#include "common/h/Types.h"

#include "DynPointMaker.h"
#include "dyninstAPI/src/function.h"
#include "dyninstAPI/src/instPoint.h"
#include "common/h/Types.h"
#include "instructionAPI/h/Instruction.h"

Point *DynPointMaker::mkFuncPoint(Point::Type t, PatchMgrPtr m, PatchFunction *f) {
   return new instPoint(t, m, SCAST_FI(f));
}

Point *DynPointMaker::mkFuncSitePoint(Point::Type t, PatchMgrPtr m, PatchFunction *f, PatchBlock *b) {
   return new instPoint(t, m, SCAST_FI(f), SCAST_BI(b));
}

Point *DynPointMaker::mkBlockPoint(Point::Type t, PatchMgrPtr m, PatchBlock *b, PatchFunction *context) {
   return new instPoint(t, m, SCAST_BI(b), SCAST_FI(context));
}

Point *DynPointMaker::mkInsnPoint(Point::Type t, PatchMgrPtr m, PatchBlock *b, Address a, 
                                  InstructionAPI::Instruction::Ptr i, PatchFunction *context) {
   return new instPoint(t, m, SCAST_BI(b), a, i, SCAST_FI(context));
}

Point *DynPointMaker::mkEdgePoint(Point::Type t, PatchMgrPtr m, PatchEdge *e, PatchFunction *f) {
   return new instPoint(t, m, SCAST_EI(e), SCAST_FI(f));
}
