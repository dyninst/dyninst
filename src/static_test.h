#ifndef STATIC_TEST_H
#define STATIC_TEST_H

#include "ParameterDict.h"

typedef struct _static_mutator {
  char *test_name;
  int (*mutator)(ParameterDict &param);
} static_mutator_t;

extern "C" int test1_1_mutatorMAIN(ParameterDict &param);
extern "C" int test1_2_mutatorMAIN(ParameterDict &param);
extern "C" int test1_3_mutatorMAIN(ParameterDict &param);
extern "C" int test1_4_mutatorMAIN(ParameterDict &param);
extern "C" int test1_5_mutatorMAIN(ParameterDict &param);
extern "C" int test1_6_mutatorMAIN(ParameterDict &param);
extern "C" int test1_7_mutatorMAIN(ParameterDict &param);
extern "C" int test1_8_mutatorMAIN(ParameterDict &param);
extern "C" int test1_9_mutatorMAIN(ParameterDict &param);
extern "C" int test1_10_mutatorMAIN(ParameterDict &param);
extern "C" int test1_11_mutatorMAIN(ParameterDict &param);
extern "C" int test1_12_mutatorMAIN(ParameterDict &param);
extern "C" int test1_13_mutatorMAIN(ParameterDict &param);
extern "C" int test1_14_mutatorMAIN(ParameterDict &param);
extern "C" int test1_15_mutatorMAIN(ParameterDict &param);
extern "C" int test1_16_mutatorMAIN(ParameterDict &param);
extern "C" int test1_17_mutatorMAIN(ParameterDict &param);
extern "C" int test1_18_mutatorMAIN(ParameterDict &param);
extern "C" int test1_19_mutatorMAIN(ParameterDict &param);
extern "C" int test1_20_mutatorMAIN(ParameterDict &param);
extern "C" int test1_21_mutatorMAIN(ParameterDict &param);
extern "C" int test1_22_mutatorMAIN(ParameterDict &param);
extern "C" int test1_23_mutatorMAIN(ParameterDict &param);
extern "C" int test1_24_mutatorMAIN(ParameterDict &param);
extern "C" int test1_25_mutatorMAIN(ParameterDict &param);
extern "C" int test1_26_mutatorMAIN(ParameterDict &param);
extern "C" int test1_27_mutatorMAIN(ParameterDict &param);
extern "C" int test1_28_mutatorMAIN(ParameterDict &param);
extern "C" int test1_29_mutatorMAIN(ParameterDict &param);
extern "C" int test1_30_mutatorMAIN(ParameterDict &param);
extern "C" int test1_31_mutatorMAIN(ParameterDict &param);
extern "C" int test1_32_mutatorMAIN(ParameterDict &param);
extern "C" int test1_33_mutatorMAIN(ParameterDict &param);
extern "C" int test1_34_mutatorMAIN(ParameterDict &param);
extern "C" int test1_35_mutatorMAIN(ParameterDict &param);
extern "C" int test1_36_mutatorMAIN(ParameterDict &param);
extern "C" int test1_37_mutatorMAIN(ParameterDict &param);
extern "C" int test1_38_mutatorMAIN(ParameterDict &param);
extern "C" int test1_39_mutatorMAIN(ParameterDict &param);
extern "C" int test1_40_mutatorMAIN(ParameterDict &param);
extern "C" int test1_41_mutatorMAIN(ParameterDict &param);
extern "C" int test2_1_mutatorMAIN(ParameterDict &param);
extern "C" int test2_2_mutatorMAIN(ParameterDict &param);
extern "C" int test2_3_mutatorMAIN(ParameterDict &param);
extern "C" int test2_4_mutatorMAIN(ParameterDict &param);
extern "C" int test2_5_mutatorMAIN(ParameterDict &param);
extern "C" int test2_6_mutatorMAIN(ParameterDict &param);
extern "C" int test2_7_mutatorMAIN(ParameterDict &param);
extern "C" int test2_8_mutatorMAIN(ParameterDict &param);
extern "C" int test2_9_mutatorMAIN(ParameterDict &param);
extern "C" int test2_10_mutatorMAIN(ParameterDict &param);
extern "C" int test2_11_mutatorMAIN(ParameterDict &param);
extern "C" int test2_12_mutatorMAIN(ParameterDict &param);
extern "C" int test2_13_mutatorMAIN(ParameterDict &param);
extern "C" int test2_14_mutatorMAIN(ParameterDict &param);
extern "C" int test3_1_mutatorMAIN(ParameterDict &param);
extern "C" int test3_2_mutatorMAIN(ParameterDict &param);
extern "C" int test3_3_mutatorMAIN(ParameterDict &param);
extern "C" int test3_4_mutatorMAIN(ParameterDict &param);
extern "C" int test3_5_mutatorMAIN(ParameterDict &param);
extern "C" int test3_6_mutatorMAIN(ParameterDict &param);
extern "C" int test3_7_mutatorMAIN(ParameterDict &param);
extern "C" int test4_1_mutatorMAIN(ParameterDict &param);
extern "C" int test4_2_mutatorMAIN(ParameterDict &param);
extern "C" int test4_3_mutatorMAIN(ParameterDict &param);
extern "C" int test4_4_mutatorMAIN(ParameterDict &param);
extern "C" int test5_1_mutatorMAIN(ParameterDict &param);
extern "C" int test5_2_mutatorMAIN(ParameterDict &param);
extern "C" int test5_3_mutatorMAIN(ParameterDict &param);
extern "C" int test5_4_mutatorMAIN(ParameterDict &param);
extern "C" int test5_5_mutatorMAIN(ParameterDict &param);
extern "C" int test5_6_mutatorMAIN(ParameterDict &param);
extern "C" int test5_7_mutatorMAIN(ParameterDict &param);
extern "C" int test5_8_mutatorMAIN(ParameterDict &param);
extern "C" int test5_9_mutatorMAIN(ParameterDict &param);
extern "C" int test6_1_mutatorMAIN(ParameterDict &param);
extern "C" int test6_2_mutatorMAIN(ParameterDict &param);
extern "C" int test6_3_mutatorMAIN(ParameterDict &param);
extern "C" int test6_4_mutatorMAIN(ParameterDict &param);
extern "C" int test6_5_mutatorMAIN(ParameterDict &param);
extern "C" int test6_6_mutatorMAIN(ParameterDict &param);
extern "C" int test6_7_mutatorMAIN(ParameterDict &param);
extern "C" int test6_8_mutatorMAIN(ParameterDict &param);
extern "C" int test7_1_mutatorMAIN(ParameterDict &param);
extern "C" int test7_2_mutatorMAIN(ParameterDict &param);
extern "C" int test7_3_mutatorMAIN(ParameterDict &param);
extern "C" int test7_4_mutatorMAIN(ParameterDict &param);
extern "C" int test7_5_mutatorMAIN(ParameterDict &param);
extern "C" int test7_6_mutatorMAIN(ParameterDict &param);
extern "C" int test7_7_mutatorMAIN(ParameterDict &param);
extern "C" int test7_8_mutatorMAIN(ParameterDict &param);
extern "C" int test7_9_mutatorMAIN(ParameterDict &param);
extern "C" int test8_1_mutatorMAIN(ParameterDict &param);
extern "C" int test8_2_mutatorMAIN(ParameterDict &param);
extern "C" int test8_3_mutatorMAIN(ParameterDict &param);
extern "C" int test9_1_mutatorMAIN(ParameterDict &param);
extern "C" int test9_2_mutatorMAIN(ParameterDict &param);
extern "C" int test9_3_mutatorMAIN(ParameterDict &param);
extern "C" int test9_4_mutatorMAIN(ParameterDict &param);
extern "C" int test9_5_mutatorMAIN(ParameterDict &param);
extern "C" int test9_6_mutatorMAIN(ParameterDict &param);
extern "C" int test9_7_mutatorMAIN(ParameterDict &param);
extern "C" int test10_1_mutatorMAIN(ParameterDict &param);
extern "C" int test10_2_mutatorMAIN(ParameterDict &param);
extern "C" int test10_3_mutatorMAIN(ParameterDict &param);
extern "C" int test10_4_mutatorMAIN(ParameterDict &param);
extern "C" int test12_1_mutatorMAIN(ParameterDict &param);
extern "C" int test12_2_mutatorMAIN(ParameterDict &param);
extern "C" int test12_3_mutatorMAIN(ParameterDict &param);
extern "C" int test12_4_mutatorMAIN(ParameterDict &param);
extern "C" int test12_5_mutatorMAIN(ParameterDict &param);
extern "C" int test12_6_mutatorMAIN(ParameterDict &param);
extern "C" int test12_7_mutatorMAIN(ParameterDict &param);
extern "C" int test12_8_mutatorMAIN(ParameterDict &param);
extern "C" int test13_1_mutatorMAIN(ParameterDict &param);
extern "C" int test14_1_mutatorMAIN(ParameterDict &param);
extern "C" int test15_1_mutatorMAIN(ParameterDict &param);
extern "C" int test16_1_mutatorMAIN(ParameterDict &param);

