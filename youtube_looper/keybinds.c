#include "include/keybinds.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include "include/actions.h"

void keybind_create(KeyBind** kb){
  *kb = malloc(sizeof(KeyBind));
  char* config_path = getenv("CONFIG_DIR");
  printf("config path: %s\n", config_path);
  for(int i = 0; i < UCHAR_MAX; i++){
    (*kb)->vals[i] = NULL;
  }

  (*kb)->vals[(unsigned char)':'] = open_command_prompt;
  (*kb)->vals[113] = cancel; // 'q' key
  (*kb)->vals[(unsigned char)'p'] = toggle_playback;
  (*kb)->vals[(unsigned char)'k'] = toggle_fastforward;
  (*kb)->vals[(unsigned char)'j'] = toggle_rewind;
}

void keybind_get(KeyBind* kb, char key, VideoState* state){
  unsigned char idx = (unsigned char)key;
  if(kb->vals[idx] != 0){
    kb->vals[idx](state);
  }
}
