/* a struct with one int */
$remote struct intStruct { 
    int style;
};		

$remote struct resStruct {
  vector<string> parts;
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
    vector<int> retVector(int len, int start);
    void nullNull();				// a comment
    int intNull();
    $async void asyncClient();
    void nullStruct(intStruct);
    int intString(string s1);
    int sumVector(vector<int>);
    int sumVectorPtr(vector<int>*);
    void triggerAsyncUpcall(int val);
    $upcall $async void asyncUpcall(int x);
    vector<string> echoCPA(vector<string>);
    $free vector<string>* echoCPAPtr(vector<string>*);
    charStruct echoCS(charStruct);  
    $free charStruct *echoCSP(charStruct*);  
    vector<charStruct> echoCSA(vector<charStruct>);
    string boolToString(boolStruct bs);
    $upcall $async void rapidUpcall(u_int count);
    string stringString(string);
    string stringStringRef(string &s);
    vector<string> refVector(vector<string> &vec);
    vector<string> norefVector(vector<string> vec);
    $upcall $async void up1();
    $upcall $async void up2();
    $upcall $async void up3();
    $upcall $async void up4();
    $upcall $async void up_done(u_int total);
    vector<resStruct> echoResStruct(vector<resStruct>);
};
