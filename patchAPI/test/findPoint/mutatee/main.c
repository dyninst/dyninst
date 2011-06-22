#include <stdio.h>
#include <unistd.h>

void foo1() {
	printf("i'm foo1\n");
}
void foo2() {
	printf("i'm foo2\n");
}

int main(int argc, const char *argv[])
{
  if (argc > 2) return 0;
  foo1();
  foo2();
	return 0;
}
