#ifndef LIST_H
#define LIST_H

#include "core/function.h"

typedef struct {
    MathFunction *items;
    int size;
    int capacity;
} FunctionList;

void list_init(FunctionList *list);
void list_free(FunctionList *list);
int list_push(FunctionList *list, MathFunction fn);
void list_remove_at(FunctionList *list, int index);

#endif
