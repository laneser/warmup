/* Demonstrate: object lifetime ends, storage not yet reused,
 * pointer retains original value, but dereference is UB.
 *
 * C99 §6.2.4¶5: automatic storage duration lifetime extends from
 * entry into the block until execution of that block ends.
 *
 * Compile with: gcc -std=c99 -Wall -Wextra -O{0,2} -o lifetime_ub lifetime_ub.c
 */
#include <stdio.h>

/* Version 1: direct return — GCC detects and replaces with NULL */
int *foo(int x)
{
    return &x;
}

/* Version 2: smuggle the address out via output parameter,
 * bypassing the compiler's -Wreturn-local-addr detection. */
void bar(int x, int **out)
{
    *out = &x;   /* x has automatic storage duration */
}
/* bar returns, x's lifetime ends, but *out still holds &x */

int main(void)
{
    /* Version 1: compiler returns NULL (exploiting UB) */
    int *p1 = foo(42);
    printf("[v1] foo(42) returned: p1 = %p\n", (void *)p1);

    /* Version 2: smuggled address — may still hold original value */
    int *p2;
    bar(42, &p2);

    printf("[v2] p2 = %p, *p2 = %d  (UB: lifetime ended)\n", (void *)p2, *p2);

    /* Overwrite the stack to show storage reuse */
    int *p3;
    bar(99, &p3);
    printf("[v3] after bar(99):    p2 = %p, *p2 = %d  (storage may be reused)\n",
           (void *)p2, *p2);

    return 0;
}
