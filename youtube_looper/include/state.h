#include <stdlib.h>
#include <stdbool.h>
#define STATE_H

typedef struct VideoState{
  bool command_bar_open;
  bool toggle_playback; 
  bool toggle_fastforward;
  bool toggle_rewind;
}VideoState;

VideoState* create_video_state();
