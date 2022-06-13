#ifndef GARR_ONCE
#define GARR_ONCE
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include "gutils.h"
#endif // GARR_ONCE

typedef struct {
    T *data;
    size_t len;
    size_t capacity;
} GARR_GENERIC(gArr);

static GARR_GENERIC(gArr) *GARR_GENERIC(gArr_new)(size_t cap)
{
    GARR_GENERIC(gArr) *arr = (GARR_GENERIC(gArr)*)calloc(1, sizeof(GARR_GENERIC(gArr)));
    if (arr == NULL)
        return NULL;
    arr->data = (T*)calloc(cap, sizeof(T));
    if (arr->data == NULL) {
        free(arr);
        return NULL;
    }
    arr->len = 0;
    arr->capacity = cap;
    return arr;
}

static GARR_GENERIC(gArr) *GARR_GENERIC(gArr_delete)(GARR_GENERIC(gArr) *arr)
{
    assert(arr != NULL);
    free(arr->data);
    free(arr);
    return NULL;
}

static int GARR_GENERIC(gArr_push)(GARR_GENERIC(gArr) *arr, T elem)        // returns 0 if OK, 1 otherwise
{
    assert(arr != NULL);
    if (arr->len >= arr->capacity) {
        if (void *newData = realloc(arr->data, arr->capacity * 2 * sizeof(T)))
            arr->data = (T*)newData;
        else
            return 1;
        arr->capacity *= 2;
    }
    arr->data[arr->len] = elem;
    ++arr->len;
    return 0;
}
#undef T
