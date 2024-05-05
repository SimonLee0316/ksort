#ifndef KSORT_H
#define KSORT_H

#include <linux/types.h>
#include "sort_types.h"

typedef int cmp_t(const void *, const void *);


extern struct workqueue_struct *workqueue;

void sort_main(void *sort_buffer,
               size_t size,
               size_t es,
               sort_method_t sort_method);

#endif
