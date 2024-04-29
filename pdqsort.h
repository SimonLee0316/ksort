#include <linux/types.h>

void pdqsort(void *base,
             size_t num,
             size_t size,
             cmp_func_t cmp_func,
             swap_func_t swap_func);