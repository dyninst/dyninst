/* a struct with one int */
$remote struct intStruct { 
    int style;
};		

$remote struct charStruct {
  string cp;
};

$remote struct boolStruct {
  bool b;
};

$remote class basicClass {
  bool b;
};

/*
 *
 */
$remote test {
    $base 2000;
    $version 1;
    int add(int, int);
    float fadd(float, float);
    vector<int> retVector(int len, int start);
    void nullNull();				// a comment
    int intNull();
    $async void asyncClient();
    void nullStruct(intStruct);
    int intString(const string s1);
    int sumVector(vector<int>);
    int sumVectorPtr(vector<int>*);
    string stringString(string);
    void triggerAsyncUpcall(int val);
    $upcall $async void asyncUpcall(int x);
    vector<string> echoCPA(vector<string>);
    $free vector<string>* echoCPAPtr(vector<string>*);
    charStruct echoCS(charStruct);  
    vector<charStruct> echoCSA(vector<charStruct>);
    string boolToString(boolStruct bs);
    $upcall $async void rapidUpcall(u_int count);
};
