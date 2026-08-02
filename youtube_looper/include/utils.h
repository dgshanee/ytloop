#define UTILS_H

#include <Python.h>

#include "commands.h"
void draw_command_prompt();

// void draw_pause();

void handle_write(VideoState *state_machine, char *command, int *letter_count,
                  char key, PyObject *event_emitter);
