#include <stdio.h>

#ifndef VIDEO_DATA_H
#include "video_data.h"
#endif

#ifndef STATE_H
#include "state.h"
#endif


typedef void (*ActionHandler)(void*);

typedef struct KeyBind{
  ActionHandler vals[UCHAR_MAX];
}KeyBind;

void open_command_prompt(void* arg);

void cancel(void* arg);

void toggle_playback(void* arg);

void toggle_fastforward(void* arg);
void toggle_rewind(void* arg);
