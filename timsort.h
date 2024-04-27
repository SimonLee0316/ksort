#include <linux/workqueue.h>
struct list_head;

typedef int (*list_cmp_func_t)(void *,
                               const struct list_head *,
                               const struct list_head *);

struct timsort_for_work {
    struct work_struct w;
    size_t n;
    list_cmp_func_t tim_cmp;
    struct list_head sample_head;
};

typedef struct element {
    struct list_head list;
    int val;
} element_t;



void timsort(struct work_struct *w);