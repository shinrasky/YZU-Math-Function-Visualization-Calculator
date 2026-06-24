#ifndef VIEW_H
#define VIEW_H

#include "utils/common.h"

typedef struct {
    double center_x;
    double center_y;
    double scale;
    int screen_width;
    int screen_height;
} View2D;

void view2d_init(View2D *view, int screen_width, int screen_height);
void view2d_resize(View2D *view, int screen_width, int screen_height);
Point2D world_to_screen(const View2D *view, double wx, double wy);
Point2D screen_to_world(const View2D *view, int sx, int sy);

#endif
