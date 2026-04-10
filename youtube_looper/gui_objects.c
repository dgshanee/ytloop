#include "include/gui_objects.h"
#include "raylib.h"
#include <stdio.h>

void draw_fast_forward(void *packet) {
  int *jump = (int *)packet;
  printf("%i\n", *jump);

  char str[32]; // plenty of space
  snprintf(str, sizeof(str), "+%i\n", *jump);

  DrawText(str, 100, 100, 50, RAYWHITE);
}

void draw_pause(void *packet) {
  printf("here\n");
  Color transparent_grey = {0, 0, 0, 128};

  int width = GetScreenWidth();
  int height = GetScreenHeight();

  DrawRectangle(0, 0, width, height, transparent_grey);
}
