#ifndef GUI_MANAGER
#define GUI_MANAGER

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

// durations of assets in frames
#define FFW_MAX_DURATION 6
#define RW_MAX_DURATION 6
#define PAUSED_MAX_DURATION 0

typedef void (*DrawFunc)(void *packet);

typedef struct asset_ticker {
  bool is_active;
  bool ticker_dependent;
  int ticker_val;
  int max_duration;

  DrawFunc draw_function;
} AssetTicker;

typedef struct gui_manager {
  AssetTicker *fast_forward;
  AssetTicker *rewind;
  AssetTicker *pause;
} GuiManager;

GuiManager *create_gui_manager();

void destroy_gui_manager(GuiManager *);

void manage_gui(GuiManager *gui_manager);

#endif
