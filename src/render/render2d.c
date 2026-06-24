#include <math.h>
#include <stdio.h>
#include <string.h>

#include "raylib.h"

#include "core/coord_sys.h"
#include "render/render2d.h"

static double nice_step(double world_per_100px) {
    double base = pow(10.0, floor(log10(world_per_100px)));
    double n = world_per_100px / base;
    if (n < 1.5) return 1.0 * base;
    if (n < 3.5) return 2.0 * base;
    if (n < 7.5) return 5.0 * base;
    return 10.0 * base;
}

void render2d_grid(const View2D *view, int offset_x, int offset_y, int width, int height) {
    double world_span_x = width / view->scale;
    double step = nice_step(world_span_x / 10.0);
    Point2D top_left = screen_to_world(view, 0, 0);
    Point2D bottom_right = screen_to_world(view, width, height);

    double x_start = floor(top_left.x / step) * step;
    double y_start = floor(bottom_right.y / step) * step;
    double x;
    double y;

    for (x = x_start; x <= bottom_right.x + step; x += step) {
        Point2D s = world_to_screen(view, x, 0.0);
        DrawLine((int)(offset_x + s.x), offset_y, (int)(offset_x + s.x), offset_y + height, (Color){225, 225, 225, 255});
    }

    for (y = y_start; y <= top_left.y + step; y += step) {
        Point2D s = world_to_screen(view, 0.0, y);
        DrawLine(offset_x, (int)(offset_y + s.y), offset_x + width, (int)(offset_y + s.y), (Color){225, 225, 225, 255});
    }
}

void render2d_axes(const View2D *view, int offset_x, int offset_y, int width, int height) {
    Point2D origin = world_to_screen(view, 0.0, 0.0);
    DrawLine(offset_x, (int)(offset_y + origin.y), offset_x + width, (int)(offset_y + origin.y), BLACK);
    DrawLine((int)(offset_x + origin.x), offset_y, (int)(offset_x + origin.x), offset_y + height, BLACK);

    DrawText("x", offset_x + width - 16, (int)(offset_y + origin.y + 6), 16, BLACK);
    DrawText("y", (int)(offset_x + origin.x + 8), offset_y + 6, 16, BLACK);
}

void render2d_function(const View2D *view,
                       int offset_x,
                       int offset_y,
                       int width,
                       int height,
                       const ExprNode *expr,
                       Color color,
                       bool is_polar) {
    int i;
    bool has_prev = false;
    Point2D prev = {0};

    for (i = 0; i < width; i++) {
        bool ok = true;
        Variable vars[1];
        Point2D world = screen_to_world(view, i, height / 2);
        double x = world.x;
        double y;
        Point2D curr;

        if (!is_polar) {
            strcpy(vars[0].name, "x");
            vars[0].value = x;
            y = expr_eval(expr, vars, 1, &ok);
            if (!ok || !isfinite(y)) {
                has_prev = false;
                continue;
            }
            curr = world_to_screen(view, x, y);
        } else {
            double theta = -3.141592653589793 + (6.283185307179586 * i / (width - 1));
            strcpy(vars[0].name, "theta");
            vars[0].value = theta;
            y = expr_eval(expr, vars, 1, &ok);
            if (!ok || !isfinite(y)) {
                has_prev = false;
                continue;
            }
            {
                Point2D p = polar_to_cartesian(y, theta);
                curr = world_to_screen(view, p.x, p.y);
            }
        }

        if (curr.x < -2000 || curr.x > width + 2000 || curr.y < -2000 || curr.y > height + 2000) {
            has_prev = false;
            continue;
        }

        if (has_prev) {
            DrawLine((int)(offset_x + prev.x),
                     (int)(offset_y + prev.y),
                     (int)(offset_x + curr.x),
                     (int)(offset_y + curr.y),
                     color);
        }
        prev = curr;
        has_prev = true;
    }
}
