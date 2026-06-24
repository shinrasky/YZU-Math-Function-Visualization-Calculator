#include "render/view.h"

void view2d_init(View2D *view, int screen_width, int screen_height) {
    view->center_x = 0.0;
    view->center_y = 0.0;
    view->scale = 60.0;
    view->screen_width = screen_width;
    view->screen_height = screen_height;
}

void view2d_resize(View2D *view, int screen_width, int screen_height) {
    view->screen_width = screen_width;
    view->screen_height = screen_height;
}

Point2D world_to_screen(const View2D *view, double wx, double wy) {
    Point2D p;
    p.x = (wx - view->center_x) * view->scale + view->screen_width * 0.5;
    p.y = view->screen_height * 0.5 - (wy - view->center_y) * view->scale;
    return p;
}

Point2D screen_to_world(const View2D *view, int sx, int sy) {
    Point2D p;
    p.x = view->center_x + (sx - view->screen_width * 0.5) / view->scale;
    p.y = view->center_y + (view->screen_height * 0.5 - sy) / view->scale;
    return p;
}
