#if defined(DYNINST_RT_STATIC_LIB)
void (*DYNINSTctors_begin)(void) __attribute__ ((section (".init_array")));
void (*DYNINSTdtors_begin)(void) __attribute__ ((section (".fini_array")));
#endif
