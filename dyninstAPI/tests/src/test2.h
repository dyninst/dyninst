
#ifndef _test2_h_
#define _test2_h_

#if defined(i386_unknown_linux2_0) || defined(ia64_unknown_linux2_4)
#define	TEST_DYNAMIC_LIB	"libX11.so.6"
#define TEST_DYNAMIC_LIB2	"libXt.so.6"
#elif defined(mips_sgi_irix6_4) || defined(alpha_dec_osf4_0)
#define	TEST_DYNAMIC_LIB	"libXaw.so"
#define TEST_DYNAMIC_LIB2	"libXt.so"
#elif defined(rs6000_ibm_aix4_1)
#define TEST_DYNAMIC_LIB        "./libtestA.so"
#define TEST_DYNAMIC_LIB_NOPATH "libtestA.so"
#define TEST_DYNAMIC_LIB2       "./libtestB.so"
#define TEST_DYNAMIC_LIB2_NOPATH "libtestB.so"
#else
#define	TEST_DYNAMIC_LIB	"libX11.so.4"
#define TEST_DYNAMIC_LIB2	"libXt.so.4"
#endif

#endif /* _test2_h_ */
