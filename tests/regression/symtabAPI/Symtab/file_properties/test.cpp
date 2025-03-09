void foo() {}

#ifdef ADD_INTERP
const char __invoke_dynamic_linker__[] __attribute__((section(".interp"))) =
    "/lib/x86_64-linux-gnu/ld-linux-x86-64.so.2";
#endif

#ifdef HAS_MAIN
int main(int, char**) {}
#endif
