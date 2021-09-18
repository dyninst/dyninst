#if defined(DYNINST_RT_STATIC_LIB)
void (*DYNINSTctors_end)(void) __attribute__((section(".init_array")));
void (*DYNINSTdtors_end)(void) __attribute__((section(".fini_array")));
#endif
