/*
 * Two versions of foo, foo@V1 (old) and foo@@V2 (default), defined at different addresses, and a
 * call to foo from inside the library.  foo is exported with default visibility, so the call goes
 * through a PLT stub whose jump slot relocation names foo@@V2 by symbol index.  Both versions are
 * dynamic symbols named "foo".
 */
int foo_v1(void) { return 1; }
int foo_v2(void) { return 2; }
__asm__(".symver foo_v1, foo@V1");
__asm__(".symver foo_v2, foo@@V2");

int foo(void);
int caller(void) { return foo() + 1; }
