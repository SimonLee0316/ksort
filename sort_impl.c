#include <linux/slab.h>
#include <linux/sort.h>
#include <linux/time.h>
#include <linux/workqueue.h>

#include "sort.h"
#include "timsort.h"

static inline char *med3(char *, char *, char *, cmp_t *, void *);
static inline void swapfunc(char *, char *, int, int);

/* Qsort routine from Bentley & McIlroy's "Engineering a Sort Function" */
#define swapcode(TYPE, parmi, parmj, n) \
    {                                   \
        long i = (n) / sizeof(TYPE);    \
        TYPE *pi = (TYPE *) (parmi);    \
        TYPE *pj = (TYPE *) (parmj);    \
        do {                            \
            TYPE t = *pi;               \
            *pi++ = *pj;                \
            *pj++ = t;                  \
        } while (--i > 0);              \
    }

static inline void swapfunc(char *a, char *b, int n, int swaptype)
{
    if (swaptype <= 1)
        swapcode(long, a, b, n) else swapcode(char, a, b, n)
}

#define q_swap(a, b)                       \
    do {                                   \
        if (swaptype == 0) {               \
            long t = *(long *) (a);        \
            *(long *) (a) = *(long *) (b); \
            *(long *) (b) = t;             \
        } else                             \
            swapfunc(a, b, es, swaptype);  \
    } while (0)

#define vecswap(a, b, n)                 \
    do {                                 \
        if ((n) > 0)                     \
            swapfunc(a, b, n, swaptype); \
    } while (0)

#define CMP(t, x, y) (cmp((x), (y)))

static inline char *med3(char *a,
                         char *b,
                         char *c,
                         cmp_t *cmp,
                         __attribute__((unused)) void *thunk)
{
    return CMP(thunk, a, b) < 0
               ? (CMP(thunk, b, c) < 0 ? b : (CMP(thunk, a, c) < 0 ? c : a))
               : (CMP(thunk, b, c) > 0 ? b : (CMP(thunk, a, c) < 0 ? a : c));
}

struct common {
    int swaptype; /* Code to use for swapping */
    size_t es;    /* Element size. */
    cmp_t *cmp;   /* Comparison function */
};

struct qsort {
    struct work_struct w;
    struct common *common;
    void *a;
    size_t n;
};

#define thunk NULL
static void qsort_algo(struct work_struct *w);

static void init_qsort(struct qsort *q,
                       void *elems,
                       size_t size,
                       struct common *common)
{
    INIT_WORK(&q->w, qsort_algo);
    q->a = elems;
    q->n = size;
    q->common = common;
}


