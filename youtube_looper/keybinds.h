#include <limits.h>
#include <stdlib.h>
#ifndef VIDEO_DATA_H
#include "video_data.h"
#endif

#ifndef STATE_H
#include "state.h"
#endif

typedef void (*ActionHandler)(VideoState*);

typedef struct KeyBind{
  ActionHandler vals[UCHAR_MAX];
}KeyBind;

void keybind_create(KeyBind** kb);

void keybind_get(KeyBind* kb, char key, VideoState* state);
