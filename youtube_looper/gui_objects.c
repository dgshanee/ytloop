#include "include/gui_objects.h"
#include <stdio.h>

void draw_fast_forward(void *packet) {
  // printf("hello world\n");
  int *jump = (int *)packet;
  // if (jump == NULL) {
  //   jump = 0;
  // };
  // *jump += 5;
  printf("%i\n", *jump);
}

void draw_pause(void *packet) {
  Color transparent_grey = {0, 0, 0, 128};

  int width = GetScreenWidth();
  int height = GetScreenHeight();

  DrawRectangle(0, 0, width, height, transparent_grey);
}