static void qsort_algo(struct work_struct *w)
{
    struct qsort *qs = container_of(w, struct qsort, w);

    bool do_free = true;
    char *pa, *pb, *pc, *pd, *pl, *pm, *pn;
    int d, r, swaptype, swap_cnt;
    void *a;      /* Array of elements. */
    size_t n, es; /* Number of elements; size. */
    cmp_t *cmp;
    size_t nl, nr;
    struct common *c;

    /* Initialize qsort arguments. */
    c = qs->common;
    es = c->es;
    cmp = c->cmp;
    swaptype = c->swaptype;
    a = qs->a;
    n = qs->n;
top:
    /* From here on qsort(3) business as usual. */
    swap_cnt = 0;
    if (n < 7) {
        for (pm = (char *) a + es; pm < (char *) a + n * es; pm += es)
            for (pl = pm; pl > (char *) a && CMP(thunk, pl - es, pl) > 0;
                 pl -= es)
                q_swap(pl, pl - es);
        return;
    }
    pm = (char *) a + (n / 2) * es;
    if (n > 7) {
        pl = (char *) a;
        pn = (char *) a + (n - 1) * es;
        if (n > 40) {
            d = (n / 8) * es;
            pl = med3(pl, pl + d, pl + 2 * d, cmp, thunk);
            pm = med3(pm - d, pm, pm + d, cmp, thunk);
            pn = med3(pn - 2 * d, pn - d, pn, cmp, thunk);
        }
        pm = med3(pl, pm, pn, cmp, thunk);
    }
    q_swap(a, pm);
    pa = pb = (char *) a + es;

    pc = pd = (char *) a + (n - 1) * es;
    for (;;) {
        while (pb <= pc && (r = CMP(thunk, pb, a)) <= 0) {
            if (r == 0) {
                swap_cnt = 1;
                q_swap(pa, pb);
                pa += es;
            }
            pb += es;
        }
        while (pb <= pc && (r = CMP(thunk, pc, a)) >= 0) {
            if (r == 0) {
                swap_cnt = 1;
                q_swap(pc, pd);
                pd -= es;
            }
            pc -= es;
        }
        if (pb > pc)
            break;
        q_swap(pb, pc);
        swap_cnt = 1;
        pb += es;
        pc -= es;
    }

    pn = (char *) a + n * es;
    r = min(pa - (char *) a, pb - pa);
    vecswap(a, pb - r, r);
    r = min(pd - pc, pn - pd - (long) es);
    vecswap(pb, pn - r, r);

    if (swap_cnt == 0) { /* Switch to insertion sort */
        r = 1 + n / 4;   /* n >= 7, so r >= 2 */
        for (pm = (char *) a + es; pm < (char *) a + n * es; pm += es)
            for (pl = pm; pl > (char *) a && CMP(thunk, pl - es, pl) > 0;
                 pl -= es) {
                q_swap(pl, pl - es);
                if (++swap_cnt > r)
                    goto nevermind;
            }
        return;
    }

nevermind:
    nl = (pb - pa) / es;
    nr = (pd - pc) / es;

    if (nl > 100 && nr > 100) {
        struct qsort *q = kmalloc(sizeof(struct qsort), GFP_KERNEL);
        init_qsort(q, a, nl, c);
        queue_work(workqueue, &q->w);
    } else if (nl > 0) {
        qs->a = a;
        qs->n = nl;
        /* The struct qsort is used for recursive call, so don't free it in
         * this iteration.
         */
        do_free = false;
        qsort_algo(w);
    }

    if (nr > 0) {
        a = pn - nr * es;
        n = nr;
        goto top;
    }

    if (do_free)
        kfree(qs);
}

/*timsort*/
struct timsort {
    struct work_struct w;
    struct list_head *head;
    list_cmp_func_t cmp;
};

static void timsort_func(struct work_struct *w);

static void init_timsort(struct timsort *t,
                         struct list_head *head,
                         list_cmp_func_t cmp)
{
    INIT_WORK(&t->w, timsort_func);
    t->head = head;
    t->cmp = cmp;
}

static void timsort_func(struct work_struct *w)
{
    struct timsort *ts = container_of(w, struct timsort, w);

    if (!ts->head) {
        printk(KERN_ERR "Error: ts->head is NULL\n");
        return;
    }

    if (!ts->cmp) {
        printk(KERN_ERR "Error: ts->cmp is NULL\n");
        return;
    }

    timsort_algo(NULL, ts->head, ts->cmp);
}
/* Function for list */
static void buf_to_list(struct list_head *head, void *buf, size_t size)
{
    int *int_buf = (int *) buf;
    for (size_t i = 0; i < size; i++) {
        element_t *new_node = kmalloc(sizeof(*new_node), GFP_KERNEL);
        if (!new_node) {
            printk(KERN_ERR "Failed to allocate memory for new node\n");
            return;
        }
        new_node->val = int_buf[i];
        list_add_tail(&new_node->list, head);
    }
}

bool list_cmp(void *priv,
              struct list_head *a,
              struct list_head *b,
              bool descend)
{
    if (priv)
        *((unsigned long long *) priv) += 1;
    element_t *a_entry = list_entry(a, element_t, list);
    element_t *b_entry = list_entry(b, element_t, list);
    return (a_entry->val > b_entry->val) ^ descend;
}
static void list_to_buf(struct list_head *head, void *buf, size_t size)
{
    int *int_buf = (int *) buf;
    struct list_head *pos;
    element_t *entry;
    size_t i = 0;

    list_for_each (pos, head) {
        entry = list_entry(pos, element_t, list);
        int_buf[i++] = entry->val;
    }
}
/*linux sort.h*/
struct linuxsort {
    struct work_struct w;
    struct common *common;
    void *a;
    size_t n;
};

