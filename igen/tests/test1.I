/* a struct with one int */
$remote typedef struct { 
    int style;
} intStruct;		

$remote typedef struct {
  char *cp;
} charStruct;

$remote array charStruct charStruct_Array;

$remote array int int_Array; 
$remote array char* char_PTR_Array; 

/*
 *
 */
$remote test {
    $base 2000;
    $version 1;
    void nullNull();				// a comment
    int intNull();
    $async void asyncClient();
    void nullStruct(intStruct);
    int intString(const char *s1);
    int add(int, int);
    float fadd(float, float);
    int sumVector(int_Array);
    int_Array retVector(int len, int start);
    char *stringString(char *);
    void triggerAsyncUpcall(int val);
    int triggerSyncUpcall(int val);
    $upcall $async void asyncUpcall(int x);
    $upcall int SyncUpcall(int x);
    $noFree char_PTR_Array echoCPA(char_PTR_Array);
    charStruct echoCS(charStruct);  
    $noFree charStruct *echoCSP(charStruct *);
    charStruct_Array echoCSA(charStruct_Array);
    $noFree charStruct_Array *echoCSAP(charStruct_Array*);
};
