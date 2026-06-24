#ifndef RENDER2D_H
#define RENDER2D_H

#include "core/expr_parser.h"
#include "core/function.h"
#include "render/view.h"

void render2d_grid(const View2D *view, int offset_x, int offset_y, int width, int height);
void render2d_axes(const View2D *view, int offset_x, int offset_y, int width, int height);
void render2d_function(const View2D *view,
                       int offset_x,
                       int offset_y,
                       int width,
                       int height,
                       const ExprNode *expr,
                       Color color,
                       bool is_polar);

#endif
