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
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>

void destroy_stream(RaylibVideo **pstr){
  RaylibVideo* str = *pstr;
  UnloadTexture(str->frame_texture);
  pthread_mutex_destroy(&str->frame_mut);
  
  if(pstr != NULL)
    free(str);
  *pstr = NULL;
}

void init_empty_texture(RaylibVideo* stream){
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
void load_video_metadata(RaylibVideo* stream){  
  AVFormatContext* pFormatCtx = NULL;
  AVCodecParameters* pCodecPar = NULL;
  int video_stream = -1;

  char* file_pass = malloc(sizeof(char) * 100);
  sprintf(file_pass, "file:%s", stream->file_path);

  int ret = avformat_open_input(&pFormatCtx, file_pass, NULL, NULL);
  assert(ret >= 0);
  if(file_pass != NULL)
    free(file_pass);

  ret = avformat_find_stream_info(pFormatCtx, NULL);
  assert(ret >= 0);

  for(int i = 0; i < pFormatCtx->nb_streams; i++){
    if(pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO){
      video_stream = i;
      break;
    }
  }
  assert(video_stream != -1);

  pCodecPar = pFormatCtx->streams[video_stream]->codecpar;

  stream->duration = pFormatCtx->duration / (AV_TIME_BASE / 1000);
  stream->width = pCodecPar->width;
  stream->height = pCodecPar->height;

  avformat_close_input(&pFormatCtx);
}


RaylibVideo* load_video(char* file_path){
  RaylibVideo* raylib_stream = malloc(sizeof(RaylibVideo));
  raylib_stream->file_path = file_path;

  load_video_metadata(raylib_stream);

  return raylib_stream;
}

void init_frame_mutex(SharedFrame* sf, int64_t video_height, int64_t video_width){
  sf->data = malloc(sizeof(uint8_t) * video_height * video_width * 3); // rgb
  assert(sf->data != NULL);

  pthread_mutex_init(&sf->mutex, NULL);
  assert(&sf->mutex != NULL);
}

void read_frame_data(SharedFrame* sf, uint8_t *dst, size_t size){
  pthread_mutex_lock(&sf->mutex);
  memcpy(dst, sf->data, size);
  pthread_mutex_unlock(&sf->mutex);
}

void write_frame_data(SharedFrame* sf, uint8_t* src, size_t size){
  pthread_mutex_lock(&sf->mutex);
  memcpy(sf->data, src, size);
  pthread_mutex_unlock(&sf->mutex);
}

void destroy_shared_frame(SharedFrame* sf){
  pthread_mutex_destroy(&sf->mutex);
  if(sf->data != NULL)
    free(sf->data);
  sf->data = NULL;
}

static GstFlowReturn on_new_sample(GstAppSink *appsink, gpointer user_data)
{
  UserData* ud = (UserData*)user_data;
  GstPipeline *pipeline = ud->pipeline;
  assert(pipeline);

  GstSample* sample = gst_app_sink_pull_sample(appsink);
  assert(sample);

  GstBuffer* buffer = gst_sample_get_buffer(sample);
  assert(buffer);

  GstMapInfo map;
  if (!gst_buffer_map(buffer, &map, GST_MAP_READ)) {
      return GST_FLOW_ERROR;
  }

  gint64 pos_ns = GST_CLOCK_TIME_NONE;

  if(gst_element_query_position(GST_ELEMENT(pipeline), 
                                GST_FORMAT_TIME, 
                                &pos_ns)){
    volatile gint ts = (gint)ud->ts_ref;
    g_atomic_int_set(&ts, pos_ns);
  }

  write_frame_data(ud->frame, map.data, map.size);
  atomic_store_explicit(&ud->dirty, true, memory_order_relaxed);

  gst_buffer_unmap(buffer, &map);
  gst_sample_unref(sample);
  
  return GST_FLOW_OK;
}


void destroy_user_data(UserData* ud){
  g_signal_handlers_disconnect_by_func(ud->appsink, G_CALLBACK(on_new_sample), ud);

  free(ud);
}


UserData* set_appsink_callback(RaylibVideo stream, GstPipeline* pipeline, GstAppSink* appsink){
  gst_app_sink_set_emit_signals(appsink, TRUE);
  
  GWeakRef weak_pipeline;
  g_weak_ref_init(&weak_pipeline, G_OBJECT(pipeline));
  assert(&weak_pipeline != NULL);

  volatile gint ts = 0;
  gint64 ts_ref = g_atomic_int_get(&ts);

  SharedFrame* sf = malloc(sizeof(SharedFrame));
  init_frame_mutex(sf, stream.height, stream.width);

  UserData* ud = malloc(sizeof(UserData));
  atomic_init(&ud->dirty, false);
  ud->frame = sf;
  ud->ts_ref = ts_ref;
  ud->pipeline = pipeline;
  ud->appsink = appsink;

  g_signal_connect(
    appsink,
    "new-sample",
    G_CALLBACK(on_new_sample),
    ud
  );

  return ud;
}

void update_raylib(RaylibVideo* stream, UserData* ud){
  atomic_bool latest_hash = atomic_load_explicit(&ud->dirty, memory_order_relaxed);

  if(!latest_hash) return;

  atomic_store_explicit(&ud->dirty, false, memory_order_relaxed);
  pthread_mutex_lock(&stream->frame_mut);
  UpdateTexture(stream->frame_texture, ud->frame->data);
  pthread_mutex_unlock(&stream->frame_mut);
}

UserData* create_gstreamer_pipeline(RaylibVideo stream){  
  char* call_str = "filesrc location=%s ! decodebin name=decode ! queue ! videoconvert"
    "! video/x-raw,format=RGB,width=%i,height=%i,colorimetry=sRGB ! appsink name=appsink sync=true decode." 
    "! queue ! audioconvert !volume volume=0.1 ! audioresample ! autoaudiosink";

  char* pipeline_str = malloc((sizeof(char) * strlen(call_str)) + (sizeof(char) * 100));
  sprintf(pipeline_str, call_str, stream.file_path, stream.width, stream.height);
  

  GError* err = NULL;
  GstElement* pipeline_ref = gst_parse_launch(pipeline_str, &err);
  assert(err == NULL);

  free(pipeline_str);

  GstPipeline* pipeline = GST_PIPELINE(pipeline_ref);

  GstElement* appsink_ref = gst_bin_get_by_name(GST_BIN(pipeline), "appsink");
  GstAppSink* appsink = GST_APP_SINK(appsink_ref);
  assert(appsink != NULL);

  gst_object_unref(appsink_ref);

  UserData* ud = set_appsink_callback(stream, pipeline, appsink);

  // play video
  GstStateChangeReturn ret;
  ret = gst_element_set_state(GST_ELEMENT(pipeline), GST_STATE_PLAYING);
  assert(ret != GST_STATE_CHANGE_FAILURE);

  // GstBus* bus = gst_element_get_bus(GST_ELEMENT(pipe));
  // GstMessage* msg;
  
  // while(1){
  //   msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE, GST_MESSAGE_STATE_CHANGED | GST_MESSAGE_ERROR | GST_MESSAGE_EOS);
  //   if(msg != NULL){
  //     GError* err;
  //     gchar* debug_info;
  //     switch(GST_MESSAGE_TYPE (msg)){
  //       case GST_MESSAGE_ERROR:
  //         gst_message_parse_error(msg, &err, &debug_info);
  //         g_printerr ("Error received from element %s: %s\n",
  //             GST_OBJECT_NAME (msg->src), err->message);
  //         g_printerr ("Debugging information: %s\n",
  //             debug_info ? debug_info : "none");
  //         g_clear_error (&err);
  //         g_free (debug_info);
  //       default:
  //         g_printerr("Unexpected message received\n");
  //     }

  //   }
  // }

  return ud;
}

void check_state(VideoState* state, RaylibVideo* str){
  if(state->command_bar_open){
    draw_command_prompt();
  }

  if(state->toggle_playback){    
    GstState pause_or_play = str->paused ? GST_STATE_PLAYING : GST_STATE_PAUSED;
    
    GstStateChangeReturn r = gst_element_set_state(GST_ELEMENT(str->thread_data->pipeline), pause_or_play);
    if(r == GST_STATE_CHANGE_FAILURE){
      printf("Error toggling playback\n");
    }
    if(r == GST_STATE_CHANGE_ASYNC){
      printf("Async\n");
    }

    GstClockTime timeout = 5 * GST_SECOND;
    r = gst_element_get_state(GST_ELEMENT(str->thread_data->pipeline), NULL, NULL, timeout);
    state->toggle_playback = false;
    str->paused = !(str->paused);
  }

  if(state->toggle_fastforward || state->toggle_rewind){
    GstClockTime jump = 5 * GST_SECOND;

    if(state->toggle_rewind) jump *= -1;
    
    gint64 pos;
    gst_element_query_position(GST_ELEMENT(str->thread_data->pipeline), GST_FORMAT_TIME, &pos);
    if(!gst_element_seek_simple(GST_ELEMENT(str->thread_data->pipeline), GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH, (pos + jump))){
      printf("seek failed\n");
    }

    state->toggle_fastforward = false;
    state->toggle_rewind = false;
  }

  if(str->paused){
    draw_pause();
  }
}

void playback_driver(RaylibVideo* str){
  UserData* ud = str->thread_data;
  VideoState* state_machine = create_video_state();

  KeyBind *kb;
  keybind_create(&kb);

  // commands
  char command[MAX_INPUT_CHARS + 1] = ":";
  int letter_count = 1;

  while(!WindowShouldClose()){
    update_raylib(str, ud);
    BeginDrawing();
      ClearBackground(RAYWHITE);

      DrawTexture(str->frame_texture, 0, 0, WHITE);
      check_state(state_machine, str);
      char c = GetCharPressed();
      if(state_machine->command_bar_open == false){
        keybind_get(kb, c, state_machine);
      }else{
        handle_write(state_machine, command, &letter_count, c);
      }
    EndDrawing();
  }

  CloseWindow();
}

int main(int argc, char* argv[]){
  assert(argc > 1);

  gst_init(NULL, NULL);

  char* media_path = argv[1];  
  RaylibVideo* stream = load_video(media_path);

  InitWindow(stream->width, stream->height, "YouTube Looper");
  SetTargetFPS(60);
  SetExitKey(KEY_NULL);

  init_empty_texture(stream);
  UserData* ud = create_gstreamer_pipeline(*stream);
  stream->thread_data = ud;

  // pthread_t* key_thread = set_keybinds();
  playback_driver(stream);

  // pthread_cancel(*key_thread);
  destroy_stream(&stream);
  destroy_user_data(ud);
  printf("Done exiting\n");
}
