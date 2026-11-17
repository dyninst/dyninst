#include <stdio.h>

volatile char g_char = 1;
volatile char g_char_next[3] = {2, 3, 4};
volatile short g_short = 5;
volatile short g_short_next = 6;
volatile long g_long = -1;

int main(void) {
  printf("g_char=%d next=%d,%d,%d g_short=%d next=%d g_long=%#lx\n", g_char, g_char_next[0],
         g_char_next[1], g_char_next[2], g_short, g_short_next, (unsigned long)g_long);
  int const ok = g_char == 0 && g_char_next[0] == 2 && g_char_next[1] == 3 && g_char_next[2] == 4 &&
                 g_short == 0 && g_short_next == 6 && g_long == 0;
  return ok ? 0 : 1;
}
