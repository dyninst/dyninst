
/* a struct with one int */
$remote typedef struct { 
    int style;
} intStruct;		

$remote typedef struct {
   int i1;
   int i2;
} s2;

$remote array int int_Array;

/*
 *
 */
$remote test {
    $base 2000;
    $version 1;
    void nullNull();
    int intNull();
    s2 *isSp(s2*);
    s2 isS(s2);
    void nullStruct(intStruct);
    int intString(char *);
    int add(int, int);
    int sumVector(int_Array);
    int_Array retVector(int len, int start);
    char *stringString(char *);
    void triggerAsyncUpcall(int val);
    $upcall $async void asyncUpcall(int x1);
};
