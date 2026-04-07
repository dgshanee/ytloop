#include "include/actions.h"
#ifndef STATE_H
#define "state.h"
#endif
#include <stdio.h>

void open_command_prompt(VideoState* arg){
  arg->command_bar_open = true;
}

void cancel(VideoState* arg){
  if(arg->command_bar_open == true){
    arg->command_bar_open = false;
    return;
  }
}

void toggle_playback(VideoState* arg){
  arg->toggle_playback = true;
}

void toggle_fastforward(VideoState* arg){
  arg->toggle_fastforward = true;
}

void toggle_rewind(VideoState* arg){
  arg->toggle_rewind = true;
}
