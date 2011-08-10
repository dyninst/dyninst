#include <stdio.h>
#include <assert.h>

int orig(int i) {
   fprintf(stderr, "Original: %d\n", i);
   return i + 5;
}

int main() {
   return orig(10);
}
