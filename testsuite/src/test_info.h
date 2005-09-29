/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

// $Id: test_info.h,v 1.1 2005/09/29 20:40:15 bpellin Exp $
#ifndef TEST_INFO_H
#define TEST_INFO_H

const unsigned int num_tests = 105;
//const unsigned int num_tests = 80;
//const unsigned int num_tests = 20;
mutatee_list_t test1_mutatee = 
   {2, {"test1.mutatee_gcc","test1.mutatee_g++"} };
mutatee_list_t test2_mutatee = 
   {2, {"test2.mutatee_gcc","test2.mutatee_g++"} };
mutatee_list_t test3_mutatee = 
   {1, {"test3.mutatee_gcc"} };
mutatee_list_t test4_mutatee = 
   {1, {"test4a.mutatee_gcc"} };
mutatee_list_t test5_mutatee = 
   {1, {"test5.mutatee_g++"} };
mutatee_list_t test6_mutatee = 
   {1, {"test6.mutatee_gcc"} };
mutatee_list_t test7_mutatee = 
   {1, {"test7.mutatee_gcc"} };
mutatee_list_t test8_mutatee =
   {1, {"test8.mutatee_gcc"} };
mutatee_list_t test9_mutatee =
   {1, {"test9.mutatee_gcc"} };
mutatee_list_t test10_mutatee =
   {1, {"test10.mutatee_gcc"} };
mutatee_list_t test12_mutatee =
   {1, {"test12.mutatee_gcc"} };
mutatee_list_t none =
   {1, {"none"}};

platforms_t all_platforms =
   { true, true, true, true, true, true, true, true};

platforms_t test5_12_platforms =
   { /*alpha_dec_osf5_1        =*/ false,
     /*i386_unknown_linux2_4   =*/ true, 
     /*i386_unknown_nt4_0      =*/ true, 
     /*ia64_unknown_linux2_4   =*/ true,
     /*x86_64_unknown_linux2_4 =*/ true,
     /*mips_sgi_irix6_5        =*/ false,
     /*rs6000_ibm_aix5_1       =*/ true,
     /*sparc_sun_solaris2_8    =*/ true,
   };

platforms_t test7_8_platforms =
   { /*alpha_dec_osf5_1        =*/ false,
     /*i386_unknown_linux2_4   =*/ true, 
     /*i386_unknown_nt4_0      =*/ true, 
     /*ia64_unknown_linux2_4   =*/ true,
     /*x86_64_unknown_linux2_4 =*/ true,
     /*mips_sgi_irix6_5        =*/ true,
     /*rs6000_ibm_aix5_1       =*/ true,
     /*sparc_sun_solaris2_8    =*/ true,
   };

platforms_t test9_platforms =
   { /*alpha_dec_osf5_1        =*/ false,
     /*i386_unknown_linux2_4   =*/ true, 
     /*i386_unknown_nt4_0      =*/ true, 
     /*ia64_unknown_linux2_4   =*/ false,
     /*x86_64_unknown_linux2_4 =*/ true,
     /*mips_sgi_irix6_5        =*/ false,
     /*rs6000_ibm_aix5_1       =*/ true,
     /*sparc_sun_solaris2_8    =*/ true,
   };

platforms_t test10_11_platforms =
   { /*alpha_dec_osf5_1        =*/ false,
     /*i386_unknown_linux2_4   =*/ false, 
     /*i386_unknown_nt4_0      =*/ false, 
     /*ia64_unknown_linux2_4   =*/ false,
     /*x86_64_unknown_linux2_4 =*/ false,
     /*mips_sgi_irix6_5        =*/ false,
     /*rs6000_ibm_aix5_1       =*/ false,
     /*sparc_sun_solaris2_8    =*/ true,
   };

