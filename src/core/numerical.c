#include <math.h>
#include <stdlib.h>

#include "core/numerical.h"

typedef struct {
    UnaryFunc f;
    void *f_ctx;
    UnaryFunc g;
    void *g_ctx;
} DiffCtx;

static double diff_func(double x, void *ctx) {
    DiffCtx *d = (DiffCtx *)ctx;
    return d->f(x, d->f_ctx) - d->g(x, d->g_ctx);
}

static double derivative(UnaryFunc f, void *ctx, double x) {
    const double h = 1e-6;
    return (f(x + h, ctx) - f(x - h, ctx)) / (2.0 * h);
}

double find_root(UnaryFunc f, void *ctx, double a, double b, double eps, int max_iter, int *ok) {
    double fa = f(a, ctx);
    double fb = f(b, ctx);
    int i;

    *ok = 0;
    if (!isfinite(fa) || !isfinite(fb) || fa * fb > 0.0) {
        return 0.0;
    }

    for (i = 0; i < max_iter; i++) {
        double m = 0.5 * (a + b);
        double fm = f(m, ctx);
        if (!isfinite(fm)) {
            return 0.0;
        }
        if (fabs(fm) < eps || fabs(b - a) < eps) {
            *ok = 1;
            return m;
        }
        if (fa * fm <= 0.0) {
            b = m;
            fb = fm;
        } else {
            a = m;
            fa = fm;
        }
    }

    {
        double x = 0.5 * (a + b);
        for (i = 0; i < max_iter; i++) {
            double fx = f(x, ctx);
            double dfx = derivative(f, ctx, x);
            if (!isfinite(fx) || !isfinite(dfx) || fabs(dfx) < 1e-12) {
                break;
            }
            if (fabs(fx) < eps) {
                *ok = 1;
                return x;
            }
            x -= fx / dfx;
            if (x < a || x > b) {
                break;
            }
        }
    }

    return 0.0;
}

Point2D *find_intersections(UnaryFunc f, void *f_ctx,
                            UnaryFunc g, void *g_ctx,
                            double x_min, double x_max,
                            int scan_steps,
                            int *count) {
    DiffCtx dctx;
    Point2D *out;
    int out_count = 0;
    int i;

    if (scan_steps < 8) {
        scan_steps = 8;
    }

    out = (Point2D *)calloc((size_t)scan_steps, sizeof(Point2D));
    if (!out) {
        *count = 0;
        return NULL;
    }

    dctx.f = f;
    dctx.f_ctx = f_ctx;
    dctx.g = g;
    dctx.g_ctx = g_ctx;

    for (i = 0; i < scan_steps; i++) {
        double a = x_min + (x_max - x_min) * i / scan_steps;
        double b = x_min + (x_max - x_min) * (i + 1) / scan_steps;
        double fa = diff_func(a, &dctx);
        double fb = diff_func(b, &dctx);
        if (!isfinite(fa) || !isfinite(fb)) {
            continue;
        }
        if (fa == 0.0 || fb == 0.0 || fa * fb < 0.0) {
            int ok = 0;
            double x = find_root(diff_func, &dctx, a, b, 1e-6, 40, &ok);
            if (ok) {
                double y = f(x, f_ctx);
                int duplicated = 0;
                int k;
                for (k = 0; k < out_count; k++) {
                    if (fabs(out[k].x - x) < 1e-3) {
                        duplicated = 1;
                        break;
                    }
                }
                if (!duplicated && out_count < scan_steps) {
                    out[out_count].x = x;
                    out[out_count].y = y;
                    out_count++;
                }
            }
        }
    }

    *count = out_count;
    return out;
}
