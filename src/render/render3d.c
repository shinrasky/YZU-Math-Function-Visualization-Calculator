#include <math.h>
#include <string.h>

#include "raylib.h"

#include "core/expr_parser.h"
#include "render/render3d.h"

void render3d_reference_scene(void) {
    DrawGrid(20, 1.0f);
    DrawCubeWires((Vector3){0.0f, 1.0f, 0.0f}, 2.0f, 2.0f, 2.0f, DARKGRAY);
}

static int eval_surface_z(const ExprNode *expr, float x, float y, float *out_z) {
    Variable vars[2];
    bool ok = true;
    double z;

    strcpy(vars[0].name, "x");
    vars[0].value = x;
    strcpy(vars[1].name, "y");
    vars[1].value = y;

    z = expr_eval(expr, vars, 2, &ok);
    if (!ok || !isfinite(z)) {
        return 0;
    }

    *out_z = (float)z;
    return 1;
}

void render3d_surface_expr(const ExprNode *expr, float range, int steps, Color color) {
    int ix;
    int iy;

    if (!expr || steps < 4 || range <= 0.0f) {
        return;
    }

    for (ix = 0; ix < steps; ix++) {
        float x0 = -range + (2.0f * range * ix) / steps;
        float x1 = -range + (2.0f * range * (ix + 1)) / steps;
        for (iy = 0; iy < steps; iy++) {
            float y0 = -range + (2.0f * range * iy) / steps;
            float y1 = -range + (2.0f * range * (iy + 1)) / steps;
            float z00;
            float z10;
            float z01;

            if (eval_surface_z(expr, x0, y0, &z00) && eval_surface_z(expr, x1, y0, &z10)) {
                DrawLine3D((Vector3){x0, z00, y0}, (Vector3){x1, z10, y0}, color);
            }
            if (eval_surface_z(expr, x0, y0, &z00) && eval_surface_z(expr, x0, y1, &z01)) {
                DrawLine3D((Vector3){x0, z00, y0}, (Vector3){x0, z01, y1}, color);
            }
        }
    }
}
