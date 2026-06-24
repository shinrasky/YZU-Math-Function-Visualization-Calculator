#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "raylib.h"

#include "core/expr_parser.h"
#include "core/numerical.h"
#include "render/render2d.h"
#include "render/render3d.h"
#include "render/view.h"
#include "ui/input.h"
#include "ui/widgets.h"
#include "utils/list.h"

typedef struct {
    ExprNode *ast;
    int is_polar;
    int is_surface;
} ParsedExpr;

typedef struct {
    const ExprNode *ast;
    const char *var_name;
} EvalCtx;

static float g_ui_font_size = 22.0f;
static float g_ui_spacing = 1.0f;

static void draw_text_ui(Font font, const char *text, float x, float y, float font_size, Color color) {
    DrawTextEx(font, text, (Vector2){x, y}, font_size, g_ui_spacing, color);
}

static float measure_text_ui(Font font, const char *text, float font_size) {
    return MeasureTextEx(font, text, font_size, g_ui_spacing).x;
}

static int append_char_buf(char *out, int max_len, int *pos, char c) {
    if (*pos >= max_len - 1) {
        return 0;
    }
    out[*pos] = c;
    (*pos)++;
    out[*pos] = '\0';
    return 1;
}

static int append_str_buf(char *out, int max_len, int *pos, const char *text) {
    int i;
    for (i = 0; text[i] != '\0'; i++) {
        if (!append_char_buf(out, max_len, pos, text[i])) {
            return 0;
        }
    }
    return 1;
}

static void skip_spaces_ltx(const char *s, int *idx) {
    while (s[*idx] != '\0' && isspace((unsigned char)s[*idx])) {
        (*idx)++;
    }
}

static int read_group_text(const char *s, int *idx, char open_ch, char close_ch, char *out, int out_max) {
    int depth = 0;
    int pos = 0;
    if (s[*idx] != open_ch) {
        return 0;
    }
    (*idx)++;
    depth = 1;
    while (s[*idx] != '\0' && depth > 0) {
        char c = s[*idx];
        (*idx)++;
        if (c == open_ch) {
            depth++;
        } else if (c == close_ch) {
            depth--;
            if (depth == 0) {
                break;
            }
        }
        if (depth > 0) {
            if (pos >= out_max - 1) {
                return 0;
            }
            out[pos++] = c;
        }
    }
    if (depth != 0) {
        return 0;
    }
    out[pos] = '\0';
    return 1;
}

static int latex_is_function_name(const char *name) {
    return strcmp(name, "sin") == 0 || strcmp(name, "cos") == 0 || strcmp(name, "tan") == 0 ||
           strcmp(name, "asin") == 0 || strcmp(name, "acos") == 0 || strcmp(name, "atan") == 0 ||
           strcmp(name, "ln") == 0 || strcmp(name, "log") == 0 || strcmp(name, "exp") == 0 ||
           strcmp(name, "abs") == 0;
}

