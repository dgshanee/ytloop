#include "../include/assets.h"
#include <sys/mman.h>

void* create_shared_memory(){
  VideoState* state = create_video_state();  

  int protection = PROT_READ | PROT_WRITE;
  int visibility = MAP_SHARED | MAP_ANONYMOUS;

  void* shmem = mmap(state, sizeof(VideoState), protection, visibility, -1, 0);

  return shmem;
}

void video_start(int argc, char* argv[], void* shmem){
  assert(argc > 1);
  assert(shmem != NULL);

  VideoState* state = (VideoState*)shmem;
  
  gst_init(NULL, NULL);

  char* media_path = argv[1];  
  RaylibVideo* stream = load_video(media_path);

  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(stream->width, stream->height, "YouTube Looper");

  SetTargetFPS(60);
  SetExitKey(KEY_NULL);

  init_empty_texture(stream);
  create_gstreamer_pipeline(stream);

  playback_driver(stream, state);

  destroy_stream(&stream);
  printf("Done exiting\n");
}
