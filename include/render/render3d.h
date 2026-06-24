#ifndef RENDER3D_H
#define RENDER3D_H

#include "raylib.h"
#include "core/expr_parser.h"

void render3d_reference_scene(void);
void render3d_surface_expr(const ExprNode *expr, float range, int steps, Color color);

#endif