static int latex_normalize_expr(const char *in, char *out, int out_max, const char **err_msg) {
    int i = 0;
    int pos = 0;

    out[0] = '\0';
    while (in[i] != '\0') {
        skip_spaces_ltx(in, &i);
        if (in[i] == '\0') {
            break;
        }

        if (in[i] == '\\') {
            char cmd[32] = {0};
            int k = 0;
            char arg_raw[512] = {0};
            char arg_norm[1024] = {0};
            i++;
            while (isalpha((unsigned char)in[i]) && k < (int)sizeof(cmd) - 1) {
                cmd[k++] = (char)tolower((unsigned char)in[i]);
                i++;
            }

            if (strcmp(cmd, "left") == 0 || strcmp(cmd, "right") == 0) {
                if (in[i] != '\0') {
                    if (!append_char_buf(out, out_max, &pos, in[i])) {
                        *err_msg = "normalized expression too long";
                        return 0;
                    }
                    i++;
                }
                continue;
            }

            if (strcmp(cmd, "cdot") == 0 || strcmp(cmd, "times") == 0) {
                if (!append_char_buf(out, out_max, &pos, '*')) {
                    *err_msg = "normalized expression too long";
                    return 0;
                }
                continue;
            }

            if (strcmp(cmd, "frac") == 0) {
                char num_raw[512] = {0};
                char den_raw[512] = {0};
                char num_norm[1024] = {0};
                char den_norm[1024] = {0};
                skip_spaces_ltx(in, &i);
                if (!read_group_text(in, &i, '{', '}', num_raw, (int)sizeof(num_raw))) {
                    *err_msg = "invalid \\frac numerator";
                    return 0;
                }
                skip_spaces_ltx(in, &i);
                if (!read_group_text(in, &i, '{', '}', den_raw, (int)sizeof(den_raw))) {
                    *err_msg = "invalid \\frac denominator";
                    return 0;
                }
                if (!latex_normalize_expr(num_raw, num_norm, (int)sizeof(num_norm), err_msg)) {
                    return 0;
                }
                if (!latex_normalize_expr(den_raw, den_norm, (int)sizeof(den_norm), err_msg)) {
                    return 0;
                }
                if (!append_str_buf(out, out_max, &pos, "((")) {
                    *err_msg = "normalized expression too long";
                    return 0;
                }
                if (!append_str_buf(out, out_max, &pos, num_norm)) {
                    *err_msg = "normalized expression too long";
                    return 0;
                }
                if (!append_str_buf(out, out_max, &pos, ")/(")) {
                    *err_msg = "normalized expression too long";
                    return 0;
                }
                if (!append_str_buf(out, out_max, &pos, den_norm)) {
                    *err_msg = "normalized expression too long";
                    return 0;
                }
                if (!append_str_buf(out, out_max, &pos, "))")) {
                    *err_msg = "normalized expression too long";
                    return 0;
                }
                continue;
            }

            if (strcmp(cmd, "sqrt") == 0) {
                skip_spaces_ltx(in, &i);
                if (in[i] == '{') {
                    if (!read_group_text(in, &i, '{', '}', arg_raw, (int)sizeof(arg_raw))) {
                        *err_msg = "invalid \\sqrt argument";
                        return 0;
                    }
                    if (!latex_normalize_expr(arg_raw, arg_norm, (int)sizeof(arg_norm), err_msg)) {
                        return 0;
                    }
                } else {
                    *err_msg = "\\sqrt requires braces";
                    return 0;
                }
                if (!append_str_buf(out, out_max, &pos, "sqrt(")) {
                    *err_msg = "normalized expression too long";
                    return 0;
                }
                if (!append_str_buf(out, out_max, &pos, arg_norm)) {
                    *err_msg = "normalized expression too long";
                    return 0;
                }
                if (!append_char_buf(out, out_max, &pos, ')')) {
                    *err_msg = "normalized expression too long";
                    return 0;
                }
                continue;
            }

            if (strcmp(cmd, "pi") == 0 || strcmp(cmd, "theta") == 0 || strcmp(cmd, "phi") == 0) {
                if (!append_str_buf(out, out_max, &pos, cmd)) {
                    *err_msg = "normalized expression too long";
                    return 0;
                }
                continue;
            }

            if (latex_is_function_name(cmd)) {
                skip_spaces_ltx(in, &i);
                if (in[i] == '{') {
                    if (!read_group_text(in, &i, '{', '}', arg_raw, (int)sizeof(arg_raw))) {
                        *err_msg = "invalid function argument";
                        return 0;
                    }
                    if (!latex_normalize_expr(arg_raw, arg_norm, (int)sizeof(arg_norm), err_msg)) {
                        return 0;
                    }
                } else if (in[i] == '(') {
                    if (!read_group_text(in, &i, '(', ')', arg_raw, (int)sizeof(arg_raw))) {
                        *err_msg = "invalid function argument";
                        return 0;
                    }
                    if (!latex_normalize_expr(arg_raw, arg_norm, (int)sizeof(arg_norm), err_msg)) {
                        return 0;
                    }
                } else {
                    *err_msg = "function requires braces or parentheses";
                    return 0;
                }
                if (!append_str_buf(out, out_max, &pos, cmd) ||
                    !append_char_buf(out, out_max, &pos, '(') ||
                    !append_str_buf(out, out_max, &pos, arg_norm) ||
                    !append_char_buf(out, out_max, &pos, ')')) {
                    *err_msg = "normalized expression too long";
                    return 0;
                }
                continue;
            }

            if (!append_str_buf(out, out_max, &pos, cmd)) {
                *err_msg = "normalized expression too long";
                return 0;
            }
            continue;
        }

        if (in[i] == '{') {
            if (!append_char_buf(out, out_max, &pos, '(')) {
                *err_msg = "normalized expression too long";
                return 0;
            }
            i++;
            continue;
        }
        if (in[i] == '}') {
            if (!append_char_buf(out, out_max, &pos, ')')) {
                *err_msg = "normalized expression too long";
                return 0;
            }
            i++;
            continue;
        }

        if (!append_char_buf(out, out_max, &pos, in[i])) {
            *err_msg = "normalized expression too long";
            return 0;
        }
        i++;
    }
    *err_msg = NULL;
    return 1;
}

