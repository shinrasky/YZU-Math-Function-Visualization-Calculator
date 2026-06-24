#ifndef FUNCTION_H
#define FUNCTION_H

#include <stdbool.h>
#include "raylib.h"
#include "utils/common.h"

typedef enum {
    FUNC_EXPLICIT,
    FUNC_IMPLICIT,
    FUNC_PARAMETRIC,
    FUNC_POLAR,
    FUNC_SURFACE,
    FUNC_SPACE_CURVE
} FuncType;

typedef struct {
    int id;
    char name[MAX_NAME_LEN];
    char expression[MAX_EXPR_LEN];
    FuncType type;
    Color color;
    bool visible;
    void *impl;
} MathFunction;

#endif
