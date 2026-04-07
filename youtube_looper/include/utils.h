#define UTILS_H

#ifndef STATE_H
#include "state.h"
#endif
void draw_command_prompt();

void draw_pause();

void handle_write(VideoState* state_machine, char* command, int* letter_count, char key);
