#include "include/gui_manager.h"
#include <assert.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/_pthread/_pthread_mutex_t.h>
#include <time.h>

#ifndef VIDEO_DATA_H
#include "include/pipeline.h"
#endif

#ifndef STATE_H
#include "include/commands.h"
#endif
#ifndef UTILS_H
#include "include/utils.h"
#endif
#include "include/gui_manager.h"

#include "raylib.h"
#include <gst/app/gstappsink.h>
#include <gst/gst.h>
#include <gst/video/video.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

#include <Python.h>

int WINDOW_HEIGHT;
int WINDOW_WIDTH;

void destroy_stream(RaylibVideo **pstr) {
  RaylibVideo *str = *pstr;
  UnloadTexture(str->frame_texture);
  pthread_mutex_destroy(&str->frame_mut);

  UserData *ud = str->thread_data;
  g_signal_handlers_disconnect_by_func(ud->appsink, G_CALLBACK(on_new_sample),
                                       ud);
  free(ud);

  if (pstr != NULL)
    free(str);
  *pstr = NULL;
}

void init_empty_texture(RaylibVideo *stream) {
  pthread_mutex_init(&stream->frame_mut, NULL);
  Image img = GenImageColor(stream->width, stream->height, (Color){0, 0, 0, 0});

  Texture2D texture = LoadTextureFromImage(img);

  texture.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8;

  UnloadImage(img);

  stream->frame_texture = texture;
}

/*
Get duration, width, and height of a video using ffmpeg
https://gist.github.com/tejainece/6f7f5d5fec677eeedd15
*/
void load_video_metadata(RaylibVideo *stream) {
  AVFormatContext *pFormatCtx = NULL;
  AVCodecParameters *pCodecPar = NULL;
  int video_stream = -1;

  char *file_pass = malloc(sizeof(char) * 100);
  sprintf(file_pass, "file:%s", stream->file_path);

  int ret = avformat_open_input(&pFormatCtx, file_pass, NULL, NULL);
  assert(ret >= 0);
  if (file_pass != NULL)
    free(file_pass);

  ret = avformat_find_stream_info(pFormatCtx, NULL);
  assert(ret >= 0);

  for (int i = 0; i < pFormatCtx->nb_streams; i++) {
    if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
      video_stream = i;
      break;
    }
  }
  assert(video_stream != -1);

  pCodecPar = pFormatCtx->streams[video_stream]->codecpar;

  stream->duration = pFormatCtx->duration / (AV_TIME_BASE / 1000);

  // clamp width and height to screen size
  stream->width = pCodecPar->width;
  stream->height = pCodecPar->height;

  avformat_close_input(&pFormatCtx);
}

RaylibVideo *load_video(char *file_path) {
  RaylibVideo *raylib_stream = malloc(sizeof(RaylibVideo));
  raylib_stream->file_path = file_path;

  load_video_metadata(raylib_stream);

  return raylib_stream;
}

void read_frame_data(SharedFrame *sf, uint8_t *dst, size_t size) {
  pthread_mutex_lock(&sf->mutex);
  memcpy(dst, sf->data, size);
  pthread_mutex_unlock(&sf->mutex);
}

void destroy_shared_frame(SharedFrame *sf) {
  pthread_mutex_destroy(&sf->mutex);
  if (sf->data != NULL)
    free(sf->data);
  sf->data = NULL;
}

