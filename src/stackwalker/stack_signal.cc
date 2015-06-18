#include<stdio.h>

void func3(int i){
    printf("func %d\n",i);
    if( i > 5 ) {
        while(1){
        };
        return ;
    }

    func3(++i);
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
