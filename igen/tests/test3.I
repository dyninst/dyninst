
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

$remote typedef struct {
	float fval;
	double dval;
} fStruct;

/*
 *
 */
$remote test {
    $base 2000;
    $version 1;
    void nullNull();				// a comment
    int intNull();
    double doubleDouble(double);
    float floatFloat(float);
    sStruct msTest(sStruct);
    fStruct fTest(fStruct);
    void nullStruct(intStruct);
    int intString(String);
    $virtual int VintString(String);
    int add(int, int);
    int sumVector($array int);
    $array int retVector(int len, int start);
    String stringString(String);
    void triggerSyncUpcall(int val);
    $upcall void syncUpcall(int x);
    void triggerAsyncUpcall(int val);
    $upcall $async void asyncUpcall(int x);
    $virtual $upcall $async void VasyncUpcall(int x);
    $cmember sStruct s;
    $smember mStruct m;
};
