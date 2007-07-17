   .machine "push"      
   .machine "ppc"       


# /* ---------------------------------- */
# /* unsigned DYNINSTthreadIndexFAST(); */
# /* ---------------------------------- */
   .globl   DYNINSTthreadIndexFAST
   .globl   .DYNINSTthreadIndexFAST
DYNINSTthreadIndexFAST:
.DYNINSTthreadIndexFAST:
   mr       3,12    # function return value = r3 = r12
   blr              # branch via link register (function return)


# /* ------------------------------------------- */
# /* int atomic_set(volatile int *int_ptr);      */
# /*                                             */
# /* The only possible values at the given       */
# /* memory location are 0 and 1.  Attempt to    */
# /* atomically update the value from 0 to 1.    */
# /* Return 1 if such an atomic update occurred; */
# /* return 0 otherwise.                         */
# /* ------------------------------------------- */
   .globl atomic_set
   .globl .atomic_set
atomic_set:
.atomic_set:
   addi     4,0,1               # r4 = 1
                                # Attempt atomic memory swap
   lwarx    5,0,3               #   r5 = *int_ptr (load reserve indexed)
   stwcx.   4,0,3               #   *int_ptr = 1  (store conditional indexed)
   bne-     atomic_set_return_0 # if atomic swap failed, return 0

   cmpw     5,4                 # if original value was already 1,
   beq-     atomic_set_return_0 #   return 0 because no value update occurred

   addi     3,0,1               # function return value = r3 = 1
   blr                          # branch via link register (function return)

atomic_set_return_0:
   addi     3,0,0               # function return value = r3 = 0
   blr                          # branch via link register (function return)


   .machine "pop"
