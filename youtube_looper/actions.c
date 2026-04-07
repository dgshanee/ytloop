#include "include/actions.h"
#ifndef STATE_H
#include "include/state.h"
#endif
#include <stdio.h>

void open_command_prompt(void* arg){
  VideoState* state = (VideoState*)arg;
  state->command_bar_open = true;
}

void cancel(void* arg){
  VideoState* state = (VideoState*)arg;
  if(state->command_bar_open == true){
    state->command_bar_open = false;
    return;
  }
}

void toggle_playback(void* arg){
  VideoState* state = (VideoState*)arg;
  state->toggle_playback = true;
}

void toggle_fastforward(void* arg){
  VideoState* state = (VideoState*)arg;
  state->toggle_fastforward = true;
}

void toggle_rewind(void* arg){
  VideoState* state = (VideoState*)arg;
  state->toggle_rewind = true;
}
