#include "include/gui_objects.h"
#include <stdio.h>

void draw_pause(void *packet) {
  Color transparent_grey = {0, 0, 0, 128};

  int width = GetScreenWidth();
  int height = GetScreenHeight();

  DrawRectangle(0, 0, width, height, transparent_grey);
}
