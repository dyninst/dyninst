#include <stdio.h>
#include <unistd.h>

int main()
{
    pid_t pid = getpid();
    printf("Hello, world. My pid is %d\n", pid);
}

