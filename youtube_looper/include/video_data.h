#include <gst/gst.h>
#include <gst/app/gstappsink.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <stdatomic.h>
#include <sys/_pthread/_pthread_mutex_t.h>
#include "raylib.h"

#define VIDEO_DATA_H
#define MAX_INPUT_CHARS 16

typedef struct {
  uint8_t *data;
  pthread_mutex_t mutex;
} SharedFrame;

typedef struct{
  GstPipeline* pipeline;
  gint64 ts_ref; // time stamp
  SharedFrame* frame;
  atomic_bool dirty;
  GstAppSink* appsink;
}UserData;

typedef struct {
  char* file_path;
  
  int64_t duration;
  uint32_t width;
  uint32_t height;
  // metadata

  uint64_t timestamp_ms;
  bool paused;
  float rate;

  UserData* thread_data;

  Texture2D frame_texture;
  pthread_mutex_t frame_mut;
} RaylibVideo;

typedef struct MasterData{
  RaylibVideo* video_stream;
  UserData* user_data;
}MasterData;
