/*
 * Quicksort benchmark: four variants on singly-linked list.
 *
 * 1. qsort_original  - two recursive calls (2021q1 quiz1)
 * 2. qsort_tco       - linD026's "TCO" (skip empty partitions)
 * 3. qsort_method2   - recurse smaller, loop larger (O(log n) stack)
 * 4. qsort_iterative - explicit stack, no recursion
 *
 * Usage:
 *   ./qsort_bench                  # print human-readable table
 *   ./qsort_bench --csv [file]     # output per-trial CSV (stdout or file)
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "qsort.h"

static bool is_sorted(node_t *list)
{
    if (!list)
        return true;
    while (list->next) {
        if (list->value > list->next->value)
            return false;
        list = list->next;
    }
    return true;
}

static double bench_one(qsort_fn fn, int *values, int n)
{
    node_t *list = make_list(values, n);

    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);
    fn(&list);
    clock_gettime(CLOCK_MONOTONIC, &t1);

    if (!is_sorted(list)) {
        fprintf(stderr, "SORT FAILED\n");
        exit(1);
    }
    free_list(list);

    return (t1.tv_sec - t0.tv_sec) * 1e9 + (t1.tv_nsec - t0.tv_nsec);
}

static void shuffle(int *arr, int n)
{
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int tmp = arr[i];
        arr[i] = arr[j];
        arr[j] = tmp;
    }
}

static int *dup_array(int *src, int n)
{
    int *dst = malloc(n * sizeof(int));
    memcpy(dst, src, n * sizeof(int));
    return dst;
}

static void run_csv(FILE *out, int n, int trials)
{
    int *values = malloc(n * sizeof(int));
    for (int i = 0; i < n; i++)
        values[i] = i;

    fprintf(out, "trial,original_ns,tco_ns,method2_ns,iterative_ns\n");

    for (int t = 0; t < trials; t++) {
        shuffle(values, n);

        int *v1 = dup_array(values, n);
        int *v2 = dup_array(values, n);
        int *v3 = dup_array(values, n);
        int *v4 = dup_array(values, n);

        double t_orig = bench_one(qsort_original,  v1, n);
        double t_tco  = bench_one(qsort_tco,       v2, n);
        double t_m2   = bench_one(qsort_method2,   v3, n);
        double t_iter = bench_one(qsort_iterative, v4, n);

        fprintf(out, "%d,%.0f,%.0f,%.0f,%.0f\n",
                t, t_orig, t_tco, t_m2, t_iter);

        free(v1);
        free(v2);
        free(v3);
        free(v4);
    }

    free(values);
}

static void run_table(int trials)
{
    const int sizes[] = {1000, 5000, 10000, 50000, 100000};
    const int nsizes = sizeof(sizes) / sizeof(sizes[0]);

    printf("Random input (average of %d trials):\n\n", trials);
    printf("%-8s  %-14s  %-14s  %-14s  %-14s\n",
           "n", "original (ms)", "tco (ms)", "method2 (ms)", "iterative (ms)");
    printf("%-8s  %-14s  %-14s  %-14s  %-14s\n",
           "--------", "--------------", "--------------",
           "--------------", "--------------");

    for (int si = 0; si < nsizes; si++) {
        int n = sizes[si];
        int *values = malloc(n * sizeof(int));
        for (int i = 0; i < n; i++)
            values[i] = i;

        double t_orig = 0, t_tco = 0, t_m2 = 0, t_iter = 0;

        for (int t = 0; t < trials; t++) {
            shuffle(values, n);

            int *v1 = dup_array(values, n);
            int *v2 = dup_array(values, n);
            int *v3 = dup_array(values, n);
            int *v4 = dup_array(values, n);

            t_orig += bench_one(qsort_original,  v1, n);
            t_tco  += bench_one(qsort_tco,       v2, n);
            t_m2   += bench_one(qsort_method2,   v3, n);
            t_iter += bench_one(qsort_iterative, v4, n);

            free(v1);
            free(v2);
            free(v3);
            free(v4);
        }

        /* Convert ns average to ms */
        printf("%-8d  %-14.3f  %-14.3f  %-14.3f  %-14.3f\n",
               n,
               t_orig / trials / 1e6,
               t_tco  / trials / 1e6,
               t_m2   / trials / 1e6,
               t_iter / trials / 1e6);

        free(values);
    }
}

int main(int argc, char **argv)
{
    const int n = 10000;
    const int trials = 1000;

    srand(time(NULL));

    if (argc >= 2 && strcmp(argv[1], "--csv") == 0) {
        FILE *out = stdout;
        if (argc >= 3) {
            out = fopen(argv[2], "w");
            if (!out) {
                perror(argv[2]);
                return 1;
            }
        }
        run_csv(out, n, trials);
        if (out != stdout)
            fclose(out);
    } else {
        run_table(trials);
    }

    return 0;
}
