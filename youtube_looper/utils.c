#include "include/utils.h"
#include "raylib.h"
#include <string.h>
#include <sys/syslimits.h>

#ifndef VIDEO_DATA_H
#include "include/video_data.h"
#endif

void draw_command_prompt() {
  DrawRectangle(0, GetScreenHeight() - 20, GetScreenWidth(), 100, BLACK);
}

void handle_write(VideoState *state, char *command, int *letter_count,
                  char key) {
  while (key > 0) {
    if ((key >= 32) && (key <= 125) && (*letter_count < MAX_INPUT)) {
      command[*letter_count] = key;
      command[*letter_count + 1] = '\0';
      *letter_count += 1;
    }

    key = GetCharPressed();
  }

  if (IsKeyPressed(KEY_BACKSPACE)) {
    *letter_count -= 1;
    if (*letter_count < 0)
      *letter_count = 0;
    command[*letter_count] = '\0';
  }

  if (IsKeyPressed(KEY_ENTER)) {
    // handle command enter

    if (strcmp(command, ":q") == 0) {
      CloseWindow();
    }

    state->command_bar_open = false;
    for (int i = 1; i < *letter_count; i++) {
      command[i] = '\0';
    }
    *letter_count = 1;
  }

  // Draw
  DrawText(command, 5, GetScreenHeight() - 15, 2, WHITE);
}

void draw_pause() {
  Color transparent_grey = {0, 0, 0, 128};

  int width = GetScreenWidth();
  int height = GetScreenHeight();

  DrawRectangle(0, 0, width, height, transparent_grey);

  // pause button
}
