#include <stdbool.h>
#include <stdlib.h>
#define STATE_H

typedef struct VideoState {
  bool command_bar_open;
  bool toggle_playback;
  bool toggle_fastforward;
  bool toggle_rewind;
  char command_str[50];
} VideoState;

VideoState *create_video_state();