static_mutator_t static_mutators[] = {
  {"test1_1", test1_1_mutatorMAIN},
  {"test1_2", test1_2_mutatorMAIN},
  {"test1_3", test1_3_mutatorMAIN},
  {"test1_4", test1_4_mutatorMAIN},
  {"test1_5", test1_5_mutatorMAIN},
  {"test1_6", test1_6_mutatorMAIN},
  {"test1_7", test1_7_mutatorMAIN},
  {"test1_8", test1_8_mutatorMAIN},
  {"test1_9", test1_9_mutatorMAIN},
  {"test1_10", test1_10_mutatorMAIN},
  {"test1_11", test1_11_mutatorMAIN},
  {"test1_12", test1_12_mutatorMAIN},
  {"test1_13", test1_13_mutatorMAIN},
  {"test1_14", test1_14_mutatorMAIN},
  {"test1_15", test1_15_mutatorMAIN},
  {"test1_16", test1_16_mutatorMAIN},
  {"test1_17", test1_17_mutatorMAIN},
  {"test1_18", test1_18_mutatorMAIN},
  {"test1_19", test1_19_mutatorMAIN},
  {"test1_20", test1_20_mutatorMAIN},
  {"test1_21", test1_21_mutatorMAIN},
  {"test1_22", test1_22_mutatorMAIN},
  {"test1_23", test1_23_mutatorMAIN},
  {"test1_24", test1_24_mutatorMAIN},
  {"test1_25", test1_25_mutatorMAIN},
  {"test1_26", test1_26_mutatorMAIN},
  {"test1_27", test1_27_mutatorMAIN},
  {"test1_28", test1_28_mutatorMAIN},
  {"test1_29", test1_29_mutatorMAIN},
  {"test1_30", test1_30_mutatorMAIN},
  {"test1_31", test1_31_mutatorMAIN},
  {"test1_32", test1_32_mutatorMAIN},
  {"test1_33", test1_33_mutatorMAIN},
  {"test1_34", test1_34_mutatorMAIN},
  {"test1_35", test1_35_mutatorMAIN},
  {"test1_36", test1_36_mutatorMAIN},
  {"test1_37", test1_37_mutatorMAIN},
  {"test1_38", test1_38_mutatorMAIN},
  {"test1_39", test1_39_mutatorMAIN},
  {"test1_40", test1_40_mutatorMAIN},
  {"test1_41", test1_41_mutatorMAIN},
  {"test2_1", test2_1_mutatorMAIN},
  {"test2_2", test2_2_mutatorMAIN},
  {"test2_3", test2_3_mutatorMAIN},
  {"test2_4", test2_4_mutatorMAIN},
  {"test2_5", test2_5_mutatorMAIN},
  {"test2_6", test2_6_mutatorMAIN},
  {"test2_7", test2_7_mutatorMAIN},
  {"test2_8", test2_8_mutatorMAIN},
  {"test2_9", test2_9_mutatorMAIN},
  {"test2_10", test2_10_mutatorMAIN},
  {"test2_11", test2_11_mutatorMAIN},
  {"test2_12", test2_12_mutatorMAIN},
  {"test2_13", test2_13_mutatorMAIN},
  {"test2_14", test2_14_mutatorMAIN},
  {"test3_1", test3_1_mutatorMAIN},
  {"test3_2", test3_2_mutatorMAIN},
  {"test3_3", test3_3_mutatorMAIN},
  {"test3_4", test3_4_mutatorMAIN},
  {"test3_5", test3_5_mutatorMAIN},
  {"test3_6", test3_6_mutatorMAIN},
  {"test3_7", test3_7_mutatorMAIN},
  {"test4_1", test4_1_mutatorMAIN},
  {"test4_2", test4_2_mutatorMAIN},
  {"test4_3", test4_3_mutatorMAIN},
  {"test4_4", test4_4_mutatorMAIN},
  {"test5_1", test5_1_mutatorMAIN},
  {"test5_2", test5_2_mutatorMAIN},
  {"test5_3", test5_3_mutatorMAIN},
  {"test5_4", test5_4_mutatorMAIN},
  {"test5_5", test5_5_mutatorMAIN},
  {"test5_6", test5_6_mutatorMAIN},
  {"test5_7", test5_7_mutatorMAIN},
  {"test5_8", test5_8_mutatorMAIN},
  {"test5_9", test5_9_mutatorMAIN},
  {"test6_1", test6_1_mutatorMAIN},
  {"test6_2", test6_2_mutatorMAIN},
  {"test6_3", test6_3_mutatorMAIN},
  {"test6_4", test6_4_mutatorMAIN},
  {"test6_5", test6_5_mutatorMAIN},
  {"test6_6", test6_6_mutatorMAIN},
  {"test6_7", test6_7_mutatorMAIN},
  {"test6_8", test6_8_mutatorMAIN},
  {"test7_1", test7_1_mutatorMAIN},
  {"test7_2", test7_2_mutatorMAIN},
  {"test7_3", test7_3_mutatorMAIN},
  {"test7_4", test7_4_mutatorMAIN},
  {"test7_5", test7_5_mutatorMAIN},
  {"test7_6", test7_6_mutatorMAIN},
  {"test7_7", test7_7_mutatorMAIN},
  {"test7_8", test7_8_mutatorMAIN},
  {"test7_9", test7_9_mutatorMAIN},
  {"test8_1", test8_1_mutatorMAIN},
  {"test8_2", test8_2_mutatorMAIN},
  {"test8_3", test8_3_mutatorMAIN},
  {"test9_1", test9_1_mutatorMAIN},
  {"test9_2", test9_2_mutatorMAIN},
  {"test9_3", test9_3_mutatorMAIN},
  {"test9_4", test9_4_mutatorMAIN},
  {"test9_5", test9_5_mutatorMAIN},
  {"test9_6", test9_6_mutatorMAIN},
  {"test9_7", test9_7_mutatorMAIN},
  {"test10_1", test10_1_mutatorMAIN},
  {"test10_2", test10_2_mutatorMAIN},
  {"test10_3", test10_3_mutatorMAIN},
  {"test10_4", test10_4_mutatorMAIN},
  {"test12_1", test12_1_mutatorMAIN},
  {"test12_2", test12_2_mutatorMAIN},
  {"test12_3", test12_3_mutatorMAIN},
  {"test12_4", test12_4_mutatorMAIN},
  {"test12_5", test12_5_mutatorMAIN},
  {"test12_6", test12_6_mutatorMAIN},
  {"test12_7", test12_7_mutatorMAIN},
  {"test12_8", test12_8_mutatorMAIN},
  {"test13_1", test13_1_mutatorMAIN},
  {"test14_1", test14_1_mutatorMAIN},
  {"test15_1", test15_1_mutatorMAIN},
  {"test16_1", test16_1_mutatorMAIN},
};
const unsigned int static_mutators_count =
  sizeof(static_mutators) / sizeof(static_mutator_t);

#endif
