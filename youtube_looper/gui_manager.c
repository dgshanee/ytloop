#include "include/gui_manager.h"

GuiManager *create_gui_manager() {
  GuiManager *res = malloc(sizeof(GuiManager));

  AssetTicker *fast_forward = malloc(sizeof(AssetTicker));
  AssetTicker *rewind = malloc(sizeof(AssetTicker));
  AssetTicker *pause = malloc(sizeof(AssetTicker));

  *fast_forward = (AssetTicker){false, true, 0, FFW_MAX_DURATION};
  *rewind = (AssetTicker){false, true, 0, RW_MAX_DURATION};
  *pause = (AssetTicker){false, false, 0, PAUSED_MAX_DURATION};

  return res;
}

void destroy_gui_manager(GuiManager *s) { free(s); }

void manage_asset(AssetTicker *ticker, void *packet) {
  if (ticker->is_active) {
    // draw asset -- if packet is not NULL, pass packet through

    if (ticker->ticker_dependent) {
      if (ticker->ticker_val > ticker->max_duration) {
        ticker->is_active = false;
        ticker->ticker_val = 0;
      }
    } else {
      return;
    }
  }
}

void manage_gui(GuiManager *gui_manager) {
  // manage_asset(gui_manager->fast_forward, NULL);
  // manage_asset(gui_manager->rewind, NULL);
  manage_asset(gui_manager->pause, NULL);
}
