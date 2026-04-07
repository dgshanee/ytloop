#include <stdio.h>
#ifndef VIDEO_DATA_H
#include "video_data.h"
#endif

#ifndef STATE_H
#include "state.h"
#endif
void open_command_prompt(VideoState* arg);

void cancel(VideoState* arg);

void toggle_playback(VideoState* arg);

void toggle_fastforward(VideoState* arg);
void toggle_rewind(VideoState* arg);
