
/* a struct with one int */
$remote typedef struct { 
    int style;
} intStruct;		

typedef struct s1 {
   int i1;
   int i2;
} s2;

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
    int intString(String);
    int add(int, int);
    int sumVector($array int);
    $array int retVector(int len, int start);
    String stringString(String);
    void triggerAsyncUpcall(int val);
    $upcall $async void asyncUpcall(int x1);
};
