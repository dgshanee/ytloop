#ifndef COMMANDS_H
#define COMMANDS_H

#include "state.h"
#include "video_data.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#endif

typedef struct VideoState {
  bool command_bar_open;
  bool toggle_playback;
  bool toggle_fastforward;
  bool toggle_rewind;
  char command_str[50];
} VideoState;

typedef void (*ActionHandler)(void *);
typedef struct KeyBind {
  ActionHandler vals[UCHAR_MAX];
} KeyBind;

/*
 *  ---STATE---
 * */
VideoState *create_video_state();

/*
 *  ---ACTIONS---
 * */
void open_command_prompt(void *arg);
void cancel(void *arg);
void toggle_playback(void *arg);
void toggle_fastforward(void *arg);
void toggle_rewind(void *arg);
