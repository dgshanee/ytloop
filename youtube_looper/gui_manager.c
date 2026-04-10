#include "include/gui_manager.h"
#include "include/gui_objects.h"
#include <stdio.h>

GuiManager *create_gui_manager() {
  GuiManager *res = malloc(sizeof(GuiManager));

  AssetTicker *fast_forward = malloc(sizeof(AssetTicker));
  AssetTicker *rewind = malloc(sizeof(AssetTicker));
  AssetTicker *pause = malloc(sizeof(AssetTicker));

  // {toggle, is_active, ticker_dependent, ticker_val, max_duration,
  // draw_function}
  *fast_forward =
      (AssetTicker){false, false, true, 0, FFW_MAX_DURATION, draw_fast_forward};
  *rewind = (AssetTicker){false, false, true, 0, RW_MAX_DURATION};
  *pause =
      (AssetTicker){false, false, false, 0, PAUSED_MAX_DURATION, draw_pause};

  res->fast_forward = fast_forward;
  res->rewind = rewind;
  res->pause = pause;
  res->packet = malloc(sizeof(void));

  return res;
}

void destroy_gui_manager(GuiManager *s) { free(s); }

void manage_asset(AssetTicker *ticker, void **packet) {
  if (ticker->toggle) {
    if (ticker->ticker_dependent) {
      ticker->is_active = true;
    } else {
      ticker->is_active = !(ticker->is_active);
    }
    ticker->ticker_val = 0;
    ticker->toggle = false;
  }

  if (ticker->is_active) {
    // draw asset -- if packet is not NULL, pass packet through
    if (packet == NULL) {
      ticker->draw_function(NULL);
    } else {
      ticker->draw_function(*packet);
    }

    if (ticker->ticker_dependent) {
      if (ticker->ticker_val > ticker->max_duration) {
        ticker->is_active = false;
        ticker->ticker_val = 0;
        // *packet = NULL;
        free(*packet);
        *packet = malloc(sizeof(void));
        printf("DONE\n\n");
      }
      ticker->ticker_val++;
    } else {
      return;
    }
  }
}

void manage_gui(GuiManager *gui_manager) {
  manage_asset(gui_manager->fast_forward, &gui_manager->packet);
  // manage_asset(gui_manager->rewind, NULL);
  manage_asset(gui_manager->pause, NULL);
}
