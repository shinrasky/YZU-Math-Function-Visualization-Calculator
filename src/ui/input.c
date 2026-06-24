#include <string.h>

#include "raylib.h"

#include "ui/input.h"

static void clamp_cursor(const char *buffer, int *cursor_pos) {
    int len = (int)strlen(buffer);
    if (*cursor_pos < 0) {
        *cursor_pos = 0;
    }
    if (*cursor_pos > len) {
        *cursor_pos = len;
    }
}

int input_cursor_from_mouse(const char *buffer,
                            int mouse_x,
                            int text_start_x,
                            Font font,
                            float font_size,
                            float spacing) {
    int len = (int)strlen(buffer);
    int i;
    for (i = 0; i <= len; i++) {
        char left[256];
        int w;
        int cx;
        if (i >= (int)sizeof(left)) {
            break;
        }
        memcpy(left, buffer, (size_t)i);
        left[i] = '\0';
        w = (int)MeasureTextEx(font, left, font_size, spacing).x;
        cx = text_start_x + w;
        if (mouse_x < cx) {
            return i;
        }
    }
    return len;
}

bool input_textbox(char *buffer, int max_len, bool active, int *cursor_pos) {
    int key;
    bool changed = false;
    static int backspace_was_down = 0;
    static double backspace_hold_start = 0.0;
    static double backspace_last_repeat = 0.0;
    static int left_was_down = 0;
    static int right_was_down = 0;
    static double nav_hold_start = 0.0;
    static double nav_last_repeat = 0.0;
    const double hold_delay = 0.35;
    const double repeat_interval = 0.05;
    double now = GetTime();

    if (!active || !cursor_pos) {
        backspace_was_down = 0;
        left_was_down = 0;
        right_was_down = 0;
        return false;
    }

    clamp_cursor(buffer, cursor_pos);

    while ((key = GetCharPressed()) > 0) {
        int len = (int)strlen(buffer);
        if (key >= 32 && key <= 126 && len < max_len - 1 && *cursor_pos >= 0 && *cursor_pos <= len) {
            memmove(buffer + *cursor_pos + 1, buffer + *cursor_pos, (size_t)(len - *cursor_pos + 1));
            buffer[*cursor_pos] = (char)key;
            (*cursor_pos)++;
            changed = true;
        }
    }

    if (IsKeyPressed(KEY_LEFT)) {
        if (*cursor_pos > 0) {
            (*cursor_pos)--;
        }
        nav_hold_start = now;
        nav_last_repeat = now;
        left_was_down = 1;
    }
    if (IsKeyPressed(KEY_RIGHT)) {
        int len = (int)strlen(buffer);
        if (*cursor_pos < len) {
            (*cursor_pos)++;
        }
        nav_hold_start = now;
        nav_last_repeat = now;
        right_was_down = 1;
    }

    if (!IsKeyDown(KEY_LEFT)) {
        left_was_down = 0;
    }
    if (!IsKeyDown(KEY_RIGHT)) {
        right_was_down = 0;
    }

    if ((left_was_down && IsKeyDown(KEY_LEFT)) || (right_was_down && IsKeyDown(KEY_RIGHT))) {
        if ((now - nav_hold_start) >= hold_delay && (now - nav_last_repeat) >= repeat_interval) {
            int len = (int)strlen(buffer);
            if (IsKeyDown(KEY_LEFT) && *cursor_pos > 0) {
                (*cursor_pos)--;
            }
            if (IsKeyDown(KEY_RIGHT) && *cursor_pos < len) {
                (*cursor_pos)++;
            }
            nav_last_repeat = now;
        }
    }

    if (IsKeyPressed(KEY_BACKSPACE)) {
        int len = (int)strlen(buffer);
        if (len > 0 && *cursor_pos > 0) {
            memmove(buffer + *cursor_pos - 1, buffer + *cursor_pos, (size_t)(len - *cursor_pos + 1));
            (*cursor_pos)--;
            changed = true;
        }
        backspace_hold_start = now;
        backspace_last_repeat = now;
        backspace_was_down = 1;
    }

    if (!IsKeyDown(KEY_BACKSPACE)) {
        backspace_was_down = 0;
    }

    if (backspace_was_down && IsKeyDown(KEY_BACKSPACE)) {
        if ((now - backspace_hold_start) >= hold_delay && (now - backspace_last_repeat) >= repeat_interval) {
            int len = (int)strlen(buffer);
            if (len > 0 && *cursor_pos > 0) {
                memmove(buffer + *cursor_pos - 1, buffer + *cursor_pos, (size_t)(len - *cursor_pos + 1));
                (*cursor_pos)--;
                changed = true;
            }
            backspace_last_repeat = now;
        }
    }

    clamp_cursor(buffer, cursor_pos);
    return changed;
}
