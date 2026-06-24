#ifndef WIDGETS_H
#define WIDGETS_H

#include <stdbool.h>
#include "raylib.h"

void ui_set_font(Font font, float font_size, float spacing);
bool ui_button(Rectangle rect, const char *label, Color normal, Color hover);
void ui_labeled_text(const char *label, int x, int y, int font_size, Color color);

#endif
