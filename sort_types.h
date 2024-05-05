#ifndef SORT_TYPES_H
#define SORT_TYPES_H

typedef enum { QSORT, TIMSORT, PDQSORT, LINUX_SORT } sort_method_t;

extern const char *get_sort_method_name(sort_method_t method);

static inline int is_valid_sort_method(int method)
{
    return method >= QSORT && method <= LINUX_SORT;
}

#endif  // SORT_TYPES_H