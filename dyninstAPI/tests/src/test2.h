
#ifndef _test2_h_
#define _test2_h_

#if defined(i386_unknown_linux2_0)
#define	TEST_DYNAMIC_LIB	"libX11.so.6"
#define TEST_DYNAMIC_LIB2	"libXt.so.6"
#else
#define	TEST_DYNAMIC_LIB	"libX11.so.4"
#define TEST_DYNAMIC_LIB2	"libXt.so.4"
#endif

#endif /* _test2_h_ */
