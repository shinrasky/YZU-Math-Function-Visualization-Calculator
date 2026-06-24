#include <stdlib.h>
#include <string.h>

#include "utils/list.h"

void list_init(FunctionList *list) {
    list->items = NULL;
    list->size = 0;
    list->capacity = 0;
}

void list_free(FunctionList *list) {
    free(list->items);
    list->items = NULL;
    list->size = 0;
    list->capacity = 0;
}

int list_push(FunctionList *list, MathFunction fn) {
    if (list->size >= list->capacity) {
        int new_cap = list->capacity == 0 ? 8 : list->capacity * 2;
        MathFunction *new_items = (MathFunction *)realloc(list->items, sizeof(MathFunction) * new_cap);
        if (!new_items) {
            return 0;
        }
        list->items = new_items;
        list->capacity = new_cap;
    }
    list->items[list->size++] = fn;
    return 1;
}

void list_remove_at(FunctionList *list, int index) {
    if (index < 0 || index >= list->size) {
        return;
    }
    if (index < list->size - 1) {
        memmove(&list->items[index], &list->items[index + 1], (size_t)(list->size - index - 1) * sizeof(MathFunction));
    }
    list->size--;
}
