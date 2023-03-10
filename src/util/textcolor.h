#ifndef TEXTCOLOR_H
#define TEXTCOLOR_H

// I don't know why I added this but it sure does make output look nice :)

enum ColorAttributes {
  COLATTR_RESET = 0,
  COLATTR_BRIGHT = 1,
  COLATTR_DIM = 2,
  COLATTR_ITALIC = 3,
  COLATTR_UNDERLINE = 4,
  COLATTR_BLINK = 5,
  COLATTR_REVERSE = 7,
  COLATTR_HIDDEN = 8,
  COLATTR_STRIKE = 9,
  COLATTR_BRIGHT_UNDERLINE = 21,
  COLATTR_OVERLINE = 53,
};

enum Colors {
  COL_BLACK,
  COL_RED,
  COL_GREEN,
  COL_ORANGE,
  COL_BLUE,
  COL_MAGENTA,
  COL_CYAN,
  COL_LGREY,
  COL_UNDEFINED,
  COL_DEFAULT
};

enum BackgroundColors {
  COLBG_DGREY = 60,
  COLBG_LRED,
  COLBG_LGREEN,
  COLBG_YELLOW,
  COLBG_LBLUE,
  COLBG_LPURPLE,
  COLBG_TEAL,
  COLBG_WHITE,
};

void set_color(int attr, int fg, int bg);
void reset_color(void);


#endif
