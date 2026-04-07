#include <assert.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/_pthread/_pthread_mutex_t.h>
#include <time.h>
#include <inttypes.h>

#ifndef VIDEO_DATA_H
#include "video_data.h"
#endif

#ifndef STATE_H
#include "state.h"
#endif
#ifndef UTILS_H
#include "utils.h"
#endif
#include "keybinds.h"

#include "raylib.h"
#include <gst/gst.h>
#include <gst/app/gstappsink.h>
#include <gst/video/video.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>

#define WINDOW_HEIGHT 0
#define WINDOW_WIDTH 0

RaylibVideo* load_video(char* file_path);
void init_empty_texture(RaylibVideo* stream);
void create_gstreamer_pipeline(RaylibVideo* stream);
void playback_driver(RaylibVideo* str, VideoState* state_machine);
void destroy_stream(RaylibVideo **pstr);
