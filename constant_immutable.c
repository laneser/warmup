/*
 * constant_immutable.c - Verify that C99 'const' means immutable, not constant.
 *
 * Three experiments:
 *   1. const variable is not a constant expression (C99 6.6)
 *   2. Casting away const is UB -- observable via optimization (C99 6.7.3.5)
 *   3. Global const placed in read-only memory (C99 6.7.3, footnote 114)
 */

#include <stdio.h>

#define BUFSIZE 256
enum { MAX_ITEMS = 100 };

const int g_const = 42;
int g_mutable = 42;

static void check_writable(const char *label, const void *addr)
{
    FILE *maps = fopen("/proc/self/maps", "r");
    if (!maps)
        return;

    unsigned long target = (unsigned long)addr;
    unsigned long start, end;
    char perms[8], line[256];

    while (fgets(line, sizeof(line), maps)) {
        if (sscanf(line, "%lx-%lx %7s", &start, &end, perms) == 3 &&
            target >= start && target < end) {
            printf("[exp3:%s] %-12s perms=%s writable=%s\n",
                   label, label, perms, perms[1] == 'w' ? "yes" : "no");
            break;
        }
    }
    fclose(maps);
}

int main(void)
{
    /* Experiment 1: const var is NOT a constant expression.
     * Uncomment to see compile error:
     *   const int N = 10;
     *   switch (0) { case N: break; }  */
    const int n = 10;
    int vla[n];
    (void)vla;
    printf("[exp1] define=%d enum=%d sizeof=%zu const_var=%d\n",
           BUFSIZE, MAX_ITEMS, sizeof(int), n);

    /* Experiment 2: casting away const is UB (C99 6.7.3.5). */
    const int local = 100;
    int *p = (int *)&local;
    *p = 999;
    printf("[result] local_name=%d local_ptr=%d\n", local, *p);

    /* Experiment 3: memory region permissions. */
    check_writable("g_const", &g_const);
    check_writable("g_mutable", &g_mutable);
    check_writable("local_const", &local);

    return 0;
}
