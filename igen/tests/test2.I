
/* a struct with one int */
$remote struct intStruct { 
    int style;
}; 

$remote struct s2 {
   int i1;
   int i2;
};

typedef struct other {
  int i;
  float k;
} other;

class scope {
public:
  typedef enum { apple, orange, pear } fruit;
};

/*
 *
 */
$remote test {
    $base 2000;
    $version 1;
    scope::fruit echoFruit(scope::fruit);
    other *testOther(other *);
    void nullNull();
    int intNull();
    s2 *isSp(s2*);
    void nullStruct(intStruct);
    int intString(string*);
    int add(const int, const int);
    int sumVector(pdvector<int>*);
    pdvector<int> *retVector(int len, int start);
    string *stringString(string*);
    void triggerAsyncUpcall(int val);
    $upcall $async void asyncUpcall(int x1);
    void hangup();

       $cignore
           int j;
           float k;
        $cignore

       $signore 
         int i;
       $signore
};
