/* a struct with one int */
$remote typedef struct { 
    int style;
} intStruct;		

$remote typedef struct { 
    int style;
    intStruct i;
} twoStruct;		

$remote class metric {
    int i;
    u_char j;

    $ignore
    virtual int do_it(){ return 0;};
    $ignore
};

$remote class metricA : metric {
    char x;
    char y;

    $ignore
    virtual int do_it() { return 1;}
    $ignore
};

$remote class metricB : metric {
    int a;
    int b;
    int c;

    $ignore
    virtual int do_it() {return 2;}
    $ignore
};

/*
 *
 */
$remote testT {
    $base 2000;
    $version 1;
    intStruct echoIntStruct(intStruct);
    twoStruct echoTwoStruct(twoStruct);
    intStruct *echoIntStructP(intStruct*);
    twoStruct *echoTwoStructP(twoStruct*);
    metric *testClassPtr(metric*);
    metric *echoMetricPtrPtr(metric*);

    $async void quit();
};