test_data_t tests[] = {
   {"test1_1", "./test1_1.so", test1_mutatee, all_platforms, STOPPED, 1, 1, CREATE, ENABLED},
   {"test1_2", "./test1_2.so", test1_mutatee, all_platforms, STOPPED, 1, 2, CREATE, ENABLED},
   {"test1_3", "./test1_3.so", test1_mutatee, all_platforms, STOPPED, 1, 3, CREATE, ENABLED},
   {"test1_4", "./test1_4.so", test1_mutatee, all_platforms, STOPPED, 1, 4, CREATE, ENABLED},
   {"test1_5", "./test1_5.so", test1_mutatee, all_platforms, STOPPED, 1, 5, CREATE, ENABLED},
   {"test1_6", "./test1_6.so", test1_mutatee, all_platforms, STOPPED, 1, 6, CREATE, ENABLED},
   {"test1_7", "./test1_7.so", test1_mutatee, all_platforms, STOPPED, 1, 7, CREATE, ENABLED},
   {"test1_8", "./test1_8.so", test1_mutatee, all_platforms, STOPPED, 1, 8, CREATE, ENABLED},
   {"test1_9", "./test1_9.so", test1_mutatee, all_platforms, STOPPED, 1, 9, CREATE, ENABLED},
   {"test1_10", "./test1_10.so", test1_mutatee, all_platforms, STOPPED, 1, 10, CREATE, ENABLED},
   {"test1_11", "./test1_11.so", test1_mutatee, all_platforms, STOPPED, 1, 11, CREATE, ENABLED},
   {"test1_12", "./test1_12.so", test1_mutatee, all_platforms, STOPPED, 1, 12, CREATE, ENABLED},
   {"test1_13", "./test1_13.so", test1_mutatee, all_platforms, STOPPED, 1, 13, CREATE, ENABLED},
   {"test1_14", "./test1_14.so", test1_mutatee, all_platforms, STOPPED, 1, 14, CREATE, ENABLED},
   {"test1_15", "./test1_15.so", test1_mutatee, all_platforms, STOPPED, 1, 15, CREATE, ENABLED},
   {"test1_16", "./test1_16.so", test1_mutatee, all_platforms, STOPPED, 1, 16, CREATE, ENABLED},
   {"test1_17", "./test1_17.so", test1_mutatee, all_platforms, STOPPED, 1, 17, CREATE, ENABLED},
   {"test1_18", "./test1_18.so", test1_mutatee, all_platforms, STOPPED, 1, 18, CREATE, ENABLED},
   {"test1_19", "./test1_19.so", test1_mutatee, all_platforms, RUNNING, 1, 19, CREATE, ENABLED},
   {"test1_20", "./test1_20.so", test1_mutatee, all_platforms, STOPPED, 1, 20, CREATE, ENABLED},
   {"test1_21", "./test1_21.so", test1_mutatee, all_platforms, STOPPED, 1, 21, CREATE, ENABLED},
   {"test1_22", "./test1_22.so", test1_mutatee, all_platforms, STOPPED, 1, 22, CREATE, ENABLED},
   {"test1_23", "./test1_23.so", test1_mutatee, all_platforms, STOPPED, 1, 23, CREATE, ENABLED},
   {"test1_24", "./test1_24.so", test1_mutatee, all_platforms, STOPPED, 1, 24, CREATE, ENABLED},
   {"test1_25", "./test1_25.so", test1_mutatee, all_platforms, STOPPED, 1, 25, CREATE, ENABLED},
   {"test1_26", "./test1_26.so", test1_mutatee, all_platforms, STOPPED, 1, 26, CREATE, ENABLED},
   {"test1_27", "./test1_27.so", test1_mutatee, all_platforms, STOPPED, 1, 27, CREATE, ENABLED},
   {"test1_28", "./test1_28.so", test1_mutatee, all_platforms, STOPPED, 1, 28, CREATE, ENABLED},
   {"test1_29", "./test1_29.so", test1_mutatee, all_platforms, STOPPED, 1, 29, CREATE, ENABLED},
   {"test1_30", "./test1_30.so", test1_mutatee, all_platforms, STOPPED, 1, 30, CREATE, ENABLED},
   {"test1_31", "./test1_31.so", test1_mutatee, all_platforms, STOPPED, 1, 31, CREATE, ENABLED},
   {"test1_32", "./test1_32.so", test1_mutatee, all_platforms, STOPPED, 1, 32, CREATE, ENABLED},
   {"test1_33", "./test1_33.so", test1_mutatee, all_platforms, STOPPED, 1, 33, CREATE, ENABLED},
   {"test1_34", "./test1_34.so", test1_mutatee, all_platforms, STOPPED, 1, 34, CREATE, ENABLED},
   {"test1_35", "./test1_35.so", test1_mutatee, all_platforms, STOPPED, 1, 35, CREATE, ENABLED},
   {"test1_36", "./test1_36.so", test1_mutatee, all_platforms, STOPPED, 1, 36, CREATE, ENABLED},
   {"test1_37", "./test1_37.so", test1_mutatee, all_platforms, STOPPED, 1, 37, CREATE, ENABLED},
   {"test1_38", "./test1_38.so", test1_mutatee, all_platforms, STOPPED, 1, 38, CREATE, ENABLED},
   {"test1_39", "./test1_39.so", test1_mutatee, all_platforms, STOPPED, 1, 39, CREATE, ENABLED},
   {"test1_40", "./test1_40.so", test1_mutatee, all_platforms, STOPPED, 1, 40, CREATE, ENABLED},
   {"test2_1", "./test2_1.so", none, all_platforms, NOMUTATEE, 2, 1, CREATE, ENABLED},
   {"test2_2", "./test2_2.so", none, all_platforms, NOMUTATEE, 2, 2, CREATE, ENABLED},
   {"test2_3", "./test2_3.so", none, all_platforms, NOMUTATEE, 2, 3, USEATTACH, ENABLED},
   {"test2_4", "./test2_4.so", none, all_platforms, NOMUTATEE, 2, 4, USEATTACH, ENABLED},
   {"test2_5", "./test2_5.so", test2_mutatee, all_platforms, STOPPED, 2, 5, CREATE, ENABLED},
   {"test2_6", "./test2_6.so", test2_mutatee, all_platforms, STOPPED, 2, 6, CREATE, ENABLED},
   {"test2_7", "./test2_7.so", test2_mutatee, all_platforms, STOPPED, 2, 7, CREATE, ENABLED},
   {"test2_8", "./test2_8.so", test2_mutatee, all_platforms, STOPPED, 2, 8, CREATE, ENABLED},
   {"test2_9", "./test2_9.so", test2_mutatee, all_platforms, STOPPED, 2, 9, CREATE, ENABLED},
   {"test2_10", "./test2_10.so", test2_mutatee, all_platforms, STOPPED, 2, 10, CREATE, ENABLED},
   {"test2_11", "./test2_11.so", test2_mutatee, all_platforms, STOPPED, 2, 11, CREATE, ENABLED},
   {"test2_12", "./test2_12.so", test2_mutatee, all_platforms, STOPPED, 2, 12, CREATE, ENABLED},
   {"test2_13", "./test2_13.so", test2_mutatee, all_platforms, STOPPED, 2, 13, CREATE, ENABLED},
   {"test2_14", "./test2_14.so", test2_mutatee, all_platforms, STOPPED, 2, 14, CREATE, ENABLED},
   {"test3_1", "./test3_1.so", test3_mutatee, all_platforms, NOMUTATEE, 3, 1, CREATE, ENABLED},
   {"test3_2", "./test3_2.so", test3_mutatee, all_platforms, NOMUTATEE, 3, 2, CREATE, ENABLED},
   {"test3_3", "./test3_3.so", test3_mutatee, all_platforms, NOMUTATEE, 3, 3, CREATE, ENABLED},
   {"test3_4", "./test3_4.so", test3_mutatee, all_platforms, NOMUTATEE, 3, 4, CREATE, ENABLED},
   {"test3_5", "./test3_5.so", test3_mutatee, all_platforms, NOMUTATEE, 3, 5, CREATE, ENABLED},
   {"test4_1", "./test4_1.so", test4_mutatee, all_platforms, NOMUTATEE, 4, 1, CREATE, ENABLED},
   {"test4_2", "./test4_2.so", test4_mutatee, all_platforms, NOMUTATEE, 4, 2, CREATE, ENABLED},
   /* Not terminating x86-linux */
   {"test4_3", "./test4_3.so", test4_mutatee, all_platforms, NOMUTATEE, 4, 3, CREATE, ENABLED},
   {"test4_4", "./test4_4.so", test4_mutatee, all_platforms, NOMUTATEE, 4, 4, CREATE, ENABLED},
   {"test5_1", "./test5_1.so", test5_mutatee, test5_12_platforms, STOPPED, 5, 1, CREATE, ENABLED},
   /* Segfaulting  x86-linux */
   {"test5_2", "./test5_2.so", test5_mutatee, test5_12_platforms, STOPPED, 5, 2, CREATE, ENABLED},
   {"test5_3", "./test5_3.so", test5_mutatee, test5_12_platforms, STOPPED, 5, 3, CREATE, ENABLED},
   {"test5_4", "./test5_4.so", test5_mutatee, test5_12_platforms, STOPPED, 5, 4, CREATE, ENABLED},
   {"test5_5", "./test5_5.so", test5_mutatee, test5_12_platforms, STOPPED, 5, 5, CREATE, ENABLED},
   {"test5_6", "./test5_6.so", test5_mutatee, test5_12_platforms, STOPPED, 5, 6, CREATE, ENABLED},
   {"test5_7", "./test5_7.so", test5_mutatee, test5_12_platforms, STOPPED, 5, 7, CREATE, ENABLED},
   {"test5_8", "./test5_8.so", test5_mutatee, test5_12_platforms, STOPPED, 5, 8, CREATE, ENABLED},
   {"test5_9", "./test5_9.so", test5_mutatee, test5_12_platforms, STOPPED, 5, 9, CREATE, ENABLED},
   {"test6_1", "./test6_1.so", test6_mutatee, all_platforms, STOPPED, 6, 1, CREATE, ENABLED},
   {"test6_2", "./test6_2.so", test6_mutatee, all_platforms, STOPPED, 6, 2, CREATE, ENABLED},
   {"test6_3", "./test6_3.so", test6_mutatee, all_platforms, STOPPED, 6, 3, CREATE, ENABLED},
   {"test6_4", "./test6_4.so", test6_mutatee, all_platforms, STOPPED, 6, 4, CREATE, ENABLED},
   {"test6_5", "./test6_5.so", test6_mutatee, all_platforms, STOPPED, 6, 5, CREATE, ENABLED},
   {"test6_6", "./test6_6.so", test6_mutatee, all_platforms, STOPPED, 6, 6, CREATE, ENABLED},
   {"test6_7", "./test6_7.so", test6_mutatee, all_platforms, STOPPED, 6, 7, CREATE, ENABLED},
   {"test6_8", "./test6_8.so", test6_mutatee, all_platforms, STOPPED, 6, 8, CREATE, ENABLED},
   {"test7_1", "./test7_1.so", test7_mutatee, test7_8_platforms, STOPPED, 7, 1, CREATE, ENABLED},
   {"test7_2", "./test7_2.so", test7_mutatee, test7_8_platforms, STOPPED, 7, 2, CREATE, ENABLED},
   {"test7_3", "./test7_3.so", test7_mutatee, test7_8_platforms, STOPPED, 7, 3, CREATE, ENABLED},
   {"test7_4", "./test7_4.so", test7_mutatee, test7_8_platforms, STOPPED, 7, 4, CREATE, ENABLED},
   {"test7_5", "./test7_5.so", test7_mutatee, test7_8_platforms, STOPPED, 7, 5, CREATE, ENABLED},
   {"test7_6", "./test7_6.so", test7_mutatee, test7_8_platforms, STOPPED, 7, 6, CREATE, ENABLED},
   {"test7_7", "./test7_7.so", test7_mutatee, test7_8_platforms, STOPPED, 7, 7, CREATE, ENABLED},
   {"test7_8", "./test7_8.so", test7_mutatee, test7_8_platforms, STOPPED, 7, 8, CREATE, ENABLED},
   {"test8_1", "./test8_1.so", test8_mutatee, test7_8_platforms, STOPPED, 8, 1, CREATE, ENABLED},
   {"test8_2", "./test8_2.so", test8_mutatee, test7_8_platforms, STOPPED, 8, 2, CREATE, ENABLED},
   {"test8_3", "./test8_3.so", test8_mutatee, test7_8_platforms, STOPPED, 8, 3, CREATE, ENABLED},
   {"test9_1", "./test9_1.so", test9_mutatee, test9_platforms, NOMUTATEE, 9, 1, CREATE, ENABLED},
   {"test9_2", "./test9_2.so", test9_mutatee, test9_platforms, NOMUTATEE, 9, 2, CREATE, ENABLED},
   {"test9_3", "./test9_3.so", test9_mutatee, test9_platforms, NOMUTATEE, 9, 3, CREATE, ENABLED},
   {"test9_4", "./test9_4.so", test9_mutatee, test9_platforms, NOMUTATEE, 9, 4, CREATE, ENABLED},
   {"test9_5", "./test9_5.so", test9_mutatee, test9_platforms, NOMUTATEE, 9, 5, CREATE, ENABLED},
   {"test9_6", "./test9_6.so", test9_mutatee, test9_platforms, NOMUTATEE, 9, 6, CREATE, ENABLED},
   {"test10_1", "./test10_1.so", test10_mutatee, test10_11_platforms, STOPPED, 10, 1, CREATE, ENABLED},
   {"test10_2", "./test10_2.so", test10_mutatee, test10_11_platforms, STOPPED, 10, 2, CREATE, ENABLED},
   {"test10_3", "./test10_3.so", test10_mutatee, test10_11_platforms, STOPPED, 10, 3, CREATE, ENABLED},
   {"test10_4", "./test10_4.so", test10_mutatee, test10_11_platforms, STOPPED, 10, 4, CREATE, ENABLED},
   {"test12_1", "./test12_1.so", test12_mutatee, test5_12_platforms, STOPPED, 12, 1, CREATE, ENABLED},
   {"test12_2", "./test12_2.so", test12_mutatee, test5_12_platforms, STOPPED, 12, 2, CREATE, ENABLED},
   /* Segfaulting on x86-linux */
   {"test12_3", "./test12_3.so", test12_mutatee, test5_12_platforms, STOPPED, 12, 3, CREATE, ENABLED},
   {"test12_4", "./test12_4.so", test12_mutatee, test5_12_platforms, STOPPED, 12, 4, CREATE, ENABLED}
};
#endif /* TEST_INFO_H */
