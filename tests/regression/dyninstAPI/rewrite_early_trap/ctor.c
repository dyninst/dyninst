// Defined in the mutatee.
int early_trap_target(void);

int ctor_value = -1;

__attribute__((constructor)) static void ctor(void) { ctor_value = 12 + early_trap_target(); }
