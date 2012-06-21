#if defined(os_windows_test)
#define DLLEXPORT __declspec( dllexport )
#else
#define DLLEXPORT
#endif

#if defined(os_windows_test) && defined(__cplusplus)
extern "C" {
#endif

//------------------------------------------------
// patch3_1
//------------------------------------------------
DLLEXPORT void patch3_1_call3_1(int *var) {
  *var = 2;
}

//------------------------------------------------
// patch3_2
//------------------------------------------------
#define PATCH3_2_MAGIC1   2200100
#define PATCH3_2_MAGIC2   2200200
#define PATCH3_2_MAGIC3   2200300
#define PATCH3_2_MAGIC4   2200400
#define PATCH3_2_MAGIC5A  2200510
#define PATCH3_2_MAGIC5B  2200520
#define PATCH3_2_MAGIC6   2200600
#define PATCH3_2_MAGIC7   2200700

DLLEXPORT int patch3_2_call4()
{
    return PATCH3_2_MAGIC4;
}

DLLEXPORT int patch3_2_call5a()
{
    return PATCH3_2_MAGIC5A;
}

DLLEXPORT int patch3_2_call6()
{
    return PATCH3_2_MAGIC6;
}


#if defined(os_windows_test) && defined(__cplusplus)
}
#endif
