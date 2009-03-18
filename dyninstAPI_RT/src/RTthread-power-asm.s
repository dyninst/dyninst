#include "RTconst.h"
#include "../../dyninstAPI/src/inst-power.h"
   .machine "push"      
   .machine "ppc"       
   .globl   DYNINSTthreadIndexFAST
   .globl   .DYNINSTthreadIndexFAST
DYNINSTthreadIndexFAST:
.DYNINSTthreadIndexFAST:
   mr       3,12
   br
   .globl   compare_and_swap2
   .globl   .compare_and_swap2
compare_and_swap2:
.compare_and_swap2:
   lwarx    5,0,3
   stwcx.   4,0,3
   bne-     compare_and_swap2
   mr       3,5
   blr
   .machine "pop"
