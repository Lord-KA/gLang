#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#define T size_t

typedef struct {
    T *data;
    size_t len;
    size_t capacity;
} gArr;

gArr *gArr_new(size_t cap)
{
    gArr *arr = (gArr*)calloc(1, sizeof(gArr));
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

gArr *gArr_delete(gArr *arr)
{
    assert(arr != NULL);
    free(arr->data);
    free(arr);
    return NULL;
}

int gArr_push(gArr *arr, T elem)        // returns 0 if OK, 1 otherwise
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
