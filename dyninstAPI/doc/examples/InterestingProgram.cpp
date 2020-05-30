#include <cstdlib>
#include <iostream>
using namespace std;
class Test{
public:
    int a;
    char b;
    Test(){

    }

};

void InterestingProcedure(){
    cout << " Calling function " << __func__ << endl; 
    Test * test = new Test();
    delete (test);
}

int main(int argc, char * argv[]){
    InterestingProcedure();
    return 0;
}
