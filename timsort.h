#ifndef TIMSORT_H
#define TIMSORT_H

#include <linux/types.h>

typedef struct {
    int val;
    struct list_head list;
} element_t;


/* Compare function type for list */
typedef bool (*list_cmp_func_t)(void *,
                                struct list_head *,
                                struct list_head *,
                                bool);



void timsort_algo(void *priv, struct list_head *head, list_cmp_func_t cmp);

#endif  // TIMSORT_H