/* a struct with one int */
$remote struct intStruct { 
    int style;
};		

$remote struct resStruct {
  pdvector<string> parts;
  u_int handle;
};

$remote struct charStruct {
  string cp;
};

$remote struct boolStruct {
  bool b;
};

$remote class basicClass {
  bool b;
  $ignore
    struct t {
      int i;
    };
  $ignore
};

$remote class derClass : basicClass {
  u_int u;
  $ignore
    // another comment
  $ignore
};

/*
 *
 */
$remote test {
    $base 2000;
    $version 1;
    $free basicClass *echoClass(basicClass *);
    basicClass echoBClass(basicClass);
    derClass echoDClass(derClass);
    int add(int, int);
    float fadd(float, float);
    pdvector<int> retVector(int len, int start);
    void nullNull();				// a comment
    int intNull();
    $async void asyncClient();
    void nullStruct(intStruct);
    int intString(string s1);
    int sumVector(pdvector<int>);
    int sumVectorPtr(pdvector<int>*);
    void triggerAsyncUpcall(int val);
    $upcall $async void asyncUpcall(int x);
    pdvector<string> echoCPA(pdvector<string>);
    $free pdvector<string>* echoCPAPtr(pdvector<string>*);
    charStruct echoCS(charStruct);  
    $free charStruct *echoCSP(charStruct*);  
    pdvector<charStruct> echoCSA(pdvector<charStruct>);
    string boolToString(boolStruct bs);
    $upcall $async void rapidUpcall(u_int count);
    string stringString(string);
    string stringStringRef(string &s);
    pdvector<string> refVector(pdvector<string> &vec);
    pdvector<string> norefVector(pdvector<string> vec);
    $upcall $async void up1();
    $upcall $async void up2();
    $upcall $async void up3();
    $upcall $async void up4();
    $upcall $async void up_done(u_int total);
    pdvector<resStruct> echoResStruct(pdvector<resStruct>);
};
