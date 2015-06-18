#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>
#include<string>

using namespace std;

void func3(int i){
    printf("func %d\n",i);
    if( i > 5 ) {
        while(1){
        };
        return ;
    }

    func3(++i);
    int fd = open("tmp.txt", O_WRONLY|O_APPEND);
    if(fd < 0)
        printf("file open failed\n");

    string sth = "write something.\n";
    int result = write(fd, sth.c_str(), sth.size() );
    if( result != sth.size() )
        printf("write failed\n");

    return ;
}

void func2(int i){
    printf("func %d\n",i);
    func3(++i);
    return ;
}

void func1(int i){
    printf("func %d\n",i);
    func2(++i);
    return ;
}

int main(){
    printf("call func1\n");
    func1(0);
    return 0;
}
