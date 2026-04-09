#include <raylib.h>

#ifndef RAYLIB_OBJECTS
#define RAYLIB_OBJECTS

/*
FAST FORWARD
*/
void draw_fast_forward(Vector2 pos);

// returns the symbol as a scalar vector
Vector2 *get_fast_forward_symbol();

// returns the text that will be displayed (varies, e.g. +10, +20)
char *get_fast_forward_text();

/*
 * REWIND
 */
void draw_rewind(Vector2 pos);

Vector2 *get_rewind_symbol();

char *get_rewind_text();

#endif
