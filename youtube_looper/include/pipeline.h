#ifndef PIPELINE_H
#define PIPELINE_H
#include "raylib.h"
#include <gst/app/gstappsink.h>
#include <gst/gst.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <stdatomic.h>
#include <sys/_pthread/_pthread_mutex_t.h>

#define MAX_INPUT_CHARS 16

/*
 *  -----DATA STRUCTS-----
 * */

/*
 *  ----SHARED FRAME----
 *  Shared frames are shared between multiple threads
 *  in the system, so the data needs to be read and written
 *  to with a mutex. The data is the frame data from the
 *  media file.
 * */
typedef struct {
  uint8_t *data;
  pthread_mutex_t mutex;
} SharedFrame;

typedef struct {
  GstPipeline *pipeline;
  gint64 timestamp_ref; // time stamp
  SharedFrame *frame;
  atomic_bool dirty;
  GstAppSink *appsink;
} PipelineContext;

typedef struct {
  // metadata
  char *file_path;

  int64_t duration;
  uint32_t width;
  uint32_t height;

  uint64_t timestamp_ms;
  bool paused;
  float rate;

  PipelineContext *thread_data;

  Texture2D frame_texture;
  pthread_mutex_t frame_mut;
} RaylibVideo;

/*
 *  -----GSTREAMER PIPELINE-----
 * */

void create_gstreamer_pipeline(RaylibVideo *stream);
PipelineContext *set_appsink_callback(RaylibVideo stream, GstPipeline *pipeline,
                                      GstAppSink *appsink);
void init_frame_mutex(SharedFrame *sf, int64_t video_height,
                      int64_t video_width);
GstFlowReturn on_new_sample(GstAppSink *appsink, gpointer user_data);
void write_frame_data(SharedFrame *sf, uint8_t *src, size_t size);

#endif
