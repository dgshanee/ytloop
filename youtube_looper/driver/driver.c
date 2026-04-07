#include "../include/assets.h"

void video_start(int argc, char* argv[]){

  assert(argc > 1);

  gst_init(NULL, NULL);

  char* media_path = argv[1];  
  RaylibVideo* stream = load_video(media_path);

  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(stream->width, stream->height, "YouTube Looper");
  // WINDOW_HEIGHT = GetScreenHeight();
  // WINDOW_WIDTH = GetScreenWidth();
  SetTargetFPS(60);
  SetExitKey(KEY_NULL);

  init_empty_texture(stream);
  create_gstreamer_pipeline(stream);

  playback_driver(stream);

  destroy_stream(&stream);
  printf("Done exiting\n");
}