static void linuxsort_algo(struct work_struct *w);

static void init_linuxsort(struct linuxsort *ls,
                           void *elems,
                           size_t size,
                           struct common *common)
{
    INIT_WORK(&ls->w, linuxsort_algo);
    ls->a = elems;
    ls->n = size;
    ls->common = common;
}

static void linuxsort_algo(struct work_struct *w)
{
    struct linuxsort *ls = container_of(w, struct linuxsort, w);

    void *a;  /* Array of elements. */
    size_t n; /* Number of elements; size. */
    cmp_t *cmp;
    struct common *c;

    /* Initialize linux sort arguments. */
    c = ls->common;
    cmp = c->cmp;
    a = ls->a;
    n = ls->n;

    sort(a, n, sizeof(int), cmp, NULL);
}

int num_cmp(const void *a, const void *b)
{
    return (*(int *) a - *(int *) b);
}

ktime_t sort_main(void *sort_buffer,
                  size_t size,
                  size_t es,
                  sort_method_t sort_method)
{
    /* The allocation must be dynamic so that the pointer can be reliably freed
     * within the work function.
     */
    static ktime_t kt;  // evaluate kernal module sorting time

    struct common common;
    switch (sort_method) {
    case TIMSORT:
        printk(KERN_INFO "Do TIMSORT\n");

        struct timsort *t = kmalloc(sizeof(struct timsort), GFP_KERNEL);

        struct list_head *head =
            (struct list_head *) kmalloc(sizeof(*head), GFP_KERNEL);
        INIT_LIST_HEAD(head);

        buf_to_list(head, sort_buffer, size);

        init_timsort(t, head, list_cmp);

        kt = ktime_get(); /*sorting time*/
        queue_work(workqueue, &t->w);

        drain_workqueue(workqueue);
        kt = ktime_sub(ktime_get(), kt);

        list_to_buf(head, sort_buffer, size);

        /* Free list */
        element_t *node, *safe;
        list_for_each_entry_safe (node, safe, head, list) {
            list_del(&node->list);
            kfree(node);
        }
        break;
    case LINUX_SORT:
        printk(KERN_INFO "Do LINUXSORT\n");

        struct linuxsort *ls = kmalloc(sizeof(struct linuxsort), GFP_KERNEL);

        common.cmp = num_cmp;

        init_linuxsort(ls, sort_buffer, size, &common);

        kt = ktime_get(); /*sorting time*/
        queue_work(workqueue, &ls->w);

        drain_workqueue(workqueue);
        kt = ktime_sub(ktime_get(), kt);

        break;
    case QSORT:
        printk(KERN_INFO "Do QSORT\n");

        struct qsort *q = kmalloc(sizeof(struct qsort), GFP_KERNEL);

        common.swaptype = ((char *) sort_buffer - (char *) 0) % sizeof(long) ||
                                  es % sizeof(long)
                              ? 2
                          : es == sizeof(long) ? 0
                                               : 1;
        common.es = es;
        common.cmp = num_cmp;

        init_qsort(q, sort_buffer, size, &common);

        kt = ktime_get();
        queue_work(workqueue, &q->w);

        /* Ensure completion of all work before proceeding, as reliance on
         * objects allocated on the stack necessitates this. If not, there is a
         * risk of the work item referencing a pointer that has ceased to exist.
         */
        drain_workqueue(workqueue);
        kt = ktime_sub(ktime_get(), kt);
        break;
    case PDQSORT:
        printk(KERN_INFO "Start sorting: pdqsort\n");
        break;
    default:
        printk(KERN_WARNING "Unknown sort method selected\n");
        break;
    }
    return kt;
}
