
#ifndef _test1_h_
#define _test1_h_

#define TEST_PTR_32BIT	((void*)0x1234faceUL)
#define TEST_PTR_48BIT	((void *)0x4321abcd8967UL)
#define TEST_PTR_64BIT	((void *)0x5678bbcb9541dabaUL)

#if defined(alpha_dec_osf4_0)	/* Always 64 bits, 48 bit addresses. */
#define TEST_PTR_SIZE	8
#define TEST_PTR	TEST_PTR_48BIT

#elif defined(mips_sgi_irix6_4)	/* Can be 64 or 32 bits. */
#if (_MIPS_SZPTR == 64)
#define TEST_PTR_SIZE	8
#define TEST_PTR	TEST_PTR_64BIT
#else /* _MIPS_SZPTR == 32 */
#define TEST_PTR_SIZE	4
#define TEST_PTR	TEST_PTR_32BIT
#endif

#else /* Others are 32 bits. */
#define TEST_PTR_SIZE	4
#define TEST_PTR	TEST_PTR_32BIT
#endif

#endif /* _test1_h_ */
