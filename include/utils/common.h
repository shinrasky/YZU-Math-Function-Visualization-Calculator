#ifndef COMMON_H
#define COMMON_H

#include <stdbool.h>

#define MATH_EPSILON 1e-7
#define MAX_EXPR_LEN 256
#define MAX_NAME_LEN 64

typedef struct {
    double x;
    double y;
} Point2D;

typedef struct {
    double x;
    double y;
    double z;
} Point3D;

#endif
