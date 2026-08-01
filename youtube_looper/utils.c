#include "include/utils.h"
#include "raylib.h"
#include <Python.h>
#include <signal.h>
#include <string.h>
#include <sys/signal.h>
#include <sys/syslimits.h>
#include <unistd.h>

#ifndef VIDEO_DATA_H
#include "include/video_data.h"
#endif

int get_python_set(PyObject *event) {
  PyGILState_STATE gstate = PyGILState_Ensure();
  printf("%p\n", event);
  if (event == NULL) {
    return -1;
  }
  printf("HERE TOO\n");
  PyObject *set_method = PyObject_GetAttrString(event, "set");
  if (set_method == NULL) {
    return -1;
  }

  PyObject *result = PyObject_CallNoArgs(set_method);
  Py_DECREF(set_method);

  if (result == -1)
    return -1;

  Py_DECREF(result);
  PyGILState_Release(gstate);
}

void handle_write(VideoState *state, char *command, int *letter_count, char key,
                  PyObject *event_emitter) {
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
    printf("HERE\n");
    printf("COMMAND: %s\n", command);
    strcpy(state->command_str, command);
    printf("COPIED COMMAND: %s\n", state->command_str);
    get_python_set(event_emitter);
    state->command_bar_open = false;
    for (int i = 1; i < *letter_count; i++) {
      command[i] = '\0';
    }
    *letter_count = 1;
  }

  // Draw
  DrawText(command, 5, GetScreenHeight() - 15, 2, WHITE);
}

// void draw_pause() {
//   Color transparent_grey = {0, 0, 0, 128};
//
//   int width = GetScreenWidth();
//   int height = GetScreenHeight();
//
//   DrawRectangle(0, 0, width, height, transparent_grey);
//
//   // pause button
// }
