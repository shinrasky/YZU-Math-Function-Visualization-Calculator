#include "raylib.h"

#include "ui/widgets.h"

static Font g_ui_font;
static float g_ui_font_size = 18.0f;
static float g_ui_spacing = 1.0f;
static int g_font_inited = 0;

static void ensure_font(void) {
    if (!g_font_inited) {
        g_ui_font = GetFontDefault();
        g_font_inited = 1;
    }
}

void ui_set_font(Font font, float font_size, float spacing) {
    g_ui_font = font;
    g_ui_font_size = font_size;
    g_ui_spacing = spacing;
    g_font_inited = 1;
}

bool ui_button(Rectangle rect, const char *label, Color normal, Color hover) {
    Vector2 m = GetMousePosition();
    Vector2 text_size;
    Vector2 text_pos;
    bool hot = CheckCollisionPointRec(m, rect);

    ensure_font();
    DrawRectangleRec(rect, hot ? hover : normal);
    DrawRectangleLinesEx(rect, 1.0f, GRAY);
    text_size = MeasureTextEx(g_ui_font, label, g_ui_font_size, g_ui_spacing);
    text_pos = (Vector2){rect.x + (rect.width - text_size.x) * 0.5f, rect.y + (rect.height - text_size.y) * 0.5f};
    DrawTextEx(g_ui_font, label, text_pos, g_ui_font_size, g_ui_spacing, BLACK);
    return hot && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

void ui_labeled_text(const char *label, int x, int y, int font_size, Color color) {
    ensure_font();
    DrawTextEx(g_ui_font, label, (Vector2){(float)x, (float)y}, (float)font_size, g_ui_spacing, color);
}
