
#ifndef _test2_h_
#define _test2_h_

#if defined(i386_unknown_linux2_0)
#define	TEST_DYNAMIC_LIB	"libX11.so.6"
#define TEST_DYNAMIC_LIB2	"libXt.so.6"
#elif defined(mips_sgi_irix6_4) || defined(alpha_dec_osf4_0)
#define	TEST_DYNAMIC_LIB	"libXaw.so"
#define TEST_DYNAMIC_LIB2	"libXt.so"
#else
#define	TEST_DYNAMIC_LIB	"libX11.so.4"
#define TEST_DYNAMIC_LIB2	"libXt.so.4"
#endif

#endif /* _test2_h_ */
