
/* a struct with one int */
$remote typedef struct { 
    int style;
} intStruct;		

$remote typedef struct {
    int computer;
    int science;
} mStruct;

$remote typedef struct {
    int computer;
    String name;
} sStruct;

/*
 *
 */
$remote test {
    $base 2000;
    $version 1;
    void nullNull();				// a comment
    int intNull();
    sStruct msTest(sStruct);
    void nullStruct(intStruct);
    int intString(String);
    int add(int, int);
    int sumVector($array int);
    $array int retVector(int len, int start);
    String stringString(String);
    void triggerSyncUpcall(int val);
    $upcall void syncUpcall(int x);
    void triggerAsyncUpcall(int val);
    $upcall $async void asyncUpcall(int x);
};
