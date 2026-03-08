/*
 * list_arr_bench.c - Benchmark linked list O(1) insert vs array O(n) insert.
 *
 * Measures wall-clock time for inserting N elements at a given position
 * (head, tail, or random) into:
 *   1. A doubly-linked circular list (kernel-style, with per-node malloc)
 *   2. A dynamic array (doubling strategy, with memmove)
 *
 * Usage: ./list_arr_bench [max_n] [trials] [elem_size]
 *   max_n     - maximum number of elements (default: 100000)
 *   trials    - repetitions per data point (default: 3)
 *   elem_size - element payload size in bytes (default: 4)
 *
 * Output: CSV to stdout for plotting.
 */

#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ---- Doubly-linked circular list (kernel-style, simplified) ---- */

struct list_head {
    struct list_head *prev, *next;
};

/*
 * Node with flexible array member for variable-size payload.
 * Total malloc size = sizeof(list_head) + elem_size.
 */
struct ll_node {
    struct list_head list;
    char data[];
};

static inline void list_init(struct list_head *head)
{
    head->prev = head;
    head->next = head;
}

static inline void list_add_between(struct list_head *new,
                                    struct list_head *prev,
                                    struct list_head *next)
{
    next->prev = new;
    new->next = next;
    new->prev = prev;
    prev->next = new;
}

static size_t g_node_size; /* sizeof(struct ll_node) + elem_size */

static struct ll_node *ll_alloc_node(void)
{
    return malloc(g_node_size);
}

/* Insert right after sentinel (head of list). O(1). */
static void ll_insert_head(struct list_head *head)
{
    struct ll_node *node = ll_alloc_node();
    list_add_between(&node->list, head, head->next);
}

/* Insert before sentinel (tail of list). O(1) via circular link. */
static void ll_insert_tail(struct list_head *head)
{
    struct ll_node *node = ll_alloc_node();
    list_add_between(&node->list, head->prev, head);
}

/*
 * Insert at position i (0-indexed), with bidirectional traversal.
 * If i <= n/2, traverse forward; otherwise backward from tail.
 */
static void ll_insert_at(struct list_head *head, int pos, int size)
{
    struct ll_node *node = ll_alloc_node();

    struct list_head *cur;
    if (pos <= size / 2) {
        cur = head;
        for (int j = 0; j < pos; j++)
            cur = cur->next;
    } else {
        cur = head;
        for (int j = size; j > pos; j--)
            cur = cur->prev;
    }

    list_add_between(&node->list, cur, cur->next);
}

static void ll_free(struct list_head *head)
{
    struct list_head *cur = head->next;
    while (cur != head) {
        struct list_head *next = cur->next;
        free((char *)cur -
             __builtin_offsetof(struct ll_node, list));
        cur = next;
    }
}

/* ---- Dynamic array (doubling strategy, variable element size) ---- */

struct dyn_array {
    char *data;
    int size;
    int capacity;
    int elem_size;
};

static void da_init(struct dyn_array *da, int elem_size)
{
    da->capacity = 4;
    da->elem_size = elem_size;
    da->data = malloc(da->capacity * elem_size);
    da->size = 0;
}

static void da_insert_at(struct dyn_array *da, int pos)
{
    if (da->size == da->capacity) {
        da->capacity *= 2;
        da->data = realloc(da->data, da->capacity * da->elem_size);
    }
    int es = da->elem_size;
    /* Shift elements [pos, size) right by one */
    memmove(da->data + (pos + 1) * es,
            da->data + pos * es,
            (da->size - pos) * es);
    /* Zero-fill the new slot (simulate writing a value) */
    memset(da->data + pos * es, 0, es);
    da->size++;
}

static void da_free(struct dyn_array *da)
{
    free(da->data);
}

/* ---- Timing helper ---- */

static inline double time_diff_ns(struct timespec *start, struct timespec *end)
{
    return (end->tv_sec - start->tv_sec) * 1e9 +
           (end->tv_nsec - start->tv_nsec);
}

/* ---- Benchmark modes ---- */

enum insert_mode { INSERT_HEAD, INSERT_TAIL, INSERT_RANDOM };

static const char *mode_name[] = {"head", "tail", "random"};

static double bench_ll(int n, enum insert_mode mode, unsigned int seed)
{
    struct list_head head;
    list_init(&head);

    srand(seed);
    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);

    for (int i = 0; i < n; i++) {
        switch (mode) {
        case INSERT_HEAD:
            ll_insert_head(&head);
            break;
        case INSERT_TAIL:
            ll_insert_tail(&head);
            break;
        case INSERT_RANDOM:
            ll_insert_at(&head, rand() % (i + 1), i);
            break;
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &t1);
    double elapsed = time_diff_ns(&t0, &t1);

    ll_free(&head);
    return elapsed;
}

static double bench_da(int n, enum insert_mode mode, unsigned int seed,
                       int elem_size)
{
    struct dyn_array da;
    da_init(&da, elem_size);

    srand(seed);
    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);

    for (int i = 0; i < n; i++) {
        int pos;
        switch (mode) {
        case INSERT_HEAD:
            pos = 0;
            break;
        case INSERT_TAIL:
            pos = i;
            break;
        case INSERT_RANDOM:
            pos = rand() % (i + 1);
            break;
        }
        da_insert_at(&da, pos);
    }

    clock_gettime(CLOCK_MONOTONIC, &t1);
    double elapsed = time_diff_ns(&t0, &t1);

    da_free(&da);
    return elapsed;
}

int main(int argc, char *argv[])
{
    int max_n = (argc > 1) ? atoi(argv[1]) : 100000;
    int trials = (argc > 2) ? atoi(argv[2]) : 3;
    int elem_size = (argc > 3) ? atoi(argv[3]) : 4;

    /* Set global node size for linked list */
    g_node_size = sizeof(struct ll_node) + elem_size;

    printf("n,mode,elem_size,ll_ns,da_ns,ll_per_op,da_per_op\n");

    for (enum insert_mode mode = INSERT_HEAD; mode <= INSERT_RANDOM; mode++) {
        int steps[] = {10, 20, 50, 100, 200, 500, 1000, 2000, 5000,
                       10000, 20000, 50000, 100000, 200000, 500000};
        int nsteps = sizeof(steps) / sizeof(steps[0]);

        for (int si = 0; si < nsteps; si++) {
            int n = steps[si];
            if (n > max_n)
                break;

            double ll_total = 0, da_total = 0;
            for (int t = 0; t < trials; t++) {
                unsigned int seed = 42 + t;
                ll_total += bench_ll(n, mode, seed);
                da_total += bench_da(n, mode, seed, elem_size);
            }

            double ll_avg = ll_total / trials;
            double da_avg = da_total / trials;
            printf("%d,%s,%d,%.0f,%.0f,%.1f,%.1f\n", n, mode_name[mode],
                   elem_size, ll_avg, da_avg, ll_avg / n, da_avg / n);
        }
    }

    return 0;
}
