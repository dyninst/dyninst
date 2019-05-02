
#include <stdio.h>
#include <unistd.h>


static int status = -1;

void foo0(void){
  status = 0;
}

void foo1(void) {
  status = 1;
}

void foo2(void) {
  status = 2;
}

void message(const char* msg){
  printf("%s\n", msg);
}

int main(int argc, const char *argv[])
{
  if (argc > 2){
    foo0();
  } else {
    foo1();
    foo2();
  }
  message("Done\n");
  return 0;
}
