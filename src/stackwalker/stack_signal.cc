#include <stdio.h>
#include <csignal>
#include <cstdlib>
#include <unistd.h>

void func2(int i);

void signalHandler(int signnum){
    switch(signnum){
        case SIGINT:
            printf("sig int\n");
            break;
        default:
            printf("default sig \n");
    }

    func2(3);

    exit(signnum);
}

void func3(int i){
    printf("func3 %d\n",i);
    if( i > 5 ) {
        //while(1){
        //};
        sleep(7);
        return ;
    }

    func3(++i);
    return ;
}

void func2(int i){
    printf("func2 %d\n",i);
    func3(++i);
    return ;
}

void func1(int i){
    printf("func1 %d\n",i);
    sleep(7);
    raise(SIGINT);

    func2(++i);
    return ;
}

int main(int argc, char **pfd ){
    printf("call func1\n");
    signal(SIGINT, signalHandler);

    func1(0);
    return 0;
}
