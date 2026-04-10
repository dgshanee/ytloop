#include "gui_manager.h"
#include <raylib.h>

#ifndef RAYLIB_OBJECTS
#define RAYLIB_OBJECTS

/*
FAST FORWARD
*/
void draw_fast_forward(GuiManager gm, void *packet);

// returns the symbol as a scalar vector
Vector2 *get_fast_forward_symbol();

// returns the text that will be displayed (varies, e.g. +10, +20)
char *get_fast_forward_text();

/*
 * REWIND
 */
void draw_rewind(GuiManager gm, void *packet);

Vector2 *get_rewind_symbol();

char *get_rewind_text();

/*
 * PAUSE
 */

void draw_pause(GuiManager gm, void *packet);
#endif
