#include <stdio.h>

void test() {
  fprintf(stderr, "I'm here!\n");
  return;
}

int main() {
  fprintf(stderr, "In main\n");
  return 1;
}
