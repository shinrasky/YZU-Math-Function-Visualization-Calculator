#ifndef NUMERICAL_H
#define NUMERICAL_H

#include "utils/common.h"

typedef double (*UnaryFunc)(double x, void *ctx);

double find_root(UnaryFunc f, void *ctx, double a, double b, double eps, int max_iter, int *ok);
Point2D *find_intersections(UnaryFunc f, void *f_ctx,
                            UnaryFunc g, void *g_ctx,
                            double x_min, double x_max,
                            int scan_steps,
                            int *count);

#endif
