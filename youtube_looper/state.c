#ifndef STATE_H
#include "state.h"
#endif

VideoState* create_video_state(){
  VideoState* res = (VideoState*)malloc(sizeof(VideoState));

  res->command_bar_open = false;
  res->toggle_playback= false;
  res->toggle_fastforward = false;
  res->toggle_rewind = false;

  return res;
}