void scale_frame(uint8_t *src, uint8_t *dest, size_t size) {
  GstVideoScaler *h_scale =
      gst_video_scaler_new(GST_VIDEO_RESAMPLER_METHOD_NEAREST,
                           GST_VIDEO_SCALER_FLAG_NONE, 0, 1, 1, NULL);
  GstVideoScaler *v_scale =
      gst_video_scaler_new(GST_VIDEO_RESAMPLER_METHOD_NEAREST,
                           GST_VIDEO_SCALER_FLAG_NONE, 0, 1, 1, NULL);

  gst_video_scaler_2d(h_scale, v_scale, GST_VIDEO_FORMAT_RGB, src, 1, dest, 1,
                      0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
}

void destroy_user_data(UserData *ud) {
  g_signal_handlers_disconnect_by_func(ud->appsink, G_CALLBACK(on_new_sample),
                                       ud);
  free(ud);
}

void update_raylib(RaylibVideo *stream, UserData *ud) {
  atomic_bool latest_hash =
      atomic_load_explicit(&ud->dirty, memory_order_relaxed);

  if (!latest_hash)
    return;

  atomic_store_explicit(&ud->dirty, false, memory_order_relaxed);
  pthread_mutex_lock(&stream->frame_mut);
  UpdateTexture(stream->frame_texture, ud->frame->data);
  pthread_mutex_unlock(&stream->frame_mut);
}

void check_state(VideoState *state, RaylibVideo *str, GuiManager *gui) {
  if (state->command_bar_open) {
    draw_command_prompt();
  }

  // PAUSING
  if (state->toggle_playback) {
    GstState pause_or_play = str->paused ? GST_STATE_PLAYING : GST_STATE_PAUSED;

    GstStateChangeReturn r = gst_element_set_state(
        GST_ELEMENT(str->thread_data->pipeline), pause_or_play);
    if (r == GST_STATE_CHANGE_FAILURE) {
      printf("Error toggling playback\n");
    }

    GstClockTime timeout = 5 * GST_SECOND;
    r = gst_element_get_state(GST_ELEMENT(str->thread_data->pipeline), NULL,
                              NULL, timeout);
    state->toggle_playback = false;
    str->paused = !(str->paused);
    // gui->pause->is_active = str->paused;
    gui->pause->toggle = true;
  }

  if (state->toggle_fastforward || state->toggle_rewind) {
    GstClockTime jump = 5 * GST_SECOND;
    gui->fast_forward->toggle = true;
    int *packet_jump = (int *)gui->packet;
    if (packet_jump == NULL) {
      packet_jump = 0;
    }

    if (state->toggle_rewind)
      jump *= -1;

    if (state->toggle_fastforward)
      *packet_jump += 5;
    else {
      *packet_jump -= 5;
    }

    gint64 pos;
    gst_element_query_position(GST_ELEMENT(str->thread_data->pipeline),
                               GST_FORMAT_TIME, &pos);
    if (!gst_element_seek_simple(GST_ELEMENT(str->thread_data->pipeline),
                                 GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH,
                                 (pos + jump))) {
      printf("seek failed\n");
    }

    state->toggle_fastforward = false;
    state->toggle_rewind = false;
  }
  //
  // if (str->paused) {
  //   draw_pause();
  // }
}

// handles window resizing
Rectangle get_video_box(RaylibVideo *str) {
  int faux_height = str->height;
  int faux_width = str->width;

  faux_height = WINDOW_HEIGHT;
  faux_width = WINDOW_WIDTH;

  Rectangle new_res = (Rectangle){0, 0, faux_width, faux_height};

  return new_res;
}

void playback_driver(RaylibVideo *str, VideoState *state_machine,
                     PyObject *event_emitter) {
  UserData *ud = str->thread_data;

  char command[MAX_INPUT_CHARS + 1] = ":";
  int letter_count = 1;

  Rectangle video_box = get_video_box(str);

  GuiManager *gui_manager = create_gui_manager();
  while (!WindowShouldClose()) {
    update_raylib(str, ud);

    if (IsWindowResized()) {
      WINDOW_HEIGHT = GetScreenHeight();
      WINDOW_WIDTH = GetScreenWidth();
      video_box = get_video_box(str);
    }

    BeginDrawing();
    ClearBackground(BLACK);

    DrawTexturePro(str->frame_texture,
                   (Rectangle){0, 0, str->width, str->height}, video_box,
                   (Vector2){0, 0}, 0.0, WHITE);
    check_state(state_machine, str, gui_manager);
    manage_gui(gui_manager);
    if (state_machine->command_bar_open == true) {
      char c = GetCharPressed();
      handle_write(state_machine, command, &letter_count, c, event_emitter);
    }
    EndDrawing();
  }

  CloseWindow();
}

// int main(int argc, char* argv[]){

//   assert(argc > 1);

//   gst_init(NULL, NULL);

//   char* media_path = argv[1];
//   RaylibVideo* stream = load_video(media_path);

//   SetConfigFlags(FLAG_WINDOW_RESIZABLE);
//   InitWindow(stream->width, stream->height, "YouTube Looper");
//   WINDOW_HEIGHT = GetScreenHeight();
//   WINDOW_WIDTH = GetScreenWidth();
//   SetTargetFPS(60);
//   SetExitKey(KEY_NULL);

//   init_empty_texture(stream);
//   create_gstreamer_pipeline(stream);

//   playback_driver(stream);

//   destroy_stream(&stream);
//   printf("Done exiting\n");
// }

// void video_start(int argc, char* argv[]){
//   assert(argc > 1);

//   gst_init(NULL, NULL);

//   char* media_path = argv[1];
//   RaylibVideo* stream = load_video(media_path);

//   SetConfigFlags(FLAG_WINDOW_RESIZABLE);
//   InitWindow(stream->width, stream->height, "YouTube Looper");
//   WINDOW_HEIGHT = GetScreenHeight();
//   WINDOW_WIDTH = GetScreenWidth();
//   SetTargetFPS(60);
//   SetExitKey(KEY_NULL);

//   init_empty_texture(stream);
//   create_gstreamer_pipeline(stream);

//   playback_driver(stream);

//   destroy_stream(&stream);
//   printf("Done exiting\n");
// }
