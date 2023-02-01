/* textcolor.c */

#include <stdio.h>
#include "textcolor.h"

void set_color(int attr, int fg, int bg) {
  printf("%c[%d;%d;%dm", 0x1B, attr, fg + 30, bg + 40);
}

void reset_color(void) {
  set_color(COLATTR_RESET, COL_DEFAULT, COL_DEFAULT);
}