static double expr_unary_eval(double x, void *ctx) {
    EvalCtx *ec = (EvalCtx *)ctx;
    Variable v;
    bool ok = true;
    strcpy(v.name, ec->var_name);
    v.value = x;
    return expr_eval(ec->ast, &v, 1, &ok);
}

static void free_function_impl(MathFunction *fn) {
    ParsedExpr *pe = (ParsedExpr *)fn->impl;
    if (pe) {
        expr_free(pe->ast);
        free(pe);
        fn->impl = NULL;
    }
}

static Color random_soft_color(void) {
    int r = GetRandomValue(40, 220);
    int g = GetRandomValue(40, 220);
    int b = GetRandomValue(40, 220);
    return (Color){(unsigned char)r, (unsigned char)g, (unsigned char)b, 255};
}

static int try_add_function(FunctionList *list, const char *input, int next_id, const char **error_msg) {
    char expr[1024];
    char input_trim[512];
    int trim_pos = 0;
    int is_polar = 0;
    int is_surface = 0;
    const char *parse_error = NULL;
    ExprNode *ast = NULL;
    ParsedExpr *pe;
    MathFunction fn;
    int i;

    if (!input || input[0] == '\0') {
        *error_msg = "expression is empty";
        return 0;
    }

    for (i = 0; input[i] != '\0' && trim_pos < (int)sizeof(input_trim) - 1; i++) {
        if (!isspace((unsigned char)input[i])) {
            input_trim[trim_pos++] = input[i];
        }
    }
    input_trim[trim_pos] = '\0';

    if (trim_pos == 0) {
        *error_msg = "expression is empty";
        return 0;
    }

    strncpy(expr, input_trim, sizeof(expr) - 1);
    expr[sizeof(expr) - 1] = '\0';

    if (strncmp(expr, "y=", 2) == 0 || strncmp(expr, "Y=", 2) == 0) {
        memmove(expr, expr + 2, strlen(expr + 2) + 1);
        is_polar = 0;
        is_surface = 0;
    } else if (strncmp(expr, "r=", 2) == 0 || strncmp(expr, "R=", 2) == 0) {
        memmove(expr, expr + 2, strlen(expr + 2) + 1);
        is_polar = 1;
        is_surface = 0;
    } else if (strncmp(expr, "z=", 2) == 0 || strncmp(expr, "Z=", 2) == 0) {
        memmove(expr, expr + 2, strlen(expr + 2) + 1);
        is_polar = 0;
        is_surface = 1;
    }

    {
        char normalized[1024] = {0};
        const char *norm_err = NULL;
        if (!latex_normalize_expr(expr, normalized, (int)sizeof(normalized), &norm_err)) {
            *error_msg = norm_err ? norm_err : "invalid latex input";
            return 0;
        }
        strncpy(expr, normalized, sizeof(expr) - 1);
        expr[sizeof(expr) - 1] = '\0';
    }

    ast = expr_parse(expr, &parse_error);
    if (!ast) {
        *error_msg = parse_error ? parse_error : "parse failed";
        return 0;
    }

    pe = (ParsedExpr *)calloc(1, sizeof(ParsedExpr));
    if (!pe) {
        expr_free(ast);
        *error_msg = "out of memory";
        return 0;
    }

    pe->ast = ast;
    pe->is_polar = is_polar;
    pe->is_surface = is_surface;

    memset(&fn, 0, sizeof(fn));
    fn.id = next_id;
    snprintf(fn.name, sizeof(fn.name), "f%d", next_id);
    snprintf(fn.expression, sizeof(fn.expression), "%s", input_trim);
    fn.type = is_surface ? FUNC_SURFACE : (is_polar ? FUNC_POLAR : FUNC_EXPLICIT);
    fn.color = random_soft_color();
    fn.visible = true;
    fn.impl = pe;

    if (!list_push(list, fn)) {
        free_function_impl(&fn);
        *error_msg = "function list expansion failed";
        return 0;
    }

    *error_msg = NULL;
    return 1;
}

