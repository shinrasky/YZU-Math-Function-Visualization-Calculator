#ifndef INPUT_H
#define INPUT_H

#include <stdbool.h>
#include "raylib.h"

bool input_textbox(char *buffer, int max_len, bool active, int *cursor_pos);
int input_cursor_from_mouse(const char *buffer,
							int mouse_x,
							int text_start_x,
							Font font,
							float font_size,
							float spacing);

#endif
