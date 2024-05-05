#include "sort_types.h"

const char *get_sort_method_name(sort_method_t method)
{
    switch (method) {
    case QSORT:
        return "Quick Sort";
    case TIMSORT:
        return "Tim Sort";
    case PDQSORT:
        return "Pattern-defeating Quick Sort";
    case LINUX_SORT:
        return "Library Sort";
    default:
        return "Unknown Method";
    }
}