int main(void) {
    const int init_w = 1280;
    const int init_h = 800;

    FunctionList functions;
    View2D view;
    char input_expr[256] = "y=sin(x)";
    int next_id = 1;
    int mode_3d = 0;
    int input_active = 1;
    int input_cursor = (int)strlen(input_expr);
    char status_text[256] = "Type or LaTeX input (e.g. y=\\sin{x}, y=\\frac{1}{x}, z=\\sqrt{x^2+y^2}) and click Add";
    Camera3D camera = {0};
    Font ui_font = GetFontDefault();
    bool custom_font_loaded = false;

    list_init(&functions);

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    InitWindow(init_w, init_h, "Math Calculator - C + raylib");
    SetTargetFPS(60);

    if (FileExists("C:/Windows/Fonts/times.ttf")) {
        ui_font = LoadFontEx("C:/Windows/Fonts/times.ttf", 32, NULL, 0);
        if (ui_font.texture.id > 0) {
            custom_font_loaded = true;
            SetTextureFilter(ui_font.texture, TEXTURE_FILTER_POINT);
        } else {
            ui_font = GetFontDefault();
        }
    }
    ui_set_font(ui_font, g_ui_font_size, g_ui_spacing);

    view2d_init(&view, init_w - 320, init_h - 40);

    camera.position = (Vector3){6.0f, 6.0f, 6.0f};
    camera.target = (Vector3){0.0f, 0.0f, 0.0f};
    camera.up = (Vector3){0.0f, 1.0f, 0.0f};
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    while (!WindowShouldClose()) {
        int screen_w = GetScreenWidth();
        int screen_h = GetScreenHeight();
        int panel_w = 320;
        int graph_w = screen_w - panel_w;
        int graph_h = screen_h - 40;
        Rectangle panel = {0, 0, (float)panel_w, (float)screen_h};
        Rectangle graph = {(float)panel_w, 0, (float)graph_w, (float)graph_h};
        Vector2 mouse = GetMousePosition();

        view2d_resize(&view, graph_w, graph_h);

        if (IsKeyPressed(KEY_TAB)) {
            mode_3d = !mode_3d;
            snprintf(status_text, sizeof(status_text), "Switched to %s mode", mode_3d ? "3D" : "2D");
        }

        if (!mode_3d) {
            int mouse_in_graph = CheckCollisionPointRec(mouse, graph);
            if (mouse_in_graph && IsMouseButtonDown(MOUSE_BUTTON_LEFT) && !input_active) {
                Vector2 delta = GetMouseDelta();
                view.center_x -= delta.x / view.scale;
                view.center_y += delta.y / view.scale;
            }
            if (mouse_in_graph) {
                float wheel = GetMouseWheelMove();
                if (fabsf(wheel) > 0.001f) {
                    double old_scale = view.scale;
                    Point2D before = screen_to_world(&view, (int)(mouse.x - panel_w), (int)mouse.y);
                    view.scale *= (wheel > 0.0f) ? 1.1 : 0.9;
                    if (view.scale < 10.0) view.scale = 10.0;
                    if (view.scale > 500.0) view.scale = 500.0;
                    if (fabs(view.scale - old_scale) > 1e-9) {
                        Point2D after = screen_to_world(&view, (int)(mouse.x - panel_w), (int)mouse.y);
                        view.center_x += before.x - after.x;
                        view.center_y += before.y - after.y;
                    }
                }
            }
        } else {
            UpdateCamera(&camera, CAMERA_FREE);
            {
                float vy = 4.5f * GetFrameTime();
                if (IsKeyDown(KEY_SPACE)) {
                    camera.position.y += vy;
                    camera.target.y += vy;
                }
                if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) {
                    camera.position.y -= vy;
                    camera.target.y -= vy;
                }
            }
        }

        if (ui_button((Rectangle){16, 16, 120, 32}, mode_3d ? "Switch 2D" : "Switch 3D", (Color){220, 235, 255, 255}, (Color){200, 225, 255, 255})) {
            mode_3d = !mode_3d;
            snprintf(status_text, sizeof(status_text), "Switched to %s mode", mode_3d ? "3D" : "2D");
        }

        {
            Rectangle input_box = {16, 62, (float)(panel_w - 32), 42};
            if (CheckCollisionPointRec(mouse, input_box) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                input_active = 1;
                input_cursor = input_cursor_from_mouse(input_expr, (int)mouse.x, 22, ui_font, g_ui_font_size, g_ui_spacing);
            }
            if (!CheckCollisionPointRec(mouse, panel) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                input_active = 0;
            }

            input_textbox(input_expr, (int)sizeof(input_expr), input_active, &input_cursor);

            if (IsKeyPressed(KEY_ENTER) || ui_button((Rectangle){16, 112, 90, 30}, "Add", (Color){210, 245, 210, 255}, (Color){190, 235, 190, 255})) {
                const char *err = NULL;
                if (try_add_function(&functions, input_expr, next_id, &err)) {
                    next_id++;
                    snprintf(status_text, sizeof(status_text), "Added function: %s", input_expr);
                    input_active = 0;
                } else {
                    snprintf(status_text, sizeof(status_text), "Add failed: %s", err ? err : "unknown error");
                }
            }

            if (ui_button((Rectangle){116, 112, 90, 30}, "Clear", (Color){255, 225, 225, 255}, (Color){255, 205, 205, 255})) {
                int i;
                for (i = 0; i < functions.size; i++) {
                    free_function_impl(&functions.items[i]);
                }
                functions.size = 0;
                snprintf(status_text, sizeof(status_text), "Function list cleared");
            }
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawRectangleRec(panel, (Color){245, 247, 250, 255});
        DrawLine(panel_w, 0, panel_w, screen_h, (Color){210, 210, 210, 255});

        draw_text_ui(ui_font, "Function Panel", 16, 8, 24, DARKBLUE);
        draw_text_ui(ui_font, "Expression", 16, 40, 20, BLACK);

        {
            Rectangle input_box = {16, 62, (float)(panel_w - 32), 42};
            float input_text_y = input_box.y + (input_box.height - g_ui_font_size) * 0.5f - 2.0f;
            DrawRectangleRec(input_box, WHITE);
            DrawRectangleLinesEx(input_box, input_active ? 2.0f : 1.0f, input_active ? BLUE : GRAY);
            draw_text_ui(ui_font, input_expr, 22, input_text_y, g_ui_font_size, BLACK);
            if (input_active && ((GetTime() * 2.0) - floor(GetTime() * 2.0) < 0.5)) {
                char left[256];
                int safe_cursor = input_cursor;
                int len = (int)strlen(input_expr);
                float tw;
                if (safe_cursor < 0) safe_cursor = 0;
                if (safe_cursor > len) safe_cursor = len;
                memcpy(left, input_expr, (size_t)safe_cursor);
                left[safe_cursor] = '\0';
                tw = measure_text_ui(ui_font, left, g_ui_font_size);
                DrawLine((int)(24 + tw), (int)(input_box.y + 6), (int)(24 + tw), (int)(input_box.y + input_box.height - 6), BLACK);
            }
        }

        draw_text_ui(ui_font, "Function List", 16, 156, 22, BLACK);
        {
            int i;
            int y = 190;
            for (i = 0; i < functions.size; i++) {
                MathFunction *fn = &functions.items[i];
                ParsedExpr *pe = (ParsedExpr *)fn->impl;
                int row_h = 38;
                float row_w = panel_w - 32.0f;
                float btn2_x;
                float btn1_x;
                float formula_left;
                float button_y;

                btn2_x = 16.0f + row_w - 36.0f;
                btn1_x = btn2_x - 32.0f;
                formula_left = 46.0f;

                Rectangle row = {16, (float)y, row_w, (float)row_h};
                DrawRectangleRec(row, fn->visible ? (Color){255, 255, 255, 255} : (Color){235, 235, 235, 255});
                DrawRectangleLinesEx(row, 1.0f, (Color){220, 220, 220, 255});
                DrawRectangle((int)row.x + 4, (int)row.y + 6, 20, 20, fn->color);

                draw_text_ui(ui_font, fn->expression, row.x + formula_left, row.y + 7.0f, 16.0f, BLACK);

                button_y = row.y + (row.height - 24.0f) * 0.5f;

                if (ui_button((Rectangle){btn1_x, button_y, 28, 24}, fn->visible ? "V" : "H", (Color){230, 240, 255, 255}, (Color){210, 230, 255, 255})) {
                    fn->visible = !fn->visible;
                }
                if (ui_button((Rectangle){btn2_x, button_y, 28, 24}, "X", (Color){255, 235, 235, 255}, (Color){255, 210, 210, 255})) {
                    free_function_impl(fn);
                    list_remove_at(&functions, i);
                    i--;
                    continue;
                }

                if (pe && pe->is_polar) {
                    draw_text_ui(ui_font, "polar", row.x + 170.0f, row.y + 8.0f, 14, DARKGRAY);
                } else if (pe && pe->is_surface) {
                    draw_text_ui(ui_font, "surface", row.x + 162.0f, row.y + 8.0f, 14, DARKGRAY);
                }

                y += row_h + 4;
                if (y > screen_h - 120) {
                    break;
                }
            }
        }

        if (!mode_3d) {
            DrawRectangle((int)graph.x, (int)graph.y, graph_w, graph_h, WHITE);
            BeginScissorMode((int)graph.x, (int)graph.y, graph_w, graph_h);
            render2d_grid(&view, panel_w, 0, graph_w, graph_h);
            render2d_axes(&view, panel_w, 0, graph_w, graph_h);

            {
                int i;
                for (i = 0; i < functions.size; i++) {
                    MathFunction *fn = &functions.items[i];
                    ParsedExpr *pe = (ParsedExpr *)fn->impl;
                    if (!fn->visible || !pe || !pe->ast || pe->is_surface) {
                        continue;
                    }
                    render2d_function(&view, panel_w, 0, graph_w, graph_h, pe->ast, fn->color, pe->is_polar ? true : false);
                }
            }

            {
                MathFunction *fna = NULL;
                MathFunction *fnb = NULL;
                ParsedExpr *ea = NULL;
                ParsedExpr *eb = NULL;
                int i;
                for (i = 0; i < functions.size; i++) {
                    MathFunction *fn = &functions.items[i];
                    ParsedExpr *pe = (ParsedExpr *)fn->impl;
                    if (!fn->visible || !pe || pe->is_polar || pe->is_surface) {
                        continue;
                    }
                    if (!fna) {
                        fna = fn;
                        ea = pe;
                    } else if (!fnb) {
                        fnb = fn;
                        eb = pe;
                        break;
                    }
                }
                if (fna && fnb && ea && eb) {
                    EvalCtx ca = {ea->ast, "x"};
                    EvalCtx cb = {eb->ast, "x"};
                    int count = 0;
                    Point2D *pts = find_intersections(expr_unary_eval, &ca, expr_unary_eval, &cb,
                                                      view.center_x - graph_w / view.scale,
                                                      view.center_x + graph_w / view.scale,
                                                      128,
                                                      &count);
                    for (i = 0; i < count; i++) {
                        Point2D s = world_to_screen(&view, pts[i].x, pts[i].y);
                        DrawCircle((int)(panel_w + s.x), (int)s.y, 4.0f, RED);
                        {
                            char t[80];
                            snprintf(t, sizeof(t), "(%.2f, %.2f)", pts[i].x, pts[i].y);
                            draw_text_ui(ui_font, t, (float)(panel_w + s.x + 6), (float)(s.y - 6), 14, RED);
                        }
                    }
                    free(pts);
                }
            }

            EndScissorMode();
        } else {
            DrawRectangle((int)graph.x, (int)graph.y, graph_w, graph_h, (Color){248, 250, 252, 255});
            BeginScissorMode((int)graph.x, (int)graph.y, graph_w, graph_h);
            BeginMode3D(camera);
            render3d_reference_scene();
            {
                int i;
                int has_surface = 0;
                for (i = 0; i < functions.size; i++) {
                    MathFunction *fn = &functions.items[i];
                    ParsedExpr *pe = (ParsedExpr *)fn->impl;
                    if (!fn->visible || !pe || !pe->ast || !pe->is_surface) {
                        continue;
                    }
                    has_surface = 1;
                    render3d_surface_expr(pe->ast, 6.0f, 64, fn->color);
                }
                if (!has_surface) {
                    DrawCubeWires((Vector3){0.0f, 1.0f, 0.0f}, 2.0f, 2.0f, 2.0f, DARKGRAY);
                }
            }
            EndMode3D();
            EndScissorMode();
            draw_text_ui(ui_font, "3D mode: add z=f(x,y) to render surfaces", (float)(panel_w + 16), 12, 20, DARKBLUE);
        }

        DrawRectangle(0, screen_h - 40, screen_w, 40, (Color){243, 243, 243, 255});
        DrawLine(0, screen_h - 40, screen_w, screen_h - 40, (Color){210, 210, 210, 255});
        draw_text_ui(ui_font, status_text, 12, (float)(screen_h - 30), 18, BLACK);

        EndDrawing();
    }

    {
        int i;
        for (i = 0; i < functions.size; i++) {
            free_function_impl(&functions.items[i]);
        }
    }
    list_free(&functions);

    if (custom_font_loaded) {
        UnloadFont(ui_font);
    }

    CloseWindow();
    return 0;
}
