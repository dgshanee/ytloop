#include <assert.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/_pthread/_pthread_mutex_t.h>
#include <time.h>

#include "pipeline.h"

#include "commands.h"
#ifndef UTILS_H
#include "utils.h"
#endif

#include "raylib.h"
#include <Python.h>
#include <gst/app/gstappsink.h>
#include <gst/gst.h>
#include <gst/video/video.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

#define WINDOW_HEIGHT 0
#define WINDOW_WIDTH 0

RaylibVideo *load_video(char *file_path);
void init_empty_texture(RaylibVideo *stream);
void create_gstreamer_pipeline(RaylibVideo *stream);
void playback_driver(RaylibVideo *str, VideoState *state_machine,
                     PyObject *event_emitter);
void destroy_stream(RaylibVideo **pstr);
