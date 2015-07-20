#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>
#include<string>
#include<fstream>
#include<sys/time.h>
#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

/* Obtain a backtrace and print it to stdout. */
//void
//print_trace (void)
#define print_trace() \
{ \
    void *array[10]; \
    size_t size; \
    char **strings; \
    size_t i; \
    size = backtrace (array, 10); \
    strings = backtrace_symbols (array, size); \
    printf ("Obtained %zd stack frames.\n", size); \
    for (i = 0; i < size; i++) \
        printf ("%s\n", strings[i]); \
    free (strings); \
}

void func3(int i){
    printf("func3 %d\n",i);
    if( i > 5 ) {
        print_trace();
        sleep(7);
        while(1){
        };
        return ;
    }

    func3(++i);

    return ;
}

void func2(int i){
    printf("func2 %d\n",i);
    //__asm__("BRK #0\n");

    void *brk = sbrk(0);
    printf("brk %p\n", brk);
    func3(++i);
    return ;
}

void func1(int i){
    printf("func1 %d\n",i);

    //call a syscall wrapper
    printf("open file\n");
    ofstream file;
    file.open("tmp.txt");
    file<< "write something.\n";
    printf("close file\n");
    file.close();

    //call a syscall directly
    const char msg[] = "Hello World!\n";
    syscall(64, STDOUT_FILENO, msg, sizeof(msg)-1);
    //syscall(4, STDOUT_FILENO, msg, sizeof(msg)-1);

    func2(++i);
    return ;
}

int main(){
    printf("call func1\n");
    int pid = getpid();
    struct timeval time;
    gettimeofday(&time, NULL);
    printf("pid (%d)\n", pid);
    func1(0);
    return 0;
}
