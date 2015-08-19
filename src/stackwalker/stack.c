#include<stdio.h>
#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>

/* Obtain a backtrace and print it to stdout. */
inline void
print_trace (void)
{
    void *array[10];
    size_t size;
    char **strings;
    size_t i;

    size = backtrace (array, 10);
    strings = backtrace_symbols (array, size);

    printf ("Obtained %zd stack frames.\n", size);

    for (i = 0; i < size; i++)
        printf ("%s\n", strings[i]);

    free (strings);
}

void func3(int i){
    printf("func3 %d\n",i);
    //print_trace();
    //while(1);
    if( i > 5 ) {
        while(1){
        };
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
    func2(++i);
    return ;
}

int main(){
    printf("call func1\n");
    func1(0);
    return 0;
}
