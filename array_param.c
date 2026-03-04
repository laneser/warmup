/* Array parameter decay: f(int a[10]) vs f(int *a) vs g(int (*a)[10])
 *
 * C99 §6.7.5.3¶7: "A declaration of a parameter as 'array of type'
 * shall be adjusted to 'qualified pointer to type'."
 *
 * Compile: gcc -std=c99 -Wall -Wextra -o array_param array_param.c
 */
#include <stdio.h>

/* f1 and f2 are the SAME after array parameter decay */
void f1(int a[10])
{
    printf("[f1] sizeof(a)  = %zu\n", sizeof(a));    /* pointer size */
    printf("[f1] sizeof(*a) = %zu\n", sizeof(*a));   /* sizeof(int) */
    printf("[f1] a          = %p\n", (void *)a);
    printf("[f1] a+1        = %p  (delta = %td)\n",
           (void *)(a+1), (char *)(a+1) - (char *)a);
    printf("[f1] &a type    : int **  (addr = %p)\n", (void *)&a);
    printf("\n");
}

void f2(int *a)
{
    printf("[f2] sizeof(a)  = %zu\n", sizeof(a));
    printf("[f2] sizeof(*a) = %zu\n", sizeof(*a));
    printf("[f2] a          = %p\n", (void *)a);
    printf("[f2] a+1        = %p  (delta = %td)\n",
           (void *)(a+1), (char *)(a+1) - (char *)a);
    printf("[f2] &a type    : int **  (addr = %p)\n", (void *)&a);
    printf("\n");
}

/* g receives a POINTER TO ARRAY — not decayed */
void g(int (*a)[10])
{
    printf("[g]  sizeof(a)  = %zu\n", sizeof(a));    /* pointer size */
    printf("[g]  sizeof(*a) = %zu\n", sizeof(*a));   /* sizeof(int[10]) */
    printf("[g]  a          = %p\n", (void *)a);
    printf("[g]  a+1        = %p  (delta = %td)\n",
           (void *)(a+1), (char *)(a+1) - (char *)a);
    printf("[g]  &a type    : int (**)[10]  (addr = %p)\n", (void *)&a);
    printf("\n");
}

int main(void)
{
    int arr[10] = {0};

    printf("arr        = %p\n", (void *)arr);
    printf("sizeof(arr)= %zu\n\n", sizeof(arr));

    f1(arr);        /* arr decays to int*, then parameter decays too */
    f2(arr);        /* arr decays to int* */
    g(&arr);        /* &arr is int (*)[10], no decay */

    return 0;
}
